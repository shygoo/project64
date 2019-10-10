#include <stdafx.h>
#include "GfxOps.h"
#include "GfxOpTypes.h"
#include "GfxParser.h"
#include "GfxLabels.h"
#include "GfxState.h"

#define COLOR_JUMP      RGB(0, 100, 0)
#define COLOR_BRANCH    RGB(100, 100, 0)
#define COLOR_ENDDL     RGB(100, 0, 0)
#define COLOR_DMA       RGB(100, 0, 100)
#define COLOR_NOP       RGB(100, 100, 100)
#define COLOR_PRIMITIVE RGB(0, 150, 150)

ucode_checksum_t CGfxOps::MicrocodeChecksums[] = {
    //checksum    version
    { 0x0D7CBFFB, UCODE_DKR },
    { 0x63BE08B1, UCODE_DKR },
    { 0x63BE08B3, UCODE_DKR },
    { 0x3A1C2B34, UCODE_F3D },
    { 0x3A1CBAC3, UCODE_F3D },
    { 0x3F7247FB, UCODE_F3D },
    { 0x4165E1FD, UCODE_F3D },
    { 0x5182F610, UCODE_F3D },
    { 0x5D1D6F53, UCODE_F3D },
    { 0x6E4D50AF, UCODE_F3D },
    { 0xAE08D5B9, UCODE_F3D },
    { 0xB54E7F93, UCODE_F3D },
    { 0xBC03E969, UCODE_F3D },
    { 0xD5604971, UCODE_F3D },
    { 0xD5D68B1F, UCODE_F3D },
    { 0xD67C2F8B, UCODE_F3D },
    { 0xE41EC47E, UCODE_F3D },
    { 0x006BD77F, UCODE_F3D },
    { 0x07200895, UCODE_F3D },
    { 0xCEE7920F, UCODE_F3DEX },
    { 0x1118B3E0, UCODE_F3DEX },
    { 0x1517A281, UCODE_F3DEX },
    { 0x1DE712FF, UCODE_F3DEX },
    { 0x24CD885B, UCODE_F3DEX },
    { 0x26A7879A, UCODE_F3DEX },
    { 0x2C7975D6, UCODE_F3DEX },
    { 0x2D3FE3F1, UCODE_F3DEX },
    { 0x327B933D, UCODE_F3DEX },
    { 0x339872A6, UCODE_F3DEX },
    { 0x3FF1A4CA, UCODE_F3DEX },
    { 0x4340AC9B, UCODE_F3DEX },
    { 0x440CFAD6, UCODE_F3DEX },
    { 0x4FE6DF78, UCODE_F3DEX },
    { 0x5257CD2A, UCODE_F3DEX },
    { 0x5414030C, UCODE_F3DEX },
    { 0x5414030D, UCODE_F3DEX },
    { 0x559FF7D4, UCODE_F3DEX },
    { 0x5DF1408C, UCODE_F3DEX },
    { 0x5EF4E34A, UCODE_F3DEX },
    { 0x6075E9EB, UCODE_F3DEX },
    { 0x60C1DCC4, UCODE_F3DEX },
    { 0x64ED27E5, UCODE_F3DEX },
    { 0x66C0B10A, UCODE_F3DEX },
    { 0x6EAA1DA8, UCODE_F3DEX },
    { 0x72A4F34E, UCODE_F3DEX },
    { 0x73999A23, UCODE_F3DEX },
    { 0x7DF75834, UCODE_F3DEX },
    { 0x7F2D0A2E, UCODE_F3DEX },
    { 0x82F48073, UCODE_F3DEX },
    { 0x832FCB99, UCODE_F3DEX },
    { 0x841CE10F, UCODE_F3DEX },
    { 0x863E1CA7, UCODE_F3DEX },
    { 0x8805FFEA, UCODE_F3DEX },
    { 0x8D5735B2, UCODE_F3DEX },
    { 0x8D5735B3, UCODE_F3DEX },
    { 0x97D1B58A, UCODE_F3DEX },
    { 0xA346A5CC, UCODE_F3DEX },
    { 0xB1821ED3, UCODE_F3DEX },
    { 0xB4577B9C, UCODE_F3DEX },
    { 0xBE78677C, UCODE_F3DEX },
    { 0xBED8B069, UCODE_F3DEX },
    { 0xC3704E41, UCODE_F3DEX },
    { 0xC46DBC3D, UCODE_F3DEX },
    { 0xC99A4C6C, UCODE_F3DEX },
    { 0xD1663234, UCODE_F3DEX },
    { 0xD2A9F59C, UCODE_F3DEX },
    { 0xD41DB5F7, UCODE_F3DEX },
    { 0xD57049A5, UCODE_F3DEX },
    { 0xD802EC04, UCODE_F3DEX },
    { 0xE89C2B92, UCODE_F3DEX },
    { 0xE9231DF2, UCODE_F3DEX },
    { 0xEC040469, UCODE_F3DEX },
    { 0xEE47381B, UCODE_F3DEX },
    { 0xFB816260, UCODE_F3DEX },
    { 0x05165579, UCODE_F3DEX },
    { 0x05777C62, UCODE_F3DEX },
    { 0x057E7C62, UCODE_F3DEX },
    { 0x5B5D3763, UCODE_F3DEX_WR },
    { 0x0FF79527, UCODE_F3DEX2 },
    { 0x168E9CD5, UCODE_F3DEX2 },
    { 0x1A1E18A0, UCODE_F3DEX2 },
    { 0x1A1E1920, UCODE_F3DEX2 },
    { 0x1A62DBAF, UCODE_F3DEX2 },
    { 0x1A62DC2F, UCODE_F3DEX2 },
    { 0x21F91834, UCODE_F3DEX2 },
    { 0x21F91874, UCODE_F3DEX2 },
    { 0x22099872, UCODE_F3DEX2 },
    { 0x2B291027, UCODE_F3DEX2 },
    { 0x2F71D1D5, UCODE_F3DEX2 },
    { 0x2F7DD1D5, UCODE_F3DEX2 },
    { 0x377359B6, UCODE_F3DEX2 },
    { 0x485ABFF2, UCODE_F3DEX2 },
    { 0x5D3099F1, UCODE_F3DEX2 },
    { 0x6124A508, UCODE_F3DEX2 },
    { 0x630A61FB, UCODE_F3DEX2 },
    { 0x65201989, UCODE_F3DEX2 },
    { 0x65201A09, UCODE_F3DEX2 },
    { 0x679E1205, UCODE_F3DEX2 },
    { 0x6D8F8F8A, UCODE_F3DEX2 },
    { 0x753BE4A5, UCODE_F3DEX2 },
    { 0x93D11F7B, UCODE_F3DEX2 },
    { 0x93D11FFB, UCODE_F3DEX2 },
    { 0x93D1FF7B, UCODE_F3DEX2 },
    { 0x9551177B, UCODE_F3DEX2 },
    { 0x955117FB, UCODE_F3DEX2 },
    { 0x95CD0062, UCODE_F3DEX2 },
    { 0xA2D0F88E, UCODE_F3DEX2 },
    { 0xAA86CB1D, UCODE_F3DEX2 },
    { 0xAAE4A5B9, UCODE_F3DEX2 },
    { 0xAD0A6292, UCODE_F3DEX2 },
    { 0xAD0A6312, UCODE_F3DEX2 },
    { 0xB62F900F, UCODE_F3DEX2 },
    { 0xBA65EA1E, UCODE_F3DEX2 },
    { 0xBC45382E, UCODE_F3DEX2 },
    { 0xC2A01ECF, UCODE_F3DEX2 },
    { 0xC901CE73, UCODE_F3DEX2 },
    { 0xC901CEF3, UCODE_F3DEX2 },
    { 0xCB8C9B6C, UCODE_F3DEX2 },
    { 0xCFA35A45, UCODE_F3DEX2 },
    { 0xDA13AB96, UCODE_F3DEX2 },
    { 0xDE7D67D4, UCODE_F3DEX2 },
    { 0xE1290FA2, UCODE_F3DEX2 },
    { 0xE65CB4AD, UCODE_F3DEX2 },
    { 0x03044B84, UCODE_F3DEX2 },
    { 0x030F4B84, UCODE_F3DEX2 },
    { 0xBA86CB1D, UCODE_F3DEXBG },
    { 0xEF54EE35, UCODE_F3DTEXA },
    { 0x5B5D36E3, UCODE_FACTOR5 },
    { 0x47D46E86, UCODE_PD },
    { 0x1EA9E30F, UCODE_S2DEX },
    { 0x299D5072, UCODE_S2DEX },
    { 0x2B5A89C2, UCODE_S2DEX },
    { 0x6BB745C9, UCODE_S2DEX },
    { 0x74AF0A74, UCODE_S2DEX },
    { 0x794C3E28, UCODE_S2DEX },
    { 0xD20DEDBF, UCODE_S2DEX },
    { 0x1F120BBB, UCODE_TURBO3D },
    { 0xF9893F70, UCODE_TURBO3D },
    { 0xFF372492, UCODE_TURBO3D },
    { 0x0D7BBFFB, UCODE_UNKNOWN },
    { 0x0FF795BF, UCODE_UNKNOWN },
    { 0x844B55B5, UCODE_UNKNOWN },
    { 0x86B1593E, UCODE_UNKNOWN },
    { 0x8EC3E124, UCODE_UNKNOWN },
    { 0xD5C4DC96, UCODE_UNKNOWN },
    { 0x0BF36D36, UCODE_ZSORT },
    { 0, UCODE_UNKNOWN }

};

