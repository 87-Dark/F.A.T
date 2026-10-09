//------------------------------------------------------------------------------------------------
// FAT_mod scipt
// Licence APL_ND
//------------------------------------------------------------------------------------------------

modded class SCR_DoorUserAction
{
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		DL_DoorLockManagerComponent manager = DL_DoorLockManagerComponent.GetInstance();
		if (manager && manager.IsLocked(GetOwner()))
		{
			SetCannotPerformReason("Verrouillée");
			return false;
		}

		return super.CanBePerformedScript(user);
	}
}
