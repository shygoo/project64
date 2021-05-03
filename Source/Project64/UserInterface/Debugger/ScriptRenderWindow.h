#pragma once

#include <stdafx.h>
#include <Project64-core/Plugins/Plugin.h>
#include <Project64-core/N64System/N64System.h>

#include <d2d1.h>
#include <dwrite.h>

typedef HRESULT(CALLBACK* LPFN_D2D1CF)(D2D1_FACTORY_TYPE, REFIID, const D2D1_FACTORY_OPTIONS*, void**);
typedef HRESULT(CALLBACK* LPFN_DWCF)(DWRITE_FACTORY_TYPE, REFIID, IUnknown**);

class COutlinedTextRenderer;

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
    ID2D1SolidColorBrush*  m_GfxStrokeBrush;
    IDWriteTextFormat*     m_GfxTextFormat;
    IDWriteTextLayout*     m_GfxTextLayout;
    int                    m_Width;
    int                    m_Height;

    stdstr                 m_FontFamilyName;
    float                  m_FontSize;
    DWRITE_FONT_WEIGHT     m_FontWeight;

    float                  m_StrokeWidth;
    uint32_t               m_FillColor;
    uint32_t               m_StrokeColor;

    COutlinedTextRenderer* m_TextRenderer;

    typedef std::vector<D2D1_POINT_2F> pointpath_t;
    std::vector<pointpath_t> m_Paths;

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
    void GfxSetFillColor(uint32_t rgba);
    uint32_t GfxGetFillColor();
    void GfxSetStrokeColor(uint32_t rgba);
    uint32_t GfxGetStrokeColor();
    void GfxSetStrokeWidth(float strokeWidth);
    float GfxGetStrokeWidth();
    void GfxSetFont(const wchar_t* fontFamily, float fontSize);
    void GfxSetFontFamily(const wchar_t* fontFamily);
    stdstr GfxGetFontFamily();
    void GfxSetFontSize(float fontSize);
    float GfxGetFontSize();
    void GfxSetFontWeight(DWRITE_FONT_WEIGHT fontWeight);
    DWRITE_FONT_WEIGHT GfxGetFontWeight();
    void GfxDrawText(float x, float y, const wchar_t* text);
    void GfxFillRect(float left, float top, float right, float bottom);
    void GfxCopyWindow(HWND hSrcWnd);

    void GfxBeginPath();
    void GfxMoveTo(float x, float y);
    void GfxLineTo(float x, float y);
    void GfxStroke();

    int GetWidth();
    int GetHeight();

    void FixPosition(HWND hMainWnd, HWND hStatusWnd);
    void SetVisible(bool bVisible);
    bool IsVisible();

    static void CpuRunningChanged(void* p);
    static void LimitFPSChanged(void* p);
    static bool CaptureWindowRGBA32(HWND hWnd, int width, int height, uint8_t* outRGBA32);
    static D2D1::ColorF D2D1ColorFromRGBA32(uint32_t color);
};


