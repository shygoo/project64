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

#include "Debugger-RegisterTabs.h"

void CRegisterTabs::Attach(HWND hWndNew)
{
	m_TabWindows.clear();

	CTabCtrl::Attach(hWndNew);

	m_GPRTab = AddTab("GPR", IDD_Debugger_GPR, TabProcGPR);
	m_FPRTab = AddTab("FPR", IDD_Debugger_FPR, TabProcFPR);
	m_COP0Tab = AddTab("COP0", IDD_Debugger_COP0, TabProcCOP0);
	m_PITab = AddTab("PI", IDD_Debugger_PI, TabProcPI);
	
	HFONT monoFont = CreateFont(-11, 0, 0, 0,
		FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		CLEARTYPE_QUALITY, FF_DONTCARE, "Consolas"
	);

	for (int i = 0; GPREditIds[i] != 0; i++)
	{
		m_GPREdits[i].Attach(m_GPRTab.GetDlgItem(GPREditIds[i]));
		m_GPREdits[i].SetFont(monoFont, FALSE);

		m_FPREdits[i].Attach(m_FPRTab.GetDlgItem(FPREditIds[i]));
		m_FPREdits[i].SetDisplayType(CEditNumber::DisplayHex);
		m_FPREdits[i].SetFont(monoFont, FALSE);
	}

	for (int i = 0; PIEditIds[i] != 0; i++)
	{
		m_PIEdits[i].Attach(m_PITab.GetDlgItem(PIEditIds[i]));
		m_PIEdits[i].SetDisplayType(CEditNumber::DisplayHex);
		m_PIEdits[i].SetFont(monoFont, FALSE);
	}

	for (int i = 0; COP0EditIds[i] != 0; i++)
	{
		m_COP0Edits[i].Attach(m_COP0Tab.GetDlgItem(COP0EditIds[i]));
		m_COP0Edits[i].SetDisplayType(CEditNumber::DisplayHex);
		m_COP0Edits[i].SetFont(monoFont, FALSE);
	}

	m_HIEdit.Attach(m_GPRTab.GetDlgItem(IDC_HI_EDIT));
	m_HIEdit.SetFont(monoFont, FALSE);

	m_LOEdit.Attach(m_GPRTab.GetDlgItem(IDC_LO_EDIT));
	m_LOEdit.SetFont(monoFont, FALSE);
}

CRect CRegisterTabs::GetPageRect()
{
	CWindow parentWin = GetParent();
	CRect pageRect;
	GetWindowRect(&pageRect);
	parentWin.ScreenToClient(&pageRect);
	AdjustRect(FALSE, &pageRect);
	return pageRect;
}

CWindow CRegisterTabs::AddTab(char* caption, int dialogId, DLGPROC dlgProc)
{
	AddItem(caption);

	CWindow parentWin = GetParent();
	CWindow tabWin = ::CreateDialog(NULL, MAKEINTRESOURCE(dialogId), parentWin, dlgProc);

	CRect pageRect = GetPageRect();

	::SetParent(tabWin, parentWin);

	::SetWindowPos(
		tabWin,
		m_hWnd,
		pageRect.left,
		pageRect.top,
		pageRect.Width(),
		pageRect.Height(),
		SWP_HIDEWINDOW
	);

	m_TabWindows.push_back(tabWin);

	int index = m_TabWindows.size() - 1;

	if (index == 0)
	{
		ShowTab(0);
	}

	return tabWin;
}

void CRegisterTabs::ShowTab(int nPage)
{
	for (int i = 0; i < m_TabWindows.size(); i++)
	{
		::ShowWindow(m_TabWindows[i], SW_HIDE);
	}
	
	CRect pageRect = GetPageRect();

	::SetWindowPos(
		m_TabWindows[nPage],
		m_hWnd,
		pageRect.left,
		pageRect.top,
		pageRect.Width(),
		pageRect.Height(),
		SWP_SHOWWINDOW
	);
}

void CRegisterTabs::RedrawCurrentTab()
{
	int nPage = GetCurSel();
	CRect pageRect = GetPageRect();

	::SetWindowPos(
		m_TabWindows[nPage],
		m_hWnd,
		pageRect.left,
		pageRect.top,
		pageRect.Width(),
		pageRect.Height(),
		SWP_SHOWWINDOW
	);
}

