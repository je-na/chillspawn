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
	int MaxSpawns;				// per player, within LimitMinutes of playtime
	int LimitMinutes;
	bool AdminOnly;
	string SpawnMessage;		// %1 = DisplayName, %2 = minutes until despawn

	// Vehicles only: how full each fluid starts, 0 to 1.
	float FuelFraction;
	float CoolantFraction;
	float OilFraction;
	float BrakeFluidFraction;

	// Invulnerable: the spawned thing and its attachments take no damage at all.
	// ProtectCrew (vehicles): people inside take no damage when it crashes.
	// Both last until the vehicle despawns or the server restarts.
	bool Invulnerable;
	bool ProtectCrew;

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
		MaxSpawns = 3;
		LimitMinutes = 60;
		SpawnMessage = "Your %1 has spawned. It will despawn within %2 minutes.";
	}
}

// One item in a kit. Contents go into it: attached where they fit (magazines,
// suppressors, batteries, holsters), otherwise into its cargo.
class CS_KitItem
{
	string ClassName;
	int Count;
	ref array<ref CS_KitItem> Contents;

	void CS_KitItem()
	{
		Count = 1;
		Contents = new array<ref CS_KitItem>;
	}
}

// A set of items bound to a chat command. Magazines and ammo spawn full,
// batteries charged, bottles full of water, everything pristine.
class CS_Kit
{
	string Command;
	string DisplayName;
	int MaxSpawns;
	int LimitMinutes;
	bool RealTimeLimit;		// count LimitMinutes in real-world time instead of playtime
	bool AdminOnly;
	bool ToInventory;		// put items on the player where there's room; the rest go on the ground
	ref array<ref CS_KitItem> Items;

	void CS_Kit()
	{
		MaxSpawns = 1;
		LimitMinutes = 60;
		Items = new array<ref CS_KitItem>;
	}
}

class CS_Config
{
	static const string DIR = "$profile:ChillSpawn";
	static const string PATH = "$profile:ChillSpawn/config.json";
	static const int CURRENT_VERSION = 2;

	int Version;
	string CommandPrefix;
	ref array<string> Admins;	// Steam64 IDs; admins have no limits and can use admin-only commands
	ref array<ref CS_SpawnPreset> Presets;
	ref array<ref CS_Kit> Kits;

	void CS_Config()
	{
		Version = CURRENT_VERSION;
		CommandPrefix = "!";
		Admins = new array<string>;
		Presets = new array<ref CS_SpawnPreset>;
		Kits = new array<ref CS_Kit>;
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
			{
				if (cfg.Version < CURRENT_VERSION)
					Print("[ChillSpawn] " + PATH + " is from an older version. Missing fields use defaults; delete the file to get the new default presets.");

				return cfg;
			}

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
		olga.MaxSpawns = 1;
		olga.LimitMinutes = 15;
		olga.Invulnerable = true;
		olga.ProtectCrew = true;
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
		ada.MaxSpawns = 1;
		ada.LimitMinutes = 15;
		ada.Invulnerable = true;
		ada.ProtectCrew = true;
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

		cfg.Kits.Insert(CreateSurvivalKit());
		cfg.Kits.Insert(CreateAdminKit());

		return cfg;
	}

	static CS_Kit CreateSurvivalKit()
	{
		CS_Kit kit = new CS_Kit();
		kit.Command = "survive";
		kit.DisplayName = "survival kit";
		kit.MaxSpawns = 1;
		kit.LimitMinutes = 120;
		kit.RealTimeLimit = true;
		kit.ToInventory = true;

		AddItem(kit.Items, "TacticalBaconCan");
		AddItem(kit.Items, "WaterBottle");
		AddItem(kit.Items, "PurificationTablets");
		AddItem(kit.Items, "BandageDressing");
		AddItem(kit.Items, "TetracyclineAntibiotics");

		return kit;
	}

	static CS_Kit CreateAdminKit()
	{
		CS_Kit kit = new CS_Kit();
		kit.Command = "adminkit";
		kit.DisplayName = "admin kit";
		kit.AdminOnly = true;

		CS_KitItem bag = AddItem(kit.Items, "AliceBag_Green");
		AddItem(bag.Contents, "Mag_M14_20Rnd", 2);
		AddItem(bag.Contents, "AmmoBox_308Win_20Rnd", 3);
		AddItem(bag.Contents, "Mag_STANAG_60Rnd", 2);
		AddItem(bag.Contents, "M4_Suppressor");
		AddItem(bag.Contents, "Battery9V");
		AddItem(bag.Contents, "HuntingKnife");

		CS_KitItem m14 = AddItem(kit.Items, "M14");
		AddItem(m14.Contents, "Mag_M14_20Rnd");

		CS_KitItem m4 = AddItem(kit.Items, "M4A1");
		AddItem(m4.Contents, "M4_MPBttstck");
		AddItem(m4.Contents, "M4_RISHndgrd");
		AddItem(m4.Contents, "M4_CarryHandleOptic");
		AddItem(m4.Contents, "M4_Suppressor");
		AddItem(m4.Contents, "Mag_STANAG_60Rnd");

		CS_KitItem vest = AddItem(kit.Items, "PlateCarrierVest");
		AddItem(vest.Contents, "PlateCarrierHolster");
		AddItem(vest.Contents, "PlateCarrierPouches");

		CS_KitItem helmet = AddItem(kit.Items, "Mich2001Helmet");
		CS_KitItem nvg = AddItem(helmet.Contents, "NVGoggles");
		AddItem(nvg.Contents, "Battery9V");

		AddItem(kit.Items, "TTsKOJacket_Camo");
		AddItem(kit.Items, "TTSKOPants");
		AddItem(kit.Items, "TTSKOBoots");
		AddItem(kit.Items, "TacticalGloves_Green");

		return kit;
	}

	protected static CS_KitItem AddItem(array<ref CS_KitItem> list, string className, int count = 1)
	{
		CS_KitItem item = new CS_KitItem();
		item.ClassName = className;
		item.Count = count;
		list.Insert(item);
		return item;
	}
}
