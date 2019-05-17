#include <stdafx.h>

#include "DisplayListParser.h"
#include "DisplayListOps.h"
#include "DisplayListDecode.h"

ucode_version_info_t CDisplayListParser::UCodeVersions[] = {
    { 0x3A1CBAC3, UCODE_F3D,    "Fast3D", Commands_F3D },
    { 0x8805FFEA, UCODE_F3DEX,  "F3DEX",  Commands_F3DEX },
	{ 0xBC45382E, UCODE_F3DEX2, "F3DEX2", Commands_F3DEX2 }, // kirby
	{ 0x5D3099F1, UCODE_F3DEX2, "F3DEX2", Commands_F3DEX2 }, // zelda
    { 0, UCODE_UNKNOWN, NULL, NULL }
};

////////////////////////////////

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
    { 0xF7, "gsDPSetFillColor",      op_gsDPSetFillColor, dec_HexParam32 },
    { 0xF8, "gsDPSetFogColor",       op_gsDPSetFogColor, dec_HexParam32 },
    { 0xF9, "gsDPSetBlendColor",     op_gsDPSetBlendColor, dec_HexParam32 },
    { 0xFA, "gsDPSetPrimColor",      op_gsDPSetPrimColor, dec_HexParam32 },
    { 0xFB, "gsDPSetEnvColor",       op_gsDPSetEnvColor, dec_HexParam32 },
    { 0xFC, "gsDPSetCombine",        NULL, NULL },
    { 0xFD, "gsDPSetTextureImage",   op_gsDPSetTextureImage, dec_gsDPSetTextureImage },
    { 0xFE, "gsDPSetDepthImage",     op_gsDPSetDepthImage, dec_gsDPSetDepthImage },
    { 0xFF, "gsDPSetColorImage",     op_gsDPSetColorImage, dec_gsDPSetColorImage },
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
    { 0xB6, "gsSPClearGeometryMode", op_gsSPClearGeometryMode_f3d, dec_gsSPClearGeometryMode_f3d },
    { 0xB7, "gsSPSetGeometryMode",   op_gsSPSetGeometryMode_f3d, dec_gsSPSetGeometryMode_f3d },
    { 0xB8, "gsSPEndDisplayList",    op_gsSPEndDisplayList, dec_NoParams },
    { 0xB9, "gsSPSetOtherMode_l",    NULL, NULL },
    { 0xBA, "gsSPSetOtherMode_h",    op_gsSPSetOtherMode_h, dec_gsSPSetOtherMode_h },
    { 0xBB, "gsSPTexture",           op_gsSPTexture_f3d, dec_gsSPTexture_f3d },
    { 0xBC, "gsSPMoveWord",          op_gsSPMoveWord_f3d, dec_gsSPMoveWord_f3d },
    { 0xBD, "gsSPPopMatrix",         NULL, NULL },
    { 0xBE, "gsSPCullDisplayList",   NULL, NULL },
    { 0xBF, "gsSP1Triangle",         NULL, dec_gsSP1Triangle_f3d },
    { 0, NULL, NULL, NULL }
};

dl_cmd_info_t CDisplayListParser::Commands_F3DEX[] = {
    { 0x06, "gsSPDisplayList",    op_gsSPDisplayList,    dec_gsSPDisplayList },
    { 0xB8, "gsSPEndDisplayList", op_gsSPEndDisplayList, dec_NoParams },
    { 0xBC, "gsSPMoveWord",       op_gsSPMoveWord_f3d,   dec_gsSPMoveWord_f3d },
    { 0, NULL, NULL, NULL }
};

dl_cmd_info_t CDisplayListParser::Commands_F3DEX2[] = {
    { 0xDB, "gsSPMoveWord",       op_gsSPMoveWord_f3dex2, dec_gsSPMoveWord_f3dex2 },
    { 0xDE, "gsSPDisplayList",    op_gsSPDisplayList,     dec_gsSPDisplayList },
    { 0xDF, "gsSPEndDisplayList", op_gsSPEndDisplayList,  dec_NoParams },
    { 0, NULL, NULL, NULL }
};

////////////////////////////////

name_lut_entry_t CDisplayListParser::TexelSizeNames[] = {
   { 0, "G_IM_SIZ_4b" },
   { 1, "G_IM_SIZ_8b" },
   { 2, "G_IM_SIZ_16b" },
   { 3, "G_IM_SIZ_32b" },
   { 0, NULL }
};

name_lut_entry_t CDisplayListParser::ImageFormatNames[] = {
    { 0, "G_IM_FMT_RGBA" },
    { 1, "G_IM_FMT_YUV" },
    { 2, "G_IM_FMT_CI" },
    { 3, "G_IM_FMT_IA" },
    { 4, "G_IM_FMT_I" },
    { 0, NULL }
};

name_lut_entry_t CDisplayListParser::GeometryModeNames[] = {
    { 0x00000001, "G_ZBUFFER" },
    { 0x00000004, "G_SHADE" },
    { 0x00001000, "G_CULL_FRONT" },
    { 0x00002000, "G_CULL_BACK" },
    { 0x00010000, "G_FOG" },
    { 0x00020000, "G_LIGHTING" },
    { 0x00040000, "G_TEXTURE_GEN" },
    { 0x00080000, "G_TEXTURE_GEN_LINEAR" },
    { 0x00000200, "G_SHADING_SMOOTH" },
    { 0x00100000, "G_LOD" },
    { 0x00800000, "G_CLIPPING" },
    { 0, NULL }
};

name_lut_entry_t CDisplayListParser::GeometryModeNames_F3DEX2[] = {
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

const char* OtherModeNames::ad[] = { "G_AD_PATTERN", "G_AD_NOTPATTERN", "G_AD_NOISE", "G_AD_DISABLE" };
const char* OtherModeNames::rd[] = { "G_CD_MAGICSQ", "G_CD_BAYER", "G_CD_NOISE" };
const char* OtherModeNames::ck[] = { "G_CK_NONE", "G_CK_KEY" };
const char* OtherModeNames::tc[] = { "G_TC_CONV", "1","2","3", "G_TC_INVALID_4", "G_TC_FILTCONV", "G_TC_FILT" };
const char* OtherModeNames::tf[] = { "G_TF_POINT", "1", "G_TF_BILERP", "G_TF_AVERAGE" };
const char* OtherModeNames::tt[] = { "G_TT_NONE", "1", "G_TT_RGBA16", "G_TT_IA16" };
const char* OtherModeNames::tl[] = { "G_TL_TILE", "G_TL_LOD" };
const char* OtherModeNames::td[] = { "G_TD_CLAMP", "G_TD_SHARPEN", "G_TD_DETAIL" };
const char* OtherModeNames::tp[] = { "G_TP_NONE", "G_TP_PERSP" };
const char* OtherModeNames::cyc[] = { "G_CYC_1CYCLE", "G_CYC_2CYCLE", "G_CYC_COPY", "G_CYC_FILL" };
const char* OtherModeNames::cd[] = { "0", "1", "G_CD_DISABLE", "G_CD_ENABLE" };
const char* OtherModeNames::pm[] = { "G_PM_NPRIMITIVE", "G_PM_1PRIMITIVE" };