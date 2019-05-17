#include <stdafx.h>
#include "DisplayListDecode.h"

void ReportDramResource(decode_context_t* dc, CHleDmemState* state, resource_type_t resType, uint32_t param = 0)
{
	dc->dramResource.type = resType;
	dc->dramResource.address = state->command.w1;
	dc->dramResource.virtAddress = state->SegmentedToVirtual(state->command.w1);
    dc->dramResource.param = param;
}

////////////

void dec_NoParams(CHleDmemState*, decode_context_t* dc)
{
    sprintf(dc->params, "");
}

void dec_gsSPMoveWord_f3d(CHleDmemState* state, decode_context_t* dc)
{
    dl_cmd_moveword_f3d_t* cmd = &state->command.moveword_f3d;

    if (cmd->index == 0x06) // MW_SEGMENT
    {
        sprintf(dc->params, "0x%02X, 0x%08X", cmd->offset / 4, cmd->data);
        dc->overrideName = "gsSPSegment";

		ReportDramResource(dc, state, RES_SEGMENT, cmd->offset/4);
    }

    if (cmd->index == 0x02 && cmd->offset == 0x00) // MW_NUMLIGHT, MWO_NUMLIGHT
    {
        int numLights = ((cmd->data - 0x80000000) / 32) - 1;
		sprintf(dc->params, "%d", numLights);
        dc->overrideName = "gsSPNumLights";
    }

	//if (cmd->index == 0x0E) // G_MW_PERSPNORM
	//{
	//	MessageBox(NULL, "perspnorm", "", MB_OK);
	//	sprintf(dc->params, "%d", (uint16_t)cmd->data);
	//	dc->overrideName = "gsSPPerspNormalize";
	//}
}

void dec_gsSPMoveMem_f3d(CHleDmemState* state, decode_context_t* dc)
{
    // gsDma2p G_MOVEMEM adrs len idx ofs 

    dl_cmd_movemem_f3d_t* cmd = &state->command.movemem_f3d;

    if (cmd->p == 0x80) // G_MV_VIEWPORT
    {
        // todo params
        sprintf(dc->params, "addr:0x%08X // 0x%08X", cmd->address,
            state->SegmentedToVirtual(cmd->address));
        dc->overrideName = "gsSPViewport";

		ReportDramResource(dc, state, RES_VIEWPORT);
    }

    if (cmd->p >= 0x86 && cmd->p <= 0x94) // G_MV_L0:7
    {
        int lightNumber = (cmd->p - 0x86) / 2;
        sprintf(dc->params, "addr:0x%08X, lightnum:%d // 0x%08X", cmd->address, lightNumber,
            state->SegmentedToVirtual(cmd->address));
        dc->overrideName = "gsSPLight";

		ReportDramResource(dc, state, RES_AMBIENT_LIGHT);
    }
}

void dec_gsSPMoveWord_f3dex2(CHleDmemState* state, decode_context_t* dc)
{
    dl_cmd_moveword_f3dex2_t* cmd = &state->command.moveword_f3dex2;

    if (cmd->index == 0x06) // MW_SEGMENT
    {
        sprintf(dc->params, "segment:0x%02X, data:0x%08X", cmd->offset / 4, cmd->data);
        dc->overrideName = "gsSPSegment";

		ReportDramResource(dc, state, RES_SEGMENT);
    }
}

void dec_gsSP1Triangle_f3d(CHleDmemState* state, decode_context_t* dc)
{
    dl_cmd_tri1_f3d_t *cmd = &state->command.tri1_f3d;
    sprintf(dc->params, "v0:%d, v1:%d, v2:%d", cmd->v0 / 10, cmd->v1 / 10, cmd->v2 / 10);
    dc->numTris = 1;
}

void dec_gsSPVertex_f3d(CHleDmemState* state, decode_context_t* dc)
{
    dl_cmd_vtx_f3d_t *cmd = &state->command.vtx_f3d;
    sprintf(dc->params, "addr:0x%08X, numv:%d, vidx:%d // 0x%08X", cmd->address, cmd->num + 1, cmd->idx,
        state->SegmentedToVirtual(cmd->address));

	ReportDramResource(dc, state, RES_VERTICES, cmd->num+1);
}

void dec_gsSPSetGeometryMode_f3d(CHleDmemState* state, decode_context_t* dc)
{
    bool havePrev = false;
	char *paramsOut = dc->params;

    for (int i = 0; CDisplayListParser::GeometryModeNames[i].name != NULL; i++)
    {
        if (state->command.w1 & CDisplayListParser::GeometryModeNames[i].value)
        {
            if (havePrev)
            {
				paramsOut += sprintf(paramsOut, " | ");
            }

			paramsOut += sprintf(paramsOut, CDisplayListParser::GeometryModeNames[i].name);
            havePrev = true;
        }
    }
}

