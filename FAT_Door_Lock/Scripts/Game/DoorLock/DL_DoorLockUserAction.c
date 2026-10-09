//------------------------------------------------------------------------------------------------
// FAT_mod scipt
// Licence APL_ND
//------------------------------------------------------------------------------------------------

class DL_DoorLockUserAction : ScriptedUserAction
{
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		return SCR_PlayerController.DL_IsLocalKeyHolder();
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		DL_DoorLockManagerComponent manager = DL_DoorLockManagerComponent.GetInstance();
		if (!manager)
		{
			SetCannotPerformReason("Système de verrou absent");
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		DL_DoorLockManagerComponent manager = DL_DoorLockManagerComponent.GetInstance();
		if (manager && manager.IsLocked(GetOwner()))
			outName = "Déverrouiller la porte";
		else
			outName = "Verrouiller la porte";

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!Replication.IsServer())
			return;

		DL_DoorLockManagerComponent manager = DL_DoorLockManagerComponent.GetInstance();
		if (!manager)
			return;

		manager.RequestToggle(pOwnerEntity, pUserEntity);

		if (manager.IsLocked(pOwnerEntity))
		{
			DoorComponent door = DoorComponent.Cast(pOwnerEntity.FindComponent(DoorComponent));
			if (door && door.GetControlValue() != 0)
			{ RplComponent rpl = RplComponent.Cast(pUserEntity.FindComponent(RplComponent)); if (rpl) door.SetControlValue(0, rpl.Id()); }
		}
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBroadcastScript()
	{
		return false;
	}
}
