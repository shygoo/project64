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
	{ 0xC0, "gDPNoop",            NULL, dec_NoParams },
	{ 0xC8, "gDPTriFill",         NULL, NULL },
	{ 0xC9, "gDPTriFillZ",        NULL, NULL },
	{ 0xCA, "gDPTriTxtr",         NULL, NULL },
	{ 0xCB, "gDPTriTxtrZ",        NULL, NULL },
	{ 0xCC, "gDPTriShade",        NULL, NULL },
	{ 0xCD, "gDPTriShadeZ",       NULL, NULL },
	{ 0xCE, "gDPTriShadeTxtr",    NULL, NULL },
	{ 0xCF, "gDPTriShadeTxtrZ",   NULL, NULL },
	{ 0xE4, "gDPTexRect",         NULL, NULL },
	{ 0xE5, "gDPTexRect2",        NULL, NULL },
	{ 0xE6, "gDPLoadSync",        NULL, dec_NoParams },
	{ 0xE7, "gDPPipeSync",        NULL, dec_NoParams },
	{ 0xE8, "gDPTileSync",        NULL, dec_NoParams },
	{ 0xE9, "gDPFullSync",        NULL, dec_NoParams },
	{ 0xEA, "gDPSetKeyGB",        NULL, NULL },
	{ 0xEB, "gDPSetKeyR",         NULL, NULL },
	{ 0xEC, "gDPSetConvert",      NULL, NULL },
	{ 0xED, "gDPSetScissor",      NULL, dec_gDPSetScissor },
	{ 0xEE, "gDPSetPrimDepth",    NULL, NULL },
	{ 0xEF, "gDPSetOtherMode",    NULL, NULL },
	{ 0xF0, "gDPLoadTLUT",        NULL, NULL },
	{ 0xF2, "gDPSetTileSize",     NULL, NULL },
	{ 0xF3, "gDPLoadBlock",       NULL, NULL },
	{ 0xF4, "gDPLoadTile",        NULL, NULL },
	{ 0xF5, "gDPSetTile",         NULL, NULL },
	{ 0xF6, "gDPFillRectangle",   NULL, dec_gDPFillRectangle },
	{ 0xF7, "gDPSetFillColor",    NULL, dec_HexParam32 },
	{ 0xF8, "gDPSetFogColor",     NULL, dec_HexParam32 },
	{ 0xF9, "gDPSetBlendColor",   NULL, dec_HexParam32 },
	{ 0xFA, "gDPSetPrimColor",    NULL, dec_HexParam32 },
	{ 0xFB, "gDPSetEnvColor",     NULL, dec_HexParam32 },
	{ 0xFC, "gDPSetCombine",      NULL, NULL },
	{ 0xFD, "gDPSetTextureImage", NULL, NULL },
	{ 0xFE, "gDPSetDepthImage",   NULL, NULL },
	{ 0xFF, "gDPSetColorImage",   NULL, NULL },
	{ 0, NULL, NULL, NULL }
};

