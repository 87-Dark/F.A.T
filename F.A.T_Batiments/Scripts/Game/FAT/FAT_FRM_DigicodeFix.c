//! Correctif digicode FRM (IntelAMPDoorLock-FRM) en serveur dedie.

modded class SCR_PlayerController
{
	void FAT_SubmitTerminalPassword(IEntity terminalEntity, string password)
	{
		if (!terminalEntity)
			return;

		RplComponent termRpl = RplComponent.Cast(terminalEntity.FindComponent(RplComponent));
		if (!termRpl)
			return;

		Rpc(FAT_RpcAsk_SubmitTerminalPassword, termRpl.Id(), password);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void FAT_RpcAsk_SubmitTerminalPassword(RplId terminalRplId, string password)
	{
		RplComponent termRpl = RplComponent.Cast(Replication.FindItem(terminalRplId));
		if (!termRpl)
			return;

		IEntity terminalEntity = termRpl.GetEntity();
		if (!terminalEntity)
			return;

		FRM_CodeTerminalComponent term = FRM_CodeTerminalComponent.Cast(terminalEntity.FindComponent(FRM_CodeTerminalComponent));
		if (!term)
			return;

		Print("[FAT][Digicode] code recu par le serveur", LogLevel.NORMAL);
		term.ServerHandlePassword(password, GetControlledEntity());
	}
}

modded class FRM_CodeTerminalComponent
{

	override void OnConfirmPassword_Local(string password)
	{
		if (Replication.IsServer())
		{
			super.OnConfirmPassword_Local(password);
			return;
		}

		SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (pc)
			pc.FAT_SubmitTerminalPassword(GetOwner(), password);
	}

	override protected BaseDoorComponent FindDoorNearTerminal(IEntity terminalEnt)
	{
		if (terminalEnt)
		{
			IEntity root = terminalEnt.GetRootParent();
			if (!root)
				root = terminalEnt;

			BaseDoorComponent door = FAT_FindAccessDoor(root);
			if (door)
				return door;
		}

		return super.FindDoorNearTerminal(terminalEnt);
	}

	protected BaseDoorComponent FAT_FindAccessDoor(IEntity start)
	{
		if (!start)
			return null;

		if (start.FindComponent(FRM_DoorAccessComponent))
		{
			BaseDoorComponent door = BaseDoorComponent.Cast(start.FindComponent(BaseDoorComponent));
			if (door)
				return door;
		}

		IEntity child = start.GetChildren();
		while (child)
		{
			BaseDoorComponent found = FAT_FindAccessDoor(child);
			if (found)
				return found;

			child = child.GetSibling();
		}

		return null;
	}
}
