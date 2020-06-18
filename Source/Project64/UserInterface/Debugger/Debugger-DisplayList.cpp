#include "stdafx.h"
#include "DebuggerUI.h"

#include "GfxParser.h"
#include "GfxRenderer.h"
#include "GfxLabels.h"

#define C5bTo8b(n) ((unsigned char)((n & 0x1F) * (255.0f / 31.0f)))
#define RGBFROM5551(n) RGB(\
    C5bTo8b(((n) >> 11) & 0x1F),\
    C5bTo8b(((n) >> 6) & 0x1F),\
    C5bTo8b(((n) >> 1) & 0x1F))

#define RGBFROM8888(n) RGB((n) >> 24, ((n) >> 16) & 0xFF, ((n) >> 8) & 0xFF)

CDebugDisplayList::CDebugDisplayList(CDebuggerUI* debugger) :
    CDebugDialog<CDebugDisplayList>(debugger),
    m_bRefreshPending(false),
    m_DrawBuffers(NULL),
    m_RenderView(NULL),
    m_bShowRender(true)
{
	m_Camera.m_Pos.z = -5.0f;

    m_StateCanvas = new CCanvas;
    m_StateCanvas->RegisterClass();

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

    m_hTreeDisplayLists = m_ResourceTreeCtrl.InsertItem(L"Display lists", -1, -1, NULL, NULL);
    m_hTreeSegments = m_ResourceTreeCtrl.InsertItem(L"Segments", -1, -1, NULL, NULL);
    m_hTreeImages = m_ResourceTreeCtrl.InsertItem(L"Images", -1, -1, NULL, NULL);
    m_hTreeVertices = m_ResourceTreeCtrl.InsertItem(L"Vertices", -1, -1, NULL, NULL);
    m_hTreeMatrices = m_ResourceTreeCtrl.InsertItem(L"Matrices", -1, -1, NULL, NULL);
    m_hTreeViewports = m_ResourceTreeCtrl.InsertItem(L"Viewports", -1, -1, NULL, NULL);
    m_hTreeLights = m_ResourceTreeCtrl.InsertItem(L"Lights", -1, -1, NULL, NULL);
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

    if (m_StateCanvas == NULL)
    {
        m_StateCanvas = new CCanvas;
        m_StateCanvas->RegisterClass();
    }

    m_StateCanvas->SubclassWindow(GetDlgItem(IDC_CUSTOM2));
    m_StateCanvas->Init();

    m_RenderView->SubclassWindow(GetDlgItem(IDC_CUSTOM1));

    m_DisplayListCtrl.Attach(GetDlgItem(IDC_LST_DLIST));
    m_ResourceTreeCtrl.Attach(GetDlgItem(IDC_TREE_RESOURCES));
    m_StateTextbox.Attach(GetDlgItem(IDC_EDIT_STATE));
    m_StatusText.Attach(GetDlgItem(IDC_STATUS_TEXT));
    //m_TileListCtrl.Attach(GetDlgItem(IDC_LST_TILES));

    m_DisplayListCtrl.ModifyStyle(LVS_OWNERDRAWFIXED, 0, 0);
    m_DisplayListCtrl.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);

    m_DisplayListCtrl.AddColumn(L"Address", DisplayListCtrl_Col_VAddr);
    m_DisplayListCtrl.AddColumn(L"SegOffset", DisplayListCtrl_Col_SegOffset);
   // m_DisplayListCtrl.AddColumn(L"Raw Command", DisplayListCtrl_Col_RawCommand);
    m_DisplayListCtrl.AddColumn(L"Command", DisplayListCtrl_Col_Command);
    m_DisplayListCtrl.AddColumn(L"Parameters", DisplayListCtrl_Col_Parameters);

    m_DisplayListCtrl.SetColumnWidth(DisplayListCtrl_Col_SegOffset, 65);
    m_DisplayListCtrl.SetColumnWidth(DisplayListCtrl_Col_VAddr, 65);
   // m_DisplayListCtrl.SetColumnWidth(DisplayListCtrl_Col_RawCommand, 115);
    m_DisplayListCtrl.SetColumnWidth(DisplayListCtrl_Col_Command, 140);
    m_DisplayListCtrl.SetColumnWidth(DisplayListCtrl_Col_Parameters, 455);

    m_StateTextbox.SetFont((HFONT)GetStockObject(ANSI_FIXED_FONT), TRUE);

    ResetResourceTreeCtrl();

    ::GetWindowRect(GetDlgItem(IDC_CUSTOM1), &m_OrgTexPreviewRect);
    ::GetWindowRect(GetDlgItem(IDC_EDIT_RESINFO), &m_OrgResInfoRect);

    ScreenToClient(m_OrgTexPreviewRect);
    ScreenToClient(m_OrgResInfoRect);

    m_DrawBuffers = new CDrawBuffers(m_OrgTexPreviewRect.Width()-2, m_OrgTexPreviewRect.Height()-2);

    CCanvas*& scv = m_StateCanvas;
	COLORREF headerColor = RGB(0xAA, 0xAA, 0xAA);
	COLORREF labelColor = RGB(0xCC, 0xCC, 0xCC);

    // Geometry mode
    scv->SetOrigin(568, 130);
    scv->AddItem(L"GEOMETRY MODE", headerColor);
    m_ITM_GM_ZBUFFER = scv->AddItem(L" ZBUFFER", labelColor);
    m_ITM_GM_SHADE = scv->AddItem(L" SHADE", labelColor);
    m_ITM_GM_FOG = scv->AddItem(L" FOG", labelColor);
    m_ITM_GM_LIGHTING = scv->AddItem(L" LIGHTING", labelColor);
    m_ITM_GM_TEXTURE_GEN = scv->AddItem(L" TEXTURE_GEN", labelColor);
    m_ITM_GM_TEXTURE_GEN_LINEAR = scv->AddItem(L" TEXTURE_GEN_LINEAR", labelColor);
    m_ITM_GM_LOD = scv->AddItem(L" LOD", labelColor);
    m_ITM_GM_CLIPPING = scv->AddItem(L" CLIPPING", labelColor);
    m_ITM_GM_SHADING_SMOOTH = scv->AddItem(L" SHADING_SMOOTH", labelColor);
    m_ITM_GM_CULL_FRONT = scv->AddItem(L" CULL_FRONT", labelColor);
    m_ITM_GM_CULL_BACK = scv->AddItem(L" CULL_BACK", labelColor);

	// Image addresses
    scv->SetOrigin(300, 70);
    scv->AddItem(L"IMAGE ADDRESSES", headerColor);
    m_StateCanvas->AddItem(L" TEXTURE", labelColor);
    m_StateCanvas->AddItem(L" COLOR", labelColor);
    m_StateCanvas->AddItem(L" DEPTH", labelColor);
    scv->SetOrigin(360, 80);
    m_ITM_TEXTUREIMAGE = scv->AddItem(L"...");
    m_ITM_COLORIMAGE = scv->AddItem(L"...");
    m_ITM_DEPTHIMAGE = scv->AddItem(L"...");

	// Combiner
    scv->SetOrigin(10, 10);
    scv->AddItem(L"COMBINER", headerColor);
    scv->AddItem(L" CYCLE 1 COLOR", labelColor);
    scv->AddItem(L" CYCLE 1 ALPHA", labelColor);
    scv->AddItem(L" CYCLE 2 COLOR", labelColor);
    scv->AddItem(L" CYCLE 2 ALPHA", labelColor);

    scv->SetOrigin(16+90, 20);
    m_ITM_CC1_COLOR = scv->AddItem(L"...");
    m_ITM_CC1_ALPHA = scv->AddItem(L"...");
    m_ITM_CC2_COLOR = scv->AddItem(L"...");
    m_ITM_CC2_ALPHA = scv->AddItem(L"...");

	// Othermode HI
    scv->SetOrigin(10, 70);
	scv->AddItem(L"OTHERMODE_H", headerColor);
	scv->AddItem(L" PIPELINE", labelColor);
	scv->AddItem(L" COLORDITHER", labelColor);
	scv->AddItem(L" CYCLETYPE", labelColor);
	scv->AddItem(L" TEXTPERSP", labelColor);
	scv->AddItem(L" TEXTDETAIL", labelColor);
	scv->AddItem(L" TEXTLOD", labelColor);
	scv->AddItem(L" TEXTLUT", labelColor);
	scv->AddItem(L" TEXTFILT", labelColor);
	scv->AddItem(L" TEXTCONV", labelColor);
	scv->AddItem(L" COMBKEY", labelColor);
	scv->AddItem(L" RGBDITHER", labelColor);
	scv->AddItem(L" ALPHADITHER", labelColor);

    scv->SetOrigin(16 + 78, 80);
	m_ITM_OMH_PIPELINE = scv->AddItem(L"...");
	m_ITM_OMH_COLORDITHER = scv->AddItem(L"...");
	m_ITM_OMH_CYCLETYPE = scv->AddItem(L"...");
	m_ITM_OMH_TEXTPERSP = scv->AddItem(L"...");
	m_ITM_OMH_TEXTDETAIL = scv->AddItem(L"...");
	m_ITM_OMH_TEXTLOD = scv->AddItem(L"...");
	m_ITM_OMH_TEXTLUT = scv->AddItem(L"...");
	m_ITM_OMH_TEXTFILT = scv->AddItem(L"...");
	m_ITM_OMH_TEXTCONV = scv->AddItem(L"...");
	m_ITM_OMH_COMBKEY = scv->AddItem(L"...");
	m_ITM_OMH_RGBDITHER = scv->AddItem(L"...");
	m_ITM_OMH_ALPHADITHER = scv->AddItem(L"...");

    // Othermode LO
    scv->SetOrigin(200, 70);
    scv->AddItem(L"OTHERMODE_L", headerColor);
    m_ITM_OML_AA_EN = scv->AddItem(L" AA_EN");
    m_ITM_OML_Z_CMP = scv->AddItem(L" Z_CMP");
    m_ITM_OML_Z_UPD = scv->AddItem(L" Z_UPD");
    m_ITM_OML_IM_RD = scv->AddItem(L" IM_RD");
    m_ITM_OML_CLR_ON_CVG = scv->AddItem(L" CLR_ON_CVG");
    m_ITM_OML_CVG_DST = scv->AddItem(L" CVG_DST");
    m_ITM_OML_ZMODE = scv->AddItem(L" ZMODE");
    m_ITM_OML_CVG_X_ALPHA = scv->AddItem(L" CVG_X_ALPHA");
    m_ITM_OML_ALPHA_CVG_SEL = scv->AddItem(L" ALPHA_CVG_SEL");
    m_ITM_OML_FORCE_BL = scv->AddItem(L" FORCE_BL");
    scv->AddItem(L" CYCLE 1", labelColor);
    scv->AddItem(L" CYCLE 2", labelColor);

    scv->SetOrigin(260, 180);
    m_ITM_OML_CYCLE1 = scv->AddItem(L"...");
    m_ITM_OML_CYCLE2 = scv->AddItem(L"...");

	// Colors
    scv->SetOrigin(420, 70);
	scv->AddItem(L"COLORS", headerColor);
	scv->AddItem(L" FILL", labelColor);
	scv->AddItem(L" FOG", labelColor);
	scv->AddItem(L" BLEND", labelColor);
	scv->AddItem(L" PRIM", labelColor);
	scv->AddItem(L" ENV", labelColor);

    scv->SetOrigin(468, 80);
    m_ITM_FILLCOLOR = scv->AddItem(L"...");
    m_ITM_FOGCOLOR = scv->AddItem(L"...");
    m_ITM_BLENDCOLOR = scv->AddItem(L"...");
    m_ITM_PRIMCOLOR = scv->AddItem(L"...");
    m_ITM_ENVCOLOR = scv->AddItem(L"...");

    scv->SetOrigin(522, 80);
    scv->SetLayoutMode(CCanvas::LAYOUT_HORIZONTAL);
    m_ITM_FILLCOLOR_PREV0 = scv->AddItem(L" ");
    scv->SetLayoutMode(CCanvas::LAYOUT_VERTICAL);
    m_ITM_FILLCOLOR_PREV1 = scv->AddItem(L" ");
    scv->GotoOriginX();
    m_ITM_FOGCOLOR_PREV = scv->AddItem(L" ");
    m_ITM_BLENDCOLOR_PREV = scv->AddItem(L" ");
    m_ITM_PRIMCOLOR_PREV = scv->AddItem(L" ");
    m_ITM_ENVCOLOR_PREV = scv->AddItem(L" ");

    scv->SetItemBgColor(m_ITM_FILLCOLOR_PREV0, RGB(0, 0, 0));
    scv->SetItemBgColor(m_ITM_FILLCOLOR_PREV1, RGB(0, 0, 0));
    scv->SetItemBgColor(m_ITM_FOGCOLOR_PREV, RGB(0, 0, 0));
    scv->SetItemBgColor(m_ITM_BLENDCOLOR_PREV, RGB(0, 0, 0));
    scv->SetItemBgColor(m_ITM_PRIMCOLOR_PREV, RGB(0, 0, 0));
    scv->SetItemBgColor(m_ITM_ENVCOLOR_PREV, RGB(0, 0, 0));

    // Lights
    scv->SetOrigin(568, 10);
    scv->AddItem(L"LIGHTS", headerColor);
    scv->AddItem(L" LOOKATX", labelColor);
    scv->AddItem(L" LOOKATY", labelColor);
    scv->AddItem(L" LIGHT1", labelColor);
    scv->AddItem(L" LIGHT2", labelColor);
    scv->AddItem(L" LIGHT3", labelColor);
    scv->AddItem(L" LIGHT4", labelColor);
    scv->AddItem(L" LIGHT5", labelColor);
    scv->AddItem(L" LIGHT6", labelColor);
    scv->AddItem(L" LIGHT7", labelColor);
    scv->AddItem(L" LIGHT8", labelColor);

    scv->SetOrigin(628, 20);
    for (int i = 0; i < 10; i++)
    {
        m_ITM_LIGHTS[i] = scv->AddItem(L"...");
    }

	// Tile descriptors
    scv->SetOrigin(10, 210);
	scv->AddItem(L"TILE DESCRIPTORS", headerColor);
	scv->AddItem(L" # TMEM  SIZ FMT  LINE SHIFTS MASKS CMS SHIFTT MASKT CMT SCALES SCALET PALETTE LEVELS ON", labelColor);

    scv->SetOrigin(16, 230);
	for (int i = 0; i < 8; i++)
	{
		m_ITM_TILES[i] = scv->AddItem(L"...");
	}

    SetTimer(TIMER_ID_DRAW, 20, NULL);

    LoadWindowPos();
    WindowCreated();

	UpdateStateCanvas((CHleGfxState*)&m_GfxParser);

    return TRUE;
}

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
		//g_Settings->SaveBool(Debugger_SteppingOps, false);
        m_bRefreshPending = true;
        m_StatusText.SetWindowText(L"Waiting for RSP task...");
        ::EnableWindow(GetDlgItem(IDC_BTN_REFRESH), false);
        break;
    case IDC_BTN_EXPORT:
        Export();
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

	//g_Settings->SaveBool(Debugger_SteppingOps, true);

	m_bShowRender = false;

    uint32_t ucodeAddr, dlistAddr, dlistSize;

	// not thread safe
    g_MMU->LW_VAddr(0xA4000FD0, ucodeAddr);
    g_MMU->LW_VAddr(0xA4000FF0, dlistAddr);
    g_MMU->LW_VAddr(0xA4000FF4, dlistSize);

    m_GfxParser.Run(ucodeAddr, dlistAddr, dlistSize);

	ucode_info_t* ucodeInfo = m_GfxParser.GetMicrocodeInfo();

    size_t numCommands = m_GfxParser.GetCommandCount();
    size_t numResources = m_GfxParser.GetRamResourceCount();
    size_t numTriangles = m_GfxParser.GetTriangleCount();

    //ucode_version_t ucodeVersion = m_GfxParser.GetUCodeVersion();
    //uint32_t ucodeChecksum = m_GfxParser.GetUCodeChecksum();

    stdstr strStatus = ucodeInfo->ucodeName;
    strStatus += stdstr_f(" (Checksum: 0x%08X) - %d commands, %d triangles", ucodeInfo->checksum, numCommands, numTriangles);
    m_StatusText.SetWindowText(strStatus.ToUTF16().c_str());

    m_DisplayListCtrl.SetRedraw(FALSE);
    m_DisplayListCtrl.DeleteAllItems();
    
    for (size_t nCmd = 0; nCmd < numCommands; nCmd++)
    {
        decoded_cmd_t* dc = m_GfxParser.GetLoggedCommand(nCmd);

        stdstr strVirtAddress = stdstr_f("%08X", dc->virtualAddress);
        stdstr strSegOffset = stdstr_f("%08X", dc->address);
        //stdstr strRawCommand = stdstr_f("%08X %08X", dc->rawCommand.w0, dc->rawCommand.w1);

        m_DisplayListCtrl.AddItem(nCmd, DisplayListCtrl_Col_VAddr, strVirtAddress.ToUTF16().c_str());
        m_DisplayListCtrl.AddItem(nCmd, DisplayListCtrl_Col_SegOffset, strSegOffset.ToUTF16().c_str());
        //m_DisplayListCtrl.AddItem(nCmd, DisplayListCtrl_Col_RawCommand, strRawCommand.ToUTF16().c_str());
        m_DisplayListCtrl.AddItem(nCmd, DisplayListCtrl_Col_Command, stdstr(dc->name).ToUTF16().c_str());
        m_DisplayListCtrl.AddItem(nCmd, DisplayListCtrl_Col_Parameters, dc->params.ToUTF16().c_str());
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
            HTREEITEM hItem = m_ResourceTreeCtrl.InsertItem(str.ToUTF16().c_str(), hParentItem, NULL);
            m_ResourceTreeCtrl.SetItemData(hItem, nRes);
        }
    }

    m_ResourceTreeCtrl.SetRedraw(TRUE);

    ::EnableWindow(GetDlgItem(IDC_BTN_EXPORT), TRUE);
    ::EnableWindow(GetDlgItem(IDC_BTN_REFRESH), TRUE);
    m_bRefreshPending = false;
	m_bShowRender = true;
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

