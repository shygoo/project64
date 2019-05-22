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

void op_gsSPVertex_f3d(CHleDmemState* state)
{
	dl_cmd_vtx_f3d_t* cmd = &state->command.vtx_f3d;

	uint32_t physAddr = state->SegmentedToPhysical(cmd->address);

	if (physAddr + cmd->num*16 >= g_MMU->RdramSize())
	{
		return;
	}

	if (cmd->idx + cmd->num >= sizeof(state->vertices) / sizeof(state->vertices[0]))
	{
		return;
	}

	uint8_t* ptr = g_MMU->Rdram() + physAddr;

	for (int i = 0; i < cmd->num; i++)
	{
		vertex_t* vtx = &state->vertices[cmd->idx + i];
		uint32_t offs = i * 16;

		vtx->x = *(int16_t*)&ptr[(offs+0) ^ 2];
		vtx->y = *(int16_t*)&ptr[(offs+2) ^ 2];
		vtx->z = *(int16_t*)&ptr[(offs+4) ^ 2];

		printf("%d %d %d\n", vtx->x, vtx->y, vtx->z);
		// todo the rest
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
    state->textureImage = cmd->address;
    state->textureImageSiz = (im_siz_t)cmd->siz;
    state->textureImageFmt = (im_fmt_t)cmd->fmt;
}

void op_gsDPSetDepthImage(CHleDmemState* state)
{
	dl_cmd_settimg_t* cmd = &state->command.settimg;
	state->depthImage = cmd->address; // todo other params
}

void op_gsDPSetColorImage(CHleDmemState* state)
{
	dl_cmd_settimg_t* cmd = &state->command.settimg;
	state->colorImage = cmd->address; // todo other params
}

void op_gsDPSetCombineLERP(CHleDmemState* state)
{
    state->combiner = state->command.setcombine;
}

void op_gsSPSetGeometryMode_f3d(CHleDmemState* state)
{
    dl_cmd_geometrymode_f3d_t* cmd = &state->command.geometrymode_f3d;
    state->geometryMode.data |= cmd->mode.data;
}

void op_gsSPClearGeometryMode_f3d(CHleDmemState* state)
{
    dl_cmd_geometrymode_f3d_t* cmd = &state->command.geometrymode_f3d;
    state->geometryMode.data &= ~cmd->mode.data;
}

void op_gsSPSetOtherMode_h(CHleDmemState* state)
{
	dl_cmd_setothermode_h_t* cmd = &state->command.setothermode_h;
	uint32_t mask = ~(((1 << cmd->len) - 1) << cmd->sft);
	state->othermode_h.data = (state->othermode_h.data & mask) | cmd->bits.data;
}

void op_gsDPSetFillColor(CHleDmemState* state)
{
	state->fillColor = state->command.w1;
}

void op_gsDPSetFogColor(CHleDmemState* state)
{
	state->fogColor = state->command.w1;
}

void op_gsDPSetBlendColor(CHleDmemState* state)
{
	state->blendColor = state->command.w1;
}

void op_gsDPSetPrimColor(CHleDmemState* state)
{
	state->primColor = state->command.w1;
}

void op_gsDPSetEnvColor(CHleDmemState* state)
{
	state->envColor = state->command.w1;
}