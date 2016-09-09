#include "stdafx.h"

#include <Project64-core/NetDebug.h>
#include <Project64/UserInterface/resource.h>
#include <Project64-core/N64System/Mips/OpCodeName.h>

#include "DebuggerUI.h"

const int CDebugCommandsView::listLength = 30;

CDebugCommandsView::CDebugCommandsView(CDebuggerUI * debugger) :
CDebugDialog<CDebugCommandsView>(debugger)
{
	m_Address = 0x80000000;
	m_ListHighlightedItem = 2;
}

CDebugCommandsView::~CDebugCommandsView(void)
{

}

void CDebugCommandsView::ShowAddress(DWORD address, BOOL top)
{
	m_ListHighlightedItem = -1;

	if (top == TRUE || address < m_StartAddress || address > m_StartAddress + 30 * 4) {
		m_StartAddress = address;
		m_Address = m_StartAddress;
	}

	char addrStr[9];
	m_cmdList.DeleteAllItems();
	for (int i = 0; i < listLength; i++) {
		OPCODE opcode;
		OPCODE& OpCode = opcode;
		sprintf(addrStr, "%08X", m_StartAddress + i * 4);
		g_MMU->LW_VAddr(m_StartAddress + i * 4, OpCode.Hex);

		char cmdCopy[64];

		const char* command = R4300iOpcodeName(OpCode.Hex, m_StartAddress + i * 4);
		strcpy(cmdCopy, command);

		char* cmdName = strtok(cmdCopy, "\t");
		char* cmdArgs = strtok(NULL, "\t");

		m_cmdList.AddItem(i, 0, addrStr);
		m_cmdList.AddItem(i, 1, cmdName);
		m_cmdList.AddItem(i, 2, cmdArgs);

		// change this to PC
		if (m_StartAddress + i * 4 == address) {
			m_ListHighlightedItem = i;
		}

	}
	//m_cmdList.SetFocus();
	//m_cmdList.SelectItem((address - m_Address) / 4);
}

LRESULT	CDebugCommandsView::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
		m_regTabs.Attach(GetDlgItem(IDC_CMD_REGTABS));
		m_cmdList.Attach(GetDlgItem(IDC_CMD_LIST));
		
		m_regTabs.DeleteAllItems();
		m_regTabs.AddItem("GPR");
		m_regTabs.AddItem("COP1");

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
		
		//m_cmdList.SetHighlightedItem(4);

	WindowCreated();
	return TRUE;
}

LRESULT CDebugCommandsView::OnDestroy(void)
{
	return 0;
}

LRESULT CDebugCommandsView::OnCustomDrawList(NMHDR* pNMHDR) {
	NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>(pNMHDR);
	DWORD drawStage = pLVCD->nmcd.dwDrawStage;
	
	if (drawStage == CDDS_PREPAINT)
	{
		return CDRF_NOTIFYITEMDRAW;
	}

	if (drawStage == CDDS_ITEMPREPAINT)
	{
		DWORD nItem = pLVCD->nmcd.dwItemSpec;
		COLORREF clrText;
		if (nItem == m_Address + (nItem * 4))
		{
			pLVCD->clrTextBk = RGB(0xDD, 0xDD, 0xDD);
			pLVCD->clrText = RGB(0xFF, 0, 0);
		}
	}

	return CDRF_DODEFAULT;
}

LRESULT CDebugCommandsView::OnClicked(WORD wNotifyCode, WORD wID, HWND, BOOL & bHandled)
{
	switch (wID)
	{
	case IDC_CMD_BPTEST:
		dbgEBPAdd(0x802CB1C0);
		break;
	case IDC_CMD_BTN_BPCLEAR:
		dbgEBPClear();
		break;
	case IDC_CMD_BTN_GO:
		dbgUnpause();
		break;
	case IDC_CMD_BTN_STEP:
		dbgPauseNext();
		dbgUnpause();
		ShowAddress(g_Reg->m_PROGRAM_COUNTER, TRUE);
		break;
	case IDCANCEL:
		EndDialog(0);
		break;
	}
	return FALSE;
}

LRESULT CDebugCommandsView::OnAddrChanged(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	char text[9];
	text[8] = '\0';
	GetDlgItemText(wID, text, 9);
	DWORD address = strtoul(text, NULL, 16);
	address &= 0x003FFFFF;
	address |= 0x80000000;
	address = address - address % 4;
	ShowAddress(address, TRUE);
	return 0;
}
