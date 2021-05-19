#include <stdafx.h>
#include "ScriptRenderWindow.h"
#include "DebuggerUI.h"

#define DEFAULT_FILLCOLOR   0xFFFFFFFF
#define DEFAULT_FONTFAMILY  "Courier New"
#define DEFAULT_FONTSIZE    14.0f
#define DEFAULT_STROKEWIDTH 0.0f
#define DEFAULT_STROKECOLOR 0x000000FF
#define DEFAULT_FONTWEIGHT  DWRITE_FONT_WEIGHT_NORMAL

CScriptRenderWindow::CScriptRenderWindow(CDebuggerUI* debugger) :
    m_Debugger(debugger),
    m_hWnd(nullptr),
    m_LibD2D1(nullptr),
    m_LibDWrite(nullptr),
    m_pfnD2D1CreateFactory(nullptr),
    m_pfnDWriteCreateFactory(nullptr),
    m_D2DFactory(nullptr),
    m_DWriteFactory(nullptr),
    m_Gfx(nullptr),
    m_GfxFillBrush(nullptr),
    m_GfxStrokeBrush(nullptr),
    m_GfxTextFormat(nullptr),
    m_Width(0),
    m_Height(0),
    m_TextRenderer(nullptr),
    m_FontSize(DEFAULT_FONTSIZE),
    m_FontFamilyName(DEFAULT_FONTFAMILY),
    m_FontWeight(DEFAULT_FONTWEIGHT),
    m_StrokeWidth(DEFAULT_STROKEWIDTH),
    m_FillColor(DEFAULT_FILLCOLOR),
    m_StrokeColor(DEFAULT_STROKECOLOR)
{
    GfxLoadLibs();
    GfxInitFactories();

    RegisterWinClass();
    Create();
}

CScriptRenderWindow::~CScriptRenderWindow()
{
    if (m_hWnd != nullptr)
    {
        DestroyWindow(m_hWnd);
    }

    if (m_TextRenderer != nullptr)
    {
        delete m_TextRenderer;
    }
    
    if (m_GfxTextFormat) m_GfxTextFormat->Release();
    if (m_GfxFillBrush) m_GfxFillBrush->Release();
    if (m_Gfx) m_Gfx->Release();
    if (m_DWriteFactory) m_DWriteFactory->Release();
    if (m_D2DFactory) m_D2DFactory->Release();

    GfxFreeLibs();
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
        _this->m_hWnd = hWnd;
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

    return _this->ScriptRenderWndProc(hWnd, uMsg, wParam, lParam);
}

void CScriptRenderWindow::CpuRunningChanged(void* p)
{
    CScriptRenderWindow* _this = (CScriptRenderWindow*)p;
    bool bRunning = g_Settings->LoadBool(GameRunning_CPU_Running);
    _this->SetVisible(bRunning);
}

//void CScriptRenderWindow::ToggleVisible()
//{
//    if(g_Settings->LoadBool(GameRunning_CPU_Running) &&
//       m_Debugger->ScriptSystem()->HaveCallbacks(JS_HOOK_GFXUPDATE) &&
//       IsWindowEnabled())
//}

void CScriptRenderWindow::LimitFPSChanged(void* /*p*/)
{
    // TODO should toggle vsync here
     
    //CScriptRenderWindow* _this = (CScriptRenderWindow*)p;
    //bool bLimitFPS = g_Settings->LoadBool(GameRunning_LimitFPS);
}

int CScriptRenderWindow::WinMouseMessageButton(DWORD uMsg)
{
    switch (uMsg)
    {
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
        return 0;
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
        return 1;
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
        return 2;
    }
    return -1;
}

void CScriptRenderWindow::ScreenMouseEventProc(HWND hWnd, DWORD uMsg, DWORD /*wParam*/, DWORD lParam)
{
    switch (uMsg)
    {
    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
        SetCapture(hWnd);
        m_Debugger->ScriptSystem()->DoMouseEvent(JS_HOOK_MOUSEDOWN,
            GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), WinMouseMessageButton(uMsg));
        break;
    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
        ReleaseCapture();
        m_Debugger->ScriptSystem()->DoMouseEvent(JS_HOOK_MOUSEUP,
            GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), WinMouseMessageButton(uMsg));
        break;
    case WM_MOUSEMOVE:
    {
        static int lastX = 0;
        static int lastY = 0;
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);
        if (lastX != x || lastY != y)
        {
            m_Debugger->ScriptSystem()->DoMouseEvent(JS_HOOK_MOUSEMOVE, x, y);
            lastX = x;
            lastY = y;
        }
    }
    break;
    }
}

