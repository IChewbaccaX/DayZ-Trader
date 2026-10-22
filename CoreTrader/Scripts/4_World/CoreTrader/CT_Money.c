//! Bargeld im Inventar zaehlen, abziehen und auszahlen (Abziehen/Auszahlen nur serverseitig)
class CT_Money
{
	static int GetUnitCount(ItemBase item)
	{
		if (item.HasQuantity())
			return Math.Round(item.GetQuantity());
		return 1;
	}

	static void GetCurrencyItems(PlayerBase player, CT_Market market, array<ItemBase> result)
	{
		array<EntityAI> all = new array<EntityAI>;
		player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, all);
		foreach (EntityAI entity : all)
		{
			ItemBase item = ItemBase.Cast(entity);
			if (item && market.IsCurrency(item.GetType()))
				result.Insert(item);
		}
	}

	static int GetCash(PlayerBase player, CT_Market market)
	{
		if (!player || !market)
			return 0;

		array<ItemBase> items = new array<ItemBase>;
		GetCurrencyItems(player, market, items);

		int total = 0;
		foreach (ItemBase item : items)
			total += market.GetCurrencyValue(item.GetType()) * GetUnitCount(item);
		return total;
	}

	//! Zieht amount ab (kleine Scheine zuerst) und gibt Wechselgeld zurueck. Vorher GetCash pruefen!
	static void RemoveCash(PlayerBase player, CT_Market market, int amount)
	{
		if (amount <= 0)
			return;

		array<ItemBase> items = new array<ItemBase>;
		GetCurrencyItems(player, market, items);

		// aufsteigend nach Wert sortieren (einfaches Insertion-Sort, Listen sind klein)
		for (int i = 1; i < items.Count(); i++)
		{
			for (int j = i; j > 0 && market.GetCurrencyValue(items[j].GetType()) < market.GetCurrencyValue(items[j - 1].GetType()); j--)
				items.SwapItems(j, j - 1);
		}

		int remaining = amount;
		foreach (ItemBase item : items)
		{
			if (remaining <= 0)
				break;

			int value = market.GetCurrencyValue(item.GetType());
			int units = GetUnitCount(item);
			int needed = (remaining + value - 1) / value;
			int take = Math.Min(units, needed);

			if (take >= units)
				GetGame().ObjectDelete(item);
			else
				item.SetQuantity(units - take);

			remaining -= take * value;
		}

		if (remaining < 0)
			AddCash(player, market, -remaining);
	}

	//! Zahlt amount in moeglichst grossen Scheinen aus; was nicht ins Inventar passt, landet am Boden
	static void AddCash(PlayerBase player, CT_Market market, int amount)
	{
		foreach (CT_CurrencyEntry currency : market.GetCurrenciesDescending())
		{
			int count = amount / currency.Value;
			if (count <= 0)
				continue;

			amount -= count * currency.Value;

			int maxStack = GetGame().ConfigGetInt("CfgVehicles " + currency.ClassName + " varQuantityMax");
			if (maxStack <= 0)
				maxStack = 1;

			while (count > 0)
			{
				int stack = Math.Min(count, maxStack);
				ItemBase money = ItemBase.Cast(CreateForPlayer(player, currency.ClassName));
				if (!money)
				{
					Print("[CoreTrader] FEHLER: Waehrung " + currency.ClassName + " konnte nicht erstellt werden");
					break;
				}
				if (money.HasQuantity())
					money.SetQuantity(stack);
				else
					stack = 1;

				count -= stack;
			}
		}
	}

	//! Inventar -> leere Haende -> Boden
	static EntityAI CreateForPlayer(PlayerBase player, string className)
	{
		EntityAI item = player.GetInventory().CreateInInventory(className);
		if (!item && !player.GetHumanInventory().GetEntityInHands())
			item = player.GetHumanInventory().CreateInHands(className);
		if (!item)
			item = EntityAI.Cast(GetGame().CreateObjectEx(className, player.GetPosition(), ECE_PLACE_ON_SURFACE));
		return item;
	}
}
