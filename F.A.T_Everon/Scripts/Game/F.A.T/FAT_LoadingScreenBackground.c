
modded class SCR_MissionHeader
{
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Fond ecran de chargement (FAT) - image plein ecran 1920x1080", "edds imageset")]
	ResourceName m_sFatLoadingBackground;

	[Attribute("0", UIWidgets.CheckBox, "Masquer le cadre photo de l'ecran de chargement (FAT)")]
	bool m_bFatHideLoadingPhoto;
}

modded class SCR_LoadingScreenComponent
{
	override void OnMissionDataRetrieved(SCR_MissionHeader header)
	{
		super.OnMissionDataRetrieved(header);

		if (!header || !m_Widgets)
			return;

		if (!header.m_sFatLoadingBackground.IsEmpty() && m_Widgets.m_wBackground)
		{
			ImageWidget bg = ImageWidget.Cast(m_Widgets.m_wBackground.FindAnyWidget("BackgroundImage"));
			if (bg)
				bg.LoadImageTexture(0, header.m_sFatLoadingBackground);
		}

		if (header.m_bFatHideLoadingPhoto && m_Widgets.m_wPhotoGroup)
			m_Widgets.m_wPhotoGroup.SetVisible(false);
	}
}
