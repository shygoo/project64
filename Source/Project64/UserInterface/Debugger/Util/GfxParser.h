#pragma once
#include <stdafx.h>
#include "GfxRenderer.h"
#include "GfxOpTypes.h"
#include "GfxOps.h"
#include "GfxState.h"

class CGfxParser : CHleGfxState
{
public:
    CGfxParser(void);
	static bool ConvertImage(uint32_t* dst, uint8_t *src, im_fmt_t fmt, im_siz_t siz, int numTexels);
    
    CBasicMeshGeometry testGeom;

public:
    void             Run(uint32_t ucodeAddr, uint32_t dlistAddr, uint32_t dlistSize);
	uint8_t*         GetRamSnapshot(void);
	ucode_version_t  GetUCodeVersion(void);
	const char*      GetUCodeName(void);
	uint32_t         GetUCodeChecksum(void);
    CHleGfxState*    GetLoggedState(size_t index);
	decoded_cmd_t*   GetLoggedCommand(size_t index);
	dram_resource_t* GetRamResource(size_t index);
	size_t           GetCommandCount(void);
	size_t           GetRamResourceCount(void);
	size_t           GetTriangleCount(void);

private:
	void Setup(uint32_t ucodeAddr, uint32_t dlistAddr, uint32_t dlistSize);
    void Step();

    uint8_t* m_RamSnapshot;
    //CScene   m_Scene;

    std::vector<CHleGfxState>    m_StateLog;
    std::vector<decoded_cmd_t>   m_CommandLog;
	std::vector<dram_resource_t> m_RamResources;

	size_t          m_TriangleCount;

	dl_cmd_info_t*  m_CommandTable;
    uint32_t        m_UCodeChecksum;
    ucode_version_t m_UCodeVersion;
    const char*     m_UCodeName;
    uint32_t        m_RootDListSize;
    uint32_t        m_RootDListEndAddress;
	uint32_t        m_VertexBufferSize;
};