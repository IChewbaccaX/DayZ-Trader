//! Wertberechnung beim Verkauf. Wird auf Client (Anzeige) und Server (verbindlich) gleich gerechnet.
class CT_ItemValue
{
	//! Gibt -1 zurueck, wenn das Item (oder sein Inhalt) bei diesem Trader nicht verkauft werden kann
	static int GetSellValue(EntityAI item, CT_Market market, int traderId, bool allowRuined, out string reason)
	{
		CT_ItemEntry entry = market.FindEntryForTrader(traderId, item.GetType());
		if (!entry || entry.SellPrice < 0)
		{
			reason = item.GetDisplayName() + " wird hier nicht angekauft.";
			return -1;
		}

		if (!allowRuined && item.IsRuined())
		{
			reason = item.GetDisplayName() + " ist ruiniert.";
			return -1;
		}

		float factor = 1.0;
		Magazine mag = Magazine.Cast(item);
		ItemBase itemBase = ItemBase.Cast(item);
		if (mag)
		{
			// Munitionshaufen anteilig, Magazine zum Festpreis
			if (item.IsInherited(Ammunition_Base) && mag.GetAmmoMax() > 0)
			{
				float ammoCount = mag.GetAmmoCount();
				factor = ammoCount / mag.GetAmmoMax();
			}
		}
		else if (itemBase && itemBase.HasQuantity() && itemBase.GetQuantityMax() > 0)
		{
			factor = itemBase.GetQuantity() / itemBase.GetQuantityMax();
		}

		int value = Math.Round(entry.SellPrice * factor);

		GameInventory inventory = item.GetInventory();
		if (!inventory)
			return value;

		for (int i = 0; i < inventory.AttachmentCount(); i++)
		{
			int attValue = GetSellValue(inventory.GetAttachmentFromIndex(i), market, traderId, allowRuined, reason);
			if (attValue < 0)
				return -1;
			value += attValue;
		}

		CargoBase cargo = inventory.GetCargo();
		if (cargo)
		{
			for (int c = 0; c < cargo.GetItemCount(); c++)
			{
				int cargoValue = GetSellValue(cargo.GetItem(c), market, traderId, allowRuined, reason);
				if (cargoValue < 0)
				{
					reason = item.GetDisplayName() + " enthaelt Gegenstaende, die hier nicht angekauft werden.";
					return -1;
				}
				value += cargoValue;
			}
		}

		return value;
	}

	/**
	 * Verkaufbare "oberste" Gegenstaende des Spielers.
	 * includeEquipped = true: zusaetzlich Hand und Waffen-/Nahkampfslots (Einzelverkauf).
	 * includeEquipped = false: nur Inhalt/Anbauten von getragener Kleidung (Alles verkaufen).
	 */
	static void GetSellCandidates(PlayerBase player, CT_Market market, bool includeEquipped, array<EntityAI> result)
	{
		EntityAI inHands = player.GetHumanInventory().GetEntityInHands();
		array<EntityAI> all = new array<EntityAI>;
		player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, all);

		foreach (EntityAI entity : all)
		{
			if (!entity || entity == player || market.IsCurrency(entity.GetType()))
				continue;

			EntityAI parent = entity.GetHierarchyParent();
			if (!parent)
				continue;

			if (parent == player)
			{
				// direkt am Spieler: Hand, Waffenslots oder getragene Kleidung
				if (includeEquipped && (entity == inHands || !entity.IsClothing()))
					result.Insert(entity);
				continue;
			}

			// in/an getragener Kleidung (Rucksack, Weste, Hose ...)
			if (parent.GetHierarchyParent() == player && parent != inHands && parent.IsClothing())
				result.Insert(entity);
		}
	}
}
