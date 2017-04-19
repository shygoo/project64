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
#include "DebuggerUI.h"

#include "Symbols.h"
#include "Breakpoints.h"
#include "Assembler.h"

#include <Project64-core/N64System/Mips/OpCodeName.h>


CDebugCommandsView::CDebugCommandsView(CDebuggerUI * debugger) :
CDebugDialog<CDebugCommandsView>(debugger)
{
	m_HistoryIndex = -1;
	m_bIgnoreAddrChange = false;
	m_StartAddress = 0x80000000;
	m_Breakpoints = m_Debugger->Breakpoints();
	m_bEditing = false;
}

CDebugCommandsView::~CDebugCommandsView(void)
{

}

LRESULT	CDebugCommandsView::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	DlgResize_Init(false, true);

	//m_ptMinTrackSize.x = 580;
	//m_ptMinTrackSize.y = 495;

	m_CommandListRows = 33;
	
	CheckCPUType();
	
	GetWindowRect(&m_DefaultWindowRect);

	// Setup address input

	m_AddressEdit.Attach(GetDlgItem(IDC_ADDR_EDIT));
	m_AddressEdit.SetDisplayType(CEditNumber::DisplayHex);
	m_AddressEdit.SetLimitText(8);

	// Setup PC register input

	m_PCEdit.Attach(GetDlgItem(IDC_PC_EDIT));
	m_PCEdit.SetDisplayType(CEditNumber::DisplayHex);
	m_PCEdit.SetLimitText(8);
	m_PCEdit.SetValue(0x80000180, false, true);

	// Setup View PC button

	m_ViewPCButton.Attach(GetDlgItem(IDC_VIEWPC_BTN));
	m_ViewPCButton.EnableWindow(FALSE);

	// Setup debugging buttons

	m_StepButton.Attach(GetDlgItem(IDC_STEP_BTN));
	m_StepButton.EnableWindow(FALSE);

	m_SkipButton.Attach(GetDlgItem(IDC_SKIP_BTN));
	m_SkipButton.EnableWindow(FALSE);

	m_GoButton.Attach(GetDlgItem(IDC_GO_BTN));
	m_GoButton.EnableWindow(FALSE);

	// Setup register tabs & inputs

	m_RegisterTabs.Attach(GetDlgItem(IDC_REG_TABS));
	m_RegisterTabs.SetColorsEnabled(false);
	m_RegisterTabs.RefreshEdits();

	// Setup breakpoint list

	m_BreakpointList.Attach(GetDlgItem(IDC_BP_LIST));
	m_BreakpointList.ModifyStyle(NULL, LBS_NOTIFY);
	RefreshBreakpointList();

	// Setup list scrollbar

	m_Scrollbar.Attach(GetDlgItem(IDC_SCRL_BAR));
	m_Scrollbar.SetScrollRange(0, 100, FALSE);
	m_Scrollbar.SetScrollPos(50, TRUE);
	//m_Scrollbar.GetScrollInfo(); // todo bigger thumb size
	//m_Scrollbar.SetScrollInfo();

	// Setup history buttons
	m_BackButton.Attach(GetDlgItem(IDC_BACK_BTN));
	m_ForwardButton.Attach(GetDlgItem(IDC_FORWARD_BTN));
	ToggleHistoryButtons();

	// Setup command list
	m_CommandList.Attach(GetDlgItem(IDC_CMD_LIST));

	// Op editor
	m_OpEdit.Attach(GetDlgItem(IDC_OP_EDIT));
	m_OpEdit.SetCommandsWindow(this);
	
	m_bIgnoreAddrChange = true;
	m_AddressEdit.SetValue(0x80000000, false, true);
	ShowAddress(0x80000000, TRUE);
	
	WindowCreated();
	return TRUE;
}

LRESULT CDebugCommandsView::OnDestroy(void)
{
	return 0;
}

LRESULT	CDebugCommandsView::OnOpKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (wParam == VK_UP)
	{
		m_SelectedAddress -= 4;
		BeginOpEdit(m_SelectedAddress);
		bHandled = TRUE;
	}
	else if (wParam == VK_DOWN)
	{
		m_SelectedAddress += 4;
		BeginOpEdit(m_SelectedAddress);
		bHandled = TRUE;
	}
	else if (wParam == VK_RETURN)
	{
		int textLen = m_OpEdit.GetWindowTextLengthA();
		char text[256];
		m_OpEdit.GetWindowTextA(text, 255);
		m_OpEdit.SetWindowTextA("");
		uint32_t op;
		bool bValid = CAssembler::AssembleLine(text, &op, m_SelectedAddress);
		if (bValid)
		{
			EditOp(m_SelectedAddress, op);
			m_SelectedAddress += 4;
			BeginOpEdit(m_SelectedAddress);
		}
		bHandled = TRUE;
	}
	else if (wParam == VK_ESCAPE)
	{
		EndOpEdit();
		bHandled = TRUE;
	}
	return 1;
}

