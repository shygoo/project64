#pragma once

#include <stdafx.h>
#include "GfxOps.h"

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

enum ucode_patch_t
{
    PATCH_NONE,
    PATCH_F3DEX_095 // MK64
};

typedef struct
{
	uint32_t        checksum;
	ucode_version_t ucodeId;
    ucode_patch_t   patchId;
} ucode_checksum_t;

typedef struct
{
	int            id;
	const char*    name;
	dl_cmd_info_t* commandTable;
} ucode_cmd_lut_t;

typedef struct
{
    uint32_t        checksum;
    ucode_version_t ucodeId;
    const char*     ucodeName;
    dl_cmd_info_t*  ucodeCommandTable;
    ucode_patch_t   patchId;
    const char*     patchName;
    dl_cmd_info_t*  patchCommandTable;
} ucode_info_t;

class CGfxMicrocode
{
public:
	static void Identify(uint8_t* ucode, ucode_info_t *info);
    //static void BuildArray(ucode_info_t* info, dl_cmd_info_t* arr);
private:
	static ucode_checksum_t     Checksums[];
    static ucode_cmd_lut_t      Microcodes[];
    static ucode_cmd_lut_t      Patches[];
};