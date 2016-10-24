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

//char* CEditEval::m_EvalString;

CDebugScripts::CDebugScripts(CDebuggerUI* debugger) :
CDebugDialog<CDebugScripts>(debugger)
{
	CScriptSystem::SetScriptsWindow(this);
}

LRESULT CDebugScripts::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	HFONT monoFont = CreateFont(-11, 0, 0, 0,
		FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		CLEARTYPE_QUALITY, FF_DONTCARE, "Consolas"
	);
	
	m_EvalEdit.Attach(GetDlgItem(IDC_EVAL_EDIT));
	m_ConsoleEdit.Attach(GetDlgItem(IDC_CONSOLE_EDIT));

	m_EvalEdit.SetFont(monoFont);
	m_ConsoleEdit.SetFont(monoFont);
	
	WindowCreated();
	return 0;
}

void CDebugScripts::ConsolePrint(const char* text)
{
	::ShowWindow(*this, SW_SHOWNOACTIVATE);
	int curLength = m_ConsoleEdit.GetWindowTextLengthA();
	int textLength = strlen(text);
	int newLength = curLength + textLength + 1;
	char* newText = (char*) malloc(newLength);
	m_ConsoleEdit.GetWindowTextA(newText, curLength + 1);
	strcpy(newText + curLength, text);
	m_ConsoleEdit.SetWindowTextA(newText);
}

void CDebugScripts::ConsoleClear()
{
	m_ConsoleEdit.SetWindowTextA("");
}

LRESULT CDebugScripts::OnClicked(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	switch (wID)
	{
	case IDCANCEL:
		EndDialog(0);
		break;
	}
	return FALSE;
}