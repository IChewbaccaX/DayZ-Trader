//! Schneller Zugriff auf die synchronisierten Trader-Daten. Wird auf Server UND Client verwendet.
class CT_Market
{
	//! Maximaler Abstand zwischen Objekt und konfigurierter Trader-Position, um es als Trader zu erkennen
	static const float OBJECT_MATCH_DISTANCE = 2.5;

	ref CT_SyncData Data;

	protected ref map<string, CT_ItemEntry> m_Entries;
	protected ref map<string, string> m_EntryCategory;
	protected ref map<string, int> m_CurrencyValues;
	//! Waehrungen absteigend nach Wert sortiert
	protected ref array<CT_CurrencyEntry> m_CurrenciesDesc;

	void CT_Market(CT_SyncData data)
	{
		Data = data;
		Build();
	}

	protected void Build()
	{
		m_Entries = new map<string, CT_ItemEntry>;
		m_EntryCategory = new map<string, string>;
		m_CurrencyValues = new map<string, int>;
		m_CurrenciesDesc = new array<CT_CurrencyEntry>;

		foreach (CT_Category category : Data.Categories)
		{
			foreach (CT_ItemEntry entry : category.Items)
			{
				Register(entry.ClassName, entry, category.Name);
				if (entry.Variants)
				{
					foreach (string variant : entry.Variants)
						Register(variant, entry, category.Name);
				}
			}
		}

		foreach (CT_CurrencyEntry currency : Data.Currencies)
		{
			if (currency.Value <= 0)
				continue;

			m_CurrencyValues.Set(CT_Util.Lower(currency.ClassName), currency.Value);

			int insertAt = m_CurrenciesDesc.Count();
			for (int i = 0; i < m_CurrenciesDesc.Count(); i++)
			{
				if (currency.Value > m_CurrenciesDesc[i].Value)
				{
					insertAt = i;
					break;
				}
			}
			if (insertAt < m_CurrenciesDesc.Count())
				m_CurrenciesDesc.InsertAt(currency, insertAt);
			else
				m_CurrenciesDesc.Insert(currency);
		}
	}

	protected void Register(string className, CT_ItemEntry entry, string category)
	{
		string key = CT_Util.Lower(className);
		if (m_Entries.Contains(key))
			Print("[CoreTrader] WARNUNG: " + className + " ist mehrfach im Markt eingetragen, erster Eintrag gilt.");
		else
		{
			m_Entries.Set(key, entry);
			m_EntryCategory.Set(key, category);
		}
	}

	CT_ItemEntry FindEntry(string className)
	{
		return m_Entries.Get(CT_Util.Lower(className));
	}

	//! Eintrag nur zurueckgeben, wenn der Trader die Kategorie fuehrt
	CT_ItemEntry FindEntryForTrader(int traderId, string className)
	{
		CT_TraderEntry trader = GetTrader(traderId);
		if (!trader)
			return null;

		string key = CT_Util.Lower(className);
		CT_ItemEntry entry = m_Entries.Get(key);
		if (!entry)
			return null;

		if (!TraderHasCategory(trader, m_EntryCategory.Get(key)))
			return null;

		return entry;
	}

	bool TraderHasCategory(CT_TraderEntry trader, string category)
	{
		foreach (string name : trader.Categories)
		{
			if (name == category)
				return true;
		}
		return false;
	}

	CT_Category GetCategory(string name)
	{
		foreach (CT_Category category : Data.Categories)
		{
			if (category.Name == name)
				return category;
		}
		return null;
	}

	CT_TraderEntry GetTrader(int traderId)
	{
		if (traderId < 0 || traderId >= Data.Traders.Count())
			return null;
		return Data.Traders[traderId];
	}

	//! Liefert die Trader-ID fuer ein angeschautes Objekt (NPC, Automat) oder -1
	int FindTraderForObject(Object obj)
	{
		if (!obj)
			return -1;

		string type = CT_Util.Lower(obj.GetType());
		vector pos = obj.GetPosition();

		foreach (int id, CT_TraderEntry trader : Data.Traders)
		{
			if (trader.ClassName != "" && CT_Util.Lower(trader.ClassName) != type)
				continue;

			if (CT_Util.Distance2D(pos, trader.Position) <= OBJECT_MATCH_DISTANCE)
				return id;
		}
		return -1;
	}

	bool IsCurrency(string className)
	{
		return m_CurrencyValues.Contains(CT_Util.Lower(className));
	}

	int GetCurrencyValue(string className)
	{
		return m_CurrencyValues.Get(CT_Util.Lower(className));
	}

	array<CT_CurrencyEntry> GetCurrenciesDescending()
	{
		return m_CurrenciesDesc;
	}
}
