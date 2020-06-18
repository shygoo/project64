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
#include "ScriptSystem.h"

//char* CEditEval::m_EvalString;

CDebugScripts::CDebugScripts(CDebuggerUI* debugger) :
    CDebugDialog<CDebugScripts>(debugger)
{
    m_SelectedScriptName = (char*)malloc(MAX_PATH);
    //CScriptSystem::SetScriptsWindow(this);
}

CDebugScripts::~CDebugScripts(void)
{
    free(m_SelectedScriptName);
}

LRESULT CDebugScripts::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    DlgResize_Init(false, true);
    DlgSavePos_Init(DebuggerUI_ScriptsPos);

    HFONT monoFont = CreateFont(-11, 0, 0, 0,
        FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, FF_DONTCARE, L"Consolas"
    );

    m_InstanceInfoEdit.Attach(GetDlgItem(IDC_CTX_INFO_EDIT));

    m_ScriptList.Attach(GetDlgItem(IDC_SCRIPT_LIST));
    m_ScriptList.AddColumn(L"Script", 0, 0);
    m_ScriptList.SetColumnWidth(0, 100);
    m_ScriptList.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
    m_ScriptList.ModifyStyle(LVS_OWNERDRAWFIXED, 0, 0);

    m_EvalEdit.Attach(GetDlgItem(IDC_EVAL_EDIT));
    m_EvalEdit.SetScriptWindow(this);
    m_EvalEdit.SetFont(monoFont);
    m_EvalEdit.EnableWindow(FALSE);

    m_ConsoleEdit.Attach(GetDlgItem(IDC_CONSOLE_EDIT));
    m_ConsoleEdit.SetLimitText(0);
    m_ConsoleEdit.SetFont(monoFont);

    RefreshList();

    LoadWindowPos();
    WindowCreated();
    return 0;
}

void CDebugScripts::OnExitSizeMove(void)
{
    SaveWindowPos(true);
}

void CDebugScripts::ConsolePrint(const char* text)
{
    ::ShowWindow(*this, SW_SHOWNOACTIVATE);

    // Get scrollbar state
    SCROLLINFO scroll;
    scroll.cbSize = sizeof(SCROLLINFO);
    scroll.fMask = SIF_ALL;
    m_ConsoleEdit.GetScrollInfo(SB_VERT, &scroll);

    m_ConsoleEdit.SetRedraw(FALSE);

    m_ConsoleEdit.AppendText(stdstr(text).ToUTF16().c_str());

    m_ConsoleEdit.SetRedraw(TRUE);

    if ((scroll.nPage + scroll.nPos) - 1 == (uint32_t)scroll.nMax)
    {
        m_ConsoleEdit.ScrollCaret();
    }
}

void CDebugScripts::RefreshConsole()
{
    CGuard guard(m_CS);

    m_Debugger->OpenScriptsWindow();
    CScriptSystem* scriptSystem = m_Debugger->ScriptSystem();
    vector<char*>* logData = scriptSystem->LogData();

    while (logData->size() != 0)
    {
        ConsolePrint((*logData)[0]);
        free((*logData)[0]);
        logData->erase(logData->begin() + 0);
    }
}

void CDebugScripts::ConsoleClear()
{
    m_ConsoleEdit.SetWindowText(L"");
}

void CDebugScripts::ConsoleCopy()
{
    if (!OpenClipboard())
    {
        return;
    }

    EmptyClipboard();

    size_t nChars = m_ConsoleEdit.GetWindowTextLength() + 1;

    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, nChars * sizeof(wchar_t));

    wchar_t* memBuf = (wchar_t*)GlobalLock(hMem);
    m_ConsoleEdit.GetWindowText(memBuf, nChars);

    GlobalUnlock(hMem);
    SetClipboardData(CF_TEXT, hMem);

    GlobalFree(hMem);
    CloseClipboard();
}