void CDebugCommandsView::CheckCPUType()
{
	CPU_TYPE cpuType;

	if (g_Settings->LoadBool(Setting_ForceInterpreterCPU))
	{
		cpuType = CPU_Interpreter;
	}
	else
	{
		cpuType = g_System->CpuType();
	}
	
	if (cpuType != CPU_Interpreter)
	{
		MessageBox("Interpreter mode required", "Invalid CPU Type", MB_OK);
	}
}

// Check if KSEG0 addr is out of bounds
bool CDebugCommandsView::AddressSafe(uint32_t vaddr)
{
	if (g_MMU == NULL)
	{
		return false;
	}

	if (vaddr >= 0x80000000 && vaddr <= 0x9FFFFFFF)
	{
		if ((vaddr & 0x1FFFFFFF) >= g_MMU->RdramSize())
		{
			return false;
		}
	}

	return true;
}

void CDebugCommandsView::ClearBranchArrows()
{
	m_BranchArrows.clear();
}

void CDebugCommandsView::AddBranchArrow(int startPos, int endPos)
{
	int startMargin = 0;
	int endMargin = 0;
	int margin = 0;

	for (int j = 0; j < m_BranchArrows.size(); j++)
	{
		BRANCHARROW arrow = m_BranchArrows[j];

		// Arrow's start or end pos within another arrow's stride
		if ((startPos >= arrow.startPos && startPos <= arrow.endPos) ||
			(endPos >= arrow.startPos && endPos <= arrow.endPos) ||
			(arrow.startPos <= startPos && arrow.startPos >= endPos))
		{
			if (margin <= arrow.margin)
			{
				margin = arrow.margin + 1;
			}
		}

		if (startPos == arrow.startPos)
		{
			startMargin = arrow.startMargin + 1;
		}

		if (startPos == arrow.endPos)
		{
			startMargin = arrow.endMargin + 1;
		}

		if (endPos == arrow.startPos)
		{
			endMargin = arrow.startMargin + 1;
		}

		if (endPos == arrow.endPos)
		{
			endMargin = arrow.endMargin + 1;
		}
	}

	m_BranchArrows.push_back({ startPos, endPos, startMargin, endMargin, margin });
}

static inline bool OpIsBranch(OPCODE opCode)
{
	uint32_t op = opCode.op;

	if (op >= R4300i_BEQ && op <= R4300i_BGTZ)
	{
		return true;
	}

	if (op >= R4300i_BEQL && op <= R4300i_BGTZL)
	{
		return true;
	}

	if (op == R4300i_REGIMM)
	{
		uint32_t rt = opCode.rt;

		if (rt >= R4300i_REGIMM_BLTZ && rt <= R4300i_REGIMM_BGEZL)
		{
			return true;
		}

		if (rt >= R4300i_REGIMM_BLTZAL && rt <= R4300i_REGIMM_BGEZALL)
		{
			return true;
		}
	}

	if (op == R4300i_CP1 && opCode.fmt == R4300i_COP1_BC)
	{
		return true;
	}

	return false;
}

static inline bool OpIsJump(OPCODE opCode)
{
	// j, jal, jr, jalr, exception

	uint32_t op = opCode.op;
	
	if (op == R4300i_J || op == R4300i_JAL)
	{
		return true;
	}
	
	if (op == R4300i_SPECIAL)
	{
		uint32_t fn = opCode.funct;

		if (fn >= R4300i_SPECIAL_JR && fn <= R4300i_SPECIAL_BREAK)
		{
			return true;
		}
	}

	if (op == R4300i_REGIMM)
	{
		uint32_t rt = opCode.rt;

		if (rt >= R4300i_REGIMM_TGEI && rt <= R4300i_REGIMM_TNEI)
		{
			return true;
		}
	}

	if (op == R4300i_CP0)
	{
		if ((opCode.rs & 0x10) != 0)
		{
			uint32_t fn = opCode.funct;
			if (fn >= R4300i_COP0_CO_TLBR && fn <= R4300i_COP0_CO_ERET)
			{
				return true;
			}
		}
	}

	return false;
}