void CScriptRenderWindow::MainWndHookProc(HWND hWnd, DWORD uMsg, DWORD wParam, DWORD lParam)
{
    ScreenMouseEventProc(hWnd, uMsg, wParam, lParam);
    
    switch (uMsg)
    {
    case WM_MOVE:
    case WM_SIZE:
    case WM_WINDOWPOSCHANGED:
        {
            CMainGui * mainWindow = (CMainGui *)GetProp(hWnd, L"Class");
            FixPosition(hWnd, (HWND)mainWindow->GetStatusBar());
        }
        break;
    case WM_ENABLE:
        if (!wParam)
        {
            SetVisible(false);
        }
        break;
    case WM_ACTIVATEAPP:
        if (wParam && g_Settings->LoadBool(GameRunning_CPU_Running) &&
            m_Debugger->ScriptSystem()->HaveCallbacks(JS_HOOK_GFXUPDATE))
        {
            SetVisible(true);
        }
        break;
    }
}

LRESULT CScriptRenderWindow::ScriptRenderWndProc(HWND hWnd, DWORD uMsg, DWORD wParam, DWORD lParam)
{
    ScreenMouseEventProc(hWnd, uMsg, wParam, lParam);

    switch (uMsg)
    {
    case WM_CREATE:
        g_Settings->RegisterChangeCB(GameRunning_CPU_Running, this, CpuRunningChanged);
        g_Settings->RegisterChangeCB(GameRunning_LimitFPS, this, LimitFPSChanged);
        GfxInitTarget();
        break;
    case WM_DESTROY:
        g_Settings->UnregisterChangeCB(GameRunning_CPU_Running, this, CpuRunningChanged);
        g_Settings->UnregisterChangeCB(GameRunning_LimitFPS, this, LimitFPSChanged);
        break;
    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
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

void CScriptRenderWindow::FixPosition(HWND hMainWnd, HWND hStatusWnd)
{
    if (m_hWnd != nullptr && hMainWnd != nullptr)
    {
        //if(IsWindowEnabled(m_hWnd))

        CRect mainRc, statusRc;
        GetClientRect(hMainWnd, &mainRc);
        ClientToScreen(hMainWnd, &mainRc.TopLeft());
        ClientToScreen(hMainWnd, &mainRc.BottomRight());

        GetClientRect(hStatusWnd, &statusRc);

        m_Width = mainRc.Width();
        m_Height = mainRc.Height() - statusRc.Height();

        SetWindowPos(m_hWnd, HWND_TOP, mainRc.left, mainRc.top, m_Width, m_Height, SWP_NOACTIVATE);

        if (m_Gfx)
        {
            m_Gfx->Resize(D2D1::SizeU(m_Width, m_Height));
        }
    }
}

void CScriptRenderWindow::SetVisible(bool bVisible)
{
    if (m_hWnd != nullptr)
    {
        ShowWindow(m_hWnd, bVisible ? SW_SHOWNOACTIVATE : SW_HIDE);
    }
}

bool CScriptRenderWindow::IsVisible()
{
    return m_hWnd ? (IsWindowVisible(m_hWnd) == TRUE) : false;
}

bool CScriptRenderWindow::CaptureWindowRGBA32(HWND hWnd, int width, int height, uint8_t* outRGBA32)
{
    size_t numBytes = width * height * 4;

    HDC hSrcDC = GetDC(hWnd);
    HDC hMemDC = CreateCompatibleDC(hSrcDC);
    HBITMAP hMemBitmap = CreateCompatibleBitmap(hSrcDC, width, height);
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemDC, hMemBitmap);

    BitBlt(hMemDC, 0, 0, width, height, hSrcDC, 0, 0, SRCCOPY);

    BITMAP bmp;
    GetObject(hMemBitmap, sizeof(bmp), &bmp);

    BITMAPINFO info = { 0 };
    info.bmiHeader.biSize = sizeof(info.bmiHeader);
    info.bmiHeader.biWidth = width;
    info.bmiHeader.biHeight = -height;
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    info.bmiHeader.biCompression = BI_RGB;

    GetDIBits(hMemDC, hMemBitmap, 0, height, outRGBA32, &info, DIB_RGB_COLORS);

    for (size_t i = 0; i < numBytes; i += 4)
    {
        *(uint32_t*)&outRGBA32[i] = _byteswap_ulong(*(uint32_t*)&outRGBA32[i] << 8);
        outRGBA32[i + 3] = 0xFF;
    }

    // TODO: Detect BitBlt error
    // SRCCOPY from main window doesn't work on Win7 unless a Direct3D GFX plugin is used
    bool bSucceeded = true;

    ReleaseDC(hWnd, hSrcDC);
    DeleteDC(hMemDC);
    DeleteObject(hMemBitmap);
    DeleteObject(hOldBitmap);

    return bSucceeded;
}

bool CScriptRenderWindow::GfxLoadLibs()
{
    m_LibD2D1 = LoadLibrary(L"d2d1.dll");
    m_LibDWrite = LoadLibrary(L"dwrite.dll");

    if (m_LibD2D1 == nullptr || m_LibDWrite == nullptr)
    {
        GfxFreeLibs();
        return false;
    }

    m_pfnD2D1CreateFactory = (LPFN_D2D1CF)GetProcAddress(m_LibD2D1, "D2D1CreateFactory");
    m_pfnDWriteCreateFactory = (LPFN_DWCF)GetProcAddress(m_LibDWrite, "DWriteCreateFactory");

    if (m_pfnD2D1CreateFactory == nullptr || m_pfnDWriteCreateFactory == nullptr)
    {
        GfxFreeLibs();
        return false;
    }

    return true;
}

void CScriptRenderWindow::GfxFreeLibs()
{
    m_pfnD2D1CreateFactory = nullptr;
    m_pfnDWriteCreateFactory = nullptr;

    if (m_LibD2D1 != nullptr)
    {
        FreeLibrary(m_LibD2D1);
        m_LibD2D1 = nullptr;
    }

    if (m_LibDWrite != nullptr)
    {
        FreeLibrary(m_LibDWrite);
        m_LibDWrite = nullptr;
    }
}

bool CScriptRenderWindow::GfxInitFactories()
{
    if (m_pfnD2D1CreateFactory == nullptr || m_pfnDWriteCreateFactory == nullptr)
    {
        return false;
    }

    HRESULT hr;
    hr = m_pfnD2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory), NULL, (void**)&m_D2DFactory);

    if (FAILED(hr))
    {
        return false;
    }

    hr = m_pfnDWriteCreateFactory(DWRITE_FACTORY_TYPE_ISOLATED, __uuidof(IDWriteFactory), (IUnknown**)&m_DWriteFactory);

    if (FAILED(hr))
    {
        return false;
    }

    return true;
}

