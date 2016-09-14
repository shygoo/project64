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
	
	BEGIN_MSG_MAP_EX(CAddBreakpointDlg)
		COMMAND_CODE_HANDLER(BN_CLICKED, OnClicked)
		MSG_WM_DESTROY(OnDestroy)
	END_MSG_MAP()
	
	LRESULT	OnClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnDestroy(void);
};

class CRegisterTabs : public CWindowImpl<CRegisterTabs, CTabCtrl>
{
public:

	BEGIN_MSG_MAP_EX(CRegisterTabs)
		//MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()
	//enum {IDC = IDC_CMD_REGTABS};
	//CRegisterTabs();

	//LRESULT	OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	void Attach(HWND hWnd);
};

class CCommandsList : public CWindowImpl<CCommandsList, CListViewCtrl>
{

public:
	/*
	BOOL ProcessWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult, DWORD dwMsgMapID = 0)
	{
		MessageBox("got window message", "goteem", MB_OK);
		return TRUE;
	}*/

	BEGIN_MSG_MAP_EX(CCommandsList)

		//MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		//MESSAGE_HANDLER(WM_MOUSEWHEEL, OnMouseWheel)
		//MESSAGE_HANDLER(WM_VSCROLL, OnMouseWheel)
		//NOTIFY_HANDLER_EX(IDC_CMD_LIST, NM_RCLICK, OnClicked)
	END_MSG_MAP()
	
	void Attach(HWND hWnd);
	
};

class CDebugCommandsView : public CDebugDialog < CDebugCommandsView >
{
public:
	enum { IDD = IDD_Debugger_Commands };

	CDebugCommandsView(CDebuggerUI * debugger);
	virtual ~CDebugCommandsView(void);

	void ShowAddress(DWORD address, BOOL top);
	void RefreshBreakpointList();

private:
	static const int listLength;

	DWORD m_StartAddress;

	CEdit m_EditAddress;
	CCommandsList m_CommandList;
	CRegisterTabs m_RegisterTabs;
	CAddBreakpointDlg m_AddBreakpointDlg;
	CListBox m_BreakpointList;

	void GotoEnteredAddress();
	
	LRESULT	OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT	OnClicked(WORD wNotifyCode, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled);
	//LRESULT	OnKeyDown(WORD wNotifyCode, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled);

	LRESULT OnAddrChanged(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	//LRESULT OnAddrKeyDown(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnMouseWheel(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(void);
	LRESULT	OnListClicked(NMHDR* pNMHDR);
	LRESULT OnCustomDrawList(NMHDR* pNMHDR);

	// mousewheel todo fix
	// win10 messages are blocked while hovering over list controls
	// win7 mousewheel only works while a control is focused

	BEGIN_MSG_MAP_EX(CDebugCommandsView)
		COMMAND_HANDLER(IDC_CMD_ADDR, EN_CHANGE, OnAddrChanged)
		//COMMAND_HANDLER(IDC_CMD_ADDR, NM_KEYDOWN, OnAddrChanged)
		NOTIFY_HANDLER_EX(IDC_CMD_LIST, NM_RCLICK, OnListClicked)
		NOTIFY_HANDLER_EX(IDC_CMD_LIST, NM_DBLCLK, OnListClicked)
		NOTIFY_HANDLER_EX(IDC_CMD_LIST, NM_CUSTOMDRAW, OnCustomDrawList)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_MOUSEWHEEL, OnMouseWheel)
		COMMAND_CODE_HANDLER(BN_CLICKED, OnClicked)
		CHAIN_MSG_MAP_MEMBER(m_CommandList)
		MSG_WM_DESTROY(OnDestroy)
	END_MSG_MAP()
};

