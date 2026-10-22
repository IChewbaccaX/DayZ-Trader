//! Alle Klassen hier werden 1:1 als JSON gespeichert bzw. an Clients synchronisiert.

class CT_CurrencyEntry
{
	string ClassName;
	int Value;

	void CT_CurrencyEntry(string className = "", int value = 0)
	{
		ClassName = className;
		Value = value;
	}
}

class CT_SafeZone
{
	string Name;
	vector Position;
	float Radius;

	void CT_SafeZone(string name = "", vector position = "0 0 0", float radius = 100)
	{
		Name = name;
		Position = position;
		Radius = radius;
	}
}

class CT_Settings
{
	int ConfigVersion = 1;
	ref array<ref CT_CurrencyEntry> Currencies;
	bool EnableBanking = true;
	int BankStartBalance = 0;
	bool EnableSellAll = true;
	bool AllowSellRuined = false;
	float MaxTradeDistance = 5.0;
	bool EnableSafeZones = true;
	float SafeZoneCheckInterval = 2.0;
	ref array<ref CT_SafeZone> SafeZones;

	void CT_Settings()
	{
		Currencies = new array<ref CT_CurrencyEntry>;
		SafeZones = new array<ref CT_SafeZone>;
	}

	void SetDefaults()
	{
		Currencies.Insert(new CT_CurrencyEntry("CT_Dollar_100", 100));
		Currencies.Insert(new CT_CurrencyEntry("CT_Dollar_10", 10));
		Currencies.Insert(new CT_CurrencyEntry("CT_Dollar_1", 1));
		SafeZones.Insert(new CT_SafeZone("Green Mountain", "3706 402 5986", 150));
	}
}

//! Ein handelbarer Artikel. BuyPrice/SellPrice = -1 bedeutet "nicht kaufbar/verkaufbar".
class CT_ItemEntry
{
	string ClassName;
	int BuyPrice;
	int SellPrice;
	//! Farb-/Tarnvarianten mit gleichem Preis
	ref TStringArray Variants;
	//! Aufsaetze, die beim Kauf optional direkt mit angebaut werden
	ref TStringArray Attachments;

	void CT_ItemEntry(string className = "", int buy = -1, int sell = -1)
	{
		ClassName = className;
		BuyPrice = buy;
		SellPrice = sell;
		Variants = new TStringArray;
		Attachments = new TStringArray;
	}

	CT_ItemEntry AddVariants(TStringArray variants)
	{
		Variants.InsertAll(variants);
		return this;
	}

	CT_ItemEntry AddAttachments(TStringArray attachments)
	{
		Attachments.InsertAll(attachments);
		return this;
	}
}

class CT_Category
{
	string Name;
	ref array<ref CT_ItemEntry> Items;

	void CT_Category(string name = "")
	{
		Name = name;
		Items = new array<ref CT_ItemEntry>;
	}

	CT_ItemEntry Add(string className, int buy, int sell)
	{
		CT_ItemEntry entry = new CT_ItemEntry(className, buy, sell);
		Items.Insert(entry);
		return entry;
	}
}

class CT_TraderEntry
{
	string Name;
	//! "NPC" oder "VENDING"
	string Type;
	//! Objektklasse (z.B. SurvivorM_Mirek). Leer = jedes Objekt an dieser Position (z.B. Map-Automat)
	string ClassName;
	vector Position;
	vector Orientation;
	//! false = Objekt existiert bereits auf der Map und wird nicht gespawnt
	bool SpawnObject = true;
	ref TStringArray Loadout;
	ref TStringArray Categories;

	void CT_TraderEntry(string name = "", string type = "NPC", string className = "", vector position = "0 0 0", vector orientation = "0 0 0")
	{
		Name = name;
		Type = type;
		ClassName = className;
		Position = position;
		Orientation = orientation;
		Loadout = new TStringArray;
		Categories = new TStringArray;
	}

	bool IsVending()
	{
		string type = Type;
		type.ToUpper();
		return type == "VENDING";
	}
}

class CT_TradersFile
{
	ref array<ref CT_TraderEntry> Traders;

	void CT_TradersFile()
	{
		Traders = new array<ref CT_TraderEntry>;
	}

	void SetDefaults()
	{
		CT_TraderEntry weapons = new CT_TraderEntry("Waffenhaendler", "NPC", "SurvivorM_Mirek", "3700 402 5980", "90 0 0");
		weapons.Loadout.InsertAll({"TacticalShirt_Black", "CargoPants_Black", "MilitaryBoots_Black", "PlateCarrierVest"});
		weapons.Categories.InsertAll({"Waffen", "Munition", "Magazine", "Aufsaetze"});
		Traders.Insert(weapons);

		CT_TraderEntry general = new CT_TraderEntry("Haendler", "NPC", "SurvivorF_Eva", "3712 402 5990", "270 0 0");
		general.Loadout.InsertAll({"Hoodie_Grey", "Jeans_Blue", "AthleticShoes_Black"});
		general.Categories.InsertAll({"Kleidung", "Medizin", "Werkzeug", "Nahrung"});
		Traders.Insert(general);

		// Verkaufsautomat: beliebiges statisches Objekt; hier ein gespawnter Kuehlschrank als Beispiel
		CT_TraderEntry vending = new CT_TraderEntry("Getraenkeautomat", "VENDING", "Refrigerator", "3708 402 5975", "0 0 0");
		vending.Categories.InsertAll({"Nahrung"});
		Traders.Insert(vending);
	}
}

