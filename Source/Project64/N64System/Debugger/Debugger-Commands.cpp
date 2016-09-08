#include "stdafx.h"

#include <Project64-core/NetDebug.h>
#include <Project64/UserInterface/resource.h>
#include <Project64-core/N64System/Mips/OpCodeName.h>

#include "DebuggerUI.h"

CDebugCommandsView::CDebugCommandsView(CDebuggerUI * debugger) :
CDebugDialog<CDebugCommandsView>(debugger)
{

}

CDebugCommandsView::~CDebugCommandsView(void)
{

}

static DWORD curAddress = 0;

void CDebugCommandsView::ShowAddress(DWORD address)
{

	if (address < curAddress || address > curAddress + 30 * 4) {
		curAddress = address;
	}

	char addrStr[9];
	m_cmdList.DeleteAllItems();
	for (int i = 0; i < 30; i++) {
		OPCODE opcode0;
		OPCODE& OpCode = opcode0;
		sprintf(addrStr, "%08X", curAddress + i * 4);
		g_MMU->LW_VAddr(curAddress + i * 4, OpCode.Hex);
		char cmdCopy[64];
		const char* command = R4300iOpcodeName(OpCode.Hex, curAddress + i * 4);
		strcpy(cmdCopy, command);
		const char* cmdName = strtok(cmdCopy, "\t");
		const char* cmdArgs = strtok(NULL, "\t");
		m_cmdList.AddItem(i, 0, addrStr);
		m_cmdList.AddItem(i, 1, cmdName);
		m_cmdList.AddItem(i, 2, cmdArgs);
	}
	m_cmdList.SetHighlightedItem(4);
	m_cmdList.SetFocus();
	m_cmdList.SelectItem((address - curAddress) / 4);
}

LRESULT	CDebugCommandsView::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{

	m_regTabs.Attach(GetDlgItem(IDC_CMD_REGTABS));
		m_regTabs.DeleteAllItems();
		m_regTabs.AddItem("GPR");
		m_regTabs.AddItem("COP1");

	m_cmdList.Attach(GetDlgItem(IDC_CMD_LIST));
		m_cmdList.AddColumn("Addr", 0);
		m_cmdList.AddColumn("Cmd", 1);
		m_cmdList.AddColumn("Args", 2);
		m_cmdList.SetColumnWidth(0, 60);
		m_cmdList.SetColumnWidth(1, 60);
		m_cmdList.SetColumnWidth(2, 120);
		m_cmdList.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);

		for (int i = 0; i < 10; i++) {
			m_cmdList.AddItem(i, 0, "test");
			m_cmdList.AddItem(i, 1, "test");
			m_cmdList.AddItem(i, 2, "test");
		}
		
		m_cmdList.SetHighlightedItem(4);

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
	case IDC_CMD_BTN_STEP:
		dbgPauseNext();
		dbgUnpause();
		ShowAddress(g_Reg->m_PROGRAM_COUNTER);
		break;
	case IDCANCEL:
		EndDialog(0);
		break;
	}
	return FALSE;
}

// list control

DWORD CListViewCtrlDbg::OnPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/)
{
	return CDRF_NOTIFYITEMDRAW;
}

DWORD CListViewCtrlDbg::OnItemPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW lpNMCustomDraw)
{
	NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>(lpNMCustomDraw);

	COLORREF clrTextBk;
	COLORREF clrText;
	if (pLVCD->nmcd.dwItemSpec == m_highlightedItem)
	{
		clrTextBk = RGB(0xFF, 0xFF, 0xFF);
		clrText = RGB(0xFF, 0x00, 0x00);
	}
	else
	{
		clrTextBk = RGB(0xFF, 0xFF, 0xAA);
		clrText = RGB(0xFF, 0xFF, 0xFF);
	}

	pLVCD->clrText = clrText;
	pLVCD->clrTextBk = clrTextBk;

	return CDRF_DODEFAULT;

}

void CListViewCtrlDbg::SetHighlightedItem(int nItem) {
	m_highlightedItem = nItem;
}