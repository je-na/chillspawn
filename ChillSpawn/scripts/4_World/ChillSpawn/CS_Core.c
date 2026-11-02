// Server-side singleton holding the mod's services. Other mods extend it with
// `modded class CS_Core` and override RegisterCommands.
class CS_Core
{
	private static ref CS_Core s_Instance;

	ref CS_Config Config;
	ref CS_PlayerTracker Tracker;
	ref CS_VehicleSpawner Spawner;
	ref CS_KitSpawner KitSpawner;
	ref CS_CommandRegistry Commands;

	static CS_Core Get()
	{
		if (!s_Instance)
		{
			s_Instance = new CS_Core();
			s_Instance.Init();
		}

		return s_Instance;
	}

	protected void Init()
	{
		Config = CS_Config.LoadOrCreate();
		Tracker = new CS_PlayerTracker();
		Spawner = new CS_VehicleSpawner();
		KitSpawner = new CS_KitSpawner();
		Commands = new CS_CommandRegistry(Config.CommandPrefix);
		RegisterCommands();
		Print(string.Format("[ChillSpawn] Loaded %1 spawn presets, %2 kits and %3 admins.", Config.Presets.Count(), Config.Kits.Count(), Config.Admins.Count()));
	}

	// Admins are listed in the config by Steam64 ID (or DayZ ID).
	bool IsAdmin(PlayerBase player)
	{
		if (!player || !player.GetIdentity() || !Config.Admins)
			return false;

		PlayerIdentity identity = player.GetIdentity();
		return Config.Admins.Find(identity.GetPlainId()) > -1 || Config.Admins.Find(identity.GetId()) > -1;
	}

	protected void RegisterCommands()
	{
		foreach (CS_SpawnPreset preset : Config.Presets)
			Commands.Register(new CS_SpawnPresetCommand(preset));

		foreach (CS_Kit kit : Config.Kits)
			Commands.Register(new CS_KitCommand(kit));

		// Registered last so a preset or kit called "list" or "help" can't replace them.
		Commands.Register(new CS_ListCommand());
		Commands.Register(new CS_HelpCommand());
	}
}
