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

LRESULT CDebugDisplayList::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	DlgResize_Init(false, true);
	DlgToolTip_Init();
	DlgSavePos_Init(DebuggerUI_DisplayListPos);

	m_DisplayListCtrl.Attach(GetDlgItem(IDC_LST_DLIST));
	m_ResourceListCtrl.Attach(GetDlgItem(IDC_LST_RESOURCES));
    m_StateTextbox.Attach(GetDlgItem(IDC_EDIT_STATE));
	m_StatusText.Attach(GetDlgItem(IDC_STATUS_TEXT));

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

	m_ResourceListCtrl.ModifyStyle(LVS_OWNERDRAWFIXED, 0, 0);
	m_ResourceListCtrl.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);

	m_ResourceListCtrl.AddColumn("Address", 0);
	m_ResourceListCtrl.AddColumn("SegOffset", 1);
	m_ResourceListCtrl.AddColumn("Resource", 2);

	m_ResourceListCtrl.SetColumnWidth(0, 65);
	m_ResourceListCtrl.SetColumnWidth(1, 65);
	m_ResourceListCtrl.SetColumnWidth(2, 95);

	LoadWindowPos();
	WindowCreated();
	return TRUE;
}

LRESULT CDebugDisplayList::OnDestroy(void)
{
	m_DisplayListCtrl.Detach();
	m_ResourceListCtrl.Detach();
    m_StateTextbox.Detach();
	m_StatusText.Detach();
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

        nCommand++;
    }

	m_DisplayListCtrl.SetRedraw(TRUE);

	m_ResourceListCtrl.SetRedraw(FALSE);
	m_ResourceListCtrl.DeleteAllItems();

	for (size_t i = 0; i < m_RamResources.size(); i++)
	{
		dram_resource_t* res = &m_RamResources[i];
		stdstr strVirtAddress = stdstr_f("%08X", res->virtAddress);
		stdstr strAddress = stdstr_f("%08X", res->address);
		stdstr strType;

		switch (res->type)
		{
		case RES_ROOT_DL: strType = "Display list (root)"; break;
		case RES_DL: strType = "Display list"; break;
		case RES_SEGMENT: strType = "DRAM segment"; break;
		case RES_COLORBUFFER: strType = "Color buffer"; break;
		case RES_DEPTHBUFFER: strType = "Depth buffer"; break;
		case RES_TEXTURE: strType = "Texture"; break;
		case RES_PALETTE: strType = "Palette"; break;
		case RES_VERTICES: strType = "Vertices"; break;
		case RES_PROJECTION_MATRIX: strType = "Projection matrix"; break;
		case RES_MODELVIEW_MATRIX: strType = "Modelview matrix"; break;
		case RES_VIEWPORT: strType = "Viewport"; break;
		case RES_DIFFUSE_LIGHT: strType = "Diffuse light"; break;
		case RES_AMBIENT_LIGHT: strType = "Ambient light"; break;
		default: strType = "?"; break;
		}

		m_ResourceListCtrl.AddItem(i, 0, strVirtAddress.c_str());
		m_ResourceListCtrl.AddItem(i, 1, strAddress.c_str());
		m_ResourceListCtrl.AddItem(i, 2, strType.c_str());
	}

	m_ResourceListCtrl.SetRedraw(TRUE);

	::EnableWindow(GetDlgItem(IDC_BTN_REFRESH), TRUE);
	
	strStatus += stdstr_f(" - %d commands", nCommand);
	m_StatusText.SetWindowTextA(strStatus.c_str());

	m_bRefreshPending = false;
}

