


function DmemVertex(position, texcoords, colors, matrixIndex)
{
    this.position = position;
    this.texcoords = texcoords;
    this.colors = colors;
}

function TileDescriptor()
{
    // gsSPTexture
    this.on = 0;
    this.scaleS = 0;
    this.scaleT = 0;
    this.levels = 0;

    // gsDPSetTile
    this.fmt = 0;
    this.siz = 0;
    this.line = 0;
    this.tmem = 0;
    this.palette = 0;
    this.cmT = 0;
    this.maskT = 0;
    this.shiftT = 0;
    this.cmS = 0;
    this.maskS = 0;
    this.shiftS = 0;

    // gsDPSetTileSize
    this.ulS = 0;
    this.ulT = 0;
    this.lrS = 0;
    this.lrT = 0;
}

TileDescriptor.bytesPerTexel = function(siz)
{
    switch(siz)
    {
        case TileDescriptor.G_IM_SIZ_4b:  return 0;
        case TileDescriptor.G_IM_SIZ_8b:  return 1;
        case TileDescriptor.G_IM_SIZ_16b: return 2;
        case TileDescriptor.G_IM_SIZ_32b: return 4;
    }
}

/*
gsSPTexture         scaleS 0xFFFF, scaleT 0xFFFF, levels 0, tile 0, on 1
gsDPSetTextureImage G_IM_FMT_RGBA, G_IM_SIZ_16b, addr 0x00000000
gsDPSetTile         G_IM_FMT_RGBA, G_IM_SIZ_16b, line  0, tmem 0x000, tile 7, palette 0, cmt 2, maskt 4, shiftt 0, cmt 2, masks 7, shifts 0
gsDPLoadSync
gsDPLoadBlock       tile 7, uls 0, ult 0, lrs 2047, dxt 64
gsDPSetTile         G_IM_FMT_RGBA, G_IM_SIZ_16b, line 32, tmem 0x000, tile 0, palette 0, cmt 2, maskt 4, shiftt 0, cmt 2, masks 7, shifts 0
gsDPSetTileSize     tile 0, uls 0, ult 0, lrs 508, lrt 60
*/


/////////////

/*
GfxState(config)
gfx.setup(dv)
gfx.run(dv)
gfx.setSegmentAddress(segment, address)
gfx.setSegmentBuffer(segment, address)
gfx.readWord(segmentOffset)
gfx.segmentedToPhysical(segmentOffset)
gfx.getCommand(segmentOffset)
gfx.importOps(opsMain, opsPatch)
gfx.loadVertices(segmentOffset, index, numVertices)
gfx.loadMatrix(segmentOffset, flags)

*/

function GfxState()
{
    this.dv = null; // main memory
    this.spSegmentBuffers = new Array(16).fill(null); // overrides spSegments
    this.commandFunctions = new Array(256).fill(null);
    this.resetState();
}

GfxState.prototype.setMainMemory = function(dv)
{
    this.dv = dv;
}

GfxState.prototype.resetState = function()
{
    this.command = null;
    
    this.triangles = [];

    this.spCommandAddress = 0;
    this.spSegments = new Array(16).fill(0);
    this.spReturnStack = new Array(16).fill(0);
    this.spReturnIndex = 0;
    this.spVertices = new Array(32);
    this.spMatrixStack = new Array(10);
    this.spMatrixIndex = 0;
    this.spProjectionMatrix = new THREE.Matrix4();
    this.dpImageAddress = 0;
    this.dpTextureMemory = new Uint8Array(4096);
    this.dpTileDescriptors = new Array(8);

    for(var i = 0; i < this.dpTileDescriptors.length; i++)
    {
        this.dpTileDescriptors[i] = new TileDescriptor();
    }

    for(var i = 0; i < this.spMatrixStack.length; i++)
    {
        this.spMatrixStack[i] = new THREE.Matrix4();
    }

    for(var i = 0; i < this.spVertices.length; i++)
    {
        this.spVertices[i] = new DmemVertex();
    }
}

GfxState.prototype.run = function(address)
{
    this.resetState();
    this.spCommandAddress = address;
    var numCommands = 0;

    while(this.spReturnIndex >= 0 && numCommands < 20000)
    {
        this.command = this.getCommand(this.spCommandAddress);
        this.spCommandAddress += 8;

        var commandFunction = this.commandFunctions[this.command.commandByte()];

        //this.debugLogStep();

        if(commandFunction != null)
        {
            commandFunction(this);
        }
        
        numCommands++;
    }
}

GfxState.prototype.setSegmentAddress = function(segment, address)
{
    this.spSegmentBuffers[segment] = null;
    this.spSegments[segment] = address;
}

