#include "stdafx.h"
#include "DebuggerUI.h"

#include "Util/GfxParser.h"
#include "Util/GfxRenderer.h"
#include "Util/GfxLabels.h"

CRenderView::CRenderView() :
    m_bRButtonDown(false),
    m_CursorX(0),
    m_CursorY(0)
{
}

bool CRenderView::KeyDown(int vkey)
{
    return m_KeysDown.count(vkey);
}

bool CRenderView::RButtonDown(void)
{
    return m_bRButtonDown;
}

void CRenderView::RegisterClass()
{
    this->GetWndClassInfo().m_wc.lpfnWndProc = m_pfnSuperWindowProc;
    this->GetWndClassInfo().Register(&m_pfnSuperWindowProc);
}

void CRenderView::DrawImage(CDrawBuffers *db)
{
    HDC hdc = GetDC();
    HBITMAP hbm = CreateBitmap(db->m_Width, db->m_Height, 1, 32, db->m_ColorBuffer);
    HDC hdcMem = CreateCompatibleDC(hdc);
    SelectObject(hdcMem, hbm);
    BitBlt(hdc, 1, 1, db->m_Width, db->m_Height, hdcMem, 0, 0, SRCCOPY);
    ReleaseDC(hdc);
    DeleteDC(hdcMem);
    DeleteObject(hbm);
}

LRESULT CRenderView::OnGetDlgCode(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    return DLGC_WANTALLKEYS;
}

LRESULT CRenderView::OnKeyUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    m_KeysDown.erase(wParam);
    return FALSE;
}

LRESULT CRenderView::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    m_KeysDown.insert(wParam);
    return FALSE;
}

LRESULT CRenderView::OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if (GetFocus() != m_hWnd)
    {
        SetFocus();
    }

    int x, y;
    x = GET_X_LPARAM(lParam);
    y = GET_Y_LPARAM(lParam);

    NMRVCLICK rvnm;
    rvnm.nmh.code = RVN_LCLICK;
    rvnm.nmh.idFrom = ::GetDlgCtrlID(m_hWnd);
    rvnm.nmh.hwndFrom = m_hWnd;

    rvnm.x = x;
    rvnm.y = y;

    m_CursorX = x;
    m_CursorY = y;

    ::SendMessage(GetParent(), WM_NOTIFY, rvnm.nmh.idFrom, (LPARAM)&rvnm);
    return FALSE;
}

LRESULT CRenderView::OnRButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if (GetFocus() != m_hWnd)
    {
        SetFocus();
    }

    NMRVCLICK rvnm;
    rvnm.nmh.code = RVN_RCLICK;
    rvnm.nmh.idFrom = ::GetDlgCtrlID(m_hWnd);
    rvnm.nmh.hwndFrom = m_hWnd;

    rvnm.x = GET_X_LPARAM(lParam);
    rvnm.y = GET_Y_LPARAM(lParam);

    m_bRButtonDown = true;

    ::SendMessage(GetParent(), WM_NOTIFY, rvnm.nmh.idFrom, (LPARAM)&rvnm);
    return FALSE;
}

LRESULT CRenderView::OnRButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    m_bRButtonDown = false;
    return FALSE;
}

LRESULT CRenderView::OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    int x, y;
    x = GET_X_LPARAM(lParam);
    y = GET_Y_LPARAM(lParam);

    NMRVMOUSEMOVE rvnm;
    rvnm.nmh.code = RVN_MOUSEMOVE;
    rvnm.nmh.idFrom = ::GetDlgCtrlID(m_hWnd);
    rvnm.nmh.hwndFrom = m_hWnd;

    rvnm.x = x;
    rvnm.y = y;
    rvnm.deltaX = x - m_CursorX;
    rvnm.deltaY = y - m_CursorY;
    rvnm.buttons = wParam;

    m_CursorX = x;
    m_CursorY = y;

    ::SendMessage(GetParent(), WM_NOTIFY, rvnm.nmh.idFrom, (LPARAM)&rvnm);
    return FALSE;
}

///////////////

//HHOOK CDebugDisplayList::hWinMessageHook = NULL;
//CDebugDisplayList* CDebugDisplayList::_this = NULL;
//static LRESULT CALLBACK HookProc(int nCode, WPARAM wParam, LPARAM lParam);

CDebugDisplayList::CDebugDisplayList(CDebuggerUI* debugger) :
    CDebugDialog<CDebugDisplayList>(debugger),
    m_bRefreshPending(false),
    m_DrawBuffers(NULL),
    m_RenderView(NULL)
{
	m_Camera.m_Pos.z = -5.0f;

    m_RenderView = new CRenderView;
    m_RenderView->RegisterClass();
}

CDebugDisplayList::~CDebugDisplayList()
{
}