void CDebugDisplayList::UpdateStateCanvas(CHleGfxState* state)
{
	ucode_info_t* ucodeInfo = m_GfxParser.GetMicrocodeInfo();

	if (state == NULL)
	{
		return;
	}

	CCanvas*& scv = m_StateCanvas;
    COLORREF clrOn = RGB(0xFF, 0xFF, 0xFF);
    COLORREF clrOff = RGB(0x44, 0x44, 0x44);

	// Image addresses
	stdstr strTextureImage = stdstr_f("%08X", state->m_dpTextureImage);
	stdstr strColorImage = stdstr_f("%08X", state->m_dpColorImage);
	stdstr strDepthImage = stdstr_f("%08X", state->m_dpDepthImage);
	scv->SetItemText(m_ITM_TEXTUREIMAGE, strTextureImage.ToUTF16().c_str());
	scv->SetItemText(m_ITM_COLORIMAGE, strColorImage.ToUTF16().c_str());
	scv->SetItemText(m_ITM_DEPTHIMAGE, strDepthImage.ToUTF16().c_str());

	// Geometry mode
	bool shading_smooth, cull_front, cull_back;

	if (ucodeInfo->ucodeId == UCODE_F3DEX2)
	{
		shading_smooth = state->m_spGeometryMode.f3dex2.shading_smooth;
		cull_front = state->m_spGeometryMode.f3dex2.cull_front;
		cull_back = state->m_spGeometryMode.f3dex2.cull_back;
	}
	else
	{
		shading_smooth = state->m_spGeometryMode.f3d.shading_smooth;
		cull_front = state->m_spGeometryMode.f3d.cull_front;
		cull_back = state->m_spGeometryMode.f3d.cull_back;
	}

	scv->SetItemColor(m_ITM_GM_ZBUFFER, state->m_spGeometryMode.f3d.zbuffer ? clrOn : clrOff);
	scv->SetItemColor(m_ITM_GM_SHADE, state->m_spGeometryMode.f3d.shade ? clrOn : clrOff);
	scv->SetItemColor(m_ITM_GM_FOG, state->m_spGeometryMode.f3d.fog ? clrOn : clrOff);
	scv->SetItemColor(m_ITM_GM_LIGHTING, state->m_spGeometryMode.f3d.lighting ? clrOn : clrOff);
	scv->SetItemColor(m_ITM_GM_TEXTURE_GEN, state->m_spGeometryMode.f3d.texture_gen ? clrOn : clrOff);
	scv->SetItemColor(m_ITM_GM_TEXTURE_GEN_LINEAR, state->m_spGeometryMode.f3d.texture_gen_linear ? clrOn : clrOff);
	scv->SetItemColor(m_ITM_GM_LOD, state->m_spGeometryMode.f3d.lod ? clrOn : clrOff);
	scv->SetItemColor(m_ITM_GM_CLIPPING, state->m_spGeometryMode.f3d.clipping ? clrOn : clrOff);
	scv->SetItemColor(m_ITM_GM_SHADING_SMOOTH, state->m_spGeometryMode.f3d.shading_smooth ? clrOn : clrOff);
	scv->SetItemColor(m_ITM_GM_CULL_FRONT, state->m_spGeometryMode.f3d.cull_front ? clrOn : clrOff);
	scv->SetItemColor(m_ITM_GM_CULL_BACK, state->m_spGeometryMode.f3d.cull_back ? clrOn : clrOff);

	// Combiner
	stdstr strCycle1Color = stdstr_f("(%s - %s) * %s + %s",
		CGfxLabels::LookupName(CGfxLabels::CCMuxA, state->m_dpCombiner.a0),
		CGfxLabels::LookupName(CGfxLabels::CCMuxB, state->m_dpCombiner.b0),
		CGfxLabels::LookupName(CGfxLabels::CCMuxC, state->m_dpCombiner.c0),
		CGfxLabels::LookupName(CGfxLabels::CCMuxD, state->m_dpCombiner.d0));

	stdstr strCycle1Alpha = stdstr_f("(%s - %s) * %s + %s",
		CGfxLabels::LookupName(CGfxLabels::ACMuxA_B_D, state->m_dpCombiner.Aa0),
		CGfxLabels::LookupName(CGfxLabels::ACMuxA_B_D, state->m_dpCombiner.Ab0),
		CGfxLabels::LookupName(CGfxLabels::ACMuxC, state->m_dpCombiner.Ac0),
		CGfxLabels::LookupName(CGfxLabels::ACMuxA_B_D, state->m_dpCombiner.Ad0));

	stdstr strCycle2Color = stdstr_f("(%s - %s) * %s + %s",
		CGfxLabels::LookupName(CGfxLabels::CCMuxA, state->m_dpCombiner.a1),
		CGfxLabels::LookupName(CGfxLabels::CCMuxB, state->m_dpCombiner.b1),
		CGfxLabels::LookupName(CGfxLabels::CCMuxC, state->m_dpCombiner.c1),
		CGfxLabels::LookupName(CGfxLabels::CCMuxD, state->m_dpCombiner.d1));

	stdstr strCycle2Alpha = stdstr_f("(%s - %s) * %s + %s",
		CGfxLabels::LookupName(CGfxLabels::ACMuxA_B_D, state->m_dpCombiner.Aa1),
		CGfxLabels::LookupName(CGfxLabels::ACMuxA_B_D, state->m_dpCombiner.Ab1),
		CGfxLabels::LookupName(CGfxLabels::ACMuxC, state->m_dpCombiner.Ac1),
		CGfxLabels::LookupName(CGfxLabels::ACMuxA_B_D, state->m_dpCombiner.Ad1));

	scv->SetItemText(m_ITM_CC1_COLOR, strCycle1Color.ToUTF16().c_str());
	scv->SetItemText(m_ITM_CC1_ALPHA, strCycle1Color.ToUTF16().c_str());
	scv->SetItemText(m_ITM_CC2_COLOR, strCycle1Color.ToUTF16().c_str());
	scv->SetItemText(m_ITM_CC2_ALPHA, strCycle1Color.ToUTF16().c_str());

	// Othermode HI
	othermode_h_t* omh = &state->m_dpOtherMode_h;

	scv->SetItemText(m_ITM_OMH_PIPELINE, stdstr(CGfxLabels::OtherMode_pm[omh->pm]).ToUTF16().c_str());
	scv->SetItemText(m_ITM_OMH_COLORDITHER, stdstr(CGfxLabels::OtherMode_cd[omh->cd]).ToUTF16().c_str());
	scv->SetItemText(m_ITM_OMH_CYCLETYPE, stdstr(CGfxLabels::OtherMode_cyc[omh->cyc]).ToUTF16().c_str());
	scv->SetItemText(m_ITM_OMH_TEXTPERSP, stdstr(CGfxLabels::OtherMode_tp[omh->tp]).ToUTF16().c_str());
	scv->SetItemText(m_ITM_OMH_TEXTDETAIL, stdstr(CGfxLabels::OtherMode_td[omh->td]).ToUTF16().c_str());
	scv->SetItemText(m_ITM_OMH_TEXTLOD, stdstr(CGfxLabels::OtherMode_tl[omh->tl]).ToUTF16().c_str());
	scv->SetItemText(m_ITM_OMH_TEXTLUT, stdstr(CGfxLabels::OtherMode_tt[omh->tt]).ToUTF16().c_str());
	scv->SetItemText(m_ITM_OMH_TEXTFILT, stdstr(CGfxLabels::OtherMode_tf[omh->tf]).ToUTF16().c_str());
	scv->SetItemText(m_ITM_OMH_TEXTCONV, stdstr(CGfxLabels::OtherMode_tc[omh->tc]).ToUTF16().c_str());
	scv->SetItemText(m_ITM_OMH_COMBKEY, stdstr(CGfxLabels::OtherMode_ck[omh->ck]).ToUTF16().c_str());
	scv->SetItemText(m_ITM_OMH_RGBDITHER, stdstr(CGfxLabels::OtherMode_rd[omh->rd]).ToUTF16().c_str());
	scv->SetItemText(m_ITM_OMH_ALPHADITHER, stdstr(CGfxLabels::OtherMode_ad[omh->ad]).ToUTF16().c_str());

	// RDP Colors
	scv->SetItemText(m_ITM_FILLCOLOR, stdstr_f("%08X", state->m_dpFillColor).ToUTF16().c_str());
	scv->SetItemText(m_ITM_FOGCOLOR, stdstr_f("%08X", state->m_dpFogColor).ToUTF16().c_str());
	scv->SetItemText(m_ITM_BLENDCOLOR, stdstr_f("%08X", state->m_dpBlendColor).ToUTF16().c_str());
	scv->SetItemText(m_ITM_PRIMCOLOR, stdstr_f("%08X", state->m_dpPrimColor).ToUTF16().c_str());
	scv->SetItemText(m_ITM_ENVCOLOR, stdstr_f("%08X", state->m_dpEnvColor).ToUTF16().c_str());

    uint16_t fill0 = state->m_dpFillColor >> 16;
    uint16_t fill1 = state->m_dpFillColor & 0xFFFF;

    scv->SetItemBgColor(m_ITM_FILLCOLOR_PREV0, RGBFROM5551(fill0));
    scv->SetItemBgColor(m_ITM_FILLCOLOR_PREV1, RGBFROM5551(fill1));
    scv->SetItemBgColor(m_ITM_FOGCOLOR_PREV, RGBFROM8888(state->m_dpFogColor));
    scv->SetItemBgColor(m_ITM_BLENDCOLOR_PREV, RGBFROM8888(state->m_dpBlendColor));
    scv->SetItemBgColor(m_ITM_PRIMCOLOR_PREV, RGBFROM8888(state->m_dpPrimColor));
    scv->SetItemBgColor(m_ITM_ENVCOLOR_PREV, RGBFROM8888(state->m_dpEnvColor));

	// Tile Descriptors
	for (int nTile = 0; nTile < 8; nTile++)
	{
		tile_t* t = &state->m_dpTileDescriptors[nTile];

		const char* sizName = CGfxLabels::TexelSizesShort[t->siz];
		const char* fmtName = CGfxLabels::ImageFormatsShort[t->fmt];

		stdstr strTileDescriptor = stdstr_f(
			"%d 0x%03X %3s %4s %4d %6d %5d %3d %6d %5d %3d 0x%04X 0x%04X %7d %6d  %d",
			nTile,
			t->tmem, sizName, fmtName, t->line,
			t->shifts, t->masks, t->cms,
			t->shiftt, t->maskt, t->cmt,
			t->scaleS, t->scaleT,
			t->palette,
			t->mipmapLevels, t->enabled
		);

		scv->SetItemText(m_ITM_TILES[nTile], strTileDescriptor.ToUTF16().c_str());
	}

    // Lights
    for (int i = 0; i < 10; i++)
    {
        stdstr strLight = stdstr_f("%02X%02X%02X (%d,%d,%d)",
            state->m_spLights[i].colorA[0],
            state->m_spLights[i].colorA[1],
            state->m_spLights[i].colorA[2],
            state->m_spLights[i].direction[0],
            state->m_spLights[i].direction[1],
            state->m_spLights[i].direction[2]);

        scv->SetItemText(m_ITM_LIGHTS[i], strLight.ToUTF16().c_str());

        if (i >= 2) {
            int lightIdx = i - 2;
            scv->SetItemColor(m_ITM_LIGHTS[i], lightIdx > state->m_spNumLights ? RGB(0x44, 0x44, 0x44) : RGB(0xFF, 0xFF, 0xFF));
        }
    }

    // Othermode LO
    othermode_l_t oml = state->m_dpOtherMode_l;

    stdstr strBlendCycle1 = stdstr_f("(%s * %s + %s - %s) / (%s + %s)",
        CGfxLabels::OtherModeL_blpm[oml.p0],
        CGfxLabels::OtherModeL_bla[oml.a0],
        CGfxLabels::OtherModeL_blpm[oml.m0],
        CGfxLabels::OtherModeL_blb[oml.b0],
        CGfxLabels::OtherModeL_bla[oml.a0],
        CGfxLabels::OtherModeL_blb[oml.b0]);

    stdstr strBlendCycle2 = stdstr_f("(%s * %s + %s - %s) / (%s + %s)",
        CGfxLabels::OtherModeL_blpm[oml.p1],
        CGfxLabels::OtherModeL_bla[oml.a1],
        CGfxLabels::OtherModeL_blpm[oml.m1],
        CGfxLabels::OtherModeL_blb[oml.b1],
        CGfxLabels::OtherModeL_bla[oml.a1],
        CGfxLabels::OtherModeL_blb[oml.b1]);

    stdstr strCVG_DST = stdstr_f(" %s", CGfxLabels::OtherModeL_cvgdst[oml.cvg_dst]);
    stdstr strZMODE = stdstr_f(" %s", CGfxLabels::OtherModeL_zmode[oml.zmode]);

    scv->SetItemColor(m_ITM_OML_AA_EN, oml.aa_en ? clrOn : clrOff);
    scv->SetItemColor(m_ITM_OML_Z_CMP, oml.z_cmp ? clrOn : clrOff);
    scv->SetItemColor(m_ITM_OML_Z_UPD, oml.z_upd ? clrOn : clrOff);
    scv->SetItemColor(m_ITM_OML_IM_RD, oml.im_rd ? clrOn : clrOff);
    scv->SetItemColor(m_ITM_OML_CLR_ON_CVG, oml.clr_on_cvg ? clrOn : clrOff);
    scv->SetItemText(m_ITM_OML_CVG_DST, strCVG_DST.ToUTF16().c_str());
    scv->SetItemText(m_ITM_OML_ZMODE, strZMODE.ToUTF16().c_str());
    scv->SetItemColor(m_ITM_OML_CVG_X_ALPHA, oml.cvg_x_alpha ? clrOn : clrOff);
    scv->SetItemColor(m_ITM_OML_ALPHA_CVG_SEL, oml.alpha_cvg_sel ? clrOn : clrOff);
    scv->SetItemColor(m_ITM_OML_FORCE_BL, oml.force_bl ? clrOn : clrOff);

    scv->SetItemText(m_ITM_OML_CYCLE1, strBlendCycle1.ToUTF16().c_str());
    scv->SetItemText(m_ITM_OML_CYCLE2, strBlendCycle2.ToUTF16().c_str());
}

