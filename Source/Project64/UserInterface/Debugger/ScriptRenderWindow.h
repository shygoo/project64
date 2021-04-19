#pragma once

#include <stdafx.h>
#include <Project64-core/Plugins/Plugin.h>
#include <Project64-core/N64System/N64System.h>

class CScriptRenderWindow
{
private:
    HWND m_hWnd;
    bool Create();
    bool RegisterWinClass();
    static LRESULT CALLBACK ScriptRenderWindow_Proc(HWND hWnd, DWORD uMsg, DWORD wParam, DWORD lParam);
    LRESULT Proc(HWND hWnd, DWORD uMsg, DWORD wParam, DWORD lParam);

public:
    CScriptRenderWindow();
    ~CScriptRenderWindow();

    HWND GetWindowHandle(void);

    void FixPosition(HWND hMainWnd);
    void SetVisible(bool bVisible);

    static void CpuRunningChanged(void* p);
};