LRESULT CDebugDisplayList::OnListItemChanged(NMHDR* pNMHDR)
{
    NMITEMACTIVATE* pIA = reinterpret_cast<NMITEMACTIVATE*>(pNMHDR);
    int nItem = pIA->iItem;

    CHleDmemState* state = m_DisplayListParser.GetLoggedState(nItem);

	// Geometry mode

    stdstr strGeoMode = "GeometryMode: ";

    name_lut_entry_t *geoModeNames = CDisplayListParser::GeometryModeNames;

    for (int i = 0; geoModeNames[i].name != NULL; i++)
    {
        if (state->geometryMode & geoModeNames[i].value)
        {
            strGeoMode += geoModeNames[i].name;
            strGeoMode += " ";
        }
    }

    strGeoMode += "\r\n";

	// Texture address

    stdstr strTextureImage = stdstr_f(
        "TextureImage: 0x%08X (0x%08X)\r\n"
		"DepthImage: 0x%08X (0x%08X)\r\n"
		"ColorImage: 0x%08X (0x%08X)\r\n",
		state->textureImage, state->SegmentedToVirtual(state->textureImage),
		state->depthImage, state->SegmentedToVirtual(state->depthImage),
		state->colorImage, state->SegmentedToVirtual(state->colorImage)
    );

	// rdp colors

	stdstr strColors = stdstr_f(
		"Colors: Fill: 0x%08X, "
		"Fog: 0x%08X, "
		"Blend: 0x%08X, "
		"Prim: 0x%08X, "
		"Env: 0x%08X\r\n",
		state->fillColor,
		state->fogColor,
		state->blendColor,
		state->primColor,
		state->envColor
	);

	// tiles

    stdstr strTileDescriptors = "";

	const char* tileNames[] = { "0 (G_TX_RENDERTILE)", "1", "2", "3", "4", "5", "6", "7 (G_TX_LOADTILE)" };

    for (int i = 0; i <= 7; i++)
    {
        hle_tile_descriptor_t* t = &state->tiles[i];
    
		if (i != 0 && i != 7)
		{
			continue; // TODO checkbox to enable uncommon tiles
		}

        //if (t->enabled == 0)
        //{
        //    strTileDescriptors += stdstr_f("Tile %d: (off)\r\n", i);
        //    continue;
        //}

        const char* sizName = CDisplayListParser::LookupName(CDisplayListParser::TexelSizeNames, t->siz);
        const char* fmtName = CDisplayListParser::LookupName(CDisplayListParser::ImageFormatNames, t->fmt);

        strTileDescriptors += stdstr_f(
            "Tile %s: "
            "tmem: 0x%03X, siz: %s, fmt: %s, line: %d, "
            "shifts: %d, masks: %d, cms: %d,\r\n"
            "shiftt: %d, maskt: %d, cmt: %d, "
            "scaleS: 0x%04X, scaleT: 0x%04X, "
            "palette: %d, "
            "mipmapLevels: %d, on: %d\r\n",
            tileNames[i],
            t->tmem, sizName, fmtName, t->line,
            t->shifts, t->masks, t->cms,
            t->shiftt, t->maskt, t->cmt,
            t->scaleS, t->scaleT,
            t->palette,
            t->mipmapLevels, t->enabled
        );
    }

	// lights

    stdstr strNumLights = stdstr_f("NumLights: %d\r\n", state->numLights);

	// othermode_h

	othermode_h_t* omh = &state->othermode_h;

	stdstr strOtherModeH = stdstr_f("OtherMode_H: %s, %s, %s, %s, %s,\r\n    %s, %s, %s, %s, %s, %s, %s\r\n",
		OtherModeNames::pm[omh->pm],
		OtherModeNames::cd[omh->cd],
		OtherModeNames::cyc[omh->cyc],
		OtherModeNames::tp[omh->tp],
		OtherModeNames::td[omh->td],
		OtherModeNames::tl[omh->tl],
		OtherModeNames::tt[omh->tt],
		OtherModeNames::tf[omh->tf],
		OtherModeNames::tc[omh->tc],
		OtherModeNames::ck[omh->ck],
		OtherModeNames::rd[omh->rd],
		OtherModeNames::ad[omh->ad]);

	/////////////

	stdstr split = "-----------------\r\n";

    stdstr strStateSummary = (
		strTextureImage + split +
		strColors + split +
		strGeoMode + split +
		strOtherModeH + split +
		strNumLights + split +
		strTileDescriptors);
    
    m_StateTextbox.SetWindowTextA(strStateSummary.c_str());

	// texture preview test

	/*
	HWND texWnd = GetDlgItem(IDC_TEX_PREVIEW);
	HDC hDC = ::GetDC(texWnd);

	int texelSize = state->lastBlockLoadTexelSize; // TEST
	int calcWidth = (state->tiles[0].line * sizeof(uint64_t)) / 2; // ; // TEST

	MessageBox(stdstr_f("lastBlockLoadTexelSize:%d, lastBlockLoad %d", state->lastBlockLoadTexelSize, state->lastBlockLoadSize).c_str());

	if (calcWidth == 0)
	{
		calcWidth = 32;
	}

	int width = calcWidth;
	int height = 32;

	uint32_t* imageBuffer = new uint32_t[width * height];
	uint8_t* imgSrc = m_DisplayListParser.GetRamSnapshot() + state->SegmentedToPhysical(state->textureAddr);

	for (int i = 0; i < 32 * 32; i++)
	{
		uint16_t px = *(uint16_t*) &imgSrc[(i*2)^2];
		uint8_t r = ((px >> 11) & 0x1F) * (255.0f / 32.0f);
		uint8_t g = ((px >> 6) & 0x1F) * (255.0f / 32.0f);
		uint8_t b = ((px >> 1) & 0x1F) * (255.0f / 32.0f);
		uint8_t a = (px & 1) * 255;
		imageBuffer[i] = a << 24 | (r << 16) | (g << 8) | (b << 0);
	}

	//::SetWindowPos(texWnd, m_hWnd, 0, 0, width*4, height*4, SWP_NOMOVE);

	HBITMAP hBitmap = CreateBitmap(width, height, 1, 32, imageBuffer);
	HDC hTempDC = CreateCompatibleDC(hDC);
	SelectObject(hTempDC, hBitmap);
	StretchBlt(hDC, 0, 0, width * 4, height * 4, hTempDC, 0, 0, width, height, SRCCOPY);
	
	::ReleaseDC(texWnd, hDC);
	::DeleteDC(hTempDC);
	::DeleteObject(hBitmap);
	delete[] imageBuffer;
	*/

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