#include "stdafx.h"
#include "DebuggerUI.h"

#include "Util/DisplayListParser.h"

CDebugDisplayList::CDebugDisplayList(CDebuggerUI* debugger) :
	CDebugDialog<CDebugDisplayList>(debugger),
	m_bRefreshPending(false)
{
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

	LoadWindowPos();
	WindowCreated();
	return TRUE;
}

LRESULT CDebugDisplayList::OnDestroy(void)
{
	m_DisplayListCtrl.Detach();
	m_ResourceTreeCtrl.Detach();
    m_StateTextbox.Detach();
	m_StatusText.Detach();
	//m_TileListCtrl.Detach();
	return 0;
}

LRESULT CDebugDisplayList::OnClicked(WORD wNotifyCode, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled)
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

    m_DisplayListParser.Reset(ucodeAddr, dlistAddr, dlistSize);
    
	ucode_version_t ucodeVersion = m_DisplayListParser.GetUCodeVersion();

	stdstr strStatus = (ucodeVersion != UCODE_UNKNOWN) ? m_DisplayListParser.GetUCodeName() : "Unknown microcode";
	strStatus += stdstr_f(" (Checksum: 0x%08X)", m_DisplayListParser.GetUCodeChecksum());

	m_DisplayListCtrl.SetRedraw(FALSE);
	m_DisplayListCtrl.DeleteAllItems();
	
	m_ListColors.clear();
	m_RamResources.clear();

    int nCommand = 0;
    uint32_t triangleCount = 0;

    while (!m_DisplayListParser.IsDone())
    {
        uint32_t virtAddress = m_DisplayListParser.GetCommandVirtualAddress();
        uint32_t segAddress = m_DisplayListParser.GetCommandAddress();
        dl_cmd_t command = m_DisplayListParser.GetRawCommand();
        int numTabs = m_DisplayListParser.GetStackIndex();

        stdstr strVirtAddress = stdstr_f("%08X", virtAddress);
        stdstr strAddress = stdstr_f("%08X", segAddress);
        stdstr strRawCommand = stdstr_f("%08X %08X", command.w0, command.w1);
		
		decode_context_t dc;
        m_DisplayListParser.StepDecode(&dc);

        m_DisplayListCtrl.AddItem(nCommand, DisplayListCtrl_Col_VAddr, strVirtAddress.c_str());
        m_DisplayListCtrl.AddItem(nCommand, DisplayListCtrl_Col_SegOffset, strAddress.c_str());
        m_DisplayListCtrl.AddItem(nCommand, DisplayListCtrl_Col_RawCommand, strRawCommand.c_str());
        m_DisplayListCtrl.AddItem(nCommand, DisplayListCtrl_Col_Command, dc.name);
        m_DisplayListCtrl.AddItem(nCommand, DisplayListCtrl_Col_Parameters, dc.params);

		m_ListColors.push_back({ dc.listFgColor, dc.listBgColor });

		if (dc.dramResource.type != RES_NONE)
		{
			m_RamResources.push_back(dc.dramResource);
		}

        triangleCount += dc.numTris;

        nCommand++;
    }

	m_DisplayListCtrl.SetRedraw(TRUE);

    m_ResourceTreeCtrl.SetRedraw(FALSE);
    ResetResourceTreeCtrl();

    CTreeViewCtrl& rtc = m_ResourceTreeCtrl;

	for (size_t resIdx = 0; resIdx < m_RamResources.size(); resIdx++)
	{
		dram_resource_t* res = &m_RamResources[resIdx];
        stdstr str = stdstr_f("0x%08X (0x%08X)", res->address, res->virtAddress);
		//stdstr strType;
        
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
            str += " Texture";
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
            m_ResourceTreeCtrl.SetItemData(hItem, resIdx);
        }
	}

    m_ResourceTreeCtrl.SetRedraw(TRUE);

	::EnableWindow(GetDlgItem(IDC_BTN_REFRESH), TRUE);
	
	strStatus += stdstr_f(" - %d commands, %d triangles", nCommand, triangleCount);
	m_StatusText.SetWindowTextA(strStatus.c_str());

	m_bRefreshPending = false;
}

