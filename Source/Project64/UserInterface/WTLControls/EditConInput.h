#include "stdafx.h"

#pragma once

enum {
    CIN_SPECIALKEY
};

enum ci_key_t {
    CI_KEY_UP,
    CI_KEY_DOWN,
    CI_KEY_ENTER
};

typedef struct
{
    NMHDR    nmh;
    ci_key_t key;
} NMCISPECIALKEY;

class CEditConInput : public CWindowImpl<CEditConInput, CEdit>
{
public:
    CEditConInput()
    {
    }

    LRESULT OnKeyDown(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
    {
        NMCISPECIALKEY nmsk;

        switch (wParam)
        {
        case VK_UP:
            nmsk = { { m_hWnd, (UINT_PTR)GetDlgCtrlID(), CIN_SPECIALKEY }, CI_KEY_UP };
            SendMessage(GetParent(), WM_NOTIFY, nmsk.nmh.idFrom, (LPARAM)&nmsk);
            break;
        case VK_DOWN:
            nmsk = { { m_hWnd, (UINT_PTR)GetDlgCtrlID(), CIN_SPECIALKEY }, CI_KEY_DOWN };
            SendMessage(GetParent(), WM_NOTIFY, nmsk.nmh.idFrom, (LPARAM)&nmsk);
            break;
        case VK_RETURN:
            nmsk = { { m_hWnd, (UINT_PTR)GetDlgCtrlID(), CIN_SPECIALKEY }, CI_KEY_ENTER };
            SendMessage(GetParent(), WM_NOTIFY, nmsk.nmh.idFrom, (LPARAM)&nmsk);
            break;
        }

        bHandled = FALSE;

        return 0;
    }

    BOOL Attach(HWND hWndNew)
    {
        return SubclassWindow(hWndNew);
    }

    BEGIN_MSG_MAP_EX(CEditEval)
        MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
    END_MSG_MAP()
};

class CEditConOutput : public CWindowImpl<CEditConOutput, CEdit>
{
private:
    LRESULT OnKeyDown(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    {
        if (GetKeyState(VK_CONTROL) < 0)
        {
            if (wParam == 'A')
            {
                this->SetSelAll();
            }
        }
        return FALSE;
    }
public:
    BOOL Attach(HWND hWndNew)
    {
        return SubclassWindow(hWndNew);
    }

    BEGIN_MSG_MAP_EX(CEditEval)
        MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
        END_MSG_MAP()
};