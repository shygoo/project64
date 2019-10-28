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
    //console.log("numv", numVertices, "index", index);

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
    //console.log("POP")
    gfx.spMatrixIndex--;
}

GfxOps.gsSP2Triangles = function(gfx)
{
    var mat4 = gfx.spMatrixStack[gfx.spMatrixIndex];

    var v = new Array(6);
    var vtrans = new Array(6);

    v[0] = gfx.spVertices[((gfx.command.w0 >> 16) & 0xFF) / 2];
    v[1] = gfx.spVertices[((gfx.command.w0 >> 8) & 0xFF) / 2];
    v[2] = gfx.spVertices[((gfx.command.w0 >> 0) & 0xFF) / 2];
    v[3] = gfx.spVertices[((gfx.command.w1 >> 16) & 0xFF) / 2];
    v[4] = gfx.spVertices[((gfx.command.w1 >> 8) & 0xFF) / 2];
    v[5] = gfx.spVertices[((gfx.command.w1 >> 0) & 0xFF) / 2];

    for(var i = 0; i < 6; i++)
    {
        vtrans[i] = v[i].clone();
        vtrans[i].applyMatrix4(mat4);
    }

    gfx.triangles.push([vtrans[0], vtrans[1], vtrans[2]]);
    gfx.triangles.push([vtrans[3], vtrans[4], vtrans[5]]);
}

GfxOps.F3DEX2 = [
    [ 0x06, GfxOps.gsSP2Triangles ],
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
