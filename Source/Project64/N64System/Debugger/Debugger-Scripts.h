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

class CEditEval : public CWindowImpl<CEditEval, CEdit>
{
private:
	//static char* m_EvalString;
	static const int HISTORY_MAX_ENTRIES = 20;
	vector<char*> m_History;
	int m_HistoryIdx;

public:
	static void CALLBACK EvalAsync(ULONG_PTR lpJsCode)
	{
		//const char* jsCode = (const char*)lpJsCode;
		//int result = duk_peval_string(CScriptSystem::m_Ctx, jsCode);
		//const char* msg = duk_safe_to_string(CScriptSystem::m_Ctx, -1);
		//CScriptSystem::ConsolePrint(msg);
		//CScriptSystem::ConsolePrint("\r\n");
		//duk_pop(CScriptSystem::m_Ctx);
	}

	LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (wParam == VK_UP)
		{
			if (m_HistoryIdx > 0)
			{
				char* code = m_History[--m_HistoryIdx];
				SetWindowTextA(code);
				int selEnd = strlen(code);
				SetSel(selEnd, selEnd);
			}
		}
		else if (wParam == VK_DOWN)
		{
			int size = m_History.size();
			if (m_HistoryIdx < size - 1)
			{
				char* code = m_History[++m_HistoryIdx];
				SetWindowTextA(code);
				int selEnd = strlen(code);
				SetSel(selEnd, selEnd);
			}
			else if(m_HistoryIdx < size)
			{
				SetWindowTextA("");
				m_HistoryIdx++;
			}
		}
		else if (wParam == VK_RETURN)
		{
			size_t codeLength = GetWindowTextLength() + 1;
			char* code = (char*)malloc(codeLength);
			GetWindowTextA(code, codeLength);
			//CScriptSystem::QueueAPC(EvalAsync, (ULONG_PTR)code); // code mem freed here
			SetWindowTextA("");
			int historySize = m_History.size();
			
			// remove duplicate
			for (int i = 0; i < historySize; i++)
			{
				if (strcmp(code, m_History[i]) == 0)
				{
					free(m_History[i]);
					m_History.erase(m_History.begin() + i);
					historySize--;
					break;
				}
			}

			// remove oldest if maxed
			if (historySize >= HISTORY_MAX_ENTRIES)
			{
				m_History.erase(m_History.begin() + 0);
				historySize--;
			}
			
			m_History.push_back(code);
			m_HistoryIdx = ++historySize;
		}
		bHandled = FALSE;
		return 0;
	}

	BOOL Attach(HWND hWndNew)
	{
		m_HistoryIdx = 0;
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
	CListViewCtrl m_ScriptList;

public:
	enum { IDD = IDD_Debugger_Scripts };

	CDebugScripts(CDebuggerUI * debugger);
	//virtual ~CDebugScripts(void);

	void ConsolePrint(const char* text);
	void ConsoleClear();

	void RefreshList();
	void RefreshConsole();

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(void)
	{
		return 0;
	}
	
	LRESULT OnClicked(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnScriptListClicked(NMHDR* pNMHDR);

	BEGIN_MSG_MAP_EX(CDebugScripts)
		COMMAND_CODE_HANDLER(BN_CLICKED, OnClicked)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		NOTIFY_HANDLER_EX(IDC_SCRIPT_LIST, NM_DBLCLK, OnScriptListClicked)
		MSG_WM_DESTROY(OnDestroy)
	END_MSG_MAP()
};