void CDebugDisplayList::ResetResourceTreeCtrl(void)
{
    m_TreeImageList.Destroy();
    m_TreeImageList.Create(16, 16, ILC_COLOR8, 0, 4);

    m_ResourceTreeCtrl.DeleteAllItems();
    //m_ResourceTreeCtrl.SetImageList(m_TreeImageList);

    m_hTreeDisplayLists = m_ResourceTreeCtrl.InsertItem("Display lists", -1, -1, NULL, NULL);
    m_hTreeSegments = m_ResourceTreeCtrl.InsertItem("Segments", -1, -1, NULL, NULL);
    m_hTreeImages = m_ResourceTreeCtrl.InsertItem("Images", -1, -1, NULL, NULL);
    m_hTreeVertices = m_ResourceTreeCtrl.InsertItem("Vertices", -1, -1, NULL, NULL);
    m_hTreeMatrices = m_ResourceTreeCtrl.InsertItem("Matrices", -1, -1, NULL, NULL);
    m_hTreeViewports = m_ResourceTreeCtrl.InsertItem("Viewports", -1, -1, NULL, NULL);
    m_hTreeLights = m_ResourceTreeCtrl.InsertItem("Lights", -1, -1, NULL, NULL);
}

LRESULT CDebugDisplayList::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    DlgResize_Init(false, true);
    DlgToolTip_Init();
    DlgSavePos_Init(DebuggerUI_DisplayListPos);

    if (m_RenderView == NULL)
    {
        m_RenderView = new CRenderView;
        m_RenderView->RegisterClass();
    }

    m_RenderView->SubclassWindow(GetDlgItem(IDC_CUSTOM1));

    m_DisplayListCtrl.Attach(GetDlgItem(IDC_LST_DLIST));
    m_ResourceTreeCtrl.Attach(GetDlgItem(IDC_TREE_RESOURCES));
    m_StateTextbox.Attach(GetDlgItem(IDC_EDIT_STATE));
    m_StatusText.Attach(GetDlgItem(IDC_STATUS_TEXT));
    //m_TileListCtrl.Attach(GetDlgItem(IDC_LST_TILES));

    m_DisplayListCtrl.ModifyStyle(LVS_OWNERDRAWFIXED, 0, 0);
    m_DisplayListCtrl.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);

    m_DisplayListCtrl.AddColumn("Address", DisplayListCtrl_Col_VAddr);
    m_DisplayListCtrl.AddColumn("SegOffset", DisplayListCtrl_Col_SegOffset);
    m_DisplayListCtrl.AddColumn("Raw Command", DisplayListCtrl_Col_RawCommand);
    m_DisplayListCtrl.AddColumn("Command", DisplayListCtrl_Col_Command);
    m_DisplayListCtrl.AddColumn("Parameters", DisplayListCtrl_Col_Parameters);

    m_DisplayListCtrl.SetColumnWidth(DisplayListCtrl_Col_SegOffset, 65);
    m_DisplayListCtrl.SetColumnWidth(DisplayListCtrl_Col_VAddr, 65);
    m_DisplayListCtrl.SetColumnWidth(DisplayListCtrl_Col_RawCommand, 115);
    m_DisplayListCtrl.SetColumnWidth(DisplayListCtrl_Col_Command, 140);
    m_DisplayListCtrl.SetColumnWidth(DisplayListCtrl_Col_Parameters, 340);

    m_StateTextbox.SetFont((HFONT)GetStockObject(ANSI_FIXED_FONT), TRUE);

    ResetResourceTreeCtrl();

    ::GetWindowRect(GetDlgItem(IDC_CUSTOM1), &m_OrgTexPreviewRect);
    ::GetWindowRect(GetDlgItem(IDC_EDIT_RESINFO), &m_OrgResInfoRect);

    ScreenToClient(m_OrgTexPreviewRect);
    ScreenToClient(m_OrgResInfoRect);

    m_DrawBuffers = new CDrawBuffers(m_OrgTexPreviewRect.Width()-2, m_OrgTexPreviewRect.Height()-2);
    printf("drawbuffers %d %d\n", m_OrgTexPreviewRect.Width(), m_OrgTexPreviewRect.Height());

    SetTimer(TIMER_ID_DRAW, 20, NULL);

    //_this = this;
    //m_ThreadId = ::GetCurrentThreadId();
    //hWinMessageHook = SetWindowsHookEx(WH_GETMESSAGE, (HOOKPROC)HookProc, NULL, m_ThreadId);

    LoadWindowPos();
    WindowCreated();
    return TRUE;
}

//LRESULT CALLBACK CDebugDisplayList::HookProc(int nCode, WPARAM wParam, LPARAM lParam)
//{
//    MSG *pMsg = (MSG*)lParam;
//
//    POINT pointerPos;
//    CRect ctrlRect;
//
//    switch (pMsg->message)
//    {
//    case WM_MOUSEWHEEL:
//        //_this->OnInterceptMouseWheel(pMsg->wParam, pMsg->lParam);
//        break;
//    //case WM_LBUTTONDOWN:
//    //    ::GetCursorPos(&pointerPos);
//    //    ::GetWindowRect(_this->GetDlgItem(IDC_CUSTOM1), &ctrlRect);
//    //    if (ctrlRect.PtInRect(pointerPos))
//    //    {
//    //        int x = pointerPos.x - ctrlRect.left;
//    //        int y = pointerPos.y - ctrlRect.top;
//    //        //_this->m_GfxParser.m
//    //        int triIndex = _this->m_DrawBuffers->GetSelect(x, y);
//    //        _this->m_GfxParser.testGeom.m_TriangleRefs[triIndex].bSelected = true;
//    //        //_this->m_GfxParser.testGeom.GetTriangle()
//    //        //_this->m_GfxParser.testGeom.m_Vertices[];
//    //        //printf("%d %d -> %d\n", x, y, );
//    //    }
//    //    break;
//    case WM_MOUSEMOVE:
//        {
//        }
//        break;
//    }
//
//    if (nCode < 0)
//    {
//        return CallNextHookEx(hWinMessageHook, nCode, wParam, lParam);
//    }
//
//    return 0;
//}

