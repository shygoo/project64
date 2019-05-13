#include "stdafx.h"
#include "DebuggerUI.h"

#include "DisplayListParser.h"

CDebugDisplayList::CDebugDisplayList(CDebuggerUI* debugger) :
	CDebugDialog<CDebugDisplayList>(debugger),
	m_bRefreshPending(false)
{
}

CDebugDisplayList::~CDebugDisplayList()
{
}

LRESULT CDebugDisplayList::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	DlgResize_Init(false, true);
	DlgToolTip_Init();
	DlgSavePos_Init(DebuggerUI_DisplayListPos);

	m_DisplayListCtrl.Attach(GetDlgItem(IDC_LST_DLIST));

	m_DisplayListCtrl.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);

	m_DisplayListCtrl.AddColumn("Address", DisplayListCtrl_Col_PAddr);
	m_DisplayListCtrl.AddColumn("SegOffset", DisplayListCtrl_Col_SegOffset);
	m_DisplayListCtrl.AddColumn("Raw Command", DisplayListCtrl_Col_RawCommand);
	m_DisplayListCtrl.AddColumn("Command", DisplayListCtrl_Col_Command);
	m_DisplayListCtrl.AddColumn("Parameters", DisplayListCtrl_Col_Parameters);

	m_DisplayListCtrl.SetColumnWidth(DisplayListCtrl_Col_SegOffset, 70);
	m_DisplayListCtrl.SetColumnWidth(DisplayListCtrl_Col_PAddr, 70);
	m_DisplayListCtrl.SetColumnWidth(DisplayListCtrl_Col_RawCommand, 120);
	m_DisplayListCtrl.SetColumnWidth(DisplayListCtrl_Col_Command, 140);
	m_DisplayListCtrl.SetColumnWidth(DisplayListCtrl_Col_Parameters, 250);

	LoadWindowPos();
	WindowCreated();
	return TRUE;
}

LRESULT CDebugDisplayList::OnDestroy(void)
{
	m_DisplayListCtrl.Detach();
	return 0;
}

LRESULT CDebugDisplayList::OnClicked(WORD wNotifyCode, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled)
{
	switch (wID)
	{
	case IDOK:
		EndDialog(0);
		break;
	case IDCANCEL:
		EndDialog(0);
		break;
	case IDC_BTN_REFRESH:
		m_bRefreshPending = true;
		SetWindowText("Display List (Waiting for RSP task...)");
		break;
	}

	return FALSE;
}

// called from RunRSP when task type == 1
void CDebugDisplayList::Refresh(void)
{
	if (m_hWnd == NULL || g_MMU == NULL || m_bRefreshPending == false)
	{
		return;
	}

	uint32_t ucodeAddr, dlistAddr, dlistSize;

	g_MMU->LW_VAddr(0xA4000FD0, ucodeAddr);
	g_MMU->LW_VAddr(0xA4000FF0, dlistAddr);
	g_MMU->LW_VAddr(0xA4000FF4, dlistSize);

	CDisplayListParser dlistParser(ucodeAddr, dlistAddr, dlistSize);
	ucode_version_t ucodeVersion = dlistParser.GetUCodeVersion();

	if (ucodeVersion != UCODE_UNKNOWN)
	{
		SetWindowText(stdstr_f("Display List - %s", dlistParser.GetUCodeName()).c_str());
	}
	else
	{
		SetWindowText(stdstr_f("Display List - Unknown microcode (%08X)", dlistParser.GetUCodeChecksum()).c_str());
	}

	m_DisplayListCtrl.SetRedraw(FALSE);
	m_DisplayListCtrl.DeleteAllItems();
	
	int numCommands = dlistParser.GetCommandCount();

	for (int i = 0; i < numCommands; i++)
	{
		hl_state_t *state = dlistParser.GetLogState(i);
	
		uint32_t physAddress = CDisplayListParser::SegmentedToPhysical(state, state->address);

		const char* commandName;
		char commandParams[512];

		commandName = dlistParser.DecodeCommand(i, commandParams);

		stdstr strPhysAddress = stdstr_f("%08X", physAddress);
		stdstr strAddress = stdstr_f("%08X", state->address);
		stdstr strRawCommand = stdstr_f("%08X %08X", state->command.w0, state->command.w1);
		stdstr strCommandNameTabbed = stdstr_f("%*s%s", state->stackIndex, "", commandName);
	
		m_DisplayListCtrl.AddItem(i, DisplayListCtrl_Col_PAddr, strPhysAddress.c_str());
		m_DisplayListCtrl.AddItem(i, DisplayListCtrl_Col_SegOffset, strAddress.c_str());
		m_DisplayListCtrl.AddItem(i, DisplayListCtrl_Col_RawCommand, strRawCommand.c_str());
		m_DisplayListCtrl.AddItem(i, DisplayListCtrl_Col_Command, strCommandNameTabbed.c_str());
		m_DisplayListCtrl.AddItem(i, DisplayListCtrl_Col_Parameters, commandParams);
	}

	m_DisplayListCtrl.SetRedraw(TRUE);

	m_bRefreshPending = false;
}