bool CScriptRenderWindow::GfxInitTarget()
{
    if (m_D2DFactory && m_hWnd)
    {
        HRESULT hr;

        // TODO should use D2D1_PRESENT_OPTIONS_IMMEDIATELY when FPS limiter is off

        hr = m_D2DFactory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(m_hWnd, D2D1::SizeU(m_Width, m_Height), D2D1_PRESENT_OPTIONS_IMMEDIATELY),
            &m_Gfx);

        if (SUCCEEDED(hr) && m_Gfx)
        {
            //m_Gfx->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
            m_TextRenderer = new COutlinedTextRenderer(m_D2DFactory, m_Gfx, &m_GfxFillBrush, &m_GfxStrokeBrush);
            return true;
        }
    }

    return false;
}

void CScriptRenderWindow::GfxCopyWindow(HWND hSrcWnd)
{
    if (!m_Gfx)
    {
        return;
    }

    CRect rc;
    GetClientRect(hSrcWnd, &rc);
    float width = (float)rc.Width();
    float height = (float)rc.Height();
    std::vector<uint8_t> frameRGBA32((size_t)width * (size_t)height * 4);
    static bool bLoggedError = false;

    if (!CaptureWindowRGBA32(hSrcWnd, (int)width, (int)height, &frameRGBA32[0]) && !bLoggedError)
    {
        m_Debugger->Debug_LogScriptsWindow("[SCRIPTSYS]: Failed to sample game screen. A Direct3D graphics plugin may be required.\r\n");
        bLoggedError = true;
    }

    ID2D1Bitmap* bitmap;

    HRESULT hr = m_Gfx->CreateBitmap(
        D2D1::SizeU((uint32_t)width, (uint32_t)height),
        &frameRGBA32[0],
        width * 4,
        D2D1::BitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)),
        &bitmap);

    if (SUCCEEDED(hr) && bitmap != nullptr)
    {
        m_Gfx->DrawBitmap(
            bitmap,
            D2D1::RectF(0, 0, width, height),
            1.0f,
            D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR,
            D2D1::RectF(0, 0, width, height));

        bitmap->Release();
    }
}

