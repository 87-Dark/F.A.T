modded class ACE_Overheating_BarrelComponentClass
{
	override protected float ComputeBarrelLength(IEntity weapon)
	{
		if (!weapon || !weapon.GetAnimation())
			return FALLBACK_BARREL_LENGTH;

		return super.ComputeBarrelLength(weapon);
	}
}
