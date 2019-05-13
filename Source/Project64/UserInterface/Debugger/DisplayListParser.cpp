#include <stdafx.h>
#include "DisplayListParser.h"

const ucode_version_info_t* CDisplayListParser::GetUCodeVersionInfo(uint32_t checksum)
{
	for (int i = 0; UCodeVersions[i].name != NULL; i++)
	{
		if (UCodeVersions[i].checksum == checksum)
		{
			return &UCodeVersions[i];
		}
	}

	return NULL;
}

CDisplayListParser::CDisplayListParser(uint32_t ucodeAddr, uint32_t dlistAddr, uint32_t dlistSize) :
	m_UCodeChecksum(0),
	m_UCodeVersion(UCODE_UNKNOWN),
	m_UCodeName(NULL),
	m_CommandTable(NULL),
	m_RootDListSize(dlistSize),
	m_VertexBufferSize(16)
{
	memset(&m_State, 0, sizeof(m_State));

	uint8_t* ucode = g_MMU->Rdram() + ucodeAddr;

	for (int i = 0; i < 3072; i += sizeof(uint32_t))
	{
		m_UCodeChecksum += *(uint32_t*)&ucode[i];
	}

	const ucode_version_info_t* info = GetUCodeVersionInfo(m_UCodeChecksum);

	if (info == NULL)
	{
		m_UCodeVersion = UCODE_UNKNOWN;
		m_UCodeName = "unknown microcode";
		return;
	}

	m_UCodeVersion = info->version;
	m_UCodeName = info->name;
	m_CommandTable = info->commandTable;

	m_State.address = dlistAddr;

	while(!m_State.bDone)
	{
		Step();
	}
}

ucode_version_t CDisplayListParser::GetUCodeVersion(void)
{
	return m_UCodeVersion;
}

const char* CDisplayListParser::GetUCodeName(void)
{
	return m_UCodeName;
}

uint32_t CDisplayListParser::GetUCodeChecksum(void)
{
	return m_UCodeChecksum;
}

int CDisplayListParser::GetCommandCount(void)
{
	return m_StateLog.size();
}

hl_state_t *CDisplayListParser::GetLogState(size_t index)
{
	if (index < GetCommandCount())
	{
		return &m_StateLog[index];
	}
}

const char* CDisplayListParser::DecodeCommand(size_t index, char* paramsBuf)
{
	hl_state_t* state = GetLogState(index);

	sprintf(paramsBuf, "?");

	if (state == NULL)
	{
		return "?";
	}

	uint8_t commandByte = state->command.w0 >> 24;

	const dl_cmd_info_t *commandInfo;

	commandInfo = LookupCommand(Commands_Global, commandByte);

	if (commandInfo == NULL && m_CommandTable != NULL)
	{
		commandInfo = LookupCommand(m_CommandTable, commandByte);
	}

	if (commandInfo != NULL)
	{
		if (commandInfo->decodeFunc != NULL)
		{
			const char* overrideName = commandInfo->decodeFunc(state, paramsBuf);

			if (overrideName != NULL)
			{
				return overrideName;
			}
		}
		
		return commandInfo->commandName;
	}

	return "?";
}

uint32_t CDisplayListParser::SegmentedToPhysical(hl_state_t* state, uint32_t address)
{
	uint32_t segment = (address >> 24) & 0x0F;
	uint32_t offset = address & 0x00FFFFFF;
	return state->segments[segment] + offset;
}

uint32_t CDisplayListParser::SegmentedToVirtual(hl_state_t* state, uint32_t address)
{
    return SegmentedToPhysical(state, address) | 0x80000000;
}

void CDisplayListParser::Step(void)
{
	uint32_t physAddress = SegmentedToPhysical(&m_State, m_State.address);

	g_MMU->LW_PAddr(physAddress, m_State.command.w0);
	g_MMU->LW_PAddr(physAddress + 4, m_State.command.w1);

	m_StateLog.push_back(m_State);

	m_State.address += 8;

	uint8_t commandByte = m_State.command.w0 >> 24;

	const dl_cmd_info_t *commandInfo;

	commandInfo = LookupCommand(Commands_Global, commandByte);

	if (commandInfo == NULL && m_CommandTable != NULL)
	{
		commandInfo = LookupCommand(m_CommandTable, commandByte);
	}

	if (commandInfo != NULL && commandInfo->opFunc != NULL)
	{
		commandInfo->opFunc(&m_State);
	}
}

