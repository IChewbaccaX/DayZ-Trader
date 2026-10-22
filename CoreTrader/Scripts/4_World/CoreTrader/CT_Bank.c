/**
 * Schnittstelle fuer Bankkonten. Standard ist ein eigenes JSON-Konto pro Spieler.
 * Fuer andere Banksysteme (z.B. LBmaster) eine Klasse ableiten und per
 * CT_TraderServer.Get().SetBankProvider(...) registrieren.
 */
class CT_BankProvider
{
	int GetBalance(PlayerBase player)
	{
		return 0;
	}

	bool Deposit(PlayerBase player, int amount)
	{
		return false;
	}

	bool Withdraw(PlayerBase player, int amount)
	{
		return false;
	}
}

class CT_BankAccount
{
	int Balance;
}

class CT_JsonBankProvider extends CT_BankProvider
{
	protected int m_StartBalance;
	protected ref map<string, ref CT_BankAccount> m_Accounts;

	void CT_JsonBankProvider(int startBalance)
	{
		m_StartBalance = startBalance;
		m_Accounts = new map<string, ref CT_BankAccount>;

		if (!FileExist(CT_Const.BANK_DIR))
			MakeDirectory(CT_Const.BANK_DIR);
	}

	protected string GetFile(string uid)
	{
		return CT_Const.BANK_DIR + uid + ".json";
	}

	protected CT_BankAccount GetAccount(PlayerBase player)
	{
		if (!player || !player.GetIdentity())
			return null;

		string uid = player.GetIdentity().GetPlainId();
		CT_BankAccount account = m_Accounts.Get(uid);
		if (account)
			return account;

		account = new CT_BankAccount;
		string file = GetFile(uid);
		if (FileExist(file))
			JsonFileLoader<CT_BankAccount>.JsonLoadFile(file, account);
		else
		{
			account.Balance = m_StartBalance;
			JsonFileLoader<CT_BankAccount>.JsonSaveFile(file, account);
		}

		m_Accounts.Set(uid, account);
		return account;
	}

	protected void Save(PlayerBase player, CT_BankAccount account)
	{
		JsonFileLoader<CT_BankAccount>.JsonSaveFile(GetFile(player.GetIdentity().GetPlainId()), account);
	}

	override int GetBalance(PlayerBase player)
	{
		CT_BankAccount account = GetAccount(player);
		if (!account)
			return 0;
		return account.Balance;
	}

	override bool Deposit(PlayerBase player, int amount)
	{
		CT_BankAccount account = GetAccount(player);
		if (!account || amount <= 0)
			return false;

		account.Balance += amount;
		Save(player, account);
		return true;
	}

	override bool Withdraw(PlayerBase player, int amount)
	{
		CT_BankAccount account = GetAccount(player);
		if (!account || amount <= 0 || account.Balance < amount)
			return false;

		account.Balance -= amount;
		Save(player, account);
		return true;
	}
}
