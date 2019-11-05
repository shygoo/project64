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
    gfx.loadMatrix(segmentOffset, flagPush | flagLoad | flagProj);
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
    //gfx.triangles.push(tri1);
    //gfx.triangles.push(tri2);
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
    //gfx.triangles.push(tri);
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

    // todo mirroring
    for(var i = 0; i < count+1; i++)
    {
        var color = gfx.getU16(gfx.dpImageAddress + i*2);
        gfx.dpTextureMemory.setUint16(tmemOffset + i*2, color);
    }
}

GfxOps.RDP = [
    [ 0xF0, GfxOps.gsDPLoadTLUTCmd ],
    [ 0xF2, GfxOps.gsDPSetTileSize ],
    [ 0xF3, GfxOps.gsDPLoadBlock ],
    [ 0xF5, GfxOps.gsDPSetTile ],
    [ 0xFD, GfxOps.gsDPSetTextureImage ],
];

GfxOps.F3DEX2 = [
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