ucode_info_t CGfxOps::Microcodes[] = {
    //version       name      op functions
    { UCODE_F3D,    "Fast3D", Commands_F3D },    // sm64
    { UCODE_F3DEX,  "F3DEX",  Commands_F3DEX },
    { UCODE_F3DEX2, "F3DEX2", Commands_F3DEX2 }, // kirby
    { UCODE_UNKNOWN, NULL, NULL }
};

// ucode-independent rdp commands
dl_cmd_info_t CGfxOps::Commands_RDP[] = {
    { 0xC0, "gsDPNoop",                 op_Nop },
    { 0xC8, "gsDPTriFill",              NULL },
    { 0xC9, "gsDPTriFillZ",             NULL },
    { 0xCA, "gsDPTriTxtr",              NULL },
    { 0xCB, "gsDPTriTxtrZ",             NULL },
    { 0xCC, "gsDPTriShade",             NULL },
    { 0xCD, "gsDPTriShadeZ",            NULL },
    { 0xCE, "gsDPTriShadeTxtr",         NULL },
    { 0xCF, "gsDPTriShadeTxtrZ",        NULL },
    { 0xE4, "gsSPTextureRectangle",     op_gsSPTextureRectangle },
    { 0xE5, "gsSPTextureRectangleFlip", op_gsSPTextureRectangleFlip },
    { 0xE6, "gsDPLoadSync",             op_Nop },
    { 0xE7, "gsDPPipeSync",             op_Nop },
    { 0xE8, "gsDPTileSync",             op_Nop },
    { 0xE9, "gsDPFullSync",             op_Nop },
    { 0xEA, "gsDPSetKeyGB",             NULL },
    { 0xEB, "gsDPSetKeyR",              NULL },
    { 0xEC, "gsDPSetConvert",           NULL },
    { 0xED, "gsDPSetScissor",           op_gsDPSetScissor },
    { 0xEE, "gsDPSetPrimDepth",         NULL },
    { 0xEF, "gsDPSetOtherMode",         NULL },
    { 0xF0, "gsDPLoadTLUTCmd",          op_gsDPLoadTLUTCmd },
    { 0xF2, "gsDPSetTileSize",          op_gsDPSetTileSize },
    { 0xF3, "gsDPLoadBlock",            op_gsDPLoadBlock },
    { 0xF4, "gsDPLoadTile",             op_gsDPLoadTile },
    { 0xF5, "gsDPSetTile",              op_gsDPSetTile },
    { 0xF6, "gsDPFillRectangle",        op_gsDPFillRectangle },
    { 0xF7, "gsDPSetFillColor",         op_gsDPSetFillColor },
    { 0xF8, "gsDPSetFogColor",          op_gsDPSetFogColor },
    { 0xF9, "gsDPSetBlendColor",        op_gsDPSetBlendColor },
    { 0xFA, "gsDPSetPrimColor",         op_gsDPSetPrimColor },
    { 0xFB, "gsDPSetEnvColor",          op_gsDPSetEnvColor },
    { 0xFC, "gsDPSetCombineLERP",       op_gsDPSetCombineLERP },
    { 0xFD, "gsDPSetTextureImage",      op_gsDPSetTextureImage },
    { 0xFE, "gsDPSetDepthImage",        op_gsDPSetDepthImage },
    { 0xFF, "gsDPSetColorImage",        op_gsDPSetColorImage },
    { 0, NULL, NULL }
};

dl_cmd_info_t CGfxOps::Commands_F3D[] = {
    { 0x00, "gsSPNoop",              op_Nop },
    { 0x01, "gsSPMatrix",            op_gsSPMatrix_f3d },
    //{ 0x02, "gsSPNoop", NULL, NULL }, ?
    { 0x03, "gsSPMoveMem",           op_gsSPMoveMem_f3d },
    { 0x04, "gsSPVertex",            op_gsSPVertex_f3d },
    //{ 0x05, "gsSPNoop", NULL, NULL },
    { 0x06, "gsSPDisplayList",       op_gsSPDisplayList_f3d },
    //{ 0x07, "gsSPNoop", NULL, NULL },
    //{ 0x08, "gsSPNoop", NULL, NULL },
    //{ 0x09, "gsSPSprite2D", NULL, NULL },
    // { 0xB1, "gsSPTri4", NULL, NULL },
    { 0xB2, "G_RDPHALF_CONT",          NULL },
    { 0xB3, "G_RDPHALF_2",             NULL },
    { 0xB4, "G_RDPHALF_1",             NULL },
    // { 0xB5, "line3d", NULL, NULL },
    { 0xB6, "gsSPClearGeometryMode", op_gsSPClearGeometryMode_f3d },
    { 0xB7, "gsSPSetGeometryMode",   op_gsSPSetGeometryMode_f3d },
    { 0xB8, "gsSPEndDisplayList",    op_gsSPEndDisplayList_f3d },
    { 0xB9, "gsSPSetOtherMode_l",    op_gsSPSetOtherMode_l_f3d },
    { 0xBA, "gsSPSetOtherMode_h",    op_gsSPSetOtherMode_h_f3d },
    { 0xBB, "gsSPTexture",           op_gsSPTexture_f3d },
    { 0xBC, "gsMoveWd",              op_gsMoveWd_f3d },
    { 0xBD, "gsSPPopMatrix",         NULL },
    { 0xBE, "gsSPCullDisplayList",   NULL },
    { 0xBF, "gsSP1Triangle",         op_gsSP1Triangle_f3d },
    { 0, NULL, NULL }
};

dl_cmd_info_t CGfxOps::Commands_F3DEX[] = {
    { 0x00, "gsSPNoop",              op_Nop },
    { 0x01, "gsSPMatrix",            op_gsSPMatrix_f3dex },
    { 0x03, "gsSPMoveMem",           op_gsSPMoveMem_f3dex },
    { 0x04, "gsSPVertex",            op_gsSPVertex_f3dex },
    { 0x06, "gsSPDisplayList",       op_gsSPDisplayList_f3dex },
    { 0xB1, "gsSP2Triangles",        op_gsSP2Triangles_f3dex },
    { 0xB5, "gsSP1Quadrangle",       op_gsSP1Quadrangle_f3dex },
    { 0xB6, "gsSPClearGeometryMode", op_gsSPClearGeometryMode_f3dex },
    { 0xB7, "gsSPSetGeometryMode",   op_gsSPSetGeometryMode_f3dex },
    { 0xB8, "gsSPEndDisplayList",    op_gsSPEndDisplayList_f3dex },
    { 0xB9, "gsSPSetOtherMode_l",    op_gsSPSetOtherMode_l_f3dex },
    { 0xBA, "gsSPSetOtherMode_h",    op_gsSPSetOtherMode_h_f3dex },
    { 0xBB, "gsSPTexture",           op_gsSPTexture_f3dex },
    { 0xBC, "gsMoveWd",              op_gsMoveWd_f3dex },
    { 0xBD, "gsSPPopMatrix",         NULL },
    { 0xBF, "gsSP1Triangle",         op_gsSP1Triangle_f3dex },
    { 0, NULL, NULL }
};

