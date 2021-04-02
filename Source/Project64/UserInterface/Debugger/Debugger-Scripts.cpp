#include "stdafx.h"
#include "DebuggerUI.h"

CDebugScripts::CDebugScripts(CDebuggerUI* debugger) :
    CDebugDialog<CDebugScripts>(debugger),
    CToolTipDialog<CDebugScripts>(),
    m_hQuitScriptDirWatchEvent(NULL),
    m_hScriptDirWatchThread(NULL)
{
}

CDebugScripts::~CDebugScripts(void)
{
}

LRESULT CDebugScripts::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    DlgResize_Init(false, true);
    DlgSavePos_Init(DebuggerUI_ScriptsPos);
    DlgToolTip_Init();

    HFONT monoFont = CreateFont(-12, 0, 0, 0,
        FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, FF_DONTCARE, L"Consolas"
    );

    m_ScriptList.Attach(GetDlgItem(IDC_SCRIPT_LIST));
    m_ScriptList.AddColumn(L"Status", 0);
    m_ScriptList.AddColumn(L"Script", 1);
    m_ScriptList.SetColumnWidth(0, 16);
    m_ScriptList.SetColumnWidth(1, LVSCW_AUTOSIZE_USEHEADER);
    m_ScriptList.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
    m_ScriptList.ModifyStyle(LVS_OWNERDRAWFIXED, 0, 0);

    m_EvalEdit.Attach(GetDlgItem(IDC_EVAL_EDIT));
    m_EvalEdit.SetScriptWindow(this);
    m_EvalEdit.SetFont(monoFont);
    m_EvalEdit.EnableWindow(FALSE);

    m_ConsoleEdit.Attach(GetDlgItem(IDC_CONSOLE_EDIT));
    m_ConsoleEdit.SetLimitText(0);
    m_ConsoleEdit.SetFont(monoFont);

    int statusPaneWidths[] = { -1 };
    m_StatusBar.Attach(GetDlgItem(IDC_STATUSBAR));
    m_StatusBar.SetParts(1, statusPaneWidths);

    RefreshList();

    LoadWindowPos();
    WindowCreated();

    m_hQuitScriptDirWatchEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    m_hScriptDirWatchThread = CreateThread(NULL, 0, ScriptDirWatchProc, (void*)this, 0, NULL);

    m_ConsoleEdit.SetWindowText(m_Debugger->ScriptSystem()->GetLog().ToUTF16().c_str());
    return 0;
}

LRESULT CDebugScripts::OnDestroy(void)
{
    SetEvent(m_hQuitScriptDirWatchEvent);
    WaitForSingleObject(m_hScriptDirWatchThread, INFINITE);
    CloseHandle(m_hQuitScriptDirWatchEvent);
    CloseHandle(m_hScriptDirWatchThread);
    return 0;
}

LRESULT CDebugScripts::OnCtlColorStatic(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
    HDC hDC = (HDC)wParam;
    HWND hCtrl = (HWND)lParam;
    WORD ctrlId = (WORD) ::GetWindowLong(hCtrl, GWL_ID);

    if (ctrlId == IDC_CONSOLE_EDIT)
    {
        SetTextColor(hDC, RGB(0xEE, 0xEE, 0xEE));
        SetBkColor(hDC, RGB(0x22, 0x22, 0x22));
        SetDCBrushColor(hDC, RGB(0x22, 0x22, 0x22));
        return (LRESULT)GetStockObject(DC_BRUSH);
    }

    return FALSE;
}

LRESULT CDebugScripts::OnCtlColorEdit(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
    HDC hDC = (HDC)wParam;
    HWND hCtrl = (HWND)lParam;
    WORD ctrlId = (WORD) ::GetWindowLong(hCtrl, GWL_ID);

    if (ctrlId == IDC_EVAL_EDIT)
    {
        SetTextColor(hDC, RGB(0xEE, 0xEE, 0xEE));
        SetBkColor(hDC, RGB(0x22, 0x22, 0x22));
        SetDCBrushColor(hDC, RGB(0x22, 0x22, 0x22));
        return (LRESULT)GetStockObject(DC_BRUSH);
    }

    return FALSE;
}

DWORD WINAPI CDebugScripts::ScriptDirWatchProc(void* ctx)
{
    CDebugScripts* _this = (CDebugScripts*)ctx;

    HANDLE hEvents[2];

    hEvents[0] = FindFirstChangeNotification(L"Scripts", FALSE, FILE_NOTIFY_CHANGE_FILE_NAME);

    if (hEvents[0] == INVALID_HANDLE_VALUE)
    {
        return 0;
    }

    hEvents[1] = _this->m_hQuitScriptDirWatchEvent;

    while (true)
    {
        DWORD status = WaitForMultipleObjects(2, hEvents, FALSE, INFINITE);

        switch (status)
        {
        case WAIT_OBJECT_0:
            if (FindNextChangeNotification(hEvents[0]) == FALSE)
            {
                return 0;
            }
            _this->PostMessage(WM_REFRESH_LIST, 0, 0);
            break;
        case WAIT_OBJECT_0 + 1:
            return 0;
        default:
            return 0;
        }
    }
}

