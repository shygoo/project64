#include <stdafx.h>
#include "GfxState.h"

uint32_t CHleGfxState::SegmentedToPhysical(uint32_t segaddr)
{
	uint32_t segment = (segaddr >> 24) & 0x0F;
	uint32_t offset = segaddr & 0x00FFFFFF;
	return m_Segments[segment] + offset;
}

uint32_t CHleGfxState::SegmentedToVirtual(uint32_t segaddr)
{
	return SegmentedToPhysical(segaddr) | 0x80000000;
}

bool CHleGfxState::LoadVertices(uint32_t address, int index, int numv)
{
    uint32_t physAddr = SegmentedToPhysical(address);

    if (physAddr + numv * sizeof(m_Vertices[0]) >= g_MMU->RdramSize())
    {
        return false;
    }

    if (index + numv >= sizeof(m_Vertices) / sizeof(m_Vertices[0]))
    {
        return false;
    }

    uint8_t* ptr = g_MMU->Rdram() + physAddr;

    for (uint32_t i = 0; i < numv; i++)
    {
        vertex_t* vtx = &m_Vertices[index + i];
        uint32_t offs = i * 16;

        vtx->x = *(int16_t*)&ptr[(offs + 0) ^ 2];
        vtx->y = *(int16_t*)&ptr[(offs + 2) ^ 2];
        vtx->z = *(int16_t*)&ptr[(offs + 4) ^ 2];
        // todo the rest
    }

    return true;
}

CHleGfxState::CHleGfxState(void) :
    m_nCommand(0),
    m_Address(0),
    m_Command({0}),
    m_StackIndex(0),
    m_OtherMode_h({0}),
    m_OtherMode_l({0}),
    m_Combiner({0}),
    m_GeometryMode({0}),
    m_NumLights(0),
    m_TextureImage(0),
    m_DepthImage(0),
    m_ColorImage(0),
    m_TextureImageFmt(G_IM_FMT_RGBA),
    m_TextureImageSiz(G_IM_SIZ_4b),
    m_FillColor(0),
    m_FogColor(0),
    m_BlendColor(0),
    m_PrimColor(0),
    m_EnvColor(0),
    lastBlockLoadTexelSize(0),
    lastBlockLoadSize(0),
    m_bDone(0)
{
    memset(m_Vertices, 0, sizeof(m_Vertices));
    memset(m_Tiles, 0, sizeof(m_Tiles));
    memset(m_Segments, 0, sizeof(m_Segments));
    memset(m_Stack, 0, sizeof(m_Stack));
}