LRESULT CDebugDisplayList::OnDestroy(void)
{
    KillTimer(TIMER_ID_DRAW);
    m_DisplayListCtrl.Detach();
    m_ResourceTreeCtrl.Detach();
    m_StateTextbox.Detach();
    m_StatusText.Detach();
    //m_TileListCtrl.Detach();
    return 0;
}

LRESULT CDebugDisplayList::OnClicked(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    switch (wID)
    {
    case IDOK:
        EndDialog(0);
        break;
    case IDCANCEL:
        EndDialog(0);
        break;
    case IDC_BTN_REFRESH:
        m_bRefreshPending = true;
        m_StatusText.SetWindowTextA("Waiting for RSP task...");
        ::EnableWindow(GetDlgItem(IDC_BTN_REFRESH), false);
        break;
    }

    return FALSE;
}

// called from RunRSP when task type == 1
void CDebugDisplayList::Refresh(void)
{
    if (m_hWnd == NULL || g_MMU == NULL || m_bRefreshPending == false)
    {
        return;
    }

    uint32_t ucodeAddr, dlistAddr, dlistSize;

    g_MMU->LW_VAddr(0xA4000FD0, ucodeAddr);
    g_MMU->LW_VAddr(0xA4000FF0, dlistAddr);
    g_MMU->LW_VAddr(0xA4000FF4, dlistSize);

    m_GfxParser.Run(ucodeAddr, dlistAddr, dlistSize);

    size_t numCommands = m_GfxParser.GetCommandCount();
    size_t numResources = m_GfxParser.GetRamResourceCount();
    size_t numTriangles = m_GfxParser.GetTriangleCount();

    ucode_version_t ucodeVersion = m_GfxParser.GetUCodeVersion();
    uint32_t ucodeChecksum = m_GfxParser.GetUCodeChecksum();

    stdstr strStatus = (ucodeVersion != UCODE_UNKNOWN) ? m_GfxParser.GetUCodeName() : "Unknown microcode";
    strStatus += stdstr_f(" (Checksum: 0x%08X) - %d commands, %d triangles", ucodeChecksum, numCommands, numTriangles);
    m_StatusText.SetWindowText(strStatus.c_str());

    m_DisplayListCtrl.SetRedraw(FALSE);
    m_DisplayListCtrl.DeleteAllItems();
    
    for (size_t nCmd = 0; nCmd < numCommands; nCmd++)
    {
        decoded_cmd_t* dc = m_GfxParser.GetLoggedCommand(nCmd);

        stdstr strVirtAddress = stdstr_f("%08X", dc->virtualAddress);
        stdstr strSegOffset = stdstr_f("%08X", dc->address);
        stdstr strRawCommand = stdstr_f("%08X %08X", dc->rawCommand.w0, dc->rawCommand.w1);

        m_DisplayListCtrl.AddItem(nCmd, DisplayListCtrl_Col_VAddr, strVirtAddress.c_str());
        m_DisplayListCtrl.AddItem(nCmd, DisplayListCtrl_Col_SegOffset, strSegOffset.c_str());
        m_DisplayListCtrl.AddItem(nCmd, DisplayListCtrl_Col_RawCommand, strRawCommand.c_str());
        m_DisplayListCtrl.AddItem(nCmd, DisplayListCtrl_Col_Command, dc->name);
        m_DisplayListCtrl.AddItem(nCmd, DisplayListCtrl_Col_Parameters, dc->params.c_str());
    }

    m_DisplayListCtrl.SetRedraw(TRUE);
    m_DisplayListCtrl.SelectItem(0);

    m_ResourceTreeCtrl.SetRedraw(FALSE);
    ResetResourceTreeCtrl();

    for (size_t nRes = 0; nRes < numResources; nRes++)
    {
        dram_resource_t* res = m_GfxParser.GetRamResource(nRes);
        stdstr str = stdstr_f("0x%08X", res->address);
        HTREEITEM hParentItem = NULL;

        switch (res->type)
        {
        case RES_ROOT_DL: hParentItem = m_hTreeDisplayLists; break;
        case RES_DL: hParentItem = m_hTreeDisplayLists; break;
        case RES_SEGMENT:
            hParentItem = m_hTreeSegments;
            str = stdstr_f("0x%02X: 0x%08X", res->param, res->address);
            break;
        case RES_COLORBUFFER:
            str += " Colorbuffer";
            hParentItem = m_hTreeImages; break;
        case RES_DEPTHBUFFER:
            str += " Depthbuffer";
            hParentItem = m_hTreeImages; break;
        case RES_TEXTURE:
            str += stdstr_f(" Texture %dx%d %s %s",
                res->imageWidth, res->imageHeight,
                CGfxLabels::TexelSizesShort[res->imageSiz],
                CGfxLabels::ImageFormatsShort[res->imageFmt]);
            hParentItem = m_hTreeImages; break;
        case RES_PALETTE:
            str += " Palette";
            hParentItem = m_hTreeImages;  break;
        case RES_VERTICES: hParentItem = m_hTreeVertices;  break;
        case RES_PROJECTION_MATRIX: hParentItem = m_hTreeMatrices;  break;
        case RES_MODELVIEW_MATRIX: hParentItem = m_hTreeMatrices;  break;
        case RES_VIEWPORT: hParentItem = m_hTreeViewports;  break;
        case RES_DIFFUSE_LIGHT: hParentItem = m_hTreeLights;  break;
        case RES_AMBIENT_LIGHT: hParentItem = m_hTreeLights;  break;
        }

        if (hParentItem != NULL)
        {
            HTREEITEM hItem = m_ResourceTreeCtrl.InsertItem(str.c_str(), hParentItem, NULL);
            m_ResourceTreeCtrl.SetItemData(hItem, nRes);
        }
    }

    m_ResourceTreeCtrl.SetRedraw(TRUE);

    ::EnableWindow(GetDlgItem(IDC_BTN_REFRESH), TRUE);
    m_bRefreshPending = false;
}

