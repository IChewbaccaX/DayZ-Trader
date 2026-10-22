//! Clientseitiger Zustand: synchronisierter Markt, Kontostaende und Events fuer die UI
class CT_ClientData
{
	protected static ref CT_ClientData s_Instance;

	ref CT_Market Market;
	int Cash;
	int Bank;
	float RequestCooldown;

	//! (int traderId)
	ref ScriptInvoker OnOpenTrader = new ScriptInvoker();
	//! (bool success, string message)
	ref ScriptInvoker OnTradeResult = new ScriptInvoker();

	static CT_ClientData Get()
	{
		if (!s_Instance)
			s_Instance = new CT_ClientData();
		return s_Instance;
	}

	static void Reset()
	{
		s_Instance = null;
	}

	bool HasConfig()
	{
		return Market != null;
	}

	void SetConfig(string json)
	{
		CT_SyncData data = new CT_SyncData();
		string error;
		JsonSerializer serializer = new JsonSerializer();
		if (!serializer.ReadFromString(data, json, error))
		{
			Print("[CoreTrader] Konfiguration vom Server ungueltig: " + error);
			return;
		}
		Market = new CT_Market(data);
	}

	void RequestConfig(float timeslice)
	{
		if (HasConfig())
			return;

		RequestCooldown -= timeslice;
		if (RequestCooldown > 0)
			return;
		RequestCooldown = 5.0;

		PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
		if (!player || !player.IsAlive())
			return;

		ScriptRPC rpc = new ScriptRPC();
		rpc.Send(player, CT_RPC.REQUEST_CONFIG, true, null);
	}

	void HandleRPC(int rpc_type, ParamsReadContext ctx)
	{
		switch (rpc_type)
		{
			case CT_RPC.SYNC_CONFIG:
			{
				string json;
				if (ctx.Read(json))
					SetConfig(json);
				break;
			}
			case CT_RPC.OPEN_TRADER:
			{
				int traderId;
				if (!ctx.Read(traderId) || !ctx.Read(Cash) || !ctx.Read(Bank))
					return;
				if (HasConfig())
					OnOpenTrader.Invoke(traderId);
				break;
			}
			case CT_RPC.RESULT:
			{
				bool success;
				string message;
				if (!ctx.Read(success) || !ctx.Read(message) || !ctx.Read(Cash) || !ctx.Read(Bank))
					return;
				OnTradeResult.Invoke(success, message);
				break;
			}
		}
	}

	// ---- Anfragen an den Server ----

	protected void Send(ScriptRPC rpc, int type)
	{
		PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
		if (player)
			rpc.Send(player, type, true, null);
	}

	void Buy(int traderId, string className, int amount, bool withAttachments, bool useBank)
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(traderId);
		rpc.Write(className);
		rpc.Write(amount);
		rpc.Write(withAttachments);
		rpc.Write(useBank);
		Send(rpc, CT_RPC.BUY);
	}

	void Sell(int traderId, EntityAI item, bool toBank)
	{
		int low, high;
		item.GetNetworkID(low, high);

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(traderId);
		rpc.Write(low);
		rpc.Write(high);
		rpc.Write(toBank);
		Send(rpc, CT_RPC.SELL);
	}

	void SellAll(int traderId, bool toBank)
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(traderId);
		rpc.Write(toBank);
		Send(rpc, CT_RPC.SELL_ALL);
	}

	void BankTransfer(int traderId, int amount, bool deposit)
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(traderId);
		rpc.Write(amount);
		if (deposit)
			Send(rpc, CT_RPC.BANK_DEPOSIT);
		else
			Send(rpc, CT_RPC.BANK_WITHDRAW);
	}
}
