#pragma once

#include <stdafx.h>
#include "GfxOpTypes.h"

//////////////////
//void op_HexParam32(CHleGfxState*, decoded_cmd_t*);
//#define op_gsSPClearGeometryMode_f3d dec_gsSPSetGeometryMode_f3d

class CHleGfxState;

enum resource_type_t
{
	RES_NONE = 0,
	RES_ROOT_DL,
	RES_DL,
	RES_SEGMENT,
	RES_COLORBUFFER,
	RES_DEPTHBUFFER,
	RES_TEXTURE,
	RES_PALETTE,
	RES_VERTICES,
	RES_PROJECTION_MATRIX,
	RES_MODELVIEW_MATRIX,
	RES_VIEWPORT,
	RES_DIFFUSE_LIGHT,
	RES_AMBIENT_LIGHT,
};

enum im_fmt_t
{
	G_IM_FMT_RGBA,
	G_IM_FMT_YUV,
	G_IM_FMT_CI,
	G_IM_FMT_IA,
	G_IM_FMT_I
};

enum im_siz_t
{
	G_IM_SIZ_4b,
	G_IM_SIZ_8b,
	G_IM_SIZ_16b,
	G_IM_SIZ_32b
};

typedef struct
{
	int nCommand;
	resource_type_t type;
	uint32_t address;
	uint32_t virtAddress;
	uint32_t param;
	uint32_t imageWidth;
	uint32_t imageHeight;
	im_fmt_t imageFmt;
	im_siz_t imageSiz;
} dram_resource_t;

typedef struct
{
	int16_t x, y, z;
    int16_t _unused;
	int16_t u, v;
    union
    {
        struct { uint8_t r, g, b; };
        struct { uint8_t nx, ny, nz; };
    };
    uint8_t a;
    
} vertex_t;

typedef struct
{
	vertex_t v0, v1, v2;
} vbuf_tri_t;

typedef struct
{
	uint32_t        address, virtualAddress;
	dl_cmd_t        rawCommand;
	union {
		const char* name, *overrideName;
	};
	stdstr          params;
	dram_resource_t dramResource;
	int             numTris;
	vbuf_tri_t      tris[4];
	COLORREF        listBgColor, listFgColor;
} decoded_cmd_t;

typedef void(*dl_op_t)(CHleGfxState* state, decoded_cmd_t* dc);

typedef struct
{
	uint8_t     commandByte;
	const char* commandName;
	dl_op_t     opFunc;
} dl_cmd_info_t;

enum ucode_version_t
{
	UCODE_UNKNOWN,
	UCODE_F3D,
	UCODE_F3DEX,
	UCODE_F3DEX2,
    UCODE_ZSORT,
    UCODE_DKR,
    UCODE_S2DEX,
    UCODE_TURBO3D,
    UCODE_PD,
    UCODE_FACTOR5,
    UCODE_F3DEX_WR,
    UCODE_F3DEXBG,
    UCODE_F3DTEXA
};

typedef struct
{
    uint32_t checksum;
    ucode_version_t version;
} ucode_checksum_t;

typedef struct
{
	ucode_version_t version;
	const char*     name;
	dl_cmd_info_t*  commandTable;
} ucode_info_t;

class CGfxOps
{
public:
	static const ucode_info_t*  LookupMicrocode(uint32_t checksum);
	static const dl_cmd_info_t* LookupCommand(dl_cmd_info_t* commands, uint8_t cmdByte);

    static ucode_checksum_t MicrocodeChecksums[];
	static ucode_info_t  Microcodes[];
	static dl_cmd_info_t Commands_RDP[];
	static dl_cmd_info_t Commands_F3D[];
	static dl_cmd_info_t Commands_F3DEX[];
	static dl_cmd_info_t Commands_F3DEX2[];

private:
	static void ReportDramResource(decoded_cmd_t* dc, CHleGfxState* state, resource_type_t resType, uint32_t param = 0);
	static void op_Nop(CHleGfxState*, decoded_cmd_t*);

