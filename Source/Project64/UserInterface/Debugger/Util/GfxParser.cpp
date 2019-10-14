#include <stdafx.h>
#include "GfxOps.h"
#include "GfxParser.h"
#include "GfxState.h"
#include "GfxMicrocode.h"

CGfxParser::CGfxParser(void) :
	m_MicrocodeChecksum(0),
	m_MicrocodeInfo({ UCODE_UNKNOWN, NULL, NULL }),
	m_RootDListSize(0),
	m_RootDListEndAddress(0),
	m_VertexBufferSize(16),
	m_RamSnapshot(NULL),
	m_TriangleCount(0),
    m_CurrentMacroLength(0)
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
    new((CHleGfxState*)this) CHleGfxState();

	m_spCommandAddress = dlistAddr;
	m_RootDListSize = dlistSize;
	m_RootDListEndAddress = dlistAddr + dlistSize; // use as endpoint if ucode is unknown
	m_VertexBufferSize = 16;
	m_TriangleCount = 0;

	m_StateLog.clear();
	m_CommandLog.clear();
	m_RamResources.clear();

	if (m_RamSnapshot != NULL)
	{
        delete[] m_RamSnapshot;
	}

	uint32_t ramSize = g_MMU->RdramSize();
    m_RamSnapshot = new uint8_t[ramSize];
	memcpy(m_RamSnapshot, g_MMU->Rdram(), ramSize);

	uint8_t* ucode = &m_RamSnapshot[ucodeAddr];
	m_MicrocodeChecksum = GfxMicrocode::Identify(ucode, &m_MicrocodeInfo);
}

void CGfxParser::Run(uint32_t ucodeAddr, uint32_t dlistAddr, uint32_t dlistSize)
{
    testGeom.Clear();
	Setup(ucodeAddr, dlistAddr, dlistSize);

    while (!m_bDone)
    {
        Step();
    }

    VerifyCommands();
}