LRESULT CDebugDisplayList::OnListItemChanged(NMHDR* pNMHDR)
{
    NMITEMACTIVATE* pIA = reinterpret_cast<NMITEMACTIVATE*>(pNMHDR);
    int nItem = pIA->iItem;
    CHleGfxState* state = m_GfxParser.GetLoggedState(nItem);
	UpdateStateCanvas(state);
    m_bShowRender = true;
    return FALSE;
}

LRESULT CDebugDisplayList::OnListDblClicked(NMHDR* /*pNMHDR*/)
{
    int nItem = m_DisplayListCtrl.GetSelectedIndex();
    CHleGfxState* state = m_GfxParser.GetLoggedState(nItem);

    if (state == NULL)
    {
        return FALSE;
    }

    uint32_t cmdAddress = state->SegmentedToVirtual(state->m_spCommandAddress);
    m_Debugger->Debug_ShowMemoryLocation(cmdAddress, true);
    return FALSE;
}

void CDebugDisplayList::OnTimer(UINT_PTR nIDEvent)
{
    if (!m_bShowRender)
    {
        return;
    }

    if (nIDEvent == TIMER_ID_DRAW)
    {
        float moveSpeed = 0.25f;

        if (m_RenderView->KeyDown(VK_SHIFT)) moveSpeed /= 8;

        if (m_RenderView->KeyDown('W')) m_Camera.TranslateZ(moveSpeed);
        if (m_RenderView->KeyDown('S')) m_Camera.TranslateZ(-moveSpeed);
        if (m_RenderView->KeyDown('D')) m_Camera.TranslateX(moveSpeed);
        if (m_RenderView->KeyDown('A')) m_Camera.TranslateX(-moveSpeed);

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
        m_bShowRender = false;
        PreviewImageResource(res);
        break;
    case RES_DL:
        m_bShowRender = true;
        break;
    default:
        ::SetWindowText(GetDlgItem(IDC_EDIT_RESINFO), strDefaultInfo.ToUTF16().c_str());
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
    //CHleGfxState* state = m_GfxParser.GetLoggedState(nItem);

    //int depth = state->m_spStackIndex * 10;

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

    CGfxParser::ConvertImage(imageBuffer, imgSrc, res->imageFmt, res->imageSiz, numTexels);

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

    int xoffs = (int)(rc.Width() / 2 - (width*scale / 2));
    int yoffs = (int)(rc.Height() / 2 - (height*scale / 2));

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

    ::SetWindowText(GetDlgItem(IDC_EDIT_RESINFO), strImageInfo.ToUTF16().c_str());
}

void CDebugDisplayList::Export(void)
{
    OPENFILENAME openfilename;
    TCHAR filePath[_MAX_PATH];

    memset(&filePath, 0, sizeof(filePath));
    memset(&openfilename, 0, sizeof(openfilename));

    wsprintf(filePath, L"*.c");

    const TCHAR* filters = (
        /*1*/ L"Display List Source (*.c)\0*.c;\0"
        /*2*/ L"Raw Display List Source (*.c)\0*.c;\0"
        /*3*/ L"Microcode binary (*.bin)\0*.bin;\0"
        /*4*/ L"RAM GFX Snapshot (*.bin)\0*.bin;\0"
    );

    const char *defaultExtensions[] = { "", ".c", ".c", ".bin", ".bin" };

    openfilename.lStructSize = sizeof(openfilename);
    openfilename.hwndOwner = (HWND)m_hWnd;
    openfilename.lpstrFilter = filters;
    openfilename.lpstrFile = filePath;
    openfilename.lpstrInitialDir = L".";
    openfilename.nMaxFile = MAX_PATH;
    openfilename.Flags = OFN_HIDEREADONLY;

    if (GetSaveFileName(&openfilename))
    {
        stdstr path;
        path.FromUTF16(filePath);

        if (openfilename.nFileExtension == 0)
        {
            path += defaultExtensions[openfilename.nFilterIndex];
        }

        switch (openfilename.nFilterIndex)
        {
        case 1: m_GfxParser.ExportDisplayListSource(path.c_str()); break;
        case 2: m_GfxParser.ExportRawDisplayListSource(path.c_str()); break;
        case 3: m_GfxParser.ExportMicrocode(path.c_str()); break;
        case 4: m_GfxParser.ExportSnapshot(path.c_str()); break;
        }
    }
}

/****************************************/

CCanvasItem::CCanvasItem(int x, int y, const TCHAR* text, COLORREF color) :
    m_X(x),
    m_Y(y),
    m_BgColor(RGB(0,0,0)),
    m_bHaveBgColor(false),
	m_Color(color),
	m_BoundingRect(x, y, x, y)
{
    //m_bNeedRedraw = true;

    if (text != NULL)
    {
        m_Text = wcsdup(text);
    }
    else
    {
        m_Text = NULL;
    }
}

CCanvas::CCanvas(void) :
    m_BackDC(NULL),
    m_BackBMP(NULL),
	m_BackgroundColor(RGB(0x22, 0x22, 0x22)),
	m_ForegroundColor(RGB(255, 255, 255)),
    m_OriginX(0),
    m_OriginY(0),
    m_ItemPosX(0),
    m_ItemPosY(0),
    m_LayoutMode(LAYOUT_VERTICAL)
{
}

void CCanvas::RegisterClass()
{
    this->GetWndClassInfo().m_wc.lpfnWndProc = m_pfnSuperWindowProc;
    this->GetWndClassInfo().Register(&m_pfnSuperWindowProc);
}

void CCanvas::SetOrigin(int x, int y)
{
    m_OriginX = x;
    m_OriginY = y;
    m_ItemPosX = x;
    m_ItemPosY = y;
}

void CCanvas::SetLayoutMode(int mode)
{
    m_LayoutMode = mode;
}

void CCanvas::Init(void)
{
    CRect wndRc;
    GetWindowRect(&wndRc);

    if (m_BackDC != NULL)
    {
        return;
    }

    HDC hdc = GetDC();
    HBITMAP hOldBMP;
    HFONT   hOldFont;

    m_BackDC = CreateCompatibleDC(hdc);
    m_BackBMP = CreateCompatibleBitmap(hdc, wndRc.Width(), wndRc.Height());
    hOldBMP = (HBITMAP)SelectObject(m_BackDC, m_BackBMP);

    m_Font = CreateFont(8, 6, 0, 0,
        FW_DONTCARE,
        FALSE,
        FALSE,
        FALSE,
        DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY,
        FF_DONTCARE,
        L"Terminal");

    hOldFont = (HFONT)SelectObject(m_BackDC, m_Font);

    DeleteObject(hOldFont);
    DeleteObject(hOldBMP);
    ReleaseDC(hdc);

	SetBkColor(m_BackDC, m_BackgroundColor);

    CRect clrRc(0, 0, wndRc.Width(), wndRc.Height());
    HBRUSH hbrush = CreateSolidBrush(m_BackgroundColor);
    FillRect(m_BackDC, clrRc, hbrush);
    DeleteObject(hbrush);
}

LRESULT CCanvas::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(&ps);
    CRect rc = ps.rcPaint;

    BitBlt(hdc,
        rc.left, rc.top,
        rc.Width(), rc.Height(),
        m_BackDC,
        rc.left, rc.top,
        SRCCOPY);

    EndPaint(&ps);
    return FALSE;
}