	// RDP
	static void op_gsDPSetScissor(CHleGfxState*, decoded_cmd_t*);
    static void op_gsDPLoadTLUT(CHleGfxState*, decoded_cmd_t*);
	static void op_gsDPSetTileSize(CHleGfxState*, decoded_cmd_t*);
	static void op_gsDPLoadBlock(CHleGfxState*, decoded_cmd_t*);
    static void op_gsDPLoadTile(CHleGfxState*, decoded_cmd_t*);
	static void op_gsDPSetTile(CHleGfxState*, decoded_cmd_t*);
	static void op_gsDPFillRectangle(CHleGfxState*, decoded_cmd_t*);
	static void op_gsDPSetFillColor(CHleGfxState*, decoded_cmd_t*);
	static void op_gsDPSetFogColor(CHleGfxState*, decoded_cmd_t*);
	static void op_gsDPSetBlendColor(CHleGfxState*, decoded_cmd_t*);
	static void op_gsDPSetPrimColor(CHleGfxState*, decoded_cmd_t*);
	static void op_gsDPSetEnvColor(CHleGfxState*, decoded_cmd_t*);
	static void op_gsDPSetCombineLERP(CHleGfxState*, decoded_cmd_t*);
	static void op_gsDPSetTextureImage(CHleGfxState*, decoded_cmd_t*);
	static void op_gsDPSetDepthImage(CHleGfxState*, decoded_cmd_t*);
	static void op_gsDPSetColorImage(CHleGfxState*, decoded_cmd_t*);

	// Shared RSP
	static void op_gsSPDisplayList(CHleGfxState*, decoded_cmd_t*);
	static void op_gsSPEndDisplayList(CHleGfxState*, decoded_cmd_t*);

	// Fast3D
	static void op_gsSPVertex_f3d(CHleGfxState*, decoded_cmd_t*);
	static void op_gsSPMoveWord_f3d(CHleGfxState*, decoded_cmd_t*);
	static void op_gsSPSetGeometryMode_f3d(CHleGfxState*, decoded_cmd_t*);
	static void op_gsSPClearGeometryMode_f3d(CHleGfxState*, decoded_cmd_t*);
	static void op_gsSPTexture_f3d(CHleGfxState*, decoded_cmd_t*);
	static void op_gsSP1Triangle_f3d(CHleGfxState*, decoded_cmd_t*);
	static void op_gsSPMatrix_f3d(CHleGfxState*, decoded_cmd_t*);
	static void op_gsSPMoveMem_f3d(CHleGfxState*, decoded_cmd_t*);
    static void op_gsSPSetOtherMode_h_f3d(CHleGfxState*, decoded_cmd_t*);
    static void op_gsSPSetOtherMode_l_f3d(CHleGfxState*, decoded_cmd_t*);

    // Fast3DEX
    static void op_gsSPVertex_f3dex(CHleGfxState*, decoded_cmd_t*);
    static void op_gsSP1Triangle_f3dex(CHleGfxState*, decoded_cmd_t*);
    static void op_gsSP1Quadrangle_f3dex(CHleGfxState*, decoded_cmd_t*);
    static void op_gsSP2Triangles_f3dex(CHleGfxState*, decoded_cmd_t*);

	// Fast3DEX2
	static void op_gsSPMoveWord_f3dex2(CHleGfxState*, decoded_cmd_t*);
    static void op_gsSPVertex_f3dex2(CHleGfxState*, decoded_cmd_t*);
    static void op_gsSP1Triangle_f3dex2(CHleGfxState*, decoded_cmd_t*);
    static void op_gsSPSetOtherMode_h_f3dex2(CHleGfxState*, decoded_cmd_t*);
    static void op_gsSPSetOtherMode_l_f3dex2(CHleGfxState*, decoded_cmd_t*);
    static void op_gsSPMoveMem_f3dex2(CHleGfxState*, decoded_cmd_t*);
    static void op_gsSPGeometryMode_f3dex2(CHleGfxState*, decoded_cmd_t*);
    static void op_gsSPMatrix_f3dex2(CHleGfxState*, decoded_cmd_t*);
};


