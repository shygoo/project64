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

#pragma once

#include <Project64/UserInterface/resource.h>

class CAddBreakpointDlg : public CDialogImpl<CAddBreakpointDlg>
{
public:
	enum { IDD = IDD_Debugger_AddBreakpoint};

private:
	BEGIN_MSG_MAP_EX(CAddBreakpointDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_CODE_HANDLER(BN_CLICKED, OnClicked)
		MSG_WM_DESTROY(OnDestroy)
	END_MSG_MAP()
	
	LRESULT	OnClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnDestroy(void);
	LRESULT	OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		CenterWindow();
		return FALSE;
	}
};

class CRegisterTabs : public CTabCtrl
{
public:
	vector<int> m_TabIds;
	vector<CWindow> m_TabWindows;
	CWindow AddTab(char* caption, int dialogId, DLGPROC dlgProc);
	void ShowTab(int nPage);
	void ResetTabs();
};

class CCommandsList : public CListViewCtrl
{
public:
	BEGIN_MSG_MAP_EX(CCommandsList)
	END_MSG_MAP()
};

class CDebugCommandsView : public CDebugDialog<CDebugCommandsView>
{
public:
	enum { IDD = IDD_Debugger_Commands };

	CDebugCommandsView(CDebuggerUI * debugger);
	virtual ~CDebugCommandsView(void);

	void ShowAddress(DWORD address, BOOL top);

	static DWORD GPREditIds[32];
	static DWORD FPREditIds[32];

	static int MapGPREdit(DWORD controlId);
	static int MapFPREdit(DWORD controlId);

private:

	DWORD m_StartAddress;
	CRect m_DefaultWindowRect;

	CEditNumber m_AddressEdit;
	CCommandsList m_CommandList;
	int m_CommandListRows;
	CRegisterTabs m_RegisterTabs;
	CAddBreakpointDlg m_AddBreakpointDlg;
	CListBox m_BreakpointList;
	CScrollBar m_Scrollbar;

	CWindow m_GPRTab;
	CEditNumber m_GPREdits[32];
	CEditNumber m_HIEdit;
	CEditNumber m_LOEdit;
	
	CWindow m_FPRTab;
	CEditNumber m_FPREdits[32];
	
	void GotoEnteredAddress();
	void CheckCPUType();
	void RefreshBreakpointList();
	void RemoveSelectedBreakpoints();
	void RefreshRegisterEdits();
	
	LRESULT	OnInitDialog         (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT	OnActivate           (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT	OnSizing             (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT	OnGetMinMaxInfo      (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMouseWheel         (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT	OnScroll             (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT	OnMeasureItem        (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnAddrChanged        (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnListBoxClicked     (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT	OnClicked            (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT	OnCommandListClicked (NMHDR* pNMHDR);
	LRESULT OnRegisterTabChange  (NMHDR* pNMHDR);
	LRESULT OnCustomDrawList     (NMHDR* pNMHDR);
	LRESULT OnDestroy            (void);

	// todo fix mousewheel
	// win10 - doesn't work while hovering over list controls
	// win7 - only works while a control has keyboard focus

	BEGIN_MSG_MAP_EX(CDebugCommandsView)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_ACTIVATE, OnActivate)
		MESSAGE_HANDLER(WM_SIZING, OnSizing)
		MESSAGE_HANDLER(WM_GETMINMAXINFO, OnGetMinMaxInfo)
		MESSAGE_HANDLER(WM_MOUSEWHEEL, OnMouseWheel)
		MESSAGE_HANDLER(WM_VSCROLL, OnScroll)
		MESSAGE_HANDLER(WM_MEASUREITEM, OnMeasureItem)
		COMMAND_HANDLER(IDC_ADDR_EDIT, EN_CHANGE, OnAddrChanged)
		COMMAND_CODE_HANDLER(LBN_DBLCLK, OnListBoxClicked)
		COMMAND_CODE_HANDLER(BN_CLICKED, OnClicked)
		NOTIFY_HANDLER_EX(IDC_CMD_LIST, NM_DBLCLK, OnCommandListClicked)
		NOTIFY_HANDLER_EX(IDC_CMD_LIST, NM_RCLICK, OnCommandListClicked)
		NOTIFY_HANDLER_EX(IDC_REG_TABS, TCN_SELCHANGE, OnRegisterTabChange)
		NOTIFY_HANDLER_EX(IDC_CMD_LIST, NM_CUSTOMDRAW, OnCustomDrawList)
		CHAIN_MSG_MAP_MEMBER(m_CommandList)
		MSG_WM_DESTROY(OnDestroy)
	END_MSG_MAP()
};