void CRegisterTabs::RefreshEdits()
{
	if (g_Reg != NULL)
	{
		char regText[9];
		for (int i = 0; i < 32; i++)
		{
			m_GPREdits[i].SetValue(g_Reg->m_GPR[i].UDW);
			m_FPREdits[i].SetValue(*(uint32_t *)g_Reg->m_FPR_S[i], false, true);
		}
		m_HIEdit.SetValue(g_Reg->m_HI.UDW);
		m_LOEdit.SetValue(g_Reg->m_LO.UDW);

		m_PIEdits[0].SetValue(g_Reg->PI_DRAM_ADDR_REG, false, true);
		m_PIEdits[1].SetValue(g_Reg->PI_CART_ADDR_REG, false, true);
		m_PIEdits[2].SetValue(g_Reg->PI_RD_LEN_REG, false, true);
		m_PIEdits[3].SetValue(g_Reg->PI_WR_LEN_REG, false, true);
		m_PIEdits[4].SetValue(g_Reg->PI_STATUS_REG, false, true);
		m_PIEdits[5].SetValue(g_Reg->PI_BSD_DOM1_LAT_REG, false, true);
		m_PIEdits[6].SetValue(g_Reg->PI_BSD_DOM1_PWD_REG, false, true);
		m_PIEdits[7].SetValue(g_Reg->PI_BSD_DOM1_PGS_REG, false, true);
		m_PIEdits[8].SetValue(g_Reg->PI_BSD_DOM1_RLS_REG, false, true);
		m_PIEdits[9].SetValue(g_Reg->PI_BSD_DOM2_LAT_REG, false, true);
		m_PIEdits[10].SetValue(g_Reg->PI_BSD_DOM2_PWD_REG, false, true);
		m_PIEdits[11].SetValue(g_Reg->PI_BSD_DOM2_PGS_REG, false, true);
		m_PIEdits[12].SetValue(g_Reg->PI_BSD_DOM2_RLS_REG, false, true);

		m_COP0Edits[0].SetValue(g_Reg->INDEX_REGISTER, false, true);
		m_COP0Edits[1].SetValue(g_Reg->RANDOM_REGISTER, false, true);
		m_COP0Edits[2].SetValue(g_Reg->ENTRYLO0_REGISTER, false, true);
		m_COP0Edits[3].SetValue(g_Reg->ENTRYLO1_REGISTER, false, true);
		m_COP0Edits[4].SetValue(g_Reg->CONTEXT_REGISTER, false, true);
		m_COP0Edits[5].SetValue(g_Reg->PAGE_MASK_REGISTER, false, true);
		m_COP0Edits[6].SetValue(g_Reg->WIRED_REGISTER, false, true);
		m_COP0Edits[7].SetValue(g_Reg->BAD_VADDR_REGISTER, false, true);
		m_COP0Edits[8].SetValue(g_Reg->COUNT_REGISTER, false, true);
		m_COP0Edits[9].SetValue(g_Reg->ENTRYHI_REGISTER, false, true);
		m_COP0Edits[10].SetValue(g_Reg->COMPARE_REGISTER, false, true);
		m_COP0Edits[11].SetValue(g_Reg->STATUS_REGISTER, false, true);
		m_COP0Edits[12].SetValue(g_Reg->CAUSE_REGISTER, false, true);
		m_COP0Edits[13].SetValue(g_Reg->EPC_REGISTER, false, true);
		m_COP0Edits[14].SetValue(g_Reg->CONFIG_REGISTER, false, true);
		m_COP0Edits[15].SetValue(g_Reg->TAGLO_REGISTER, false, true);
		m_COP0Edits[16].SetValue(g_Reg->TAGHI_REGISTER, false, true);
		m_COP0Edits[17].SetValue(g_Reg->ERROREPC_REGISTER, false, true);
		m_COP0Edits[18].SetValue(g_Reg->FAKE_CAUSE_REGISTER, false, true);
	}
	else
	{
		for (int i = 0; i < 32; i++)
		{
			m_GPREdits[i].SetValue(0);
			m_FPREdits[i].SetWindowTextA("00000000");
		}
		m_HIEdit.SetValue(0);
		m_LOEdit.SetValue(0);

		for (int i = 0; i < 13; i++)
		{
			m_PIEdits[i].SetValue(0, false, true);
		}

		for (int i = 0; i < 19; i++)
		{
			m_COP0Edits[i].SetValue(0, false, true);
		}
	}
}