void CDebugDisplayList::SetPreviewColor(WORD ctrlId, uint32_t colorPair)
{
    uint16_t color16 = colorPair & 0xFFFF;
    uint8_t r = (uint8_t)((color16 >> 11) & 0x1F) * (255.0f / 31.0f);
    uint8_t g = (uint8_t)((color16 >> 6) & 0x1F) * (255.0f / 31.0f);
    uint8_t b = (uint8_t)((color16 >> 1) & 0x1F) * (255.0f / 31.0f);
    //uint8_t a = (color16 & 1) * 255;

    HWND hWndPrevFillColor = GetDlgItem(ctrlId);
    HDC dc = ::GetDC(hWndPrevFillColor);
    HBRUSH hbr = CreateSolidBrush(RGB(r, g, b));
    CRect rcWnd;
    ::GetWindowRect(hWndPrevFillColor, &rcWnd);
    RECT rc = { 1, 1, rcWnd.Width() - 1, rcWnd.Height() - 1 };
    FillRect(dc, &rc, hbr);
    DeleteObject(hbr);
    ::ReleaseDC(hWndPrevFillColor, dc);
}

LRESULT CDebugDisplayList::OnListItemChanged(NMHDR* pNMHDR)
{
    NMITEMACTIVATE* pIA = reinterpret_cast<NMITEMACTIVATE*>(pNMHDR);
    int nItem = pIA->iItem;

    CHleGfxState* state = m_GfxParser.GetLoggedState(nItem);

    if (state == NULL)
    {
        return FALSE;
    }

    // Geometry mode

    ::SendMessage(GetDlgItem(IDC_CHK_GM_ZBUFFER), BM_SETCHECK, state->m_GeometryMode.f3d.zbuffer ? BST_CHECKED : BST_UNCHECKED, 0);
    ::SendMessage(GetDlgItem(IDC_CHK_GM_SHADE), BM_SETCHECK, state->m_GeometryMode.f3d.shade ? BST_CHECKED : BST_UNCHECKED, 0);
    ::SendMessage(GetDlgItem(IDC_CHK_GM_FOG), BM_SETCHECK, state->m_GeometryMode.f3d.fog ? BST_CHECKED : BST_UNCHECKED, 0);
    ::SendMessage(GetDlgItem(IDC_CHK_GM_LIGHTING), BM_SETCHECK, state->m_GeometryMode.f3d.lighting ? BST_CHECKED : BST_UNCHECKED, 0);
    ::SendMessage(GetDlgItem(IDC_CHK_GM_TEXTURE_GEN), BM_SETCHECK, state->m_GeometryMode.f3d.texture_gen ? BST_CHECKED : BST_UNCHECKED, 0);
    ::SendMessage(GetDlgItem(IDC_CHK_GM_TEXTURE_GEN_LINEAR), BM_SETCHECK, state->m_GeometryMode.f3d.texture_gen_linear ? BST_CHECKED : BST_UNCHECKED, 0);
    ::SendMessage(GetDlgItem(IDC_CHK_GM_LOD), BM_SETCHECK, state->m_GeometryMode.f3d.lod ? BST_CHECKED : BST_UNCHECKED, 0);
    ::SendMessage(GetDlgItem(IDC_CHK_GM_CLIPPING), BM_SETCHECK, state->m_GeometryMode.f3d.clipping ? BST_CHECKED : BST_UNCHECKED, 0);

    bool shading_smooth, cull_front, cull_back;

    if (m_GfxParser.GetUCodeVersion() == UCODE_F3DEX2)
    {
        shading_smooth = state->m_GeometryMode.f3dex2.shading_smooth;
        cull_front = state->m_GeometryMode.f3dex2.cull_front;
        cull_back = state->m_GeometryMode.f3dex2.cull_back;
    }
    else
    {
        shading_smooth = state->m_GeometryMode.f3d.shading_smooth;
        cull_front = state->m_GeometryMode.f3d.cull_front;
        cull_back = state->m_GeometryMode.f3d.cull_back;
    }

    ::SendMessage(GetDlgItem(IDC_CHK_GM_SHADING_SMOOTH), BM_SETCHECK, shading_smooth ? BST_CHECKED : BST_UNCHECKED, 0);
    ::SendMessage(GetDlgItem(IDC_CHK_GM_CULL_FRONT), BM_SETCHECK, cull_front ? BST_CHECKED : BST_UNCHECKED, 0);
    ::SendMessage(GetDlgItem(IDC_CHK_GM_CULL_BACK), BM_SETCHECK, cull_back ? BST_CHECKED : BST_UNCHECKED, 0);


    // Texture address

    ::SetWindowText(GetDlgItem(IDC_EDIT_TEXTUREIMAGE), stdstr_f("0x%08X (0x%08X)", state->m_TextureImage, state->SegmentedToVirtual(state->m_TextureImage)).c_str());
    ::SetWindowText(GetDlgItem(IDC_EDIT_DEPTHIMAGE), stdstr_f("0x%08X (0x%08X)", state->m_DepthImage, state->SegmentedToVirtual(state->m_DepthImage)).c_str());
    ::SetWindowText(GetDlgItem(IDC_EDIT_COLORIMAGE), stdstr_f("0x%08X (0x%08X)", state->m_ColorImage, state->SegmentedToVirtual(state->m_ColorImage)).c_str());

    // rdp colors

    ::SetWindowText(GetDlgItem(IDC_EDIT_FILLCOLOR), stdstr_f("0x%08X", state->m_FillColor).c_str());
    ::SetWindowText(GetDlgItem(IDC_EDIT_FOGCOLOR), stdstr_f("0x%08X", state->m_FogColor).c_str());
    ::SetWindowText(GetDlgItem(IDC_EDIT_BLENDCOLOR), stdstr_f("0x%08X", state->m_BlendColor).c_str());
    ::SetWindowText(GetDlgItem(IDC_EDIT_PRIMCOLOR), stdstr_f("0x%08X", state->m_PrimColor).c_str());
    ::SetWindowText(GetDlgItem(IDC_EDIT_ENVCOLOR), stdstr_f("0x%08X", state->m_EnvColor).c_str());

    SetPreviewColor(IDC_PREVIEW_FILLCOLOR, state->m_FillColor);
    SetPreviewColor(IDC_PREVIEW_FOGCOLOR, state->m_FogColor);
    SetPreviewColor(IDC_PREVIEW_BLENDCOLOR, state->m_BlendColor);
    SetPreviewColor(IDC_PREVIEW_PRIMCOLOR, state->m_PrimColor);
    SetPreviewColor(IDC_PREVIEW_ENVCOLOR, state->m_EnvColor);

    // tiles

    stdstr strTileDescriptors = "# tmem  siz fmt  line shiftS maskS cmS shiftT maskT cmT scaleS scaleT palette levels on\r\n";

    for (int i = 0; i <= 7; i++)
    {
        tile_t* t = &state->m_Tiles[i];
    
        if (t->enabled == 0 && i != 7)
        {
            // skip the rendertiles if they are disabled
            continue;
        }

        const char* sizName = CGfxLabels::TexelSizesShort[t->siz];
        const char* fmtName = CGfxLabels::ImageFormatsShort[t->fmt];

        strTileDescriptors += stdstr_f(
            "%d 0x%03X %3s %4s %4d %6d %5d %3d %6d %5d %3d 0x%04X 0x%04X %7d %6d  %d\r\n",
            i,
            t->tmem, sizName, fmtName, t->line,
            t->shifts, t->masks, t->cms,
            t->shiftt, t->maskt, t->cmt,
            t->scaleS, t->scaleT,
            t->palette,
            t->mipmapLevels, t->enabled
        );
    }

    // lights

    //stdstr strNumLights = stdstr_f("NumLights: %d\r\n", state->numLights);

    // othermode_h

    othermode_h_t* omh = &state->m_OtherMode_h;

    ::SetWindowText(GetDlgItem(IDC_EDIT_OMH_PIPELINE), CGfxLabels::OtherMode_pm[omh->pm]);
    ::SetWindowText(GetDlgItem(IDC_EDIT_OMH_COLORDITHER), CGfxLabels::OtherMode_cd[omh->cd]);
    ::SetWindowText(GetDlgItem(IDC_EDIT_OMH_CYCLETYPE), CGfxLabels::OtherMode_cyc[omh->cyc]);
    ::SetWindowText(GetDlgItem(IDC_EDIT_OMH_TEXTPERSP), CGfxLabels::OtherMode_tp[omh->tp]);
    ::SetWindowText(GetDlgItem(IDC_EDIT_OMH_TEXTDETAIL), CGfxLabels::OtherMode_td[omh->td]);
    ::SetWindowText(GetDlgItem(IDC_EDIT_OMH_TEXTLOD), CGfxLabels::OtherMode_tl[omh->tl]);
    ::SetWindowText(GetDlgItem(IDC_EDIT_OMH_TEXTLUT), CGfxLabels::OtherMode_tt[omh->tt]);
    ::SetWindowText(GetDlgItem(IDC_EDIT_OMH_TEXTFILT), CGfxLabels::OtherMode_tf[omh->tf]);
    ::SetWindowText(GetDlgItem(IDC_EDIT_OMH_TEXTCONV), CGfxLabels::OtherMode_tc[omh->tc]);
    ::SetWindowText(GetDlgItem(IDC_EDIT_OMH_COMBKEY), CGfxLabels::OtherMode_ck[omh->ck]);
    ::SetWindowText(GetDlgItem(IDC_EDIT_OMH_RGBDITHER), CGfxLabels::OtherMode_rd[omh->rd]);
    ::SetWindowText(GetDlgItem(IDC_EDIT_OMH_ALPHADITHER), CGfxLabels::OtherMode_ad[omh->ad]);

    /////////////

    stdstr strCycle1Color = stdstr_f("(%s - %s) * %s + %s\r\n",
        CGfxLabels::LookupName(CGfxLabels::CCMuxA, state->m_Combiner.a0),
        CGfxLabels::LookupName(CGfxLabels::CCMuxB, state->m_Combiner.b0),
        CGfxLabels::LookupName(CGfxLabels::CCMuxC, state->m_Combiner.c0),
        CGfxLabels::LookupName(CGfxLabels::CCMuxD, state->m_Combiner.d0));

    stdstr strCycle1Alpha = stdstr_f("(%s - %s) * %s + %s\r\n",
        CGfxLabels::LookupName(CGfxLabels::ACMuxA_B_D, state->m_Combiner.Aa0),
        CGfxLabels::LookupName(CGfxLabels::ACMuxA_B_D, state->m_Combiner.Ab0),
        CGfxLabels::LookupName(CGfxLabels::ACMuxC, state->m_Combiner.Ac0),
        CGfxLabels::LookupName(CGfxLabels::ACMuxA_B_D, state->m_Combiner.Ad0));

    stdstr strCycle2Color = stdstr_f("(%s - %s) * %s + %s\r\n",
        CGfxLabels::LookupName(CGfxLabels::CCMuxA, state->m_Combiner.a1),
        CGfxLabels::LookupName(CGfxLabels::CCMuxB, state->m_Combiner.b1),
        CGfxLabels::LookupName(CGfxLabels::CCMuxC, state->m_Combiner.c1),
        CGfxLabels::LookupName(CGfxLabels::CCMuxD, state->m_Combiner.d1));

    stdstr strCycle2Alpha = stdstr_f("(%s - %s) * %s + %s\r\n",
        CGfxLabels::LookupName(CGfxLabels::ACMuxA_B_D, state->m_Combiner.Aa1),
        CGfxLabels::LookupName(CGfxLabels::ACMuxA_B_D, state->m_Combiner.Ab1),
        CGfxLabels::LookupName(CGfxLabels::ACMuxC, state->m_Combiner.Ac1),
        CGfxLabels::LookupName(CGfxLabels::ACMuxA_B_D, state->m_Combiner.Ad1));

    ::SetWindowText(GetDlgItem(IDC_EDIT_CC_1C), strCycle1Color.c_str());
    ::SetWindowText(GetDlgItem(IDC_EDIT_CC_1A), strCycle1Alpha.c_str());
    ::SetWindowText(GetDlgItem(IDC_EDIT_CC_2C), strCycle2Color.c_str());
    ::SetWindowText(GetDlgItem(IDC_EDIT_CC_2A), strCycle2Alpha.c_str());

    stdstr split = "";

    stdstr strStateSummary = (strTileDescriptors);
    
    m_StateTextbox.SetWindowTextA(strStateSummary.c_str());

    return FALSE;
}