void dec_gsSPDisplayList(CHleDmemState* state, decode_context_t* dc)
{
    dl_cmd_dl_t *cmd = &state->command.dl;

    sprintf(dc->params, "0x%08X // 0x%08X", cmd->address,
        state->SegmentedToVirtual(cmd->address));

    if (cmd->branch)
    {
        dc->overrideName = "gsSPBranchList";
		dc->listFgColor = RGB(100, 100, 0);
    }
	else
	{
		ReportDramResource(dc, state, RES_DL);
		dc->listFgColor = RGB(0, 100, 0);
	}
}

void dec_HexParam32(CHleDmemState* state, decode_context_t* dc)
{
    sprintf(dc->params, "0x%08X", state->command.w1);
}

void dec_gsSPMatrix_f3d(CHleDmemState* state, decode_context_t* dc)
{
    dl_cmd_mtx_f3d_t* cmd = &state->command.mtx_f3d;

    sprintf(dc->params, "addr:0x%08X, (%s | %s | %s) // 0x%08X", cmd->address,
        (cmd->params & 1) ? "G_MTX_PROJECTION" : "G_MTX_MODELVIEW",
        (cmd->params & 2) ? "G_MTX_LOAD" : "G_MTX_MUL",
        (cmd->params & 4) ? "G_MTX_PUSH" : "G_MTX_NOPUSH",
        state->SegmentedToVirtual(cmd->address));

	ReportDramResource(dc, state, (cmd->params & 1) ? RES_PROJECTION_MATRIX : RES_MODELVIEW_MATRIX);
}

void dec_gsDPFillRectangle(CHleDmemState* state, decode_context_t* dc)
{
    dl_cmd_fillrect_t* cmd = &state->command.fillrect;
    sprintf(dc->params, "ulx:%d, uly:%d, lrx:%d, lry:%d", cmd->ulx >> 2, cmd->uly >> 2, cmd->lrx >> 2, cmd->lry >> 2);
}

void dec_gsDPSetScissor(CHleDmemState* state, decode_context_t* dc)
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

    sprintf(dc->params, "mode:%s, ulx:%d, uly:%d, lrx:%d, lry:%d", szMode, cmd->ulx >> 2, cmd->uly >> 2, cmd->lrx >> 2, cmd->lry >> 2);
}

void dec_gsDPSetTextureImage(CHleDmemState* state, decode_context_t* dc)
{
    dl_cmd_settimg_t* cmd = &state->command.settimg;

    const char* fmtName = "?";
    const char* sizName = "?";

    fmtName = CDisplayListParser::LookupName(CDisplayListParser::ImageFormatNames, cmd->fmt);
    sizName = CDisplayListParser::LookupName(CDisplayListParser::TexelSizeNames, cmd->siz);

    sprintf(dc->params, "fmt:%s, siz:%s, w:%d, addr:0x%08X // 0x%08X", fmtName, sizName, cmd->width + 1, cmd->address,
        state->SegmentedToVirtual(cmd->address));
}

void dec_gsDPSetDepthImage(CHleDmemState* state, decode_context_t* dc)
{
	dl_cmd_setzimg_t* cmd = &state->command.setzimg;

	const char* fmtName = "?";
	const char* sizName = "?";

	fmtName = CDisplayListParser::LookupName(CDisplayListParser::ImageFormatNames, cmd->fmt);
	sizName = CDisplayListParser::LookupName(CDisplayListParser::TexelSizeNames, cmd->siz);

	sprintf(dc->params, "fmt:%s, siz:%s, w:%d, addr:0x%08X // 0x%08X", fmtName, sizName, cmd->width + 1, cmd->address,
		state->SegmentedToVirtual(cmd->address));

	ReportDramResource(dc, state, RES_DEPTHBUFFER);
}

void dec_gsDPSetColorImage(CHleDmemState* state, decode_context_t* dc)
{
	dl_cmd_setcimg_t* cmd = &state->command.setcimg;

	const char* fmtName = "?";
	const char* sizName = "?";

	fmtName = CDisplayListParser::LookupName(CDisplayListParser::ImageFormatNames, cmd->fmt);
	sizName = CDisplayListParser::LookupName(CDisplayListParser::TexelSizeNames, cmd->siz);

	sprintf(dc->params, "fmt:%s, siz:%s, w:%d, addr:0x%08X // 0x%08X", fmtName, sizName, cmd->width + 1, cmd->address,
		state->SegmentedToVirtual(cmd->address));

	ReportDramResource(dc, state, RES_COLORBUFFER);
    dc->dramResource.imageWidth = 320;
    dc->dramResource.imageHeight = 240;
}

