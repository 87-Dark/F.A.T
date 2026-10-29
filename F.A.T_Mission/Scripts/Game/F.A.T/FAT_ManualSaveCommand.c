[BaseContainerProps(configRoot: true)]
modded class SCR_ChatPanelManager
{
	override protected void InitDefaultChatCommands()
	{
		super.InitDefaultChatCommands();

		ChatCommandInvoker invoker = GetCommandInvoker("sauvegarde");
		if (invoker)
			invoker.Insert(FAT_OnSaveCommand);
	}

	protected void FAT_OnSaveCommand(SCR_ChatPanel panel, string data)
	{
		SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!pc)
			return;

		if (!SCR_Global.IsAdmin())
		{
			ShowHelpMessage("Sauvegarde : reserve aux admins (#login).");
			return;
		}

		pc.FAT_RequestManualSave();
		ShowHelpMessage("Sauvegarde demandee au serveur.");
	}
}

modded class SCR_PlayerController
{
	void FAT_RequestManualSave()
	{
		Rpc(FAT_RpcAsk_ManualSave);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void FAT_RpcAsk_ManualSave()
	{
		int playerId = GetPlayerId();
		if (!SCR_Global.IsAdmin(playerId))
		{
			Print(string.Format("[FAT][Save] refus : joueur %1 non admin", playerId), LogLevel.WARNING);
			return;
		}

		SaveGameManager manager = GetGame().GetSaveGameManager();
		if (!manager || !manager.IsSavingAllowed())
		{
			Print("[FAT][Save] sauvegarde non autorisee pour cette mission", LogLevel.WARNING);
			return;
		}

		Print(string.Format("[FAT][Save] sauvegarde manuelle demandee par le joueur %1", playerId), LogLevel.NORMAL);
		manager.RequestSavePoint(ESaveGameType.MANUAL, "Sauvegarde manuelle F.A.T");
	}
}
