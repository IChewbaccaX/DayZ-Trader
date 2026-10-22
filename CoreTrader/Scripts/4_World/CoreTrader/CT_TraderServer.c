//! Serverseitige Trader-Logik: Konfiguration, Trader-Spawns, Handel, Bank und Safezones
class CT_TraderServer
{
	protected static ref CT_TraderServer s_Instance;

	ref CT_Settings Settings;
	ref CT_Market Market;
	protected ref CT_BankProvider m_Bank;
	protected string m_SyncJson;
	protected ref array<Object> m_TraderObjects;

	static CT_TraderServer Get()
	{
		if (!s_Instance)
			s_Instance = new CT_TraderServer();
		return s_Instance;
	}

	static bool IsRunning()
	{
		return s_Instance != null && s_Instance.Market != null;
	}

	void Init()
	{
		LoadConfig();
		SpawnTraders();

		if (Settings.EnableSafeZones && Settings.SafeZones.Count() > 0)
		{
			int interval = Math.Max(Settings.SafeZoneCheckInterval, 0.5) * 1000;
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(CheckSafeZones, interval, true);
		}

		Print("[CoreTrader] Gestartet: " + Market.Data.Traders.Count() + " Trader, " + Market.Data.Categories.Count() + " Kategorien");
	}

	void SetBankProvider(CT_BankProvider provider)
	{
		m_Bank = provider;
	}

	// ------------------------------------------------------------------
	// Konfiguration
	// ------------------------------------------------------------------

	protected void LoadConfig()
	{
		if (!FileExist(CT_Const.PROFILE_DIR))
			MakeDirectory(CT_Const.PROFILE_DIR);

		Settings = new CT_Settings();
		if (FileExist(CT_Const.SETTINGS_FILE))
			JsonFileLoader<CT_Settings>.JsonLoadFile(CT_Const.SETTINGS_FILE, Settings);
		else
		{
			Settings.SetDefaults();
			JsonFileLoader<CT_Settings>.JsonSaveFile(CT_Const.SETTINGS_FILE, Settings);
		}

		CT_TradersFile traders = new CT_TradersFile();
		if (FileExist(CT_Const.TRADERS_FILE))
			JsonFileLoader<CT_TradersFile>.JsonLoadFile(CT_Const.TRADERS_FILE, traders);
		else
		{
			traders.SetDefaults();
			JsonFileLoader<CT_TradersFile>.JsonSaveFile(CT_Const.TRADERS_FILE, traders);
		}

		CT_MarketFile market = new CT_MarketFile();
		if (FileExist(CT_Const.MARKET_FILE))
			JsonFileLoader<CT_MarketFile>.JsonLoadFile(CT_Const.MARKET_FILE, market);
		else
		{
			market.SetDefaults();
			JsonFileLoader<CT_MarketFile>.JsonSaveFile(CT_Const.MARKET_FILE, market);
		}

		Sanitize(traders, market);

		CT_SyncData data = new CT_SyncData();
		data.Currencies = Settings.Currencies;
		data.EnableBanking = Settings.EnableBanking;
		data.EnableSellAll = Settings.EnableSellAll;
		data.Traders = traders.Traders;
		data.Categories = market.Categories;

		Market = new CT_Market(data);
		m_Bank = new CT_JsonBankProvider(Settings.BankStartBalance);

		JsonSerializer serializer = new JsonSerializer();
		serializer.WriteToString(data, false, m_SyncJson);
	}

	//! Fehlende Arrays aus handgeschriebenem JSON auffuellen und ungueltige Klassen melden
	protected void Sanitize(CT_TradersFile traders, CT_MarketFile market)
	{
		if (!Settings.Currencies)
			Settings.Currencies = new array<ref CT_CurrencyEntry>;
		if (!Settings.SafeZones)
			Settings.SafeZones = new array<ref CT_SafeZone>;
		if (!traders.Traders)
			traders.Traders = new array<ref CT_TraderEntry>;
		if (!market.Categories)
			market.Categories = new array<ref CT_Category>;

		if (Settings.Currencies.Count() == 0)
			Print("[CoreTrader] WARNUNG: Keine Waehrung in Settings.json konfiguriert!");

		foreach (CT_TraderEntry trader : traders.Traders)
		{
			if (!trader.Loadout)
				trader.Loadout = new TStringArray;
			if (!trader.Categories)
				trader.Categories = new TStringArray;
		}

		foreach (CT_Category category : market.Categories)
		{
			if (!category.Items)
				category.Items = new array<ref CT_ItemEntry>;

			foreach (CT_ItemEntry entry : category.Items)
			{
				if (!entry.Variants)
					entry.Variants = new TStringArray;
				if (!entry.Attachments)
					entry.Attachments = new TStringArray;

				if (!CT_Util.ClassExists(entry.ClassName))
					Print("[CoreTrader] WARNUNG: Unbekannte Klasse '" + entry.ClassName + "' in Kategorie " + category.Name);
			}
		}
	}