dl_cmd_info_t* CDisplayListParser::LookupCommand(dl_cmd_info_t* commands, uint8_t cmdByte)
{
	for (int i = 0; commands[i].commandName != NULL; i++)
	{
		if (commands[i].commandByte == cmdByte)
		{
			return &commands[i];
		}
	}

	return NULL;
}

ucode_version_info_t CDisplayListParser::UCodeVersions[] = {
	{ 0x3A1CBAC3, UCODE_F3D, "Fast3D", Commands_F3D },
	{ 0x8805FFEA, UCODE_F3DEX, "F3DEX", Commands_F3DEX },
	{ 0xBC45382E, UCODE_F3DEX2, "F3DEX2", Commands_F3DEX2 },
	{ 0, UCODE_UNKNOWN, NULL, NULL }
};

// ucode-independent rdp commands
dl_cmd_info_t CDisplayListParser::Commands_Global[] = {
	{ 0xC0, "gsDPNoop",              NULL, dec_NoParams },
	{ 0xC8, "gsDPTriFill",           NULL, NULL },
	{ 0xC9, "gsDPTriFillZ",          NULL, NULL },
	{ 0xCA, "gsDPTriTxtr",           NULL, NULL },
	{ 0xCB, "gsDPTriTxtrZ",          NULL, NULL },
	{ 0xCC, "gsDPTriShade",          NULL, NULL },
	{ 0xCD, "gsDPTriShadeZ",         NULL, NULL },
	{ 0xCE, "gsDPTriShadeTxtr",      NULL, NULL },
	{ 0xCF, "gsDPTriShadeTxtrZ",     NULL, NULL },
	{ 0xE4, "gsDPTextureRectangle",  NULL, NULL },
	{ 0xE5, "gsDPTextureRectangle2", NULL, NULL },
	{ 0xE6, "gsDPLoadSync",          NULL, dec_NoParams },
	{ 0xE7, "gsDPPipeSync",          NULL, dec_NoParams },
	{ 0xE8, "gsDPTileSync",          NULL, dec_NoParams },
	{ 0xE9, "gsDPFullSync",          NULL, dec_NoParams },
	{ 0xEA, "gsDPSetKeyGB",          NULL, NULL },
	{ 0xEB, "gsDPSetKeyR",           NULL, NULL },
	{ 0xEC, "gsDPSetConvert",        NULL, NULL },
	{ 0xED, "gsDPSetScissor",        NULL, dec_gsDPSetScissor },
	{ 0xEE, "gsDPSetPrimDepth",      NULL, NULL },
	{ 0xEF, "gsDPSetOtherMode",      NULL, NULL },
	{ 0xF0, "gsDPLoadTLUT",          NULL, NULL },
	{ 0xF2, "gsDPSetTileSize",       NULL, dec_gsDPSetTileSize },
	{ 0xF3, "gsDPLoadBlock",         NULL, dec_gsDPLoadBlock },
	{ 0xF4, "gsDPLoadTile",          NULL, NULL },
	{ 0xF5, "gsDPSetTile",           op_gsDPSetTile, dec_gsDPSetTile },
	{ 0xF6, "gsDPFillRectangle",     NULL, dec_gsDPFillRectangle },
	{ 0xF7, "gsDPSetFillColor",      NULL, dec_HexParam32 },
	{ 0xF8, "gsDPSetFogColor",       NULL, dec_HexParam32 },
	{ 0xF9, "gsDPSetBlendColor",     NULL, dec_HexParam32 },
	{ 0xFA, "gsDPSetPrimColor",      NULL, dec_HexParam32 },
	{ 0xFB, "gsDPSetEnvColor",       NULL, dec_HexParam32 },
	{ 0xFC, "gsDPSetCombine",        NULL, NULL },
	{ 0xFD, "gsDPSetTextureImage",   NULL, dec_gsDPSetTextureImage },
	{ 0xFE, "gsDPSetDepthImage",     NULL, NULL },
	{ 0xFF, "gsDPSetColorImage",     NULL, NULL },
	{ 0, NULL, NULL, NULL }
};