void dec_gsDPSetTile(CHleDmemState* state, decode_context_t* dc)
{
    dl_cmd_settile_t* cmd = &state->command.settile;

    const char* fmtName = "?";
    const char* sizName = "?";

    fmtName = CDisplayListParser::LookupName(CDisplayListParser::ImageFormatNames, cmd->fmt);
    sizName = CDisplayListParser::LookupName(CDisplayListParser::TexelSizeNames, cmd->siz);

    sprintf(dc->params, "fmt:%s, siz:%s, line:%d, tmem:0x%03X, tile:%d, palette:%d, cmt:%d, maskt:%d, shiftt:%d, cmt:%d, masks:%d, shifts:%d", fmtName, sizName,
        cmd->line, cmd->tmem, cmd->tile, cmd->palette,
        cmd->cmt, cmd->maskt, cmd->shiftt,
        cmd->cms, cmd->masks, cmd->shifts);
}

void dec_gsDPLoadBlock(CHleDmemState* state, decode_context_t* dc)
{
    dl_cmd_loadblock_t* cmd = &state->command.loadblock;
	sprintf(dc->params, "tile:%d, uls:%d, ult:%d, lrs:%d, dxt:%d",
		cmd->tile, cmd->uls, cmd->ult, cmd->lrs, cmd->dxt);
	
    //if (cmd->uls != 0 || cmd->ult != 0)
    //{
    //	MessageBox(NULL, "nonzero uls/ult", "", MB_OK);
    //}

    uint8_t siz = state->textureImageSiz;

    int width, height;

    int numTexelsToLoad = cmd->lrs + 1;
    int bytesPerTexel = (1 << siz) / 2;

    int bytesPerLine = ceil((2048.0f / cmd->dxt) * 8);

    if (bytesPerTexel != 0)
    {
        width = bytesPerLine / bytesPerTexel;
    }
    else
    {
        width = bytesPerLine * 2; // I4, two texels per byte
    }
	
    height = numTexelsToLoad / width;

    dc->dramResource.type = RES_TEXTURE;
    dc->dramResource.address = state->textureImage;
    dc->dramResource.virtAddress = state->SegmentedToVirtual(state->textureImage);
    dc->dramResource.imageWidth = width;
    dc->dramResource.imageHeight = height;
	dc->dramResource.imageSiz = state->textureImageSiz;
	dc->dramResource.imageFmt = state->textureImageFmt;
}

void dec_gsDPSetTileSize(CHleDmemState* state, decode_context_t* dc)
{
    dl_cmd_settilesize_t* cmd = &state->command.settilesize;

	if ((cmd->lrs & 3) == 0 && (cmd->lrt & 3) == 0)
	{
		sprintf(dc->params, "tile:%d, uls:%d, ult:%d, lrs:(%d << 2), lrt:(%d << 2)", cmd->tile, cmd->uls, cmd->ult, cmd->lrs >> 2, cmd->lrt >> 2);
		
	}

    sprintf(dc->params, "tile:%d, uls:%d, ult:%d, lrs:%d, lrt:%d", cmd->tile, cmd->uls, cmd->ult, cmd->lrs, cmd->lrt);
}

void dec_gsSPTexture_f3d(CHleDmemState* state, decode_context_t* dc)
{
    dl_cmd_texture_f3d_t* cmd = &state->command.texture_f3d;
    sprintf(dc->params, "s:%d, t:%d, levels:%d, tile:%d, on:%d", cmd->s, cmd->t, cmd->level, cmd->tile, cmd->on);
}

void dec_gsSPSetOtherMode_h(CHleDmemState* state, decode_context_t* dc)
{
	dl_cmd_setothermode_h_t* cmd = &state->command.setothermode_h;
	othermode_h_t* bits = &cmd->bits;

	switch (cmd->sft)
	{
	case 23: sprintf(dc->params, OtherModeNames::pm[bits->pm]); dc->overrideName = "gsDPPipelineMode"; break;
	case 22: sprintf(dc->params, OtherModeNames::cd[bits->cd]); dc->overrideName = "gsDPSetColorDither"; break; // hw1 only
	case 20: sprintf(dc->params, OtherModeNames::cyc[bits->cyc]); dc->overrideName = "gsDPSetCycleType"; break;
	case 19: sprintf(dc->params, OtherModeNames::tp[bits->tp]); dc->overrideName = "gsDPSetTexturePersp"; break;
	case 17: sprintf(dc->params, OtherModeNames::td[bits->td]); dc->overrideName = "gsDPSetTextureDetail"; break;
	case 16: sprintf(dc->params, OtherModeNames::tl[bits->tl]); dc->overrideName = "gsDPSetTextureLOD"; break;
	case 14: sprintf(dc->params, OtherModeNames::tt[bits->tt]); dc->overrideName = "gsDPSetTextureLUT"; break;
	case 12: sprintf(dc->params, OtherModeNames::tf[bits->tf]); dc->overrideName = "gsDPSetTextureFilter"; break;
	case 9:  sprintf(dc->params, OtherModeNames::tc[bits->tc]); dc->overrideName = "gsDPSetTextureConvert"; break;
	case 8:  sprintf(dc->params, OtherModeNames::ck[bits->ck]); dc->overrideName = "gsDPSetCombineKey"; break;
	case 6:  sprintf(dc->params, OtherModeNames::rd[bits->rd]); dc->overrideName = "gsDPSetColorDither"; break; // hw2 only
	case 4:  sprintf(dc->params, OtherModeNames::ad[bits->ad]); dc->overrideName = "gsDPSetAlphaDither"; break; // hw2 only
	}
}

