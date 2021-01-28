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
#pragma once

#include <stdafx.h>
#include <Project64-core/Plugins/PluginClass.h>
#include <Project64-core/N64System/N64Class.h>

class CDebugRenderWindow :
    public RenderWindow
{
private:
    CMainGui* m_MainGui;
    HWND m_hWnd;
    CDebugRenderWindow();
public:
    CDebugRenderWindow(CMainGui* mainGui);
    ~CDebugRenderWindow();

    bool Create();
    bool RegisterWinClass();
    static LRESULT CALLBACK DebugRenderWindow_Proc(HWND hWnd, DWORD uMsg, DWORD wParam, DWORD lParam);

    bool ResetPluginsInUiThread(CPlugins * plugins, CN64System * System);
    void* GetWindowHandle(void) const;
    void* GetStatusBar(void) const;
    void* GetModuleInstance(void) const;
};
