class CT_Util
{
	static string Lower(string text)
	{
		string result = text;
		result.ToLower();
		return result;
	}

	static bool ClassExists(string className)
	{
		if (className == "")
			return false;

		return GetGame().ConfigIsExisting("CfgVehicles " + className) || GetGame().ConfigIsExisting("CfgWeapons " + className) || GetGame().ConfigIsExisting("CfgMagazines " + className);
	}

	static string GetDisplayName(string className)
	{
		string name;
		TStringArray roots = {"CfgVehicles", "CfgWeapons", "CfgMagazines"};
		foreach (string root : roots)
		{
			string path = root + " " + className + " displayName";
			if (GetGame().ConfigIsExisting(path) && GetGame().ConfigGetText(path, name) && name != "")
				return name;
		}
		return className;
	}

	//! 1234567 -> "$1.234.567"
	static string FormatMoney(int value)
	{
		string sign = "";
		if (value < 0)
		{
			sign = "-";
			value = -value;
		}

		string digits = value.ToString();
		int len = digits.Length();
		string result = "";
		for (int i = 0; i < len; i++)
		{
			if (i > 0 && (len - i) % 3 == 0)
				result += ".";
			result += digits.Get(i);
		}
		return sign + "$" + result;
	}

	//! Horizontaler Abstand (Hoehe wird ignoriert)
	static float Distance2D(vector a, vector b)
	{
		a[1] = 0;
		b[1] = 0;
		return vector.Distance(a, b);
	}
}
