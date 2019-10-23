#include <stdafx.h>
#include "GfxState.h"

CHleGfxState::CHleGfxState(void) :
    m_nCommand(0),
    m_spCommandAddress(0),
    m_spCommand({ 0 }),
    m_spStackIndex(0),
    m_dpOtherMode_h({ 0 }),
    m_dpOtherMode_l({ 0 }),
    m_dpCombiner({ 0 }),
    m_spGeometryMode({ 0 }),
    m_spNumLights(0),
    m_dpTextureImage(0),
    m_dpDepthImage(0),
    m_dpColorImage(0),
    m_dpTextureImageFmt(G_IM_FMT_RGBA),
    m_dpTextureImageSiz(G_IM_SIZ_4b),
    m_dpFillColor(0),
    m_dpFogColor(0),
    m_dpBlendColor(0),
    m_dpPrimColor(0),
    m_dpEnvColor(0),
    lastBlockLoadTexelSize(0),
    lastBlockLoadSize(0),
    m_bDone(0),
    m_spMatrixIndex(0),
    m_spProjectionMatrix({0})
{
    memset(m_spVertices, 0, sizeof(m_spVertices));
    memset(m_dpTileDescriptors, 0, sizeof(m_dpTileDescriptors));
    memset(m_spSegments, 0, sizeof(m_spSegments));
    memset(m_spStack, 0, sizeof(m_spStack));
    memset(m_spMatrixStack, 0, sizeof(m_spMatrixStack));
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

bool CHleGfxState::LoadVertices(uint32_t address, int index, int numv)
{
    uint32_t physAddr = SegmentedToPhysical(address);

    if (physAddr + numv * sizeof(m_spVertices[0]) >= g_MMU->RdramSize())
    {
        return false;
    }

    if (index + numv >= sizeof(m_spVertices) / sizeof(m_spVertices[0]))
    {
        return false;
    }

    uint8_t* ptr = g_MMU->Rdram() + physAddr;

    for (uint32_t i = 0; i < numv; i++)
    {
        vertex_t* vtx = &m_spVertices[index + i];
        uint32_t offs = i * 16;

        vtx->x = *(int16_t*)&ptr[(offs + 0) ^ 2];
        vtx->y = *(int16_t*)&ptr[(offs + 2) ^ 2];
        vtx->z = *(int16_t*)&ptr[(offs + 4) ^ 2];
        // todo the rest
    }

    return true;
}

void CHleGfxState::LoadMatrix(uint32_t address, bool bPush, bool bLoad, bool bProjection)
{
    uint32_t physAddr = SegmentedToPhysical(address);

    if (physAddr + sizeof(rsp_mtx_t) >= g_MMU->RdramSize())
    {
        return;
    }

    rsp_mtx_t dramSrcMtx;
    rsp_mtx_t* dmemSrcMtx;
    rsp_mtx_t* dmemDstMtx;
    
    uint8_t* ptr = g_MMU->Rdram() + physAddr;

    for(int i = 0; i < 4; i++)
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

vertex_t CHleGfxState::Transform(vertex_t* v)
{
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

bool CHleGfxState::GetCommand(uint32_t address, dl_cmd_t *command)
{
    uint32_t physAddress = SegmentedToPhysical(address);

    dl_cmd_t _command;
    bool bRead0 = g_MMU->LW_PAddr(physAddress, _command.w0);
    bool bRead1 = g_MMU->LW_PAddr(physAddress + 4, _command.w1);

    if (!bRead0 || !bRead1)
    {
        return false;
    }

    *command = _command;
    return true;
}

int CHleGfxState::GetCommands(uint32_t address, int numCommands, dl_cmd_t commands[])
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