	// ------------------------------------------------------------------
	// Trader spawnen
	// ------------------------------------------------------------------

	protected void SpawnTraders()
	{
		m_TraderObjects = new array<Object>;

		foreach (CT_TraderEntry trader : Market.Data.Traders)
		{
			if (!trader.SpawnObject || trader.ClassName == "")
				continue;

			Object obj = GetGame().CreateObject(trader.ClassName, trader.Position, false, false, true);
			if (!obj)
			{
				Print("[CoreTrader] FEHLER: Trader '" + trader.Name + "' (" + trader.ClassName + ") konnte nicht gespawnt werden");
				continue;
			}

			obj.SetPosition(trader.Position);
			obj.SetOrientation(trader.Orientation);
			obj.SetAllowDamage(false);

			EntityAI entity = EntityAI.Cast(obj);
			if (entity)
			{
				foreach (string item : trader.Loadout)
					entity.GetInventory().CreateInInventory(item);
			}

			m_TraderObjects.Insert(obj);
		}
	}

	// ------------------------------------------------------------------
	// RPC
	// ------------------------------------------------------------------

	void HandleRPC(PlayerBase player, int rpc_type, ParamsReadContext ctx)
	{
		if (!player || !player.GetIdentity())
			return;

		if (rpc_type == CT_RPC.REQUEST_CONFIG)
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write(m_SyncJson);
			rpc.Send(player, CT_RPC.SYNC_CONFIG, true, player.GetIdentity());
			return;
		}

		// einfache Flood-Sperre fuer alle Handels-RPCs
		int now = GetGame().GetTime();
		if (now - player.m_CT_LastTradeTime < CT_Const.TRADE_COOLDOWN_MS)
			return;
		player.m_CT_LastTradeTime = now;

		int traderId;
		if (!ctx.Read(traderId))
			return;