void CCanvas::DrawItem(CCanvasItem* item)
{
    // erase the old bounding rect
    if (item->m_BoundingRect.Width() > 0 &&
        item->m_BoundingRect.Height() > 0)
    {
        HBRUSH hbrush = CreateSolidBrush(m_BackgroundColor);
        FillRect(m_BackDC, &item->m_BoundingRect, hbrush);
        InvalidateRect(&item->m_BoundingRect);
        DeleteObject(hbrush);
    }

    COLORREF orgBgColor;

    if (item->m_bHaveBgColor)
    {
        orgBgColor = SetBkColor(m_BackDC, item->m_BgColor);
    }
    
    // update bounding rect and draw text
    ::SetTextColor(m_BackDC, item->m_Color);
    DrawText(m_BackDC, item->m_Text, -1, &item->m_BoundingRect, DT_TOP | DT_CALCRECT);
    DrawText(m_BackDC, item->m_Text, -1, &item->m_BoundingRect, DT_TOP);
    InvalidateRect(&item->m_BoundingRect);

    if (item->m_bHaveBgColor)
    {
        SetBkColor(m_BackDC, orgBgColor);
    }
}

CCanvasItem* CCanvas::GetItem(int index)
{
	if (index < 0 || index >= m_Items.size())
	{
		return NULL;
	}

	return m_Items[index];
}

