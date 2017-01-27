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

CDebugStackTrace::CDebugStackTrace(CDebuggerUI* debugger) :
CDebugDialog<CDebugStackTrace>(debugger)
{
}

CDebugStackTrace::~CDebugStackTrace()
{
}

LRESULT CDebugStackTrace::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	DlgResize_Init();

	m_List.Attach(GetDlgItem(IDC_STACKTRACE_LIST));
	m_List.AddColumn("Routine", 0);
	m_List.AddColumn("Name", 0);
	WindowCreated();
	return TRUE;
}

LRESULT CDebugStackTrace::OnActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	RefreshList();
	return FALSE;
}

LRESULT CDebugStackTrace::OnDestroy(void)
{
	return FALSE;
}

LRESULT CDebugStackTrace::OnClicked(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	switch (wID)
	{
	case IDCANCEL:
		EndDialog(0);
		break;
	}
	return FALSE;
}

void CDebugStackTrace::RefreshList()
{
	vector<uint32_t>* stackTrace = m_Debugger->StackTrace();

	m_List.SetRedraw(FALSE);
	m_List.DeleteAllItems();

	for (int i = 0; i < stackTrace->size(); i++)
	{
		m_List.AddItem(i, 0, stdstr_f("%08X", stackTrace->at(i)).c_str());
		m_List.AddItem(i, 1, "symbol");
	}

	m_List.SetRedraw(TRUE);
}