void CScriptRenderWindow::GfxBeginDraw()
{
    if (!m_Gfx)
    {
        return;
    }

    GfxSetFillColor(DEFAULT_FILLCOLOR);
    GfxSetStrokeColor(DEFAULT_STROKECOLOR);
    GfxSetStrokeWidth(DEFAULT_STROKEWIDTH);
    GfxSetFont(stdstr(DEFAULT_FONTFAMILY).ToUTF16().c_str(), DEFAULT_FONTSIZE);

    m_Gfx->BeginDraw();
}

void CScriptRenderWindow::GfxEndDraw()
{
    if (!m_Gfx)
    {
        return;
    }

    m_Gfx->EndDraw();
}

void CScriptRenderWindow::GfxSetFillColor(uint32_t rgba)
{
    if (!m_Gfx)
    {
        return;
    }

    m_FillColor = rgba;
    D2D1::ColorF colorF = D2D1ColorFromRGBA32(m_FillColor);

    if (m_GfxFillBrush == nullptr)
    {
        m_Gfx->CreateSolidColorBrush(colorF, &m_GfxFillBrush);
    }

    m_GfxFillBrush->SetColor(colorF);
}

uint32_t CScriptRenderWindow::GfxGetFillColor()
{
    return m_FillColor;
}

void CScriptRenderWindow::GfxSetStrokeColor(uint32_t rgba)
{
    if (!m_Gfx)
    {
        return;
    }

    m_FillColor = rgba;
    D2D1::ColorF colorF = D2D1ColorFromRGBA32(m_FillColor);

    if (m_GfxStrokeBrush == nullptr)
    {
        m_Gfx->CreateSolidColorBrush(colorF, &m_GfxStrokeBrush);
    }

    m_GfxStrokeBrush->SetColor(colorF);
}

uint32_t CScriptRenderWindow::GfxGetStrokeColor()
{
    return m_StrokeColor;
}

void CScriptRenderWindow::GfxSetStrokeWidth(float strokeWidth)
{
    if (m_TextRenderer != nullptr)
    {
        m_StrokeWidth = strokeWidth;
        m_TextRenderer->SetStrokeWidth(strokeWidth);
    }
}

float CScriptRenderWindow::GfxGetStrokeWidth()
{
    return m_StrokeWidth;
}

void CScriptRenderWindow::GfxRefreshTextFormat()
{
    if (!m_DWriteFactory)
    {
        return;
    }

    if (m_GfxTextFormat != nullptr)
    {
        m_GfxTextFormat->Release();
        m_GfxTextFormat = nullptr;
    }

    m_DWriteFactory->CreateTextFormat(
        m_FontFamilyName.ToUTF16().c_str(),
        nullptr,
        m_FontWeight,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        m_FontSize,
        L"",
        &m_GfxTextFormat);
}

void CScriptRenderWindow::GfxSetFont(const wchar_t* fontFamily, float fontSize)
{
    m_FontFamilyName = stdstr().FromUTF16(fontFamily);
    m_FontSize = fontSize;
    GfxRefreshTextFormat();
}

void CScriptRenderWindow::GfxSetFontFamily(const wchar_t* fontFamily)
{
    m_FontFamilyName = stdstr().FromUTF16(fontFamily);
    GfxRefreshTextFormat();
}

stdstr CScriptRenderWindow::GfxGetFontFamily()
{
    return m_FontFamilyName;
}

void CScriptRenderWindow::GfxSetFontSize(float fontSize)
{
    m_FontSize = fontSize;
    GfxRefreshTextFormat();
}

float CScriptRenderWindow::GfxGetFontSize()
{
    return m_FontSize;
}

void CScriptRenderWindow::GfxSetFontWeight(DWRITE_FONT_WEIGHT fontWeight)
{
    m_FontWeight = fontWeight;
    GfxRefreshTextFormat();
}

DWRITE_FONT_WEIGHT CScriptRenderWindow::GfxGetFontWeight()
{
    return m_FontWeight;
}

