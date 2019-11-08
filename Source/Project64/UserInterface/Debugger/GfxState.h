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

typedef struct
{
    uint8_t colorA[3];
    uint8_t _pad0;
    uint8_t colorB[3];
    uint8_t _pad1;
    int8_t  direction[3];
    uint8_t _pad2;
} light_t;

class rsp_mtx_t
{
public:
    int16_t intpart[4][4];
    uint16_t fracpart[4][4];

    float GetF(int row, int col);
    int32_t Get(int row, int col);
    static int32_t MtxDotProduct(rsp_mtx_t* a, int rowA, rsp_mtx_t* b, int colB);
    rsp_mtx_t Mul(rsp_mtx_t* b);
};

class CHleGfxState
{
public:
    CHleGfxState(void);
    uint32_t SegmentedToPhysical(uint32_t segaddr);
    uint32_t SegmentedToVirtual(uint32_t segaddr);
    bool     LoadVertices(uint32_t address, int index, int numv);
    void     LoadMatrix(uint32_t address, bool bPush, bool bLoad, bool bProjection);
    void     LoadLight(uint32_t address, int index);
    bool     GetCommand(uint32_t address, dl_cmd_t *command);
    int      GetCommands(uint32_t address, int numCommands, dl_cmd_t commands[]);
    vertex_t Transform(vertex_t* vertex);
    
    bool     m_bDone;
    uint32_t m_nCommand;

    uint8_t  lastBlockLoadTexelSize;
    uint16_t lastBlockLoadSize;

    // RSP registers/dmem variables
	uint32_t m_spCommandAddress;
	dl_cmd_t m_spCommand;
	uint32_t m_spSegments[16];
	uint32_t m_spStack[32]; // larger than necessary
	uint32_t m_spStackIndex;
	vertex_t m_spVertices[64];
    uint8_t  m_spNumLights;
    light_t  m_spLights[10]; // lookatx, lookaty, + 8 lights

    int       m_spMatrixIndex;
    rsp_mtx_t m_spMatrixStack[10];
    rsp_mtx_t m_spProjectionMatrix;

    union
    {
        geometrymode_f3d_t f3d;
        geometrymode_f3dex2_t f3dex2;
        uint32_t data;
    } m_spGeometryMode;

    // RDP registers
	tile_t m_dpTileDescriptors[8];
	othermode_h_t m_dpOtherMode_h;
	othermode_l_t m_dpOtherMode_l;
	dl_cmd_setcombine_t m_dpCombiner;
	uint32_t m_dpTextureImage;
    im_fmt_t m_dpTextureImageFmt;
    im_siz_t m_dpTextureImageSiz;
	uint32_t m_dpDepthImage;
	uint32_t m_dpColorImage;
	uint32_t m_dpFillColor;
	uint32_t m_dpFogColor;
	uint32_t m_dpBlendColor;
	uint32_t m_dpPrimColor;
	uint32_t m_dpEnvColor;
};