LRESULT CDebugDisplayList::OnListDblClicked(NMHDR* pNMHDR)
{
    int nItem = m_DisplayListCtrl.GetSelectedIndex();
    CHleGfxState* state = m_GfxParser.GetLoggedState(nItem);

    if (state == NULL)
    {
        return FALSE;
    }

    uint32_t cmdAddress = state->SegmentedToVirtual(state->m_Address);
    m_Debugger->Debug_ShowMemoryLocation(cmdAddress, true);
    return FALSE;
}

void CDebugDisplayList::OnTimer(UINT_PTR nIDEvent)
{
    if (nIDEvent == TIMER_ID_DRAW)
    {
        if (m_RenderView->KeyDown('W')) m_Camera.TranslateZ(0.25f);
        if (m_RenderView->KeyDown('S')) m_Camera.TranslateZ(-0.25f);
        if (m_RenderView->KeyDown('D')) m_Camera.TranslateX(0.25f);
        if (m_RenderView->KeyDown('A')) m_Camera.TranslateX(-0.25f);

        if (m_RenderView->KeyDown(VK_LEFT)) m_Camera.RotateY(4.0f);
        if (m_RenderView->KeyDown(VK_RIGHT)) m_Camera.RotateY(-4.0f);
        if (m_RenderView->KeyDown(VK_UP)) m_Camera.RotateX(4.0f);
        if (m_RenderView->KeyDown(VK_DOWN)) m_Camera.RotateX(-4.0f);

        if (m_RenderView->KeyDown(VK_SPACE))
        {
            m_Camera.SetPos(0, 0, -5.0f);
            m_Camera.SetRot(0, 0, 0);
        }
        
        m_DrawBuffers->Render(&m_GfxParser.testGeom, &m_Camera);
        m_RenderView->DrawImage(m_DrawBuffers);
    }
}

