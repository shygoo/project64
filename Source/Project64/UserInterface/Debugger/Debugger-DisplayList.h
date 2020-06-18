#pragma once

#include <stdafx.h>
#include "DebuggerUI.h"
#include "GfxParser.h"
#include "GfxRenderer.h"

class CCanvasItem
{
public:
    int      m_X, m_Y;
    CRect    m_BoundingRect;
	COLORREF m_Color;
    bool     m_bHaveBgColor;
    COLORREF m_BgColor;
	//bool     m_bNeedRedraw;
	TCHAR*   m_Text;
	CCanvasItem(int x, int y, const TCHAR* text, COLORREF color = RGB(255, 255, 255));
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
	COLORREF m_BackgroundColor;
	COLORREF m_ForegroundColor;
    int m_OriginX;
    int m_OriginY;
    int m_ItemPosX;
    int m_ItemPosY;
    int m_LayoutMode;

    enum {
        LAYOUT_VERTICAL = 1,
        LAYOUT_HORIZONTAL = 2
    };

    void GotoOriginX(void);
    void SetLayoutMode(int mode);
    void SetOrigin(int x, int y);
	void SetItemText(int nItem, const TCHAR* text);
	void SetItemColor(int nItem, COLORREF color);
    void SetItemBgColor(int nItem, COLORREF color);
	void Init(void);
	size_t AddItem(const TCHAR* text, COLORREF color = RGB(255, 255, 255));

private:
	CCanvasItem* GetItem(int index);
	void DrawItem(CCanvasItem* item);
	std::vector<CCanvasItem*> m_Items;
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        int x, y;
        x = GET_X_LPARAM(lParam);
        y = GET_Y_LPARAM(lParam);
        printf("%d, %d\n", x, y);
        return FALSE;
    }

	BEGIN_MSG_MAP(CCanvas)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
        MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
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
        //DisplayListCtrl_Col_RawCommand,
        DisplayListCtrl_Col_Command,
        DisplayListCtrl_Col_Parameters
    };

    CDebugDisplayList(CDebuggerUI * debugger);
    virtual ~CDebugDisplayList(void);

    void Refresh(void);

private:
    int
        m_ITM_TEXTUREIMAGE,
        m_ITM_COLORIMAGE,
        m_ITM_DEPTHIMAGE,
        m_ITM_GM_ZBUFFER,
        m_ITM_GM_SHADE,
        m_ITM_GM_FOG,
        m_ITM_GM_LIGHTING,
        m_ITM_GM_TEXTURE_GEN,
        m_ITM_GM_TEXTURE_GEN_LINEAR,
        m_ITM_GM_LOD,
        m_ITM_GM_CLIPPING,
        m_ITM_GM_SHADING_SMOOTH,
        m_ITM_GM_CULL_FRONT,
        m_ITM_GM_CULL_BACK,
        m_ITM_CC1_COLOR,
        m_ITM_CC1_ALPHA,
        m_ITM_CC2_COLOR,
        m_ITM_CC2_ALPHA,
        m_ITM_OMH_PIPELINE,
        m_ITM_OMH_COLORDITHER,
        m_ITM_OMH_CYCLETYPE,
        m_ITM_OMH_TEXTPERSP,
        m_ITM_OMH_TEXTDETAIL,
        m_ITM_OMH_TEXTLOD,
        m_ITM_OMH_TEXTLUT,
        m_ITM_OMH_TEXTFILT,
        m_ITM_OMH_TEXTCONV,
        m_ITM_OMH_COMBKEY,
        m_ITM_OMH_RGBDITHER,
        m_ITM_OMH_ALPHADITHER,
        m_ITM_FILLCOLOR,
        m_ITM_FOGCOLOR,
        m_ITM_BLENDCOLOR,
        m_ITM_PRIMCOLOR,
        m_ITM_ENVCOLOR,
        m_ITM_FILLCOLOR_PREV0,
        m_ITM_FILLCOLOR_PREV1,
        m_ITM_FOGCOLOR_PREV,
        m_ITM_BLENDCOLOR_PREV,
        m_ITM_PRIMCOLOR_PREV,
        m_ITM_ENVCOLOR_PREV,
        m_ITM_OML_AA_EN,
        m_ITM_OML_Z_CMP,
        m_ITM_OML_Z_UPD,
        m_ITM_OML_IM_RD,
        m_ITM_OML_CLR_ON_CVG,
        m_ITM_OML_CVG_DST,
        m_ITM_OML_ZMODE,
        m_ITM_OML_CVG_X_ALPHA,
        m_ITM_OML_ALPHA_CVG_SEL,
        m_ITM_OML_FORCE_BL,
        m_ITM_OML_CYCLE1,
        m_ITM_OML_CYCLE2;

    int m_ITM_LIGHTS[10];
	int m_ITM_TILES[8];

    bool m_bRefreshPending;
    bool m_bShowRender;
    
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
    
	void UpdateStateCanvas(CHleGfxState* state);
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
    END_DLGRESIZE_MAP()

    BEGIN_TOOLTIP_MAP()
    END_TOOLTIP_MAP()
};