void CScriptRenderWindow::GfxDrawText(float x, float y, const wchar_t* text)
{
    if (!m_Gfx)
    {
        return;
    }

    if (m_StrokeWidth > 0.0f)
    {
        IDWriteTextLayout* textLayout = nullptr;
        HRESULT hr = m_DWriteFactory->CreateTextLayout(text, wcslen(text), m_GfxTextFormat, (float)m_Width, (float)m_Height, &textLayout);
        if (SUCCEEDED(hr))
        {
            textLayout->Draw(nullptr, m_TextRenderer, x, y);
            textLayout->Release();
        }
    }
    else
    {
        m_Gfx->DrawText(text, wcslen(text), m_GfxTextFormat, D2D1::RectF(x, y, (float)m_Width, (float)m_Height), m_GfxFillBrush);
    }
}

void CScriptRenderWindow::GfxFillRect(float left, float top, float right, float bottom)
{
    if (!m_Gfx)
    {
        return;
    }

    m_Gfx->FillRectangle(D2D1::RectF(left, top, right, bottom), m_GfxFillBrush);
}

void CScriptRenderWindow::GfxStrokeRect(float left, float top, float right, float bottom)
{
    if (!m_Gfx)
    {
        return;
    }

    m_Gfx->DrawRectangle(D2D1::RectF(left, top, right, bottom), m_GfxStrokeBrush, m_StrokeWidth);
}

int CScriptRenderWindow::GetWidth()
{
    return m_Width;
}

int CScriptRenderWindow::GetHeight()
{
    return m_Height;
}

D2D1::ColorF CScriptRenderWindow::D2D1ColorFromRGBA32(uint32_t color)
{
    float r = ((color >> 24) & 0xFF) / 255.0f;
    float g = ((color >> 16) & 0xFF) / 255.0f;
    float b = ((color >>  8) & 0xFF) / 255.0f;
    float a = ((color >>  0) & 0xFF) / 255.0f;
    return D2D1::ColorF(r, g, b, a);
}

void CScriptRenderWindow::GfxBeginPath()
{
    m_Paths.clear();
}

void CScriptRenderWindow::GfxMoveTo(float x, float y)
{
    pointpath_t path({ D2D1::Point2F(x, y) });
    m_Paths.push_back(path);
}

void CScriptRenderWindow::GfxLineTo(float x, float y)
{
    pointpath_t& path = m_Paths.back();
    path.push_back({ D2D1::Point2F(x, y) });
}

void CScriptRenderWindow::GfxStroke()
{
    for (size_t nPath = 0; nPath < m_Paths.size(); nPath++)
    {
        pointpath_t& path = m_Paths[nPath];

        if (path.size() < 2)
        {
            continue;
        }

        for (size_t nPoint = 0; nPoint < path.size() - 1; nPoint++)
        {
            D2D1_POINT_2F& pointA = path[nPoint];
            D2D1_POINT_2F& pointB = path[nPoint + 1];
            
            m_Gfx->DrawLine(
                D2D1::Point2F(pointA.x, pointA.y),
                D2D1::Point2F(pointB.x, pointB.y),
                m_GfxStrokeBrush,
                m_StrokeWidth);
        }
    }
}

// todo make sure this implementation is similar to web CanvasRenderingContext2D.fill
void CScriptRenderWindow::GfxFill()
{
    ID2D1PathGeometry* pathGeometry;
    ID2D1GeometrySink* sink;

    if (FAILED(m_D2DFactory->CreatePathGeometry(&pathGeometry)))
    {
        return;
    }

    if (FAILED(pathGeometry->Open(&sink)))
    {
        return;
    }

    for (size_t nPath = 0; nPath < m_Paths.size(); nPath++)
    {
        pointpath_t& path = m_Paths[nPath];
        if (path.size() < 3)
        {
            continue;
        }
        sink->BeginFigure(path[0], D2D1_FIGURE_BEGIN_FILLED);

        for (size_t nPoint = 1; nPoint < path.size(); nPoint++)
        {
            sink->AddLine(path[nPoint]);
        }

        sink->EndFigure(D2D1_FIGURE_END_CLOSED);
    }

    sink->Close();

    m_Gfx->FillGeometry(pathGeometry, m_GfxFillBrush);
    
    sink->Release();
    pathGeometry->Release();
}