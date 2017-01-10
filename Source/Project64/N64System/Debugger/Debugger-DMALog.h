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
#include "DebuggerUI.h"

class CDebugDMALogView :
	public CDebugDialog < CDebugDMALogView >
{
	BEGIN_MSG_MAP_EX(CDebugDMALog)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_CODE_HANDLER(BN_CLICKED, OnClicked)
		MESSAGE_HANDLER(WM_ACTIVATE, OnActivate)
	END_MSG_MAP()

	LRESULT				OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT				OnClicked(WORD wNotifyCode, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled);
	LRESULT				OnActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

public:
	enum { IDD = IDD_Debugger_DMALog };

	CDebugDMALogView(CDebuggerUI * debugger);
	virtual ~CDebugDMALogView(void);

	CListViewCtrl m_DMAList;

	LRESULT OnDestroy(void)
	{
		return 0;
	}

	void RefreshList();

};