void CDebugScripts::OnExitSizeMove(void)
{
    SaveWindowPos(true);
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

    if (hMem == NULL)
    {
        return;
    }

    wchar_t* memBuf = (wchar_t*)GlobalLock(hMem);

    if (memBuf == NULL)
    {
        GlobalUnlock(hMem);
        GlobalFree(hMem);
        return;
    }

    m_ConsoleEdit.GetWindowText(memBuf, nChars);

    GlobalUnlock(hMem);
    SetClipboardData(CF_UNICODETEXT, hMem);

    GlobalFree(hMem);
    CloseClipboard();
}

void CDebugScripts::ConsolePrint(const char* text)
{
    if (m_hWnd != NULL)
    {
        char* textCopy = _strdup(text); // OnConsolePrint will free this
        PostMessage(WM_CONSOLE_PRINT, (WPARAM)textCopy);
        Sleep(5); // Prevent flooding of the message queue
    }
}

void CDebugScripts::ConsoleClear()
{
    if (m_hWnd != NULL)
    {
        PostMessage(WM_CONSOLE_CLEAR);
    }
}

void CDebugScripts::RefreshList()
{
    if (m_hWnd != NULL)
    {
        PostMessage(WM_REFRESH_LIST);
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
    case IDC_RUN_BTN:
        RunSelected();
        break;
    case ID_POPUP_STOP:
    case IDC_STOP_BTN:
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
    case IDC_SCRIPTDIR_BTN:
        ShellExecute(NULL, L"open", L"Scripts", NULL, NULL, SW_SHOW);
        break;
    }
    return FALSE;
}

LRESULT CDebugScripts::OnScriptListDblClicked(NMHDR* pNMHDR)
{
    NMITEMACTIVATE* pIA = reinterpret_cast<NMITEMACTIVATE*>(pNMHDR);
    int nItem = pIA->iItem;

    if (nItem == -1)
    {
        return 0;
    }

    ToggleSelected();

    return 0;
}

void CDebugScripts::RefreshStatus()
{
    jsstatus_t status = m_Debugger->ScriptSystem()->GetStatus(m_SelectedScriptName.c_str());

    stdstr statusText;
    CPath(stdstr_f("Scripts\\%s", m_SelectedScriptName.c_str())).GetFullyQualified(statusText);

    if (status == JS_STATUS_STARTED)
    {
        statusText += " (Started)";
        m_EvalEdit.EnableWindow(TRUE);
    }
    else
    {
        if (status == JS_STATUS_STARTING)
        {
            statusText += " (Starting)";
        }
        m_EvalEdit.EnableWindow(FALSE);
    }
    
    m_StatusBar.SetText(0, statusText.ToUTF16().c_str());
}

LRESULT CDebugScripts::OnScriptListRClicked(NMHDR* pNMHDR)
{
    NMITEMACTIVATE* pIA = reinterpret_cast<NMITEMACTIVATE*>(pNMHDR);
    int nItem = pIA->iItem;

    if (nItem == -1)
    {
        return 0;
    }

    jsstatus_t status = m_Debugger->ScriptSystem()->GetStatus(m_SelectedScriptName.c_str());

    HMENU hMenu = LoadMenu(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_SCRIPT_POPUP));
    HMENU hPopupMenu = GetSubMenu(hMenu, 0);

    if (status == JS_STATUS_STARTING || status == JS_STATUS_STARTED)
    {
        EnableMenuItem(hPopupMenu, ID_POPUP_RUN, MF_DISABLED | MF_GRAYED);
    }
    else
    {
        EnableMenuItem(hPopupMenu, ID_POPUP_STOP, MF_DISABLED | MF_GRAYED);
    }
    
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
    m_ScriptList.GetItemText(nItem, 1, scriptName, MAX_PATH);

    jsstatus_t status = m_Debugger->ScriptSystem()->GetStatus(stdstr("").FromUTF16(scriptName).c_str());

    if (status == JS_STATUS_STARTING)
    {
        pLVCD->clrTextBk = RGB(0xFF, 0xFF, 0xAA);
    }
    else if (status == JS_STATUS_STARTED)
    {
        pLVCD->clrTextBk = RGB(0xAA, 0xFF, 0xAA);
    }

    return CDRF_DODEFAULT;
}