void CDebugCommandsView::ShowAddress(DWORD address, BOOL top)
{
	if (top == TRUE)
	{
		m_StartAddress = address;

		if (!m_Breakpoints->isDebugging())
		{
			// Disable buttons
			m_ViewPCButton.EnableWindow(FALSE);
			m_StepButton.EnableWindow(FALSE);
			m_SkipButton.EnableWindow(FALSE);
			m_GoButton.EnableWindow(FALSE);

			m_RegisterTabs.SetColorsEnabled(false);
		}
	}
	else
	{
		bool bOutOfView = address < m_StartAddress ||
			address > m_StartAddress + (m_CommandListRows - 1) * 4;

		if (bOutOfView)
		{
			m_StartAddress = address;
			m_bIgnoreAddrChange = true;
			m_AddressEdit.SetValue(address, false, true);
		}

		if (m_History.size() == 0 || m_History[m_HistoryIndex] != m_StartAddress)
		{
			m_History.push_back(m_StartAddress);
			m_HistoryIndex = m_History.size() - 1;
			ToggleHistoryButtons();
		}
		
		m_bIgnorePCChange = true;
		m_PCEdit.SetValue(g_Reg->m_PROGRAM_COUNTER, false, true);

		// Enable buttons
		m_ViewPCButton.EnableWindow(TRUE);
		m_StepButton.EnableWindow(TRUE);
		m_SkipButton.EnableWindow(TRUE);
		m_GoButton.EnableWindow(TRUE);

		m_RegisterTabs.SetColorsEnabled(true);
	}
	
	m_CommandList.SetRedraw(FALSE);
	m_CommandList.DeleteAllItems();
	
	ClearBranchArrows();
	
	CSymbols::EnterCriticalSection();

	for (int i = 0; i < m_CommandListRows; i++)
	{	
		uint32_t opAddr = m_StartAddress + i * 4;

		m_CommandList.AddItem(i, CCommandList::COL_ARROWS, " ");

		char addrStr[9];
		sprintf(addrStr, "%08X", opAddr);
		
		m_CommandList.AddItem(i, CCommandList::COL_ADDRESS, addrStr);

		OPCODE OpCode;
		bool bAddrOkay = false;

		if (AddressSafe(opAddr))
		{
			bAddrOkay = g_MMU->LW_VAddr(opAddr, OpCode.Hex);
		}

		if(!bAddrOkay)
		{
			m_CommandList.AddItem(i, CCommandList::COL_COMMAND, "***");
			continue;
		}
		
		char* command = (char*)R4300iOpcodeName(OpCode.Hex, opAddr);
		char* cmdName = strtok((char*)command, "\t");
		char* cmdArgs = strtok(NULL, "\t");
		
		// Show subroutine symbol name for JAL target
		if (OpCode.op == R4300i_JAL)
		{
			uint32_t targetAddr = (0x80000000 | (OpCode.target << 2));

			// todo move symbols management to CDebuggerUI
			const char* targetSymbolName = CSymbols::GetNameByAddress(targetAddr);
			if (targetSymbolName != NULL)
			{
				cmdArgs = (char*)targetSymbolName;
			}
		}
		
		m_CommandList.AddItem(i, CCommandList::COL_COMMAND, cmdName);
		m_CommandList.AddItem(i, CCommandList::COL_PARAMETERS, cmdArgs);

		// Show routine symbol name for this address
		const char* routineSymbolName = CSymbols::GetNameByAddress(opAddr);
		if (routineSymbolName != NULL)
		{
			m_CommandList.AddItem(i, CCommandList::COL_SYMBOL, routineSymbolName);
		}

		// Add arrow for branch instruction
		if (OpIsBranch(OpCode))
		{
			int startPos = i;
			int endPos = startPos + (int16_t)OpCode.offset + 1;

			AddBranchArrow(startPos, endPos);
		}

		// Branch arrow for close J
		if (OpCode.op == R4300i_J)
		{
			uint32_t target = (OpCode.target << 2);
			int dist = target - (opAddr & 0x3FFFFFF);
			if (abs(dist) < 0x10000)
			{
				int startPos = i;
				int endPos = startPos + (dist / 4);
				AddBranchArrow(startPos, endPos);
			}
		}
	}

	CSymbols::LeaveCriticalSection();
	
	if (!top) // update registers when called via breakpoint/stepping
	{
		m_RegisterTabs.RefreshEdits();
	}

	RefreshBreakpointList();
	
	m_CommandList.SetRedraw(TRUE);
}

