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
#include "DebuggerUI.h"
#include "ScriptSystem.h"

class CScriptList : public CListViewCtrl
{
public:
	BEGIN_MSG_MAP_EX(CScriptList)
	END_MSG_MAP()
};

class CEditEval : public CWindowImpl<CEditEval, CEdit>
{
private:
	//static char* m_EvalString;
	static const int HISTORY_MAX_ENTRIES = 20;
	vector<char*> m_History;
	int m_HistoryIdx;
	CDebugScripts* m_ScriptWindow;

public:
	CEditEval()
	{
		m_HistoryIdx = 0;
	}
	
	void SetScriptWindow(CDebugScripts* scriptWindow)
	{
		m_ScriptWindow = scriptWindow;
	}

	LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	
	BOOL Attach(HWND hWndNew)
	{
		return SubclassWindow(hWndNew);
	}

	BEGIN_MSG_MAP_EX(CEditEval)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
	END_MSG_MAP()
};

class CDebugScripts :
	public CDebugDialog < CDebugScripts >
{
private:
	CEditEval m_EvalEdit;
	CEdit m_ConsoleEdit;
	CScriptList m_ScriptList;
	char* m_SelectedScriptName;

public:
	enum { IDD = IDD_Debugger_Scripts };

	CDebugScripts(CDebuggerUI * debugger);
	//virtual ~CDebugScripts(void);

	void ConsolePrint(const char* text);
	void ConsoleClear();

	void RefreshList();
	void RefreshConsole();

	void EvaluateInSelectedInstance(char* code);

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(void)
	{
		return 0;
	}
	
	LRESULT OnClicked(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnScriptListClicked(NMHDR* pNMHDR);
	LRESULT OnScriptListCustomDraw(NMHDR* pNMHDR);

	BEGIN_MSG_MAP_EX(CDebugScripts)
		COMMAND_CODE_HANDLER(BN_CLICKED, OnClicked)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		NOTIFY_HANDLER_EX(IDC_SCRIPT_LIST, NM_DBLCLK, OnScriptListClicked)
		NOTIFY_HANDLER_EX(IDC_SCRIPT_LIST, NM_CUSTOMDRAW, OnScriptListCustomDraw)
		CHAIN_MSG_MAP_MEMBER(m_ScriptList)
		MSG_WM_DESTROY(OnDestroy)
	END_MSG_MAP()
};