GfxState.prototype.setSegmentBuffer = function(segment, buffer)
{
    this.spSegments[segment] = 0;
    this.spSegmentBuffers[segment] = buffer;
}

GfxState.prototype._get = function(segmentOffset)
{
    var segment = (segmentOffset >>> 24) & 0x0F;
    var offset = (segmentOffset & 0x00FFFFFF);

    if(this.spSegmentBuffers[segment] != null)
    {
        return { dv: this.spSegmentBuffers[segment], offset: offset };
    }

    return { dv: this.dv, offset: this.segmentedToPhysical(segmentOffset) };
}

GfxState.prototype.getU32 = function(segmentOffset)
{
    var src = this._get(segmentOffset);
    return src.dv.getUint32(src.offset);
}

GfxState.prototype.getU16 = function(segmentOffset)
{
    var src = this._get(segmentOffset);
    return src.dv.getUint16(src.offset);
}

GfxState.prototype.getS16 = function(segmentOffset)
{
    var src = this._get(segmentOffset);
    return src.dv.getInt16(src.offset);
}

GfxState.prototype.segmentedToPhysical = function(segmentOffset)
{
    var segment = (segmentOffset >>> 24) & 0x0F;
    var offset = segmentOffset & 0x00FFFFFF;
    return (this.spSegments[segment] & 0x00FFFFFF) + offset;
}

GfxState.prototype.getCommand = function(segmentOffset)
{
    var word0 = this.getU32(segmentOffset + 0);
    var word1 = this.getU32(segmentOffset + 4)
    return new GfxCommand(word0, word1);
}

GfxState.prototype._importCommandFunctions = function(arr)
{
    for(var i = 0; i < arr.length; i++)
    {
        var commandByte = arr[i][0];
        var commandFunc = arr[i][1];
        this.commandFunctions[commandByte] = commandFunc;
    }
}

GfxState.prototype.importOps = function(dpOps, spOps, spOpsPatch)
{
    for(var i = 0; i < 256; i++)
    {
        this.commandFunctions[i] = null;
    }

    this._importCommandFunctions(dpOps);
    this._importCommandFunctions(spOps);

    if(spOpsPatch)
    {
        this._importCommandFunctions(spOpsPatch);
    }
}

GfxState.prototype.loadVertices = function(segmentOffset, index, numVertices)
{
    for(var i = 0; i < numVertices; i++)
    {
        var offset = segmentOffset + (i * 16);

        var x = this.getS16(offset + 0x00);
        var y = this.getS16(offset + 0x02);
        var z = this.getS16(offset + 0x04);

        var position = new THREE.Vector3(x, y, z);
        position.applyMatrix4(this.spMatrixStack[this.spMatrixIndex]);

        this.spVertices[index + i] = new DmemVertex(position, null, null);
    }
}

GfxState.prototype.loadMatrix = function(segmentOffset, flags)
{
    var bPush = !!(flags & 0x01);
    var bLoad = !!(flags & 0x02);
    var bProj = !!(flags & 0x04);

    var dmemSrcMtx = null;
    var dmemDstMtx = null;
    var dramSrcMtx = new THREE.Matrix4();

    for(var i = 0; i < 16; i++)
    {
        var offset = segmentOffset + (i * 2);
        var intpart = this.getU16(offset);
        var fracpart = this.getU16(offset + 32);
        var fixed = (intpart << 16) | fracpart; // signed
        dramSrcMtx.elements[i] = fixed / 65536;
    }

    if(bProj)
    {
        dmemSrcMtx = this.spProjectionMatrix;
        dmemDstMtx = this.spProjectionMatrix;
    }
    else
    {
        dmemSrcMtx = this.spMatrixStack[this.spMatrixIndex];

        if(bPush)
        {
            this.spMatrixIndex++;
        }

        dmemDstMtx = this.spMatrixStack[this.spMatrixIndex];
    }

    if(bLoad)
    {
        dmemDstMtx.copy(dramSrcMtx)
    }
    else
    {
        dmemDstMtx.multiplyMatrices(dmemSrcMtx, dramSrcMtx);
    }
}

GfxState.prototype.debugLogStep = function()
{
    console.log(this.command.w0.toString(16), this.command.w0.toString(16))

    switch(this.command.commandByte())
    {
    case 0x01: console.log("vertex"); break;
    case 0x02: console.log("modvertex"); break;
    case 0x05: console.log("tri1"); break;
    case 0x06: console.log("tri2"); break;
    case 0x07: console.log("quad7"); break;
    case 0x04: console.log("branch_z"); break;
    case 0x03: console.log("culldl"); break;
    }
}