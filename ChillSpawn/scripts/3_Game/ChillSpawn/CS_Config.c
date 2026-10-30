// One spawnable thing: a vehicle or item plus its attachments, bound to a chat command.
class CS_SpawnPreset
{
	string Command;
	string DisplayName;
	string ClassName;
	ref array<string> Attachments;
	float DistanceMetres;
	float BearingDegrees;		// 0 = north, 90 = east
	int DespawnSeconds;			// 0 = never despawn
	int MaxPerPlaytimeHour;
	string SpawnMessage;		// %1 = DisplayName, %2 = minutes until despawn

	// Vehicles only: how full each fluid starts, 0 to 1.
	float FuelFraction;
	float CoolantFraction;
	float OilFraction;
	float BrakeFluidFraction;

	// Size of the clear space needed at the spawn point, in metres.
	float ClearanceWidth;
	float ClearanceLength;
	float ClearanceHeight;

	void CS_SpawnPreset()
	{
		Attachments = new array<string>;
		DistanceMetres = 6;
		BearingDegrees = 0;
		FuelFraction = 1;
		CoolantFraction = 1;
		OilFraction = 1;
		BrakeFluidFraction = 1;
		ClearanceWidth = 2.5;
		ClearanceLength = 5;
		ClearanceHeight = 2;
		DespawnSeconds = 900;
		MaxPerPlaytimeHour = 3;
		SpawnMessage = "Your %1 has spawned. It will despawn within %2 minutes.";
	}
}

class CS_Config
{
	static const string DIR = "$profile:ChillSpawn";
	static const string PATH = "$profile:ChillSpawn/config.json";

	int Version;
	string CommandPrefix;
	ref array<ref CS_SpawnPreset> Presets;

	void CS_Config()
	{
		Version = 1;
		CommandPrefix = "!";
		Presets = new array<ref CS_SpawnPreset>;
	}

	static CS_Config LoadOrCreate()
	{
		if (!FileExist(DIR))
			MakeDirectory(DIR);

		CS_Config cfg;
		string error;

		if (FileExist(PATH))
		{
			if (JsonFileLoader<CS_Config>.LoadFile(PATH, cfg, error))
				return cfg;

			// Keep the owner's broken file so they can fix it; run on defaults meanwhile.
			Print("[ChillSpawn] Could not read " + PATH + ": " + error + ". Using defaults.");
			return CreateDefault();
		}

		cfg = CreateDefault();
		if (!JsonFileLoader<CS_Config>.SaveFile(PATH, cfg, error))
			Print("[ChillSpawn] Could not write " + PATH + ": " + error);

		return cfg;
	}

	static CS_Config CreateDefault()
	{
		CS_Config cfg = new CS_Config();

		CS_SpawnPreset olga = new CS_SpawnPreset();
		olga.Command = "car";
		olga.DisplayName = "Olga 24";
		olga.ClassName = "CivilianSedan_Wine";
		olga.Attachments.Insert("CivSedanWheel");
		olga.Attachments.Insert("CivSedanWheel");
		olga.Attachments.Insert("CivSedanWheel");
		olga.Attachments.Insert("CivSedanWheel");
		olga.Attachments.Insert("SparkPlug");
		olga.Attachments.Insert("CarRadiator");
		olga.Attachments.Insert("CarBattery");
		olga.Attachments.Insert("HeadlightH7");
		olga.Attachments.Insert("HeadlightH7");
		olga.Attachments.Insert("CivSedanDoors_Driver_Wine");
		olga.Attachments.Insert("CivSedanDoors_CoDriver_Wine");
		olga.Attachments.Insert("CivSedanDoors_BackLeft_Wine");
		olga.Attachments.Insert("CivSedanDoors_BackRight_Wine");
		olga.Attachments.Insert("CivSedanHood_Wine");
		olga.Attachments.Insert("CivSedanTrunk_Wine");
		cfg.Presets.Insert(olga);

		CS_SpawnPreset ada = new CS_SpawnPreset();
		ada.Command = "ada";
		ada.DisplayName = "Ada 4x4";
		ada.ClassName = "OffroadHatchback";
		ada.Attachments.Insert("HatchbackWheel");
		ada.Attachments.Insert("HatchbackWheel");
		ada.Attachments.Insert("HatchbackWheel");
		ada.Attachments.Insert("HatchbackWheel");
		ada.Attachments.Insert("SparkPlug");
		ada.Attachments.Insert("CarRadiator");
		ada.Attachments.Insert("CarBattery");
		ada.Attachments.Insert("HeadlightH7");
		ada.Attachments.Insert("HeadlightH7");
		ada.Attachments.Insert("HatchbackDoors_Driver");
		ada.Attachments.Insert("HatchbackDoors_CoDriver");
		ada.Attachments.Insert("HatchbackHood");
		ada.Attachments.Insert("HatchbackTrunk");
		cfg.Presets.Insert(ada);

		// Items work the same way: no attachments or fluids, a small clear space, close by.
		CS_SpawnPreset hatchet = new CS_SpawnPreset();
		hatchet.Command = "hatchet";
		hatchet.DisplayName = "hatchet";
		hatchet.ClassName = "Hatchet";
		hatchet.DistanceMetres = 1;
		hatchet.ClearanceWidth = 0.5;
		hatchet.ClearanceLength = 0.5;
		hatchet.ClearanceHeight = 0.3;
		cfg.Presets.Insert(hatchet);

		return cfg;
	}
}