dl_cmd_info_t CDisplayListParser::Commands_F3D[] = {
	{ 0x00, "gsSPNoop",              NULL, dec_NoParams },
	{ 0x01, "gsSPMatrix",            NULL, dec_gsSPMatrix_f3d },
	//{ 0x02, "gsSPNoop", NULL, NULL }, ?
	{ 0x03, "gsSPMoveMem",           NULL, dec_gsSPMoveMem_f3d },
	{ 0x04, "gsSPVertex",            NULL, dec_gsSPVertex_f3d },
	//{ 0x05, "gsSPNoop", NULL, NULL },
	{ 0x06, "gsSPDisplayList",       op_gsSPDisplayList, dec_gsSPDisplayList },
	//{ 0x07, "gsSPNoop", NULL, NULL },
	//{ 0x08, "gsSPNoop", NULL, NULL },
	//{ 0x09, "gsSPSprite2D", NULL, NULL },
	// { 0xB1, "gsSPTri4", NULL, NULL },
	{ 0xB2, "rdphalf_cont",         NULL, NULL },
	{ 0xB3, "rdphalf_2",            NULL, NULL },
	{ 0xB4, "rdphalf_1",            NULL, NULL },
	// { 0xB5, "line3d", NULL, NULL },
	{ 0xB6, "gsSPClearGeometryMode", NULL, dec_gsSPSetGeometryMode_f3d },
	{ 0xB7, "gsSPSetGeometryMode",   NULL, dec_gsSPSetGeometryMode_f3d },
	{ 0xB8, "gsSPEndDisplayList",    op_gsSPEndDisplayList, dec_NoParams },
	{ 0xB9, "gsSPSetOtherModeLow",   NULL, NULL },
	{ 0xBA, "gsSPSetOtherModeHigh",  NULL, NULL },
	{ 0xBB, "gsSPTexture",           NULL, dec_gsSPTexture_f3d },
	{ 0xBC, "gsSPMoveWord",          op_gsSPMoveWord_f3d, dec_gsSPMoveWord_f3d },
	{ 0xBD, "gsSPPopMatrix",         NULL, NULL },
	{ 0xBE, "gsSPCullDisplayList",   NULL, NULL },
	{ 0xBF, "gsSP1Triangle",         NULL, dec_gsSP1Triangle_f3d },
	{ 0, NULL, NULL, NULL }
};

dl_cmd_info_t CDisplayListParser::Commands_F3DEX[] = {
	{ 0x06, "gsSPDisplayList",       op_gsSPDisplayList, dec_gsSPDisplayList },
	{ 0xB8, "gsSPEndDisplayList",    op_gsSPEndDisplayList, dec_NoParams },
	{ 0xBC, "gsSPMoveWord",          op_gsSPMoveWord_f3d, dec_gsSPMoveWord_f3d },
	{ 0, NULL, NULL, NULL }
};

dl_cmd_info_t CDisplayListParser::Commands_F3DEX2[] = {
	{ 0xDB, "gsSPMoveWord",          op_gsSPMoveWord_f3dex2, dec_gsSPMoveWord_f3dex2 },
	{ 0xDE, "gsSPDisplayList",       op_gsSPDisplayList, dec_gsSPDisplayList },
	{ 0xDF, "gsSPEndDisplayList",    op_gsSPEndDisplayList, dec_NoParams },
	{ 0, NULL, NULL, NULL }
};

geo_mode_bitname_t CDisplayListParser::GeometryModeNames[] = {
	{ 0x00000001, "G_ZBUFFER" },
	{ 0x00000004, "G_SHADE" },
	{ 0x00001000, "G_CULL_FRONT" },
	{ 0x00002000, "G_CULL_BACK" },
	{ 0x00010000, "G_FOG" },
	{ 0x00020000, "G_LIGHTING" },
	{ 0x00040000, "G_TEXTURE_GEN" },
	{ 0x00080000, "G_TEXTURE_GEN_LINEAR" },
	{ 0x00000200, "G_SHADING_SMOOTH" },
	{ 0x00800000, "G_CLIPPING" },
	{ 0, NULL }
};

geo_mode_bitname_t CDisplayListParser::GeometryModeNames_F3DEX2[] = {
	{ 0x00000001, "G_ZBUFFER" },
	{ 0x00000004, "G_SHADE" },
	{ 0x00000200, "G_CULL_FRONT" },
	{ 0x00000400, "G_CULL_BACK" },
	{ 0x00010000, "G_FOG" },
	{ 0x00020000, "G_LIGHTING" },
	{ 0x00040000, "G_TEXTURE_GEN" },
	{ 0x00080000, "G_TEXTURE_GEN_LINEAR" },
	{ 0x00200000, "G_SHADING_SMOOTH" },
	{ 0x00800000, "G_CLIPPING" },
	{ 0, NULL }
};

