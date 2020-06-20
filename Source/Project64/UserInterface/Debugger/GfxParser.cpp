#include <stdafx.h>
#include <iostream>
#include <fstream>

#include "GfxOps.h"
#include "GfxParser.h"
#include "GfxState.h"
#include "GfxMicrocode.h"

CGfxParser::CGfxParser(CDebuggerUI* debugger) :
    m_Debugger(debugger),
    m_MicrocodeInfo({ 0 }),
    m_MicrocodeAddress(0),
    m_RootDisplayListSize(0),
    m_RootDisplayListAddress(0),
    m_RootDisplayListEndAddress(0),
    m_VertexBufferSize(16),
    m_TriangleCount(0),
    m_CurrentMacroLength(0)
{
}

CGfxParser::~CGfxParser(void)
{
    if (m_RamSnapshot != NULL)
    {
        delete[] m_RamSnapshot;
    }
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
    ClearState();

    m_spCommandAddress = dlistAddr;

    m_MicrocodeAddress = ucodeAddr;
    m_RootDisplayListAddress = dlistAddr;
    m_RootDisplayListSize = dlistSize;
    m_RootDisplayListEndAddress = dlistAddr + dlistSize; // use as endpoint if microcode is unknown
    m_VertexBufferSize = 16;
    m_TriangleCount = 0;

    m_StateLog.clear();
    m_CommandLog.clear();
    m_RamResources.clear();

    if (m_RamSnapshot != NULL)
    {
        delete[] m_RamSnapshot;
    }

    m_RamSnapshotSize = g_MMU->RdramSize();
    m_RamSnapshot = new uint8_t[m_RamSnapshotSize];
    memcpy(m_RamSnapshot, g_MMU->Rdram(), m_RamSnapshotSize);

    uint8_t* ucode = &m_RamSnapshot[ucodeAddr];
    CGfxMicrocode::BuildArray(ucode, &m_MicrocodeInfo, m_CommandArray);
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

    dl_cmd_info_t* commandInfo;
    decoded_cmd_t dc;

    memset(&dc, 0, sizeof(decoded_cmd_t));

    dc.dramResource.nCommand = m_CommandLog.size();
    dc.address = m_spCommandAddress;
    dc.rawCommand = m_spCommand;
    dc.virtualAddress = SegmentedToVirtual(dc.address);
    dc.name = "?";
    dc.params = stdstr("?");

    m_spCommandAddress += 8;
    commandInfo = &m_CommandArray[m_spCommand.commandByte];

    if (commandInfo->commandName != NULL && commandInfo->opFunc != NULL)
    {
        dc.name = commandInfo->commandName;
        commandInfo->opFunc(this, &dc);
    }
    else
    {
        dc.name = "gsUnknown";
        dc.params = stdstr_f("0x%08X, 0x%08X", m_spCommand.w0, m_spCommand.w1);
        dc.listBgColor = RGB(0xFF, 0xCC, 0xCC);
    }

    if (m_MicrocodeInfo.ucodeId == UCODE_UNKNOWN &&
        m_spCommandAddress >= m_RootDisplayListEndAddress)
    {
        // if the ucode is unknown, only parse the rdp commands from the root display list
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
        dc.params = stdstr("...");
        m_CurrentMacroLength--;
    }

    if (dc.macroLength > 0)
    {
        m_CurrentMacroLength = dc.macroLength;
    }

    m_CommandLog.push_back(dc);

    printf("%s %s\n", dc.name, dc.params.c_str());
}