dl_cmd_info_t CGfxOps::Commands_F3DEX2[] = {
    { 0x00, "gsSPNoop",           op_Nop },
    { 0x01, "gsSPVertex",         op_gsSPVertex_f3dex2 },
    { 0x05, "gsSP1Triangle",      op_gsSP1Triangle_f3dex2 },
    { 0x06, "gsSP2Triangles",     op_gsSP2Triangles_f3dex2 },
    { 0xD7, "gsSPTexture",        op_gsSPTexture_f3d }, // todo
    { 0xD9, "gsSPGeometryMode",   op_gsSPGeometryMode_f3dex2 },
    { 0xDA, "gsSPMatrix",         op_gsSPMatrix_f3dex2 },
    { 0xDB, "gsMoveWd",           op_gsMoveWd_f3dex2 },
    { 0xDC, "gsSPMoveMem",        op_gsSPMoveMem_f3dex2 },
    { 0xDE, "gsSPDisplayList",    op_gsSPDisplayList_f3dex2 },
    { 0xDF, "gsSPEndDisplayList", op_gsSPEndDisplayList_f3dex2 },
    { 0xE2, "gsSPSetOtherMode_l", op_gsSPSetOtherMode_l_f3dex2 },
    { 0xE3, "gsSPSetOtherMode_h", op_gsSPSetOtherMode_h_f3dex2 },
    { 0xE1, "G_RDPHALF_1",        NULL },
    { 0xF1, "G_RDPHALF_2",        NULL },
    { 0, NULL, NULL }
};

/**********************************/

void decoded_cmd_t::Rename(const char* _name)
{
    name = _name;
}

void decoded_cmd_t::SetDramResource(CHleGfxState* state, resource_type_t resType, uint32_t param)
{
    dramResource.type = resType;
    dramResource.address = state->m_spCommand.w1;
    dramResource.virtAddress = state->SegmentedToVirtual(state->m_spCommand.w1);
    dramResource.param = param;
}

/**********************************/

const dl_cmd_info_t* CGfxOps::LookupCommand(dl_cmd_info_t* commands, uint8_t cmdByte)
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

const ucode_info_t* CGfxOps::LookupMicrocode(uint32_t checksum)
{
    ucode_version_t version = UCODE_UNKNOWN;

    for (int i = 0; MicrocodeChecksums[i].checksum != 0; i++)
    {
        if (MicrocodeChecksums[i].checksum == checksum)
        {
            version = MicrocodeChecksums[i].version;
            break;
        }
    }

    if (version == UCODE_UNKNOWN)
    {
        return NULL;
    }

    for (int i = 0; Microcodes[i].version != 0; i++)
    {
        if (Microcodes[i].version == version)
        {
            return &Microcodes[i];
        }
    }

    return NULL;
}

/**********************************/

void CGfxOps::op_Nop(CHleGfxState*, decoded_cmd_t* dc)
{
    dc->params = "";
    dc->listFgColor = COLOR_NOP;
}

void CGfxOps::op_gsSPTextureRectangle(CHleGfxState* state, decoded_cmd_t* dc)
{
    dl_cmd_texrect_t* cmd = &state->m_spCommand.texrect;
    // note: assuming no frac bits
    dc->params = stdstr_f("/*ulx*/  (%d << 2), /*uly*/  (%d << 2), /*lrx*/  (%d << 2), /*lry*/  (%d << 2), /*tile*/  %d",
        (cmd->xl >> 2), (cmd->yl >> 2), (cmd->xh >> 2), (cmd->yh >> 2), cmd->tile);
    
    // lookahead for dphalf1, dphalf2 for complete command
    dl_cmd_t cmd0, cmd1;
    bool bRead0 = state->GetCommand(state->m_spCommandAddress + 0, &cmd0);
    bool bRead1 = state->GetCommand(state->m_spCommandAddress + 8, &cmd1);
    if ((bRead0 && bRead1) && cmd0.commandByte == 0xE1 && cmd1.commandByte == 0xF1)
    {
        dc->params += stdstr_f(", %d, %d, %d, %d", cmd0.texrect_half1.s, cmd0.texrect_half1.t,
            cmd1.texrect_half2.dsdx, cmd1.texrect_half2.dsdy);

        // skip dphalf1, dphalf2 commands
        // todo make this optional
        state->m_spCommandAddress += 16;
    }

    dc->listFgColor = COLOR_PRIMITIVE;
}

void CGfxOps::op_gsSPDisplayList_f3d(CHleGfxState* state, decoded_cmd_t* dc)
{
    dl_cmd_dl_t* cmd = &state->m_spCommand.dl;

    if (cmd->branch == 0)
    {
        state->m_spStack[state->m_spStackIndex++] = state->m_spCommandAddress;

        dc->SetDramResource(state, RES_DL);
        dc->listFgColor = COLOR_JUMP;
    }
    else
    {
        dc->Rename("gsSPBranchList");
        dc->listFgColor = COLOR_BRANCH;
    }

    state->m_spCommandAddress = cmd->address;

    dc->params = stdstr_f("0x%08X /*0x%08X*/", cmd->address,
        state->SegmentedToVirtual(cmd->address));
}

void CGfxOps::op_gsSPEndDisplayList_f3d(CHleGfxState* state, decoded_cmd_t* dc)
{
    if (state->m_spStackIndex > 0)
    {
        state->m_spCommandAddress = state->m_spStack[--state->m_spStackIndex];
    }
    else
    {
        state->m_bDone = true;
    }

    dc->params = "";
    dc->listFgColor = COLOR_ENDDL;
}

void CGfxOps::op_gsSP1Triangle_f3d(CHleGfxState* state, decoded_cmd_t* dc)
{
    dl_cmd_tri1_f3d_t *cmd = &state->m_spCommand.tri1_f3d;
    dc->params = stdstr_f("%d, %d, %d", cmd->v0 / 10, cmd->v1 / 10, cmd->v2 / 10);

    // report triangle for the mesh builder
    dc->numTris = 1;
    dc->tris[0] = {
        state->m_spVertices[cmd->v0 / 10],
        state->m_spVertices[cmd->v1 / 10],
        state->m_spVertices[cmd->v2 / 10]
    };

    dc->listFgColor = COLOR_PRIMITIVE;
}

void CGfxOps::op_gsSP1Triangle_f3dex2(CHleGfxState* state, decoded_cmd_t* dc)
{
    dl_cmd_tri1_f3dex2_t *cmd = &state->m_spCommand.tri1_f3dex2;
    dc->params = stdstr_f("%d, %d, %d", cmd->v0 / 2, cmd->v1 / 2, cmd->v2 / 2);

    // report triangle for the mesh builder
    dc->numTris = 1;
    dc->tris[0] = {
        state->m_spVertices[cmd->v0 / 2],
        state->m_spVertices[cmd->v1 / 2],
        state->m_spVertices[cmd->v2 / 2]
    };

    dc->listFgColor = COLOR_PRIMITIVE;
}

void CGfxOps::op_gsSP1Triangle_f3dex(CHleGfxState* state, decoded_cmd_t* dc)
{
    dl_cmd_tri1_f3d_t *cmd = &state->m_spCommand.tri1_f3d;
    dc->params = stdstr_f("%d, %d, %d", cmd->v0 / 2, cmd->v1 / 2, cmd->v2 / 2);

    // report triangle for the mesh builder
    dc->numTris = 1;
    dc->tris[0] = {
        state->m_spVertices[cmd->v0 / 2],
        state->m_spVertices[cmd->v1 / 2],
        state->m_spVertices[cmd->v2 / 2]
    };

    dc->listFgColor = COLOR_PRIMITIVE;
}

