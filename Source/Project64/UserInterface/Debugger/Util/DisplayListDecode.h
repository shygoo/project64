#pragma once

#include "DisplayListParser.h"

void ReportDramResource(decode_context_t* dc, CHleDmemState* state, resource_type_t resType, uint32_t param);

void dec_NoParams(CHleDmemState*, decode_context_t*);
void dec_HexParam32(CHleDmemState*, decode_context_t*);
void dec_gsDPFillRectangle(CHleDmemState*, decode_context_t*);
void dec_gsDPSetScissor(CHleDmemState*, decode_context_t*);
void dec_gsDPSetTextureImage(CHleDmemState*, decode_context_t*);
void dec_gsDPSetDepthImage(CHleDmemState*, decode_context_t*);
void dec_gsDPSetColorImage(CHleDmemState*, decode_context_t*);
void dec_gsDPLoadBlock(CHleDmemState*, decode_context_t*);
void dec_gsDPSetTileSize(CHleDmemState*, decode_context_t*);
void dec_gsDPSetTile(CHleDmemState*, decode_context_t*);
void dec_gsSPMoveWord_f3d(CHleDmemState*, decode_context_t*);
void dec_gsSP1Triangle_f3d(CHleDmemState*, decode_context_t*);
void dec_gsSPVertex_f3d(CHleDmemState*, decode_context_t*);
void dec_gsSPSetGeometryMode_f3d(CHleDmemState*, decode_context_t*);
#define dec_gsSPClearGeometryMode_f3d dec_gsSPSetGeometryMode_f3d
void dec_gsSPMatrix_f3d(CHleDmemState*, decode_context_t*);
void dec_gsSPMoveWord_f3dex2(CHleDmemState*, decode_context_t*);
void dec_gsSPMoveMem_f3d(CHleDmemState*, decode_context_t*);
void dec_gsSPTexture_f3d(CHleDmemState*, decode_context_t*);
void dec_gsSPDisplayList(CHleDmemState*, decode_context_t*);
void dec_gsSPSetOtherMode_h(CHleDmemState*, decode_context_t*);