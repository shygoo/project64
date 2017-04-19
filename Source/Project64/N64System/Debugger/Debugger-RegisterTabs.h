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
#include "stdafx.h"

#include "Breakpoints.h"

#ifndef COUNT_OF
	#define COUNT_OF(a) (sizeof(a) / sizeof(a[0]))
#endif

class CEditReg64 : public CWindowImpl<CEditReg64, CEdit>
{
public:
	static uint64_t ParseValue(char* wordPair);
	BOOL Attach(HWND hWndNew);
	LRESULT OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnLostFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	uint64_t GetValue();
	void SetValue(uint32_t h, uint32_t l);
	void SetValue(uint64_t value);

	BEGIN_MSG_MAP_EX(CRegEdit64)
		MESSAGE_HANDLER(WM_CHAR, OnChar)
		MESSAGE_HANDLER(WM_KILLFOCUS, OnLostFocus)
	END_MSG_MAP()
};

class CRegisterTabs : public CTabCtrl
{
private:
	// for static dlgprocs, assumes single instance
	static bool m_bColorsEnabled; 

	vector<CWindow> m_TabWindows;
	
	static constexpr DWORD GPREditIds[] = {
		IDC_R0_EDIT,  IDC_R1_EDIT,  IDC_R2_EDIT,  IDC_R3_EDIT,
		IDC_R4_EDIT,  IDC_R5_EDIT,  IDC_R6_EDIT,  IDC_R7_EDIT,
		IDC_R8_EDIT,  IDC_R9_EDIT,  IDC_R10_EDIT, IDC_R11_EDIT,
		IDC_R12_EDIT, IDC_R13_EDIT, IDC_R14_EDIT, IDC_R15_EDIT,
		IDC_R16_EDIT, IDC_R17_EDIT, IDC_R18_EDIT, IDC_R19_EDIT,
		IDC_R20_EDIT, IDC_R21_EDIT, IDC_R22_EDIT, IDC_R23_EDIT,
		IDC_R24_EDIT, IDC_R25_EDIT, IDC_R26_EDIT, IDC_R27_EDIT,
		IDC_R28_EDIT, IDC_R29_EDIT, IDC_R30_EDIT, IDC_R31_EDIT,
		0
	};

	static constexpr DWORD FPREditIds[] = {
		IDC_F0_EDIT,  IDC_F1_EDIT,  IDC_F2_EDIT,  IDC_F3_EDIT,
		IDC_F4_EDIT,  IDC_F5_EDIT,  IDC_F6_EDIT,  IDC_F7_EDIT,
		IDC_F8_EDIT,  IDC_F9_EDIT,  IDC_F10_EDIT, IDC_F11_EDIT,
		IDC_F12_EDIT, IDC_F13_EDIT, IDC_F14_EDIT, IDC_F15_EDIT,
		IDC_F16_EDIT, IDC_F17_EDIT, IDC_F18_EDIT, IDC_F19_EDIT,
		IDC_F20_EDIT, IDC_F21_EDIT, IDC_F22_EDIT, IDC_F23_EDIT,
		IDC_F24_EDIT, IDC_F25_EDIT, IDC_F26_EDIT, IDC_F27_EDIT,
		IDC_F28_EDIT, IDC_F29_EDIT, IDC_F30_EDIT, IDC_F31_EDIT,
		0
	};

	static constexpr DWORD PIEditIds[] = {
		IDC_PI00_EDIT, IDC_PI04_EDIT, IDC_PI08_EDIT, IDC_PI0C_EDIT,
		IDC_PI10_EDIT, IDC_PI14_EDIT, IDC_PI18_EDIT, IDC_PI1C_EDIT,
		IDC_PI20_EDIT, IDC_PI24_EDIT, IDC_PI28_EDIT, IDC_PI2C_EDIT,
		IDC_PI30_EDIT,
		0
	};

	static constexpr DWORD COP0EditIds[] = {
		IDC_COP0_0_EDIT,  IDC_COP0_1_EDIT,  IDC_COP0_2_EDIT,  IDC_COP0_3_EDIT,
		IDC_COP0_4_EDIT,  IDC_COP0_5_EDIT,  IDC_COP0_6_EDIT,  IDC_COP0_7_EDIT,
		IDC_COP0_8_EDIT,  IDC_COP0_9_EDIT,  IDC_COP0_10_EDIT, IDC_COP0_11_EDIT,
		IDC_COP0_12_EDIT, IDC_COP0_13_EDIT, IDC_COP0_14_EDIT, IDC_COP0_15_EDIT,
		IDC_COP0_16_EDIT, IDC_COP0_17_EDIT, IDC_COP0_18_EDIT,
		0
	};

	static int MapEdit(DWORD controlId, const DWORD* edits)
	{
		for (int i = 0; edits[i] != 0; i++)
		{
			if (edits[i] == controlId)
			{
				return i;
			}
		}
		return -1;
	}
	
	CWindow m_PITab;
	CEditNumber m_PIEdits[COUNT_OF(PIEditIds) - 1];
	
	CWindow m_GPRTab;
	CEditReg64 m_GPREdits[COUNT_OF(GPREditIds) - 1];
	CEditReg64 m_HIEdit;
	CEditReg64 m_LOEdit;
	
	CWindow m_FPRTab;
	CEditNumber m_FPREdits[COUNT_OF(FPREditIds) - 1];
	
	CWindow m_COP0Tab;
	CEditNumber m_COP0Edits[COUNT_OF(COP0EditIds) - 1];

public:
	void Attach(HWND hWndNew);
	CWindow AddTab(char* caption, int dialogId, DLGPROC dlgProc);
	void ShowTab(int nPage);
	CRect GetPageRect();
	void RedrawCurrentTab();

	void RefreshEdits();
	
	void SetColorsEnabled(bool bColorsEnabled);

	static INT_PTR CALLBACK TabProcGPR(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK TabProcFPR(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK TabProcPI(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK TabProcCOP0(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	
	//LRESULT OnSelChange(NMHDR* lpNMHDR)
	//
	//BEGIN_MSG_MAP_EX(CRegisterTabs)
	//	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	//	//REFLECTED_NOTIFY_CODE_HANDLER_EX(TCN_SELCHANGE, OnSelChange)
	//	//DEFAULT_REFLECTION_HANDLER()
	//END_MSG_MAP()
};