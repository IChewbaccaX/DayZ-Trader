modded class MissionGameplay
{
	override void OnInit()
	{
		super.OnInit();
		CT_ClientData.Reset();
		CT_ClientData.Get().OnOpenTrader.Insert(CT_OpenTraderMenu);
	}

	override void OnUpdate(float timeslice)
	{
		super.OnUpdate(timeslice);
		CT_ClientData.Get().RequestConfig(timeslice);
	}

	void CT_OpenTraderMenu(int traderId)
	{
		UIManager ui = GetGame().GetUIManager();
		CT_TraderMenu current = CT_TraderMenu.Cast(ui.GetMenu());
		if (current)
		{
			current.SetTrader(traderId);
			return;
		}

		if (ui.GetMenu())
			return;

		CT_TraderMenu menu = new CT_TraderMenu();
		ui.ShowScriptedMenu(menu, null);
		menu.SetTrader(traderId);
	}
}
