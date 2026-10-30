class CfgPatches
{
	class ChillSpawn
	{
		units[] = {};
		weapons[] = {};
		requiredVersion = 0.1;
		requiredAddons[] = {"DZ_Data", "DZ_Scripts"};
	};
};

class CfgMods
{
	class ChillSpawn
	{
		dir = "ChillSpawn";
		name = "Chill Spawn";
		author = "je-na";
		type = "mod";
		dependencies[] = {"Game", "World", "Mission"};

		class defs
		{
			class gameScriptModule
			{
				value = "";
				files[] = {"ChillSpawn/scripts/3_Game"};
			};
			class worldScriptModule
			{
				value = "";
				files[] = {"ChillSpawn/scripts/4_World"};
			};
			class missionScriptModule
			{
				value = "";
				files[] = {"ChillSpawn/scripts/5_Mission"};
			};
		};
	};
};
