// Base class for chat commands. To add one, subclass this and register it in
// CS_Core.RegisterCommands (see README).
class CS_ChatCommand
{
	string GetName()
	{
		return "";
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

	void GetNames(out array<string> names)
	{
		names = new array<string>;
		foreach (string name, CS_ChatCommand command : m_Commands)
			names.Insert(name);

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

		command.Execute(player, args);
		return true;
	}
}

// Spawns a configured preset, enforcing its playtime-based rate limit.
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

	override void Execute(PlayerBase player, array<string> args)
	{
		if (!player.IsAlive() || !player.GetIdentity())
			return;

		if (player.IsInVehicle())
		{
			CS_Messenger.Send(player, "Get out of your vehicle first.");
			return;
		}

		CS_Core core = CS_Core.Get();
		string id = player.GetIdentity().GetId();

		float waitSeconds;
		int used = core.Tracker.CountRecentSpawns(id, m_Preset.Command, waitSeconds);
		if (used >= m_Preset.MaxPerPlaytimeHour)
		{
			int waitMinutes = Math.Ceil(waitSeconds / 60.0);
			CS_Messenger.Send(player, string.Format("You have used all %1 spawns for this hour of play. Try again in %2 minutes of play.", m_Preset.MaxPerPlaytimeHour, waitMinutes));
			return;
		}

		string error;
		EntityAI ent = core.Spawner.Spawn(player, m_Preset, error);
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

		core.Tracker.RecordSpawn(id, m_Preset.Command);

		int despawnMinutes = Math.Ceil(m_Preset.DespawnSeconds / 60.0);
		CS_Messenger.Send(player, string.Format(m_Preset.SpawnMessage, m_Preset.DisplayName, despawnMinutes));
	}
}

// Lists every spawnable preset with how many spawns the player has left this
// hour of play. One line per preset so long lists don't get cut off in chat.
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
		string prefix = core.Commands.GetPrefix();
		string id = player.GetIdentity().GetId();

		if (core.Config.Presets.Count() == 0)
		{
			CS_Messenger.Send(player, "Nothing is set up to spawn. Ask a server admin.");
			return;
		}

		CS_Messenger.Send(player, "You can spawn:");
		foreach (CS_SpawnPreset preset : core.Config.Presets)
		{
			float waitSeconds;
			int used = core.Tracker.CountRecentSpawns(id, preset.Command, waitSeconds);
			int left = preset.MaxPerPlaytimeHour - used;
			if (left < 0)
				left = 0;

			string line = string.Format("%1%2 - %3 (%4 of %5 left this hour)", prefix, preset.Command, preset.DisplayName, left, preset.MaxPerPlaytimeHour);
			if (left == 0)
			{
				int waitMinutes = Math.Ceil(waitSeconds / 60.0);
				line = line + string.Format(", next in %1 min", waitMinutes);
			}

			CS_Messenger.Send(player, line);
		}
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
		core.Commands.GetNames(names);

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
