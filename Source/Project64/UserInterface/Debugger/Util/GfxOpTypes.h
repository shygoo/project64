#pragma once

#include <stdafx.h>
//#include "GfxParser.h"
//#include "GfxState.h"

typedef struct
{
	struct { uint32_t index : 8, offset : 16, _c : 8; }; // w0
	uint32_t data; // w1
} dl_cmd_moveword_f3d_t;

#define dl_cmd_moveword_f3dex_t dl_cmd_moveword_f3d_t

typedef struct
{
	struct { uint32_t offset : 16, index : 8, _c : 8; }; // w0
	uint32_t data; // w1
} dl_cmd_moveword_f3dex2_t;

typedef struct
{
	struct { uint32_t len : 16, idx : 4, num : 4, _c : 8; };
	uint32_t address;
} dl_cmd_vtx_f3d_t;

typedef struct
{
    struct { uint32_t len : 10, num : 6, idx : 8, _c : 8; };
    uint32_t address;
} dl_cmd_vtx_f3dex_t;

typedef struct
{
    struct { uint32_t idx : 8, _u0 : 4, num : 8, _u1 : 4, _c : 8; };
    uint32_t address;
} dl_cmd_vtx_f3dex2_t;

typedef struct
{
    struct { uint32_t _u0 : 24, _c : 8; };
    struct { uint32_t v2 : 8, v1 : 8, v0 : 8, _u1 : 8; };
} dl_cmd_tri1_f3d_t;

#define dl_cmd_tri1_f3dex_t dl_cmd_tri1_f3d_t

typedef struct
{
    struct { uint32_t v2 : 8, v1 : 8, v0 : 8, _c : 8; };
    uint32_t  _u0;
} dl_cmd_tri1_f3dex2_t;

typedef struct
{
    struct { uint32_t v2 : 8, v1 : 8, v0 : 8, _c : 8; };
    struct { uint32_t v5 : 8, v4 : 8, v3 : 8, _u1 : 8; };
} dl_cmd_tri2_f3dex_t;

#define dl_cmd_tri2_f3dex2_t dl_cmd_tri2_f3dex_t;

typedef struct
{
    struct { uint32_t _u0 : 24, _c : 8; };
    struct { uint32_t v3 : 8, v2 : 8, v1 : 8, v0 : 8; };
} dl_cmd_quad_f3dex_t; // mk64, different from gbi.h?

typedef struct
{
	struct { uint32_t size : 16, params : 8, _c : 8; };
	uint32_t address;
} dl_cmd_mtx_f3d_t;

#define dl_cmd_mtx_f3dex_t dl_cmd_mtx_f3d_t

typedef struct
{
    struct { uint32_t params : 8, _u0: 8, size : 8, _c : 8; };
    uint32_t address;
} dl_cmd_mtx_f3dex2_t;

typedef struct
{
	struct { uint32_t lry : 12, lrx : 12, _c : 8; };
	struct { uint32_t uly : 12, ulx : 12, _u0 : 8; };
} dl_cmd_fillrect_t;

typedef struct
{
	struct { uint32_t uly : 12, ulx : 12, _c : 8; };
	struct { uint32_t lry : 12, lrx : 12, mode : 2, _u0 : 2; };
} dl_cmd_setscissor_t;

typedef struct
{
	struct { uint32_t width : 12, _u0: 7, siz : 2, fmt : 3, _c : 8; };
	uint32_t address;
} dl_cmd_settimg_t;

#define dl_cmd_setzimg_t dl_cmd_settimg_t
#define dl_cmd_setcimg_t dl_cmd_settimg_t

typedef struct
{
	struct { uint32_t ult : 12, uls : 12, _c : 8; };
	struct { uint32_t dxt : 12, lrs : 12, tile : 3, _u0: 5; };
} dl_cmd_loadblock_t;

typedef struct
{
    struct { uint32_t ult : 12, uls : 12, _c : 8; };
    struct { uint32_t lrt : 12, lrs : 12, tile : 3, _u0 : 5; };
} dl_cmd_loadtile_t;

#define dl_cmd_settilesize_t dl_cmd_loadtile_t

typedef struct
{
    struct { uint32_t _u0 : 24, _c : 8; };
    struct { uint32_t _u1 : 12, count : 12, tile : 4, _u2 : 4; };
} dl_cmd_loadtlut_t;

