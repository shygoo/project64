#pragma once
#include <stdafx.h>

#define RM_C1_MASK 0xCCCCFFFF
#define RM_C2_MASK 0x3333FFFF

typedef struct
{
	uint32_t value;
	const char *name;
} gfx_label_t;

typedef struct
{
	const char* name;
	uint8_t a, b, c, d;
	uint8_t Aa, Ab, Ac, Ad;
} cc_preset_lut_entry_t;

class CGfxLabels
{
public:
	static const char* LookupName(gfx_label_t* set, uint32_t value);
    static stdstr GeometryModeString(uint32_t bits);

	static const char* TexelSizes[];
	static const char* ImageFormats[];
	static const char* TexelSizesShort[];
	static const char* ImageFormatsShort[];
	static const char* OtherMode_ad[];
	static const char* OtherMode_rd[];
	static const char* OtherMode_ck[];
	static const char* OtherMode_tc[];
	static const char* OtherMode_tf[];
	static const char* OtherMode_tt[];
	static const char* OtherMode_tl[];
	static const char* OtherMode_td[];
	static const char* OtherMode_tp[];
	static const char* OtherMode_cyc[];
	static const char* OtherMode_cd[];
	static const char* OtherMode_pm[];
    
	static const char* OtherMode_ac[];
    static const char* OtherMode_zs[];

    static const char* OtherModeL_cvgdst[];
    static const char* OtherModeL_zmode[];
    static const char* OtherModeL_blpm[];
    static const char* OtherModeL_bla[];
    static const char* OtherModeL_blb[];
	//static const char* OtherMode_rm[];
	//static const char* OtherMode_rm2[];

	//static gfx_label_t OtherModeA;
	//static gfx_label_t OtherModeB;

    static gfx_label_t LightColorOffsets[];
    static gfx_label_t ClipDirections[];
    static gfx_label_t ClipRatios[];
	static gfx_label_t GeometryModes[];
	static gfx_label_t GeometryModes_F3DEX2[];
	static cc_preset_lut_entry_t CombineModes[];
	static gfx_label_t RenderModesCycle1[];
	static gfx_label_t RenderModesCycle2[];
	static gfx_label_t CCMuxA[];
	static gfx_label_t CCMuxB[];
	static gfx_label_t CCMuxC[];
	static gfx_label_t CCMuxD[];
	static gfx_label_t ACMuxA_B_D[];
	static gfx_label_t ACMuxC[];
};