LRESULT CDebugScripts::OnScriptListItemChanged(NMHDR* pNMHDR)
{
    NMLISTVIEW* lpStateChange = reinterpret_cast<NMLISTVIEW*>(pNMHDR);
    if ((lpStateChange->uNewState ^  lpStateChange->uOldState) & LVIS_SELECTED)
    {
        if (lpStateChange->iItem == -1)
        {
            return FALSE;
        }

        wchar_t ScriptName[MAX_PATH];

        m_ScriptList.GetItemText(lpStateChange->iItem, 1, ScriptName, MAX_PATH);
        m_SelectedScriptName = stdstr().FromUTF16(ScriptName).c_str();

        jsstatus_t status = m_Debugger->ScriptSystem()->GetStatus(m_SelectedScriptName.c_str());

        ::EnableWindow(GetDlgItem(IDC_STOP_BTN), status == JS_STATUS_STARTING || status == JS_STATUS_STARTED);
        ::EnableWindow(GetDlgItem(IDC_RUN_BTN), status == JS_STATUS_STOPPED);

        RefreshStatus();
    }
    return FALSE;
}

LRESULT CDebugScripts::OnConsolePrint(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    char *text = (char*)wParam;

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

    free(text);

    return FALSE;
}

LRESULT CDebugScripts::OnConsoleClear(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    m_ConsoleEdit.SetWindowText(L"");
    return FALSE;
}

LRESULT CDebugScripts::OnRefreshList(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    int nIndex = m_ScriptList.GetSelectedIndex();

    CPath SearchPath("Scripts", "*");

    if (!SearchPath.FindFirst(CPath::FIND_ATTRIBUTE_ALLFILES))
    {
        return FALSE;
    }

    m_ScriptList.SetRedraw(false);
    m_ScriptList.DeleteAllItems();

    size_t nItem = 0;

    do
    {
        stdstr scriptFileName = SearchPath.GetNameExtension();
        jsstatus_t status = m_Debugger->ScriptSystem()->GetStatus(scriptFileName.c_str());
        const wchar_t *statusIcon = L"";
    
        switch (status)
        {
        case JS_STATUS_STARTED:
            statusIcon = L"*";
            break;
        case JS_STATUS_STARTING:
            statusIcon = L">";
            break;
        default:
            statusIcon = L"-";
            break;
        }
    
        m_ScriptList.AddItem(nItem, 0, statusIcon);
        m_ScriptList.SetItemText(nItem, 1, scriptFileName.ToUTF16().c_str());
        nItem++;
    } while (SearchPath.FindNext());

    m_ScriptList.SetRedraw(true);
    m_ScriptList.Invalidate();

    if (nIndex >= 0)
    {
        m_ScriptList.SelectItem(nIndex);
        RefreshStatus();
    }
    return FALSE;
}

//LRESULT OnStatusChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
//{
//
//}

void CDebugScripts::EvaluateInSelectedInstance(const char* code)
{
    m_Debugger->ScriptSystem()->Eval(m_SelectedScriptName.c_str(), code);
}

void CDebugScripts::RunSelected()
{
    if (m_SelectedScriptName.empty())
    {
        return;
    }

    stdstr path = stdstr("scripts/") + m_SelectedScriptName;

    m_Debugger->ScriptSystem()->StartScript(m_SelectedScriptName.c_str(), path.c_str());
}

void CDebugScripts::StopSelected()
{
    m_Debugger->ScriptSystem()->StopScript(m_SelectedScriptName.c_str());
}

void CDebugScripts::ToggleSelected()
{
    jsstatus_t status = m_Debugger->ScriptSystem()->GetStatus(m_SelectedScriptName.c_str());

    if (status == JS_STATUS_STOPPED)
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

        size_t codeLength = GetWindowTextLength();
        wchar_t* code = new wchar_t[codeLength + 1];
        GetWindowText(code, codeLength + 1);

        m_ScriptWindow->EvaluateInSelectedInstance(stdstr().FromUTF16(code).c_str());

        SetWindowText(L"");
        int historySize = m_History.size();

        // If there is a duplicate entry move it to the bottom
        for (int i = 0; i < historySize; i++)
        {
            if (wcscmp(code, m_History[i]) == 0)
            {
                wchar_t* str = m_History[i];
                m_History.erase(m_History.begin() + i);
                m_History.push_back(str);

                delete[] code;
                bHandled = FALSE;
                return 0;
            }
        }

        // Remove oldest if maxed
        if (historySize >= HISTORY_MAX_ENTRIES)
        {
            delete[] m_History[0];
            m_History.erase(m_History.begin() + 0);
            historySize--;
        }

        m_History.push_back(code);
        m_HistoryIdx = ++historySize;
    }
    bHandled = FALSE;
    return 0;
}