dl_cmd_info_t CDisplayListParser::Commands_F3D[] = {
	{ 0x00, "gSPNoop",              NULL, dec_NoParams },
	{ 0x01, "gSPMatrix",            NULL, dec_gSPMatrix_f3d },
	//{ 0x02, "gSPNoop", NULL, NULL }, ?
	{ 0x03, "gSPMoveMem",           NULL, NULL },
	{ 0x04, "gSPVertex",            NULL, dec_gSPVertex_f3d },
	//{ 0x05, "gSPNoop", NULL, NULL },
	{ 0x06, "gSPDisplayList",       op_gSPDisplayList, dec_gSPDisplayList },
	//{ 0x07, "gSPNoop", NULL, NULL },
	//{ 0x08, "gSPNoop", NULL, NULL },
	//{ 0x09, "gSPSprite2D", NULL, NULL },
	// { 0xB1, "gSPTri4", NULL, NULL },
	{ 0xB2, "rdphalf_cont",         NULL, NULL },
	{ 0xB3, "rdphalf_2",            NULL, NULL },
	{ 0xB4, "rdphalf_1",            NULL, NULL },
	// { 0xB5, "line3d", NULL, NULL },
	{ 0xB6, "gSPClearGeometryMode", NULL, NULL },
	{ 0xB7, "gSPSetGeometryMode",   NULL, dec_gSPSetGeometryMode_f3d },
	{ 0xB8, "gSPEndDisplayList",    op_gSPEndDisplayList, dec_NoParams },
	{ 0xB9, "gSPSetOtherModeLow",   NULL, NULL },
	{ 0xBA, "gSPSetOtherModeHigh",  NULL, NULL },
	{ 0xBB, "gSPTexture",           NULL, NULL },
	{ 0xBC, "gSPMoveWord",          op_gSPMoveWord_f3d, dec_gSPMoveWord_f3d },
	{ 0xBD, "gSPPopMatrix",         NULL, NULL },
	{ 0xBE, "gSPCullDisplayList",   NULL, NULL },
	{ 0xBF, "gSP1Triangle",         NULL, dec_gSP1Triangle_f3d },
	{ 0, NULL, NULL, NULL }
};

dl_cmd_info_t CDisplayListParser::Commands_F3DEX[] = {
	{ 0x06, "gSPDisplayList",       op_gSPDisplayList, dec_gSPDisplayList },
	{ 0xB8, "gSPEndDisplayList",    op_gSPEndDisplayList, dec_NoParams },
	{ 0xBC, "gSPMoveWord",          op_gSPMoveWord_f3d, dec_gSPMoveWord_f3d },
	{ 0, NULL, NULL, NULL }
};