void CGfxOps::op_gsSP2Triangles_f3dex(CHleGfxState* state, decoded_cmd_t* dc)
{
    dl_cmd_tri2_f3dex_t *cmd = &state->m_spCommand.tri2_f3dex;
    dc->params = stdstr_f("%d, %d, %d, %d, %d, %d",
        cmd->v0 / 2, cmd->v1 / 2, cmd->v2 / 2, cmd->v3 / 2, cmd->v4 / 2, cmd->v5 / 2);

    // report triangle for the mesh builder
    dc->numTris = 2;
    dc->tris[0] = {
        state->m_spVertices[cmd->v0 / 2],
        state->m_spVertices[cmd->v1 / 2],
        state->m_spVertices[cmd->v2 / 2]
    };

    dc->tris[1] = {
        state->m_spVertices[cmd->v3 / 2],
        state->m_spVertices[cmd->v4 / 2],
        state->m_spVertices[cmd->v5 / 2]
    };

    dc->listFgColor = COLOR_PRIMITIVE;
}

void CGfxOps::op_gsSP1Quadrangle_f3dex(CHleGfxState* state, decoded_cmd_t* dc)
{
    dl_cmd_quad_f3dex_t *cmd = &state->m_spCommand.quad_f3dex;
    dc->params = stdstr_f("%d, %d, %d, %d", cmd->v0 / 2, cmd->v1 / 2, cmd->v2 / 2, cmd->v3 / 2);

    // todo
    dc->listFgColor = COLOR_PRIMITIVE;
}

void CGfxOps::op_gsSPVertex_f3dex2(CHleGfxState* state, decoded_cmd_t* dc)
{
    dl_cmd_vtx_f3dex2_t* cmd = &state->m_spCommand.vtx_f3dex2;
    int idx = (cmd->idx - (cmd->num * 2)) / 2;
    state->LoadVertices(cmd->address, idx, cmd->num);

    dc->params = stdstr_f("/*addr*/ 0x%08X, /*numv*/ %d, /*vidx*/ %d /*0x%08X*/", cmd->address, cmd->num, idx,
        state->SegmentedToVirtual(cmd->address));

    dc->listFgColor = COLOR_DMA;
}

void CGfxOps::op_gsSPVertex_f3dex(CHleGfxState* state, decoded_cmd_t* dc)
{
    dl_cmd_vtx_f3dex_t* cmd = &state->m_spCommand.vtx_f3dex;

    state->LoadVertices(cmd->address, cmd->idx, cmd->num + 1);
    
    dc->params = stdstr_f("/*addr*/ 0x%08X, /*numv*/ %d, /*vidx*/ %d /*0x%08X*/", cmd->address, cmd->num, cmd->idx,
        state->SegmentedToVirtual(cmd->address));

    dc->SetDramResource(state, RES_VERTICES, cmd->num + 1);
    dc->listFgColor = COLOR_DMA;
}

void CGfxOps::op_gsSPVertex_f3d(CHleGfxState* state, decoded_cmd_t* dc)
{
    dl_cmd_vtx_f3d_t* cmd = &state->m_spCommand.vtx_f3d;

    state->LoadVertices(cmd->address, cmd->idx, cmd->num + 1);

    dc->params = stdstr_f("/*addr*/ 0x%08X, /*numv*/ %d, /*vidx*/ %d /*0x%08X*/", cmd->address, cmd->num + 1, cmd->idx,
        state->SegmentedToVirtual(cmd->address));

    dc->SetDramResource(state, RES_VERTICES, cmd->num + 1);
    dc->listFgColor = COLOR_DMA;
}

void CGfxOps::op_gsMoveWd_f3d(CHleGfxState* state, decoded_cmd_t* dc)
{
    dl_cmd_moveword_f3d_t* cmd = &state->m_spCommand.moveword_f3d;

    // G_MW_MATRIX		0x00
    // G_MW_CLIP		0x04
    // G_MW_SEGMENT		0x06
    // G_MW_FOG		    0x08
    // G_MW_LIGHTCOL    0x0a
    // G_MW_FORCEMTX    0x0c ex2
    // G_MW_POINTS		0x0c
    // G_MW_PERSPNORM   0x0e

    if (cmd->index == 0x02 && cmd->offset == 0x00) // MW_NUMLIGHT, MWO_NUMLIGHT
    {
        state->m_spNumLights = (uint8_t)((cmd->data - 0x80000000) / 32) - 1;

        dc->params = stdstr_f("%d", state->m_spNumLights);
        dc->Rename("gsSPNumLights");
    }
    else if (cmd->index == 0x06) // MW_SEGMENT
    {
        int segno = cmd->offset / 4;
        uint32_t physAddress = cmd->data;
        state->m_spSegments[segno] = physAddress;

        dc->params = stdstr_f("/*segment*/ 0x%02X, /*address*/ 0x%08X", cmd->offset / 4, cmd->data);
        dc->Rename("gsSPSegment");
        dc->SetDramResource(state, RES_SEGMENT, cmd->offset / 4);
    }

    dc->listFgColor = COLOR_DMA;
}

void CGfxOps::op_gsMoveWd_f3dex2(CHleGfxState* state, decoded_cmd_t* dc)
{
    dl_cmd_moveword_f3dex2_t* cmd = &state->m_spCommand.moveword_f3dex2;

    if (cmd->index == 0x06) // MW_SEGMENT
    {
        dc->Rename("gsSPSegment");
        dc->params = stdstr_f("/*segment*/ 0x%02X, /*address*/ 0x%08X", cmd->offset / 4, cmd->data);

        state->m_spSegments[cmd->offset / 4] = cmd->data;
        dc->SetDramResource(state, RES_SEGMENT);
    }
    else if (cmd->index == 0x02) // MW_NUMLIGHT
    {
        dc->Rename("gsSPNumLights");
        dc->params = stdstr_f("%d", cmd->data / 24);
    }
    else if (cmd->index == 0x04) // MW_CLIP
    {
        // lookahead for 3 more MW_CLIP for gSPClipRatio
        dl_cmd_t cmd0, cmd1, cmd2;
        bool bRead0, bRead1, bRead2;
        state->GetCommand(state->m_spCommandAddress + 0, &cmd0);
        state->GetCommand(state->m_spCommandAddress + 8, &cmd1);
        state->GetCommand(state->m_spCommandAddress + 16, &cmd2);

        if (cmd0.commandByte == 0xDB &&
            cmd1.commandByte == 0xDB &&
            cmd2.commandByte == 0xDB &&
            cmd0.moveword_f3dex2.index == 0x04 &&
            cmd1.moveword_f3dex2.index == 0x04 &&
            cmd2.moveword_f3dex2.index == 0x04)
        {
            // note: assuming ratio is valid and equal for each command
            dc->Rename("gSPClipRatio");
            dc->params = stdstr_f("FRUSTRATIO_%d", cmd0.moveword_f3dex2.data);

            // skip the additional movewd commands
            // todo make this optional
            state->m_spCommandAddress += 24;
        }
        else
        {
            const char* szClipDir = CGfxLabels::LookupName(CGfxLabels::ClipDirections, cmd->offset);
            const char* szClipRatio = CGfxLabels::LookupName(CGfxLabels::ClipRatios, cmd->data);
            dc->params = stdstr_f("G_MW_CLIP, %s, %s", szClipDir, szClipRatio);
        }
    }
    else if (cmd->index == 0x08 && cmd->offset == 0) // MW_FOG, MWO_FOG
    {
        dc->Rename("gsSPFogFactor");
        dc->params = stdstr_f("0x%04X, 0x%04X", (cmd->data >> 16), (cmd->data & 0xFFFF));
    }
    else if (cmd->index == 0x0A)
    {
        // todo: lookahead for another G_MW_LIGHTCOL for gSPLightColor
        const char* szLight = CGfxLabels::LookupName(CGfxLabels::LightColorOffsets, cmd->offset);
        dc->params = stdstr_f("G_MW_LIGHTCOL, %s, 0x%08X", szLight, cmd->data);
    }
    else if (cmd->index = 0x0e && cmd->offset == 0)
    {
        dc->Rename("gsSPPerspNormalize");
        dc->params = stdstr_f("%d", cmd->data);
    }
}

