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
public:
	enum { IDD = IDD_Debugger_DMALog };

	CDebugDMALogView(CDebuggerUI * debugger);
	virtual ~CDebugDMALogView(void);

	void RefreshList();

private:
	
	CListViewCtrl m_DMAList;
	CEdit         m_DMARamEdit;
	CStatic       m_DMARomStatic;

	LRESULT				OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT				OnClicked(WORD wNotifyCode, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled);
	LRESULT				OnActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT             OnRamAddrChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	LRESULT OnDestroy(void)
	{
		return 0;
	}

	uint32_t ConvertRamRom(uint32_t ramAddr);

	BEGIN_MSG_MAP_EX(CDebugDMALog)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_CODE_HANDLER(BN_CLICKED, OnClicked)
		MESSAGE_HANDLER(WM_ACTIVATE, OnActivate)
		COMMAND_HANDLER(IDC_DMA_RAM_EDIT, EN_CHANGE, OnRamAddrChanged)
	END_MSG_MAP()
};
