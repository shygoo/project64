#include <stdafx.h>
#include "GfxLabels.h"

const char* CGfxLabels::LookupName(gfx_label_t* set, uint32_t value)
{
	for (int i = 0; set[i].name != NULL; i++)
	{
		if (set[i].value == value)
		{
			return set[i].name;
		}
	}
	return NULL;
}

const char* CGfxLabels::TexelSizes[] = { "G_IM_SIZ_4b", "G_IM_SIZ_8b", "G_IM_SIZ_16b", "G_IM_SIZ_32b" };
const char* CGfxLabels::ImageFormats[] = { "G_IM_FMT_RGBA", "G_IM_FMT_YUV", "G_IM_FMT_CI", "G_IM_FMT_IA", "G_IM_FMT_I" };
const char* CGfxLabels::TexelSizesShort[] = { "4b", "8b", "16b", "32b" };
const char* CGfxLabels::ImageFormatsShort[] = { "RGBA", "YUV", "CI", "IA", "I" };
const char* CGfxLabels::OtherMode_ad[] = { "G_AD_PATTERN", "G_AD_NOTPATTERN", "G_AD_NOISE", "G_AD_DISABLE" };
const char* CGfxLabels::OtherMode_rd[] = { "G_CD_MAGICSQ", "G_CD_BAYER", "G_CD_NOISE" };
const char* CGfxLabels::OtherMode_ck[] = { "G_CK_NONE", "G_CK_KEY" };
const char* CGfxLabels::OtherMode_tc[] = { "G_TC_CONV", "1", "2", "3", "G_TC_INVALID_4", "G_TC_FILTCONV", "G_TC_FILT" };
const char* CGfxLabels::OtherMode_tf[] = { "G_TF_POINT", "1", "G_TF_BILERP", "G_TF_AVERAGE" };
const char* CGfxLabels::OtherMode_tt[] = { "G_TT_NONE", "1", "G_TT_RGBA16", "G_TT_IA16" };
const char* CGfxLabels::OtherMode_tl[] = { "G_TL_TILE", "G_TL_LOD" };
const char* CGfxLabels::OtherMode_td[] = { "G_TD_CLAMP", "G_TD_SHARPEN", "G_TD_DETAIL" };
const char* CGfxLabels::OtherMode_tp[] = { "G_TP_NONE", "G_TP_PERSP" };
const char* CGfxLabels::OtherMode_cyc[] = { "G_CYC_1CYCLE", "G_CYC_2CYCLE", "G_CYC_COPY", "G_CYC_FILL" };
const char* CGfxLabels::OtherMode_cd[] = { "0", "1", "G_CD_DISABLE", "G_CD_ENABLE" };
const char* CGfxLabels::OtherMode_pm[] = { "G_PM_NPRIMITIVE", "G_PM_1PRIMITIVE" };

gfx_label_t CGfxLabels::GeometryModes[] = {
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

gfx_label_t CGfxLabels::GeometryModes_F3DEX2[] = {
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

gfx_label_t CGfxLabels::RenderModesCycle1[] = {
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

gfx_label_t CGfxLabels::RenderModesCycle2[] = {
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
cc_preset_lut_entry_t CGfxLabels::CombineModes[] = {
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

gfx_label_t CGfxLabels::CCMuxA[] = {
	{ 0,  "COMBINED" },{ 1,  "TEXEL0" },{ 2,  "TEXEL1" },{ 3,  "PRIMITIVE" },
	{ 4,  "SHADE" },{ 5,  "ENVIRONMENT" },{ 6,  "ONE" },{ 7,  "NOISE" },
	{ 15, "ZERO" },
	{ 0 , NULL },
};

gfx_label_t CGfxLabels::CCMuxB[] = {
	{ 0,  "COMBINED" },{ 1,  "TEXEL0" },{ 2,  "TEXEL1" },{ 3,  "PRIMITIVE" },
	{ 4,  "SHADE" },{ 5,  "ENVIRONMENT" },{ 6,  "CENTER" },{ 7,  "K4" },
	{ 15, "ZERO" },
	{ 0 , NULL }
};

gfx_label_t CGfxLabels::CCMuxC[] = {
	{ 0,  "COMBINED" },{ 1,  "TEXEL0" },{ 2,  "TEXEL1" },{ 3,  "PRIMITIVE" },
	{ 4,  "SHADE" },{ 5, "ENVIRONMENT" },{ 6, "SCALE" },{ 7,  "COMBINED_ALPHA" },
	{ 8,  "TEXEL0_ALPHA" },{ 9,  "TEXEL1_ALPHA" },{ 10, "PRIMITIVE_ALPHA" },
	{ 11, "SHADE_ALPHA" },{ 12, "ENV_ALPHA" },{ 13, "LOD_FRACTION" },
	{ 14, "PRIM_LOD_FRAC" },{ 15, "K5" },
	{ 31, "ZERO" },
	{ 0 , NULL }
};

gfx_label_t CGfxLabels::CCMuxD[] = {
	{ 0,  "COMBINED" },{ 1,  "TEXEL0" },{ 2,  "TEXEL1" },{ 3,  "PRIMITIVE" },
	{ 4,  "SHADE" },{ 5,  "ENVIRONMENT" },{ 6,  "ONE" },
	{ 7,  "ZERO" },
	{ 0 , NULL }
};

gfx_label_t CGfxLabels::ACMuxA_B_D[] = {
	{ 0, "COMBINED" },{ 1,  "TEXEL0" },{ 2, "TEXEL1" },{ 3,  "PRIMITIVE" },
	{ 4, "SHADE" },{ 5,  "ENVIRONMENT" },{ 6,  "ONE" },
	{ 7, "ZERO" },
	{ 0 , NULL }
};

gfx_label_t CGfxLabels::ACMuxC[] = {
	{ 0,  "LOD_FRACTION" },{ 1,  "TEXEL0" },{ 2,  "TEXEL1" },
	{ 3,  "PRIMITIVE" },{ 4,  "SHADE" },{ 5,  "ENVIRONMENT" },{ 6, "PRIM_LOD_FRAC" },
	{ 7,  "ZERO" },
	{ 0 , NULL }
};
