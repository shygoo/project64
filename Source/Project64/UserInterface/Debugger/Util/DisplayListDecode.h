#pragma once

#include "DisplayListParser.h"

const char* dec_NoParams(CHleDmemState*, char*);
const char* dec_HexParam32(CHleDmemState*, char*);
const char* dec_gsDPFillRectangle(CHleDmemState*, char*);
const char* dec_gsDPSetScissor(CHleDmemState*, char*);
const char* dec_gsDPSetTextureImage(CHleDmemState*, char*);
const char* dec_gsDPLoadBlock(CHleDmemState*, char*);
const char* dec_gsDPSetTileSize(CHleDmemState*, char*);
const char* dec_gsDPSetTile(CHleDmemState*, char*);
const char* dec_gsSPMoveWord_f3d(CHleDmemState*, char*);
const char* dec_gsSP1Triangle_f3d(CHleDmemState*, char*);
const char* dec_gsSPVertex_f3d(CHleDmemState*, char*);
const char* dec_gsSPSetGeometryMode_f3d(CHleDmemState*, char*);
#define dec_gsSPClearGeometryMode_f3d dec_gsSPSetGeometryMode_f3d
const char* dec_gsSPMatrix_f3d(CHleDmemState*, char*);
const char* dec_gsSPMoveWord_f3dex2(CHleDmemState*, char*);
const char* dec_gsSPMoveMem_f3d(CHleDmemState*, char*);
const char* dec_gsSPTexture_f3d(CHleDmemState*, char*);
const char* dec_gsSPDisplayList(CHleDmemState*, char*);
