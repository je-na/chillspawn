// Base class for chat commands. To add one, subclass this and register it in
// CS_Core.RegisterCommands (see README).
class CS_ChatCommand
{
	string GetName()
	{
		return "";
	}

	// Admin-only commands are hidden from other players and ignored if they type them.
	bool IsAdminOnly()
	{
		return false;
	}

	void Execute(PlayerBase player, array<string> args)
	{
	}
}

class CS_CommandRegistry
{
	protected string m_Prefix;
	protected ref map<string, ref CS_ChatCommand> m_Commands;

	void CS_CommandRegistry(string prefix)
	{
		m_Prefix = prefix;
		m_Commands = new map<string, ref CS_ChatCommand>;
	}

	void Register(CS_ChatCommand command)
	{
		string name = command.GetName();
		name.ToLower();

		if (m_Commands.Contains(name))
			Print("[ChillSpawn] Command " + name + " registered twice; the last one wins.");

		m_Commands.Set(name, command);
	}

	string GetPrefix()
	{
		return m_Prefix;
	}

	void GetNames(out array<string> names, bool includeAdminOnly)
	{
		names = new array<string>;
		foreach (string name, CS_ChatCommand command : m_Commands)
		{
			if (command.IsAdminOnly() && !includeAdminOnly)
				continue;

			names.Insert(name);
		}

		names.Sort();
	}

	// Returns true when the text was one of our commands.
	bool TryDispatch(PlayerBase player, string text)
	{
		text = text.Trim();
		if (m_Prefix == "" || text.IndexOf(m_Prefix) != 0)
			return false;

		text = text.Substring(m_Prefix.Length(), text.Length() - m_Prefix.Length());

		array<string> args = new array<string>;
		text.Split(" ", args);
		if (args.Count() == 0)
			return false;

		string name = args[0];
		name.ToLower();
		args.RemoveOrdered(0);

		CS_ChatCommand command = m_Commands.Get(name);
		if (!command)
			return false;

		if (command.IsAdminOnly() && !CS_Core.Get().IsAdmin(player))
			return false;

		command.Execute(player, args);
		return true;
	}
}

// Per-player spawn limits counted in playtime. Admins have no limits.
class CS_Limits
{
	// True if the player may spawn now; otherwise tells them how long to wait.
	// realTime counts the limit in real-world minutes instead of playtime.
	static bool Allow(PlayerBase player, string key, int maxSpawns, int limitMinutes, bool realTime)
	{
		CS_Core core = CS_Core.Get();
		if (core.IsAdmin(player))
			return true;

		float waitSeconds;
		int used = core.Tracker.CountRecentSpawns(player.GetIdentity().GetId(), key, limitMinutes * 60, realTime, waitSeconds);
		if (used < maxSpawns)
			return true;

		int waitMinutes = Math.Ceil(waitSeconds / 60.0);
		string unit = " minutes of play";
		if (realTime)
			unit = " minutes";

		CS_Messenger.Send(player, string.Format("You can spawn this %1 time(s) every %2%3. Try again in %4%3.", maxSpawns, limitMinutes, unit, waitMinutes));
		return false;
	}

	static void Record(PlayerBase player, string key)
	{
		CS_Core core = CS_Core.Get();
		if (!core.IsAdmin(player))
			core.Tracker.RecordSpawn(player.GetIdentity().GetId(), key);
	}

	// One line for !list: what the command spawns and how many are left.
	static string Describe(PlayerBase player, string command, string displayName, string key, int maxSpawns, int limitMinutes, bool realTime)
	{
		CS_Core core = CS_Core.Get();
		string prefix = core.Commands.GetPrefix();

		if (core.IsAdmin(player))
			return string.Format("%1%2 - %3 (no limit)", prefix, command, displayName);

		float waitSeconds;
		int used = core.Tracker.CountRecentSpawns(player.GetIdentity().GetId(), key, limitMinutes * 60, realTime, waitSeconds);
		int left = maxSpawns - used;
		if (left < 0)
			left = 0;

		string every = string.Format("every %1 min of play", limitMinutes);
		if (realTime)
			every = string.Format("every %1 min", limitMinutes);

		string line = string.Format("%1%2 - %3 (%4 of %5 left, %6)", prefix, command, displayName, left, maxSpawns, every);
		if (left == 0)
		{
			int waitMinutes = Math.Ceil(waitSeconds / 60.0);
			line = line + string.Format(", next in %1 min", waitMinutes);
		}

		return line;
	}
}

// Spawns a configured preset, enforcing its playtime limit.
class CS_SpawnPresetCommand : CS_ChatCommand
{
	protected ref CS_SpawnPreset m_Preset;

	void CS_SpawnPresetCommand(CS_SpawnPreset preset)
	{
		m_Preset = preset;
	}

	override string GetName()
	{
		return m_Preset.Command;
	}

	override bool IsAdminOnly()
	{
		return m_Preset.AdminOnly;
	}

