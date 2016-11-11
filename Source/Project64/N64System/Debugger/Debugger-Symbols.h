/****************************************************************************
*                                                                           *
* Project64 - A Nintendo 64 emulator.                                       *
* http://www.pj64-emu.com/                                                  *
* Copyright (C) 2012 Project64. All rights reserved.                        *
*                                                                           *
* License:                                                                  *
* GNU/GPLv2 http://www.gnu.org/licenses/gpl-2.0.html                        *
*                                                                           *
****************************************************************************/

#pragma once
#include "DebuggerUI.h"

class CAddSymbolDlg : public CDialogImpl<CAddSymbolDlg>
{
public:
	enum { IDD = IDD_Debugger_AddSymbol };

private:

	CEditNumber m_AddressEdit;
	CComboBox   m_TypeComboBox;
	CEdit       m_NameEdit;
	CEdit       m_DescriptionEdit;
	
	LRESULT	OnClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnDestroy(void)
	{
		return 0;
	}
	LRESULT	OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	BEGIN_MSG_MAP_EX(CAddSymbolDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_CODE_HANDLER(BN_CLICKED, OnClicked)
		MSG_WM_DESTROY(OnDestroy)
	END_MSG_MAP()

};

// todo maybe add char* ownerName and use a TreeView

class CDebugSymbols :
	public CDebugDialog<CDebugSymbols>,
	public CDialogResize<CDebugSymbols>
{
private:
	CListViewCtrl m_SymbolsListView;
	CAddSymbolDlg m_AddSymbolDlg;

public:
	enum { IDD = IDD_Debugger_Symbols };
	
	CDebugSymbols(CDebuggerUI * debugger);
	//virtual ~CDebugScripts(void);
	
	void Refresh();

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClicked(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT	OnListClicked(NMHDR* pNMHDR);
	LRESULT OnDestroy(void)
	{
		return 0;
	}

	BEGIN_MSG_MAP_EX(CDebugSymbols)
		COMMAND_CODE_HANDLER(BN_CLICKED, OnClicked)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MSG_WM_DESTROY(OnDestroy)
		NOTIFY_HANDLER_EX(IDC_SYMBOLS_LIST, NM_DBLCLK, OnListClicked)
		//NOTIFY_HANDLER_EX(IDC_CMD_LIST, NM_RCLICK, OnListClicked)
		CHAIN_MSG_MAP(CDialogResize<CDebugSymbols>)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CDebugSymbols)
		DLGRESIZE_CONTROL(IDC_SEARCH_EDIT, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_REMOVESYMBOL_BTN, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_ADDSYMBOL_BTN, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_SYMBOLS_LIST, DLSZ_SIZE_X | DLSZ_SIZE_Y)
	END_DLGRESIZE_MAP()
};

