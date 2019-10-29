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
        gfx.spSegments[segment] = gfx.command.w1;
    }
}

GfxOps.gsSPVertex = function(gfx)
{
    var numVertices = (gfx.command.w0 >> 12 & 0xFF);
    var index = ((gfx.command.w0 & 0xFF) - numVertices * 2) / 2;
    gfx.loadVertices(gfx.command.w1, index, numVertices+1);
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

//GfxOps.F3DEX2.G_MTX_PUSH = 0x01;
//GfxOps.F3DEX2.G_MTX_PUSH = 0x01;
//GfxOps.F3DEX2.G_MTX_PUSH = 0x01;
