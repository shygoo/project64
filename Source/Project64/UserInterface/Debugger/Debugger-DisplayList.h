#pragma once

#include <stdafx.h>
#include "DebuggerUI.h"
#include "GfxParser.h"
#include "GfxRenderer.h"

class CCanvasItem
{
public:
    COLORREF    m_Color;
    bool        m_bNeedRedraw;
    char* m_Text;
    CRect m_BoundingRect;
    int m_X, m_Y;
    CCanvasItem(int x, int y, const char* text, COLORREF color);
};

class CCanvas :
    public CWindowImpl<CCanvas>
{
public:
    CCanvas(void);
    DECLARE_WND_CLASS(_T("Canvas"))
    void RegisterClass();
    HBITMAP m_BackBMP;
    HDC     m_BackDC;
    HFONT   m_Font;

    void SetItemText(int nItem, const char* text);
    void SetItemColor(int nItem, COLORREF color);
    //void SetTextColor(COLORREF color);
    void Text(int x, int y, const char* text);
    void Init(void);
    int AddItem(int x, int y, const char* text, COLORREF color = 0);

private:
    void DrawItem(CCanvasItem* item);
    std::vector<CCanvasItem*> m_Items;
    //CCanvasItem m_Items;
    LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    //LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    //LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    BEGIN_MSG_MAP(CCanvas)
        MESSAGE_HANDLER(WM_PAINT, OnPaint)
        //MESSAGE_HANDLER(WM_CREATE, OnCreate)
        //MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
    END_MSG_MAP()
};

enum
{
    RVN_LCLICK,
    RVN_RCLICK,
    RVN_MOUSEMOVE
};

typedef struct
{
    NMHDR nmh;
    int x, y;
} NMRVCLICK;

typedef struct
{
    NMHDR nmh;
    int x, y;
    int deltaX, deltaY;
    WORD buttons;
} NMRVMOUSEMOVE;

