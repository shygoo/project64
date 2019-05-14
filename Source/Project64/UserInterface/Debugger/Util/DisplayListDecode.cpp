#include <stdafx.h>
#include "DisplayListDecode.h"

const char* dec_NoParams(CHleDmemState*, char* paramsBuf)
{
    sprintf(paramsBuf, "");
    return NULL;
}

const char* dec_gsSPMoveWord_f3d(CHleDmemState* state, char* paramsBuf)
{
    dl_cmd_moveword_f3d_t* cmd = &state->command.moveword_f3d;

    if (cmd->index == 0x06) // MW_SEGMENT
    {
        sprintf(paramsBuf, "0x%02X, 0x%08X", cmd->offset / 4, (uint32_t)cmd->data);
        return "gsSPSegment";
    }

    if (cmd->index == 0x02 && cmd->offset == 0x00) // MW_NUMLIGHT, MWO_NUMLIGHT
    {
        int numLights = ((cmd->data - 0x80000000) / 32) - 1;
        sprintf(paramsBuf, "%d", numLights);
        return "gsSPNumLights";
    }

    return NULL;
}

const char* dec_gsSPMoveMem_f3d(CHleDmemState* state, char* paramsBuf)
{
    // gsDma2p G_MOVEMEM adrs len idx ofs 

    dl_cmd_movemem_f3d_t* cmd = &state->command.movemem_f3d;

    if (cmd->p == 0x80) // G_MV_VIEWPORT
    {
        // todo params
        sprintf(paramsBuf, "0x%08X // 0x%08X", cmd->address,
            state->SegmentedToVirtual(cmd->address));
        return "gsSPViewport";
    }

    if (cmd->p >= 0x86 && cmd->p <= 0x94) // G_MV_L0:7
    {
        int lightNumber = (cmd->p - 0x86) / 2;
        sprintf(paramsBuf, "0x%08X, %d // 0x%08X", cmd->address, lightNumber,
            state->SegmentedToVirtual(cmd->address));
        return "gsSPLight";
    }

    return NULL;
}

const char* dec_gsSPMoveWord_f3dex2(CHleDmemState* state, char* paramsBuf)
{
    dl_cmd_moveword_f3dex2_t* cmd = &state->command.moveword_f3dex2;

    if (cmd->index == 0x06) // MW_SEGMENT
    {
        sprintf(paramsBuf, "0x%02X, 0x%08X", cmd->offset / 4, (uint32_t)cmd->data);
        return "gsSPSegment";
    }

    return NULL;
}

const char* dec_gsSP1Triangle_f3d(CHleDmemState* state, char* paramsBuf)
{
    dl_cmd_tri1_f3d_t *cmd = &state->command.tri1_f3d;
    sprintf(paramsBuf, "%d, %d, %d", cmd->v0 / 10, cmd->v1 / 10, cmd->v2 / 10);
    return NULL;
}

const char* dec_gsSPVertex_f3d(CHleDmemState* state, char* paramsBuf)
{
    dl_cmd_vtx_f3d_t *cmd = &state->command.vtx_f3d;
    sprintf(paramsBuf, "0x%08X, %d, %d // 0x%08X", cmd->address, cmd->num + 1, cmd->idx,
        state->SegmentedToVirtual(cmd->address));
    return NULL;
}

const char* dec_gsSPSetGeometryMode_f3d(CHleDmemState* state, char* paramsBuf)
{
    bool havePrev = false;

    for (int i = 0; CDisplayListParser::GeometryModeNames[i].name != NULL; i++)
    {
        if (state->command.w1 & CDisplayListParser::GeometryModeNames[i].value)
        {
            if (havePrev)
            {
                paramsBuf += sprintf(paramsBuf, " | ");
            }

            paramsBuf += sprintf(paramsBuf, CDisplayListParser::GeometryModeNames[i].name);
            havePrev = true;
        }
    }
    return NULL;
}

const char* dec_gsSPDisplayList(CHleDmemState* state, char* paramsBuf)
{
    dl_cmd_dl_t *cmd = &state->command.dl;
    sprintf(paramsBuf, "0x%08X // 0x%08X", cmd->address,
        state->SegmentedToVirtual(cmd->address));

    if (cmd->branch)
    {
        return "gsSPBranchList";
    }

    return NULL;
}

const char* dec_HexParam32(CHleDmemState* state, char* paramsBuf)
{
    sprintf(paramsBuf, "0x%08X", state->command.w1);
    return NULL;
}

const char* dec_gsSPMatrix_f3d(CHleDmemState* state, char* paramsBuf)
{
    dl_cmd_mtx_f3d_t* cmd = &state->command.mtx_f3d;

    sprintf(paramsBuf, "0x%08X, (%s | %s | %s) // 0x%08X", cmd->address,
        (cmd->params & 1) ? "G_MTX_PROJECTION" : "G_MTX_MODELVIEW",
        (cmd->params & 2) ? "G_MTX_LOAD" : "G_MTX_MUL",
        (cmd->params & 4) ? "G_MTX_PUSH" : "G_MTX_NOPUSH",
        state->SegmentedToVirtual(cmd->address));

    return NULL;
}