void CDisplayListParser::op_gsSPDisplayList(hl_state_t* state)
{
	dl_cmd_dl_t* cmd = &state->command.dl;

	if (cmd->branch == 0)
	{
		state->stack[state->stackIndex++] = state->address;
	}
	state->address = cmd->address;
}

void CDisplayListParser::op_gsSPEndDisplayList(hl_state_t* state)
{
	if (state->stackIndex > 0)
	{
		state->address = state->stack[--state->stackIndex];
	}
	else
	{
		state->bDone = true;
	}
}

void CDisplayListParser::op_gsSPMoveWord_f3d(hl_state_t* state)
{
	dl_cmd_moveword_f3d_t* cmd = &state->command.moveword_f3d;

	if (cmd->index == 0x06) // MW_SEGMENT
	{
		int segno = cmd->offset / 4;
		uint32_t physAddress = cmd->data;
		state->segments[segno] = physAddress;
	}
}

void CDisplayListParser::op_gsSPMoveWord_f3dex2(hl_state_t* state)
{
	dl_cmd_moveword_f3dex2_t* cmd = &state->command.moveword_f3dex2;

	//MessageBox(NULL, "moveword f3dex2", "", MB_OK);

	if (cmd->index == 0x06) // MW_SEGMENT
	{
		int segno = cmd->offset / 4;
		uint32_t physAddress = cmd->data;
		state->segments[segno] = physAddress;
	}
}

void CDisplayListParser::op_gsDPSetTile(hl_state_t* state)
{
    dl_cmd_settile_t* cmd = &state->command.settile;
    hl_tile_descriptor_t *td = &state->tiles[cmd->tile];

    td->tmem = cmd->tmem;
    td->line = cmd->line;
    td->siz = cmd->siz;
    td->fmt = cmd->fmt;
    td->palette = cmd->palette;
    td->cmt = cmd->cmt;
    td->maskt = cmd->maskt;
    td->shiftt = cmd->shiftt;
    td->cms = cmd->cms;
    td->masks = cmd->masks;
    td->shifts = cmd->shifts;
}

/////////////

const char* CDisplayListParser::dec_NoParams(hl_state_t*, char* paramsBuf)
{
	sprintf(paramsBuf, "");
	return NULL;
}

const char* CDisplayListParser::dec_gsSPMoveWord_f3d(hl_state_t* state, char* paramsBuf)
{
	dl_cmd_moveword_f3d_t* cmd = &state->command.moveword_f3d;

	if (cmd->index == 0x06) // MW_SEGMENT
	{
		sprintf(paramsBuf, "0x%02X, 0x%08X", cmd->offset/4, (uint32_t)cmd->data);
		return "gsSPSegment";
	}

	return NULL;
}

const char* CDisplayListParser::dec_gsSPMoveMem_f3d(hl_state_t* state, char* paramsBuf)
{
    // gsDma2p G_MOVEMEM adrs len idx ofs 

    dl_cmd_movemem_f3d_t* cmd = &state->command.movemem_f3d;

    if (cmd->p == 0x80) // G_MV_VIEWPORT
    {
        // todo params
        sprintf(paramsBuf, "0x%08X // 0x%08X", cmd->address,
            SegmentedToVirtual(state, cmd->address));
        return "gsSPViewport";
    }

    if (cmd->p >= 0x86 && cmd->p <= 0x94) // G_MV_L0:7
    {
        int lightNumber = (cmd->p - 0x86) / 2;
        sprintf(paramsBuf, "0x%08X, %d // 0x%08X", cmd->address, lightNumber,
            SegmentedToVirtual(state, cmd->address));
        return "gsSPLight";
    }

    return NULL;
}

const char* CDisplayListParser::dec_gsSPMoveWord_f3dex2(hl_state_t* state, char* paramsBuf)
{
	dl_cmd_moveword_f3dex2_t* cmd = &state->command.moveword_f3dex2;

	if (cmd->index == 0x06) // MW_SEGMENT
	{
		sprintf(paramsBuf, "0x%02X, 0x%08X", cmd->offset / 4, (uint32_t)cmd->data);
		return "gsSPSegment";
	}

	return NULL;
}

const char* CDisplayListParser::dec_gsSP1Triangle_f3d(hl_state_t* state, char* paramsBuf)
{
	dl_cmd_tri1_f3d_t *cmd = &state->command.tri1_f3d;
	sprintf(paramsBuf, "%d, %d, %d", cmd->v0 / 10, cmd->v1 / 10, cmd->v2 / 10);
	return NULL;
}

