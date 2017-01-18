/****************************************************************************
*                                                                           *
* Project64 - A Nintendo 64 emulator.                                      *
* http://www.pj64-emu.com/                                                  *
* Copyright (C) 2012 Project64. All rights reserved.                        *
*                                                                           *
* License:                                                                  *
* GNU/GPLv2 http://www.gnu.org/licenses/gpl-2.0.html                        *
*                                                                           *
****************************************************************************/

#include "stdafx.h"
#include "DebuggerUI.h"

#include <stdio.h>
#include <Common/path.h>

#include "Symbols.h"

CDebugSymbols::CDebugSymbols(CDebuggerUI * debugger) :
	CDebugDialog<CDebugSymbols>(debugger)
{
	
}
/*

type,address,name,description

u32,80370000,variable1
code,80370000,variable1

*/

LRESULT CDebugSymbols::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	DlgResize_Init();

	m_SymbolsListView.Attach(GetDlgItem(IDC_SYMBOLS_LIST));
	m_SymbolsListView.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);

	m_SymbolsListView.AddColumn("Address", 0);
	m_SymbolsListView.AddColumn("Type", 1);
	m_SymbolsListView.AddColumn("Name", 2);
	m_SymbolsListView.AddColumn("Description", 3);
	m_SymbolsListView.AddColumn("Value", 4);

	m_SymbolsListView.SetColumnWidth(0, 70);
	m_SymbolsListView.SetColumnWidth(1, 40);
	m_SymbolsListView.SetColumnWidth(2, 100);
	m_SymbolsListView.SetColumnWidth(3, 120);

	CSymbols::Load();
	Refresh();

	WindowCreated();
	return 0;
}

LRESULT CDebugSymbols::OnClicked(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	switch (wID)
	{
	case IDCANCEL:
		EndDialog(0);
		break;
	case IDC_ADDSYMBOL_BTN:
		m_AddSymbolDlg.DoModal();
		CSymbols::Save();
		Refresh();
		break;
	case IDC_REMOVESYMBOL_BTN:
		{
			int id = m_SymbolsListView.GetItemData(m_SymbolsListView.GetSelectedIndex());
			CSymbols::RemoveEntryById(id);
			CSymbols::Save();
			Refresh();
			break;
		}
	}
	return FALSE;
}

LRESULT	CDebugSymbols::OnListClicked(NMHDR* pNMHDR)
{
	// Open it in memory viewer/commands viewer
	NMITEMACTIVATE* pIA = reinterpret_cast<NMITEMACTIVATE*>(pNMHDR);
	int nItem = pIA->iItem;

	int id = m_SymbolsListView.GetItemData(nItem);
	SYMBOLENTRY* symbol = CSymbols::GetEntryById(id);
	
	if (symbol->type == 0) // code
	{
		m_Debugger->Debug_ShowCommandsLocation(symbol->address, true);
	}
	else // data/number
	{
		m_Debugger->Debug_ShowMemoryLocation(symbol->address, true);
	}

	return CDRF_DODEFAULT;
}

void CDebugSymbols::Refresh()
{
	m_SymbolsListView.SetRedraw(FALSE);
	m_SymbolsListView.DeleteAllItems();
	int count = CSymbols::GetCount();
	for (int i = 0; i < count; i++)
	{
		stdstr addrStr = stdstr_f("%08X", CSymbols::GetAddressByIndex(i));
		char* type = CSymbols::GetTypeStrByIndex(i);
		char* name = CSymbols::GetNameByIndex(i);
		char* desc = CSymbols::GetDescriptionByIndex(i);

		int id = CSymbols::GetIdByIndex(i);

		m_SymbolsListView.AddItem(i, 0, addrStr.c_str());
		m_SymbolsListView.AddItem(i, 1, type);
		m_SymbolsListView.AddItem(i, 2, name);
		m_SymbolsListView.AddItem(i, 3, desc);

		m_SymbolsListView.SetItemData(i, id);
	}
	m_SymbolsListView.SetRedraw(TRUE);
}

// Add Symbol dialog

LRESULT CAddSymbolDlg::OnClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	switch (wID)
	{
	case IDCANCEL:
		EndDialog(0);
		break;
	case IDOK:
		uint32_t address = m_AddressEdit.GetValue();
		int type = m_TypeComboBox.GetCurSel();

		int nameLen = m_NameEdit.GetWindowTextLengthA() + 1;
		int descLen = m_DescriptionEdit.GetWindowTextLengthA() + 1;

		char* name = (char*)malloc(nameLen);
		char* description = (char*)malloc(descLen);

		m_NameEdit.GetWindowTextA(name, nameLen);
		m_DescriptionEdit.GetWindowTextA(description, descLen);

		CSymbols::Add(type, address, name, description);
		EndDialog(0);
		break;
	}
	return 0;
}

LRESULT	CAddSymbolDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CenterWindow();

	m_AddressEdit.Attach(GetDlgItem(IDC_ADDR_EDIT));
	m_AddressEdit.SetDisplayType(CEditNumber::DisplayHex);
	m_TypeComboBox.Attach(GetDlgItem(IDC_TYPE_COMBOBOX));
	m_NameEdit.Attach(GetDlgItem(IDC_NAME_EDIT));
	m_DescriptionEdit.Attach(GetDlgItem(IDC_DESC_EDIT));

	for (int i = 0;; i++)
	{
		char* type = CSymbols::SymbolTypes[i];
		if (type == NULL)
		{
			break;
		}
		m_TypeComboBox.AddString(type);
	}

	return FALSE;
}