var GfxOps = {};

GfxOps.gsSPDisplayList = function(gfx)
{
    if((gfx.command.w0 & 0x00010000) == 0)
    {
        gfx.spReturnStack[gfx.spReturnIndex++] = gfx.spCommandAddress;
    }
    gfx.spCommandAddress = gfx.command.w1;
}

GfxOps.gsSPEndDisplayList = function(gfx)
{
    if(--gfx.spReturnIndex >= 0)
    {
        gfx.spCommandAddress = gfx.spReturnStack[gfx.spReturnIndex]; 
    }
}

GfxOps.gsMoveWd = function(gfx)
{
    if((gfx.command.w0 & 0x00FF0000) == 0x00060000) // setsegment
    {
        var segment = (gfx.command.w0 & 0xFFFF) / 4;
        gfx.setSegmentAddress(segment, gfx.command.w1);
    }
}

GfxOps.gsSPVertex = function(gfx)
{
    var numVertices = (gfx.command.w0 >> 12 & 0xFF);
    var index = ((gfx.command.w0 & 0xFF) - numVertices * 2) / 2;
    gfx.loadVertices(gfx.command.w1, index, numVertices);
}

GfxOps.gsSPMatrix = function(gfx)
{
    var segmentOffset = gfx.command.w1;
    var params = gfx.command.w0 & 0xFF;
    var flagPush = (params ^ 1) & 0x01;
    var flagLoad = params & 0x02;
    var flagProj = params & 0x04; 
    gfx.loadMatrix(segmentOffset, flagPush, flagLoad, flagProj);
}

GfxOps.gsSPPopMatrix = function(gfx)
{
    gfx.spMatrixIndex--;
}

GfxOps.gsSP2Triangles = function(gfx)
{
    var tri1 = new Array(3);
    var tri2 = new Array(3);
    var w0 = gfx.command.w0;
    var w1 = gfx.command.w1;
    tri1[0] = gfx.spVertices[((w0 >> 16) & 0xFF) / 2];
    tri1[1] = gfx.spVertices[((w0 >> 8) & 0xFF) / 2];
    tri1[2] = gfx.spVertices[((w0 >> 0) & 0xFF) / 2];
    tri2[0] = gfx.spVertices[((w1 >> 16) & 0xFF) / 2];
    tri2[1] = gfx.spVertices[((w1 >> 8) & 0xFF) / 2];
    tri2[2] = gfx.spVertices[((w1 >> 0) & 0xFF) / 2];
    gfx.updateCurrentMaterial();
    gfx.addTriangle(tri1);
    gfx.addTriangle(tri2);
}

GfxOps.gsSP1Triangle = function(gfx)
{
    var tri = new Array(3);
    var w0 = gfx.command.w0;
    tri[0] = gfx.spVertices[((w0 >> 16) & 0xFF) / 2];
    tri[1] = gfx.spVertices[((w0 >> 8) & 0xFF) / 2];
    tri[2] = gfx.spVertices[((w0 >> 0) & 0xFF) / 2];
    gfx.updateCurrentMaterial();
    gfx.addTriangle(tri);
}

GfxOps.gsDPSetTextureImage = function(gfx)
{
    gfx.dpImageAddress = gfx.command.w1;
}

GfxOps.gsDPLoadBlock = function(gfx)
{
    var cmd = gfx.command;

    var uls = cmd.w0f(12, 12);
    var ult = cmd.w0f(0, 12);
    var tile = cmd.w1f(24, 3);
    var lrs = cmd.w1f(12, 12);
    var dxt = cmd.w1f(0, 12);

    var tileDesc = gfx.dpTileDescriptors[tile];

    // ignoring uls, ult parameters

    var numTexelsToLoad = lrs + 1;
    var numBytesPerTexel = TileDescriptor.bytesPerTexel(tileDesc.siz);
    var numBytesToLoad = numTexelsToLoad * numBytesPerTexel;    

    //console.log("loadtile siz: " + tileDesc.siz, numBytesPerTexel);

    var tmemOffset = tileDesc.tmem * 8;
    for(var i = 0; i < numBytesToLoad; i++)
    {
        gfx.dpTextureMemory.setUint8(tmemOffset + i,
             gfx.getU8(gfx.dpImageAddress + i));
    }

    gfx.lastLoadBlock = gfx.segmentedToPhysical(gfx.dpImageAddress);
    //console.log(gfx.lastLoadBlock.toString(16));

    //console.log("tmem: loaded ", numBytesToLoad, " bytes to 0x" + tmemOffset.toString(16), "(tile" + tile +")");
}

