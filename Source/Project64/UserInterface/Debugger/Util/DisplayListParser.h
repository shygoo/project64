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
	uint32_t value;
	const char *name;
} name_lut_entry_t;

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
    uint32_t textureAddr;
    uint32_t geometryMode;
    uint8_t  numLights;
    bool bDone;

    uint32_t SegmentedToPhysical(uint32_t segaddr);
    uint32_t SegmentedToVirtual(uint32_t segaddr);
};

typedef void (*dl_cmd_operation_func_t)(CHleDmemState* state);
typedef const char* (*dl_cmd_decoder_func_t)(CHleDmemState* state, char* paramsBuf);

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

    static name_lut_entry_t GeometryModeNames[];
    static name_lut_entry_t GeometryModeNames_F3DEX2[];
    static name_lut_entry_t TexelSizeNames[];
    static name_lut_entry_t ImageFormatNames[];

    static const char* LookupName(name_lut_entry_t* set, uint32_t value);

	static const ucode_version_info_t* GetUCodeVersionInfo(uint32_t checksum);
	ucode_version_t GetUCodeVersion(void);
	const char* GetUCodeName(void);
	uint32_t GetUCodeChecksum(void);

    bool IsDone(void);
    const char* StepDecode(char* paramsTextBuf, uint32_t* flags);

    uint32_t GetCommandAddress(void);
    uint32_t GetCommandVirtualAddress(void);
    dl_cmd_t GetRawCommand(void);
    int GetStackIndex(void);

    CHleDmemState* GetLoggedState(size_t index);

private:
    static ucode_version_info_t UCodeVersions[];

    static dl_cmd_info_t Commands_Global[];
    static dl_cmd_info_t Commands_F3D[];
    static dl_cmd_info_t Commands_F3DEX[];
    static dl_cmd_info_t Commands_F3DEX2[];

    static dl_cmd_info_t* LookupCommand(dl_cmd_info_t* commands, uint8_t cmdByte);

    CHleDmemState m_State;
    std::vector<CHleDmemState> m_StateLog;

    uint32_t m_UCodeChecksum;
    ucode_version_t m_UCodeVersion;
    const char *m_UCodeName;
    dl_cmd_info_t *m_CommandTable;

    uint32_t m_RootDListSize;
    uint32_t m_VertexBufferSize;
};