ucode_info_t* CGfxParser::GetMicrocodeInfo(void)
{
	return &m_MicrocodeInfo;
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

/*
dlist_verifier.c
------------------------------------
#include <stdio.h>
#include <strings.h>

#define _LANGUAGE_C
#define gsUnknown(w0, w1) {(w0), (w1)}
#define _SHIFTL(v, s, w) (((unsigned int)(v) & ((1 << (w)) - 1)) << (s))
typedef unsigned int u32;

#include "gbi_version_def.inc.c"
#include "gbi.h"
#include "gbi_redefs.h"

Gfx dlistMacros[] = {
#include "dlist_macros.inc.c"
};

Gfx dlistRaw[] = {
#include "dlist_raw.inc.c"
};

int main(void)
{
	int dlistOk = 1;
	
	int rawLength = sizeof(dlistRaw) / sizeof(Gfx);
	int macroLength = sizeof(dlistRaw) / sizeof(Gfx);
	
	for(int i = 0; i < rawLength; i++)
	{
		if(memcmp(&dlistRaw[i], &dlistMacros[i], sizeof(Gfx)) != 0)
		{
			printf("#%d: generated: %08X %08X, actual: %08X %08X\n",
				i, dlistMacros[i].words.w0, dlistMacros[i].words.w1, dlistRaw[i].words.w0, dlistRaw[i].words.w1);
			return 1;
		}
	}

	return 0;
}

*/

// recompile the decoded display list to check if the binary matches
void CGfxParser::VerifyCommands(void)
{
    int status;
    ucode_info_t& info = m_MicrocodeInfo;

    ExportDisplayListSource("dlist_macros.inc.c");
    ExportRawDisplayListSource("dlist_raw.inc.c");

    std::ofstream gbiVersionFile;
	gbiVersionFile.open("gbi_version_def.inc.c", std::ios::out | std::ios::binary);

    if (info.ucodeVersionDef != NULL)
    {
        gbiVersionFile << "#define " << info.ucodeVersionDef << "\n";
    }

    if (info.patchVersionDef != NULL)
    {
        gbiVersionFile << "#define " << info.patchVersionDef << "\n";
    }

	gbiVersionFile.close();

	stdstr strCommand = stdstr_f("gcc dlist_verifier.c -o dlist_verifier");

    printf("compiling (%s)...\n", strCommand.c_str());
    status = system(strCommand.c_str());

    if (status != 0)
    {
        printf("error: compilation failed\n");
        return;
    }

    printf("running (%s)...\n", "dlist_verifier");
    status = system("dlist_verifier");

    std::remove("dlist_verifier.exe");
    std::remove("gbi_version_def.inc.c");

    printf(status == 0 ? "dlist OK\n" : "dlist does not match\n");
}

bool CGfxParser::ExportDisplayListSource(const char* path)
{
    ucode_info_t& info = m_MicrocodeInfo;

    std::ofstream file;
    file.open(path, std::ios::out | std::ios::binary);

    if (!file.is_open())
    {
        return false;
    }

    if (info.ucodeVersionDef != NULL)
    {
        file << "// " << info.ucodeVersionDef << "\n";
    }

    if (info.patchVersionDef != NULL)
    {
        file << "// " << info.patchVersionDef << "\n";
    }

    for (int i = 0; i < m_CommandLog.size(); i++)
    {
        decoded_cmd_t dc = m_CommandLog[i];

        file << stdstr_f("\t/* #%d: %08X %08X: %08X %08X*/",
            i, dc.virtualAddress, dc.address, dc.rawCommand.w0, dc.rawCommand.w1);

        if (strcmp("...", m_CommandLog[i].name) != 0)
        {
            file << stdstr_f(" %s(%s),", dc.name, dc.params.c_str());
        }

        file << "\n";
    }

    file.close();
}

bool CGfxParser::ExportRawDisplayListSource(const char* path)
{
    std::ofstream file;
    file.open(path, std::ios::out | std::ios::binary);

    if (!file.is_open())
    {
        return false;
    }

    for (int i = 0; i < m_CommandLog.size(); i++)
    {
        decoded_cmd_t dc = m_CommandLog[i];

        file << stdstr_f("\t/*%08X %08X*/{ 0x%08X, 0x%08X },\n",
            dc.virtualAddress, dc.address,
            dc.rawCommand.w0, dc.rawCommand.w1);
    }

    file.close();
}

bool CGfxParser::ExportSnapshot(const char* path)
{
    struct
    {
        uint32_t microcodeAddress;
        uint32_t microcodeChecksum;
        uint32_t rootDisplayListAddress;
        uint32_t rootDisplayListSize;
    } footer;

    footer.microcodeAddress = m_MicrocodeAddress;
    footer.microcodeChecksum = m_MicrocodeInfo.checksum;
    footer.rootDisplayListAddress = m_RootDisplayListAddress;
    footer.rootDisplayListSize = m_RootDisplayListSize;

    std::ofstream file;
    file.open(path, std::ios::out | std::ios::binary);

    if (!file.is_open())
    {
        return false;
    }

    char* ramSnapshotBE = new char[m_RamSnapshotSize];

    for (size_t i = 0; i < m_RamSnapshotSize; i += sizeof(uint32_t))
    {
        *(uint32_t*)&ramSnapshotBE[i] = _byteswap_ulong(*(uint32_t*)&m_RamSnapshot[i]);
    }

    file.write(ramSnapshotBE, m_RamSnapshotSize);
    file.write("GINF", 4);
    file.write((const char*)&footer, sizeof(footer));
    file.close();

    delete[] ramSnapshotBE;
}

bool CGfxParser::ExportMicrocode(const char* path)
{
    std::ofstream file;
    file.open(path, std::ios::out | std::ios::binary);

    if (!file.is_open())
    {
        return false;
    }

    if (m_MicrocodeAddress + 4096 >= m_RamSnapshotSize)
    {
        return false;
    }

    char* ucodeBE = new char[4096];

    for (size_t i = 0; i < 4096; i += sizeof(uint32_t))
    {
        *(uint32_t*)&ucodeBE[i] = _byteswap_ulong(*(uint32_t*)&m_RamSnapshot[m_MicrocodeAddress + i]);
    }

    file.write(ucodeBE, 4096);
    file.close();
    delete[] ucodeBE;
}

bool CGfxParser::LoadVertices(uint32_t address, int index, int numv)
{
    uint32_t physAddr = SegmentedToPhysical(address);

    if (physAddr + numv * sizeof(vertex_t) > m_RamSnapshotSize)
    {
        return false;
    }

    if (index + numv >= sizeof(m_spVertices) / sizeof(m_spVertices[0]))
    {
        return false;
    }

    uint8_t* ptr = &m_RamSnapshot[physAddr];

    for (uint32_t i = 0; i < numv; i++)
    {
        vertex_t* vtx = &m_spVertices[index + i];
        uint32_t offs = i * 16;

        vtx->x = *(int16_t*)&ptr[(offs + 0) ^ 2];
        vtx->y = *(int16_t*)&ptr[(offs + 2) ^ 2];
        vtx->z = *(int16_t*)&ptr[(offs + 4) ^ 2];

        *vtx = Transform(vtx);
        // todo the rest
    }

    return true;
}

void CGfxParser::LoadMatrix(uint32_t address, bool bPush, bool bLoad, bool bProjection)
{
    uint32_t physAddr = SegmentedToPhysical(address);

    if (physAddr + sizeof(rsp_mtx_t) >= m_RamSnapshotSize)
    {
        return;
    }

    rsp_mtx_t dramSrcMtx;
    rsp_mtx_t* dmemSrcMtx;
    rsp_mtx_t* dmemDstMtx;

    uint8_t* ptr = &m_RamSnapshot[physAddr];

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            uint32_t offs = (i * 8) + (j * 2);
            dramSrcMtx.intpart[i][j] = *(int16_t*)&ptr[(offs + 0) ^ 2];
            dramSrcMtx.fracpart[i][j] = *(int16_t*)&ptr[(offs + 32) ^ 2];
        }
    }

    if (bProjection)
    {
        dmemSrcMtx = &m_spProjectionMatrix;
        dmemDstMtx = &m_spProjectionMatrix;
    }
    else // modelview
    {
        dmemSrcMtx = &m_spMatrixStack[m_spMatrixIndex];

        if (bPush)
        {
            m_spMatrixIndex++;
        }

        if (m_spMatrixIndex >= sizeof(m_spMatrixStack) / sizeof(rsp_mtx_t))
        {
            printf("matrix stack overrun\n");
            m_bDone = true;
            return;
        }

        dmemDstMtx = &m_spMatrixStack[m_spMatrixIndex];
    }

    if (bLoad)
    {
        *dmemDstMtx = dramSrcMtx;
    }
    else // mul
    {
        *dmemDstMtx = dmemSrcMtx->Mul(&dramSrcMtx);
    }
}

