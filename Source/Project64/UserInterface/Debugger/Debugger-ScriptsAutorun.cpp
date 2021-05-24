#include <stdafx.h>
#include "DebuggerUI.h"
#include <sstream>

CScriptsAutorunDlg::CScriptsAutorunDlg() :
    CDialogImpl<CScriptsAutorunDlg>(),
    m_hQuitScriptDirWatchEvent(nullptr),
    m_hScriptDirWatchThread(nullptr),
    m_bScriptListNeedsRefocus(false),
    m_bAutorunListNeedsRefocus(false)
{
}

CScriptsAutorunDlg::~CScriptsAutorunDlg()
{
}

INT_PTR CScriptsAutorunDlg::DoModal(stdstr selectedScriptName)
{
    m_InitSelectedScriptName = selectedScriptName;
    return CDialogImpl<CScriptsAutorunDlg>::DoModal();
}

LRESULT CScriptsAutorunDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    CenterWindow();

    m_ScriptList.Attach(GetDlgItem(IDC_SCRIPT_LIST));
    m_AutorunList.Attach(GetDlgItem(IDC_AUTORUN_LIST));

    m_ScriptList.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
    m_ScriptList.AddColumn(L"Script", 0);
    m_ScriptList.SetColumnWidth(0, LVSCW_AUTOSIZE_USEHEADER);
    
    m_AutorunList.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
    m_AutorunList.AddColumn(L"Script", 0);
    m_AutorunList.SetColumnWidth(0, LVSCW_AUTOSIZE_USEHEADER);

    m_hQuitScriptDirWatchEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
    m_hScriptDirWatchThread = CreateThread(nullptr, 0, ScriptDirWatchProc, (void*)this, 0, nullptr);

    LoadAutorunSet();
    RefreshAutorunList();
    RefreshScriptList();
    return 0;
}

LRESULT CScriptsAutorunDlg::OnDestroy(void)
{
    SetEvent(m_hQuitScriptDirWatchEvent);
    WaitForSingleObject(m_hScriptDirWatchThread, INFINITE);
    CloseHandle(m_hQuitScriptDirWatchEvent);
    CloseHandle(m_hScriptDirWatchThread);

    m_ScriptList.Detach();
    m_AutorunList.Detach();

    return 0;
}

LRESULT CScriptsAutorunDlg::OnOKCancel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hwnd*/, BOOL& /*bHandled*/)
{
    EndDialog(0);
    return 0;
}


LRESULT CScriptsAutorunDlg::OnAdd(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hwnd*/, BOOL& /*bHandled*/)
{
    m_bScriptListNeedsRefocus = true;
    AddSelected();
    return 0;
}

LRESULT CScriptsAutorunDlg::OnRemove(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hwnd*/, BOOL& /*bHandled*/)
{
    m_bAutorunListNeedsRefocus = true;
    RemoveSelected();
    return 0;
}

LRESULT CScriptsAutorunDlg::OnScriptListDblClicked(NMHDR* /*pNMHDR*/)
{
    AddSelected();
    return 0;
}

LRESULT CScriptsAutorunDlg::OnCtrlSetFocus(NMHDR* pNMHDR)
{
    bool bEnableScriptButtons = false;
    bool bEnableAutorunButtons = false;
    
    switch (pNMHDR->idFrom)
    {
    case IDC_SCRIPT_LIST:
    case IDC_ADD_BTN:
        bEnableScriptButtons = true;
        bEnableAutorunButtons = false;
        break;
    case IDC_AUTORUN_LIST:
    case IDC_REMOVE_BTN:
        bEnableAutorunButtons = true;
        bEnableScriptButtons = false;
        break;
    }
    
    ::EnableWindow(GetDlgItem(IDC_ADD_BTN), bEnableScriptButtons);
    ::EnableWindow(GetDlgItem(IDC_REMOVE_BTN), bEnableAutorunButtons);

    return 0;
}

LRESULT CScriptsAutorunDlg::OnAutorunListDblClicked(NMHDR* /*pNMHDR*/)
{
    RemoveSelected();
    return 0;
}

