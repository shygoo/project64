#include "stdafx.h"

#include "DebuggerUI.h"

CDebugAddBreakpoint::CDebugAddBreakpoint(CDebuggerUI* debugger) :
	CDebugDialog<CDebugAddBreakpoint>(debugger)
{

}

LRESULT CDebugAddBreakpoint::OnClicked(WORD /*wNotifyCode*/, WORD wID, HWND, BOOL& /*bHandled*/)
{
	switch (wID)
	{
	case IDOK:
	{
		char addrStr[9];
		GetDlgItemText(IDC_ADDR_EDIT, addrStr, 9);
		uint32_t address = strtoul(addrStr, NULL, 16);

		int read = ((CButton)GetDlgItem(IDC_CHK_READ)).GetCheck();
		int write = ((CButton)GetDlgItem(IDC_CHK_WRITE)).GetCheck();
		int exec = ((CButton)GetDlgItem(IDC_CHK_EXEC)).GetCheck();

		CBreakpoints* breakpoints = m_Debugger->Breakpoints();

		if (read)
		{
			breakpoints->RBPAdd(address);
		}
		if (write)
		{
			breakpoints->WBPAdd(address);
		}
		if (exec)
		{
			breakpoints->EBPAdd(address);
		}
		EndDialog(0);
		break;
	}
	case IDCANCEL:
		EndDialog(0);
		break;
	}
	return FALSE;
}

LRESULT CDebugAddBreakpoint::OnDestroy(void)
{
	return 0;
}