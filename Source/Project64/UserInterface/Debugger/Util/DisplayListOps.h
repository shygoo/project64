#pragma once

#include "DisplayListParser.h"

void op_gsSPDisplayList(CHleDmemState* state);
void op_gsSPEndDisplayList(CHleDmemState* state);
void op_gsSPMoveWord_f3d(CHleDmemState* state);
void op_gsSPMoveWord_f3dex2(CHleDmemState* state);
void op_gsDPSetTile(CHleDmemState* state);
void op_gsDPSetTextureImage(CHleDmemState* state);

void op_gsSPSetGeometryMode_f3d(CHleDmemState* state);
void op_gsSPClearGeometryMode_f3d(CHleDmemState* state);
void op_gsSPTexture_f3d(CHleDmemState* state);