LRESULT CDebugDisplayList::OnRenderViewClicked(NMHDR* pNMHDR)
{
    NMRVCLICK *pnmrv = (NMRVCLICK*)pNMHDR;

    int clickIndex = m_DrawBuffers->GetSelect(pnmrv->x, pnmrv->y);

    if (clickIndex == -1)
    {
        return FALSE;
    }

    geom_tri_ref_t *triangle = &m_GfxParser.testGeom.m_TriangleRefs[clickIndex];
    m_GfxParser.testGeom.m_SelectedTriIdx = clickIndex;

    m_DisplayListCtrl.SelectItem(triangle->nCommand);
    //m_DisplayListCtrl.SetFocus();
    return FALSE;
}

LRESULT CDebugDisplayList::OnRenderViewMouseMove(NMHDR* pNMHDR)
{
    NMRVMOUSEMOVE *pnmrv = (NMRVMOUSEMOVE*)pNMHDR;

    if (pnmrv->buttons & MK_MBUTTON)
    {
        m_Camera.TranslateX(-0.05f * pnmrv->deltaX);
        m_Camera.TranslateY(-0.05f * pnmrv->deltaY);
    }
    else if (pnmrv->buttons & MK_RBUTTON)
    {
        m_Camera.RotateX(-1.0f * pnmrv->deltaY);
        m_Camera.RotateY(-1.0f * pnmrv->deltaX);
    }

    return FALSE;
}

