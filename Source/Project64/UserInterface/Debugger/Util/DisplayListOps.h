#pragma once

#include "DisplayListParser.h"

void ReportDramResource(decode_context_t* dc, CHleDmemState* state, resource_type_t resType, uint32_t param);

void op_Nop(CHleDmemState*, decode_context_t*);

// RDP
void op_gsDPSetScissor     (CHleDmemState*, decode_context_t*);
void op_gsDPSetTileSize    (CHleDmemState*, decode_context_t*);
void op_gsDPLoadBlock      (CHleDmemState*, decode_context_t*);
void op_gsDPSetTile        (CHleDmemState*, decode_context_t*);
void op_gsDPFillRectangle  (CHleDmemState*, decode_context_t*);
void op_gsDPSetFillColor   (CHleDmemState*, decode_context_t*);
void op_gsDPSetFogColor    (CHleDmemState*, decode_context_t*);
void op_gsDPSetBlendColor  (CHleDmemState*, decode_context_t*);
void op_gsDPSetPrimColor   (CHleDmemState*, decode_context_t*);
void op_gsDPSetEnvColor    (CHleDmemState*, decode_context_t*);
void op_gsDPSetCombineLERP (CHleDmemState*, decode_context_t*);
void op_gsDPSetTextureImage(CHleDmemState*, decode_context_t*);
void op_gsDPSetDepthImage  (CHleDmemState*, decode_context_t*);
void op_gsDPSetColorImage  (CHleDmemState*, decode_context_t*);

// Shared RSP
void op_gsSPDisplayList    (CHleDmemState* state);
void op_gsSPEndDisplayList (CHleDmemState* state);
void op_gsSPSetOtherMode_h (CHleDmemState* state);
void op_gsSPSetOtherMode_l (CHleDmemState*, decode_context_t*);

// Fast3D
void op_gsSPVertex_f3d(CHleDmemState* state);
void op_gsSPMoveWord_f3d(CHleDmemState* state);
void op_gsSPSetGeometryMode_f3d(CHleDmemState* state);
void op_gsSPClearGeometryMode_f3d(CHleDmemState* state);
void op_gsSPTexture_f3d(CHleDmemState* state);
void op_gsSP1Triangle_f3d(CHleDmemState*, decode_context_t*);
void op_gsSPMatrix_f3d(CHleDmemState*, decode_context_t*);
void op_gsSPMoveMem_f3d(CHleDmemState*, decode_context_t*);

// F3DEX2
void op_gsSPMoveWord_f3dex2(CHleDmemState* state);


//////////////////

//void op_HexParam32(CHleDmemState*, decode_context_t*);

void op_gsSPMoveWord_f3d(CHleDmemState*, decode_context_t*);
void op_gsSPVertex_f3d(CHleDmemState*, decode_context_t*);
void op_gsSPSetGeometryMode_f3d(CHleDmemState*, decode_context_t*);
void op_gsSPMoveWord_f3dex2(CHleDmemState*, decode_context_t*);
void op_gsSPTexture_f3d(CHleDmemState*, decode_context_t*);

void op_gsSPDisplayList(CHleDmemState*, decode_context_t*);



//#define op_gsSPClearGeometryMode_f3d dec_gsSPSetGeometryMode_f3d
