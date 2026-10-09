//------------------------------------------------------------------------------------------------
// FAT_mod scipt
// Licence APL_ND
//------------------------------------------------------------------------------------------------

modded class SCR_PlayerController
{
	[RplProp()]
	protected bool m_bDL_DoorKeyHolder;

	//------------------------------------------------------------------------------------------------
	void DL_SetDoorKeyHolder(bool value)
	{
		if (m_bDL_DoorKeyHolder == value)
			return;

		m_bDL_DoorKeyHolder = value;
		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	bool DL_IsDoorKeyHolder()
	{
		return m_bDL_DoorKeyHolder;
	}

	//------------------------------------------------------------------------------------------------
	static bool DL_IsLocalKeyHolder()
	{
		SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!pc)
			return false;

		return pc.DL_IsDoorKeyHolder();
	}
}