LRESULT CDebugDisplayList::OnResourceTreeSelChanged(NMHDR* pNMHDR)
{
    NMTREEVIEW* pnmtv = reinterpret_cast<NMTREEVIEW*>(pNMHDR);
    TVITEM tvItem = pnmtv->itemNew;

    HTREEITEM hItem = tvItem.hItem;
    HTREEITEM hParentItem = m_ResourceTreeCtrl.GetParentItem(hItem);

    if (hParentItem == NULL) // don't do anything for root items
    {
        return FALSE;
    }

    int nRes = m_ResourceTreeCtrl.GetItemData(hItem);
    dram_resource_t* res = m_GfxParser.GetRamResource(nRes);
    
    // todo add this to context menu
    //m_DisplayListCtrl.EnsureVisible(res->nCommand, FALSE);
    //m_DisplayListCtrl.SelectItem(res->nCommand);
    
    CRect& texPrevRc = m_OrgTexPreviewRect;
    CRect& resInfoRc = m_OrgResInfoRect;

    if (res->type == RES_TEXTURE || res->type == RES_DL)
    {
        ::ShowWindow(GetDlgItem(IDC_CUSTOM1), SW_SHOW);
        ::MoveWindow(GetDlgItem(IDC_EDIT_RESINFO),
            resInfoRc.left, resInfoRc.top,
            resInfoRc.Width(), resInfoRc.Height(), TRUE);
    }
    else
    {
        ::ShowWindow(GetDlgItem(IDC_CUSTOM1), SW_HIDE);
        ::MoveWindow(GetDlgItem(IDC_EDIT_RESINFO),
            texPrevRc.left, texPrevRc.top,
            texPrevRc.Width(), texPrevRc.Height(), TRUE);
    }

    stdstr strDefaultInfo = stdstr_f("Segment offset: 0x%08X\r\nVirtual address: 0x%08X", res->address, res->virtAddress);

    switch (res->type)
    {
    case RES_TEXTURE:
        PreviewImageResource(res);
        break;
    case RES_DL:
        break;
    default:
        ::SetWindowText(GetDlgItem(IDC_EDIT_RESINFO), strDefaultInfo.c_str());
        break;
    }
    
    // dumb test
    //CImageList imgList = m_TreeImageList;
    //int idx = imgList.Add(hBitmap);
    //pnmtv->itemNew.iImage = idx;
    //pnmtv->itemNew.iSelectedImage = idx;
    //pnmtv->itemNew.mask &= ~(TVIF_IMAGE | TVIF_SELECTEDIMAGE);
    //m_ResourceTreeCtrl.SetItemImage(hItem, idx, idx);
    //m_ResourceTreeCtrl.RedrawWindow();
    
    return FALSE;
}