size_t CCanvas::AddItem(const TCHAR* text, COLORREF color)
{
    CCanvasItem* item = new CCanvasItem(m_ItemPosX, m_ItemPosY, text, color);
    DrawItem(item);
    m_Items.push_back(item);

    if (m_LayoutMode & LAYOUT_VERTICAL)
    {
        m_ItemPosY += item->m_BoundingRect.Height() + 2;
    }

    if (m_LayoutMode & LAYOUT_HORIZONTAL)
    {
        m_ItemPosX += item->m_BoundingRect.Width() + 2;
    }
    
    return m_Items.size() - 1;
}

void CCanvas::SetItemColor(int id, COLORREF color)
{
	CCanvasItem* item = GetItem(id);

    if (item == NULL)
    {
        return;
    }

    if (item->m_Color == color)
    {
        return;
    }

    item->m_Color = color;
    DrawItem(item);
}

void CCanvas::SetItemBgColor(int id, COLORREF color)
{
    CCanvasItem* item = GetItem(id);

    if (item == NULL)
    {
        return;
    }

    if (item->m_BgColor == color && item->m_bHaveBgColor)
    {
        return;
    }

    item->m_bHaveBgColor = true;
    item->m_BgColor = color;
    DrawItem(item);
}

void CCanvas::SetItemText(int id, const TCHAR* text)
{
	CCanvasItem* item = GetItem(id);

    if (item == NULL)
    {
        return;
    }

    if (wcscmp(text, item->m_Text) == 0)
    {
        return;
    }

    free(item->m_Text);
    item->m_Text = wcsdup(text);
    DrawItem(item);
}

void CCanvas::GotoOriginX(void)
{
    m_ItemPosX = m_OriginX;
}

/****************************************/

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