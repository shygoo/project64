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
        //gfx.spSegments[segment] = gfx.command.w1;
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
    var v = new Array(6);
    var w0 = gfx.command.w0;
    var w1 = gfx.command.w1;
    v[0] = gfx.spVertices[((w0 >> 16) & 0xFF) / 2].position;
    v[1] = gfx.spVertices[((w0 >> 8) & 0xFF) / 2].position;
    v[2] = gfx.spVertices[((w0 >> 0) & 0xFF) / 2].position;
    v[3] = gfx.spVertices[((w1 >> 16) & 0xFF) / 2].position;
    v[4] = gfx.spVertices[((w1 >> 8) & 0xFF) / 2].position;
    v[5] = gfx.spVertices[((w1 >> 0) & 0xFF) / 2].position;
    gfx.triangles.push([v[0], v[1], v[2]]);
    gfx.triangles.push([v[3], v[4], v[5]]);
}

GfxOps.gsSP1Triangle = function(gfx)
{
    var v = new Array(3);
    var w0 = gfx.command.w0;
    v[0] = gfx.spVertices[((w0 >> 16) & 0xFF) / 2].position;
    v[1] = gfx.spVertices[((w0 >> 8) & 0xFF) / 2].position;
    v[2] = gfx.spVertices[((w0 >> 0) & 0xFF) / 2].position;
    gfx.triangles.push([v[0], v[1], v[2]]);
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

    // test
    var numTexels = lrs;
    var numBytesPerTexel = TileDescriptor.bytesPerTexel(tileDesc.siz);
    var numBytes;

    if(numBytesPerTexel == 0) // 4b
    {
        numBytes = numTexels / 2;
    }
    else
    {
        numBytes = numTexels * numBytesPerTexel;
    }
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
    var tile = cmd.w0f(24, 3);
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

GfxOps.RDP = [
    [0xFD, GfxOps.gsDPSetTextureImage ]
];

GfxOps.F3DEX2 = [
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