typedef struct
{
	struct { uint32_t tmem : 9, line : 9, _u0: 1, siz : 2, fmt : 3, _c : 8; };
	struct { uint32_t shifts : 4, masks : 4, cms : 2, shiftt : 4, maskt : 4, cmt : 2, palette : 4, tile : 3, _u1: 5; };
} dl_cmd_settile_t;

typedef struct
{
	struct { uint32_t on : 8, tile : 3, level : 3, _u0: 2, bowtie : 8, _c : 8; };
	struct { uint32_t t : 16, s : 16; };
} dl_cmd_texture_f3d_t;

typedef struct
{
	struct { uint32_t l : 16, p : 8, _c : 8; };
	uint32_t address;
} dl_cmd_movemem_f3d_t;

typedef struct
{
    struct { uint32_t i : 8, o : 8, l : 8, _c : 8; };
    uint32_t address;
} dl_cmd_movemem_f3dex2_t;

typedef struct
{
    struct { uint32_t : 24, _c : 8; };
    //geometrymode_f3d_t mode;
	uint32_t mode;
} dl_cmd_geometrymode_f3d_t; // note: for both set and clear

typedef struct
{
    struct { uint32_t c : 24, _c : 8; };
    uint32_t s;
} dl_cmd_geometrymode_f3dex2_t;

typedef struct
{
	struct { uint32_t _u0: 16, branch : 8, _c : 8; };
	uint32_t address;
} dl_cmd_dl_t;

typedef struct
{
	struct { uint32_t len : 8, sft : 8, _u0 : 8, _c : 8; };
	//othermode_h_t bits;
	uint32_t mode;
} dl_cmd_setothermode_h_t;

typedef struct
{
    struct { uint32_t len : 8, sft : 8, _u0 : 8, _c : 8; };
    //othermode_l_t bits;
	uint32_t mode;
} dl_cmd_setothermode_l_t;

typedef struct
{
    struct { uint32_t c1 : 5, a1 : 4, Ac0 : 3, Aa0 : 3, c0 : 5, a0 : 4, _c : 8; };
    struct { uint32_t Ad1 : 3, Ab1 : 3, d1 : 3, Ad0 : 3, Ab0 : 3, d0 : 3, Ac1 : 3, Aa1 : 3, b1 : 4, b0 : 4; };
} dl_cmd_setcombine_t;

typedef union
{
	struct { uint32_t w0, w1; };

    /* shared rdp commands */
	dl_cmd_fillrect_t    fillrect;
	dl_cmd_setscissor_t  setscissor;
    dl_cmd_settimg_t     settimg;
    dl_cmd_setzimg_t     setzimg;
    dl_cmd_setcimg_t     setcimg;
	dl_cmd_settile_t     settile;
	dl_cmd_loadblock_t   loadblock;
    dl_cmd_loadtile_t    loadtile;
    dl_cmd_loadtlut_t    loadtlut;
	dl_cmd_settilesize_t settilesize;
    dl_cmd_setcombine_t  setcombine;

    /* shared rsp commands */
    dl_cmd_dl_t dl;
    dl_cmd_setothermode_h_t setothermode_h;
    dl_cmd_setothermode_l_t setothermode_l;

    /* fast3d commands */
	dl_cmd_moveword_f3d_t moveword_f3d;
	dl_cmd_movemem_f3d_t  movemem_f3d;
	dl_cmd_tri1_f3d_t     tri1_f3d;
	dl_cmd_mtx_f3d_t      mtx_f3d;
	dl_cmd_vtx_f3d_t      vtx_f3d;
	dl_cmd_texture_f3d_t  texture_f3d;
    dl_cmd_geometrymode_f3d_t geometrymode_f3d;

    /* f3dex commands */
    dl_cmd_mtx_f3dex_t  mtx_f3dex;
    dl_cmd_vtx_f3dex_t  vtx_f3dex;
    dl_cmd_tri1_f3dex_t tri1_f3dex;
    dl_cmd_quad_f3dex_t quad_f3dex;
    dl_cmd_tri2_f3dex_t tri2_f3dex;

    /* f3dex2 commands */
    dl_cmd_moveword_f3dex2_t moveword_f3dex2;
    dl_cmd_vtx_f3dex2_t      vtx_f3dex2;
    dl_cmd_tri1_f3dex2_t     tri1_f3dex2;
    dl_cmd_movemem_f3dex2_t  movemem_f3dex2;
    dl_cmd_mtx_f3dex2_t      mtx_f3dex2;
    dl_cmd_geometrymode_f3dex2_t geometrymode_f3dex2;
} dl_cmd_t;
