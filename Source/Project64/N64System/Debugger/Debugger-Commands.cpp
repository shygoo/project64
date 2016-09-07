#include "stdafx.h"

#include <Project64-core/NetDebug.h>
#include <Project64/UserInterface/resource.h>

#include "DebuggerUI.h"

CDebugCommandsView::CDebugCommandsView(CDebuggerUI * debugger) :
CDebugDialog<CDebugCommandsView>(debugger)
{

}

CDebugCommandsView::~CDebugCommandsView(void)
{

}

void CDebugCommandsView::ShowAddress(DWORD Address, bool VAddr)
{
}

LRESULT	CDebugCommandsView::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	WindowCreated();
	return TRUE;
}

LRESULT CDebugCommandsView::OnDestroy(void)
{
	return 0;
}

LRESULT CDebugCommandsView::OnClicked(WORD wNotifyCode, WORD wID, HWND, BOOL & bHandled)
{
	switch (wID)
	{
	case IDC_CMD_BPTEST:
		dbgEBPAdd(0x802CB1C0);
		break;
	case IDC_CMD_BPTEST2:
		dbgEBPClear();
		break;
	case IDC_CMD_BTN_GO:
		dbgUnpause();
		break;
	case IDCANCEL:
		EndDialog(0);
		break;
	}
	return FALSE;
}

