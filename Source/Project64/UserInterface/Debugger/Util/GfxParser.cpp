#include <stdafx.h>
#include "GfxOps.h"
#include "GfxParser.h"
#include "GfxState.h"

CGfxParser::CGfxParser(void) :
	m_UCodeChecksum(0),
	m_UCodeVersion(UCODE_UNKNOWN),
	m_UCodeName(NULL),
	m_CommandTable(NULL),
	m_RootDListSize(0),
	m_RootDListEndAddress(0),
	m_VertexBufferSize(16),
	m_RamSnapshot(NULL),
	m_TriangleCount(0)
{
}

uint8_t* CGfxParser::GetRamSnapshot(void)
{
	return m_RamSnapshot;
}

CHleGfxState* CGfxParser::GetLoggedState(size_t index)
{
    if (index >= m_StateLog.size())
    {
        return NULL;
    }

    return &m_StateLog[index];
}

decoded_cmd_t* CGfxParser::GetLoggedCommand(size_t index)
{
	if (index >= m_CommandLog.size())
	{
		return NULL;
	}

	return &m_CommandLog[index];
}

dram_resource_t* CGfxParser::GetRamResource(size_t index)
{
	if (index >= m_RamResources.size())
	{
		return NULL;
	}

	return &m_RamResources[index];
}

size_t CGfxParser::GetCommandCount(void)
{
	return m_CommandLog.size();
}

size_t CGfxParser::GetRamResourceCount(void)
{
	return m_RamResources.size();
}

size_t CGfxParser::GetTriangleCount(void)
{
	return m_TriangleCount;
}

void CGfxParser::Setup(uint32_t ucodeAddr, uint32_t dlistAddr, uint32_t dlistSize)
{
	m_UCodeChecksum = 0;
	m_UCodeVersion = UCODE_UNKNOWN;
	m_UCodeName = "unknown microcode";
	m_CommandTable = NULL;
	m_RootDListSize = dlistSize;
	m_RootDListEndAddress = dlistAddr + dlistSize; // use as endpoint if ucode is unknown
	m_VertexBufferSize = 16;
	m_TriangleCount = 0;

	m_StateLog.clear();
	m_CommandLog.clear();
	m_RamResources.clear();

	if (m_RamSnapshot != NULL)
	{
		free(m_RamSnapshot);
	}

	uint32_t ramSize = g_MMU->RdramSize();
	m_RamSnapshot = (uint8_t*)malloc(ramSize);
	memcpy(m_RamSnapshot, g_MMU->Rdram(), ramSize);

	memset(&m_State, 0, sizeof(m_State));
	m_State.m_Address = dlistAddr;

    uint8_t* ucode = m_RamSnapshot + ucodeAddr;

    for (int i = 0; i < 3072; i += sizeof(uint32_t))
    {
        m_UCodeChecksum += *(uint32_t*)&ucode[i];
    }

    const ucode_info_t* info = CGfxOps::LookupMicrocode(m_UCodeChecksum);

    if (info != NULL)
    {
		m_UCodeVersion = info->version;
		m_UCodeName = info->name;
		m_CommandTable = info->commandTable;
    }
}

void CGfxParser::Run(uint32_t ucodeAddr, uint32_t dlistAddr, uint32_t dlistSize)
{
    testGeom.Clear();
	Setup(ucodeAddr, dlistAddr, dlistSize);

    while (!m_State.bDone)
    {
        Step();
    }
}