class COutlinedTextRenderer :
    public IDWriteTextRenderer
{
    unsigned long m_RefCount;

    ID2D1Factory*          m_D2DFactory;
    ID2D1HwndRenderTarget* m_Gfx;

    ID2D1SolidColorBrush**  m_StrokeBrush;
    ID2D1SolidColorBrush**  m_FillBrush;
    float                   m_StrokeWidth;

public:
    COutlinedTextRenderer(
        ID2D1Factory* d2dFactory,
        ID2D1HwndRenderTarget* gfx,
        ID2D1SolidColorBrush** fillBrush,
        ID2D1SolidColorBrush** strokeBrush)
        :
        m_D2DFactory(d2dFactory),
        m_Gfx(gfx),
        m_FillBrush(fillBrush),
        m_StrokeBrush(strokeBrush),
        m_StrokeWidth(2.0f)
    {
    }

    ~COutlinedTextRenderer()
    {
    }

    void SetStrokeWidth(float strokeWidth)
    {
        m_StrokeWidth = strokeWidth;
    }

    IFACEMETHOD(DrawGlyphRun)(
        void*                               /*clientDrawingContext*/,
        FLOAT                               baselineOriginX,
        FLOAT                               baselineOriginY,
        DWRITE_MEASURING_MODE               /*measuringMode*/,
        DWRITE_GLYPH_RUN const*             glyphRun,
        DWRITE_GLYPH_RUN_DESCRIPTION const* /*glyphRunDescription*/,
        IUnknown*                           /*clientDrawingEffect*/)
    {
        HRESULT hr;

        ID2D1PathGeometry* pathGeometry = nullptr;
        ID2D1GeometrySink* geometrySink = nullptr;
        ID2D1TransformedGeometry* transformedGeometry = nullptr;

        hr = m_D2DFactory->CreatePathGeometry(&pathGeometry);

        if (FAILED(hr))
        {
            goto cleanup;
        }

        hr = pathGeometry->Open(&geometrySink);

        if (FAILED(hr))
        {
            goto cleanup;
        }

        hr = glyphRun->fontFace->GetGlyphRunOutline(
            glyphRun->fontEmSize,
            glyphRun->glyphIndices,
            glyphRun->glyphAdvances,
            glyphRun->glyphOffsets,
            glyphRun->glyphCount,
            glyphRun->isSideways,
            FALSE,
            geometrySink);

        if (FAILED(hr))
        {
            goto cleanup;
        }

        geometrySink->Close();

        D2D1::Matrix3x2F matrix = D2D1::Matrix3x2F(1, 0, 0, 1, baselineOriginX, baselineOriginY);

        hr = m_D2DFactory->CreateTransformedGeometry(pathGeometry, &matrix, &transformedGeometry);

        if (FAILED(hr))
        {
            goto cleanup;
        }

        m_Gfx->DrawGeometry(transformedGeometry, *m_StrokeBrush, m_StrokeWidth);
        m_Gfx->FillGeometry(transformedGeometry, *m_FillBrush);

    cleanup:
        if (pathGeometry != nullptr)
        {
            pathGeometry->Release();
        }

        if (geometrySink != nullptr)
        {
            geometrySink->Release();
        }

        if (transformedGeometry != nullptr)
        {
            transformedGeometry->Release();
        }

        return hr;
    }

    IFACEMETHOD(DrawStrikethrough)(
        void*                       /*clientDrawingContext*/,
        FLOAT                       /*baselineOriginX*/,
        FLOAT                       /*baselineOriginY*/,
        DWRITE_STRIKETHROUGH const* /*strikethrough*/,
        IUnknown*                   /*clientDrawingEffect*/)
    {
        return E_NOTIMPL;
    }

    IFACEMETHOD(DrawUnderline)(
        void*                   /*clientDrawingContext*/,
        FLOAT                   /*baselineOriginX*/,
        FLOAT                   /*baselineOriginY*/,
        DWRITE_UNDERLINE const* /*underline*/,
        IUnknown*               /*clientDrawingEffect*/)
    {
        return E_NOTIMPL;
    }

    IFACEMETHOD(DrawInlineObject)(
        void*                /*clientDrawingContext*/,
        FLOAT                /*originX*/,
        FLOAT                /*originY*/,
        IDWriteInlineObject* /*inlineObject*/,
        BOOL                 /*isSideways*/,
        BOOL                 /*isRightToLeft*/,
        IUnknown*            /*clientDrawingEffect*/)
    {
        return E_NOTIMPL;
    }

    IFACEMETHOD(IsPixelSnappingDisabled)(void* /*clientDrawingContext*/, BOOL* isDisabled)
    {
        *isDisabled = FALSE;
        return S_OK;
    }

    IFACEMETHOD(GetCurrentTransform)(void* /*clientDrawingContext*/, DWRITE_MATRIX* transform)
    {
        m_Gfx->GetTransform(reinterpret_cast<D2D1_MATRIX_3X2_F*>(transform));
        return S_OK;
    }

    IFACEMETHOD(GetPixelsPerDip)(void* /*clientDrawingContext*/, FLOAT* pixelsPerDip)
    {
        float x, y;
        m_Gfx->GetDpi(&x, &y);
        *pixelsPerDip = x / 96;
        return S_OK;
    }

    IFACEMETHOD_(unsigned long, AddRef)()
    {
        return InterlockedIncrement(&m_RefCount);
    }

    IFACEMETHOD_(unsigned long, Release)()
    {
        unsigned long newCount = InterlockedDecrement(&m_RefCount);

        if (newCount == 0)
        {
            delete this;
            return 0;
        }

        return newCount;
    }

    IFACEMETHOD(QueryInterface)(IID const& riid, void** ppvObject)
    {
        if (__uuidof(IDWriteTextRenderer) == riid ||
            __uuidof(IDWritePixelSnapping) == riid ||
            __uuidof(IUnknown) == riid)
        {
            *ppvObject = this;
        }
        else
        {
            *ppvObject = nullptr;
            return E_FAIL;
        }

        this->AddRef();

        return S_OK;
    }
};
