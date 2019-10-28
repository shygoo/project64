

function getMeta(dv)
{
    var offset = dv.byteLength - 0x14;

    var meta = {
        signature:              dv.getUint32(offset + 0x00, false),
        microcodeAddress:       dv.getUint32(offset + 0x04, true),
        microcodeChecksum:      dv.getUint32(offset + 0x08, true),
        rootDisplayListAddress: dv.getUint32(offset + 0x0C, true),
        rootDisplayListSize:    dv.getUint32(offset + 0x10, true)
    };
    
    //console.log(meta);

    return meta;
}

/////////////

function GfxState(dv)
{
    this.dv = dv;
    this.meta = getMeta(dv);
    this.commandFunctions = new Array(256).fill(null);
    this.command = null;
    
    this.triangles = [];

    this.spCommandAddress = this.meta.rootDisplayListAddress;
    this.spSegments = new Array(16).fill(0);
    this.spReturnStack = new Array(16).fill(0);
    this.spReturnIndex = 0;
    this.spVertices = new Array(32).fill(null);
    this.spMatrixStack = new Array(16);
    this.spMatrixIndex = 0;
    this.spProjectionMatrix = new THREE.Matrix4();
    //this.spProjectionMatrix.elements.fill(0);

    for(var i = 0; i < 16; i++)
    {
        this.spMatrixStack[i] = new THREE.Matrix4();
        //this.spMatrixStack[i].elements.fill(0);
        //console.log(this.spMatrixStack[i])
    }

    //console.log(GfxOps.F3DEX2)

    this.importCommandFunctions(GfxOps.F3DEX2);

    //for(var i = 0; i < this.commandFunctions.length; i++)
    //{
    //    console.log(this.commandFunctions[i])
    //}

    console.log(this.spCommandAddress.toString(16))
}

GfxState.prototype.segmentedToPhysical = function(segmentOffset)
{
    var segment = (segmentOffset >>> 24) & 0x0F;
    var offset = segmentOffset & 0x00FFFFFF;
    return (this.spSegments[segment] & 0x00FFFFFF) + offset;
}

GfxState.prototype.getCommand = function(segmentOffset)
{
    var paddr = this.segmentedToPhysical(segmentOffset);
    var word0 = this.dv.getUint32(paddr + 0);
    var word1 = this.dv.getUint32(paddr + 4);
    return new GfxCommand(word0, word1);
}

GfxState.prototype.importCommandFunctions = function(arr)
{
    for(var i = 0; i < arr.length; i++)
    {
        var commandByte = arr[i][0];
        var commandFunc = arr[i][1];
        this.commandFunctions[commandByte] = commandFunc;
    }
}

GfxState.prototype.run = function()
{
    //var debugCount = 0;

    while(this.spReturnIndex >= 0)
    {
        this.command = this.getCommand(this.spCommandAddress);
        this.spCommandAddress += 8;

        var commandFunction = this.commandFunctions[this.command.commandByte()];

        if(commandFunction != null)
        {
            commandFunction(this);
        }
        
        //console.log(this.command.w0.toString(16), this.command.w1.toString(16))

        //if(debugCount > 20)
        //{
        //    break;
        //}
        //debugCount++;
    }

    //console.log("done");
    //console.log(debugCount)
}


GfxState.prototype.loadVertices = function(segmentOffset, index, numVertices)
{
    var paddr = this.segmentedToPhysical(segmentOffset);

    for(var i = 0; i < numVertices; i++)
    {
        var offs = paddr + i * 16;

        var x = this.dv.getInt16(offs + 0x00);
        var y = this.dv.getInt16(offs + 0x02);
        var z = this.dv.getInt16(offs + 0x04);

        this.spVertices[index + i] = new THREE.Vector3(x, y, z);
    }
}

GfxState.prototype.loadMatrix = function(segmentOffset, flags)
{
    var paddr = this.segmentedToPhysical(segmentOffset);

    var bPush = !!(flags & 0x01);
    var bLoad = !!(flags & 0x02);
    var bProj = !!(flags & 0x04);

    var dmemSrcMtx = null;
    var dmemDstMtx = null;
    var dramSrcMtx = new THREE.Matrix4();
    //dramSrcMtx.elements.fill(0);

    for(var i = 0; i < 16; i++)
    {
        var offs = paddr + i*2;

        var intpart = this.dv.getUint16(offs);
        var fracpart = this.dv.getUint16(offs + 32);

        var fixed = (intpart << 16) | fracpart; // signed
        var f = fixed / 65536;
        dramSrcMtx.elements[i] = f;
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
        //console.log("load", dmemSrcMtx, dramSrcMtx)
        //glMatrix.mat4.copy(dmemDstMtx, dramSrcMtx);
        dmemDstMtx.copy(dramSrcMtx)
    }
    else
    {
        //glMatrix.mat4.multiply(dmemDstMtx, dmemSrcMtx, dramSrcMtx);
        dmemDstMtx.multiplyMatrices(dmemSrcMtx, dramSrcMtx);
    }

    //console.log(dramMtx.toString())
}