void CGfxOps::op_gsSPTexture_f3d(CHleGfxState* state, decoded_cmd_t* dc)
{
    dl_cmd_texture_f3d_t* cmd = &state->m_spCommand.texture_f3d;
    tile_t* td = &state->m_dpTileDescriptors[cmd->tile];
    td->scaleS = cmd->s;
    td->scaleT = cmd->t;
    td->enabled = cmd->on;
    td->mipmapLevels = cmd->level;

    dc->params = stdstr_f("/*scaleS*/ 0x%04X, /*scaleT*/ 0x%04X, /*levels*/ %d, /*tile*/ %d, /*on*/ %d", cmd->s, cmd->t, cmd->level, cmd->tile, cmd->on);
}

void CGfxOps::op_gsDPSetTile(CHleGfxState* state, decoded_cmd_t* dc)
{
    dl_cmd_settile_t* cmd = &state->m_spCommand.settile;
    tile_t *td = &state->m_dpTileDescriptors[cmd->tile];

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

    const char* fmtName = "?";
    const char* sizName = "?";

    fmtName = CGfxLabels::ImageFormats[cmd->fmt];
    sizName = CGfxLabels::TexelSizes[cmd->siz];

    dc->params = stdstr_f("/*fmt*/ %s, /*siz*/ %s, /*line*/ %d, /*tmem*/ 0x%03X, /*tile*/ %d, /*palette*/ %d, /*cmt*/ %d, /*maskt*/ %d, /*shiftt*/ %d, /*cmt*/ %d, /*masks*/ %d, /*shifts*/ %d", fmtName, sizName,
        cmd->line, cmd->tmem, cmd->tile, cmd->palette,
        cmd->cmt, cmd->maskt, cmd->shiftt,
        cmd->cms, cmd->masks, cmd->shifts);
}

void CGfxOps::op_gsDPSetTextureImage(CHleGfxState* state, decoded_cmd_t* dc)
{
    dl_cmd_settimg_t* cmd = &state->m_spCommand.settimg;
    state->m_dpTextureImage = cmd->address;
    state->m_dpTextureImageSiz = (im_siz_t)cmd->siz;
    state->m_dpTextureImageFmt = (im_fmt_t)cmd->fmt;

    const char* fmtName = "?";
    const char* sizName = "?";

    fmtName = CGfxLabels::ImageFormats[cmd->fmt];
    sizName = CGfxLabels::TexelSizes[cmd->siz];

    dc->params = stdstr_f("/*fmt*/ %s, /*siz*/ %s, /*w*/ %d, /*addr*/ 0x%08X /*0x%08X*/", fmtName, sizName, cmd->width + 1, cmd->address,
        state->SegmentedToVirtual(cmd->address));

    // lookahead for image load macro
    dl_cmd_t cmds[6];
    int nRead = state->GetCommands(state->m_spCommandAddress, 6, cmds);
    if (nRead == 6)
    {
        if (cmds[0].commandByte == 0xF5 && // gDPSetTile
            cmds[1].commandByte == 0xE6 && // gDPLoadSync
            cmds[2].commandByte == 0xF3 && // gDPLoadBlock
            cmds[3].commandByte == 0xE7 && // gDPPipeSync
            cmds[4].commandByte == 0xF5 && // gDPSetTile
            cmds[5].commandByte == 0xF2)   // gDPSetTileSize
        {
            // TODO
            dc->listBgColor = RGB(255, 200, 200);
            dc->macroLength = 6;
            /*
            loadtextureblock or loadmultiblock macro
            gDPLoadTextureBlock
            gDPLoadTextureBlockYuv
            gDPLoadTextureBlockS
            gDPLoadMultiBlockS
            gDPLoadTextureBlockYuvS
            _gDPLoadTextureBlock
            _gDPLoadTextureBlockTile
            gDPLoadMultiBlock
            gDPLoadTextureBlock_4b
            gDPLoadTextureBlock_4bS
            gDPLoadMultiBlock_4b
            gDPLoadMultiBlock_4bS
            _gDPLoadTextureBlock_4b
            */
        }
        else if (cmds[0].commandByte == 0xF5 && // gDPSetTile
                 cmds[1].commandByte == 0xE6 && // gDPLoadSync
                 cmds[2].commandByte == 0xF4 && // gDPLoadTile
                 cmds[3].commandByte == 0xE7 && // gDPPipeSync
                 cmds[4].commandByte == 0xF5 && // gDPSetTile
                 cmds[5].commandByte == 0xF2)   // gDPSetTileSize
        {
            // TODO
            dc->listBgColor = RGB(255, 255, 200);
            dc->macroLength = 6;
            /*
            loadtexturetile or loadmultitile macro
            gDPLoadTextureTile
            gDPLoadMultiTile
            gDPLoadTextureTile_4b
            gDPLoadMultiTile_4b
            */
        }
        else if (cmds[0].commandByte == 0xE8 && // gDPTileSync
                 cmds[1].commandByte == 0xF5 && // gDPSetTile
                 cmds[2].commandByte == 0xE6 && // gDPLoadSync
                 cmds[3].commandByte == 0xF0 && // gDPLoadTLUTCmd
                 cmds[4].commandByte == 0xE7)   // gDPPipeSync
        {
            // todo ensure G_IM_FMT_RGBA, G_IM_SIZ_16b
            int tmem = cmds[1].settile.tmem;
            int count = cmds[3].loadtlut.count + 1;

            if (count == 256 && tmem == 256)
            {
                dc->listBgColor = RGB(255, 255, 200);
                dc->Rename("gsDPLoadTLUT_pal256");
                dc->params = stdstr_f("/*addr*/ 0x%08X /*0x%08X*/", cmd->address,
                    state->SegmentedToVirtual(cmd->address));

                //state->m_spCommandAddress += 8 * 5; // skip
                dc->macroLength = 5;
            }
            else if (count == 16 && tmem >= 256 && ((tmem-256)%16) == 0)
            {
                dc->listBgColor = RGB(255, 255, 200);
                int pal = (tmem - 256) / 16;
                dc->Rename("gDPLoadTLUT_pal16");
                dc->params = stdstr_f("/*pal*/ %d, /*addr*/ 0x%08X /*0x%08X*/", pal, cmd->address,
                    state->SegmentedToVirtual(cmd->address));

                //state->m_spCommandAddress += 8 * 5; // skip
                dc->macroLength = 5;
            }
            else
            {
                dc->listBgColor = RGB(255, 255, 200);
                dc->Rename("gsDPLoadTLUT");
                dc->params = stdstr_f("/*count*/  %d, /*tmem*/ 0x%03X, /*addr*/ 0x%08X /*0x%08X*/", count,
                    tmem, cmd->address,
                    state->SegmentedToVirtual(cmd->address));

                //state->m_spCommandAddress += 8 * 5; // skip
                dc->macroLength = 5;
            }
        }
    }
}

void CGfxOps::op_gsDPSetDepthImage(CHleGfxState* state, decoded_cmd_t* dc)
{
    dl_cmd_settimg_t* cmd = &state->m_spCommand.settimg;
    state->m_dpDepthImage = cmd->address; // todo other params

    const char* fmtName = "?";
    const char* sizName = "?";

    fmtName = CGfxLabels::ImageFormats[cmd->fmt];
    sizName = CGfxLabels::TexelSizes[cmd->siz];

    dc->params = stdstr_f("/*fmt*/ %s, /*siz*/ %s, /*w*/ %d, /*addr*/ 0x%08X /*0x%08X*/", fmtName, sizName, cmd->width + 1, cmd->address,
        state->SegmentedToVirtual(cmd->address));

    dc->SetDramResource(state, RES_DEPTHBUFFER);
}

void CGfxOps::op_gsDPSetColorImage(CHleGfxState* state, decoded_cmd_t* dc)
{
    dl_cmd_settimg_t* cmd = &state->m_spCommand.settimg;
    state->m_dpColorImage = cmd->address; // todo other params

    const char* fmtName = "?";
    const char* sizName = "?";

    fmtName = CGfxLabels::ImageFormats[cmd->fmt];
    sizName = CGfxLabels::TexelSizes[cmd->siz];

    dc->params = stdstr_f("/*fmt*/ %s, /*siz*/ %s, /*w*/ %d, /*addr*/ 0x%08X /*0x%08X*/", fmtName, sizName, cmd->width + 1, cmd->address,
        state->SegmentedToVirtual(cmd->address));

    dc->SetDramResource(state, RES_COLORBUFFER);
    dc->dramResource.imageWidth = 320;
    dc->dramResource.imageHeight = 240;
}

