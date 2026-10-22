class CfgPatches
{
	class CoreTrader
	{
		units[] = {"CT_Dollar_1", "CT_Dollar_10", "CT_Dollar_100"};
		weapons[] = {};
		requiredVersion = 0.1;
		requiredAddons[] = {"DZ_Data", "DZ_Scripts", "DZ_Characters", "DZ_Gear_Consumables"};
	};
};

class CfgMods
{
	class CoreTrader
	{
		dir = "CoreTrader";
		name = "Core Trader";
		author = "CoreTrader";
		type = "mod";
		dependencies[] = {"Game", "World", "Mission"};

		class defs
		{
			class gameScriptModule
			{
				value = "";
				files[] = {"CoreTrader/Scripts/3_Game"};
			};
			class worldScriptModule
			{
				value = "";
				files[] = {"CoreTrader/Scripts/4_World"};
			};
			class missionScriptModule
			{
				value = "";
				files[] = {"CoreTrader/Scripts/5_Mission"};
			};
		};
	};
};

class CfgVehicles
{
	class Inventory_Base;

	// Standard-Waehrung. Kann in Settings.json durch beliebige andere Items ersetzt werden.
	class CT_Dollar_Base: Inventory_Base
	{
		scope = 0;
		displayName = "Dollar";
		descriptionShort = "Waehrung fuer den Core Trader.";
		model = "\dz\gear\consumables\Paper.p3d";
		weight = 0;
		weightPerQuantity = 1;
		itemSize[] = {1, 1};
		canBeSplit = 1;
		varQuantityInit = 1;
		varQuantityMin = 0;
		varQuantityMax = 1000;
		varQuantityDestroyOnMin = 1;
		destroyOnEmpty = 1;
		quantityBar = 0;
	};
	class CT_Dollar_1: CT_Dollar_Base
	{
		scope = 2;
		displayName = "1 Dollar";
	};
	class CT_Dollar_10: CT_Dollar_Base
	{
		scope = 2;
		displayName = "10 Dollar";
	};
	class CT_Dollar_100: CT_Dollar_Base
	{
		scope = 2;
		displayName = "100 Dollar";
	};
};