LRESULT CScriptsAutorunDlg::OnRefreshScriptList(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    int nSelectedItem = m_ScriptList.GetSelectedIndex();

    CPath searchPath("Scripts", "*");

    if (!searchPath.FindFirst(CPath::FIND_ATTRIBUTE_FILES))
    {
        return 0;
    }

    m_ScriptList.SetRedraw(false);
    m_ScriptList.DeleteAllItems();

    size_t nItem = 0;

    do
    {
        stdstr scriptFileName = searchPath.GetNameExtension();
        if (searchPath.GetExtension() == "js" && m_AutorunSet.count(scriptFileName) == 0)
        {
            m_ScriptList.AddItem(nItem, 0, scriptFileName.ToUTF16().c_str());
            if (scriptFileName == m_InitSelectedScriptName)
            {
                nSelectedItem = nItem;
                m_bScriptListNeedsRefocus = true;
                m_InitSelectedScriptName = "";
            }

            nItem++;
        }
    } while (searchPath.FindNext());

    m_ScriptList.SetColumnWidth(0, LVSCW_AUTOSIZE_USEHEADER);

    int itemCount = m_ScriptList.GetItemCount();
    if (itemCount != 0 && nSelectedItem != -1)
    {
        m_ScriptList.SelectItem(nSelectedItem >= itemCount ? itemCount - 1 : nSelectedItem);
    }

    if (m_bScriptListNeedsRefocus)
    {
        m_ScriptList.SetFocus();
        m_bScriptListNeedsRefocus = false;
    }

    m_ScriptList.SetRedraw(true);

    return 0;
}

LRESULT CScriptsAutorunDlg::OnRefreshAutorunList(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    int nSelectedItem = m_AutorunList.GetSelectedIndex();

    m_AutorunList.SetRedraw(FALSE);
    m_AutorunList.DeleteAllItems();

    int nItem = 0;
    std::set<std::string>::iterator it;
    for (it = m_AutorunSet.begin(); it != m_AutorunSet.end(); it++)
    {
        m_AutorunList.AddItem(nItem, 0, stdstr(*it).ToUTF16().c_str());
        if (*it == m_InitSelectedScriptName)
        {
            nSelectedItem = nItem;
            m_bAutorunListNeedsRefocus = true;
            m_InitSelectedScriptName = "";
        }
        nItem++;
    }

    int itemCount = m_AutorunList.GetItemCount();
    if (itemCount != 0 && nSelectedItem != -1)
    {
        m_AutorunList.SelectItem(nSelectedItem >= itemCount ? itemCount - 1 : nSelectedItem);
    }

    if (m_bAutorunListNeedsRefocus)
    {
        m_AutorunList.SetFocus();
        m_bAutorunListNeedsRefocus = false;
    }

    m_AutorunList.SetRedraw(TRUE);

    return 0;
}

DWORD WINAPI CScriptsAutorunDlg::ScriptDirWatchProc(void* ctx)
{
    CScriptsAutorunDlg* _this = (CScriptsAutorunDlg*)ctx;

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
            _this->RefreshScriptList();
            break;
        case WAIT_OBJECT_0 + 1:
            return 0;
        default:
            return 0;
        }
    }
}

void CScriptsAutorunDlg::AddSelected()
{
    int nItem = m_ScriptList.GetSelectedIndex();
    if (nItem == -1)
    {
        return;
    }

    wchar_t scriptName[MAX_PATH];
    m_ScriptList.GetItemText(nItem, 0, scriptName, MAX_PATH);
    m_AutorunSet.insert(stdstr().FromUTF16(scriptName));

    SaveAutorunSet();
    RefreshAutorunList();
    RefreshScriptList();
}

void CScriptsAutorunDlg::RemoveSelected()
{
    int nItem = m_AutorunList.GetSelectedIndex();
    if (nItem == -1)
    {
        return;
    }

    wchar_t scriptName[MAX_PATH];
    m_AutorunList.GetItemText(nItem, 0, scriptName, MAX_PATH);
    m_AutorunSet.erase(stdstr().FromUTF16(scriptName));

    SaveAutorunSet();
    RefreshAutorunList();
    RefreshScriptList();
}

void CScriptsAutorunDlg::RefreshScriptList()
{
    if (m_hWnd != nullptr)
    {
        PostMessage(WM_REFRESH_LIST);
    }
}

void CScriptsAutorunDlg::RefreshAutorunList()
{
    if (m_hWnd != nullptr)
    {
        PostMessage(WM_REFRESH_AUTORUN_LIST);
    }
}

void CScriptsAutorunDlg::LoadAutorunSet()
{
    m_AutorunSet.clear();

    std::istringstream joinedNames(g_Settings->LoadStringVal(Debugger_AutorunScripts));
    std::string scriptName;

    while (std::getline(joinedNames, scriptName, '|'))
    {
        m_AutorunSet.insert(scriptName);
    }
}

void CScriptsAutorunDlg::SaveAutorunSet()
{
    std::string joinedNames = "";

    std::set<std::string>::iterator it;
    for (it = m_AutorunSet.begin(); it != m_AutorunSet.end(); it++)
    {
        if (it != m_AutorunSet.begin())
        {
            joinedNames += "|";
        }
        joinedNames += *it;
    }

    g_Settings->SaveString(Debugger_AutorunScripts, joinedNames.c_str());
}