// decode and execute command
void CGfxParser::Step(void)
{
    bool bRead = GetCommand(m_spCommandAddress, &m_spCommand);

    if (!bRead)
    {
		printf("invalid command address? %08X %08X\n", m_spCommandAddress, SegmentedToPhysical(m_spCommandAddress));
        m_bDone = true;
        return;
    }

    m_StateLog.push_back(*(CHleGfxState*)this);

    const dl_cmd_info_t *commandInfo;
    decoded_cmd_t dc;

	memset(&dc, 0, sizeof(decoded_cmd_t));

    dc.dramResource.nCommand = m_CommandLog.size();
	dc.address = m_spCommandAddress;
	dc.rawCommand = m_spCommand;
	dc.virtualAddress = SegmentedToVirtual(dc.address);
	dc.name = "?";
	dc.params = "?";

	m_spCommandAddress += 8;

    commandInfo = CGfxOps::LookupCommand(CGfxOps::Commands_RDP, m_spCommand.commandByte);

    if (commandInfo == NULL && m_MicrocodeInfo.commandTable != NULL)
    {
        commandInfo = CGfxOps::LookupCommand(m_MicrocodeInfo.commandTable, m_spCommand.commandByte);
    }

    if (commandInfo != NULL)
    {
		dc.name = commandInfo->commandName;

		// execute command
		if (commandInfo->opFunc != NULL)
		{
			commandInfo->opFunc(this, &dc);
		}
    }
	
	if (m_MicrocodeInfo.version == UCODE_UNKNOWN && m_spCommandAddress >= m_RootDListEndAddress)
	{
        // if the ucode is unknown, only parse the root display list
		m_bDone = true;
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

    if (m_CurrentMacroLength > 0)
    {
        dc.name = "...";
        dc.params = "...";
        m_CurrentMacroLength--;
    }

    if (dc.macroLength > 0)
    {
        m_CurrentMacroLength = dc.macroLength;
    }

    m_CommandLog.push_back(dc);
}

ucode_version_t CGfxParser::GetUCodeVersion(void)
{
	return m_MicrocodeInfo.version;
}

const char* CGfxParser::GetUCodeName(void)
{
	return m_MicrocodeInfo.name;
}

uint32_t CGfxParser::GetUCodeChecksum(void)
{
	return m_MicrocodeChecksum;
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

#define VERIFIER_GBI_VERSION     "F3DEX_GBI_2"
#define VERIFIER_COMPILER_PATH   "gcc"
#define VERIFIER_GBI_HEADER_PATH "gbi.h"
#define VERIFIER_SRC_PATH        "dl_verify.c"
#define VERIFIER_DST_PATH        "dl_verify"

#define STRINGIZE(text) #text
#define VERIFIER_SRC_MAIN STRINGIZE(\
int main(void) \
{ \
	int dlistOk = 1;\
	for(int i = 0; i < length; i++)\
	{ \
		if (memcmp(&dlist[i], &dlist_raw[i], sizeof(Gfx)) != 0) \
		{ \
			printf("#%%d: generated: %%08X %%08X, actual: %%08X %%08X\n", \
				i, dlist[i].words.w0, dlist[i].words.w1, dlist_raw[i].words.w0, dlist_raw[i].words.w1); \
			return 1; \
		} \
	} \
	return 0; \
})

// recompile the decoded display list to check if the binary matches
void CGfxParser::VerifyCommands(void)
{
	int status;

	//system("cls");
	printf("writing dlist verification source to %s...\n", VERIFIER_SRC_PATH);

	FILE* fp = fopen(VERIFIER_SRC_PATH, "wb");

	fprintf(fp,
		"#include <stdio.h>\n"
		"#include <strings.h>\n"
		"#define %s\n"
		"#define _LANGUAGE_C\n"
		"#define _SHIFTL(v, s, w) (((unsigned int)(v) & ((1 << (w)) - 1)) << (s))\n"
		"typedef unsigned int u32;\n"
		"#include \"%s\"\n\n"
		"int length = %d;\n\n",
		VERIFIER_GBI_VERSION,
		VERIFIER_GBI_HEADER_PATH,
		m_CommandLog.size()
	);

	// macros
	fprintf(fp, "Gfx dlist[] = {\n\t/*vaddr    segoffs   command             macro*/\n");
	for (int i = 0; i < m_CommandLog.size(); i++)
	{
		decoded_cmd_t dc = m_CommandLog[i];

		fprintf(fp, "\t/* #%d: %08X %08X: %08X %08X*/",
			i, dc.virtualAddress, dc.address,
			dc.rawCommand.w0, dc.rawCommand.w1);

		if (strcmp("...", m_CommandLog[i].name) != 0)
		{
			fprintf(fp, " %s(%s),",
				dc.name, dc.params.c_str());
		}
		fprintf(fp, "\n");
	}
	fprintf(fp, "};\n\n");

	// strings
	fprintf(fp, "const char *dlist_strs[] = {\n");
	for (int i = 0; i < m_CommandLog.size(); i++)
	{
		decoded_cmd_t dc = m_CommandLog[i];
		if (strcmp("...", m_CommandLog[i].name) != 0)
		{
			fprintf(fp, "\"%s(%s)\",",
				dc.name, dc.params.c_str());
		}
		fprintf(fp, "\n");
	}
	fprintf(fp, "};\n\n");

	// raw numbers
	fprintf(fp, "Gfx dlist_raw[] = {\n");
	for (int i = 0; i < m_CommandLog.size(); i++)
	{
		decoded_cmd_t dc = m_CommandLog[i];
		fprintf(fp, "\t/*%08X %08X*/ { 0x%08X, 0x%08X },\n",
			dc.virtualAddress, dc.address,
			dc.rawCommand.w0, dc.rawCommand.w1);
	}
	fprintf(fp, "};\n\n");

	// main
	fprintf(fp, VERIFIER_SRC_MAIN);

	stdstr strCommand = stdstr_f("%s %s -o %s",
		VERIFIER_COMPILER_PATH, VERIFIER_SRC_PATH, VERIFIER_DST_PATH);

	fclose(fp);
	printf("compiling (%s)...\n", strCommand.c_str());
	status = system(strCommand.c_str());

	if (status != 0)
	{
		printf("error: compilation failed\n");
		return;
	}

	printf("running (%s)...\n", VERIFIER_DST_PATH);
	status = system(VERIFIER_DST_PATH);

	printf(status == 0 ? "dlist OK\n" : "dlist does not match\n");
}