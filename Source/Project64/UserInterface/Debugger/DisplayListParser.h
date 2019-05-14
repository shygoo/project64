#pragma once
#include <stdafx.h>
#include "DisplayListCommands.h"

typedef struct
{
	int16_t x, y, z;
	int16_t u, v;
} vertex_t;

typedef struct
{
	uint32_t mask;
	const char *name;
} geo_mode_bitname_t;

typedef struct
{
    // via gDPSetTile
    uint8_t  siz;
    uint8_t  fmt;
    uint16_t line;
    uint16_t tmem; // tmem offset 0~511
    uint8_t  palette; // 0~15
    uint8_t  cmt; // t axis mirror/wrap/clamp flags
    uint8_t  maskt; 
    uint8_t  shiftt; // t coord shift
    uint8_t  cms;
    uint8_t  masks;
    uint8_t  shifts; // s coord shift

    // via gSPTexture
    uint16_t scaleS;
    uint16_t scaleT;
    uint8_t  mipmapLevels;
    uint8_t  enabled;

    // via setTextureImage

} hle_tile_descriptor_t;

typedef struct
{
	uint32_t address;
	dl_cmd_t command;
	uint32_t segments[16];
	uint32_t stack[16]; // todo proper stack size for each ucode
	uint32_t stackIndex;
	vertex_t vertices[64];
	bool     bDone;

    hle_tile_descriptor_t tiles[8];
    uint32_t textureAddr;
} hle_dmem_state_t;

typedef enum
{
	UCODE_UNKNOWN,
	UCODE_F3D,
	UCODE_F3DEX,
	UCODE_F3DEX2
} ucode_version_t;

typedef void (*dl_cmd_operation_func_t)(hle_dmem_state_t* state);
typedef const char* (*dl_cmd_decoder_func_t)(hle_dmem_state_t* state, char* paramsBuf);

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
private:
	static ucode_version_info_t UCodeVersions[];

	static dl_cmd_info_t Commands_Global[];
	static dl_cmd_info_t Commands_F3D[];
	static dl_cmd_info_t Commands_F3DEX[];
	static dl_cmd_info_t Commands_F3DEX2[];

	static geo_mode_bitname_t GeometryModeNames[];
	static geo_mode_bitname_t GeometryModeNames_F3DEX2[];

	static dl_cmd_info_t* LookupCommand(dl_cmd_info_t* commands, uint8_t cmdByte);

	hle_dmem_state_t m_State;
	//std::vector<hle_dmem_state_t> m_StateLog;

	uint32_t m_UCodeChecksum;
	ucode_version_t m_UCodeVersion;
	const char *m_UCodeName;
	dl_cmd_info_t *m_CommandTable;

	uint32_t m_RootDListSize;
	uint32_t m_VertexBufferSize;

public:
	CDisplayListParser(uint32_t ucodeAddr, uint32_t dlistAddr, uint32_t dlistSize);

	static const ucode_version_info_t* GetUCodeVersionInfo(uint32_t checksum);
    static uint32_t SegmentedToPhysical(hle_dmem_state_t* state, uint32_t address);
    static uint32_t SegmentedToVirtual(hle_dmem_state_t* state, uint32_t address);

	ucode_version_t GetUCodeVersion(void);
	const char* GetUCodeName(void);
	uint32_t GetUCodeChecksum(void);

	int GetCommandCount(void);
	hle_dmem_state_t *GetLogState(size_t index);
	//const char* DecodeCommand(size_t index, char* paramsBuf);

private:
	const char* DecodeNextCommand(char* paramsTextBuf, uint32_t* flags);
	//void Step(void);
	
	static void op_gsSPDisplayList(hle_dmem_state_t* state);
	static void op_gsSPEndDisplayList(hle_dmem_state_t* state);
	static void op_gsSPMoveWord_f3d(hle_dmem_state_t* state);
    static void op_gsSPMoveWord_f3dex2(hle_dmem_state_t* state);
    static void op_gsDPSetTile(hle_dmem_state_t* state);

	static const char* dec_NoParams(hle_dmem_state_t*, char*);
	static const char* dec_HexParam32(hle_dmem_state_t*, char*);
	static const char* dec_gsDPFillRectangle(hle_dmem_state_t*, char*);
	static const char* dec_gsDPSetScissor(hle_dmem_state_t*, char*);
	static const char* dec_gsDPSetTextureImage(hle_dmem_state_t*, char*);
	static const char* dec_gsDPLoadBlock(hle_dmem_state_t*, char*);
	static const char* dec_gsDPSetTileSize(hle_dmem_state_t*, char*);
	static const char* dec_gsDPSetTile(hle_dmem_state_t*, char*);
	static const char* dec_gsSPMoveWord_f3d(hle_dmem_state_t*, char*);
	static const char* dec_gsSP1Triangle_f3d(hle_dmem_state_t*, char*);
	static const char* dec_gsSPVertex_f3d(hle_dmem_state_t*, char*);
	static const char* dec_gsSPSetGeometryMode_f3d(hle_dmem_state_t*, char*);
	static const char* dec_gsSPMatrix_f3d(hle_dmem_state_t*, char*);
	static const char* dec_gsSPMoveWord_f3dex2(hle_dmem_state_t*, char*);
	static const char* dec_gsSPMoveMem_f3d(hle_dmem_state_t*, char*);
	static const char* dec_gsSPTexture_f3d(hle_dmem_state_t*, char*);
	static const char* dec_gsSPDisplayList(hle_dmem_state_t*, char*);
};