// Highlight command list items & draw branch arrows
LRESULT CDebugCommandsView::OnCustomDrawList(NMHDR* pNMHDR)
{
	NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>(pNMHDR);
	DWORD drawStage = pLVCD->nmcd.dwDrawStage;

	HDC hDC = pLVCD->nmcd.hdc;
	
	switch (drawStage)
	{
	case CDDS_PREPAINT: 
		return (CDRF_NOTIFYITEMDRAW | CDRF_NOTIFYPOSTPAINT);
	case CDDS_POSTPAINT:
		DrawBranchArrows(hDC);
		return CDRF_DODEFAULT;
	case CDDS_ITEMPREPAINT:
		return CDRF_NOTIFYSUBITEMDRAW;
	case (CDDS_ITEMPREPAINT | CDDS_SUBITEM):
		break;
	default:
		return CDRF_DODEFAULT;
	}
	
	DWORD nItem = pLVCD->nmcd.dwItemSpec;
	DWORD nSubItem = pLVCD->iSubItem;

	uint32_t address = m_StartAddress + (nItem * 4);
	uint32_t pc = (g_Reg != NULL) ? g_Reg->m_PROGRAM_COUNTER : 0;

	OPCODE pcOpcode;
	if (g_MMU != NULL)
	{
		g_MMU->LW_VAddr(pc, pcOpcode.Hex);
	}
	
	if (nSubItem == CCommandList::COL_ARROWS)
	{
		return CDRF_DODEFAULT;
	}

	if (nSubItem == CCommandList::COL_ADDRESS) // addr
	{
		if (m_Breakpoints->EBPExists(address))
		{
			// breakpoint
			pLVCD->clrTextBk = RGB(0x44, 0x00, 0x00);
			pLVCD->clrText = (address == pc) ?
				RGB(0xFF, 0xFF, 0x00) : // breakpoint & current pc
				RGB(0xFF, 0xCC, 0xCC);
		}
		else if (address == pc && m_Breakpoints->isDebugging())
		{
			// pc
			pLVCD->clrTextBk = RGB(0x88, 0x88, 0x88);
			pLVCD->clrText = RGB(0xFF, 0xFF, 0);
		}
		else
		{
			//default
			pLVCD->clrTextBk = RGB(0xEE, 0xEE, 0xEE);
			pLVCD->clrText = RGB(0x44, 0x44, 0x44);
		}

		return CDRF_DODEFAULT;
	}

	// (nSubItem == 1 || nSubItem == 2) 

	// cmd & args
	OPCODE opCode;
	bool bAddrOkay = false;

	if (AddressSafe(address))
	{
		bAddrOkay = g_MMU->LW_VAddr(address, opCode.Hex);
	}

	if (!bAddrOkay)
	{
		// unmapped/invalid
		pLVCD->clrTextBk = RGB(0xFF, 0xFF, 0xFF);
		pLVCD->clrText = RGB(0xFF, 0x00, 0x00);
	}
	else if (address == pc && m_Breakpoints->isDebugging())
	{
		// pc
		pLVCD->clrTextBk = RGB(0xFF, 0xFF, 0xAA);
		pLVCD->clrText = RGB(0x22, 0x22, 0);
	}
	else if (IsOpEdited(address))
	{
		// opcode is not original
		pLVCD->clrTextBk = RGB(0xFF, 0xEE, 0xFF);
		pLVCD->clrText = RGB(0xFF, 0x00, 0xFF);
	}
	else if (opCode.op == R4300i_ADDIU && opCode.rt == 29) // stack shift
	{
		if ((short)opCode.immediate < 0) // stack alloc
		{
			// sky blue bg, dark blue fg
			pLVCD->clrTextBk = RGB(0xCC, 0xDD, 0xFF);
			pLVCD->clrText = RGB(0x00, 0x11, 0x44);
		}
		else // stack free
		{
			// salmon bg, dark red fg
			pLVCD->clrTextBk = RGB(0xFF, 0xDD, 0xDD);
			pLVCD->clrText = RGB(0x44, 0x00, 0x00);
		}
	}
	else if (opCode.Hex == 0x00000000) // nop
	{
		// gray fg
		pLVCD->clrTextBk = RGB(0xFF, 0xFF, 0xFF);
		pLVCD->clrText = RGB(0x88, 0x88, 0x88);
	}
	else if (OpIsJump(opCode))
	{
		// jumps
		pLVCD->clrText = RGB(0x00, 0x66, 0x00);
		pLVCD->clrTextBk = RGB(0xEE, 0xFF, 0xEE);
	}
	else if (OpIsBranch(opCode))
	{
		// branches
		pLVCD->clrTextBk = RGB(0xFF, 0xFF, 0xFF);
		pLVCD->clrText = RGB(0x33, 0x77, 0x00);
	}
	else
	{
		pLVCD->clrTextBk = RGB(0xFF, 0xFF, 0xFF);
		pLVCD->clrText = RGB(0x00, 0x00, 0x00);
	}

	if (!m_Breakpoints->isDebugging())
	{
		return CDRF_DODEFAULT;
	}

	// color register usage
	// todo localise to temp register context (dont look before/after jumps and frame shifts)
	COLORREF clrUsedRegister = RGB(0xF5, 0xF0, 0xFF); // light purple
	COLORREF clrAffectedRegister = RGB(0xFF, 0xF0, 0xFF); // light pink

	int pcUsedRegA = 0, pcUsedRegB = 0, pcChangedReg = 0;
	int curUsedRegA = 0, curUsedRegB = 0, curChangedReg = 0;

	if (pcOpcode.op == R4300i_SPECIAL)
	{
		pcUsedRegA = pcOpcode.rs;
		pcUsedRegB = pcOpcode.rt;
		pcChangedReg = pcOpcode.rd;
	}
	else
	{
		pcUsedRegA = pcOpcode.rs;
		pcChangedReg = pcOpcode.rt;
	}

	if (opCode.op == R4300i_SPECIAL)
	{
		curUsedRegA = opCode.rs;
		curUsedRegB = opCode.rt;
		curChangedReg = opCode.rd;
	}
	else
	{
		curUsedRegA = opCode.rs;
		curChangedReg = opCode.rt;
	}

	if (address < pc)
	{
		if (curChangedReg != 0 && (pcUsedRegA == curChangedReg || pcUsedRegB == curChangedReg))
		{
			pLVCD->clrTextBk = clrUsedRegister;
		}
	}
	else if (address > pc)
	{
		if (pcChangedReg != 0 && (curUsedRegA == pcChangedReg || curUsedRegB == pcChangedReg))
		{
			pLVCD->clrTextBk = clrAffectedRegister;
		}
	}
	return CDRF_DODEFAULT;
}