void CGfxOps::op_gsDPSetCombineLERP(CHleGfxState* state, decoded_cmd_t* dc)
{
    dl_cmd_setcombine_t* cmd = &state->m_spCommand.setcombine;
    state->m_dpCombiner = *cmd;

    //////////

    const char* cycle1Preset = NULL;
    const char* cycle2Preset = NULL;

    for (int i = 0; CGfxLabels::CombineModes[i].name != NULL; i++)
    {
        cc_preset_lut_entry_t* cc = &CGfxLabels::CombineModes[i];

        if (cmd->a0 == cc->a && cmd->b0 == cc->b && cmd->c0 == cc->c && cmd->d0 == cc->d &&
            cmd->Aa0 == cc->Aa && cmd->Ab0 == cc->Ab && cmd->Ac0 == cc->Ac && cmd->Ad0 == cc->Ad)
        {
            cycle1Preset = cc->name;
            break;
        }
    }

    for (int i = 0; CGfxLabels::CombineModes[i].name != NULL; i++)
    {
        cc_preset_lut_entry_t* cc = &CGfxLabels::CombineModes[i];

        if (cmd->a1 == cc->a && cmd->b1 == cc->b && cmd->c1 == cc->c && cmd->d1 == cc->d &&
            cmd->Aa1 == cc->Aa && cmd->Ab1 == cc->Ab && cmd->Ac1 == cc->Ac && cmd->Ad1 == cc->Ad)
        {
            cycle2Preset = cc->name;
            break;
        }
    }

    if (cycle1Preset != NULL && cycle2Preset != NULL)
    {
        dc->params = stdstr_f("%s, %s", cycle1Preset, cycle2Preset);
        dc->Rename("gsDPSetCombineMode");
        return;
    }

    stdstr c1 = stdstr_f("%s, %s, %s, %s, %s, %s, %s, %s",
        CGfxLabels::LookupName(CGfxLabels::CCMuxA, cmd->a0),
        CGfxLabels::LookupName(CGfxLabels::CCMuxB, cmd->b0),
        CGfxLabels::LookupName(CGfxLabels::CCMuxC, cmd->c0),
        CGfxLabels::LookupName(CGfxLabels::CCMuxD, cmd->d0),
        CGfxLabels::LookupName(CGfxLabels::ACMuxA_B_D, cmd->Aa0),
        CGfxLabels::LookupName(CGfxLabels::ACMuxA_B_D, cmd->Ab0),
        CGfxLabels::LookupName(CGfxLabels::ACMuxC, cmd->Ac0),
        CGfxLabels::LookupName(CGfxLabels::ACMuxA_B_D, cmd->Ad0));

    stdstr c2 = stdstr_f("%s, %s, %s, %s, %s, %s, %s, %s",
        CGfxLabels::LookupName(CGfxLabels::CCMuxA, cmd->a1),
        CGfxLabels::LookupName(CGfxLabels::CCMuxB, cmd->b1),
        CGfxLabels::LookupName(CGfxLabels::CCMuxC, cmd->c1),
        CGfxLabels::LookupName(CGfxLabels::CCMuxD, cmd->d1),
        CGfxLabels::LookupName(CGfxLabels::ACMuxA_B_D, cmd->Aa1),
        CGfxLabels::LookupName(CGfxLabels::ACMuxA_B_D, cmd->Ab1),
        CGfxLabels::LookupName(CGfxLabels::ACMuxC, cmd->Ac1),
        CGfxLabels::LookupName(CGfxLabels::ACMuxA_B_D, cmd->Ad1));

    dc->params = stdstr_f("%s, %s", c1.c_str(), c2.c_str());
}

void CGfxOps::op_gsSPSetGeometryMode_f3d(CHleGfxState* state, decoded_cmd_t* dc)
{
    dl_cmd_geometrymode_f3d_t* cmd = &state->m_spCommand.geometrymode_f3d;
    state->m_spGeometryMode.data |= cmd->mode;

    bool havePrev = false;
    dc->params = "";

    for (int i = 0; CGfxLabels::GeometryModes[i].name != NULL; i++)
    {
        if (state->m_spCommand.w1 & CGfxLabels::GeometryModes[i].value)
        {
            if (havePrev)
            {
                dc->params += " | ";
            }

            dc->params += CGfxLabels::GeometryModes[i].name;
            havePrev = true;
        }
    }
}

void CGfxOps::op_gsSPClearGeometryMode_f3d(CHleGfxState* state, decoded_cmd_t* dc)
{
    dl_cmd_geometrymode_f3d_t* cmd = &state->m_spCommand.geometrymode_f3d;
    state->m_spGeometryMode.data &= ~cmd->mode;

    bool havePrev = false;
    dc->params = "";

    for (int i = 0; CGfxLabels::GeometryModes[i].name != NULL; i++)
    {
        if (state->m_spCommand.w1 & CGfxLabels::GeometryModes[i].value)
        {
            if (havePrev)
            {
                dc->params += " | ";
            }

            dc->params += CGfxLabels::GeometryModes[i].name;
            havePrev = true;
        }
    }
}

void CGfxOps::op_gsSPGeometryMode_f3dex2(CHleGfxState* state, decoded_cmd_t* dc)
{
    dl_cmd_geometrymode_f3dex2_t* cmd = &state->m_spCommand.geometrymode_f3dex2;
    state->m_spGeometryMode.data &= ~cmd->c;
    state->m_spGeometryMode.data |= cmd->s;

    if (cmd->c == 0xFFFFFF)
    {
        dc->Rename("gsSPSetGeometryMode");
    }
    else if (cmd->s == 0)
    {
        dc->Rename("gsSPClearGeometryMode");
    }

    // TODO clear bits
    bool havePrev = false;
    dc->params = "";

    for (int i = 0; CGfxLabels::GeometryModes_F3DEX2[i].name != NULL; i++)
    {
        if (cmd->s & CGfxLabels::GeometryModes_F3DEX2[i].value)
        {
            if (havePrev)
            {
                dc->params += " | ";
            }

            dc->params += CGfxLabels::GeometryModes_F3DEX2[i].name;
            havePrev = true;
        }
    }
}

void CGfxOps::op_gsSPSetOtherMode_h_f3d(CHleGfxState* state, decoded_cmd_t* dc)
{
    dl_cmd_setothermode_h_t* cmd = &state->m_spCommand.setothermode_h;
    uint32_t mask = ~(((1 << cmd->len) - 1) << cmd->sft);
    state->m_dpOtherMode_h.data = (state->m_dpOtherMode_h.data & mask) | cmd->mode;

    othermode_h_t omh;
    omh.data = cmd->mode;

    int field = cmd->sft;

    switch (field)
    {
    case 4:  dc->params = stdstr_f(CGfxLabels::OtherMode_ad[omh.ad]); dc->Rename("gsDPSetAlphaDither"); break; // hw2 only
    case 6:  dc->params = stdstr_f(CGfxLabels::OtherMode_rd[omh.rd]); dc->Rename("gsDPSetColorDither"); break; // hw2 only
    case 8:  dc->params = stdstr_f(CGfxLabels::OtherMode_ck[omh.ck]); dc->Rename("gsDPSetCombineKey"); break;
    case 9:  dc->params = stdstr_f(CGfxLabels::OtherMode_tc[omh.tc]); dc->Rename("gsDPSetTextureConvert"); break;
    case 12: dc->params = stdstr_f(CGfxLabels::OtherMode_tf[omh.tf]); dc->Rename("gsDPSetTextureFilter"); break;
    case 14: dc->params = stdstr_f(CGfxLabels::OtherMode_tt[omh.tt]); dc->Rename("gsDPSetTextureLUT"); break;
    case 16: dc->params = stdstr_f(CGfxLabels::OtherMode_tl[omh.tl]); dc->Rename("gsDPSetTextureLOD"); break;
    case 17: dc->params = stdstr_f(CGfxLabels::OtherMode_td[omh.td]); dc->Rename("gsDPSetTextureDetail"); break;
    case 19: dc->params = stdstr_f(CGfxLabels::OtherMode_tp[omh.tp]); dc->Rename("gsDPSetTexturePersp"); break;
    case 20: dc->params = stdstr_f(CGfxLabels::OtherMode_cyc[omh.cyc]); dc->Rename("gsDPSetCycleType"); break;
    case 22: dc->params = stdstr_f(CGfxLabels::OtherMode_cd[omh.cd]); dc->Rename("gsDPSetColorDither"); break; // hw1 only
    case 23: dc->params = stdstr_f(CGfxLabels::OtherMode_pm[omh.pm]); dc->Rename("gsDPPipelineMode"); break;
    }
}