	override void Execute(PlayerBase player, array<string> args)
	{
		if (!player.IsAlive() || !player.GetIdentity())
			return;

		if (player.IsInVehicle())
		{
			CS_Messenger.Send(player, "Get out of your vehicle first.");
			return;
		}

		if (!CS_Limits.Allow(player, m_Preset.Command, m_Preset.MaxSpawns, m_Preset.LimitMinutes, false))
			return;

		string error;
		EntityAI ent = CS_Core.Get().Spawner.Spawn(player, m_Preset, error);
		if (!ent && error == CS_VehicleSpawner.ERR_NO_SPACE)
		{
			CS_Messenger.Send(player, "There is no clear space here. Move to open, dry ground and try again.");
			return;
		}

		if (!ent)
		{
			Print("[ChillSpawn] Spawn failed for " + m_Preset.Command + ": " + error);
			CS_Messenger.Send(player, "Spawn failed. Tell a server admin.");
			return;
		}

		CS_Limits.Record(player, m_Preset.Command);

		int despawnMinutes = Math.Ceil(m_Preset.DespawnSeconds / 60.0);
		CS_Messenger.Send(player, string.Format(m_Preset.SpawnMessage, m_Preset.DisplayName, despawnMinutes));
	}
}

// Drops a configured kit at the player's feet.
class CS_KitCommand : CS_ChatCommand
{
	protected ref CS_Kit m_Kit;

	void CS_KitCommand(CS_Kit kit)
	{
		m_Kit = kit;
	}

	override string GetName()
	{
		return m_Kit.Command;
	}

	override bool IsAdminOnly()
	{
		return m_Kit.AdminOnly;
	}

	// Kits share the limit history with presets, so their keys are kept apart.
	string GetLimitKey()
	{
		return "kit:" + m_Kit.Command;
	}

	override void Execute(PlayerBase player, array<string> args)
	{
		if (!player.IsAlive() || !player.GetIdentity())
			return;

		if (player.IsInVehicle())
		{
			CS_Messenger.Send(player, "Get out of your vehicle first.");
			return;
		}

		if (!CS_Limits.Allow(player, GetLimitKey(), m_Kit.MaxSpawns, m_Kit.LimitMinutes, m_Kit.RealTimeLimit))
			return;

		if (CS_Core.Get().KitSpawner.Spawn(player, m_Kit) == 0)
		{
			Print("[ChillSpawn] Kit " + m_Kit.Command + " spawned nothing.");
			CS_Messenger.Send(player, "Spawn failed. Tell a server admin.");
			return;
		}

		CS_Limits.Record(player, GetLimitKey());

		if (m_Kit.ToInventory)
			CS_Messenger.Send(player, string.Format("Your %1 is in your inventory. Anything that didn't fit is on the ground in front of you.", m_Kit.DisplayName));
		else
			CS_Messenger.Send(player, string.Format("Your %1 is on the ground in front of you.", m_Kit.DisplayName));
	}
}

// Lists everything the player can spawn, with how many they have left. One line
// per entry so long lists don't get cut off in chat.
class CS_ListCommand : CS_ChatCommand
{
	override string GetName()
	{
		return "list";
	}

	override void Execute(PlayerBase player, array<string> args)
	{
		if (!player.GetIdentity())
			return;

		CS_Core core = CS_Core.Get();
		bool admin = core.IsAdmin(player);
		array<string> lines = new array<string>;

		foreach (CS_SpawnPreset preset : core.Config.Presets)
		{
			if (!preset.AdminOnly || admin)
				lines.Insert(CS_Limits.Describe(player, preset.Command, preset.DisplayName, preset.Command, preset.MaxSpawns, preset.LimitMinutes, false));
		}

		foreach (CS_Kit kit : core.Config.Kits)
		{
			if (!kit.AdminOnly || admin)
				lines.Insert(CS_Limits.Describe(player, kit.Command, kit.DisplayName, "kit:" + kit.Command, kit.MaxSpawns, kit.LimitMinutes, kit.RealTimeLimit));
		}

		if (lines.Count() == 0)
		{
			CS_Messenger.Send(player, "Nothing is set up to spawn. Ask a server admin.");
			return;
		}

		CS_Messenger.Send(player, "You can spawn:");
		foreach (string line : lines)
			CS_Messenger.Send(player, line);
	}
}

class CS_HelpCommand : CS_ChatCommand
{
	override string GetName()
	{
		return "help";
	}

	override void Execute(PlayerBase player, array<string> args)
	{
		CS_Core core = CS_Core.Get();
		string prefix = core.Commands.GetPrefix();

		array<string> names;
		core.Commands.GetNames(names, core.IsAdmin(player));

		string list = "";
		foreach (string name : names)
		{
			if (list != "")
				list = list + ", ";
			list = list + prefix + name;
		}

		CS_Messenger.Send(player, "Commands: " + list);
		CS_Messenger.Send(player, string.Format("Type %1list to see what you can spawn and how many you have left.", prefix));
	}
}