void CDebugScripts::RefreshList()
{
    CGuard guard(m_CS);

    int nIndex = m_ScriptList.GetSelectedIndex();

    CPath SearchPath("Scripts", "*");

    if (!SearchPath.FindFirst(CPath::FIND_ATTRIBUTE_ALLFILES))
    {
        return;
    }

    m_ScriptList.SetRedraw(false);
    m_ScriptList.DeleteAllItems();

    do
    {
        stdstr scriptFileName = SearchPath.GetNameExtension();
        m_ScriptList.AddItem(0, 0, scriptFileName.ToUTF16().c_str());
    } while (SearchPath.FindNext());

    m_ScriptList.SetRedraw(true);
    m_ScriptList.Invalidate();

    if (nIndex >= 0)
    {
        m_ScriptList.SelectItem(nIndex);
        RefreshStatus();
    }
}

LRESULT CDebugScripts::OnClicked(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    switch (wID)
    {
    case IDCANCEL:
        EndDialog(0);
        break;
    case ID_POPUP_RUN:
        RunSelected();
        break;
    case ID_POPUP_STOP:
        StopSelected();
        break;
    case ID_POPUP_SCRIPT_EDIT:
        EditSelected();
        break;
    case IDC_CLEAR_BTN:
        ConsoleClear();
        break;
    case IDC_COPY_BTN:
        ConsoleCopy();
        break;
    }
    return FALSE;
}

LRESULT CDebugScripts::OnScriptListDblClicked(NMHDR* pNMHDR)
{
    // Run script on double click
    NMITEMACTIVATE* pIA = reinterpret_cast<NMITEMACTIVATE*>(pNMHDR);
    int nItem = pIA->iItem;

    m_ScriptList.SelectItem(nItem);

    ToggleSelected();

    return 0;
}

void CDebugScripts::RefreshStatus()
{
    CGuard guard(m_CS);
    INSTANCE_STATE state = m_Debugger->ScriptSystem()->GetInstanceState(m_SelectedScriptName);

    char* szState = "";
    switch (state)
    {
    case STATE_RUNNING: szState = "Running"; break;
    case STATE_STARTED: szState = "Started"; break;
    case STATE_STOPPED: szState = "Stopped"; break;
    case STATE_INVALID: szState = "Not running"; break;
    }

    stdstr instanceInfo = stdstr_f("%s (%s)", m_SelectedScriptName, szState);

    m_InstanceInfoEdit.SetWindowText(instanceInfo.ToUTF16().c_str());

    if (state == STATE_RUNNING)
    {
        m_EvalEdit.EnableWindow(TRUE);
    }
    else
    {
        m_EvalEdit.EnableWindow(FALSE);
    }
}

LRESULT CDebugScripts::OnScriptListClicked(NMHDR* pNMHDR)
{
    // Select instance for console input
    NMITEMACTIVATE* pIA = reinterpret_cast<NMITEMACTIVATE*>(pNMHDR);
    int nItem = pIA->iItem;

    wchar_t ScriptName[MAX_PATH];
    m_ScriptList.GetItemText(nItem, 0, ScriptName, MAX_PATH);
    strcpy(m_SelectedScriptName, stdstr().FromUTF16(ScriptName).c_str());

    RefreshStatus();

    return 0;
}

LRESULT CDebugScripts::OnScriptListRClicked(NMHDR* pNMHDR)
{
    OnScriptListClicked(pNMHDR);

    HMENU hMenu = LoadMenu(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_SCRIPT_POPUP));
    HMENU hPopupMenu = GetSubMenu(hMenu, 0);

    /*
    if (m_Breakpoints->m_RBP.size() == 0 && m_Breakpoints->m_WBP.size() == 0)
    {
        EnableMenuItem(hPopupMenu, ID_POPUPMENU_CLEARALLBPS, MF_DISABLED | MF_GRAYED);
    }
    */

    POINT mouse;
    GetCursorPos(&mouse);

    TrackPopupMenu(hPopupMenu, TPM_LEFTALIGN, mouse.x, mouse.y, 0, m_hWnd, NULL);

    DestroyMenu(hMenu);

    return 0;
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

    wchar_t scriptName[MAX_PATH];
    m_ScriptList.GetItemText(nItem, 0, scriptName, MAX_PATH);

    INSTANCE_STATE state = m_Debugger->ScriptSystem()->GetInstanceState(stdstr("").FromUTF16(scriptName).c_str());

    if (state == STATE_STARTED)
    {
        pLVCD->clrTextBk = RGB(0xFF, 0xFF, 0xAA);
    }
    else if (state == STATE_RUNNING)
    {
        pLVCD->clrTextBk = RGB(0xAA, 0xFF, 0xAA);
    }

    return CDRF_DODEFAULT;
}

