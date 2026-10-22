modded class PlayerBase
{
	bool m_CT_InSafeZone;
	int m_CT_LastTradeTime;

	override void SetActions(out TInputActionMap InputActionMap)
	{
		super.SetActions(InputActionMap);
		AddAction(ActionCTOpenTrader, InputActionMap);
	}

	override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		super.OnRPC(sender, rpc_type, ctx);

		if (rpc_type < CT_RPC.REQUEST_CONFIG || rpc_type > CT_RPC.RESULT)
			return;

		if (GetGame().IsDedicatedServer())
		{
			// nur der Besitzer darf RPCs ueber sein eigenes Spielerobjekt senden
			if (!sender || !GetIdentity() || sender.GetId() != GetIdentity().GetId())
				return;

			if (CT_TraderServer.IsRunning())
				CT_TraderServer.Get().HandleRPC(this, rpc_type, ctx);
		}
		else
		{
			CT_ClientData.Get().HandleRPC(rpc_type, ctx);
		}
	}
}