void CDebugDisplayList::SetPreviewColor(WORD ctrlId, uint32_t colorPair)
{
	uint16_t color16 = colorPair & 0xFFFF;
	uint8_t r = ((color16 >> 11) & 0x1F) * (255.0f / 31.0f);
	uint8_t g = ((color16 >> 6) & 0x1F) * (255.0f / 31.0f);
	uint8_t b = ((color16 >> 1) & 0x1F) * (255.0f / 31.0f);
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

    CHleDmemState* state = m_DisplayListParser.GetLoggedState(nItem);

	// Geometry mode

	::SendMessage(GetDlgItem(IDC_CHK_GM_ZBUFFER), BM_SETCHECK, state->geometryMode.zbuffer ? BST_CHECKED : BST_UNCHECKED, 0);
	::SendMessage(GetDlgItem(IDC_CHK_GM_SHADE), BM_SETCHECK, state->geometryMode.shade ? BST_CHECKED : BST_UNCHECKED, 0);
	::SendMessage(GetDlgItem(IDC_CHK_GM_CULL_FRONT), BM_SETCHECK, state->geometryMode.cull_front ? BST_CHECKED : BST_UNCHECKED, 0);
	::SendMessage(GetDlgItem(IDC_CHK_GM_CULL_BACK), BM_SETCHECK, state->geometryMode.cull_back ? BST_CHECKED : BST_UNCHECKED, 0);
	::SendMessage(GetDlgItem(IDC_CHK_GM_FOG), BM_SETCHECK, state->geometryMode.fog ? BST_CHECKED : BST_UNCHECKED, 0);
	::SendMessage(GetDlgItem(IDC_CHK_GM_LIGHTING), BM_SETCHECK, state->geometryMode.lighting ? BST_CHECKED : BST_UNCHECKED, 0);
	::SendMessage(GetDlgItem(IDC_CHK_GM_TEXTURE_GEN), BM_SETCHECK, state->geometryMode.texture_gen ? BST_CHECKED : BST_UNCHECKED, 0);
	::SendMessage(GetDlgItem(IDC_CHK_GM_TEXTURE_GEN_LINEAR), BM_SETCHECK, state->geometryMode.texture_gen_linear ? BST_CHECKED : BST_UNCHECKED, 0);
	::SendMessage(GetDlgItem(IDC_CHK_GM_SHADING_SMOOTH), BM_SETCHECK, state->geometryMode.shading_smooth ? BST_CHECKED : BST_UNCHECKED, 0);
	::SendMessage(GetDlgItem(IDC_CHK_GM_LOD), BM_SETCHECK, state->geometryMode.lod ? BST_CHECKED : BST_UNCHECKED, 0);
	::SendMessage(GetDlgItem(IDC_CHK_GM_CLIPPING), BM_SETCHECK, state->geometryMode.clipping ? BST_CHECKED : BST_UNCHECKED, 0);

	// Texture address

	::SetWindowText(GetDlgItem(IDC_EDIT_TEXTUREIMAGE), stdstr_f("0x%08X (0x%08X)", state->textureImage, state->SegmentedToVirtual(state->textureImage)).c_str());
	::SetWindowText(GetDlgItem(IDC_EDIT_DEPTHIMAGE), stdstr_f("0x%08X (0x%08X)", state->depthImage, state->SegmentedToVirtual(state->depthImage)).c_str());
	::SetWindowText(GetDlgItem(IDC_EDIT_COLORIMAGE), stdstr_f("0x%08X (0x%08X)", state->colorImage, state->SegmentedToVirtual(state->colorImage)).c_str());

	// rdp colors

	::SetWindowText(GetDlgItem(IDC_EDIT_FILLCOLOR), stdstr_f("0x%08X", state->fillColor).c_str());
	::SetWindowText(GetDlgItem(IDC_EDIT_FOGCOLOR), stdstr_f("0x%08X", state->fogColor).c_str());
	::SetWindowText(GetDlgItem(IDC_EDIT_BLENDCOLOR), stdstr_f("0x%08X", state->blendColor).c_str());
	::SetWindowText(GetDlgItem(IDC_EDIT_PRIMCOLOR), stdstr_f("0x%08X", state->primColor).c_str());
	::SetWindowText(GetDlgItem(IDC_EDIT_ENVCOLOR), stdstr_f("0x%08X", state->envColor).c_str());

	SetPreviewColor(IDC_PREVIEW_FILLCOLOR, state->fillColor);
	SetPreviewColor(IDC_PREVIEW_FOGCOLOR, state->fogColor);
	SetPreviewColor(IDC_PREVIEW_BLENDCOLOR, state->blendColor);
	SetPreviewColor(IDC_PREVIEW_PRIMCOLOR, state->primColor);
	SetPreviewColor(IDC_PREVIEW_ENVCOLOR, state->envColor);

	// tiles

	stdstr strTileDescriptors = "# tmem  siz fmt  line shiftS maskS cms shiftT maskT cmT scaleS scaleT palette levels on\r\n";

    for (int i = 0; i <= 7; i++)
    {
        hle_tile_descriptor_t* t = &state->tiles[i];
    
		if (t->enabled == 0 && i != 7)
		{
            // skip the render tiles if they are disabled
			continue;
		}

        const char* sizName = CDisplayListParser::LookupName(CDisplayListParser::TexelSizeShortNames, t->siz);
        const char* fmtName = CDisplayListParser::LookupName(CDisplayListParser::ImageFormatShortNames, t->fmt);

        strTileDescriptors += stdstr_f(
            "%d 0x%03X %3s %4s %4d %6d %5d %3d %6d %5d %3d 0x%04X 0x%04X %7d %6d %d\r\n",
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

	othermode_h_t* omh = &state->othermode_h;

	::SetWindowText(GetDlgItem(IDC_EDIT_OMH_PIPELINE), OtherModeNames::pm[omh->pm]);
	::SetWindowText(GetDlgItem(IDC_EDIT_OMH_COLORDITHER), OtherModeNames::cd[omh->cd]);
	::SetWindowText(GetDlgItem(IDC_EDIT_OMH_CYCLETYPE), OtherModeNames::cyc[omh->cyc]);
	::SetWindowText(GetDlgItem(IDC_EDIT_OMH_TEXTPERSP), OtherModeNames::tp[omh->tp]);
	::SetWindowText(GetDlgItem(IDC_EDIT_OMH_TEXTDETAIL), OtherModeNames::td[omh->td]);
	::SetWindowText(GetDlgItem(IDC_EDIT_OMH_TEXTLOD), OtherModeNames::tl[omh->tl]);
	::SetWindowText(GetDlgItem(IDC_EDIT_OMH_TEXTLUT), OtherModeNames::tt[omh->tt]);
	::SetWindowText(GetDlgItem(IDC_EDIT_OMH_TEXTFILT), OtherModeNames::tf[omh->tf]);
	::SetWindowText(GetDlgItem(IDC_EDIT_OMH_TEXTCONV), OtherModeNames::tc[omh->tc]);
	::SetWindowText(GetDlgItem(IDC_EDIT_OMH_COMBKEY), OtherModeNames::ck[omh->ck]);
	::SetWindowText(GetDlgItem(IDC_EDIT_OMH_RGBDITHER), OtherModeNames::rd[omh->rd]);
	::SetWindowText(GetDlgItem(IDC_EDIT_OMH_ALPHADITHER), OtherModeNames::ad[omh->ad]);

	/////////////

    stdstr strCycle1Color = stdstr_f("(%s - %s) * %s + %s\r\n",
        CDisplayListParser::LookupName(CDisplayListParser::CCMuxA, state->combiner.a0),
        CDisplayListParser::LookupName(CDisplayListParser::CCMuxB, state->combiner.b0),
        CDisplayListParser::LookupName(CDisplayListParser::CCMuxC, state->combiner.c0),
        CDisplayListParser::LookupName(CDisplayListParser::CCMuxD, state->combiner.d0));

    stdstr strCycle1Alpha = stdstr_f("(%s - %s) * %s + %s\r\n",
        CDisplayListParser::LookupName(CDisplayListParser::ACMuxA_B_D, state->combiner.Aa0),
        CDisplayListParser::LookupName(CDisplayListParser::ACMuxA_B_D, state->combiner.Ab0),
        CDisplayListParser::LookupName(CDisplayListParser::ACMuxC, state->combiner.Ac0),
        CDisplayListParser::LookupName(CDisplayListParser::ACMuxA_B_D, state->combiner.Ad0));

	stdstr strCycle2Color = stdstr_f("(%s - %s) * %s + %s\r\n",
		CDisplayListParser::LookupName(CDisplayListParser::CCMuxA, state->combiner.a1),
		CDisplayListParser::LookupName(CDisplayListParser::CCMuxB, state->combiner.b1),
		CDisplayListParser::LookupName(CDisplayListParser::CCMuxC, state->combiner.c1),
		CDisplayListParser::LookupName(CDisplayListParser::CCMuxD, state->combiner.d1));

	stdstr strCycle2Alpha = stdstr_f("(%s - %s) * %s + %s\r\n",
		CDisplayListParser::LookupName(CDisplayListParser::ACMuxA_B_D, state->combiner.Aa1),
		CDisplayListParser::LookupName(CDisplayListParser::ACMuxA_B_D, state->combiner.Ab1),
		CDisplayListParser::LookupName(CDisplayListParser::ACMuxC, state->combiner.Ac1),
		CDisplayListParser::LookupName(CDisplayListParser::ACMuxA_B_D, state->combiner.Ad1));

	::SetWindowText(GetDlgItem(IDC_EDIT_CC_1C), strCycle1Color.c_str());
	::SetWindowText(GetDlgItem(IDC_EDIT_CC_1A), strCycle1Alpha.c_str());
	::SetWindowText(GetDlgItem(IDC_EDIT_CC_2C), strCycle2Color.c_str());
	::SetWindowText(GetDlgItem(IDC_EDIT_CC_2A), strCycle2Alpha.c_str());

    //stdstr strCombiner = cycle1Color + cycle1Alpha;

    //

	stdstr split = "";

    stdstr strStateSummary = (
		//strTextureImage + split +
		//strColors + split +
		//strGeoMode + split +
		//strOtherModeH + split +
        //strCombiner + split +
		//strNumLights + split +
		strTileDescriptors);
    
    m_StateTextbox.SetWindowTextA(strStateSummary.c_str());

    return FALSE;
}


LRESULT CDebugDisplayList::OnResourceTreeSelChanged(NMHDR* pNMHDR)
{
    NMTREEVIEW* pnmtv = reinterpret_cast<NMTREEVIEW*>(pNMHDR);
    TVITEM tvItem = pnmtv->itemNew;

    HTREEITEM hItem = tvItem.hItem;
    HTREEITEM hParentItem = m_ResourceTreeCtrl.GetParentItem(hItem);

    if (hParentItem == NULL)
    {
        // don't do anything for root items
        return FALSE;
    }

    int resIdx = m_ResourceTreeCtrl.GetItemData(hItem);

    dram_resource_t* res = &m_RamResources[resIdx];
    
    // todo add this to context menu
    //m_DisplayListCtrl.EnsureVisible(res->nCommand, FALSE);
    //m_DisplayListCtrl.SelectItem(res->nCommand);

    // texture preview test

    HWND texWnd = GetDlgItem(IDC_TEX_PREVIEW);
    HDC hDC = ::GetDC(texWnd);

    int width = (res->imageWidth != 0) ? res->imageWidth : 32;
    int height = (res->imageHeight != 0) ? res->imageHeight : 32;
	int numTexels = width * height;

    //MessageBox(stdstr_f("%d", width).c_str(), "", MB_OK);

    uint32_t* imageBuffer = new uint32_t[numTexels];
    uint8_t* imgSrc = m_DisplayListParser.GetRamSnapshot() + (res->virtAddress - 0x80000000);

	bool valid = CDisplayListParser::ConvertImage(imageBuffer, imgSrc, res->imageFmt, res->imageSiz, numTexels);

    //::SetWindowPos(texWnd, m_hWnd, 0, 0, width*4, height*4, SWP_NOMOVE);

    CRect rc;
    ::GetWindowRect(texWnd, &rc);

    int scaledWidth = width * 4;
    int scaledHeight = height * 4;

    while (scaledWidth >= rc.Width() || scaledHeight >= rc.Height())
    {
        scaledWidth /= 2;
        scaledHeight /= 2;
    }

    HBITMAP hBitmap = CreateBitmap(width, height, 1, 32, imageBuffer);
    HDC hTempDC = CreateCompatibleDC(hDC);

    HBRUSH hBrush = CreateSolidBrush(RGB(240, 240, 240));

    RECT rcFill = { 1, 1, rc.Width()-1, rc.Height()-1 };

    SelectObject(hTempDC, hBitmap);

    int xoffs = rc.Width() / 2 - (scaledWidth / 2);
    int yoffs = rc.Height() / 2 - (scaledHeight / 2);

    FillRect(hDC, &rcFill, hBrush);
    StretchBlt(hDC, xoffs, yoffs, scaledWidth, scaledHeight,
        hTempDC, 0, 0, width, height, SRCCOPY);

    // dumb test
    //CImageList imgList = m_TreeImageList;
    //int idx = imgList.Add(hBitmap);
    //pnmtv->itemNew.iImage = idx;
    //pnmtv->itemNew.iSelectedImage = idx;
    //pnmtv->itemNew.mask &= ~(TVIF_IMAGE | TVIF_SELECTEDIMAGE);
    //m_ResourceTreeCtrl.SetItemImage(hItem, idx, idx);

    m_ResourceTreeCtrl.RedrawWindow();

    ::ReleaseDC(texWnd, hDC);
    ::DeleteDC(hTempDC);
    ::DeleteObject(hBitmap);
    ::DeleteObject(hBrush);
    delete[] imageBuffer;

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

	list_entry_colors_t colors = m_ListColors[nItem];

	switch (nSubItem)
	{
	case DisplayListCtrl_Col_VAddr:
	case DisplayListCtrl_Col_SegOffset:
		pLVCD->clrTextBk = RGB(0xEE, 0xEE, 0xEE);
		pLVCD->clrText = RGB(0x44, 0x44, 0x44);
		break;
	case DisplayListCtrl_Col_Command:
		pLVCD->clrTextBk = (colors.bgColor != 0) ? colors.bgColor : RGB(255, 255, 255);
		pLVCD->clrText = (colors.fgColor != 0) ? colors.fgColor : RGB(0, 0, 0);
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