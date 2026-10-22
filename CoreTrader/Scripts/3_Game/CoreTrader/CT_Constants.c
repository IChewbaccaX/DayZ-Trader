class CT_Const
{
	static const string PROFILE_DIR    = "$profile:CoreTrader/";
	static const string BANK_DIR       = "$profile:CoreTrader/Bank/";
	static const string SETTINGS_FILE  = "$profile:CoreTrader/Settings.json";
	static const string TRADERS_FILE   = "$profile:CoreTrader/Traders.json";
	static const string MARKET_FILE    = "$profile:CoreTrader/Market.json";

	static const string LAYOUT_MENU    = "CoreTrader/GUI/layouts/trader_menu.layout";

	static const int MAX_BUY_AMOUNT    = 100;
	static const int TRADE_COOLDOWN_MS = 250;
}

//! RPC-IDs; grosser Offset, damit sie nicht mit Vanilla/anderen Mods kollidieren
enum CT_RPC
{
	REQUEST_CONFIG = 7739100,
	SYNC_CONFIG,
	OPEN_TRADER,
	BUY,
	SELL,
	SELL_ALL,
	BANK_DEPOSIT,
	BANK_WITHDRAW,
	RESULT
}
