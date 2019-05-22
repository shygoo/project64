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
    { 0xFC, "gsDPSetCombineLERP",    op_gsDPSetCombineLERP, dec_gsDPSetCombineLERP },
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
    { 0x04, "gsSPVertex",            op_gsSPVertex_f3d, dec_gsSPVertex_f3d },
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
    { 0xB9, "gsSPSetOtherMode_l",    NULL, dec_gsSPSetOtherMode_l },
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

name_lut_entry_t CDisplayListParser::TexelSizeShortNames[] = {
	{ 0, "4b" },
	{ 1, "8b" },
	{ 2, "16b" },
	{ 3, "32b" },
	{ 0, NULL }
};

name_lut_entry_t CDisplayListParser::ImageFormatShortNames[] = {
	{ 0, "RGBA" },
	{ 1, "YUV" },
	{ 2, "CI" },
	{ 3, "IA" },
	{ 4, "I" },
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

name_lut_entry_t CDisplayListParser::RenderModePresetNamesCycle1[] = {
    { 0x00407248, "G_RM_AA_DEC_LINE" },
    { 0x00442048, "G_RM_AA_OPA_SURF" },
    { 0x00402048, "G_RM_AA_OPA_TERR" },
    { 0x0040004B, "G_RM_AA_PCL_SURF" },
    { 0x00442248, "G_RM_AA_SUB_SURF" },
    { 0x00402248, "G_RM_AA_SUB_TERR" },
    { 0x00443048, "G_RM_AA_TEX_EDGE" },
    { 0x00403048, "G_RM_AA_TEX_TERR" },
    { 0x00407048, "G_RM_AA_XLU_LINE" },
    { 0x004041C8, "G_RM_AA_XLU_SURF" },
    { 0x00407F58, "G_RM_AA_ZB_DEC_LINE" },
    { 0x00442D58, "G_RM_AA_ZB_OPA_DECAL" },
    { 0x00442478, "G_RM_AA_ZB_OPA_INTER" },
    { 0x00442078, "G_RM_AA_ZB_OPA_SURF" },
    { 0x00402078, "G_RM_AA_ZB_OPA_TERR" },
    { 0x0040007B, "G_RM_AA_ZB_PCL_SURF" },
    { 0x00442278, "G_RM_AA_ZB_SUB_SURF" },
    { 0x00402278, "G_RM_AA_ZB_SUB_TERR" },
    { 0x00443078, "G_RM_AA_ZB_TEX_EDGE" },
    { 0x00443478, "G_RM_AA_ZB_TEX_INTER" },
    { 0x00403078, "G_RM_AA_ZB_TEX_TERR" },
    { 0x00404DD8, "G_RM_AA_ZB_XLU_DECAL" },
    { 0x004045D8, "G_RM_AA_ZB_XLU_INTER" },
    { 0x00407858, "G_RM_AA_ZB_XLU_LINE" },
    { 0x004049D8, "G_RM_AA_ZB_XLU_SURF" },
    { 0x04484340, "G_RM_ADD" },
    { 0x00404340, "G_RM_CLD_SURF" },
    { 0x00000000, "G_RM_NOOP" },
    { 0x0C080000, "G_RM_OPA_CI" },
    { 0x0C084000, "G_RM_OPA_SURF" },
    { 0x0C084203, "G_RM_PCL_SURF" },
    { 0x00442008, "G_RM_RA_OPA_SURF" },
    { 0x00442D18, "G_RM_RA_ZB_OPA_DECAL" },
    { 0x00442438, "G_RM_RA_ZB_OPA_INTER" },
    { 0x00442038, "G_RM_RA_ZB_OPA_SURF" },
    { 0x0C087008, "G_RM_TEX_EDGE" },
    { 0x0C844040, "G_RM_VISCVG" },
    { 0x00404240, "G_RM_XLU_SURF" },
    { 0x00404B50, "G_RM_ZB_CLD_SURF" },
    { 0x00442E10, "G_RM_ZB_OPA_DECAL" },
    { 0x00442230, "G_RM_ZB_OPA_SURF" },
    { 0x00404F50, "G_RM_ZB_OVL_SURF" },
    { 0x0C080233, "G_RM_ZB_PCL_SURF" },
    { 0x00404E50, "G_RM_ZB_XLU_DECAL" },
    { 0x00404A50, "G_RM_ZB_XLU_SURF" },
    // messed up?
    //{ 0xC8000000, "G_RM_FOG_SHADE_A" },
    //{ 0xC4000000, "G_RM_FOG_PRIM_A" },
    //{ 0x0C080000, "G_RM_PASS" },
    { 0, NULL }
};

name_lut_entry_t CDisplayListParser::RenderModePresetNamesCycle2[] = {
    { 0x00107248, "G_RM_AA_DEC_LINE2" },
    { 0x00112048, "G_RM_AA_OPA_SURF2" },
    { 0x00102048, "G_RM_AA_OPA_TERR2" },
    { 0x0010004B, "G_RM_AA_PCL_SURF2" },
    { 0x00112248, "G_RM_AA_SUB_SURF2" },
    { 0x00102248, "G_RM_AA_SUB_TERR2" },
    { 0x00113048, "G_RM_AA_TEX_EDGE2" },
    { 0x00103048, "G_RM_AA_TEX_TERR2" },
    { 0x00107048, "G_RM_AA_XLU_LINE2" },
    { 0x001041C8, "G_RM_AA_XLU_SURF2" },
    { 0x00107F58, "G_RM_AA_ZB_DEC_LINE2" },
    { 0x00112D58, "G_RM_AA_ZB_OPA_DECAL2" },
    { 0x00112478, "G_RM_AA_ZB_OPA_INTER2" },
    { 0x00112078, "G_RM_AA_ZB_OPA_SURF2" },
    { 0x00102078, "G_RM_AA_ZB_OPA_TERR2" },
    { 0x0010007B, "G_RM_AA_ZB_PCL_SURF2" },
    { 0x00112278, "G_RM_AA_ZB_SUB_SURF2" },
    { 0x00102278, "G_RM_AA_ZB_SUB_TERR2" },
    { 0x00113078, "G_RM_AA_ZB_TEX_EDGE2" },
    { 0x00113478, "G_RM_AA_ZB_TEX_INTER2" },
    { 0x00103078, "G_RM_AA_ZB_TEX_TERR2" },
    { 0x00104DD8, "G_RM_AA_ZB_XLU_DECAL2" },
    { 0x001045D8, "G_RM_AA_ZB_XLU_INTER2" },
    { 0x00107858, "G_RM_AA_ZB_XLU_LINE2" },
    { 0x001049D8, "G_RM_AA_ZB_XLU_SURF2" },
    { 0x01124340, "G_RM_ADD2" },
    { 0x00104340, "G_RM_CLD_SURF2" },
    { 0x00000000, "G_RM_NOOP2" },
    { 0x03020000, "G_RM_OPA_CI2" },
    { 0x03024000, "G_RM_OPA_SURF2" },
    { 0x03024203, "G_RM_PCL_SURF2" },
    { 0x00112008, "G_RM_RA_OPA_SURF2" },
    { 0x00112D18, "G_RM_RA_ZB_OPA_DECAL2" },
    { 0x00112438, "G_RM_RA_ZB_OPA_INTER2" },
    { 0x00112038, "G_RM_RA_ZB_OPA_SURF2" },
    { 0x03027008, "G_RM_TEX_EDGE2" },
    { 0x03214040, "G_RM_VISCVG2" },
    { 0x00104240, "G_RM_XLU_SURF2" },
    { 0x00104B50, "G_RM_ZB_CLD_SURF2" },
    { 0x00112E10, "G_RM_ZB_OPA_DECAL2" },
    { 0x00112230, "G_RM_ZB_OPA_SURF2" },
    { 0x00104F50, "G_RM_ZB_OVL_SURF2" },
    { 0x03020233, "G_RM_ZB_PCL_SURF2" },
    { 0x00104E50, "G_RM_ZB_XLU_DECAL2" },
    { 0x00104A50, "G_RM_ZB_XLU_SURF2" },
    { 0, NULL }
};

// TODO clean this up
cc_preset_lut_entry_t CDisplayListParser::CombinerPresetNames[] = {
    { "G_CC_PRIMITIVE",             15, 15, 31,  3, 7, 7, 7, 3 },
    { "G_CC_SHADE",                 15, 15, 31,  4, 7, 7, 7, 4 },
    { "G_CC_MODULATEI",             1,  15,  4,  7, 7, 7, 7, 4 },
    { "G_CC_MODULATEIA",            1,  15,  4,  7, 1, 7, 4, 7 },
    { "G_CC_MODULATEIDECALA",       1,  15,  4,  7, 7, 7, 7, 1 },
    { "G_CC_MODULATEI_PRIM",        1,  15,  3,  7, 7, 7, 7, 3 },
    { "G_CC_MODULATEIA_PRIM",       1,  15,  3,  7, 1, 7, 3, 7 },
    { "G_CC_MODULATEIDECALA_PRIM",  1,  15,  3,  7, 7, 7, 7, 1 },
    { "G_CC_DECALRGB",              15, 15, 31,  1, 7, 7, 7, 4 },
    { "G_CC_DECALRGBA",             15, 15, 31,  1, 7, 7, 7, 1 },
    { "G_CC_BLENDI",                5,   4,  1,  4, 7, 7, 7, 4 },
    { "G_CC_BLENDIA",               5,   4,  1,  4, 1, 7, 4, 7 },
    { "G_CC_BLENDIDECALA",          5,   4,  1,  4, 7, 7, 7, 1 },
    { "G_CC_BLENDRGBA",             1,   4,  8,  4, 7, 7, 7, 4 },
    { "G_CC_BLENDRGBDECALA",        1,   4,  8,  4, 7, 7, 7, 1 },
    { "G_CC_ADDRGB",                6,  15,  1,  4, 7, 7, 7, 4 },
    { "G_CC_ADDRGBDECALA",          6,  15,  1,  4, 7, 7, 7, 1 },
    { "G_CC_REFLECTRGB",            5,  15,  1,  4, 7, 7, 7, 4 },
    { "G_CC_REFLECTRGBDECALA",      5,  15,  1,  4, 7, 7, 7, 1 },
    { "G_CC_HILITERGB",             3,   4,  1,  4, 7, 7, 7, 4 },
    { "G_CC_HILITERGBA",            3,   4,  1,  4, 3, 4, 1, 4 },
    { "G_CC_HILITERGBDECALA",       3,   4,  1,  4, 7, 7, 7, 1 },
    { "G_CC_SHADEDECALA",           15, 15,  31, 4, 7, 7, 7, 1 },
    { "G_CC_BLENDPE",               3,   5,  1,  5, 1, 7, 4, 7 },
    { "G_CC_BLENDPEDECALA",         3,   5,  1,  5, 7, 7, 7, 1 },
    { "_G_CC_BLENDPE",              5,   3,  1,  3, 1, 7, 4, 7 },
    { "_G_CC_BLENDPEDECALA",        5,   3,  1,  3, 7, 7, 7, 1 },
    { "_G_CC_TWOCOLORTEX",          3,   4,  1,  4, 7, 7, 7, 4 },
    { "_G_CC_SPARSEST",             3,   1, 13,  1, 3, 1, 0, 1 },
    { "G_CC_TEMPLERP",              2,   1, 14,  1, 2, 1, 6, 1 },
    { "G_CC_TRILERP",               2,   1, 13,  1, 2, 1, 0, 1 },
    { "G_CC_INTERFERENCE",          1,  15,  2,  7, 1, 7, 2, 7 },
    { "G_CC_1CYUV2RGB",             1,   7, 15,  1, 7, 7, 7, 4 },
    { "G_CC_YUV2RGB",               2,   7, 15,  2, 7, 7, 7, 7 },
    { "G_CC_PASS2",                 15, 15, 31,  0, 7, 7, 7, 0 },
    { "G_CC_MODULATEI2",            0,  15,  4,  7, 7, 7, 7, 4 },
    { "G_CC_MODULATEIA2",           0,   5,  4,  7, 0, 7, 4, 7 },
    { "G_CC_MODULATEI_PRIM2",       0,  15,  3,  7, 7, 7, 7, 3 },
    { "G_CC_MODULATEIA_PRIM2",      0,  15,  3,  7, 0, 7, 3, 7 },
    { "G_CC_DECALRGB2",             15, 15, 31,  0, 7, 7, 7, 4 },
    { "G_CC_BLENDI2",               5,   4,  0,  4, 7, 7, 7, 4 },
    { "G_CC_BLENDIA2",              5,   4,  0,  4, 0, 7, 4, 7 },
    { "G_CC_CHROMA_KEY2",           1,   6,  6,  7, 7, 7, 7, 7 },
    { "G_CC_HILITERGB2",            5,   0,  1,  0, 7, 7, 7, 4 },
    { "G_CC_HILITERGBA2",           5,   0,  1,  0, 5, 0, 1, 0 },
    { "G_CC_HILITERGBDECALA2",      5,   0,  1,  0, 7, 7, 7, 1 },
    { "G_CC_HILITERGBPASSA2",       5,   0,  1,  0, 7, 7, 7, 0 },
    { NULL, 0, 0, 0, 0, 0, 0, 0, 0 }
};

name_lut_entry_t CDisplayListParser::CCMuxA[] = {
    { 0,  "COMBINED" },{ 1,  "TEXEL0" },{ 2,  "TEXEL1" },{ 3,  "PRIMITIVE" },
    { 4,  "SHADE" },{ 5,  "ENVIRONMENT" },{ 6,  "ONE" },{ 7,  "NOISE" },
    { 15, "ZERO" },
    { 0 , NULL },
};

name_lut_entry_t CDisplayListParser::CCMuxB[] = {
    { 0,  "COMBINED" },{ 1,  "TEXEL0" },{ 2,  "TEXEL1" },{ 3,  "PRIMITIVE" },
    { 4,  "SHADE" },{ 5,  "ENVIRONMENT" },{ 6,  "CENTER" },{ 7,  "K4" },
    { 15, "ZERO" },
    { 0 , NULL }
};

name_lut_entry_t CDisplayListParser::CCMuxC[] = {
    { 0,  "COMBINED" },{ 1,  "TEXEL0" },{ 2,  "TEXEL1" },{ 3,  "PRIMITIVE" },
    { 4,  "SHADE" },{ 5, "ENVIRONMENT" },{ 6, "SCALE" },{ 7,  "COMBINED_ALPHA" },
    { 8,  "TEXEL0_ALPHA" },{ 9,  "TEXEL1_ALPHA" },{ 10, "PRIMITIVE_ALPHA" },
    { 11, "SHADE_ALPHA" },{ 12, "ENV_ALPHA" },{ 13, "LOD_FRACTION" },
    { 14, "PRIM_LOD_FRAC" },{ 15, "K5" },
    { 31, "ZERO" },
    { 0 , NULL }
};

name_lut_entry_t CDisplayListParser::CCMuxD[] = {
    { 0,  "COMBINED" },{ 1,  "TEXEL0" },{ 2,  "TEXEL1" },{ 3,  "PRIMITIVE" },
    { 4,  "SHADE" },{ 5,  "ENVIRONMENT" },{ 6,  "ONE" },
    { 7,  "ZERO" },
    { 0 , NULL }
};

name_lut_entry_t CDisplayListParser::ACMuxA_B_D[] = {
    { 0, "COMBINED" },{ 1,  "TEXEL0" },{ 2, "TEXEL1" },{ 3,  "PRIMITIVE" },
    { 4, "SHADE" },{ 5,  "ENVIRONMENT" },{ 6,  "ONE" },
    { 7, "ZERO" },
    { 0 , NULL }
};

name_lut_entry_t CDisplayListParser::ACMuxC[] = {
    { 0,  "LOD_FRACTION" },{ 1,  "TEXEL0" },{ 2,  "TEXEL1" },
    { 3,  "PRIMITIVE" },{ 4,  "SHADE" },{ 5,  "ENVIRONMENT" },{ 6, "PRIM_LOD_FRAC" },
    { 7,  "ZERO" },
    { 0 , NULL }
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