GfxOps.gsSPTexture = function(gfx)
{
    var cmd = gfx.command;
    var tile = cmd.w0f(8, 3);
    var tileDesc = gfx.dpTileDescriptors[tile];

    tileDesc.levels = cmd.w0f(11,3);
    tileDesc.on = cmd.w0f(0, 8);
    tileDesc.scaleS = cmd.w1f(16, 16);
    tileDesc.scaleT = cmd.w1f(0, 16);
}

GfxOps.gsDPSetTile = function(gfx)
{
    var cmd = gfx.command;
    var tile = cmd.w1f(24, 3);
    var tileDesc = gfx.dpTileDescriptors[tile];

    tileDesc.fmt = cmd.w0f(21, 3);
    tileDesc.siz = cmd.w0f(19, 2);
    tileDesc.line = cmd.w0f(9, 9);
    tileDesc.tmem = cmd.w0f(0, 9);

    tileDesc.palette = cmd.w1f(20, 4);
    tileDesc.cmT = cmd.w1f(18, 2);
    tileDesc.maskT = cmd.w1f(14, 4);
    tileDesc.shiftT = cmd.w1f(10, 4);
    tileDesc.cmS = cmd.w1f(8, 2);
    tileDesc.maskS = cmd.w1f(4, 4);
    tileDesc.shiftS = cmd.w1f(0, 4);
}

GfxOps.gsDPSetTileSize = function(gfx)
{
    var cmd = gfx.command;
    var tile = cmd.w1f(24, 3);
    var tileDesc = gfx.dpTileDescriptors[tile];

    tileDesc.ulS = cmd.w1f(12, 12);
    tileDesc.ulT = cmd.w1f(0, 12);
    tileDesc.lrS = cmd.w1f(12, 12);
    tileDesc.lrT = cmd.w1f(0, 12);
}

GfxOps.gsDPLoadTLUTCmd = function(gfx)
{
    var cmd = gfx.command;
    var tile = cmd.w1f(24, 3);
    var count = cmd.w1f(14, 10);

    var tileDesc = gfx.dpTileDescriptors[tile];
    var tmemOffset = tileDesc.tmem * 8;

    console.log("loading tlut to ", tmemOffset, count+1);

    // todo mirroring?
    for(var i = 0; i < count+1; i++)
    {
        var color = gfx.getU16(gfx.dpImageAddress + i*2);
        gfx.dpTextureMemory.setUint16(tmemOffset + i*2, color);
    }
}

GfxOps.gsSPSetOtherMode_L = function(gfx)
{
    var cmd = gfx.command;
    var len = cmd.w0f(0, 8) + 1;
    var sft = 32 - cmd.w1f(8, 8) - len;
    var mask = ~(((1 << len)-1) << sft);
    gfx.dpOtherModeL = gfx.dpOtherModeL & mask | cmd.w1;
}

GfxOps.gsDPSetFogColor = function(gfx)
{
    gfx.dpFogColor = gfx.command.w1;
}

GfxOps.RDP = [
    [ 0xF0, GfxOps.gsDPLoadTLUTCmd ],
    [ 0xF2, GfxOps.gsDPSetTileSize ],
    [ 0xF3, GfxOps.gsDPLoadBlock ],
    [ 0xF5, GfxOps.gsDPSetTile ],
    [ 0xFD, GfxOps.gsDPSetTextureImage ],
    [ 0xF8, GfxOps.gsDPSetFogColor ]
];

GfxOps.F3DEX2 = [
    [ 0xE2, GfxOps.gsSPSetOtherMode_L],
    [ 0xD7, GfxOps.gsSPTexture ],
    [ 0x05, GfxOps.gsSP1Triangle ],
    [ 0x06, GfxOps.gsSP2Triangles ],
    [ 0x07, GfxOps.gsSP2Triangles ],
    [ 0x01, GfxOps.gsSPVertex ],
    [ 0xD8, GfxOps.gsSPPopMatrix],
    [ 0xDA, GfxOps.gsSPMatrix ],
    [ 0xDB, GfxOps.gsMoveWd ],
    [ 0xDE, GfxOps.gsSPDisplayList ],
    [ 0xDF, GfxOps.gsSPEndDisplayList ],
];

GfxOps.gsMoveWd_f3d = function(gfx)
{
    var index = gfx.command.w0f(0, 8);
    var offset = gfx.command.w0f(8, 16);
    if(index == 0x06) // setsegment
    {
        var segment = offset / 4;
        gfx.setSegmentAddress(segment, gfx.command.w1 & 0x01FFFFFF);
    }
    else if(index == 0x02)
    {
        gfx.spNumLights = ((gfx.command.w1 - 0x80000000) / 32) - 1;
        console.log("numlights", gfx.spNumLights);
    }
}

