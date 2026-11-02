// Gives a kit to a player: into their inventory if the kit says so and there's
// room, otherwise on the ground in front of them. Contents are attached where
// they fit, otherwise put in cargo. Magazines and ammo are full, batteries
// charged, bottles full of water; script-created items start pristine.
class CS_KitSpawner
{
	// Items per row in front of the player before starting a new row.
	static const int ROW_SIZE = 7;

	// Returns how many top-level items were created.
	int Spawn(PlayerBase player, CS_Kit kit)
	{
		vector origin = player.GetPosition();
		vector dir = player.GetDirection();
		int created = 0;
		int dropped = 0;

		foreach (CS_KitItem item : kit.Items)
		{
			for (int n = 0; n < item.Count; n++)
			{
				EntityAI ent = null;
				if (kit.ToInventory)
					ent = player.GetInventory().CreateInInventory(item.ClassName);

				if (!ent)
				{
					vector pos = DropPosition(origin, dir, dropped);
					ent = EntityAI.Cast(GetGame().CreateObjectEx(item.ClassName, pos, ECE_PLACE_ON_SURFACE));
					dropped++;
				}

				if (!ent)
				{
					Print("[ChillSpawn] Could not create " + item.ClassName);
					continue;
				}

				Prepare(ent);
				AddContents(ent, item.Contents);
				created++;
			}
		}

		return created;
	}

	// Spreads items in rows in front of the player so they don't stack.
	protected vector DropPosition(vector origin, vector dir, int index)
	{
		int column = index % ROW_SIZE;
		int row = index / ROW_SIZE;
		float angle = Math.Atan2(dir[0], dir[2]) + (column - ROW_SIZE / 2) * 0.3;
		float distance = 1.2 + row * 0.6;

		vector pos = origin;
		pos[0] = origin[0] + Math.Sin(angle) * distance;
		pos[2] = origin[2] + Math.Cos(angle) * distance;
		pos[1] = GetGame().SurfaceY(pos[0], pos[2]);
		return pos;
	}

	protected void AddContents(EntityAI parent, array<ref CS_KitItem> contents)
	{
		if (!contents)
			return;

		Weapon_Base weapon = Weapon_Base.Cast(parent);
		foreach (CS_KitItem item : contents)
		{
			for (int n = 0; n < item.Count; n++)
			{
				EntityAI child;
				if (weapon && GetGame().ConfigIsExisting("CfgMagazines " + item.ClassName))
				{
					// Loads the gun properly: full magazine and a round chambered.
					child = weapon.SpawnAttachedMagazine(item.ClassName, WeaponWithAmmoFlags.CHAMBER);
				}
				else
				{
					child = parent.GetInventory().CreateAttachment(item.ClassName);
					if (!child)
						child = parent.GetInventory().CreateInInventory(item.ClassName);
				}

				if (!child)
				{
					Print("[ChillSpawn] Could not put " + item.ClassName + " in " + parent.GetType());
					continue;
				}

				Prepare(child);
				AddContents(child, item.Contents);
			}
		}
	}

	protected void Prepare(EntityAI ent)
	{
		Magazine mag = Magazine.Cast(ent);
		if (mag)
			mag.ServerSetAmmoMax();

		ItemBase item = ItemBase.Cast(ent);
		if (item && !mag && item.IsLiquidContainer())
		{
			item.SetLiquidType(LIQUID_WATER);
			item.SetQuantityMax();
		}
		else if (item && !mag && item.HasQuantity())
		{
			item.SetQuantityMax();
		}

		if (ent.GetCompEM())
			ent.GetCompEM().SetEnergy(ent.GetCompEM().GetEnergyMax());
	}
}
