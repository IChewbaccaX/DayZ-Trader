class CT_TraderMenu extends UIScriptedMenu
{
	protected static const int COLOR_OK      = 0xFF5BC26A;
	protected static const int COLOR_ERROR   = 0xFFD43B3B;

	protected int m_TraderId = -1;
	protected float m_RefreshDelay;

	protected TextWidget m_TraderName;
	protected TextWidget m_CashText;
	protected TextWidget m_BankText;
	protected TextWidget m_StatusText;
	protected TextWidget m_DetailName;
	protected TextWidget m_DetailBuy;
	protected TextWidget m_DetailSell;
	protected TextWidget m_AttachmentInfo;
	protected TextWidget m_SellInfo;
	protected TextWidget m_AttachmentsLabel;

	protected TextListboxWidget m_CategoryList;
	protected TextListboxWidget m_ItemList;
	protected TextListboxWidget m_VariantList;
	protected TextListboxWidget m_SellList;

	protected CheckBoxWidget m_AttachmentsCheck;
	protected CheckBoxWidget m_UseBankCheck;
	protected EditBoxWidget m_AmountEdit;
	protected EditBoxWidget m_BankAmountEdit;

	protected Widget m_BuyButton;
	protected Widget m_SellButton;
	protected Widget m_SellAllButton;
	protected Widget m_DepositButton;
	protected Widget m_WithdrawButton;
	protected Widget m_CloseButton;
	protected Widget m_BankPanel;
	protected Widget m_AttachmentsRow;
	protected Widget m_UseBankRow;

	//! Zeilen der Listen -> Daten
	protected ref TStringArray m_Categories = new TStringArray;
	protected ref array<CT_ItemEntry> m_Items = new array<CT_ItemEntry>;
	protected ref TStringArray m_Variants = new TStringArray;
	protected ref array<EntityAI> m_SellItems = new array<EntityAI>;
	protected ref array<int> m_SellValues = new array<int>;

	override Widget Init()
	{
		layoutRoot = GetGame().GetWorkspace().CreateWidgets(CT_Const.LAYOUT_MENU);

		m_TraderName      = TextWidget.Cast(layoutRoot.FindAnyWidget("TraderName"));
		m_CashText        = TextWidget.Cast(layoutRoot.FindAnyWidget("CashText"));
		m_BankText        = TextWidget.Cast(layoutRoot.FindAnyWidget("BankText"));
		m_StatusText      = TextWidget.Cast(layoutRoot.FindAnyWidget("StatusText"));
		m_DetailName      = TextWidget.Cast(layoutRoot.FindAnyWidget("DetailName"));
		m_DetailBuy       = TextWidget.Cast(layoutRoot.FindAnyWidget("DetailBuy"));
		m_DetailSell      = TextWidget.Cast(layoutRoot.FindAnyWidget("DetailSell"));
		m_AttachmentInfo  = TextWidget.Cast(layoutRoot.FindAnyWidget("AttachmentInfo"));
		m_SellInfo        = TextWidget.Cast(layoutRoot.FindAnyWidget("SellInfo"));

		m_CategoryList    = TextListboxWidget.Cast(layoutRoot.FindAnyWidget("CategoryList"));
		m_ItemList        = TextListboxWidget.Cast(layoutRoot.FindAnyWidget("ItemList"));
		m_VariantList     = TextListboxWidget.Cast(layoutRoot.FindAnyWidget("VariantList"));
		m_SellList        = TextListboxWidget.Cast(layoutRoot.FindAnyWidget("SellList"));

		m_AttachmentsCheck = CheckBoxWidget.Cast(layoutRoot.FindAnyWidget("AttachmentsCheck"));
		m_UseBankCheck     = CheckBoxWidget.Cast(layoutRoot.FindAnyWidget("UseBankCheck"));
		m_AmountEdit       = EditBoxWidget.Cast(layoutRoot.FindAnyWidget("AmountEdit"));
		m_BankAmountEdit   = EditBoxWidget.Cast(layoutRoot.FindAnyWidget("BankAmountEdit"));

		m_BuyButton      = layoutRoot.FindAnyWidget("BuyButton");
		m_SellButton     = layoutRoot.FindAnyWidget("SellButton");
		m_SellAllButton  = layoutRoot.FindAnyWidget("SellAllButton");
		m_DepositButton  = layoutRoot.FindAnyWidget("DepositButton");
		m_WithdrawButton = layoutRoot.FindAnyWidget("WithdrawButton");
		m_CloseButton    = layoutRoot.FindAnyWidget("CloseButton");
		m_BankPanel      = layoutRoot.FindAnyWidget("BankPanel");
		m_AttachmentsRow = layoutRoot.FindAnyWidget("AttachmentsRow");
		m_UseBankRow     = layoutRoot.FindAnyWidget("UseBankRow");
		m_AttachmentsLabel = TextWidget.Cast(layoutRoot.FindAnyWidget("AttachmentsLabel"));

		CT_ClientData.Get().OnTradeResult.Insert(OnTradeResult);
		return layoutRoot;
	}

	void ~CT_TraderMenu()
	{
		if (CT_ClientData.Get())
			CT_ClientData.Get().OnTradeResult.Remove(OnTradeResult);
	}

	override void OnShow()
	{
		super.OnShow();
		GetGame().GetMission().AddActiveInputExcludes({"menu"});
	}

	override void OnHide()
	{
		super.OnHide();
		GetGame().GetMission().RemoveActiveInputExcludes({"menu"}, true);
	}

	protected CT_Market GetMarket()
	{
		return CT_ClientData.Get().Market;
	}

	void SetTrader(int traderId)
	{
		m_TraderId = traderId;
		CT_TraderEntry trader = GetMarket().GetTrader(traderId);
		if (!trader)
		{
			Close();
			return;
		}

		m_TraderName.SetText(trader.Name);

		bool banking = GetMarket().Data.EnableBanking;
		m_BankPanel.Show(banking);
		m_UseBankRow.Show(banking);
		m_BankText.Show(banking);
		m_SellAllButton.Show(GetMarket().Data.EnableSellAll);

		m_Categories.Clear();
		m_CategoryList.ClearItems();
		foreach (string name : trader.Categories)
		{
			if (!GetMarket().GetCategory(name))
				continue;
			m_Categories.Insert(name);
			m_CategoryList.AddItem(name, null, 0);
		}

		if (m_Categories.Count() > 0)
			m_CategoryList.SelectRow(0);
		FillItems();

		SetStatus("", true);
		RefreshBalances();
		RefreshSellList();
	}

	// ------------------------------------------------------------------
	// Kaufen-Seite
	// ------------------------------------------------------------------

	protected void FillItems()
	{
		m_Items.Clear();
		m_ItemList.ClearItems();

		int row = m_CategoryList.GetSelectedRow();
		if (row >= 0 && row < m_Categories.Count())
		{
			CT_Category category = GetMarket().GetCategory(m_Categories[row]);
			foreach (CT_ItemEntry entry : category.Items)
			{
				if (entry.BuyPrice < 0)
					continue;

				m_ItemList.AddItem(CT_Util.GetDisplayName(entry.ClassName) + "   -   " + CT_Util.FormatMoney(entry.BuyPrice), null, 0);
				m_Items.Insert(entry);
			}
		}

		if (m_Items.Count() > 0)
			m_ItemList.SelectRow(0);
		FillDetails();
	}

	protected CT_ItemEntry GetSelectedEntry()
	{
		int row = m_ItemList.GetSelectedRow();
		if (row < 0 || row >= m_Items.Count())
			return null;
		return m_Items[row];
	}

	protected void FillDetails()
	{
		m_Variants.Clear();
		m_VariantList.ClearItems();

		CT_ItemEntry entry = GetSelectedEntry();
		if (!entry)
		{
			m_DetailName.SetText("");
			m_DetailBuy.SetText("");
			m_DetailSell.SetText("");
			m_AttachmentInfo.SetText("");
			m_AttachmentsRow.Show(false);
			return;
		}

		m_Variants.Insert(entry.ClassName);
		m_Variants.InsertAll(entry.Variants);
		foreach (string variant : m_Variants)
			m_VariantList.AddItem(CT_Util.GetDisplayName(variant), null, 0);
		m_VariantList.SelectRow(0);

		m_DetailName.SetText(CT_Util.GetDisplayName(entry.ClassName));
		m_DetailBuy.SetText("Kaufpreis: " + CT_Util.FormatMoney(entry.BuyPrice));
		if (entry.SellPrice >= 0)
			m_DetailSell.SetText("Ankauf: " + CT_Util.FormatMoney(entry.SellPrice));
		else
			m_DetailSell.SetText("Ankauf: -");

		// Smart Attachments: nur Aufsaetze anzeigen, die einen Preis haben
		string names = "";
		int attachmentPrice = 0;
		foreach (string attachment : entry.Attachments)
		{
			CT_ItemEntry attEntry = GetMarket().FindEntry(attachment);
			if (!attEntry || attEntry.BuyPrice < 0)
				continue;
			if (names != "")
				names += ", ";
			names += CT_Util.GetDisplayName(attachment);
			attachmentPrice += attEntry.BuyPrice;
		}

		m_AttachmentsRow.Show(names != "");
		if (names != "")
		{
			m_AttachmentsLabel.SetText("Mit Aufsaetzen (+" + CT_Util.FormatMoney(attachmentPrice) + ")");
			m_AttachmentInfo.SetText(names);
		}
		else
		{
			m_AttachmentInfo.SetText("");
		}
	}

	protected void Buy()
	{
		CT_ItemEntry entry = GetSelectedEntry();
		if (!entry)
			return;

		string className = entry.ClassName;
		int variantRow = m_VariantList.GetSelectedRow();
		if (variantRow >= 0 && variantRow < m_Variants.Count())
			className = m_Variants[variantRow];

		int amount = m_AmountEdit.GetText().ToInt();
		amount = Math.Clamp(amount, 1, CT_Const.MAX_BUY_AMOUNT);
		m_AmountEdit.SetText(amount.ToString());

		bool withAttachments = m_AttachmentsRow.IsVisible() && m_AttachmentsCheck.IsChecked();
		CT_ClientData.Get().Buy(m_TraderId, className, amount, withAttachments, UseBank());
	}

	// ------------------------------------------------------------------
	// Verkaufen-Seite
	// ------------------------------------------------------------------

	protected void RefreshSellList()
	{
		m_SellItems.Clear();
		m_SellValues.Clear();
		m_SellList.ClearItems();

		PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
		if (!player)
			return;

		array<EntityAI> candidates = new array<EntityAI>;
		CT_ItemValue.GetSellCandidates(player, GetMarket(), true, candidates);

		int total = 0;
		string reason;
		foreach (EntityAI item : candidates)
		{
			int value = CT_ItemValue.GetSellValue(item, GetMarket(), m_TraderId, false, reason);
			if (value < 0)
				continue;

			m_SellList.AddItem(item.GetDisplayName() + "   -   " + CT_Util.FormatMoney(value), null, 0);
			m_SellItems.Insert(item);
			m_SellValues.Insert(value);
			total += value;
		}

		if (m_SellItems.Count() > 0)
		{
			m_SellList.SelectRow(0);
			m_SellInfo.SetText("Gesamtwert: " + CT_Util.FormatMoney(total));
		}
		else
		{
			m_SellInfo.SetText("Nichts zum Verkaufen");
		}
	}

	protected void Sell()
	{
		int row = m_SellList.GetSelectedRow();
		if (row < 0 || row >= m_SellItems.Count() || !m_SellItems[row])
			return;

		CT_ClientData.Get().Sell(m_TraderId, m_SellItems[row], UseBank());
	}

	// ------------------------------------------------------------------
	// Bank / Status
	// ------------------------------------------------------------------

	protected bool UseBank()
	{
		return GetMarket().Data.EnableBanking && m_UseBankCheck.IsChecked();
	}

	protected void BankTransfer(bool deposit)
	{
		int amount = m_BankAmountEdit.GetText().ToInt();
		if (amount <= 0)
		{
			SetStatus("Bitte einen Betrag eingeben.", false);
			return;
		}
		CT_ClientData.Get().BankTransfer(m_TraderId, amount, deposit);
	}

	protected void RefreshBalances()
	{
		m_CashText.SetText("Bargeld: " + CT_Util.FormatMoney(CT_ClientData.Get().Cash));
		m_BankText.SetText("Bank: " + CT_Util.FormatMoney(CT_ClientData.Get().Bank));
	}

	protected void SetStatus(string text, bool success)
	{
		m_StatusText.SetText(text);
		if (success)
			m_StatusText.SetColor(COLOR_OK);
		else
			m_StatusText.SetColor(COLOR_ERROR);
	}

	void OnTradeResult(bool success, string message)
	{
		SetStatus(message, success);
		RefreshBalances();
		// Inventaraenderungen kommen per Netzwerk etwas spaeter an
		m_RefreshDelay = 0.4;
	}

	// ------------------------------------------------------------------
	// Events
	// ------------------------------------------------------------------

	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (w == m_BuyButton)
			Buy();
		else if (w == m_SellButton)
			Sell();
		else if (w == m_SellAllButton)
			CT_ClientData.Get().SellAll(m_TraderId, UseBank());
		else if (w == m_DepositButton)
			BankTransfer(true);
		else if (w == m_WithdrawButton)
			BankTransfer(false);
		else if (w == m_CloseButton)
			Close();
		else
			return super.OnClick(w, x, y, button);

		return true;
	}

	override bool OnItemSelected(Widget w, int x, int y, int row, int column, int oldRow, int oldColumn)
	{
		if (w == m_CategoryList)
		{
			FillItems();
			return true;
		}
		if (w == m_ItemList)
		{
			FillDetails();
			return true;
		}
		return super.OnItemSelected(w, x, y, row, column, oldRow, oldColumn);
	}

	override void Update(float timeslice)
	{
		super.Update(timeslice);

		if (GetUApi().GetInputByID(UAUIBack).LocalPress())
		{
			Close();
			return;
		}

		PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
		if (!player || !player.IsAlive())
		{
			Close();
			return;
		}

		if (m_RefreshDelay > 0)
		{
			m_RefreshDelay -= timeslice;
			if (m_RefreshDelay <= 0)
				RefreshSellList();
		}
	}
}
