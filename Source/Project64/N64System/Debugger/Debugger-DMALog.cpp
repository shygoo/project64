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
}

CDebugDMALogView::~CDebugDMALogView()
{
}

void CDebugDMALogView::RefreshList()
{
	m_DMAList.DeleteAllItems();

	for (int i = 0; i < m_Debugger->DMALog()->size(); i++)
	{
		DMALogEntry entry = (*m_Debugger->DMALog())[i];
		m_DMAList.AddItem(i, 0, stdstr_f("%08X", entry.romAddr).c_str());
		m_DMAList.AddItem(i, 1, stdstr_f("%08X", entry.ramAddr).c_str());
		m_DMAList.AddItem(i, 2, stdstr_f("%08X (%d)", entry.length, entry.length).c_str());
		m_DMAList.AddItem(i, 3, stdstr_f("%d", entry.count).c_str());
	}
}

LRESULT CDebugDMALogView::OnActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	RefreshList();
	return FALSE;
}

LRESULT CDebugDMALogView::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_DMAList.Attach(GetDlgItem(IDC_DMA_LIST));

	m_DMAList.AddColumn("ROM", 0);
	m_DMAList.AddColumn("RAM", 1);
	m_DMAList.AddColumn("Length", 2);
	m_DMAList.AddColumn("Count", 3);

	m_DMAList.SetColumnWidth(0, 70);
	m_DMAList.SetColumnWidth(1, 70);
	m_DMAList.SetColumnWidth(2, 120);
	m_DMAList.SetColumnWidth(3, 40);

	RefreshList();

	WindowCreated();
	return TRUE;
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