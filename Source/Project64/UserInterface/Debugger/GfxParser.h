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
    CGfxParser(CDebuggerUI* debugger);
    ~CGfxParser(void);
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

    bool             ExportDisplayListSource(const char* path);
    bool             ExportRawDisplayListSource(const char* path);
    bool             ExportSnapshot(const char* path);
    bool             ExportMicrocode(const char* path);

    bool             LoadVertices(uint32_t address, int index, int numv);
    void             LoadMatrix(uint32_t address, bool bPush, bool bLoad, bool bProjection);
    void             LoadLight(uint32_t address, int index);
    bool             GetCommand(uint32_t address, dl_cmd_t *command);
    int              GetCommands(uint32_t address, int numCommands, dl_cmd_t commands[]);
    vertex_t         Transform(vertex_t* vertex);

private:
	void Setup(uint32_t ucodeAddr, uint32_t dlistAddr, uint32_t dlistSize);
    void Step(void);

    CDebuggerUI* m_Debugger;

    std::vector<CHleGfxState>    m_StateLog;
    std::vector<decoded_cmd_t>   m_CommandLog;
	std::vector<dram_resource_t> m_RamResources;
    int m_CurrentMacroLength;
    size_t m_TriangleCount;
     
    dl_cmd_info_t   m_CommandArray[256];

    uint8_t*        m_RamSnapshot;
    size_t          m_RamSnapshotSize;

	ucode_info_t    m_MicrocodeInfo;

    uint32_t        m_MicrocodeAddress;
    uint32_t        m_RootDisplayListAddress;
    uint32_t        m_RootDisplayListSize;
    uint32_t        m_RootDisplayListEndAddress;
	uint32_t        m_VertexBufferSize;
};