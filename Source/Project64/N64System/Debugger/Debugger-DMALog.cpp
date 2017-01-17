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

CDebugDMALogView::CDebugDMALogView(CDebuggerUI* debugger) :
CDebugDialog<CDebugDMALogView>(debugger)
{
	m_bFilterChanged = false;
	m_bUniqueRomAddresses = true;
}

CDebugDMALogView::~CDebugDMALogView()
{
}

bool CDebugDMALogView::FilterEntry(int dmaLogIndex)
{
	DMALogEntry entry = m_Debugger->DMALog()->at(dmaLogIndex);

	for (int i = 0; i < dmaLogIndex; i++)
	{
		DMALogEntry testEntry = m_Debugger->DMALog()->at(i);

		// Don't show if another entry has the same ROM address
		if (entry.romAddr == testEntry.romAddr)
		{
			return false;
		}
	}

	return true;
}

void CDebugDMALogView::RefreshList()
{
	if (g_Rom == NULL)
	{
		return;
	}
	
	int startIndex;
	int dmaLogSize = m_Debugger->DMALog()->size();
	
	if (dmaLogSize == 0)
	{
		// Reset
		m_DMAList.DeleteAllItems();
		startIndex = 0;
		m_bFilterChanged = false;
	}
	else
	{
		// Continue from last index
		startIndex = m_nLastStartIndex;
	}
	
	m_DMAList.SetRedraw(FALSE);

	int itemIndex = m_DMAList.GetItemCount();
	
	for (int i = startIndex; i < dmaLogSize; i++)
	{
		DMALogEntry entry = m_Debugger->DMALog()->at(i);
		
		if (!FilterEntry(i))
		{
			continue;
		}
		
		m_DMAList.AddItem(itemIndex, 0, stdstr_f("%08X", entry.romAddr).c_str());
		m_DMAList.AddItem(itemIndex, 1, stdstr_f("%08X", entry.ramAddr).c_str());
		m_DMAList.AddItem(itemIndex, 2, stdstr_f("%08X (%d)", entry.length, entry.length).c_str());

		// Get four character string at rom address
		uint8_t* rom = g_Rom->GetRomAddress();
		char sig[5]; sprintf(sig, "%.4s", &rom[entry.romAddr]);
		*(uint32_t*)sig = _byteswap_ulong(*(uint32_t*)sig);

		// Todo checkbox to display all in hex
		if (isalnum(sig[0]) && isalnum(sig[1]) && isalnum(sig[2]) && isalnum(sig[3]))
		{
			m_DMAList.AddItem(itemIndex, 4, sig);
		}

		itemIndex++;
	}
	
	m_DMAList.SetRedraw(TRUE);

	m_nLastStartIndex = dmaLogSize;
}

DWORD WINAPI CDebugDMALogView::AutoRefreshProc(void* _this)
{
	CDebugDMALogView* self = (CDebugDMALogView*)_this;
	while (true)
	{
		self->RefreshList();
		Sleep(100);
	}
}

LRESULT CDebugDMALogView::OnActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	//RefreshList();
	return FALSE;
}

LRESULT CDebugDMALogView::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	DlgResize_Init(false, false);

	m_bConvertingAddress = false;

	m_DMAList.Attach(GetDlgItem(IDC_DMA_LIST));

	m_DMAList.AddColumn("ROM", 0);
	m_DMAList.AddColumn("RAM", 1);
	m_DMAList.AddColumn("Length", 2);
	m_DMAList.AddColumn("Symbol (RAM)", 3);
	m_DMAList.AddColumn("Signature", 4);

	m_DMAList.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);

	m_DMAList.SetColumnWidth(0, 65);
	m_DMAList.SetColumnWidth(1, 65);
	m_DMAList.SetColumnWidth(2, 120);
	//m_DMAList.SetColumnWidth(3, 50);
	//m_DMAList.SetColumnWidth(4, 50);
	//m_DMAList.SetColumnWidth(5, 50);

	m_DMARamEdit.Attach(GetDlgItem(IDC_DMA_RAM_EDIT));
	m_DMARamEdit.SetLimitText(8);

	m_DMARomEdit.Attach(GetDlgItem(IDC_DMA_ROM_EDIT));
	m_DMARomEdit.SetLimitText(8);

	RefreshList();

	WindowCreated();

	m_AutoRefreshThread = CreateThread(NULL, 0, AutoRefreshProc, (void*)this, 0, NULL);

	return TRUE;
}

LRESULT CDebugDMALogView::OnDestroy(void)
{
	if (m_AutoRefreshThread != NULL)
	{
		TerminateThread(m_AutoRefreshThread, 0);
		CloseHandle(m_AutoRefreshThread);
	}
	return 0;
}

LRESULT CDebugDMALogView::OnClicked(WORD /*wNotifyCode*/, WORD wID, HWND, BOOL& /*bHandled*/)
{
	switch (wID)
	{
	case IDOK:
		EndDialog(0);
		break;
	case IDCANCEL:
		EndDialog(0);
		break;
	case IDC_CLEAR_BTN:
		m_Debugger->DMALog()->clear();
		RefreshList();
		break;
	}
	return FALSE;
}

LRESULT CDebugDMALogView::OnRamAddrChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if (m_bConvertingAddress)
	{
		return FALSE;
	}
	char szRamAddr[9];
	char szRomAddr[9];
	m_DMARamEdit.GetWindowTextA(szRamAddr, 9);
	uint32_t ramAddr = strtoul(szRamAddr, NULL, 16);
	uint32_t romAddr = ConvertRamRom(ramAddr);
	sprintf(szRomAddr, "%08X", romAddr);
	m_bConvertingAddress = true;
	m_DMARomEdit.SetWindowTextA(szRomAddr);
	m_bConvertingAddress = false;
	return FALSE;
}

LRESULT CDebugDMALogView::OnRomAddrChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if (m_bConvertingAddress)
	{
		return FALSE;
	}
	char szRamAddr[9];
	char szRomAddr[9];
	m_DMARomEdit.GetWindowTextA(szRomAddr, 9);
	uint32_t romAddr = strtoul(szRomAddr, NULL, 16);
	uint32_t ramAddr = ConvertRomRam(romAddr);
	sprintf(szRamAddr, "%08X", ramAddr);
	m_bConvertingAddress = true;
	m_DMARamEdit.SetWindowTextA(szRamAddr);
	m_bConvertingAddress = false;
	return FALSE;
}

// todo move to a class for a dma log object
uint32_t CDebugDMALogView::ConvertRamRom(uint32_t ramAddr)
{
	for (int i = 0; i < m_Debugger->DMALog()->size(); i++)
	{
		DMALogEntry entry = m_Debugger->DMALog()->at(i);
		if (ramAddr >= entry.ramAddr && ramAddr < entry.ramAddr + entry.length)
		{
			return entry.romAddr + (ramAddr - entry.ramAddr);
		}
	}
	return 0x00000000;
}

// todo move to a class for a dma log object
uint32_t CDebugDMALogView::ConvertRomRam(uint32_t romAddr)
{
	for (int i = 0; i < m_Debugger->DMALog()->size(); i++)
	{
		DMALogEntry entry = m_Debugger->DMALog()->at(i);
		if (romAddr >= entry.romAddr && romAddr < entry.romAddr + entry.length)
		{
			return entry.ramAddr + (romAddr - entry.romAddr);
		}
	}
	return 0x00000000;
}