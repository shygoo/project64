#pragma once
#include <stdafx.h>
#include "DisplayListCommands.h"
#include "Renderer.h"

typedef struct
{
	int16_t x, y, z;
	int16_t u, v;
} vertex_t;

typedef struct
{
	uint32_t value;
	const char *name;
} name_lut_entry_t;

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

#define RM_C1_MASK 0xCCCCFFFF
#define RM_C2_MASK 0x3333FFFF

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

} hle_tile_descriptor_t;

typedef enum
{
	UCODE_UNKNOWN,
	UCODE_F3D,
	UCODE_F3DEX,
	UCODE_F3DEX2
} ucode_version_t;

class OtherModeNames
{
public:
	static const char* ad[]; 
	static const char* rd[];
	static const char* ck[];
	static const char* tc[];
	static const char* tf[];
	static const char* tt[];
	static const char* tl[];
	static const char* td[];
	static const char* tp[];
	static const char* cyc[];
	static const char* cd[];
	static const char* pm[];
};

typedef struct
{
    const char* name;
    int a, b, c, d;
    int Aa, Ab, Ac, Ad;
} cc_preset_lut_entry_t;

class CHleDmemState
{
public:
    uint32_t address;
    dl_cmd_t command;
    uint32_t segments[16];
    uint32_t stack[16];
    uint32_t stackIndex;
    vertex_t vertices[64];
    hle_tile_descriptor_t tiles[8];
	othermode_h_t othermode_h;
    geometrymode_f3d_t geometryMode;
    uint8_t  numLights;
	uint32_t textureImage, depthImage, colorImage;
	im_fmt_t textureImageFmt;
	im_siz_t textureImageSiz;
	uint32_t fillColor, fogColor, blendColor, primColor, envColor;

    dl_cmd_setcombine_t combiner;

    bool bDone;

	uint8_t lastBlockLoadTexelSize;
	uint16_t lastBlockLoadSize;

    uint32_t SegmentedToPhysical(uint32_t segaddr);
    uint32_t SegmentedToVirtual(uint32_t segaddr);
};

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
    vertex_t v0, v1, v2;
} vbuf_tri_t;

typedef struct
{
	union {
		const char* name;
		const char* overrideName;
	};
	char params[512];
	dram_resource_t dramResource;
    int numTris;
    vbuf_tri_t tris[4];
	COLORREF listBgColor, listFgColor;
} decode_context_t;

typedef void (*dl_cmd_operation_func_t)(CHleDmemState* state);
typedef void (*dl_cmd_decoder_func_t)(CHleDmemState* state, decode_context_t* dc);

typedef struct {
	uint8_t commandByte;
	const char* commandName;
	dl_cmd_operation_func_t opFunc;
	dl_cmd_decoder_func_t decodeFunc;
} dl_cmd_info_t;

typedef struct
{
	uint32_t checksum;
	ucode_version_t version;
	const char* name;
	dl_cmd_info_t* commandTable;
} ucode_version_info_t;

class CDisplayListParser
{
public:
    CDisplayListParser(void);

    void Reset(uint32_t ucodeAddr, uint32_t dlistAddr, uint32_t dlistSize);

	static bool ConvertImage(uint32_t* dst, uint8_t *src, im_fmt_t fmt, im_siz_t siz, int numTexels);

    static name_lut_entry_t GeometryModeNames[];
    static name_lut_entry_t GeometryModeNames_F3DEX2[];
    static name_lut_entry_t TexelSizeNames[];
	static name_lut_entry_t ImageFormatNames[];
	static name_lut_entry_t TexelSizeShortNames[];
	static name_lut_entry_t ImageFormatShortNames[];
    static cc_preset_lut_entry_t CombinerPresetNames[];
    static name_lut_entry_t RenderModePresetNamesCycle1[];
    static name_lut_entry_t RenderModePresetNamesCycle2[];
    static name_lut_entry_t CCMuxA[];
    static name_lut_entry_t CCMuxB[];
    static name_lut_entry_t CCMuxC[];
    static name_lut_entry_t CCMuxD[];
    static name_lut_entry_t ACMuxA_B_D[];
    static name_lut_entry_t ACMuxC[];

    static const char* LookupName(name_lut_entry_t* set, uint32_t value);
    static const ucode_version_info_t* GetUCodeVersionInfo(uint32_t checksum);

private:
    static ucode_version_info_t UCodeVersions[];
    static dl_cmd_info_t* LookupCommand(dl_cmd_info_t* commands, uint8_t cmdByte);
    static dl_cmd_info_t Commands_Global[];
    static dl_cmd_info_t Commands_F3D[];
    static dl_cmd_info_t Commands_F3DEX[];
    static dl_cmd_info_t Commands_F3DEX2[];

public:
    void Run();

	ucode_version_t GetUCodeVersion(void);
	const char*     GetUCodeName(void);
	uint32_t        GetUCodeChecksum(void);
    
    CHleDmemState* GetLoggedState(size_t index);
	uint8_t*       GetRamSnapshot(void);

private:
    void     StepDecode();
    uint32_t GetCommandAddress(void);
    uint32_t GetCommandVirtualAddress(void);
    dl_cmd_t GetRawCommand(void);
    int      GetStackIndex(void);
    //bool     IsDone(void);

    uint8_t* m_RamSnapshot;
    CScene   m_Scene;
    //int      m_nCommand;

    CHleDmemState m_State; // hle dmem/rdp state

    std::vector<CHleDmemState> m_StateLog;
    std::vector<decode_context_t> m_DecodedCommands;

    uint32_t        m_UCodeChecksum;
    ucode_version_t m_UCodeVersion;
    const char*     m_UCodeName;
    uint32_t        m_VertexBufferSize;

    dl_cmd_info_t*  m_CommandTable;

    uint32_t m_RootDListSize;
    uint32_t m_RootDListEndAddress;
};