LRESULT CDebugDisplayList::OnCustomDrawList(NMHDR* pNMHDR)
{
    NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>(pNMHDR);
    DWORD drawStage = pLVCD->nmcd.dwDrawStage;

    switch (drawStage)
    {
    case CDDS_PREPAINT: return (CDRF_NOTIFYITEMDRAW | CDRF_NOTIFYPOSTPAINT);
    case CDDS_ITEMPREPAINT: return CDRF_NOTIFYSUBITEMDRAW;
    case (CDDS_ITEMPREPAINT | CDDS_SUBITEM): break;
    default:
        return CDRF_DODEFAULT;
    }

    uint32_t nItem = (uint32_t)pLVCD->nmcd.dwItemSpec;
    uint32_t nSubItem = pLVCD->iSubItem;

    decoded_cmd_t* dc = m_GfxParser.GetLoggedCommand(nItem);
    CHleGfxState* state = m_GfxParser.GetLoggedState(nItem);

    int depth = state->m_StackIndex * 10;

    switch (nSubItem)
    {
    case DisplayListCtrl_Col_VAddr:
    case DisplayListCtrl_Col_SegOffset:
        pLVCD->clrTextBk = RGB(0xEE, 0xEE, 0xEE);
        pLVCD->clrText = RGB(0x44, 0x44, 0x44);
        break;
    case DisplayListCtrl_Col_Command:
        pLVCD->clrTextBk = (dc->listBgColor != 0) ? dc->listBgColor : RGB(255, 255, 255);
        pLVCD->clrText = (dc->listFgColor != 0) ? dc->listFgColor : RGB(0, 0, 0);
        break;
    default:
        pLVCD->clrTextBk = RGB(255, 255, 255);
        pLVCD->clrText = RGB(0, 0, 0);
        break;
    }

    return CDRF_DODEFAULT;
}

LRESULT CDebugDisplayList::OnMeasureItem(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
    if (wParam == IDC_LST_DLIST || wParam == IDC_LST_RESOURCES)
    {
        CClientDC dc(m_hWnd);
        dc.SelectFont(GetFont());
        TEXTMETRIC tm;
        dc.GetTextMetrics(&tm);

        int rowHeight = tm.tmHeight + tm.tmExternalLeading;

        MEASUREITEMSTRUCT* lpMeasureItem = (MEASUREITEMSTRUCT*)lParam;
        lpMeasureItem->itemHeight = rowHeight;
    }
    return FALSE;
}

void CDebugDisplayList::PreviewImageResource(dram_resource_t* res)
{
    // todo do this inside a custom control
    // handle proper repainting etc
    // should decode images in gfxparser

    HWND texWnd = GetDlgItem(IDC_CUSTOM1);
    HDC hDC = ::GetDC(texWnd);

    int width = (res->imageWidth != 0) ? res->imageWidth : 32;
    int height = (res->imageHeight != 0) ? res->imageHeight : 32;
    int numTexels = width * height;

    uint32_t* imageBuffer = new uint32_t[numTexels];
    uint8_t* imgSrc = m_GfxParser.GetRamSnapshot() + (res->virtAddress - 0x80000000);

    bool valid = CGfxParser::ConvertImage(imageBuffer, imgSrc, res->imageFmt, res->imageSiz, numTexels);

    CRect rc;
    ::GetWindowRect(texWnd, &rc);

    float scale = 4.0f;

    while ((width * scale) >= rc.Width() || (height * scale) >= rc.Height())
    {
        scale /= 2;
    }

    HBITMAP hBitmap = CreateBitmap(width, height, 1, 32, imageBuffer);
    HDC hTempDC = CreateCompatibleDC(hDC);

    HBRUSH hBrush = CreateSolidBrush(RGB(240, 240, 240));

    RECT rcFill = { 1, 1, rc.Width() - 1, rc.Height() - 1 };

    SelectObject(hTempDC, hBitmap);

    int xoffs = rc.Width() / 2 - (width*scale / 2);
    int yoffs = rc.Height() / 2 - (height*scale / 2);

    FillRect(hDC, &rcFill, hBrush);
    StretchBlt(hDC, xoffs, yoffs, width*scale, height*scale,
        hTempDC, 0, 0, width, height, SRCCOPY);

    ::ReleaseDC(texWnd, hDC);
    ::DeleteDC(hTempDC);
    ::DeleteObject(hBitmap);
    ::DeleteObject(hBrush);
    delete[] imageBuffer;

    stdstr strImageInfo = stdstr_f(
        "%dx%d %s %s (Scaled %d%%)\r\n"
        "Segment offset: 0x%08X\r\n"
        "Virtual address: 0x%08X",
        res->imageWidth, res->imageHeight,
        CGfxLabels::TexelSizesShort[res->imageSiz],
        CGfxLabels::ImageFormatsShort[res->imageFmt],
        (int)(scale*100),
        res->address, res->virtAddress);

    ::SetWindowText(GetDlgItem(IDC_EDIT_RESINFO), strImageInfo.c_str());
}