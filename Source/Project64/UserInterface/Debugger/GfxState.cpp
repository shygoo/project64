#include <stdafx.h>
#include "GfxState.h"
#include "DebugMMU.h"

CHleGfxState::CHleGfxState(void)
{
    ClearState();
}

void CHleGfxState::ClearState(void)
{
    m_nCommand = 0;
    m_spCommandAddress = 0;
    m_spCommand = { 0 };
    m_spStackIndex = 0;
    m_dpOtherMode_h = { 0 };
    m_dpOtherMode_l = { 0 };
    m_dpCombiner = { 0 };
    m_spGeometryMode = { 0 };
    m_spNumLights = 0;
    m_dpTextureImage = 0;
    m_dpDepthImage = 0;
    m_dpColorImage = 0;
    m_dpTextureImageFmt = G_IM_FMT_RGBA;
    m_dpTextureImageSiz = G_IM_SIZ_4b;
    m_dpFillColor = 0;
    m_dpFogColor = 0;
    m_dpBlendColor = 0;
    m_dpPrimColor = 0;
    m_dpEnvColor = 0;
    lastBlockLoadTexelSize = 0;
    lastBlockLoadSize = 0;
    m_bDone = 0;
    m_spMatrixIndex = 0;
    m_spProjectionMatrix = { 0 };

    memset(m_spVertices, 0, sizeof(m_spVertices));
    memset(m_dpTileDescriptors, 0, sizeof(m_dpTileDescriptors));
    memset(m_spSegments, 0, sizeof(m_spSegments));
    memset(m_spStack, 0, sizeof(m_spStack));
    memset(m_spMatrixStack, 0, sizeof(m_spMatrixStack));
    memset(m_spLights, 0, sizeof(m_spLights));
}

uint32_t CHleGfxState::SegmentedToPhysical(uint32_t segaddr)
{
    uint32_t segment = (segaddr >> 24) & 0x0F;
    uint32_t offset = segaddr & 0x00FFFFFF;
    return (m_spSegments[segment] & 0x1FFFFFFF) + offset;
}

uint32_t CHleGfxState::SegmentedToVirtual(uint32_t segaddr)
{
    return SegmentedToPhysical(segaddr) | 0x80000000;
}

int32_t rsp_mtx_t::Get(int row, int col)
{
    return (intpart[row][col] << 16) | fracpart[row][col];
}

float rsp_mtx_t::GetF(int row, int col)
{
    return Get(row, col) / (float)0x10000;
}

int32_t rsp_mtx_t::MtxDotProduct(rsp_mtx_t* a, int rowA, rsp_mtx_t* b, int colB)
{
    int64_t res = 0;
    for (int i = 0; i < 4; i++)
    {
        res += a->Get(rowA, i) * b->Get(i, colB);
    }
    return (int32_t)(res >> 16);
}

rsp_mtx_t rsp_mtx_t::Mul(rsp_mtx_t* b)
{
    rsp_mtx_t res;

    for (int row = 0; row < 4; row++)
    {
        for (int col = 0; col < 4; col++)
        {
            int32_t dp = MtxDotProduct(this, row, b, col);
            res.intpart[row][col] = dp >> 16;
            res.fracpart[row][col] = dp & 0xFFFF;
        }
    }

    return res;
}