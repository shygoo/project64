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

class GfxMicrocode
{
public:
	static uint32_t Identify(uint8_t* ucode, ucode_info_t *info);
private:
	static ucode_checksum_t     Checksums[];
	static ucode_info_t         Microcodes[];
};