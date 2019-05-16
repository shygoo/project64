#pragma once

#include "DisplayListParser.h"


void op_gsDPSetTile(CHleDmemState* state);
void op_gsDPSetTextureImage(CHleDmemState* state);
void op_gsDPSetDepthImage(CHleDmemState* state);
void op_gsDPSetColorImage(CHleDmemState* state);
void op_gsDPSetFillColor(CHleDmemState* state);
void op_gsDPSetFogColor(CHleDmemState* state);
void op_gsDPSetBlendColor(CHleDmemState* state);
void op_gsDPSetPrimColor(CHleDmemState* state);
void op_gsDPSetEnvColor(CHleDmemState* state);

void op_gsSPDisplayList(CHleDmemState* state);
void op_gsSPEndDisplayList(CHleDmemState* state);
void op_gsSPSetOtherMode_h(CHleDmemState* state);

void op_gsSPMoveWord_f3d(CHleDmemState* state);
void op_gsSPSetGeometryMode_f3d(CHleDmemState* state);
void op_gsSPClearGeometryMode_f3d(CHleDmemState* state);
void op_gsSPTexture_f3d(CHleDmemState* state);

void op_gsSPMoveWord_f3dex2(CHleDmemState* state);