LRESULT	CDebugCommandsView::OnMeasureItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (wParam == IDC_CMD_LIST)
	{
		MEASUREITEMSTRUCT* lpMeasureItem = (MEASUREITEMSTRUCT*)lParam;
		lpMeasureItem->itemHeight = CCommandList::ROW_HEIGHT;
	}
	return FALSE;
}

// Draw branch arrows
void CDebugCommandsView::DrawBranchArrows(HDC listDC)
{
	COLORREF colors[] = {
		RGB(240, 240, 240), // white
		RGB(30, 135, 255), // blue
		RGB(255, 0, 200), // pink
		RGB(215, 155, 0), // yellow
		RGB(100, 180, 0), // green
		RGB(200, 100, 255), // purple
		RGB(120, 120, 120), // gray
		RGB(0, 220, 160), // cyan
		RGB(255, 100, 0), // orange
		RGB(255, 255, 0), // yellow
	};
	
	int nColors = sizeof(colors) / sizeof(COLORREF);
	
	CRect listRect;
	m_CommandList.GetWindowRect(&listRect);
	ScreenToClient(&listRect);
	
	CRect headRect;
	m_CommandList.GetHeader().GetWindowRect(&headRect);
	ScreenToClient(&headRect);
	
	int colWidth = m_CommandList.GetColumnWidth(CCommandList::COL_ARROWS);

	int baseX = colWidth - 4;
	int baseY = headRect.bottom + 7;
	
	CRect paneRect;
	paneRect.top = headRect.bottom;
	paneRect.left = 0;
	paneRect.right = colWidth;
	paneRect.bottom = listRect.bottom;
	
	COLORREF bgColor = RGB(30, 30, 30);
	HBRUSH hBrushBg = CreateSolidBrush(bgColor);
	FillRect(listDC, &paneRect, hBrushBg);
	DeleteObject(hBrushBg);

	for (int i = 0; i < m_BranchArrows.size(); i++)
	{
		int colorIdx = i % nColors;
		COLORREF color = colors[colorIdx];
		
		BRANCHARROW arrow = m_BranchArrows[i];
	
		int begX = baseX - arrow.startMargin * 3;
		int endX = baseX - arrow.endMargin * 3;

		int begY = baseY + arrow.startPos * CCommandList::ROW_HEIGHT;
		int endY = baseY + arrow.endPos * CCommandList::ROW_HEIGHT;

		bool bEndVisible = true;

		if (endY < headRect.bottom)
		{
			endY = headRect.bottom + 1;
			bEndVisible = false;
		}
		else if (endY > listRect.bottom)
		{
			endY = listRect.bottom - 2;
			bEndVisible = false;
		}

		int marginX = baseX - (4 + arrow.margin * 3);

		// draw start pointer
		SetPixel(listDC, begX + 0, begY - 1, color);
		SetPixel(listDC, begX + 1, begY - 2, color);
		SetPixel(listDC, begX + 0, begY + 1, color);
		SetPixel(listDC, begX + 1, begY + 2, color);

		// draw outline
		HPEN hPenOutline = CreatePen(PS_SOLID, 3, bgColor);
		SelectObject(listDC, hPenOutline);
		MoveToEx(listDC, begX - 1, begY, NULL);
		LineTo(listDC, marginX, begY);
		LineTo(listDC, marginX, endY);
		if (bEndVisible)
		{
			LineTo(listDC, endX + 2, endY);
		}
		DeleteObject(hPenOutline);

		// draw fill line
		HPEN hPen = CreatePen(PS_SOLID, 1, color);
		SelectObject(listDC, hPen);
		MoveToEx(listDC, begX - 1, begY, NULL);
		LineTo(listDC, marginX, begY);
		LineTo(listDC, marginX, endY);
		if (bEndVisible)
		{
			LineTo(listDC, endX + 2, endY);
		}
		DeleteObject(hPen);

		// draw end pointer
		if (bEndVisible)
		{
			SetPixel(listDC, endX - 0, endY - 1, color);
			SetPixel(listDC, endX - 1, endY - 2, color);
			SetPixel(listDC, endX - 1, endY - 1, color);
			SetPixel(listDC, endX - 0, endY + 1, color);
			SetPixel(listDC, endX - 1, endY + 2, color);
			SetPixel(listDC, endX - 1, endY + 1, color);
			SetPixel(listDC, endX - 1, endY + 3, RGB(30, 30, 30));
			SetPixel(listDC, endX - 1, endY - 3, RGB(30, 30, 30));
		}
	}
}

