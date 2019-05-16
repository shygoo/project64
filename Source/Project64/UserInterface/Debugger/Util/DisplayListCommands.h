#pragma once

#include <stdafx.h>

typedef union
{
	uint32_t data;
	struct
	{
		uint32_t : 4;
		uint32_t ad : 2; // alpha dither
		uint32_t rd : 2; // rgb dither
		uint32_t ck : 1; // chroma key
		uint32_t tc : 3; // texture convert
		uint32_t tf : 2; // texture filter
		uint32_t tt : 2; // texture lut
		uint32_t tl : 1; // texture lod
		uint32_t td : 2; // texture detail
		uint32_t tp : 1; // texture perspective
		uint32_t cyc : 2; // cycle type
		uint32_t cd : 1; // color dither
		uint32_t pm : 1; // pipeline mode
		uint32_t : 8;
	};
} othermode_h_t;

////////////////

typedef struct
{
	struct { uint32_t index : 8; uint32_t offset : 16; uint32_t c : 8; }; // w0
	uint32_t data; // w1
} dl_cmd_moveword_f3d_t;

#define dl_cmd_moveword_f3dex_t dl_cmd_moveword_f3d_t

typedef struct
{
	struct { uint32_t offset : 16; uint32_t index : 8;  uint32_t c : 8; }; // w0
	uint32_t data; // w1
} dl_cmd_moveword_f3dex2_t;

typedef struct
{
	struct { uint32_t len : 16; uint32_t idx : 4; uint32_t num : 4; uint32_t c : 8; };
	uint32_t address;
} dl_cmd_vtx_f3d_t;

typedef struct
{
	uint32_t w0;
	struct { uint32_t v2 : 8; uint32_t v1 : 8; uint32_t v0 : 8; uint32_t c : 8; };
} dl_cmd_tri1_f3d_t;

typedef struct
{
	struct { uint32_t size : 16; uint32_t params : 8; uint32_t c : 8; };
	uint32_t address;
} dl_cmd_mtx_f3d_t;

#define dl_cmd_mtx_f3dex_t dl_cmd_mtx_f3d_t

typedef struct
{
	struct { uint32_t lry : 12; uint32_t lrx : 12; uint32_t c : 8; };
	struct { uint32_t uly : 12; uint32_t ulx : 12; uint32_t : 8; };
} dl_cmd_fillrect_t;

typedef struct
{
	struct { uint32_t uly : 12; uint32_t ulx : 12; uint32_t c : 8; };
	struct { uint32_t lry : 12; uint32_t lrx : 12; uint32_t mode : 2; uint32_t : 2; };
} dl_cmd_setscissor_t;

typedef struct
{
	struct { uint32_t width : 12; uint32_t : 7; uint32_t siz : 2; uint32_t fmt : 3; uint32_t c : 8; };
	uint32_t address;
} dl_cmd_settimg_t;

typedef struct
{
	struct { uint32_t ult : 12; uint32_t uls : 12; uint32_t c : 8; };
	struct { uint32_t dxt : 12; uint32_t lrs : 12; uint32_t tile : 3; uint32_t : 5; };
} dl_cmd_loadblock_t;

typedef struct
{
	struct { uint32_t ult : 12; uint32_t uls : 12; uint32_t c : 8; };
	struct { uint32_t lrt : 12; uint32_t lrs : 12; uint32_t tile : 3; uint32_t : 5; };
} dl_cmd_settilesize_t;

typedef struct
{
	struct { uint32_t tmem : 9; uint32_t line : 9; uint32_t : 1; uint32_t siz : 2; uint32_t fmt : 3; uint32_t c : 8; };
	struct { uint32_t shifts : 4; uint32_t masks : 4; uint32_t cms : 2; uint32_t shiftt : 4; uint32_t maskt : 4; uint32_t cmt : 2; uint32_t palette : 4; uint32_t tile : 3; uint32_t : 5; };
} dl_cmd_settile_t;

typedef struct
{
	struct { uint32_t on : 8; uint32_t tile : 3; uint32_t level : 3; uint32_t : 2; uint32_t bowtie : 8; uint32_t c : 8; };
	struct { uint32_t t : 16; uint32_t s : 16; };
} dl_cmd_texture_f3d_t;

typedef struct
{
	struct { uint32_t l : 16; uint32_t p : 8; uint32_t c : 8; };
	uint32_t address;
} dl_cmd_movemem_f3d_t;

typedef struct
{
    struct { uint32_t : 24; uint32_t c : 8; };
    uint32_t mode;
} dl_cmd_setgeometrymode_f3d_t;

typedef struct
{
	struct { uint32_t : 16; uint32_t branch : 8; uint32_t c : 8; };
	uint32_t address;
} dl_cmd_dl_t;

typedef struct
{
	struct { uint32_t len : 8; uint32_t sft : 8; uint32_t : 8; uint32_t c : 8; };
	othermode_h_t bits;
} dl_cmd_setothermode_h_t;

typedef union
{
	struct { uint32_t w0, w1; };

    /* shared rdp commands */
	dl_cmd_fillrect_t    fillrect;
	dl_cmd_setscissor_t  setscissor;
	dl_cmd_settimg_t     settimg;
	dl_cmd_settile_t     settile;
	dl_cmd_loadblock_t   loadblock;
	dl_cmd_settilesize_t settilesize;

    /* shared rsp commands */
    dl_cmd_dl_t dl;
	dl_cmd_setothermode_h_t setothermode_h;

    /* fast3d commands */
	dl_cmd_moveword_f3d_t moveword_f3d;
	dl_cmd_movemem_f3d_t  movemem_f3d;
	dl_cmd_tri1_f3d_t     tri1_f3d;
	dl_cmd_mtx_f3d_t      mtx_f3d;
	dl_cmd_vtx_f3d_t      vtx_f3d;
	dl_cmd_texture_f3d_t  texture_f3d;
    dl_cmd_setgeometrymode_f3d_t setgeometrymode_f3d;

    /* f3dex commands */
    dl_cmd_mtx_f3dex_t mtx_f3dex;

    /* f3dex2 commands */
    dl_cmd_moveword_f3dex2_t moveword_f3dex2;
} dl_cmd_t;