void CGfxOps::op_gsSPSetOtherMode_h_f3dex2(CHleGfxState* state, decoded_cmd_t* dc)
{
    dl_cmd_setothermode_h_t* cmd = &state->m_spCommand.setothermode_h;
    uint32_t mask = ~(((1 << cmd->len) - 1) << cmd->sft);
    state->m_dpOtherMode_h.data = (state->m_dpOtherMode_h.data & mask) | cmd->mode;

    othermode_h_t omh;
    omh.data = cmd->mode;

    int field = 32 - (cmd->sft + (cmd->len + 1));

    switch (field)
    {
    case 4:  dc->params = stdstr_f(CGfxLabels::OtherMode_ad[omh.ad]); dc->Rename("gsDPSetAlphaDither"); break; // hw2 only
    case 6:  dc->params = stdstr_f(CGfxLabels::OtherMode_rd[omh.rd]); dc->Rename("gsDPSetColorDither"); break; // hw2 only
    case 8:  dc->params = stdstr_f(CGfxLabels::OtherMode_ck[omh.ck]); dc->Rename("gsDPSetCombineKey"); break;
    case 9:  dc->params = stdstr_f(CGfxLabels::OtherMode_tc[omh.tc]); dc->Rename("gsDPSetTextureConvert"); break;
    case 12: dc->params = stdstr_f(CGfxLabels::OtherMode_tf[omh.tf]); dc->Rename("gsDPSetTextureFilter"); break;
    case 14: dc->params = stdstr_f(CGfxLabels::OtherMode_tt[omh.tt]); dc->Rename("gsDPSetTextureLUT"); break;
    case 16: dc->params = stdstr_f(CGfxLabels::OtherMode_tl[omh.tl]); dc->Rename("gsDPSetTextureLOD"); break;
    case 17: dc->params = stdstr_f(CGfxLabels::OtherMode_td[omh.td]); dc->Rename("gsDPSetTextureDetail"); break;
    case 19: dc->params = stdstr_f(CGfxLabels::OtherMode_tp[omh.tp]); dc->Rename("gsDPSetTexturePersp"); break;
    case 20: dc->params = stdstr_f(CGfxLabels::OtherMode_cyc[omh.cyc]); dc->Rename("gsDPSetCycleType"); break;
    case 22: dc->params = stdstr_f(CGfxLabels::OtherMode_cd[omh.cd]); dc->Rename("gsDPSetColorDither"); break; // hw1 only
    case 23: dc->params = stdstr_f(CGfxLabels::OtherMode_pm[omh.pm]); dc->Rename("gsDPPipelineMode"); break;
    }
}

void CGfxOps::op_gsSPSetOtherMode_l_f3d(CHleGfxState* state, decoded_cmd_t* dc)
{
    dl_cmd_setothermode_l_t* cmd = &state->m_spCommand.setothermode_l;
    othermode_l_t oml;
    oml.data = cmd->mode;

    int field = cmd->sft;

    if (field == 3)
    {
        const char* szPresetC1 = CGfxLabels::LookupName(CGfxLabels::RenderModesCycle1, oml.data & RM_C1_MASK);
        const char* szPresetC2 = CGfxLabels::LookupName(CGfxLabels::RenderModesCycle2, oml.data & RM_C2_MASK);

        if (szPresetC1 != NULL && szPresetC2 != NULL)
        {
            dc->params = stdstr_f("%s, %s", szPresetC1, szPresetC2);
        }

        dc->Rename("gsDPSetRenderMode");
        // todo fallback if no presets
        return;
    }

    if (field == 0)
    {
        //todo
        dc->Rename("gsDPSetAlphaCompare");
    }

    if (field == 2)
    {
        //todo
        dc->Rename("gsDPSetDepthSource");
    }
}

void CGfxOps::op_gsSPSetOtherMode_l_f3dex2(CHleGfxState* state, decoded_cmd_t* dc)
{
    dl_cmd_setothermode_l_t* cmd = &state->m_spCommand.setothermode_l;
    othermode_l_t oml;
    oml.data = cmd->mode;

    int field = 32 - (cmd->sft + (cmd->len + 1));

    if (field == 3)
    {
        const char* szPresetC1 = CGfxLabels::LookupName(CGfxLabels::RenderModesCycle1, oml.data & RM_C1_MASK);
        const char* szPresetC2 = CGfxLabels::LookupName(CGfxLabels::RenderModesCycle2, oml.data & RM_C2_MASK);

        if (szPresetC1 != NULL && szPresetC2 != NULL)
        {
            dc->params = stdstr_f("%s, %s", szPresetC1, szPresetC2);
        }

        dc->Rename("gsDPSetRenderMode");
        // todo fallback if no presets
        return;
    }

    if (field == 0)
    {
        //todo
        dc->Rename("gsDPSetAlphaCompare");
    }

    if (field == 2)
    {
        //todo
        dc->Rename("gsDPSetDepthSource");
    }
}

void CGfxOps::op_gsDPSetFillColor(CHleGfxState* state, decoded_cmd_t* dc)
{
    state->m_dpFillColor = state->m_spCommand.w1;
    dc->params = stdstr_f("0x%08X", state->m_spCommand.w1);
}

void CGfxOps::op_gsDPSetFogColor(CHleGfxState* state, decoded_cmd_t* dc)
{
    state->m_dpFogColor = state->m_spCommand.w1;
    dc->params = stdstr_f("0x%08X", state->m_spCommand.w1);
}

void CGfxOps::op_gsDPSetBlendColor(CHleGfxState* state, decoded_cmd_t* dc)
{
    state->m_dpBlendColor = state->m_spCommand.w1;
    dc->params = stdstr_f("0x%08X", state->m_spCommand.w1);
}

void CGfxOps::op_gsDPSetPrimColor(CHleGfxState* state, decoded_cmd_t* dc)
{
    state->m_dpPrimColor = state->m_spCommand.w1;
    dc->params = stdstr_f("0x%08X", state->m_spCommand.w1);
}

void CGfxOps::op_gsDPSetEnvColor(CHleGfxState* state, decoded_cmd_t* dc)
{
    state->m_dpEnvColor = state->m_spCommand.w1;
    dc->params = stdstr_f("0x%08X", state->m_spCommand.w1);
}

void CGfxOps::op_gsDPLoadTLUTCmd(CHleGfxState* state, decoded_cmd_t* dc)
{
    dl_cmd_loadtlut_t* cmd = &state->m_spCommand.loadtlut;
    dc->params = stdstr_f("/*tile*/ %d, /*count*/ %d", cmd->tile, cmd->count);
}

void CGfxOps::op_gsDPSetTileSize(CHleGfxState* state, decoded_cmd_t* dc)
{
    dl_cmd_settilesize_t* cmd = &state->m_spCommand.settilesize;

    if ((cmd->lrs & 3) == 0 && (cmd->lrt & 3) == 0)
    {
        dc->params = stdstr_f("/*tile*/ %d, /*uls*/ %d, /*ult*/ %d, /*lrs*/ (%d << 2), /*lrt*/ (%d << 2)", cmd->tile, cmd->uls, cmd->ult, cmd->lrs >> 2, cmd->lrt >> 2);
    }

    dc->params = stdstr_f("/*tile*/ %d, /*uls*/ %d, /*ult*/ %d, /*lrs*/ %d, /*lrt*/ %d", cmd->tile, cmd->uls, cmd->ult, cmd->lrs, cmd->lrt);
}