void CDebugCommandsView::RefreshBreakpointList()
{
	m_BreakpointList.ResetContent();
	char rowStr[16];
	for (int i = 0; i < m_Breakpoints->m_nRBP; i++)
	{
		sprintf(rowStr, "R %08X", m_Breakpoints->m_RBP[i]);
		int index = m_BreakpointList.AddString(rowStr);
		m_BreakpointList.SetItemData(index, m_Breakpoints->m_RBP[i]);
	}
	for (int i = 0; i < m_Breakpoints->m_nWBP; i++)
	{
		sprintf(rowStr, "W %08X", m_Breakpoints->m_WBP[i]);
		int index = m_BreakpointList.AddString(rowStr);
		m_BreakpointList.SetItemData(index, m_Breakpoints->m_WBP[i]);
	}
	for (int i = 0; i < m_Breakpoints->m_nEBP; i++)
	{
		sprintf(rowStr, "E %08X", m_Breakpoints->m_EBP[i]);
		int index = m_BreakpointList.AddString(rowStr);
		m_BreakpointList.SetItemData(index, m_Breakpoints->m_EBP[i]);
	}
}

void CDebugCommandsView::RemoveSelectedBreakpoints()
{
	int nItem = m_BreakpointList.GetCurSel();
	
	if (nItem == LB_ERR)
	{
		return;
	}

	char itemText[32];
	m_BreakpointList.GetText(nItem, itemText);

	uint32_t address = m_BreakpointList.GetItemData(nItem);

	switch (itemText[0])
	{
	case 'E':
		m_Breakpoints->EBPRemove(address);
		break;
	case 'W':
		m_Breakpoints->WBPRemove(address);
		break;
	case 'R':
		m_Breakpoints->RBPRemove(address);
		break;
	}

	RefreshBreakpointList();
}

LRESULT CDebugCommandsView::OnClicked(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	switch (wID)
	{
	case IDC_BACK_BTN:
		if (m_HistoryIndex > 0)
		{
			m_HistoryIndex--;
			m_AddressEdit.SetValue(m_History[m_HistoryIndex], false, true);
			ToggleHistoryButtons();
		}
		break;
	case IDC_FORWARD_BTN:
		if (m_History.size() > 0 && m_HistoryIndex < m_History.size() - 1)
		{
			m_HistoryIndex++;
			m_AddressEdit.SetValue(m_History[m_HistoryIndex], false, true);
			ToggleHistoryButtons();
		}
		break;
	case IDC_VIEWPC_BTN:
		if (g_Reg != NULL && m_Breakpoints->isDebugging())
		{
			ShowAddress(g_Reg->m_PROGRAM_COUNTER, TRUE);
		}
		break;
	case IDC_SYMBOLS_BTN:
		m_Debugger->Debug_ShowSymbolsWindow();
		break;
	case IDC_GO_BTN:
		m_Debugger->Debug_RefreshStackWindow();
		m_Breakpoints->StopDebugging();
		m_Breakpoints->Resume();
		m_RegisterTabs.SetColorsEnabled(false);
		m_RegisterTabs.RefreshEdits();
		ShowAddress(m_StartAddress, TRUE);
		break;
	case IDC_STEP_BTN:
		m_Debugger->Debug_RefreshStackWindow();
		m_Breakpoints->KeepDebugging();
		m_Breakpoints->Resume();
		break;
	case IDC_SKIP_BTN:
		m_Breakpoints->KeepDebugging();
		m_Breakpoints->Skip();
		m_Breakpoints->Resume();
		break;
	case IDC_CLEARBP_BTN:
		m_Breakpoints->BPClear();
		RefreshBreakpointList();
		ShowAddress(m_StartAddress, TRUE);
		break;
	case IDC_ADDBP_BTN:
		m_AddBreakpointDlg.DoModal(m_Debugger);
		RefreshBreakpointList();
		ShowAddress(m_StartAddress, TRUE);
		break;
	case IDC_RMBP_BTN:
		RemoveSelectedBreakpoints();
		ShowAddress(m_StartAddress, TRUE);
		break;
	case IDCANCEL:
		EndDialog(0);
		break;
	//popup
	case ID_POPUPMENU_EDIT:
		BeginOpEdit(m_SelectedAddress);
		break;
	case ID_POPUPMENU_INSERTNOP:
		EditOp(m_SelectedAddress, 0x00000000);
		ShowAddress(m_StartAddress, TRUE);
		break;
	case ID_POPUPMENU_RESTORE:
		RestoreOp(m_SelectedAddress);
		ShowAddress(m_StartAddress, TRUE);
		break;
	case ID_POPUPMENU_RESTOREALL:
		RestoreAllOps();
		ShowAddress(m_StartAddress, TRUE);
		break;
	case ID_POPUPMENU_ADDSYMBOL:
		m_AddSymbolDlg.DoModal(m_Debugger, m_SelectedAddress, CSymbols::TYPE_CODE);
		break;
	}
	return FALSE;
}

