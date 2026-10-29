[ComponentEditorProps(category: "FAT/GameMode", description: "Regle de vue : conducteur libre (1ere/3e), tout le reste en 1ere personne forcee. A ajouter sur l'entite GameMode du scenario.")]
class FAT_DriverCameraComponentClass : SCR_BaseGameModeComponentClass
{
}

class FAT_DriverCameraComponent : SCR_BaseGameModeComponent
{
	[Attribute("1", UIWidgets.CheckBox, "Activer la regle : conducteur libre, tout le reste en 1ere personne forcee")]
	protected bool m_bViewRuleEnabled;

	bool IsViewRuleEnabled()
	{
		return m_bViewRuleEnabled;
	}
}
