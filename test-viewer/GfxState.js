


function DmemVertex(position, texcoords, colors, matrixIndex)
{
    this.position = position;
    this.texcoords = texcoords;
    this.colors = colors;
}


/////////////

function GfxState()
{
}

GfxState.prototype.setup = function(dv)
{
    this.dv = dv;

    var metaOffset = dv.byteLength - 0x14;
    this.meta = {
        signature:              dv.getUint32(metaOffset + 0x00, false),
        microcodeAddress:       dv.getUint32(metaOffset + 0x04, true),
        microcodeChecksum:      dv.getUint32(metaOffset + 0x08, true),
        rootDisplayListAddress: dv.getUint32(metaOffset + 0x0C, true),
        rootDisplayListSize:    dv.getUint32(metaOffset + 0x10, true)
    };
    
    this.commandFunctions = new Array(256).fill(null);
    this.command = null;
    
    this.triangles = [];

    this.spCommandAddress = this.meta.rootDisplayListAddress;
    this.spSegments = new Array(16).fill(0);
    this.spReturnStack = new Array(16).fill(0);
    this.spReturnIndex = 0;
    this.spVertices = new Array(32);
    this.spMatrixStack = new Array(10);
    this.spMatrixIndex = 0;
    this.spProjectionMatrix = new THREE.Matrix4();

    for(var i = 0; i < this.spMatrixStack.length; i++)
    {
        this.spMatrixStack[i] = new THREE.Matrix4();
    }

    for(var i = 0; i < this.spVertices.length; i++)
    {
        this.spVertices[i] = new DmemVertex();
    }

    this.importCommandFunctions(GfxOps.F3DEX2);
}

GfxState.prototype.run = function(dv)
{
    this.setup(dv);

    var numCommands = 0;

    while(this.spReturnIndex >= 0 && numCommands < 20000)
    {
        this.command = this.getCommand(this.spCommandAddress);
        this.spCommandAddress += 8;

        //this.debugLogStep();

        var commandFunction = this.commandFunctions[this.command.commandByte()];

        if(commandFunction != null)
        {
            commandFunction(this);
        }
        
        numCommands++;
    }
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

GfxState.prototype.debugLogStep = function()
{
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

GfxState.prototype.loadVertices = function(segmentOffset, index, numVertices)
{
    var paddr = this.segmentedToPhysical(segmentOffset);

    for(var i = 0; i < numVertices; i++)
    {
        var offset = paddr + (i * 16);

        var x = this.dv.getInt16(offset + 0x00);
        var y = this.dv.getInt16(offset + 0x02);
        var z = this.dv.getInt16(offset + 0x04);

        var position = new THREE.Vector3(x, y, z);
        position.applyMatrix4(this.spMatrixStack[this.spMatrixIndex]);

        //var vertex 
        this.spVertices[index + i] = new DmemVertex(position, null, null);
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

    for(var i = 0; i < 16; i++)
    {
        var offset = paddr + (i * 2);
        var intpart = this.dv.getUint16(offset);
        var fracpart = this.dv.getUint16(offset + 32);
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
