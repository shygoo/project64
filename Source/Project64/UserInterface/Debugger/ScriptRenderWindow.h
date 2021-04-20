#pragma once

#include <stdafx.h>
#include <Project64-core/Plugins/Plugin.h>
#include <Project64-core/N64System/N64System.h>

#include <d2d1.h>
#include <dwrite.h>

typedef HRESULT(CALLBACK* LPFN_D2D1CF)(D2D1_FACTORY_TYPE, REFIID, const D2D1_FACTORY_OPTIONS*, void**);
typedef HRESULT(CALLBACK* LPFN_DWCF)(DWRITE_FACTORY_TYPE, REFIID, IUnknown**);

class CScriptRenderWindow
{
private:
    HWND m_hWnd;
    bool Create();
    bool RegisterWinClass();
    static LRESULT CALLBACK ScriptRenderWindow_Proc(HWND hWnd, DWORD uMsg, DWORD wParam, DWORD lParam);
    LRESULT Proc(HWND hWnd, DWORD uMsg, DWORD wParam, DWORD lParam);

    CDebuggerUI*           m_Debugger;

    HMODULE                m_LibD2D1;
    HMODULE                m_LibDWrite;
    LPFN_D2D1CF            m_pfnD2D1CreateFactory;
    LPFN_DWCF              m_pfnDWriteCreateFactory;
    ID2D1Factory*          m_D2DFactory;
    IDWriteFactory*        m_DWriteFactory;
    ID2D1HwndRenderTarget* m_Gfx;
    ID2D1SolidColorBrush*  m_GfxFillBrush;
    IDWriteTextFormat*     m_GfxTextFormat;
    int                    m_Width;
    int                    m_Height;

    stdstr                 m_FontFamilyName;
    float                  m_FontSize;

    bool GfxLoadLibs();
    void GfxFreeLibs();
    bool GfxInitFactories();
    bool GfxInitTarget();
    void GfxRefreshTextFormat();

public:
    CScriptRenderWindow(CDebuggerUI* debugger);
    ~CScriptRenderWindow();

    HWND GetWindowHandle(void);
    
    void GfxBeginDraw();
    void GfxEndDraw();
    void GfxSetFillColor(float r, float g, float b, float alpha = 1.0f);

    void GfxSetFont(const wchar_t* fontFamily, float fontSize);

    void GfxSetFontFamily(const wchar_t* fontFamily);
    void GfxSetFontSize(float fontSize);

    void GfxDrawText(float x, float y, const wchar_t* text);
    void GfxFillRect(float left, float top, float right, float bottom);
    void GfxCopyWindow(HWND hSrcWnd);

    int GetWidth();
    int GetHeight();

    void FixPosition(HWND hMainWnd);
    void SetVisible(bool bVisible);
    bool IsVisible();

    static void CpuRunningChanged(void* p);
    static void LimitFPSChanged(void* p);
    static void CaptureWindowRGBA32(HWND hWnd, int width, int height, uint8_t* outRGBA32);
};