LRESULT CDebugCommandsView::OnMouseWheel(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	uint32_t newAddress = m_StartAddress - ((short)HIWORD(wParam) / WHEEL_DELTA) * 4;
	
	m_StartAddress = newAddress;

	m_AddressEdit.SetValue(m_StartAddress, false, true);

	return TRUE;
}

void CDebugCommandsView::GotoEnteredAddress()
{
	char text[9];

	m_AddressEdit.GetWindowTextA(text, 9);

	DWORD address = strtoul(text, NULL, 16);
	address = address - address % 4;
	ShowAddress(address, TRUE);
}

void CDebugCommandsView::BeginOpEdit(uint32_t address)
{
	CRect listRect;
	m_CommandList.GetWindowRect(&listRect);
	ScreenToClient(&listRect);
	
	m_bEditing = true;
	//ShowAddress(address, FALSE);
	int nItem = (address - m_StartAddress) / 4;

	CRect itemRect;
	m_CommandList.GetSubItemRect(nItem, CCommandList::COL_COMMAND, 0, &itemRect);
	//itemRect.bottom += 0;
	itemRect.left += listRect.left + 3;
	itemRect.right += 100;
	
	uint32_t opcode;
	g_MMU->LW_VAddr(address, opcode);
	char* command = (char*)R4300iOpcodeName(opcode, address);

	m_OpEdit.ShowWindow(SW_SHOW);
	m_OpEdit.MoveWindow(&itemRect);
	m_OpEdit.BringWindowToTop();
	m_OpEdit.SetWindowTextA(command);
	m_OpEdit.SetFocus();
	m_OpEdit.SetSelAll();

	m_CommandList.RedrawWindow();
	m_OpEdit.RedrawWindow();
}

void CDebugCommandsView::EndOpEdit()
{
	m_bEditing = false;
	m_OpEdit.SetWindowTextA("");
	m_OpEdit.ShowWindow(SW_HIDE);
}

LRESULT CDebugCommandsView::OnAddrChanged(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (m_bIgnoreAddrChange)
	{
		m_bIgnoreAddrChange = false;
		return 0;
	}
	GotoEnteredAddress();
	return 0;
}

LRESULT CDebugCommandsView::OnPCChanged(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (m_bIgnorePCChange)
	{
		m_bIgnoreAddrChange = true;
		return 0;
	}
	if (g_Reg != NULL && m_Breakpoints->isDebugging())
	{
		g_Reg->m_PROGRAM_COUNTER = m_PCEdit.GetValue();
	}
	return 0;
}

LRESULT	CDebugCommandsView::OnCommandListClicked(NMHDR* pNMHDR)
{
	EndOpEdit();
	return 0;
}

LRESULT	CDebugCommandsView::OnCommandListDblClicked(NMHDR* pNMHDR)
{
	// Set PC breakpoint
	NMITEMACTIVATE* pIA = reinterpret_cast<NMITEMACTIVATE*>(pNMHDR);
	int nItem = pIA->iItem;
	
	uint32_t address = m_StartAddress + nItem * 4;
	if (m_Breakpoints->EBPExists(address))
	{
		m_Breakpoints->EBPRemove(address);
	}
	else
	{
		m_Breakpoints->EBPAdd(address);
	}
	// Cancel blue highlight
	m_AddressEdit.SetFocus();
	RefreshBreakpointList();

	return CDRF_DODEFAULT;
}

LRESULT	CDebugCommandsView::OnCommandListRightClicked(NMHDR* pNMHDR)
{
	EndOpEdit();

	NMITEMACTIVATE* pIA = reinterpret_cast<NMITEMACTIVATE*>(pNMHDR);
	int nItem = pIA->iItem;

	uint32_t address = m_StartAddress + nItem * 4;
	m_SelectedAddress = address;

	HMENU hMenu = LoadMenu(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_OP_POPUP));
	HMENU hPopupMenu = GetSubMenu(hMenu, 0);

	POINT mouse;
	GetCursorPos(&mouse);

	TrackPopupMenu(hPopupMenu, TPM_LEFTALIGN, mouse.x, mouse.y, 0, m_hWnd, NULL);

	DestroyMenu(hMenu);
	
	return CDRF_DODEFAULT;
}


LRESULT CDebugCommandsView::OnListBoxClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if (wID == IDC_BP_LIST)
	{
		int index = m_BreakpointList.GetCaretIndex();
		uint32_t address = m_BreakpointList.GetItemData(index);
		int len = m_BreakpointList.GetTextLen(index);
		char* rowText = (char*)malloc(len + 1);
		rowText[len] = '\0';
		m_BreakpointList.GetText(index, rowText);
		if (*rowText == 'E')
		{
			ShowAddress(address, true);
		}
		else
		{
			m_Debugger->Debug_ShowMemoryLocation(address, true);
		}
		free(rowText);
	}
	return FALSE;
}

