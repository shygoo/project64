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

#include <stdio.h>
#include <Common/path.h>

CDebugSymbols::CDebugSymbols(CDebuggerUI * debugger) :
	CDebugDialog<CDebugSymbols>(debugger)
{
	m_SymFileBuffer = NULL;
}
/*

type,address,name,description

u32,80370000,variable1
code,80370000,variable1

*/

void CDebugSymbols::LoadSymbols()
{

	if (g_Settings->LoadStringVal(Game_GameName).length() == 0)
	{
		// no game is loaded
		return;
	}

	stdstr winTitle;
	winTitle.Format("Symbols - %s", g_Settings->LoadStringVal(Game_GameName).c_str());

	SetWindowTextA(winTitle.c_str());

	stdstr symFileName;
	symFileName.Format("%s.sym", g_Settings->LoadStringVal(Game_GameName).c_str());

	CPath symFilePath(g_Settings->LoadStringVal(Directory_NativeSave).c_str(), symFileName.c_str());
	
	if (g_Settings->LoadBool(Setting_UniqueSaveDir))
	{
		symFilePath.AppendDirectory(g_Settings->LoadStringVal(Game_UniqueSaveDir).c_str());
	}
	if (!symFilePath.DirectoryExists())
	{
		symFilePath.DirectoryCreate();
	}

	m_SymFileHandle.Open(symFilePath, CFileBase::modeReadWrite | CFileBase::modeCreate);
	m_SymFileHandle.SeekToBegin();

	if (m_SymFileBuffer != NULL)
	{
		free(m_SymFileBuffer);
	}

	uint32_t symFileSize = m_SymFileHandle.GetLength();
	m_SymFileBuffer = (char*)malloc(symFileSize + 1);
	m_SymFileBuffer[symFileSize] = '\0';
	m_SymFileHandle.Read(m_SymFileBuffer, symFileSize);

}

LRESULT CDebugSymbols::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	LoadSymbols();

	WindowCreated();
	return 0;
}

LRESULT CDebugSymbols::OnClicked(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	switch (wID)
	{
	case IDCANCEL:
		EndDialog(0);
		break;
	}
	return FALSE;
}