modded class MissionServer
{
	override void OnInit()
	{
		super.OnInit();
		CT_TraderServer.Get().Init();
	}
}
