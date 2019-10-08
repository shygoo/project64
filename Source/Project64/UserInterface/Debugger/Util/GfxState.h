#pragma once

#include <stdafx.h>
#include "GfxOpTypes.h"
#include "GfxOps.h"

typedef union
{
    uint32_t data;
    struct
    {
        uint32_t
            zbuffer : 1, _u0 : 1,
            shade : 1, _u1 : 6,
            cull_front : 1,
            cull_back : 1, _u2 : 5,
            fog : 1,
            lighting : 1,
            texture_gen : 1,
            texture_gen_linear : 1, _u3 : 1,
            shading_smooth : 1,
            lod : 1,
            clipping : 1, _u5 : 8;
    };
} geometrymode_f3dex2_t;

typedef union
{
	uint32_t data;
	struct {
		uint32_t
			zbuffer : 1, _u0 : 1,
			shade : 1, _u1 : 6,
			shading_smooth : 1, _u2 : 2,
			cull_front : 1,
			cull_back : 1, _u3 : 2,
			fog : 1,
			lighting : 1,
			texture_gen : 1,
			texture_gen_linear : 1,
			lod : 1, _u4 : 2,
			clipping : 1, _u5 : 8;
	};
} geometrymode_f3d_t;

typedef union
{
	uint32_t data;
	struct
	{
		uint32_t _u0 : 4, ad : 2, rd : 2, ck : 1, tc : 3, tf : 2, tt : 2,
			tl : 1, td : 2, tp : 1, cyc : 2, cd : 1, pm : 1, c : 8;
	};
} othermode_h_t;

typedef union
{
	uint32_t data;
	struct
	{
		uint32_t
			alphacompare : 2,
			zsrcsel : 1,
			// cycle independent blender settings
			aa_en : 1,
			z_cmp : 1,
			z_upd : 1,
			im_rd : 1,
			clr_on_cvg : 1,
			cvg_dst : 2,
			zmode : 2,
			cvg_x_alpha : 1,
			alpha_cvg_sel : 1,
			force_bl : 1,
			_u0 : 1, // ?
			// cycle dependent blender settings
			b1 : 2, b0 : 2, m1 : 2, m0 : 2, a1 : 2, a0 : 2, p1 : 2, p0 : 2;
	};
} othermode_l_t;

typedef struct
{
	// via gDPSetTile
	struct {
		uint32_t tmem : 9;
		uint32_t line : 9;
	};

	struct {
		uint32_t siz : 2;
		uint32_t fmt : 3;
		uint32_t shifts : 4;
		uint32_t masks : 4;
		uint32_t cms : 2;
		uint32_t shiftt : 4;
		uint32_t maskt : 4;
		uint32_t cmt : 2;
		uint32_t palette : 4;
	};

	// via gSPTexture
	uint16_t scaleS;
	uint16_t scaleT;
	uint8_t  mipmapLevels;
	uint8_t  enabled;
} tile_t;

class CHleGfxState
{
public:
    uint32_t m_nCommand;
	uint32_t m_Address;
	dl_cmd_t m_Command;

	uint32_t m_Segments[16];
	uint32_t m_Stack[16];
	uint32_t m_StackIndex;
	vertex_t m_Vertices[64];
	tile_t   m_Tiles[8];
	othermode_h_t m_OtherMode_h;
	othermode_l_t m_OtherMode_l;
	dl_cmd_setcombine_t m_Combiner;
    union
    {
        geometrymode_f3d_t f3d;
        geometrymode_f3dex2_t f3dex2;
        uint32_t data;
    } m_GeometryMode;
	uint8_t  m_NumLights;
	uint32_t m_TextureImage;
	uint32_t m_DepthImage;
	uint32_t m_ColorImage;
	im_fmt_t m_TextureImageFmt;
	im_siz_t m_TextureImageSiz;
	uint32_t m_FillColor;
	uint32_t m_FogColor;
	uint32_t m_BlendColor;
	uint32_t m_PrimColor;
	uint32_t m_EnvColor;

	uint8_t lastBlockLoadTexelSize;
	uint16_t lastBlockLoadSize;

	bool m_bDone;

    CHleGfxState(void);
	uint32_t SegmentedToPhysical(uint32_t segaddr);
	uint32_t SegmentedToVirtual(uint32_t segaddr);
    bool LoadVertices(uint32_t address, int index, int numv);
};
