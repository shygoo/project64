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

//char* CEditEval::m_EvalString;

CDebugScripts::CDebugScripts(CDebuggerUI* debugger) :
CDebugDialog<CDebugScripts>(debugger)
{
	//CScriptSystem::SetScriptsWindow(this);
}

LRESULT CDebugScripts::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	HFONT monoFont = CreateFont(-11, 0, 0, 0,
		FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		CLEARTYPE_QUALITY, FF_DONTCARE, "Consolas"
	);
	
	m_ScriptList.Attach(GetDlgItem(IDC_SCRIPT_LIST));
	m_ScriptList.AddColumn("Script", 0, 0);
	m_ScriptList.SetColumnWidth(0, 100);
	m_ScriptList.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
	m_ScriptList.ModifyStyle(LVS_OWNERDRAWFIXED, 0, 0);

	m_EvalEdit.Attach(GetDlgItem(IDC_EVAL_EDIT));
	m_ConsoleEdit.Attach(GetDlgItem(IDC_CONSOLE_EDIT));

	m_EvalEdit.SetFont(monoFont);
	m_ConsoleEdit.SetFont(monoFont);
	
	RefreshList();

	WindowCreated();
	return 0;
}

void CDebugScripts::ConsolePrint(const char* text)
{
	::ShowWindow(*this, SW_SHOWNOACTIVATE);
	int curLength = m_ConsoleEdit.GetWindowTextLengthA();
	int textLength = strlen(text);
	int newLength = curLength + textLength + 1;
	char* newText = (char*) malloc(newLength);
	m_ConsoleEdit.GetWindowTextA(newText, curLength + 1);
	strcpy(newText + curLength, text);
	m_ConsoleEdit.SetWindowTextA(newText);
}

void CDebugScripts::RefreshConsole()
{
	m_Debugger->Debug_ShowScriptsWindow();
	CScriptSystem* scriptSystem = m_Debugger->ScriptSystem();
	vector<char*>* logData = scriptSystem->LogData();

	while(logData->size() != 0)
	{
		ConsolePrint((*logData)[0]);
		free((*logData)[0]);
		logData->erase(logData->begin() + 0);
	}
}

void CDebugScripts::ConsoleClear()
{
	m_ConsoleEdit.SetWindowTextA("");
}

void CDebugScripts::RefreshList()
{
	CPath SearchPath("Scripts", "*");

	if (!SearchPath.FindFirst(CPath::FIND_ATTRIBUTE_ALLFILES))
	{
		return;
	}

	do
	{
		stdstr scriptFileName = SearchPath.GetNameExtension();
		m_ScriptList.AddItem(0, 0, scriptFileName.c_str());
	} while (SearchPath.FindNext());
}

LRESULT CDebugScripts::OnClicked(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	switch (wID)
	{
	case IDCANCEL:
		EndDialog(0);
		break;
	}
	return FALSE;
}

LRESULT	CDebugScripts::OnScriptListClicked(NMHDR* pNMHDR)
{
	// Set PC breakpoint (right click, double click)
	NMITEMACTIVATE* pIA = reinterpret_cast<NMITEMACTIVATE*>(pNMHDR);
	int nItem = pIA->iItem;

	char scriptName[MAX_PATH];
	m_ScriptList.GetItemText(nItem, 0, scriptName, MAX_PATH);
	
	m_Debugger->ScriptSystem()->RunScript(scriptName);

	return CDRF_DODEFAULT;
}

LRESULT CDebugScripts::OnScriptListCustomDraw(NMHDR* pNMHDR)
{
	NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>(pNMHDR);
	DWORD drawStage = pLVCD->nmcd.dwDrawStage;

	switch (drawStage)
	{
		case CDDS_PREPAINT:
			return CDRF_NOTIFYITEMDRAW;
		case CDDS_ITEMPREPAINT:
			return CDRF_NOTIFYSUBITEMDRAW;
		case (CDDS_ITEMPREPAINT | CDDS_SUBITEM):
			break;
		default:
			return CDRF_DODEFAULT;
	}

	DWORD nItem = pLVCD->nmcd.dwItemSpec;

	char scriptName[MAX_PATH];
	m_ScriptList.GetItemText(nItem, 0, scriptName, MAX_PATH);

	CScriptInstance::INSTANCE_STATE state = m_Debugger->ScriptSystem()->GetInstanceState(scriptName);

	if (state == CScriptInstance::STATE_STARTED)
	{
		pLVCD->clrTextBk = RGB(0xFF, 0xFF, 0xAA);
	}
	else if (state == CScriptInstance::STATE_RUNNING)
	{
		pLVCD->clrTextBk = RGB(0xAA, 0xFF, 0xAA);
	}
	
	return CDRF_DODEFAULT;
}