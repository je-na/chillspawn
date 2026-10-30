class CS_VehicleSpawner
{
	static const string ERR_NO_SPACE = "no clear space";

	// How long to wait before trying again when someone is still in the vehicle.
	static const int CREW_RETRY_MS = 60000;

	// When the configured spot is blocked, try these bearing offsets (degrees)
	// at these extra distances (metres).
	protected ref array<float> m_BearingOffsets = {0, 30, -30, 60, -60, 90, -90, 135, -135, 180};
	protected ref array<float> m_DistanceOffsets = {0, 3, 6};

	EntityAI Spawn(PlayerBase player, CS_SpawnPreset preset, out string error)
	{
		vector pos;
		float yaw;
		if (!FindClearSpot(player, preset, pos, yaw))
		{
			error = ERR_NO_SPACE;
			return null;
		}

		EntityAI ent = EntityAI.Cast(GetGame().CreateObjectEx(preset.ClassName, pos, ECE_PLACE_ON_SURFACE));
		if (!ent)
		{
			error = "could not create " + preset.ClassName;
			return null;
		}

		ent.SetOrientation(Vector(yaw, 0, 0));

		foreach (string attachment : preset.Attachments)
		{
			EntityAI att = ent.GetInventory().CreateAttachment(attachment);
			if (!att)
			{
				Print("[ChillSpawn] Could not attach " + attachment + " to " + preset.ClassName);
				continue;
			}

			// Batteries and the like start fully charged.
			if (att.GetCompEM())
				att.GetCompEM().SetEnergy(att.GetCompEM().GetEnergyMax());
		}

		Car car = Car.Cast(ent);
		if (car)
		{
			FillFluid(car, CarFluid.FUEL, preset.FuelFraction);
			FillFluid(car, CarFluid.COOLANT, preset.CoolantFraction);
			FillFluid(car, CarFluid.OIL, preset.OilFraction);
			FillFluid(car, CarFluid.BRAKE, preset.BrakeFluidFraction);
		}

		if (preset.DespawnSeconds > 0)
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(Despawn, preset.DespawnSeconds * 1000, false, ent);

		return ent;
	}

	// Searches around the configured spot for ground that is dry and free of
	// buildings, trees and other vehicles. The vehicle faces the way the player
	// would look to see it.
	protected bool FindClearSpot(PlayerBase player, CS_SpawnPreset preset, out vector pos, out float yaw)
	{
		vector origin = player.GetPosition();

		foreach (float extra : m_DistanceOffsets)
		{
			foreach (float offset : m_BearingOffsets)
			{
				float bearing = preset.BearingDegrees + offset;
				float distance = preset.DistanceMetres + extra;
				float rad = bearing * Math.DEG2RAD;

				pos = origin;
				pos[0] = origin[0] + Math.Sin(rad) * distance;
				pos[2] = origin[2] + Math.Cos(rad) * distance;
				pos[1] = GetGame().SurfaceY(pos[0], pos[2]);
				yaw = bearing;

				if (IsClear(player, preset, pos, yaw))
					return true;
			}
		}

		return false;
	}

	protected bool IsClear(PlayerBase player, CS_SpawnPreset preset, vector pos, float yaw)
	{
		if (GetGame().SurfaceIsSea(pos[0], pos[2]) || GetGame().SurfaceIsPond(pos[0], pos[2]))
			return false;

		// Lift the box off the ground so terrain bumps don't count as obstacles.
		vector centre = pos;
		centre[1] = pos[1] + preset.ClearanceHeight / 2 + 0.3;

		vector size = Vector(preset.ClearanceWidth, preset.ClearanceHeight, preset.ClearanceLength);
		array<Object> excluded = new array<Object>;
		excluded.Insert(player);

		return !GetGame().IsBoxColliding(centre, Vector(yaw, 0, 0), size, excluded);
	}

	protected void FillFluid(Car car, CarFluid fluid, float fraction)
	{
		if (fraction <= 0)
			return;

		car.Fill(fluid, car.GetFluidCapacity(fluid) * Math.Clamp(fraction, 0, 1));
	}

	void Despawn(EntityAI ent)
	{
		if (!ent)
			return;

		Transport transport = Transport.Cast(ent);
		if (transport)
		{
			for (int i = 0; i < transport.CrewSize(); i++)
			{
				if (transport.CrewMember(i))
				{
					GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(Despawn, CREW_RETRY_MS, false, ent);
					return;
				}
			}
		}

		GetGame().ObjectDelete(ent);
	}
}
