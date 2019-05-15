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
    m_StateTextbox.Attach(GetDlgItem(IDC_EDIT_STATE));

	m_DisplayListCtrl.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);

	m_DisplayListCtrl.AddColumn("Address", DisplayListCtrl_Col_PAddr);
	m_DisplayListCtrl.AddColumn("SegOffset", DisplayListCtrl_Col_SegOffset);
	m_DisplayListCtrl.AddColumn("Raw Command", DisplayListCtrl_Col_RawCommand);
	m_DisplayListCtrl.AddColumn("Command", DisplayListCtrl_Col_Command);
	m_DisplayListCtrl.AddColumn("Parameters", DisplayListCtrl_Col_Parameters);

	m_DisplayListCtrl.SetColumnWidth(DisplayListCtrl_Col_SegOffset, 70);
	m_DisplayListCtrl.SetColumnWidth(DisplayListCtrl_Col_PAddr, 70);
	m_DisplayListCtrl.SetColumnWidth(DisplayListCtrl_Col_RawCommand, 120);
	m_DisplayListCtrl.SetColumnWidth(DisplayListCtrl_Col_Command, 140);
	m_DisplayListCtrl.SetColumnWidth(DisplayListCtrl_Col_Parameters, 250);

	LoadWindowPos();
	WindowCreated();
	return TRUE;
}

LRESULT CDebugDisplayList::OnDestroy(void)
{
	m_DisplayListCtrl.Detach();
    m_StateTextbox.Detach();
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
		SetWindowText("Display List (Waiting for RSP task...)");
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

	if (ucodeVersion != UCODE_UNKNOWN)
	{
		SetWindowText(stdstr_f("Display List - %s", m_DisplayListParser.GetUCodeName()).c_str());
	}
	else
	{
		SetWindowText(stdstr_f("Display List - Unknown microcode (%08X)", m_DisplayListParser.GetUCodeChecksum()).c_str());
	}

	m_DisplayListCtrl.SetRedraw(FALSE);
	m_DisplayListCtrl.DeleteAllItems();
	
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

        char szParams[1024];
        const char* szCommandName = m_DisplayListParser.StepDecode(szParams, NULL);

        stdstr strCommandNameTabbed = std::string(numTabs * 4, ' ') + szCommandName;
        stdstr strParamsTabbed = std::string(numTabs*4, ' ') + szParams;

        m_DisplayListCtrl.AddItem(nCommand, DisplayListCtrl_Col_PAddr, strVirtAddress.c_str());
        m_DisplayListCtrl.AddItem(nCommand, DisplayListCtrl_Col_SegOffset, strAddress.c_str());
        m_DisplayListCtrl.AddItem(nCommand, DisplayListCtrl_Col_RawCommand, strRawCommand.c_str());
        m_DisplayListCtrl.AddItem(nCommand, DisplayListCtrl_Col_Command, strCommandNameTabbed.c_str());
        m_DisplayListCtrl.AddItem(nCommand, DisplayListCtrl_Col_Parameters, strParamsTabbed.c_str());

        nCommand++;
    }

	m_DisplayListCtrl.SetRedraw(TRUE);
	::EnableWindow(GetDlgItem(IDC_BTN_REFRESH), TRUE);
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
        "TextureImage: 0x%08X (0x%08X)\r\n",
        state->textureAddr,
        state->SegmentedToVirtual(state->textureAddr)
    );

	// tiles

    stdstr strTileDescriptors = "";

    for (int i = 0; i < 8; i++)
    {
        hle_tile_descriptor_t* t = &state->tiles[i];
    
        if (t->enabled == 0)
        {
            strTileDescriptors += stdstr_f("Tile %d: (off)\r\n", i);
            continue;
        }

        const char* sizName = CDisplayListParser::LookupName(CDisplayListParser::TexelSizeNames, t->siz);
        const char* fmtName = CDisplayListParser::LookupName(CDisplayListParser::ImageFormatNames, t->fmt);

        strTileDescriptors += stdstr_f(
            "Tile %d:\r\n"
            "tmem: 0x%03X, siz: %s, fmt: %s, line: %d, "
            "shifts: %d, masks: %d, cms: %d, "
            "shiftt: %d, maskt: %d, cmt: %d,\r\n"
            "scaleS: %d, scaleT: %d, "
            "palette: %d, "
            "mipmapLevels: %d, on: %d\r\n",
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

    stdstr strNumLights = stdstr_f("NumLights: %d\r\n", state->numLights);

	/////////////

    stdstr strStateSummary = strGeoMode + strTextureImage + strNumLights + strTileDescriptors;
    
    m_StateTextbox.SetWindowTextA(strStateSummary.c_str());

	// texture preview test

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

    return FALSE;
}