// decode and execute command
void CGfxParser::Step(void)
{
    uint32_t physAddress = m_State.SegmentedToPhysical(m_State.m_Address);

    g_MMU->LW_PAddr(physAddress, m_State.m_Command.w0);
    g_MMU->LW_PAddr(physAddress + 4, m_State.m_Command.w1);

	// todo dump states to file instead?
    m_StateLog.push_back(m_State);
    
    uint8_t commandByte = m_State.m_Command.w0 >> 24;

    const dl_cmd_info_t *commandInfo;
    decoded_cmd_t dc;

	memset(&dc, 0, sizeof(decoded_cmd_t));

    dc.dramResource.nCommand = m_CommandLog.size();
	dc.address = m_State.m_Address;
	dc.rawCommand = m_State.m_Command;
	dc.virtualAddress = m_State.SegmentedToVirtual(dc.address);
	dc.name = "?";
	dc.params = "?";

	m_State.m_Address += 8;

    commandInfo = CGfxOps::LookupCommand(CGfxOps::Commands_RDP, commandByte);

    if (commandInfo == NULL && m_CommandTable != NULL)
    {
        commandInfo = CGfxOps::LookupCommand(m_CommandTable, commandByte);
    }

    if (commandInfo != NULL)
    {
		dc.name = commandInfo->commandName;

		// execute command
		if (commandInfo->opFunc != NULL)
		{
			commandInfo->opFunc(&m_State, &dc);
		}
    }
	
	if (m_UCodeVersion == UCODE_UNKNOWN && m_State.m_Address >= m_RootDListEndAddress)
	{
		m_State.bDone = true;
	}
    
    if (dc.numTris != 0)
    {
        for (int i = 0; i < dc.numTris; i++)
        {
            CVec4 v0, v1, v2;
            // N64VertexToVec3
            v0 = { (float)dc.tris[i].v0.x * 50 / 0x7FFF, (float)dc.tris[i].v0.y * 50 / 0x7FFF, (float)dc.tris[i].v0.z * 50 / 0x7FFF };
            v1 = { (float)dc.tris[i].v1.x * 50 / 0x7FFF, (float)dc.tris[i].v1.y * 50 / 0x7FFF, (float)dc.tris[i].v1.z * 50 / 0x7FFF };
            v2 = { (float)dc.tris[i].v2.x * 50 / 0x7FFF, (float)dc.tris[i].v2.y * 50 / 0x7FFF, (float)dc.tris[i].v2.z * 50 / 0x7FFF };
            int v0idx = testGeom.AddVertexUnique(v0);
            int v1idx = testGeom.AddVertexUnique(v1);
            int v2idx = testGeom.AddVertexUnique(v2);
            testGeom.AddTriangleRef(v0idx, v1idx, v2idx, m_CommandLog.size());
        }

        m_TriangleCount += dc.numTris;
    }

	if (dc.dramResource.type != RES_NONE)
	{
		m_RamResources.push_back(dc.dramResource);
	}

    m_CommandLog.push_back(dc);
}

ucode_version_t CGfxParser::GetUCodeVersion(void)
{
	return m_UCodeVersion;
}

const char* CGfxParser::GetUCodeName(void)
{
	return m_UCodeName;
}

uint32_t CGfxParser::GetUCodeChecksum(void)
{
	return m_UCodeChecksum;
}

bool CGfxParser::ConvertImage(uint32_t* dst, uint8_t* src, im_fmt_t fmt, im_siz_t siz, int numTexels)
{
	if (fmt == G_IM_FMT_RGBA && siz == G_IM_SIZ_16b)
	{
		for (int i = 0; i < numTexels; i++)
		{
			uint16_t px = *(uint16_t*)&src[(i * 2) ^ 2];
			uint8_t r = (uint8_t)((px >> 11) & 0x1F) * (255.0f / 32.0f);
			uint8_t g = (uint8_t)((px >> 6) & 0x1F) * (255.0f / 32.0f);
			uint8_t b = (uint8_t)((px >> 1) & 0x1F) * (255.0f / 32.0f);
			uint8_t alpha = (px & 1) * 255;
			dst[i] = alpha << 24 | (r << 16) | (g << 8) | (b << 0);
		}
	}
	else if (fmt == G_IM_FMT_IA && siz == G_IM_SIZ_16b)
	{
		for (int i = 0; i < numTexels; i++)
		{
			uint16_t px = *(uint16_t*)&src[(i * 2) ^ 2];
			uint8_t intensity = (px >> 8);
			uint8_t alpha = px & 0xFF;
			dst[i] = (alpha << 24) | (intensity << 16) | (intensity << 8) | (intensity << 0);
		}
	}
	else if (fmt == G_IM_FMT_IA && siz == G_IM_SIZ_8b)
	{
		for (int i = 0; i < numTexels; i++)
		{
			uint8_t px = src[i ^ 3];
			uint8_t intensity = (px >> 4) * 0x11;
			uint8_t alpha = (px & 0xF) * 0x11;
			dst[i] = (alpha << 24) | (intensity << 16) | (intensity << 8) | (intensity << 0);
		}
	}
	else if (fmt == G_IM_FMT_I && siz == G_IM_SIZ_8b)
	{
		for (int i = 0; i < numTexels; i++)
		{
			uint8_t intensity = src[i ^ 3];
			dst[i] = (0xFF << 24) | (intensity << 16) | (intensity << 8) | (intensity << 0);
		}
	}
	else
	{
		return false;
		//MessageBox(NULL, stdstr_f("unhandled image texel/fmt combo fmt:%d siz:%d", res->imageFmt, res->imageSiz).c_str(), MB_OK);
	}

	return true;
}