		switch (rpc_type)
		{
			case CT_RPC.BUY:
			{
				string className;
				int amount;
				bool withAttachments, useBank;
				if (ctx.Read(className) && ctx.Read(amount) && ctx.Read(withAttachments) && ctx.Read(useBank))
					HandleBuy(player, traderId, className, amount, withAttachments, useBank);
				break;
			}
			case CT_RPC.SELL:
			{
				int low, high;
				bool toBankSell;
				if (ctx.Read(low) && ctx.Read(high) && ctx.Read(toBankSell))
					HandleSell(player, traderId, EntityAI.Cast(GetGame().GetObjectByNetworkId(low, high)), toBankSell);
				break;
			}
			case CT_RPC.SELL_ALL:
			{
				bool toBankAll;
				if (ctx.Read(toBankAll))
					HandleSellAll(player, traderId, toBankAll);
				break;
			}
			case CT_RPC.BANK_DEPOSIT:
			{
				int depositAmount;
				if (ctx.Read(depositAmount))
					HandleBank(player, traderId, depositAmount, true);
				break;
			}
			case CT_RPC.BANK_WITHDRAW:
			{
				int withdrawAmount;
				if (ctx.Read(withdrawAmount))
					HandleBank(player, traderId, withdrawAmount, false);
				break;
			}
		}
	}

	//! Wird von der Aktion aufgerufen, wenn der Spieler mit einem Trader interagiert
	void OpenTrader(PlayerBase player, int traderId)
	{
		if (!CheckTrader(player, traderId))
			return;

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(traderId);
		rpc.Write(CT_Money.GetCash(player, Market));
		rpc.Write(GetBankBalance(player));
		rpc.Send(player, CT_RPC.OPEN_TRADER, true, player.GetIdentity());
	}

	protected void SendResult(PlayerBase player, bool success, string message)
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(success);
		rpc.Write(message);
		rpc.Write(CT_Money.GetCash(player, Market));
		rpc.Write(GetBankBalance(player));
		rpc.Send(player, CT_RPC.RESULT, true, player.GetIdentity());
	}

	protected CT_TraderEntry CheckTrader(PlayerBase player, int traderId)
	{
		CT_TraderEntry trader = Market.GetTrader(traderId);
		if (!trader || !player.IsAlive() || player.IsUnconscious() || player.IsRestrained())
			return null;

		if (CT_Util.Distance2D(player.GetPosition(), trader.Position) > Settings.MaxTradeDistance)
			return null;

		return trader;
	}

	// ------------------------------------------------------------------
	// Bezahlen
	// ------------------------------------------------------------------

	protected bool BankEnabled()
	{
		return Settings.EnableBanking && m_Bank;
	}

	protected int GetBankBalance(PlayerBase player)
	{
		if (!BankEnabled())
			return 0;
		return m_Bank.GetBalance(player);
	}

	protected bool Pay(PlayerBase player, int amount, bool useBank)
	{
		if (amount <= 0)
			return true;

		if (useBank && BankEnabled())
			return m_Bank.Withdraw(player, amount);

		if (CT_Money.GetCash(player, Market) < amount)
			return false;

		CT_Money.RemoveCash(player, Market, amount);
		return true;
	}

	protected void Payout(PlayerBase player, int amount, bool toBank)
	{
		if (amount <= 0)
			return;

		if (toBank && BankEnabled() && m_Bank.Deposit(player, amount))
			return;

		CT_Money.AddCash(player, Market, amount);
	}

	// ------------------------------------------------------------------
	// Kaufen / Verkaufen
	// ------------------------------------------------------------------

	protected void HandleBuy(PlayerBase player, int traderId, string className, int amount, bool withAttachments, bool useBank)
	{
		if (!CheckTrader(player, traderId))
			return;

		amount = Math.Clamp(amount, 1, CT_Const.MAX_BUY_AMOUNT);

		CT_ItemEntry entry = Market.FindEntryForTrader(traderId, className);
		if (!entry || entry.BuyPrice < 0 || !CT_Util.ClassExists(className))
		{
			SendResult(player, false, "Dieser Artikel kann hier nicht gekauft werden.");
			return;
		}

		int unitPrice = entry.BuyPrice;
		TStringArray attachments = new TStringArray;
		if (withAttachments)
		{
			foreach (string attachment : entry.Attachments)
			{
				CT_ItemEntry attEntry = Market.FindEntry(attachment);
				if (attEntry && attEntry.BuyPrice >= 0 && CT_Util.ClassExists(attachment))
				{
					attachments.Insert(attachment);
					unitPrice += attEntry.BuyPrice;
				}
			}
		}

		int total = unitPrice * amount;
		if (!Pay(player, total, useBank))
		{
			SendResult(player, false, "Nicht genug Geld. Benoetigt: " + CT_Util.FormatMoney(total));
			return;
		}

		for (int i = 0; i < amount; i++)
		{
			EntityAI item = CT_Money.CreateForPlayer(player, className);
			if (!item)
				continue;

			PrepareBoughtItem(item);

			foreach (string att : attachments)
			{
				EntityAI attached = item.GetInventory().CreateAttachment(att);
				if (attached)
					PrepareBoughtItem(attached);
				else
					CT_Money.CreateForPlayer(player, att);
			}
		}

		string name = CT_Util.GetDisplayName(className);
		if (amount > 1)
			name = amount.ToString() + "x " + name;
		SendResult(player, true, name + " gekauft fuer " + CT_Util.FormatMoney(total));
	}

	//! Munition voll, Magazine leer, Mengen-Items (z.B. Verbaende) voll
	protected void PrepareBoughtItem(EntityAI entity)
	{
		Magazine mag = Magazine.Cast(entity);
		if (mag)
		{
			if (entity.IsInherited(Ammunition_Base))
				mag.ServerSetAmmoCount(mag.GetAmmoMax());
			else
				mag.ServerSetAmmoCount(0);
			return;
		}

		ItemBase item = ItemBase.Cast(entity);
		if (item && item.HasQuantity() && !item.IsLiquidContainer())
			item.SetQuantityMax();
	}

	protected void HandleSell(PlayerBase player, int traderId, EntityAI item, bool toBank)
	{
		if (!CheckTrader(player, traderId))
			return;

		if (!item || item.GetHierarchyRootPlayer() != player || item == player || Market.IsCurrency(item.GetType()))
		{
			SendResult(player, false, "Dieser Gegenstand kann nicht verkauft werden.");
			return;
		}

		string reason;
		int value = CT_ItemValue.GetSellValue(item, Market, traderId, Settings.AllowSellRuined, reason);
		if (value < 0)
		{
			SendResult(player, false, reason);
			return;
		}

		string name = item.GetDisplayName();
		GetGame().ObjectDelete(item);
		Payout(player, value, toBank);
		SendResult(player, true, name + " verkauft fuer " + CT_Util.FormatMoney(value));
	}

	protected void HandleSellAll(PlayerBase player, int traderId, bool toBank)
	{
		if (!Settings.EnableSellAll || !CheckTrader(player, traderId))
			return;

		array<EntityAI> candidates = new array<EntityAI>;
		CT_ItemValue.GetSellCandidates(player, Market, false, candidates);

		int total = 0;
		int count = 0;
		string reason;
		foreach (EntityAI item : candidates)
		{
			if (!item)
				continue;

			int value = CT_ItemValue.GetSellValue(item, Market, traderId, Settings.AllowSellRuined, reason);
			if (value < 0)
				continue;

			GetGame().ObjectDelete(item);
			total += value;
			count++;
		}

		if (count == 0)
		{
			SendResult(player, false, "Du hast nichts, was dieser Haendler ankauft.");
			return;
		}

		Payout(player, total, toBank);
		SendResult(player, true, count.ToString() + " Gegenstaende verkauft fuer " + CT_Util.FormatMoney(total));
	}

	// ------------------------------------------------------------------
	// Bank
	// ------------------------------------------------------------------

	protected void HandleBank(PlayerBase player, int traderId, int amount, bool deposit)
	{
		if (!BankEnabled() || !CheckTrader(player, traderId))
			return;

		if (amount <= 0)
		{
			SendResult(player, false, "Ungueltiger Betrag.");
			return;
		}

		if (deposit)
		{
			if (CT_Money.GetCash(player, Market) < amount)
			{
				SendResult(player, false, "Nicht genug Bargeld.");
				return;
			}
			CT_Money.RemoveCash(player, Market, amount);
			m_Bank.Deposit(player, amount);
			SendResult(player, true, CT_Util.FormatMoney(amount) + " eingezahlt.");
		}
		else
		{
			if (!m_Bank.Withdraw(player, amount))
			{
				SendResult(player, false, "Nicht genug Guthaben auf der Bank.");
				return;
			}
			CT_Money.AddCash(player, Market, amount);
			SendResult(player, true, CT_Util.FormatMoney(amount) + " abgehoben.");
		}
	}

	// ------------------------------------------------------------------
	// Safezones
	// ------------------------------------------------------------------

	protected void CheckSafeZones()
	{
		array<Man> players = new array<Man>;
		GetGame().GetPlayers(players);

		foreach (Man man : players)
		{
			PlayerBase player = PlayerBase.Cast(man);
			if (!player || !player.GetIdentity() || !player.IsAlive())
				continue;

			CT_SafeZone zone = FindSafeZone(player.GetPosition());
			bool inside = zone != null;
			if (inside == player.m_CT_InSafeZone)
				continue;

			player.m_CT_InSafeZone = inside;
			player.SetAllowDamage(!inside);

			if (inside)
				NotificationSystem.SendNotificationToPlayerExtended(player, 5, "Safezone", "Du betrittst " + zone.Name + ". Du bist hier unverwundbar.");
			else
				NotificationSystem.SendNotificationToPlayerExtended(player, 5, "Safezone", "Du hast die Safezone verlassen.");
		}
	}

	CT_SafeZone FindSafeZone(vector position)
	{
		foreach (CT_SafeZone zone : Settings.SafeZones)
		{
			if (CT_Util.Distance2D(position, zone.Position) <= zone.Radius)
				return zone;
		}
		return null;
	}
}
