// Crash protection for people in vehicles ChillSpawn spawned with ProtectCrew.
// Crash damage to the crew is applied by server-side script, so no client mod
// is needed.
modded class CarScript
{
	protected bool m_CS_ProtectCrew;

	void CS_SetProtectCrew(bool protect)
	{
		m_CS_ProtectCrew = protect;
	}

	override void DamageCrew(float dmg)
	{
		if (m_CS_ProtectCrew)
			return;

		super.DamageCrew(dmg);
	}
}
