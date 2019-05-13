#pragma once
#include <stdafx.h>

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
	struct { uint32_t index : 8; uint32_t offset : 16; uint32_t c : 8; }; // w0
	uint32_t data; // w1
} dl_cmd_moveword_f3d_t;

#define dl_cmd_moveword_f3dex_t dl_cmd_moveword_f3d_t

typedef struct
{
	struct { uint32_t offset : 16; uint32_t index : 8;  uint32_t c : 8; }; // w0
	uint32_t data; // w1
} dl_cmd_moveword_f3dex2_t;

typedef struct
{
	struct { uint32_t len : 16; uint32_t idx : 4; uint32_t num : 4; uint32_t c : 8; };
	uint32_t address;
} dl_cmd_vtx_f3d_t;

typedef struct
{
	uint32_t w0;
	struct { uint32_t v2 : 8; uint32_t v1 : 8; uint32_t v0 : 8; uint32_t c : 8; };
} dl_cmd_tri1_f3d_t;

typedef struct
{
	struct { uint32_t size : 16; uint32_t params : 8; uint32_t c : 8; };
	uint32_t address;
} dl_cmd_mtx_f3d_t;

#define dl_cmd_mtx_f3dex_t dl_cmd_mtx_f3d_t

typedef struct
{
	struct { uint32_t lry : 12; uint32_t lrx : 12; uint32_t c : 8; };
	struct { uint32_t uly : 12; uint32_t ulx : 12; uint32_t : 8; };
} dl_cmd_fillrect_t;

typedef struct
{
	struct { uint32_t uly : 12; uint32_t ulx : 12; uint32_t c : 8; };
	struct { uint32_t lry : 12; uint32_t lrx : 12; uint32_t mode : 2; uint32_t : 2; };
} dl_cmd_setscissor_t;

typedef struct
{
	struct { uint32_t : 16; uint32_t branch : 8; uint32_t c : 8; };
	uint32_t address;
} dl_cmd_dl_t;

typedef union
{
	struct { uint32_t w0, w1; };
	dl_cmd_moveword_f3d_t moveword_f3d;
	dl_cmd_moveword_f3dex2_t moveword_f3dex2;
	dl_cmd_tri1_f3d_t tri1_f3d;
	dl_cmd_vtx_f3d_t vtx_f3d;
	dl_cmd_dl_t dl;
	dl_cmd_mtx_f3d_t mtx_f3d;
	dl_cmd_mtx_f3dex_t mtx_f3dex;
	dl_cmd_fillrect_t fillrect;
	dl_cmd_setscissor_t setscissor;
} dl_cmd_t;

typedef struct
{
	uint32_t address;
	dl_cmd_t command;
	uint32_t segments[16];
	uint32_t stack[16]; // todo proper stack size for each ucode
	uint32_t stackIndex;
	vertex_t vertices[64];
	bool     bDone;
} hl_state_t;

typedef enum
{
	UCODE_UNKNOWN,
	UCODE_F3D,
	UCODE_F3DEX,
	UCODE_F3DEX2
} ucode_version_t;

typedef void (*dl_cmd_operation_func_t)(hl_state_t* state);
typedef const char* (*dl_cmd_decoder_func_t)(hl_state_t* state, char* paramsBuf);

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

	hl_state_t m_State;
	std::vector<hl_state_t> m_StateLog;

	uint32_t m_UCodeChecksum;
	ucode_version_t m_UCodeVersion;
	const char *m_UCodeName;
	dl_cmd_info_t *m_CommandTable;

	uint32_t m_RootDListSize;
	uint32_t m_VertexBufferSize;

public:
	CDisplayListParser(uint32_t ucodeAddr, uint32_t dlistAddr, uint32_t dlistSize);

	static const ucode_version_info_t* GetUCodeVersionInfo(uint32_t checksum);
	static uint32_t SegmentedToPhysical(hl_state_t* state, uint32_t address);

	ucode_version_t GetUCodeVersion(void);
	const char* GetUCodeName(void);
	uint32_t GetUCodeChecksum(void);

	int GetCommandCount(void);
	hl_state_t *GetLogState(size_t index);
	const char* DecodeCommand(size_t index, char* paramsBuf);

private:
	void Step(void);
	
	static void op_gSPDisplayList(hl_state_t* state);
	static void op_gSPEndDisplayList(hl_state_t* state);
	static void op_gSPMoveWord_f3d(hl_state_t* state);
	static void op_gSPMoveWord_f3dex2(hl_state_t* state);

	static const char* dec_gSPMoveWord_f3d(hl_state_t*, char*);
	static const char* dec_gSPMoveWord_f3dex2(hl_state_t*, char*);
	static const char* dec_gSP1Triangle_f3d(hl_state_t*, char*);
	static const char* dec_gSPVertex_f3d(hl_state_t*, char*);
	static const char* dec_gSPSetGeometryMode_f3d(hl_state_t*, char*);
	static const char* dec_gSPDisplayList(hl_state_t*, char*);
	static const char* dec_gSPMatrix_f3d(hl_state_t*, char*);
	static const char* dec_gDPFillRectangle(hl_state_t*, char*);
	static const char* dec_gDPSetScissor(hl_state_t*, char*);
	static const char* dec_HexParam32(hl_state_t*, char*);
	static const char* dec_NoParams(hl_state_t*, char*);
};