void CGfxOps::op_gsSPMoveMem_f3d(CHleGfxState* state, decoded_cmd_t* dc)
{
    dl_cmd_movemem_f3d_t* cmd = &state->m_spCommand.movemem_f3d;

    if (cmd->p == 0x80) // G_MV_VIEWPORT
    {
        dc->Rename("gsSPViewport");
        // todo params
        dc->params = stdstr_f("/*addr*/ 0x%08X /*0x%08X*/", cmd->address,
            state->SegmentedToVirtual(cmd->address));

        dc->SetDramResource(state, RES_VIEWPORT);
    }
    else if (cmd->p >= 0x86 && cmd->p <= 0x94) // G_MV_L0:7
    {
        dc->Rename("gsSPLight");

        int lightNumber = (cmd->p - 0x86) / 2;
        dc->params = stdstr_f("/*addr*/ 0x%08X, /*lightnum*/ %d /*0x%08X*/", cmd->address, lightNumber,
            state->SegmentedToVirtual(cmd->address));

        dc->SetDramResource(state, RES_AMBIENT_LIGHT);
    }

    dc->listFgColor = COLOR_DMA;
}

void CGfxOps::op_gsSPMoveMem_f3dex2(CHleGfxState* state, decoded_cmd_t* dc)
{
    dl_cmd_movemem_f3dex2_t* cmd = &state->m_spCommand.movemem_f3dex2;

	dc->params = stdstr_f("/*addr*/ 0x%08X /*0x%08X*/", cmd->address,
		state->SegmentedToVirtual(cmd->address));

    if (cmd->i == 0x08) // G_MV_VIEWPORT
    {
        dc->Rename("gsSPViewport");
    }
    else if (cmd->i == 0x0A) // G_MV_LIGHT
    {
        int offs = cmd->o / 3;

        if (offs == 0)
        {
            dc->Rename("gsSPLookAtX");
        }
        else if (offs == 1)
        {
            dc->Rename("gsSPLookAtY");
        }
        else if(offs >= 2 && offs <= 10)
        {
            dc->Rename("gsSPLight");
			dc->params = stdstr_f("/*addr*/ 0x%08X, LIGHT_%d /*0x%08X*/", cmd->address, (offs-2),
				state->SegmentedToVirtual(cmd->address));
        }
    }
    else if (cmd->i == 0x0E) // G_MV_MATRIX
    {
        dc->Rename("gsSPForceMatrix");
    }
}

void CGfxOps::op_gsDPLoadBlock(CHleGfxState* state, decoded_cmd_t* dc)
{
    dl_cmd_loadblock_t* cmd = &state->m_spCommand.loadblock;
    dc->params = stdstr_f("/*tile*/ %d, /*uls*/ %d, /*ult*/ %d, /*lrs*/ %d, /*dxt*/ %d",
        cmd->tile, cmd->uls, cmd->ult, cmd->lrs, cmd->dxt);

    //if (cmd->uls != 0 || cmd->ult != 0)
    //{
    //    MessageBox(NULL, "nonzero uls/ult", "", MB_OK);
    //}

    uint8_t siz = (uint8_t)state->m_dpTextureImageSiz;

    int width, height;

    int numTexelsToLoad = cmd->lrs + 1;
    int bytesPerTexel = (1 << siz) / 2;

    int bytesPerLine = (int)ceil((2048.0f / cmd->dxt) * 8);

    if (bytesPerTexel != 0)
    {
        width = bytesPerLine / bytesPerTexel;
    }
    else
    {
        width = bytesPerLine * 2; // I4, two texels per byte
    }

    height = numTexelsToLoad / width;

    dc->dramResource.type = RES_TEXTURE;
    dc->dramResource.address = state->m_dpTextureImage;
    dc->dramResource.virtAddress = state->SegmentedToVirtual(state->m_dpTextureImage);
    dc->dramResource.imageWidth = width;
    dc->dramResource.imageHeight = height;
    dc->dramResource.imageSiz = state->m_dpTextureImageSiz;
    dc->dramResource.imageFmt = state->m_dpTextureImageFmt;

    dc->listFgColor = COLOR_DMA;
}

void CGfxOps::op_gsDPLoadTile(CHleGfxState* state, decoded_cmd_t* dc)
{
    dl_cmd_loadtile_t* cmd = &state->m_spCommand.loadtile;
    dc->params = stdstr_f("/*tile*/ %d, /*uls*/ %d, /*ult*/ %d, /*lrs*/ %d, /*lrt*/ %d",
        cmd->tile, cmd->uls, cmd->ult, cmd->lrs, cmd->lrt);
    //todo
}

void CGfxOps::op_gsDPSetScissor(CHleGfxState* state, decoded_cmd_t* dc)
{
    dl_cmd_setscissor_t* cmd = &state->m_spCommand.setscissor;

    const char* szMode = "?";

    switch (cmd->mode)
    {
    case 0: szMode = "G_SC_NON_INTERLACE"; break;
    case 2: szMode = "G_SC_EVEN_INTERLACE"; break;
    case 3: szMode = "G_SC_ODD_INTERLACE"; break;
    }

    // TODO check for frac bits and use gsDPSetScissorFrac if set

    dc->params = stdstr_f("/*mode*/ %s, /*ulx*/ %d, /*uly*/ %d, /*lrx*/ %d, /*lry*/ %d",
        szMode, cmd->ulx >> 2, cmd->uly >> 2, cmd->lrx >> 2, cmd->lry >> 2);
}

void CGfxOps::op_gsDPFillRectangle(CHleGfxState* state, decoded_cmd_t* dc)
{
    dl_cmd_fillrect_t* cmd = &state->m_spCommand.fillrect;
    // note: ignoring frac bits
    dc->params = stdstr_f("/*ulx*/ (%d << 2), /*uly*/ (%d << 2), /*lrx*/ (%d << 2), /*lry*/ (%d << 2)", cmd->ulx >> 2, cmd->uly >> 2, cmd->lrx >> 2, cmd->lry >> 2);

    dc->listFgColor = COLOR_PRIMITIVE;
}

void CGfxOps::op_gsSPMatrix_f3d(CHleGfxState* state, decoded_cmd_t* dc)
{
    dl_cmd_mtx_f3d_t* cmd = &state->m_spCommand.mtx_f3d;

    dc->params = stdstr_f("/*addr*/ 0x%08X, (%s | %s | %s) /*0x%08X*/", cmd->address,
        (cmd->params & 1) ? "G_MTX_PROJECTION" : "G_MTX_MODELVIEW",
        (cmd->params & 2) ? "G_MTX_LOAD" : "G_MTX_MUL",
        (cmd->params & 4) ? "G_MTX_PUSH" : "G_MTX_NOPUSH",
        state->SegmentedToVirtual(cmd->address));

    dc->SetDramResource(state, (cmd->params & 1) ? RES_PROJECTION_MATRIX : RES_MODELVIEW_MATRIX);
    dc->listFgColor = COLOR_DMA;
}

void CGfxOps::op_gsSPMatrix_f3dex2(CHleGfxState* state, decoded_cmd_t* dc)
{
    dl_cmd_mtx_f3dex2_t* cmd = &state->m_spCommand.mtx_f3dex2;

    dc->params = stdstr_f("/*addr*/ 0x%08X, (%s | %s | %s) /*0x%08X*/", cmd->address,
        (cmd->params & 1) ? "G_MTX_PROJECTION" : "G_MTX_MODELVIEW",
        (cmd->params & 2) ? "G_MTX_LOAD" : "G_MTX_MUL",
        (cmd->params & 4) ? "G_MTX_PUSH" : "G_MTX_NOPUSH",
        state->SegmentedToVirtual(cmd->address));

    dc->SetDramResource(state, (cmd->params & 1) ? RES_PROJECTION_MATRIX : RES_MODELVIEW_MATRIX);
    dc->listFgColor = COLOR_DMA;
}