void CGfxParser::LoadLight(uint32_t address, int index)
{
    uint32_t physAddr = SegmentedToPhysical(address);

    if (physAddr + sizeof(light_t) >= m_RamSnapshotSize)
    {
        return;
    }

    if (index < 0 || index > 9)
    {
        return;
    }

    light_t* light = &m_spLights[index];
    uint8_t* ptr = &m_RamSnapshot[physAddr];

    light->colorA[0] = ptr[0 ^ 3];
    light->colorA[1] = ptr[1 ^ 3];
    light->colorA[2] = ptr[2 ^ 3];

    light->colorB[0] = ptr[4 ^ 3];
    light->colorB[1] = ptr[5 ^ 3];
    light->colorB[2] = ptr[6 ^ 3];

    light->direction[0] = ptr[8 ^ 3];
    light->direction[1] = ptr[9 ^ 3];
    light->direction[2] = ptr[10 ^ 3];
}

vertex_t CGfxParser::Transform(vertex_t* v)
{
    if (m_spMatrixIndex >= sizeof(m_spMatrixStack) / sizeof(rsp_mtx_t))
    {
        printf("CGfxParser::Transform mtx stack overrun\n");
        return {0,0,0};
    }

    rsp_mtx_t* mtx = &m_spMatrixStack[m_spMatrixIndex];

    int16_t x = v->x * mtx->GetF(0, 0) + v->y * mtx->GetF(1, 0) + v->z * mtx->GetF(2, 0) + 1 * mtx->GetF(3, 0);
    int16_t y = v->x * mtx->GetF(0, 1) + v->y * mtx->GetF(1, 1) + v->z * mtx->GetF(2, 1) + 1 * mtx->GetF(3, 1);
    int16_t z = v->x * mtx->GetF(0, 2) + v->y * mtx->GetF(1, 2) + v->z * mtx->GetF(2, 2) + 1 * mtx->GetF(3, 2);

    vertex_t _v = *v;
    _v.x = (float)x;
    _v.y = (float)y;
    _v.z = (float)z;

    return _v;
}

bool CGfxParser::GetCommand(uint32_t address, dl_cmd_t *command)
{
    uint32_t physAddress = SegmentedToPhysical(address);

    if (physAddress + sizeof(dl_cmd_t) > m_RamSnapshotSize)
    {
        return false;
    }

    *command = *(dl_cmd_t*)&m_RamSnapshot[physAddress];
    return true;
}

int CGfxParser::GetCommands(uint32_t address, int numCommands, dl_cmd_t commands[])
{
    int nRead = 0;

    for (int i = 0; i < numCommands; i++)
    {
        if (!GetCommand(address + i * 8, &commands[i]))
        {
            break;
        }
        nRead++;
    }

    return nRead;
}