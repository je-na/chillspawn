// Server-side singleton holding the mod's services. Other mods extend it with
// `modded class CS_Core` and override RegisterCommands.
class CS_Core
{
	private static ref CS_Core s_Instance;

	ref CS_Config Config;
	ref CS_PlayerTracker Tracker;
	ref CS_VehicleSpawner Spawner;
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
		Commands = new CS_CommandRegistry(Config.CommandPrefix);
		RegisterCommands();
		Print("[ChillSpawn] Loaded " + Config.Presets.Count() + " spawn presets.");
	}

	protected void RegisterCommands()
	{
		foreach (CS_SpawnPreset preset : Config.Presets)
			Commands.Register(new CS_SpawnPresetCommand(preset));

		// Registered after the presets so a preset called "list" or "help" can't replace them.
		Commands.Register(new CS_ListCommand());
		Commands.Register(new CS_HelpCommand());
	}
}
