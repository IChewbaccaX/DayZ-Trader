//! "Handeln" an NPC-Tradern und Verkaufsautomaten
class ActionCTOpenTrader : ActionInteractBase
{
	void ActionCTOpenTrader()
	{
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_INTERACTONCE;
		m_StanceMask = DayZPlayerConstants.STANCEMASK_ERECT | DayZPlayerConstants.STANCEMASK_CROUCH;
		m_Text = "Handeln";
	}

	override void CreateConditionComponents()
	{
		m_ConditionItem = new CCINone;
		m_ConditionTarget = new CCTObject(UAMaxDistances.DEFAULT);
	}

	override bool IsLockTargetOnUse()
	{
		return false;
	}

	static CT_Market GetActiveMarket()
	{
		if (GetGame().IsDedicatedServer())
		{
			if (CT_TraderServer.IsRunning())
				return CT_TraderServer.Get().Market;
			return null;
		}
		return CT_ClientData.Get().Market;
	}

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		Object obj = target.GetObject();
		if (!obj || obj == player)
			return false;

		CT_Market market = GetActiveMarket();
		if (!market)
			return false;

		return market.FindTraderForObject(obj) >= 0;
	}

	override void OnExecuteServer(ActionData action_data)
	{
		if (!CT_TraderServer.IsRunning())
			return;

		int traderId = CT_TraderServer.Get().Market.FindTraderForObject(action_data.m_Target.GetObject());
		if (traderId >= 0)
			CT_TraderServer.Get().OpenTrader(action_data.m_Player, traderId);
	}
}

modded class ActionConstructor
{
	override void RegisterActions(TTypenameArray actions)
	{
		super.RegisterActions(actions);
		actions.Insert(ActionCTOpenTrader);
	}
}
