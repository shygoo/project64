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

#include <stdafx.h>
#include <Project64/UserInterface/resource.h>

class CDebugAddBreakpoint :
	public CDebugDialog < CDebugAddBreakpoint >
{
public:
	enum { IDD = IDD_Debugger_AddBreakpoint };

	CDebugAddBreakpoint(CDebuggerUI* debugger);

private:
	BEGIN_MSG_MAP_EX(CDebugAddBreakpoint)
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