const char* CDisplayListParser::dec_gsSPVertex_f3d(hl_state_t* state, char* paramsBuf)
{
	dl_cmd_vtx_f3d_t *cmd = &state->command.vtx_f3d;
	sprintf(paramsBuf, "0x%08X, %d, %d // 0x%08X", cmd->address, cmd->num+1, cmd->idx,
        SegmentedToVirtual(state, cmd->address));
	return NULL;
}

const char* CDisplayListParser::dec_gsSPSetGeometryMode_f3d(hl_state_t* state, char* paramsBuf)
{
	bool havePrev = false;

	for (int i = 0; GeometryModeNames[i].name != NULL; i++)
	{
		if (state->command.w1 & GeometryModeNames[i].mask)
		{
			if (havePrev)
			{
				paramsBuf += sprintf(paramsBuf, " | ");
			}
			
			paramsBuf += sprintf(paramsBuf, GeometryModeNames[i].name);
			havePrev = true;
		}
	}
	return NULL;
}

const char* CDisplayListParser::dec_gsSPDisplayList(hl_state_t* state, char* paramsBuf)
{
	dl_cmd_dl_t *cmd = &state->command.dl;
	sprintf(paramsBuf, "0x%08X // 0x%08X", cmd->address,
        SegmentedToVirtual(state, cmd->address));

	if (cmd->branch)
	{
		return "gsSPBranchList";
	}

	return NULL;
}

const char* CDisplayListParser::dec_HexParam32(hl_state_t* state, char* paramsBuf)
{
	sprintf(paramsBuf, "0x%08X", state->command.w1);
	return NULL;
}

const char* CDisplayListParser::dec_gsSPMatrix_f3d(hl_state_t* state, char* paramsBuf)
{
	dl_cmd_mtx_f3d_t* cmd = &state->command.mtx_f3d;

	sprintf(paramsBuf, "0x%08X, (%s | %s | %s) // 0x%08X", cmd->address,
		(cmd->params & 1) ? "G_MTX_PROJECTION" : "G_MTX_MODELVIEW",
		(cmd->params & 2) ? "G_MTX_LOAD" : "G_MTX_MUL",
		(cmd->params & 4) ? "G_MTX_PUSH" : "G_MTX_NOPUSH",
        SegmentedToVirtual(state, cmd->address));
	
	return NULL;
}

const char* CDisplayListParser::dec_gsDPFillRectangle(hl_state_t* state, char* paramsBuf)
{
	dl_cmd_fillrect_t* cmd = &state->command.fillrect;
	sprintf(paramsBuf, "%d, %d, %d, %d", cmd->ulx >> 2, cmd->uly >> 2, cmd->lrx >> 2, cmd->lry >> 2);
	return NULL;
}

const char* CDisplayListParser::dec_gsDPSetScissor(hl_state_t* state, char* paramsBuf)
{
	dl_cmd_setscissor_t* cmd = &state->command.setscissor;

	const char* szMode = "?";

	switch (cmd->mode)
	{
	case 0: szMode = "G_SC_NON_INTERLACE"; break;
	case 2: szMode = "G_SC_EVEN_INTERLACE"; break;
	case 3: szMode = "G_SC_ODD_INTERLACE"; break;
	}

	// TODO check for frac bits and use gsDPSetScissorFrac if set

	sprintf(paramsBuf, "%s, %d, %d, %d, %d", szMode, cmd->ulx >> 2, cmd->uly >> 2, cmd->lrx >> 2, cmd->lry >> 2);
	return NULL;
}

const char* CDisplayListParser::dec_gsDPSetTextureImage(hl_state_t* state, char* paramsBuf)
{
    dl_cmd_settimg_t* cmd = &state->command.settimg;

    const char* fmtName = "?";
    const char* sizName = "?";

    switch (cmd->fmt)
    {
    case 0: fmtName = "G_IM_FMT_RGBA"; break;
    case 1: fmtName = "G_IM_FMT_YUV"; break;
    case 2: fmtName = "G_IM_FMT_CI"; break;
    case 3: fmtName = "G_IM_FMT_IA"; break;
    case 4: fmtName = "G_IM_FMT_I"; break;
    }

    switch (cmd->siz)
    {
    case 0: sizName = "G_IM_SIZ_4b"; break;
    case 1: sizName = "G_IM_SIZ_8b"; break;
    case 2: sizName = "G_IM_SIZ_16b"; break;
    case 3: sizName = "G_IM_SIZ_32b"; break;
    }

    sprintf(paramsBuf, "%s, %s, %d, 0x%08X // 0x%08X", fmtName, sizName, cmd->width+1, cmd->address,
        SegmentedToVirtual(state, cmd->address));

    return NULL;
}