const char* dec_gsDPFillRectangle(CHleDmemState* state, char* paramsBuf)
{
    dl_cmd_fillrect_t* cmd = &state->command.fillrect;
    sprintf(paramsBuf, "%d, %d, %d, %d", cmd->ulx >> 2, cmd->uly >> 2, cmd->lrx >> 2, cmd->lry >> 2);
    return NULL;
}

const char* dec_gsDPSetScissor(CHleDmemState* state, char* paramsBuf)
{
    dl_cmd_setscissor_t* cmd = &state->command.setscissor;

    const char* szMode = "?";

    switch (cmd->mode)
    {
    case 0: szMode = "G_SC_NON_INTERLACE"; break;
    case 2: szMode = "G_SC_EVEN_INTERLACE"; break;
    case 3: szMode = "G_SC_ODD_INTERLACE"; break;
    }

    // TODO check for frac bits and use gsDPSetScissorFrac if set

    sprintf(paramsBuf, "%s, %d, %d, %d, %d", szMode, cmd->ulx >> 2, cmd->uly >> 2, cmd->lrx >> 2, cmd->lry >> 2);
    return NULL;
}

const char* dec_gsDPSetTextureImage(CHleDmemState* state, char* paramsBuf)
{
    dl_cmd_settimg_t* cmd = &state->command.settimg;

    const char* fmtName = "?";
    const char* sizName = "?";

    fmtName = CDisplayListParser::LookupName(CDisplayListParser::ImageFormatNames, cmd->fmt);
    sizName = CDisplayListParser::LookupName(CDisplayListParser::TexelSizeNames, cmd->siz);

    sprintf(paramsBuf, "%s, %s, %d, 0x%08X // 0x%08X", fmtName, sizName, cmd->width + 1, cmd->address,
        state->SegmentedToVirtual(cmd->address));

    return NULL;
}

const char* dec_gsDPSetTile(CHleDmemState* state, char* paramsBuf)
{
    dl_cmd_settile_t* cmd = &state->command.settile;

    const char* fmtName = "?";
    const char* sizName = "?";

    fmtName = CDisplayListParser::LookupName(CDisplayListParser::ImageFormatNames, cmd->fmt);
    sizName = CDisplayListParser::LookupName(CDisplayListParser::TexelSizeNames, cmd->siz);

    sprintf(paramsBuf, "%s, %s, %d, 0x%03X, %d, %d, %d, %d, %d, %d, %d, %d", fmtName, sizName,
        cmd->line, cmd->tmem, cmd->tile, cmd->palette,
        cmd->cmt, cmd->maskt, cmd->shiftt,
        cmd->cms, cmd->masks, cmd->shifts);

    return NULL;
}

const char* dec_gsDPLoadBlock(CHleDmemState* state, char* paramsBuf)
{
    dl_cmd_loadblock_t* cmd = &state->command.loadblock;

    uint8_t siz = state->tiles[cmd->tile].siz;

    int bytesPerTexel = 0;

    switch (siz)
    {
    case 0: bytesPerTexel = 0; break; // G_IM_SIZ_4b
    case 1: bytesPerTexel = 1; break; // G_IM_SIZ_8b
    case 2: bytesPerTexel = 2; break; // G_IM_SIZ_16b
    case 3: bytesPerTexel = 4; break; // G_IM_SIZ_32b
    }

    int bytesPerLine = (0x800 / cmd->dxt) / sizeof(uint64_t);
    int width = bytesPerLine / bytesPerTexel;

    //sprintf(paramsBuf, "tile:%d, uls:%d, ult:%d, lrs:%d, dxt:%d // test %dx%d",
    //    cmd->tile, cmd->uls, cmd->ult, cmd->lrs, cmd->dxt,
    //    width, cmd->lrs);

    return NULL;
}

const char* dec_gsDPSetTileSize(CHleDmemState* state, char* paramsBuf)
{
    dl_cmd_settilesize_t* cmd = &state->command.settilesize;
    sprintf(paramsBuf, "tile:%d, uls:%d, ult:%d, lrs:%d, lrt:%d", cmd->tile, cmd->uls, cmd->ult, cmd->lrs, cmd->lrt);
    return NULL;
}

const char* dec_gsSPTexture_f3d(CHleDmemState* state, char* paramsBuf)
{
    dl_cmd_texture_f3d_t* cmd = &state->command.texture_f3d;
    sprintf(paramsBuf, "s:%d, t:%d, levels:%d, tile:%d, on:%d", cmd->s, cmd->t, cmd->level, cmd->tile, cmd->on);
    return NULL;
}