LRESULT CDebugCommandsView::OnActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	WORD type = LOWORD(wParam);

	if (type == WA_INACTIVE)
	{
		return FALSE;
	}

	if (type == WA_CLICKACTIVE)
	{
		CheckCPUType();
	}

	ShowAddress(m_StartAddress, TRUE);

	return FALSE;
}

LRESULT	CDebugCommandsView::OnSizing(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CRect listRect;
	m_CommandList.GetWindowRect(listRect);

	CRect headRect;
	CHeaderCtrl listHead = m_CommandList.GetHeader();
	listHead.GetWindowRect(&headRect);

	int rowsHeight = listRect.Height() - headRect.Height();
	
	int nRows = (rowsHeight / CCommandList::ROW_HEIGHT);
	
	if (m_CommandListRows != nRows)
	{
		m_CommandListRows = nRows;
		ShowAddress(m_StartAddress, TRUE);
	}
	
	m_RegisterTabs.RedrawCurrentTab();

	// Fix cmd list header
	listHead.ResizeClient(listRect.Width(), headRect.Height());

	return FALSE;
}

LRESULT CDebugCommandsView::OnScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	WORD type = LOWORD(wParam);

	switch(type)
	{
	case SB_LINEUP:
		ShowAddress(m_StartAddress - 8, TRUE);
		break;
	case SB_LINEDOWN:
		ShowAddress(m_StartAddress + 8, TRUE);
		break;
	case SB_THUMBTRACK:
		{
			//int scrollPos = HIWORD(wParam);
			//ShowAddress(m_StartAddress + (scrollPos - 50) * 4, TRUE);
		}
		break;
	}

	return FALSE;
}

void CDebugCommandsView::Reset()
{
	ClearEditedOps();
	m_History.clear();
	ToggleHistoryButtons();
}

void CDebugCommandsView::ClearEditedOps()
{
	m_EditedOps.clear();
}

BOOL CDebugCommandsView::IsOpEdited(uint32_t address)
{
	for (int i = 0; i < m_EditedOps.size(); i++)
	{
		if (m_EditedOps[i].address == address)
		{
			return TRUE;
		}
	}
	return FALSE;
}

void CDebugCommandsView::EditOp(uint32_t address, uint32_t op)
{
	uint32_t currentOp;
	g_MMU->LW_VAddr(address, currentOp);

	if (currentOp == op)
	{
		return;
	}

	g_MMU->SW_VAddr(address, op);

	if (!IsOpEdited(address))
	{
		m_EditedOps.push_back({ address, currentOp });
	}

	ShowAddress(m_StartAddress, TRUE);
}

void CDebugCommandsView::RestoreOp(uint32_t address)
{
	for (int i = 0; i < m_EditedOps.size(); i++)
	{
		if (m_EditedOps[i].address == address)
		{
			g_MMU->SW_VAddr(m_EditedOps[i].address, m_EditedOps[i].originalOp);
			m_EditedOps.erase(m_EditedOps.begin() + i);
			break;
		}
	}
}

void CDebugCommandsView::RestoreAllOps()
{
	int lastIndex = m_EditedOps.size() - 1;
	for (int i = lastIndex; i >= 0; i--)
	{
		g_MMU->SW_VAddr(m_EditedOps[i].address, m_EditedOps[i].originalOp);
		m_EditedOps.erase(m_EditedOps.begin() + i);
	}
}

void CDebugCommandsView::ShowPIRegTab()
{
	m_RegisterTabs.SetCurSel(2);
	m_RegisterTabs.ShowTab(2);
}

LRESULT CDebugCommandsView::OnRegisterTabChange(NMHDR* pNMHDR)
{
	int nPage = m_RegisterTabs.GetCurSel();
	m_RegisterTabs.ShowTab(nPage);
	m_RegisterTabs.RedrawCurrentTab();
	m_RegisterTabs.RedrawWindow();
	return FALSE;
}

void CDebugCommandsView::ToggleHistoryButtons()
{
	if (m_History.size() != 0 && m_HistoryIndex > 0)
	{
		m_BackButton.EnableWindow(TRUE);
	}
	else
	{
		m_BackButton.EnableWindow(FALSE);
	}

	if (m_History.size() != 0 && m_HistoryIndex < m_History.size() - 1)
	{
		m_ForwardButton.EnableWindow(TRUE);
	}
	else
	{
		m_ForwardButton.EnableWindow(FALSE);
	}
}

// Opcode editor

LRESULT CEditOp::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (m_CommandsWindow == NULL)
	{
		return FALSE;
	}
	return m_CommandsWindow->OnOpKeyDown(uMsg, wParam, lParam, bHandled);
}

void CEditOp::SetCommandsWindow(CDebugCommandsView* commandsWindow)
{
	m_CommandsWindow = commandsWindow;
}

BOOL CEditOp::Attach(HWND hWndNew)
{
	return SubclassWindow(hWndNew);
}