void dec_gsDPSetCombineLERP(CHleDmemState* state, decode_context_t* dc)
{
    dl_cmd_setcombine_t* cmd = &state->command.setcombine;

    const char* cycle1Preset = NULL;
    const char* cycle2Preset = NULL;

    for (int i = 0; CDisplayListParser::CombinerPresetNames[i].name != NULL; i++)
    {
        cc_preset_lut_entry_t* cc = &CDisplayListParser::CombinerPresetNames[i];

        if (cmd->a0 == cc->a && cmd->b0 == cc->b && cmd->c0 == cc->c && cmd->d0 == cc->d &&
            cmd->Aa0 == cc->Aa && cmd->Ab0 == cc->Ab && cmd->Ac0 == cc->Ac && cmd->Ad0 == cc->Ad)
        {
            cycle1Preset = cc->name;
            break;
        }
    }

    for (int i = 0; CDisplayListParser::CombinerPresetNames[i].name != NULL; i++)
    {
        cc_preset_lut_entry_t* cc = &CDisplayListParser::CombinerPresetNames[i];

        if (cmd->a1 == cc->a && cmd->b1 == cc->b && cmd->c1 == cc->c && cmd->d1 == cc->d &&
            cmd->Aa1 == cc->Aa && cmd->Ab1 == cc->Ab && cmd->Ac1 == cc->Ac && cmd->Ad1 == cc->Ad)
        {
            cycle2Preset = cc->name;
            break;
        }
    }

    if (cycle1Preset != NULL && cycle2Preset != NULL)
    {
        sprintf(dc->params, "%s, %s", cycle1Preset, cycle2Preset);
        dc->overrideName = "gsDPSetCombineMode";
        return;
    }

    stdstr c1 = stdstr_f("%s, %s, %s, %s, %s, %s, %s, %s",
        CDisplayListParser::LookupName(CDisplayListParser::CCMuxA, cmd->a0),
        CDisplayListParser::LookupName(CDisplayListParser::CCMuxB, cmd->b0),
        CDisplayListParser::LookupName(CDisplayListParser::CCMuxC, cmd->c0),
        CDisplayListParser::LookupName(CDisplayListParser::CCMuxD, cmd->d0),
        CDisplayListParser::LookupName(CDisplayListParser::ACMuxA_B_D, cmd->Aa0),
        CDisplayListParser::LookupName(CDisplayListParser::ACMuxA_B_D, cmd->Ab0),
        CDisplayListParser::LookupName(CDisplayListParser::ACMuxC, cmd->Ac0),
        CDisplayListParser::LookupName(CDisplayListParser::ACMuxA_B_D, cmd->Ad0));

    stdstr c2 = stdstr_f("%s, %s, %s, %s, %s, %s, %s, %s",
        CDisplayListParser::LookupName(CDisplayListParser::CCMuxA, cmd->a1),
        CDisplayListParser::LookupName(CDisplayListParser::CCMuxB, cmd->b1),
        CDisplayListParser::LookupName(CDisplayListParser::CCMuxC, cmd->c1),
        CDisplayListParser::LookupName(CDisplayListParser::CCMuxD, cmd->d1),
        CDisplayListParser::LookupName(CDisplayListParser::ACMuxA_B_D, cmd->Aa1),
        CDisplayListParser::LookupName(CDisplayListParser::ACMuxA_B_D, cmd->Ab1),
        CDisplayListParser::LookupName(CDisplayListParser::ACMuxC, cmd->Ac1),
        CDisplayListParser::LookupName(CDisplayListParser::ACMuxA_B_D, cmd->Ad1));


    sprintf(dc->params, "%s, %s", c1.c_str(), c2.c_str());
}