dl_cmd_info_t CDisplayListParser::Commands_F3DEX2[] = {
	{ 0xDB, "gSPMoveWord",          op_gSPMoveWord_f3dex2, dec_gSPMoveWord_f3dex2 },
	{ 0xDE, "gSPDisplayList",       op_gSPDisplayList, dec_gSPDisplayList },
	{ 0xDF, "gSPEndDisplayList",    op_gSPEndDisplayList, dec_NoParams },
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

void CDisplayListParser::op_gSPDisplayList(hl_state_t* state)
{
	dl_cmd_dl_t* cmd = &state->command.dl;

	if (cmd->branch == 0)
	{
		state->stack[state->stackIndex++] = state->address;
	}
	state->address = cmd->address;
}

void CDisplayListParser::op_gSPEndDisplayList(hl_state_t* state)
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

void CDisplayListParser::op_gSPMoveWord_f3d(hl_state_t* state)
{
	dl_cmd_moveword_f3d_t* cmd = &state->command.moveword_f3d;

	if (cmd->index == 0x06) // MW_SEGMENT
	{
		int segno = cmd->offset / 4;
		uint32_t physAddress = cmd->data;
		state->segments[segno] = physAddress;
	}
}

void CDisplayListParser::op_gSPMoveWord_f3dex2(hl_state_t* state)
{
	dl_cmd_moveword_f3dex2_t* cmd = &state->command.moveword_f3dex2;

	MessageBox(NULL, "moveword f3dex2", "", MB_OK);

	if (cmd->index == 0x06) // MW_SEGMENT
	{
		int segno = cmd->offset / 4;
		uint32_t physAddress = cmd->data;
		state->segments[segno] = physAddress;
	}
}

/////////////

const char* CDisplayListParser::dec_NoParams(hl_state_t*, char* paramsBuf)
{
	sprintf(paramsBuf, "");
	return NULL;
}

const char* CDisplayListParser::dec_gSPMoveWord_f3d(hl_state_t* state, char* paramsBuf)
{
	dl_cmd_moveword_f3d_t* cmd = &state->command.moveword_f3d;

	if (cmd->index == 0x06) // MW_SEGMENT
	{
		sprintf(paramsBuf, "0x%02X, 0x%08X", cmd->offset/4, (uint32_t)cmd->data);
		return "gSPSegment";
	}

	return NULL;
}

const char* CDisplayListParser::dec_gSPMoveWord_f3dex2(hl_state_t* state, char* paramsBuf)
{
	dl_cmd_moveword_f3dex2_t* cmd = &state->command.moveword_f3dex2;

	if (cmd->index == 0x06) // MW_SEGMENT
	{
		sprintf(paramsBuf, "0x%02X, 0x%08X", cmd->offset / 4, (uint32_t)cmd->data);
		return "gSPSegment";
	}

	return NULL;
}

const char* CDisplayListParser::dec_gSP1Triangle_f3d(hl_state_t* state, char* paramsBuf)
{
	dl_cmd_tri1_f3d_t *cmd = &state->command.tri1_f3d;
	sprintf(paramsBuf, "%d, %d, %d", cmd->v0 / 10, cmd->v1 / 10, cmd->v2 / 10);
	return NULL;
}

const char* CDisplayListParser::dec_gSPVertex_f3d(hl_state_t* state, char* paramsBuf)
{
	dl_cmd_vtx_f3d_t *cmd = &state->command.vtx_f3d;
	sprintf(paramsBuf, "0x%08X, %d, %d", cmd->address, cmd->num+1, cmd->idx);
	return NULL;
}

const char* CDisplayListParser::dec_gSPSetGeometryMode_f3d(hl_state_t* state, char* paramsBuf)
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

const char* CDisplayListParser::dec_gSPDisplayList(hl_state_t* state, char* paramsBuf)
{
	dl_cmd_dl_t *cmd = &state->command.dl;
	sprintf(paramsBuf, "0x%08X", cmd->address);

	if (cmd->branch)
	{
		return "gSPBranchList";
	}

	return NULL;
}

const char* CDisplayListParser::dec_HexParam32(hl_state_t* state, char* paramsBuf)
{
	sprintf(paramsBuf, "0x%08X", state->command.w1);
	return NULL;
}

const char* CDisplayListParser::dec_gSPMatrix_f3d(hl_state_t* state, char* paramsBuf)
{
	dl_cmd_mtx_f3d_t* cmd = &state->command.mtx_f3d;

	sprintf(paramsBuf, "0x%08X, (%s | %s | %s)", cmd->address,
		(cmd->params & 1) ? "G_MTX_PROJECTION" : "G_MTX_MODELVIEW",
		(cmd->params & 2) ? "G_MTX_LOAD" : "G_MTX_MUL",
		(cmd->params & 4) ? "G_MTX_PUSH" : "G_MTX_NOPUSH");
	
	return NULL;
}

const char* CDisplayListParser::dec_gDPFillRectangle(hl_state_t* state, char* paramsBuf)
{
	dl_cmd_fillrect_t* cmd = &state->command.fillrect;
	sprintf(paramsBuf, "%d, %d, %d, %d", cmd->ulx >> 2, cmd->uly >> 2, cmd->lrx >> 2, cmd->lry >> 2);
	return NULL;
}

const char* CDisplayListParser::dec_gDPSetScissor(hl_state_t* state, char* paramsBuf)
{
	dl_cmd_setscissor_t* cmd = &state->command.setscissor;

	const char* szMode = "?";

	switch (cmd->mode)
	{
	case 0: szMode = "G_SC_NON_INTERLACE"; break;
	case 2: szMode = "G_SC_EVEN_INTERLACE"; break;
	case 3: szMode = "G_SC_ODD_INTERLACE"; break;
	}

	// TODO check for frac bits and use gDPSetScissorFrac if set

	sprintf(paramsBuf, "%s, %d, %d, %d, %d", szMode, cmd->ulx >> 2, cmd->uly >> 2, cmd->lrx >> 2, cmd->lry >> 2);
	return NULL;
}