GfxOps.gsSPVertex_f3d = function(gfx)
{
    var numVertices = gfx.command.w0f(20, 4) + 1;
    var index = gfx.command.w0f(16, 4);
    gfx.loadVertices(gfx.command.w1, index, numVertices);

    console.log(gfx.command.w0.toString(16), gfx.command.w1.toString(16))
}

GfxOps.gsSP1Triangle_f3d = function(gfx)
{
    var tri = new Array(3);
    var w1 = gfx.command.w1;
    tri[0] = gfx.spVertices[((w1 >> 16) & 0xFF) / 10];
    tri[1] = gfx.spVertices[((w1 >> 8) & 0xFF) / 10];
    tri[2] = gfx.spVertices[((w1 >> 0) & 0xFF) / 10];
    gfx.updateCurrentMaterial();
    gfx.addTriangle(tri);
}

GfxOps.gsSPMatrix_f3d = function(gfx)
{
    var segmentOffset = gfx.command.w1;
    var params = gfx.command.w0f(16, 8);
    var flagPush = params & 0x04;
    var flagLoad = params & 0x02;
    var flagProj = params & 0x01; 
    gfx.loadMatrix(segmentOffset, flagPush, flagLoad, flagProj);
}

GfxOps.gsSPTexture_f3d = function(gfx)
{
    var cmd = gfx.command;
    var tile = cmd.w0f(8, 3);
    var tileDesc = gfx.dpTileDescriptors[tile];

    tileDesc.levels = cmd.w0f(11,3);
    tileDesc.on = cmd.w0f(0, 1);
    tileDesc.scaleS = cmd.w1f(16, 16);
    tileDesc.scaleT = cmd.w1f(0, 16);
}

GfxOps.gsSPSetGeometryMode_f3d = function(gfx)
{
    var mode = gfx.spGeometryMode;
    var cmd = gfx.command;

    mode.zbuffer |= cmd.w1f(0, 1);
    mode.shade |= cmd.w1f(2, 1);
    mode.cull_front |= cmd.w1f(12, 1)
    mode.cull_back |= cmd.w1f(13, 1);
    mode.fog |= cmd.w1f(16, 1);
    mode.lighting |= cmd.w1f(17, 1);
    mode.texture_gen |= cmd.w1f(18, 1);
    mode.texture_gen_linear |= cmd.w1f(19, 1);
    mode.shading_smooth |= cmd.w1f(9, 1);
    mode.g_clipping |= cmd.w1f(23, 1);

    console.log('set shade', mode.shade);
}

GfxOps.gsSPClearGeometryMode_f3d = function(gfx)
{
    var mode = gfx.spGeometryMode;
    var cmd = gfx.command;

    mode.zbuffer &= ~cmd.w1f(0, 1);
    mode.shade &= ~cmd.w1f(2, 1);
    mode.cull_front &= ~cmd.w1f(12, 1)
    mode.cull_back &= ~cmd.w1f(13, 1);
    mode.fog &= ~cmd.w1f(16, 1);
    mode.lighting &= ~cmd.w1f(17, 1);
    mode.texture_gen &= ~cmd.w1f(18, 1);
    mode.texture_gen_linear &= ~cmd.w1f(19, 1);
    mode.shading_smooth &= ~cmd.w1f(9, 1);
    mode.g_clipping &= ~cmd.w1f(23, 1);

    console.log('clear shade', mode.shade);
}

GfxOps.Fast3D = [
    [ 0x01, GfxOps.gsSPMatrix_f3d ],
    [ 0xBC, GfxOps.gsMoveWd_f3d ],
    [ 0xBD, GfxOps.gsSPPopMatrix ],
    [ 0xBF, GfxOps.gsSP1Triangle_f3d ],
    [ 0x04, GfxOps.gsSPVertex_f3d ],
    [ 0x06, GfxOps.gsSPDisplayList ],
    [ 0xB8, GfxOps.gsSPEndDisplayList ],
    [ 0xB9, GfxOps.gsSPSetOtherMode_L ],
    [ 0xBB, GfxOps.gsSPTexture_f3d ],
    [ 0xB6, GfxOps.gsSPClearGeometryMode_f3d ],
    [ 0xB7, GfxOps.gsSPSetGeometryMode_f3d ],
];
