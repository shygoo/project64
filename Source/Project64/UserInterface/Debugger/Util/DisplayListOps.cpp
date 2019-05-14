#include <stdafx.h>
#include "DisplayListOps.h"
#include "DisplayListParser.h"

void op_gsSPDisplayList(CHleDmemState* state)
{
    dl_cmd_dl_t* cmd = &state->command.dl;

    if (cmd->branch == 0)
    {
        state->stack[state->stackIndex++] = state->address;
    }
    state->address = cmd->address;
}

void op_gsSPEndDisplayList(CHleDmemState* state)
{
    if (state->stackIndex > 0)
    {
        state->address = state->stack[--state->stackIndex];
    }
    else
    {
        state->bDone = true;
    }
}

void op_gsSPMoveWord_f3d(CHleDmemState* state)
{
    dl_cmd_moveword_f3d_t* cmd = &state->command.moveword_f3d;

    if (cmd->index == 0x06) // MW_SEGMENT
    {
        int segno = cmd->offset / 4;
        uint32_t physAddress = cmd->data;
        state->segments[segno] = physAddress;
    }

    if (cmd->index == 0x02 && cmd->offset == 0x00) // MW_NUMLIGHT, MWO_NUMLIGHT
    {
        state->numLights = ((cmd->data - 0x80000000) / 32) - 1;
    }
}

void op_gsSPMoveWord_f3dex2(CHleDmemState* state)
{
    dl_cmd_moveword_f3dex2_t* cmd = &state->command.moveword_f3dex2;

    if (cmd->index == 0x06) // MW_SEGMENT
    {
        int segno = cmd->offset / 4;
        uint32_t physAddress = cmd->data;
        state->segments[segno] = physAddress;
    }
}

void op_gsSPTexture_f3d(CHleDmemState* state)
{
    dl_cmd_texture_f3d_t* cmd = &state->command.texture_f3d;
    hle_tile_descriptor_t* td = &state->tiles[cmd->tile];
    td->scaleS = cmd->s;
    td->scaleT = cmd->t;
    td->enabled = cmd->on;
    td->mipmapLevels = cmd->level;
}

void op_gsDPSetTile(CHleDmemState* state)
{
    dl_cmd_settile_t* cmd = &state->command.settile;
    hle_tile_descriptor_t *td = &state->tiles[cmd->tile];

    td->tmem = cmd->tmem;
    td->line = cmd->line;
    td->siz = cmd->siz;
    td->fmt = cmd->fmt;
    td->palette = cmd->palette;
    td->cmt = cmd->cmt;
    td->maskt = cmd->maskt;
    td->shiftt = cmd->shiftt;
    td->cms = cmd->cms;
    td->masks = cmd->masks;
    td->shifts = cmd->shifts;
}

void op_gsDPSetTextureImage(CHleDmemState* state)
{
    dl_cmd_settimg_t* cmd = &state->command.settimg;
    state->textureAddr = cmd->address; // todo are other params used?
}

void op_gsSPSetGeometryMode_f3d(CHleDmemState* state)
{
    dl_cmd_setgeometrymode_f3d_t* cmd = &state->command.setgeometrymode_f3d;
    state->geometryMode |= cmd->mode;
}

void op_gsSPClearGeometryMode_f3d(CHleDmemState* state)
{
    dl_cmd_setgeometrymode_f3d_t* cmd = &state->command.setgeometrymode_f3d;
    state->geometryMode &= ~cmd->mode;
}