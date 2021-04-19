#include <stdafx.h>
#include "ScriptRenderWindow.h"

CScriptRenderWindow::CScriptRenderWindow() :
    m_hWnd(nullptr)
{
    RegisterWinClass();
    Create();
}

CScriptRenderWindow::~CScriptRenderWindow()
{
    if (m_hWnd != nullptr)
    {
        DestroyWindow(m_hWnd);
    }
}

bool CScriptRenderWindow::RegisterWinClass()
{
    WNDCLASSEX wcl = { 0 };
    wcl.cbSize = sizeof(WNDCLASSEX);
    wcl.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcl.hInstance = GetModuleHandle(nullptr);
    wcl.lpfnWndProc = (WNDPROC)ScriptRenderWindow_Proc;
    wcl.lpszClassName = L"ScriptRenderWindow";
    wcl.style = CS_VREDRAW | CS_HREDRAW;
    wcl.cbClsExtra = 0;
    wcl.cbWndExtra = 0;
    wcl.lpszMenuName = L"ScriptRenderWindow";
    wcl.hCursor = LoadCursor(0, IDC_ARROW);
    wcl.hIcon = LoadIcon(0, IDI_APPLICATION);
    wcl.hIconSm = LoadIcon(0, IDI_APPLICATION);

    return (RegisterClassEx(&wcl) != 0);
}

HWND CScriptRenderWindow::GetWindowHandle(void)
{
    return m_hWnd;
}

bool CScriptRenderWindow::Create()
{
    m_hWnd = CreateWindowEx(WS_EX_TOOLWINDOW,
        L"ScriptRenderWindow", L"ScriptRenderWindow",
        WS_POPUP, 0, 0, 320, 240,
        nullptr, nullptr,
        GetModuleHandle(nullptr), this);

    return (m_hWnd != nullptr);
}

LRESULT CALLBACK CScriptRenderWindow::ScriptRenderWindow_Proc(HWND hWnd, DWORD uMsg, DWORD wParam, DWORD lParam)
{
    CScriptRenderWindow* _this = nullptr;

    if (uMsg == WM_CREATE)
    {
        CREATESTRUCT* createStruct = (CREATESTRUCT*)lParam;
        _this = (CScriptRenderWindow*)createStruct->lpCreateParams;
        SetProp(hWnd, L"Class", (HANDLE)createStruct->lpCreateParams);
    }
    else
    {
        _this = (CScriptRenderWindow*)GetProp(hWnd, L"Class");
    }

    if (_this == nullptr)
    {
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    return _this->Proc(hWnd, uMsg, wParam, lParam);
}

void CScriptRenderWindow::CpuRunningChanged(void* p)
{
    CScriptRenderWindow* _this = (CScriptRenderWindow*)p;
    bool bRunning = g_Settings->LoadBool(GameRunning_CPU_Running);
    _this->SetVisible(bRunning);
}

LRESULT CScriptRenderWindow::Proc(HWND hWnd, DWORD uMsg, DWORD wParam, DWORD lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
        g_Settings->RegisterChangeCB(GameRunning_CPU_Running, this, CpuRunningChanged);
        break;
    case WM_DESTROY:
        g_Settings->UnregisterChangeCB(GameRunning_CPU_Running, this, CpuRunningChanged);
        break;
    case WM_SETFOCUS:
    case WM_LBUTTONDOWN:
        if (g_Plugins && g_Plugins->MainWindow() && g_Plugins->MainWindow()->GetWindowHandle())
        {
            HWND hMainWnd = (HWND)g_Plugins->MainWindow()->GetWindowHandle();
            BringWindowToTop(hMainWnd);
            SetFocus(hMainWnd);
        }
        break;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void CScriptRenderWindow::FixPosition(HWND hMainWnd)
{
    if (m_hWnd != nullptr && hMainWnd != nullptr)
    {
        CRect rc;
        GetClientRect(hMainWnd, &rc);
        ClientToScreen(hMainWnd, &rc.TopLeft());
        ClientToScreen(hMainWnd, &rc.BottomRight());
        SetWindowPos(m_hWnd, HWND_TOP, rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOACTIVATE);
    }
}

void CScriptRenderWindow::SetVisible(bool bVisible)
{
    if (m_hWnd != nullptr)
    {
        ShowWindow(m_hWnd, bVisible ? SW_SHOW : SW_HIDE);
    }
}