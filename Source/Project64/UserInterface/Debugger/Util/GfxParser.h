#pragma once
#include <stdafx.h>
#include "GfxRenderer.h"
#include "GfxOpTypes.h"
#include "GfxOps.h"
#include "GfxState.h"
#include "GfxMicrocode.h"

class CGfxParser : public CHleGfxState
{
	//friend class CGfxOps;
public:
    CGfxParser(void);
	static bool ConvertImage(uint32_t* dst, uint8_t *src, im_fmt_t fmt, im_siz_t siz, int numTexels);
    
    CBasicMeshGeometry testGeom;

public:
    void             Run(uint32_t ucodeAddr, uint32_t dlistAddr, uint32_t dlistSize);
    void             VerifyCommands(void);
	uint8_t*         GetRamSnapshot(void);

	ucode_info_t*    GetMicrocodeInfo(void);

    CHleGfxState*    GetLoggedState(size_t index);
	decoded_cmd_t*   GetLoggedCommand(size_t index);
	dram_resource_t* GetRamResource(size_t index);
	size_t           GetCommandCount(void);
	size_t           GetRamResourceCount(void);
	size_t           GetTriangleCount(void);

private:
	void Setup(uint32_t ucodeAddr, uint32_t dlistAddr, uint32_t dlistSize);
    void Step(void);

    std::vector<CHleGfxState>    m_StateLog;
    std::vector<decoded_cmd_t>   m_CommandLog;
	std::vector<dram_resource_t> m_RamResources;
    int m_CurrentMacroLength;

    dl_cmd_info_t m_CommandArray[256];
    uint8_t* m_RamSnapshot;

	size_t          m_TriangleCount;

	ucode_info_t    m_MicrocodeInfo;
    uint32_t        m_RootDListSize;
    uint32_t        m_RootDListEndAddress;
	uint32_t        m_VertexBufferSize;
};