class CRenderView :
    public CWindowImpl<CRenderView>
{
private:
    std::set<int> m_KeysDown;
    bool m_bRButtonDown;
    int m_CursorX;
    int m_CursorY;
public:
    CRenderView(void);

    DECLARE_WND_CLASS(_T("RenderView"))
    void RegisterClass();
    
    LRESULT OnGetDlgCode(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnKeyUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnRButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnRButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    void DrawImage(CDrawBuffers *db);
    bool KeyDown(int vkey);
    bool RButtonDown(void);
    
    BEGIN_MSG_MAP(CRenderView)
        MESSAGE_HANDLER(WM_GETDLGCODE, OnGetDlgCode)
        MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
        MESSAGE_HANDLER(WM_KEYUP, OnKeyUp)
        MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
        MESSAGE_HANDLER(WM_RBUTTONDOWN, OnRButtonDown)
        MESSAGE_HANDLER(WM_RBUTTONUP, OnRButtonUp)
        MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
    END_MSG_MAP()
};

class CDebugDisplayList :
    public CDebugDialog<CDebugDisplayList>,
    public CDialogResize<CDebugDisplayList>,
    public CToolTipDialog<CDebugDisplayList>
{
public:
    enum { IDD = IDD_Debugger_DisplayList };
    enum { TIMER_ID_DRAW };
    enum {
        DisplayListCtrl_Col_VAddr,
        DisplayListCtrl_Col_SegOffset,
        DisplayListCtrl_Col_RawCommand,
        DisplayListCtrl_Col_Command,
        DisplayListCtrl_Col_Parameters
    };

    CDebugDisplayList(CDebuggerUI * debugger);
    virtual ~CDebugDisplayList(void);

    void Refresh(void);

private:
    int m_ItemTextureImage;
    int m_ItemColorImage;
    int m_ItemDepthImage;
    int m_ItemGeomZBUFFER;
    int m_ItemGeomSHADE;
    int m_ItemGeomFOG;
    int m_ItemGeomLIGHTING;
    int m_ItemGeomTEXTURE_GEN;
    int m_ItemGeomTEXTURE_GEN_LINEAR;
    int m_ItemGeomLOD;
    int m_ItemGeomCLIPPING;
    int m_ItemGeomSHADING_SMOOTH;
    int m_ItemGeomCULL_FRONT;
    int m_ItemGeomCULL_BACK;

    int m_ItemCC1Color;
    int m_ItemCC1Alpha;
    int m_ItemCC2Color;
    int m_ItemCC2Alpha;

    bool m_bRefreshPending;
    bool m_bShowRender;
    //DWORD m_ThreadId;
    //static HHOOK hWinMessageHook;
    //static CDebugDisplayList* _this;
    //static LRESULT CALLBACK HookProc(int nCode, WPARAM wParam, LPARAM lParam);
    
    CGfxParser    m_GfxParser;

    CRenderView*  m_RenderView;
    CCanvas*      m_StateCanvas;
    CListViewCtrl m_DisplayListCtrl;
    CEdit         m_StateTextbox;
    CStatic       m_StatusText;
    CTreeViewCtrl m_ResourceTreeCtrl;
    CImageList    m_TreeImageList;

    //CScene m_Scene;
    //CBasicMeshGeometry geom; // TEMPORARY

    HTREEITEM m_hTreeDisplayLists;
    HTREEITEM m_hTreeSegments;
    HTREEITEM m_hTreeImages;
    HTREEITEM m_hTreeVertices;
    HTREEITEM m_hTreeMatrices;
    HTREEITEM m_hTreeViewports;
    HTREEITEM m_hTreeLights;

    CRect m_OrgTexPreviewRect;
    CRect m_OrgResInfoRect;
    
    void ResetResourceTreeCtrl(void);
    void SetPreviewColor(WORD ctrlId, uint32_t colorPair);

    void PreviewImageResource(dram_resource_t* res);

    LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnDestroy(void);
    LRESULT OnClicked(WORD wNotifyCode, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled);
    LRESULT OnListItemChanged(NMHDR* pNMHDR);
    LRESULT OnListDblClicked(NMHDR* pNMHDR);
    LRESULT OnResourceTreeSelChanged(NMHDR* pNMHDR);
    LRESULT OnCustomDrawList(NMHDR* pNMHDR);
    LRESULT OnMeasureItem(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
    void    OnTimer(UINT_PTR nIDEvent);

    LRESULT OnRenderViewClicked(NMHDR* pNMHDR);
    LRESULT OnRenderViewMouseMove(NMHDR* pNMHDR);

    void Export(void);

    CDrawBuffers *m_DrawBuffers;
    CCamera m_Camera;

    //void    OnExitSizeMove(void);
    //void Export(void);

    BEGIN_MSG_MAP_EX(CDebugDisplayList)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_CODE_HANDLER(BN_CLICKED, OnClicked)
        NOTIFY_HANDLER_EX(IDC_LST_DLIST, LVN_ITEMCHANGED, OnListItemChanged)
        NOTIFY_HANDLER_EX(IDC_LST_DLIST, NM_DBLCLK, OnListDblClicked)
        NOTIFY_HANDLER_EX(IDC_TREE_RESOURCES, TVN_SELCHANGED, OnResourceTreeSelChanged)
        NOTIFY_HANDLER_EX(IDC_LST_DLIST, NM_CUSTOMDRAW, OnCustomDrawList)
        MESSAGE_HANDLER(WM_MEASUREITEM, OnMeasureItem)
        MSG_WM_TIMER(OnTimer)

        NOTIFY_HANDLER_EX(IDC_CUSTOM1, RVN_LCLICK, OnRenderViewClicked)
        NOTIFY_HANDLER_EX(IDC_CUSTOM1, RVN_MOUSEMOVE, OnRenderViewMouseMove)

        //MSG_WM_EXITSIZEMOVE(OnExitSizeMove)
        CHAIN_MSG_MAP(CDialogResize<CDebugDisplayList>)
    END_MSG_MAP()

    BEGIN_DLGRESIZE_MAP(CDebugDisplayList)
        DLGRESIZE_CONTROL(IDC_LST_DLIST, DLSZ_SIZE_X | DLSZ_SIZE_Y)
        DLGRESIZE_CONTROL(IDC_BTN_REFRESH, DLSZ_MOVE_X)
        DLGRESIZE_CONTROL(IDC_BTN_EXPORT, DLSZ_MOVE_X)
        DLGRESIZE_CONTROL(IDC_GRP_STATE, DLSZ_SIZE_X | DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_EDIT_STATE, DLSZ_SIZE_X | DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_GRP_RESOURCES, DLSZ_MOVE_X | DLSZ_SIZE_Y)
        DLGRESIZE_CONTROL(IDC_TREE_RESOURCES, DLSZ_MOVE_X | DLSZ_SIZE_Y)
        DLGRESIZE_CONTROL(IDC_EDIT_RESINFO, DLSZ_MOVE_X | DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_CUSTOM1, DLSZ_MOVE_X | DLSZ_MOVE_Y)

        DLGRESIZE_CONTROL(IDC_GRP_GEOMODE, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_GRP_OTHERMODEH, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_GRP_CC1, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_GRP_CC2, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_GRP_IMGADDRS, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_GRP_COLORS, DLSZ_MOVE_Y)

        DLGRESIZE_CONTROL(IDC_CHK_GM_ZBUFFER, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_CHK_GM_SHADE, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_CHK_GM_CULL_FRONT, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_CHK_GM_CULL_BACK, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_CHK_GM_FOG, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_CHK_GM_LIGHTING, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_CHK_GM_TEXTURE_GEN, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_CHK_GM_TEXTURE_GEN_LINEAR, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_CHK_GM_SHADING_SMOOTH, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_CHK_GM_LOD, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_CHK_GM_CLIPPING, DLSZ_MOVE_Y)

        DLGRESIZE_CONTROL(IDC_EDIT_OMH_PIPELINE, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_EDIT_OMH_COLORDITHER, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_EDIT_OMH_CYCLETYPE, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_EDIT_OMH_TEXTDETAIL, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_EDIT_OMH_TEXTPERSP, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_EDIT_OMH_TEXTLOD, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_EDIT_OMH_TEXTLUT, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_EDIT_OMH_TEXTFILT, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_EDIT_OMH_TEXTCONV, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_EDIT_OMH_COMBKEY, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_EDIT_OMH_RGBDITHER, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_EDIT_OMH_ALPHADITHER, DLSZ_MOVE_Y)

        DLGRESIZE_CONTROL(IDC_EDIT_CC_1C, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_EDIT_CC_1A, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_EDIT_CC_2C, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_EDIT_CC_2A, DLSZ_MOVE_Y)

        DLGRESIZE_CONTROL(IDC_EDIT_TEXTUREIMAGE, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_EDIT_COLORIMAGE, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_EDIT_DEPTHIMAGE, DLSZ_MOVE_Y)

        DLGRESIZE_CONTROL(IDC_EDIT_FILLCOLOR, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_EDIT_FOGCOLOR, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_EDIT_BLENDCOLOR, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_EDIT_PRIMCOLOR, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_EDIT_ENVCOLOR, DLSZ_MOVE_Y)

        DLGRESIZE_CONTROL(IDC_PREVIEW_FILLCOLOR, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_PREVIEW_FOGCOLOR, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_PREVIEW_BLENDCOLOR, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_PREVIEW_PRIMCOLOR, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_PREVIEW_ENVCOLOR, DLSZ_MOVE_Y)

        DLGRESIZE_CONTROL(IDC_LBL_PIPELINE, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_LBL_COLORDITHER, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_LBL_CYCLETYPE, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_LBL_TEXTDETAIL, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_LBL_TEXTPERSP, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_LBL_TEXTLOD, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_LBL_TEXTLUT, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_LBL_TEXTFILT, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_LBL_TEXTCONV, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_LBL_COMBKEY, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_LBL_RGBDITHER, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_LBL_ALPHADITHER, DLSZ_MOVE_Y)

        DLGRESIZE_CONTROL(IDC_LBL_COLOR1, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_LBL_ALPHA1, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_LBL_COLOR2, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_LBL_ALPHA2, DLSZ_MOVE_Y)

        DLGRESIZE_CONTROL(IDC_LBL_TEXTURE, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_LBL_COLORBUFFER, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_LBL_DEPTHBUFFER, DLSZ_MOVE_Y)

        DLGRESIZE_CONTROL(IDC_LBL_FILL, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_LBL_FOG, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_LBL_BLEND, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_LBL_PRIM, DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_LBL_ENV, DLSZ_MOVE_Y)
    END_DLGRESIZE_MAP()

    BEGIN_TOOLTIP_MAP()
    END_TOOLTIP_MAP()
};