const char* CDisplayListParser::dec_gsDPSetTile(hl_state_t* state, char* paramsBuf)
{
    dl_cmd_settile_t* cmd = &state->command.settile;

    const char* fmtName = "?";
    const char* sizName = "?";

    switch (cmd->fmt)
    {
    case 0: fmtName = "G_IM_FMT_RGBA"; break;
    case 1: fmtName = "G_IM_FMT_YUV"; break;
    case 2: fmtName = "G_IM_FMT_CI"; break;
    case 3: fmtName = "G_IM_FMT_IA"; break;
    case 4: fmtName = "G_IM_FMT_I"; break;
    }

    switch (cmd->siz)
    {
    case 0: sizName = "G_IM_SIZ_4b"; break;
    case 1: sizName = "G_IM_SIZ_8b"; break;
    case 2: sizName = "G_IM_SIZ_16b"; break;
    case 3: sizName = "G_IM_SIZ_32b"; break;
    }

    sprintf(paramsBuf, "%s, %s, %d, 0x%03X, %d, %d, %d, %d, %d, %d, %d, %d", fmtName, sizName,
        cmd->line, cmd->tmem, cmd->tile, cmd->palette,
        cmd->cmt, cmd->maskt, cmd->shiftt,
        cmd->cms, cmd->masks, cmd->shifts);

    return NULL;
}

const char* CDisplayListParser::dec_gsDPLoadBlock(hl_state_t* state, char* paramsBuf)
{
    // counter inced every 64 bits of image
    // (4 texels for rgba16)
    // 8 words = 32 width

    /*
    var width = 64;
    var bytesPerTexel = 2; // rbga16
    var wordsPerRow = ((width * bytesPerTexel) / 8) | 0;
    var dxt = (((1 << 11) + wordsPerRow - 1) / wordsPerRow) | 0;
    */

    dl_cmd_loadblock_t* cmd = &state->command.loadblock;

    uint8_t siz = state->tiles[cmd->tile].siz;

    int bytesPerTexel = 0;

    switch (siz)
    {
    case 0: bytesPerTexel = 0; break; // G_IM_SIZ_4b
    case 1: bytesPerTexel = 1; break; // G_IM_SIZ_8b
    case 2: bytesPerTexel = 2; break; // G_IM_SIZ_16b
    case 3: bytesPerTexel = 4; break; // G_IM_SIZ_32b
    }

    int bytesPerLine = (0x800 / cmd->dxt) / sizeof(uint64_t);
    int width = bytesPerLine / bytesPerTexel;

    // 16

    // 0x100 0x100 0x100 0x100 0x100 0x100 0x100 0x100
    // 8      8    8     8     8     8     8     8 = 64bytes / 2Bpp = 32 width

    // dxt = amount to increment internal counter by for every 8 bytes of image data
    // when counter reaches 0x800, roll over and start next line

    // 0x800 / dxt = words per line
    // words per line * 8 = bytes per line
    // bytes per line / bytes per texel = image width


    sprintf(paramsBuf, "tile:%d, uls:%d, ult:%d, lrs:%d, dxt:%d // %dx%d",
        cmd->tile, cmd->uls, cmd->ult, cmd->lrs, cmd->dxt,
        width, cmd->lrs);
    return NULL;
}

const char* CDisplayListParser::dec_gsDPSetTileSize(hl_state_t* state, char* paramsBuf)
{
    dl_cmd_settilesize_t* cmd = &state->command.settilesize;
    sprintf(paramsBuf, "tile:%d, uls:%d, ult:%d, lrs:%d, lrt:%d", cmd->tile, cmd->uls, cmd->ult, cmd->lrs, cmd->lrt);
    return NULL;
}

const char* CDisplayListParser::dec_gsSPTexture_f3d(hl_state_t* state, char* paramsBuf)
{
    dl_cmd_texture_f3d_t* cmd = &state->command.texture_f3d;
    sprintf(paramsBuf, "s:%d, t:%d, levels:%d, tile:%d, on:%d", cmd->s, cmd->t, cmd->level, cmd->tile, cmd->on);
    return NULL;
}