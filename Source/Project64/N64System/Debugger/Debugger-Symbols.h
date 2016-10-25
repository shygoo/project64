/****************************************************************************
*                                                                           *
* Project64 - A Nintendo 64 emulator.                                       *
* http://www.pj64-emu.com/                                                  *
* Copyright (C) 2012 Project64. All rights reserved.                        *
*                                                                           *
* License:                                                                  *
* GNU/GPLv2 http://www.gnu.org/licenses/gpl-2.0.html                        *
*                                                                           *
****************************************************************************/

#pragma once
#include "DebuggerUI.h"

typedef struct {
	int type;
	uint32_t address;
	char* name;
	char* description;
} SYMBOLENTRY;

class CDebugSymbols : public CDebugDialog<CDebugSymbols>
{
public:
	enum { IDD = IDD_Debugger_Symbols };

	CDebugSymbols(CDebuggerUI * debugger);
	//virtual ~CDebugScripts(void);

	CFile m_SymFileHandle;
	char* m_SymFileBuffer;

	void LoadSymbols();

	LRESULT OnDestroy(void)
	{
		return 0;
	}

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClicked(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	BEGIN_MSG_MAP_EX(CDebugSymbols)
		COMMAND_CODE_HANDLER(BN_CLICKED, OnClicked)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MSG_WM_DESTROY(OnDestroy)
	END_MSG_MAP()
};