class CT_MarketFile
{
	ref array<ref CT_Category> Categories;

	void CT_MarketFile()
	{
		Categories = new array<ref CT_Category>;
	}

	void SetDefaults()
	{
		CT_Category cat;

		cat = new CT_Category("Waffen");
		cat.Add("AKM", 2500, 1000).AddAttachments({"AK_WoodBttstck", "AK_WoodHndgrd"});
		cat.Add("M4A1", 4000, 1600).AddAttachments({"M4_OEBttstck", "M4_PlasticHndgrd", "M4_CarryHandleOptic"});
		cat.Add("Mosin9130", 1200, 500).AddVariants({"Mosin9130_Black", "Mosin9130_Green", "Mosin9130_Camo"});
		cat.Add("SKS", 1100, 450);
		cat.Add("Glock19", 600, 250);
		Categories.Insert(cat);

		cat = new CT_Category("Munition");
		cat.Add("Ammo_762x39", 150, 60);
		cat.Add("Ammo_556x45", 180, 70);
		cat.Add("Ammo_762x54", 200, 80);
		cat.Add("Ammo_9x19", 60, 25);
		Categories.Insert(cat);

		cat = new CT_Category("Magazine");
		cat.Add("Mag_AKM_30Rnd", 250, 100);
		cat.Add("Mag_STANAG_30Rnd", 300, 120);
		cat.Add("Mag_Glock_15Rnd", 120, 50);
		Categories.Insert(cat);

		cat = new CT_Category("Aufsaetze");
		cat.Add("AK_WoodBttstck", 150, 60);
		cat.Add("AK_WoodHndgrd", 120, 50);
		cat.Add("M4_OEBttstck", 200, 80);
		cat.Add("M4_PlasticHndgrd", 150, 60);
		cat.Add("M4_CarryHandleOptic", 300, 120);
		cat.Add("PUScopeOptic", 400, 150);
		cat.Add("PSO1Optic", 600, 240);
		Categories.Insert(cat);

		cat = new CT_Category("Kleidung");
		cat.Add("TaloonBag_Blue", 400, 150).AddVariants({"TaloonBag_Orange", "TaloonBag_Violet", "TaloonBag_Green"});
		cat.Add("Hoodie_Grey", 150, 50).AddVariants({"Hoodie_Black", "Hoodie_Blue", "Hoodie_Green", "Hoodie_Red"});
		cat.Add("MilitaryBoots_Black", 300, 100).AddVariants({"MilitaryBoots_Beige", "MilitaryBoots_Bluerock", "MilitaryBoots_Brown", "MilitaryBoots_Redpunk"});
		cat.Add("PlateCarrierVest", 1500, 600);
		Categories.Insert(cat);

		cat = new CT_Category("Medizin");
		cat.Add("BandageDressing", 50, 20);
		cat.Add("Morphine", 200, 80);
		cat.Add("Epinephrine", 200, 80);
		cat.Add("SalineBagIV", 300, 120);
		Categories.Insert(cat);

		cat = new CT_Category("Werkzeug");
		cat.Add("Hatchet", 200, 80);
		cat.Add("Shovel", 250, 100);
		cat.Add("Pliers", 150, 60);
		cat.Add("Hammer", 100, 40);
		Categories.Insert(cat);

		cat = new CT_Category("Nahrung");
		cat.Add("BakedBeansCan", 40, 15);
		cat.Add("SpaghettiCan", 40, 15);
		cat.Add("SodaCan_Cola", 25, 10).AddVariants({"SodaCan_Pipsi", "SodaCan_Spite", "SodaCan_Kvass"});
		cat.Add("WaterBottle", 60, 20);
		Categories.Insert(cat);
	}
}

//! Was der Server an jeden Client sendet (als JSON-String)
class CT_SyncData
{
	ref array<ref CT_CurrencyEntry> Currencies;
	bool EnableBanking;
	bool EnableSellAll;
	ref array<ref CT_TraderEntry> Traders;
	ref array<ref CT_Category> Categories;

	void CT_SyncData()
	{
		Currencies = new array<ref CT_CurrencyEntry>;
		Traders = new array<ref CT_TraderEntry>;
		Categories = new array<ref CT_Category>;
	}
}