INT_PTR CALLBACK CRegisterTabs::TabProcGPR(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{

	if (msg == WM_INITDIALOG)
	{
		return TRUE;
	}

	/*
	if (msg == WM_CTLCOLOREDIT)
	{
	if (g_Reg == NULL)
	{
	return FALSE;
	}

	HDC hdc = (HDC)wParam;

	SetTextColor(hdc, signExt ? RGB(255, 0, 0) : RGB(0, 0, 0));
	SetBkColor(hdc, RGB(255,255,255));

	return (INT_PTR)CreateSolidBrush(RGB(255,255,255));
	}*/

	/*if (registersUpdating)
	{
	return FALSE;
	}*/

	if (msg != WM_COMMAND)
	{
		return FALSE;
	}

	WORD notification = HIWORD(wParam);

	if (notification == EN_KILLFOCUS)
	{
		CBreakpoints* breakpoints = ((CDebuggerUI*)g_Debugger)->Breakpoints();
		if (g_Reg == NULL || !breakpoints->isDebugging())
		{
			return FALSE;
		}

		int ctrlId = LOWORD(wParam);

		// (if IDC_PC_EDIT here with early ret)

		HWND ctrl = ::GetDlgItem(hDlg, ctrlId);
		char text[20];
		::GetWindowText(ctrl, text, 20);

		uint64_t value = CEditReg64::ParseValue(text);

		if (ctrlId == IDC_HI_EDIT)
		{
			g_Reg->m_HI.UDW = value;
		}
		else if (ctrlId == IDC_LO_EDIT)
		{
			g_Reg->m_HI.UDW = value;
		}
		else
		{
			int nReg = MapEdit(ctrlId, GPREditIds);
			g_Reg->m_GPR[nReg].UDW = value;
		}
	}

	return FALSE;
}

INT_PTR CALLBACK CRegisterTabs::TabProcFPR(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_INITDIALOG)
	{
		return TRUE;
	}
	if (msg != WM_COMMAND)
	{
		return FALSE;
	}

	WORD notification = HIWORD(wParam);

	if (notification == EN_KILLFOCUS)
	{

		WORD controlID = LOWORD(wParam);
		char regText[9];
		CWindow edit = ::GetDlgItem(hDlg, controlID);
		edit.GetWindowTextA(regText, 9);
		uint32_t value = strtoul(regText, NULL, 16);
		sprintf(regText, "%08X", value);
		edit.SetWindowTextA(regText);

		CBreakpoints* breakpoints = ((CDebuggerUI*)g_Debugger)->Breakpoints();
		if (g_Reg == NULL || !breakpoints->isDebugging())
		{
			return FALSE;
		}

		int nReg = MapEdit(controlID, FPREditIds);

		*(uint32_t*)g_Reg->m_FPR_S[nReg] = value;
	}

	return FALSE;
}

INT_PTR CALLBACK CRegisterTabs::TabProcPI(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_INITDIALOG)
	{
		return TRUE;
	}
	if (msg != WM_COMMAND)
	{
		return FALSE;
	}

	WORD notification = HIWORD(wParam);

	if (notification == EN_KILLFOCUS)
	{

		WORD controlID = LOWORD(wParam);
		char regText[9];
		CWindow edit = ::GetDlgItem(hDlg, controlID);
		edit.GetWindowTextA(regText, 9);
		uint32_t value = strtoul(regText, NULL, 16);
		sprintf(regText, "%08X", value);
		edit.SetWindowTextA(regText);

		CBreakpoints* breakpoints = ((CDebuggerUI*)g_Debugger)->Breakpoints();
		if (g_MMU == NULL || !breakpoints->isDebugging())
		{
			return FALSE;
		}

		int nReg = MapEdit(controlID, PIEditIds);

		switch (nReg)
		{
		case 0:  g_Reg->PI_DRAM_ADDR_REG = value; break;
		case 1:  g_Reg->PI_CART_ADDR_REG = value; break;
		case 2:  g_Reg->PI_RD_LEN_REG = value; break;
		case 3:  g_Reg->PI_WR_LEN_REG = value; break;
		case 4:  g_Reg->PI_STATUS_REG = value; break;
		case 5:  g_Reg->PI_BSD_DOM1_LAT_REG = value; break;
		case 6:  g_Reg->PI_BSD_DOM1_PWD_REG = value; break;
		case 7:  g_Reg->PI_BSD_DOM1_PGS_REG = value; break;
		case 8:  g_Reg->PI_BSD_DOM1_RLS_REG = value; break;
		case 9:  g_Reg->PI_BSD_DOM2_LAT_REG = value; break;
		case 10: g_Reg->PI_BSD_DOM2_PWD_REG = value; break;
		case 11: g_Reg->PI_BSD_DOM2_PGS_REG = value; break;
		case 12: g_Reg->PI_BSD_DOM2_RLS_REG = value; break;
		}
	}
	return FALSE;
}

