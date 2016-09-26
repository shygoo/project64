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

#include "ScriptSystem.h"

class CEditEval : public CWindowImpl<CEditEval, CEdit>
{
public:
	LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (wParam == VK_RETURN)
		{
			size_t codeLength = GetWindowTextLength() + 1;
			char* code = (char*)malloc(codeLength);
			GetWindowTextA(code, codeLength);
			CScriptSystem::Eval(code);
			free(code);
			SetWindowTextA("");
		}
		bHandled = FALSE;
		return 0;
	}

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

public:
	enum { IDD = IDD_Debugger_Scripts };

	CDebugScripts(CDebuggerUI * debugger);
	//virtual ~CDebugScripts(void);
	
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnDestroy(void)
	{
		return 0;
	}
	
	LRESULT CDebugScripts::OnClicked(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	BEGIN_MSG_MAP_EX(CDebugScripts)
		COMMAND_CODE_HANDLER(BN_CLICKED, OnClicked)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MSG_WM_DESTROY(OnDestroy)
	END_MSG_MAP()

};