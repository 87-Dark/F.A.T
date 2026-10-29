
modded class SCR_CharacterCameraHandlerComponent
{
	protected FAT_DriverCameraComponent m_FatCfg;
	protected bool m_bFatCfgSearched;
	protected CompartmentAccessComponent m_FatCompAccess;

	protected bool FAT_ShouldForceFirstPerson()
	{
		if (!m_bFatCfgSearched)
		{
			IEntity gm = GetGame().GetGameMode();
			if (!gm)
				return false;

			m_bFatCfgSearched = true;
			m_FatCfg = FAT_DriverCameraComponent.Cast(gm.FindComponent(FAT_DriverCameraComponent));
		}

		if (!m_FatCfg || !m_FatCfg.IsViewRuleEnabled())
			return false;

		IEntity owner = GetOwner();
		if (!owner || owner != SCR_PlayerController.GetLocalControlledEntity())
			return false;

		if (!m_FatCompAccess)
			m_FatCompAccess = CompartmentAccessComponent.Cast(owner.FindComponent(CompartmentAccessComponent));

		if (m_FatCompAccess)
		{
			BaseCompartmentSlot slot = m_FatCompAccess.GetCompartment();
			if (slot && slot.IsPiloting())
				return false;
		}

		return true;
	}

	override void OnBeforeCameraUpdate(float pDt, bool pIsKeyframe)
	{
		super.OnBeforeCameraUpdate(pDt, pIsKeyframe);

		if (IsInThirdPerson() && FAT_ShouldForceFirstPerson())
			SetThirdPerson(false);
	}

	override void OnCameraSwitchPressed()
	{
		if (FAT_ShouldForceFirstPerson())
			return;

		super.OnCameraSwitchPressed();
	}
}