INT_PTR CALLBACK CRegisterTabs::TabProcCOP0(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_INITDIALOG)
	{
		return TRUE;
	}
	if (msg != WM_COMMAND)
	{
		return FALSE;
	}

	WORD notification = HIWORD(wParam);

	if (notification != EN_KILLFOCUS)
	{
		return FALSE;
	}

	WORD controlID = LOWORD(wParam);
	char regText[9];
	CWindow edit = ::GetDlgItem(hDlg, controlID);
	edit.GetWindowTextA(regText, 9);
	uint32_t value = strtoul(regText, NULL, 16);
	sprintf(regText, "%08X", value);
	edit.SetWindowTextA(regText);

	CBreakpoints* breakpoints = ((CDebuggerUI*)g_Debugger)->Breakpoints();
	if (g_MMU == NULL || !breakpoints->isDebugging())
	{
		return FALSE;
	}

	int nReg = MapEdit(controlID, COP0EditIds);

	switch (nReg)
	{
	case 0:  g_Reg->INDEX_REGISTER = value; break;
	case 1:  g_Reg->RANDOM_REGISTER = value; break;
	case 2:  g_Reg->ENTRYLO0_REGISTER = value; break;
	case 3:  g_Reg->ENTRYLO1_REGISTER = value; break;
	case 4:  g_Reg->CONTEXT_REGISTER = value; break;
	case 5:  g_Reg->PAGE_MASK_REGISTER = value; break;
	case 6:  g_Reg->WIRED_REGISTER = value; break;
	case 7:  g_Reg->BAD_VADDR_REGISTER = value; break;
	case 8:  g_Reg->COUNT_REGISTER = value; break;
	case 9:  g_Reg->ENTRYHI_REGISTER = value; break;
	case 10: g_Reg->COMPARE_REGISTER = value; break;
	case 11: g_Reg->STATUS_REGISTER = value; break;
	case 12: g_Reg->CAUSE_REGISTER = value; break;
	case 13: g_Reg->EPC_REGISTER = value; break;
	case 14: g_Reg->CONFIG_REGISTER = value; break;
	case 15: g_Reg->TAGLO_REGISTER = value; break;
	case 16: g_Reg->TAGHI_REGISTER = value; break;
	case 17: g_Reg->ERROREPC_REGISTER = value; break;
	case 18: g_Reg->FAKE_CAUSE_REGISTER = value; break;
	}

	return FALSE;
}

// CEditReg64

uint64_t CEditReg64::ParseValue(char* wordPair)
{
	uint32_t a, b;
	uint64_t ret;
	a = strtoul(wordPair, &wordPair, 16);
	if (*wordPair == ' ')
	{
		wordPair++;
		b = strtoul(wordPair, NULL, 16);
		ret = (uint64_t)a << 32;
		ret |= b;
		return ret;
	}
	return (uint64_t)a;
}

BOOL CEditReg64::Attach(HWND hWndNew)
{
	return SubclassWindow(hWndNew);
}

LRESULT CEditReg64::OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CBreakpoints* breakpoints = ((CDebuggerUI*)g_Debugger)->Breakpoints();
	if (!breakpoints->isDebugging())
	{
		goto canceled;
	}

	char charCode = wParam;

	if (!isxdigit(charCode) && charCode != ' ')
	{
		if (!isalnum(charCode))
		{
			goto unhandled;
		}
		goto canceled;
	}

	if (isalpha(charCode) && !isupper(charCode))
	{
		SendMessage(uMsg, toupper(wParam), lParam);
		goto canceled;
	}

	char text[20];
	GetWindowText(text, 20);
	int textLen = strlen(text);

	if (textLen >= 17)
	{
		int selStart, selEnd;
		GetSel(selStart, selEnd);
		if (selEnd - selStart == 0)
		{
			goto canceled;
		}
	}

	if (charCode == ' ' && strchr(text, ' ') != NULL)
	{
		goto canceled;
	}

unhandled:
	bHandled = FALSE;
	return 0;

canceled:
	bHandled = TRUE;
	return 0;
}

uint64_t CEditReg64::GetValue()
{
	char text[20];
	GetWindowText(text, 20);
	return ParseValue(text);
}

LRESULT CEditReg64::OnLostFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	SetValue(GetValue()); // clean up
	bHandled = FALSE;
	return 0;
}

void CEditReg64::SetValue(uint32_t h, uint32_t l)
{
	char text[20];
	sprintf(text, "%08X %08X", h, l);
	SetWindowText(text);
}

void CEditReg64::SetValue(uint64_t value)
{
	uint32_t h = (value & 0xFFFFFFFF00000000LL) >> 32;
	uint32_t l = (value & 0x00000000FFFFFFFFLL);
	SetValue(h, l);
}