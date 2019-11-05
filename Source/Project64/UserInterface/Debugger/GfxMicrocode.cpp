#include <stdafx.h>
#include "GfxMicrocode.h"
#include "GfxOps.h"

void CGfxMicrocode::BuildArray(uint8_t* ucode, ucode_info_t* info, dl_cmd_info_t arr[256])
{
    info->ucodeId = UCODE_UNKNOWN;
    info->ucodeName = "Unknown microcode";
    info->ucodeVersionDef = NULL;
    info->ucodeCommandTable = NULL;
    info->patchId = PATCH_NONE;
    info->patchName = "";
    info->patchVersionDef = NULL;
    info->patchCommandTable = NULL;
	info->checksum = 0;

    // generate checksum
	for (int i = 0; i < 3072; i += sizeof(uint32_t))
	{
		info->checksum += *(uint32_t*)&ucode[i];
	}

    // lookup ucode id and patch id
	for (int i = 0; Checksums[i].checksum != 0; i++)
	{
		if (Checksums[i].checksum == info->checksum)
		{
            info->ucodeId = Checksums[i].ucodeId;
            info->patchId = Checksums[i].patchId;
			break;
		}
	}

	if (info->ucodeId != UCODE_UNKNOWN)
	{
        // lookup ucode name and command table
        for (int i = 0; Microcodes[i].id != UCODE_UNKNOWN; i++)
        {
            if (Microcodes[i].id == info->ucodeId)
            {
                info->ucodeName = Microcodes[i].name;
                info->ucodeVersionDef = Microcodes[i].versionDef;
                info->ucodeCommandTable = Microcodes[i].commandTable;
                break;
            }
        }
	}

    if (info->patchId != PATCH_NONE)
    {
        // lookup patch name and command table
        for (int i = 0; Patches[i].id != PATCH_NONE; i++)
        {
            if (Patches[i].id == info->patchId)
            {
                info->patchName = Patches[i].name;
                info->patchVersionDef = Patches[i].versionDef;
                info->patchCommandTable = Patches[i].commandTable;
                printf("%s %s\n", info->patchName, info->patchVersionDef);
                break;
            }
        }
    }

	for (int i = 0; i < 256; i++)
	{
		arr[i] = { 0 };
	}
	
	// import RDP commands
	for (int i = 0; CGfxOps::Commands_RDP[i].commandName != NULL; i++)
	{
		uint8_t index = CGfxOps::Commands_RDP[i].commandByte;
		arr[index] = CGfxOps::Commands_RDP[i];
	}
	
	// import base RSP commands
	if (info->ucodeCommandTable != NULL)
	{
		for (int i = 0; info->ucodeCommandTable[i].commandName != NULL; i++)
		{
			uint8_t index = info->ucodeCommandTable[i].commandByte;
			arr[index] = info->ucodeCommandTable[i];
		}
	}
	
	// import patched commands
	if (info->patchCommandTable != NULL)
	{
		for (int i = 0; info->patchCommandTable[i].commandName != NULL; i++)
		{
			uint8_t index = info->patchCommandTable[i].commandByte;
			arr[index] = info->patchCommandTable[i];
		}
	}
}

ucode_lut_t CGfxMicrocode::Microcodes[] = {
	//ucode id      name      gbi def        op functions
	{ UCODE_F3D,    "Fast3D", "F3D_GBI",     CGfxOps::Commands_F3D },    // sm64
	{ UCODE_F3DEX,  "F3DEX",  "F3DEX_GBI",   CGfxOps::Commands_F3DEX },  // mk64
	{ UCODE_F3DEX2, "F3DEX2", "F3DEX_GBI_2", CGfxOps::Commands_F3DEX2 }, // zelda, kirby
	{ UCODE_UNKNOWN, NULL,    NULL,          NULL }
};

ucode_lut_t CGfxMicrocode::Patches[] = {
    //patch id         name            gbi def            op functions
    { PATCH_F3DEX_095, "(Beta v0.95)", "F3DEX_PATCH_095", CGfxOps::Patch_F3DEX_BETA },
    { PATCH_NONE,      NULL,           NULL,              NULL}
};

