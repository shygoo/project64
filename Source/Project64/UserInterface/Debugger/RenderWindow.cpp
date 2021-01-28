/****************************************************************************
*                                                                           *
* Project64 - A Nintendo 64 emulator.                                       *
* http://www.pj64-emu.com/                                                  *
* Copyright (C) 2021 Project64. All rights reserved.                        *
*                                                                           *
* License:                                                                  *
* GNU/GPLv2 http://www.gnu.org/licenses/gpl-2.0.html                        *
*                                                                           *
****************************************************************************/
#include <stdafx.h>
#include "RenderWindow.h"

CDebugRenderWindow::CDebugRenderWindow(CMainGui* mainGui) :
    m_MainGui(mainGui),
    m_hWnd(NULL)
{
    RegisterWinClass();
    Create();
}

CDebugRenderWindow::~CDebugRenderWindow()
{
    if (m_hWnd != NULL)
    {
        DestroyWindow(m_hWnd);
    }
}

bool CDebugRenderWindow::ResetPluginsInUiThread(CPlugins* plugins, CN64System* System)
{
    return m_MainGui->ResetPluginsInUiThread(plugins, System);
}

void* CDebugRenderWindow::GetWindowHandle(void) const
{
    return (void*)m_hWnd;
}

void* CDebugRenderWindow::GetStatusBar(void) const
{
    return m_MainGui->GetStatusBar();
}

void* CDebugRenderWindow::GetModuleInstance(void) const
{
    return m_MainGui->GetModuleInstance();
}

LRESULT CALLBACK CDebugRenderWindow::DebugRenderWindow_Proc(HWND hWnd, DWORD uMsg, DWORD wParam, DWORD lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
        {
            LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
            CMainGui * _this = (CMainGui *)lpcs->lpCreateParams;
            SetProp(hWnd, L"Class", _this);
            break;
        }
    case WM_SIZE:
        {
            CDebugRenderWindow* _this = (CDebugRenderWindow*)GetProp(hWnd, L"Class");
            int width = LOWORD(lParam);
            int height = HIWORD(lParam);
            SetWindowPos((HWND)_this->m_MainGui->GetWindowHandle(), NULL, 0, 0, width, height, SWP_NOMOVE);
            
            //_this->m_MainGui->

            break;
        }
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

bool CDebugRenderWindow::Create()
{
    m_hWnd = CreateWindowEx(WS_EX_TOPMOST,
        L"DebugRenderWindow", L"DebugRenderWindow",
        WS_OVERLAPPED | WS_VISIBLE, 1, 1, 256, 256,
        0, 0, GetModuleHandle(NULL), this);

    return (m_hWnd != NULL);
}

bool CDebugRenderWindow::RegisterWinClass()
{
    WNDCLASSEX wcl = { 0 };
    wcl.cbSize = sizeof(WNDCLASSEX);
    wcl.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(255, 0, 0));
    wcl.hInstance = GetModuleHandle(NULL);
    wcl.lpfnWndProc = (WNDPROC)DebugRenderWindow_Proc;
    wcl.lpszClassName = L"DebugRenderWindow";
    wcl.style = CS_VREDRAW | CS_HREDRAW;
    wcl.cbClsExtra = NULL;
    wcl.cbWndExtra = NULL;
    wcl.lpszMenuName = L"DebugRenderWindow";
    wcl.hCursor = LoadCursor(0, IDC_ARROW);
    wcl.hIcon = LoadIcon(0, IDI_APPLICATION);
    wcl.hIconSm = LoadIcon(0, IDI_APPLICATION);

    return (RegisterClassEx(&wcl) != 0);
}