void CDebugScripts::EvaluateInSelectedInstance(const char* code)
{
    INSTANCE_STATE state = m_Debugger->ScriptSystem()->GetInstanceState(m_SelectedScriptName);

    if (state == STATE_RUNNING || state == STATE_STARTED)
    {
        CScriptInstance* instance = m_Debugger->ScriptSystem()->GetInstance(m_SelectedScriptName);
        //instance->EvalAsync(code);
        instance->Eval(code);
    }
}

// Console input
LRESULT CEditEval::OnKeyDown(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{
    if (wParam == VK_UP)
    {
        if (m_HistoryIdx > 0)
        {
            wchar_t* code = m_History[--m_HistoryIdx];
            SetWindowText(code);
            int selEnd = wcslen(code);
            SetSel(selEnd, selEnd);
        }
    }
    else if (wParam == VK_DOWN)
    {
        int size = m_History.size();
        if (m_HistoryIdx < size - 1)
        {
            wchar_t* code = m_History[++m_HistoryIdx];
            SetWindowText(code);
            int selEnd = wcslen(code);
            SetSel(selEnd, selEnd);
        }
        else if (m_HistoryIdx < size)
        {
            SetWindowText(L"");
            m_HistoryIdx++;
        }
    }
    else if (wParam == VK_RETURN)
    {
        if (m_ScriptWindow == NULL)
        {
            bHandled = FALSE;
            return 0;
        }

        size_t codeLength = GetWindowTextLength() + 1;
        wchar_t* code = (wchar_t*)malloc(codeLength * sizeof(wchar_t));
        GetWindowText(code, codeLength);

        m_ScriptWindow->EvaluateInSelectedInstance(stdstr().FromUTF16(code).c_str());

        SetWindowText(L"");
        int historySize = m_History.size();

        // remove duplicate
        for (int i = 0; i < historySize; i++)
        {
            if (wcscmp(code, m_History[i]) == 0)
            {
                free(m_History[i]);
                m_History.erase(m_History.begin() + i);
                historySize--;
                break;
            }
        }

        // remove oldest if maxed
        if (historySize >= HISTORY_MAX_ENTRIES)
        {
            m_History.erase(m_History.begin() + 0);
            historySize--;
        }

        m_History.push_back(code);
        m_HistoryIdx = ++historySize;
    }
    bHandled = FALSE;
    return 0;
}

void CDebugScripts::RunSelected()
{
    INSTANCE_STATE state = m_Debugger->ScriptSystem()->GetInstanceState(m_SelectedScriptName);

    if (state == STATE_INVALID || state == STATE_STOPPED)
    {
        m_Debugger->ScriptSystem()->RunScript(m_SelectedScriptName);
    }
    else
    {
        m_Debugger->Debug_LogScriptsWindow("[Error: Script is already running]\n");
    }
}

void CDebugScripts::StopSelected()
{
    m_Debugger->ScriptSystem()->StopScript(m_SelectedScriptName);

    //m_Debugger->Debug_RefreshScriptsWindow();
}

void CDebugScripts::ToggleSelected()
{
    INSTANCE_STATE state = m_Debugger->ScriptSystem()->GetInstanceState(m_SelectedScriptName);

    if (state == STATE_INVALID || state == STATE_STOPPED)
    {
        RunSelected();
    }
    else
    {
        StopSelected();
    }
}

void CDebugScripts::EditSelected()
{
    ShellExecute(NULL, L"edit", stdstr(m_SelectedScriptName).ToUTF16().c_str(), NULL, L"Scripts", SW_SHOWNORMAL);
}