ucode_checksum_lut_t CGfxMicrocode::Checksums[] = {
	//checksum    ucode id        patch id
	{ 0x0D7CBFFB, UCODE_DKR,      PATCH_NONE },
	{ 0x63BE08B1, UCODE_DKR,      PATCH_NONE },
	{ 0x63BE08B3, UCODE_DKR,      PATCH_NONE },
	{ 0x3A1C2B34, UCODE_F3D,      PATCH_NONE },
	{ 0x3A1CBAC3, UCODE_F3D,      PATCH_NONE },
	{ 0x3F7247FB, UCODE_F3D,      PATCH_NONE },
	{ 0x4165E1FD, UCODE_F3D,      PATCH_NONE },
	{ 0x5182F610, UCODE_F3D,      PATCH_NONE },
	{ 0x5D1D6F53, UCODE_F3D,      PATCH_NONE },
	{ 0x6E4D50AF, UCODE_F3D,      PATCH_NONE },
	{ 0xAE08D5B9, UCODE_F3D,      PATCH_NONE },
	{ 0xB54E7F93, UCODE_F3D,      PATCH_NONE },
	{ 0xBC03E969, UCODE_F3D,      PATCH_NONE },
	{ 0xD5604971, UCODE_F3D,      PATCH_NONE },
	{ 0xD5D68B1F, UCODE_F3D,      PATCH_NONE },
	{ 0xD67C2F8B, UCODE_F3D,      PATCH_NONE },
	{ 0xE41EC47E, UCODE_F3D,      PATCH_NONE },
	{ 0x006BD77F, UCODE_F3D,      PATCH_NONE },
	{ 0x07200895, UCODE_F3D,      PATCH_NONE },
	{ 0xCEE7920F, UCODE_F3DEX,    PATCH_NONE },
	{ 0x1118B3E0, UCODE_F3DEX,    PATCH_NONE },
	{ 0x1517A281, UCODE_F3DEX,    PATCH_NONE },
	{ 0x1DE712FF, UCODE_F3DEX,    PATCH_NONE },
	{ 0x24CD885B, UCODE_F3DEX,    PATCH_NONE },
	{ 0x26A7879A, UCODE_F3DEX,    PATCH_NONE },
	{ 0x2C7975D6, UCODE_F3DEX,    PATCH_NONE },
	{ 0x2D3FE3F1, UCODE_F3DEX,    PATCH_NONE },
	{ 0x327B933D, UCODE_F3DEX,    PATCH_NONE },
	{ 0x339872A6, UCODE_F3DEX,    PATCH_NONE },
	{ 0x3FF1A4CA, UCODE_F3DEX,    PATCH_NONE },
	{ 0x4340AC9B, UCODE_F3DEX,    PATCH_NONE },
	{ 0x440CFAD6, UCODE_F3DEX,    PATCH_NONE },
	{ 0x4FE6DF78, UCODE_F3DEX,    PATCH_NONE },
	{ 0x5257CD2A, UCODE_F3DEX,    PATCH_NONE },
	{ 0x5414030C, UCODE_F3DEX,    PATCH_NONE },
	{ 0x5414030D, UCODE_F3DEX,    PATCH_NONE },
	{ 0x559FF7D4, UCODE_F3DEX,    PATCH_NONE },
	{ 0x5DF1408C, UCODE_F3DEX,    PATCH_NONE },
	{ 0x5EF4E34A, UCODE_F3DEX,    PATCH_NONE },
	{ 0x6075E9EB, UCODE_F3DEX,    PATCH_NONE },
	{ 0x60C1DCC4, UCODE_F3DEX,    PATCH_NONE },
	{ 0x64ED27E5, UCODE_F3DEX,    PATCH_NONE },
	{ 0x66C0B10A, UCODE_F3DEX,    PATCH_NONE },
	{ 0x6EAA1DA8, UCODE_F3DEX,    PATCH_NONE },
	{ 0x72A4F34E, UCODE_F3DEX,    PATCH_NONE },
	{ 0x73999A23, UCODE_F3DEX,    PATCH_NONE },
	{ 0x7DF75834, UCODE_F3DEX,    PATCH_NONE },
	{ 0x7F2D0A2E, UCODE_F3DEX,    PATCH_NONE },
	{ 0x82F48073, UCODE_F3DEX,    PATCH_NONE },
	{ 0x832FCB99, UCODE_F3DEX,    PATCH_NONE },
	{ 0x841CE10F, UCODE_F3DEX,    PATCH_NONE },
	{ 0x863E1CA7, UCODE_F3DEX,    PATCH_NONE },
	{ 0x8805FFEA, UCODE_F3DEX,    PATCH_F3DEX_095 }, // MK64
	{ 0x8D5735B2, UCODE_F3DEX,    PATCH_NONE },
	{ 0x8D5735B3, UCODE_F3DEX,    PATCH_NONE },
	{ 0x97D1B58A, UCODE_F3DEX,    PATCH_NONE },
	{ 0xA346A5CC, UCODE_F3DEX,    PATCH_NONE },
	{ 0xB1821ED3, UCODE_F3DEX,    PATCH_NONE },
	{ 0xB4577B9C, UCODE_F3DEX,    PATCH_NONE },
	{ 0xBE78677C, UCODE_F3DEX,    PATCH_NONE },
	{ 0xBED8B069, UCODE_F3DEX,    PATCH_NONE },
	{ 0xC3704E41, UCODE_F3DEX,    PATCH_NONE },
	{ 0xC46DBC3D, UCODE_F3DEX,    PATCH_NONE },
	{ 0xC99A4C6C, UCODE_F3DEX,    PATCH_NONE },
	{ 0xD1663234, UCODE_F3DEX,    PATCH_NONE },
	{ 0xD2A9F59C, UCODE_F3DEX,    PATCH_NONE },
	{ 0xD41DB5F7, UCODE_F3DEX,    PATCH_NONE },
	{ 0xD57049A5, UCODE_F3DEX,    PATCH_NONE },
	{ 0xD802EC04, UCODE_F3DEX,    PATCH_NONE },
	{ 0xE89C2B92, UCODE_F3DEX,    PATCH_NONE },
	{ 0xE9231DF2, UCODE_F3DEX,    PATCH_NONE },
	{ 0xEC040469, UCODE_F3DEX,    PATCH_NONE },
	{ 0xEE47381B, UCODE_F3DEX,    PATCH_NONE },
	{ 0xFB816260, UCODE_F3DEX,    PATCH_NONE },
	{ 0x05165579, UCODE_F3DEX,    PATCH_NONE },
	{ 0x05777C62, UCODE_F3DEX,    PATCH_NONE },
	{ 0x057E7C62, UCODE_F3DEX,    PATCH_NONE },
	{ 0x5B5D3763, UCODE_F3DEX_WR, PATCH_NONE },
	{ 0x0FF79527, UCODE_F3DEX2,   PATCH_NONE },
	{ 0x168E9CD5, UCODE_F3DEX2,   PATCH_NONE },
	{ 0x1A1E18A0, UCODE_F3DEX2,   PATCH_NONE },
	{ 0x1A1E1920, UCODE_F3DEX2,   PATCH_NONE },
	{ 0x1A62DBAF, UCODE_F3DEX2,   PATCH_NONE },
	{ 0x1A62DC2F, UCODE_F3DEX2,   PATCH_NONE },
	{ 0x21F91834, UCODE_F3DEX2,   PATCH_NONE },
	{ 0x21F91874, UCODE_F3DEX2,   PATCH_NONE },
	{ 0x22099872, UCODE_F3DEX2,   PATCH_NONE },
	{ 0x2B291027, UCODE_F3DEX2,   PATCH_NONE },
	{ 0x2F71D1D5, UCODE_F3DEX2,   PATCH_NONE },
	{ 0x2F7DD1D5, UCODE_F3DEX2,   PATCH_NONE },
	{ 0x377359B6, UCODE_F3DEX2,   PATCH_NONE },
	{ 0x485ABFF2, UCODE_F3DEX2,   PATCH_NONE },
	{ 0x5D3099F1, UCODE_F3DEX2,   PATCH_NONE },
	{ 0x6124A508, UCODE_F3DEX2,   PATCH_NONE },
	{ 0x630A61FB, UCODE_F3DEX2,   PATCH_NONE },
	{ 0x65201989, UCODE_F3DEX2,   PATCH_NONE },
	{ 0x65201A09, UCODE_F3DEX2,   PATCH_NONE },
	{ 0x679E1205, UCODE_F3DEX2,   PATCH_NONE },
	{ 0x6D8F8F8A, UCODE_F3DEX2,   PATCH_NONE },
	{ 0x753BE4A5, UCODE_F3DEX2,   PATCH_NONE },
	{ 0x93D11F7B, UCODE_F3DEX2,   PATCH_NONE },
	{ 0x93D11FFB, UCODE_F3DEX2,   PATCH_NONE },
	{ 0x93D1FF7B, UCODE_F3DEX2,   PATCH_NONE },
	{ 0x9551177B, UCODE_F3DEX2,   PATCH_NONE },
	{ 0x955117FB, UCODE_F3DEX2,   PATCH_NONE },
	{ 0x95CD0062, UCODE_F3DEX2,   PATCH_NONE },
	{ 0xA2D0F88E, UCODE_F3DEX2,   PATCH_NONE },
	{ 0xAA86CB1D, UCODE_F3DEX2,   PATCH_NONE },
	{ 0xAAE4A5B9, UCODE_F3DEX2,   PATCH_NONE },
	{ 0xAD0A6292, UCODE_F3DEX2,   PATCH_NONE },
	{ 0xAD0A6312, UCODE_F3DEX2,   PATCH_NONE },
	{ 0xB62F900F, UCODE_F3DEX2,   PATCH_NONE },
	{ 0xBA65EA1E, UCODE_F3DEX2,   PATCH_NONE },
	{ 0xBC45382E, UCODE_F3DEX2,   PATCH_NONE },
	{ 0xC2A01ECF, UCODE_F3DEX2,   PATCH_NONE },
	{ 0xC901CE73, UCODE_F3DEX2,   PATCH_NONE },
	{ 0xC901CEF3, UCODE_F3DEX2,   PATCH_NONE },
	{ 0xCB8C9B6C, UCODE_F3DEX2,   PATCH_NONE },
	{ 0xCFA35A45, UCODE_F3DEX2,   PATCH_NONE },
	{ 0xDA13AB96, UCODE_F3DEX2,   PATCH_NONE },
	{ 0xDE7D67D4, UCODE_F3DEX2,   PATCH_NONE },
	{ 0xE1290FA2, UCODE_F3DEX2,   PATCH_NONE },
	{ 0xE65CB4AD, UCODE_F3DEX2,   PATCH_NONE },
	{ 0x03044B84, UCODE_F3DEX2,   PATCH_NONE },
	{ 0x030F4B84, UCODE_F3DEX2,   PATCH_NONE },
	{ 0xBA86CB1D, UCODE_F3DEXBG,  PATCH_NONE },
	{ 0xEF54EE35, UCODE_F3DTEXA,  PATCH_NONE },
	{ 0x5B5D36E3, UCODE_FACTOR5,  PATCH_NONE },
	{ 0x47D46E86, UCODE_PD,       PATCH_NONE },
	{ 0x1EA9E30F, UCODE_S2DEX,    PATCH_NONE },
	{ 0x299D5072, UCODE_S2DEX,    PATCH_NONE },
	{ 0x2B5A89C2, UCODE_S2DEX,    PATCH_NONE },
	{ 0x6BB745C9, UCODE_S2DEX,    PATCH_NONE },
	{ 0x74AF0A74, UCODE_S2DEX,    PATCH_NONE },
	{ 0x794C3E28, UCODE_S2DEX,    PATCH_NONE },
	{ 0xD20DEDBF, UCODE_S2DEX,    PATCH_NONE },
	{ 0x1F120BBB, UCODE_TURBO3D,  PATCH_NONE },
	{ 0xF9893F70, UCODE_TURBO3D,  PATCH_NONE },
	{ 0xFF372492, UCODE_TURBO3D,  PATCH_NONE },
	{ 0x0D7BBFFB, UCODE_UNKNOWN,  PATCH_NONE },
	{ 0x0FF795BF, UCODE_UNKNOWN,  PATCH_NONE },
	{ 0x844B55B5, UCODE_UNKNOWN,  PATCH_NONE },
	{ 0x86B1593E, UCODE_UNKNOWN,  PATCH_NONE },
	{ 0x8EC3E124, UCODE_UNKNOWN,  PATCH_NONE },
	{ 0xD5C4DC96, UCODE_UNKNOWN,  PATCH_NONE },
	{ 0x0BF36D36, UCODE_ZSORT,    PATCH_NONE },
	{ 0,          UCODE_UNKNOWN,  PATCH_NONE }
};