const G_IM_SIZ_4b  = 0;
const G_IM_SIZ_8b  = 1;
const G_IM_SIZ_16b = 2;
const G_IM_SIZ_32b = 3;

const G_IM_FMT_RGBA = 0;
const G_IM_FMT_YUV  = 1;
const G_IM_FMT_CI   = 2;
const G_IM_FMT_IA   = 3;
const G_IM_FMT_I    = 4;

function DmemVertex(position, texcoords, color, normal, alpha)
{
    this.position = position; // THREE.Vector3
    this.texcoords = texcoords; // THREE.Vector2
    this.color = color; // THREE.Color
    this.alpha = alpha; // float
    this.normal = normal; // THREE.Vector3
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
        case G_IM_SIZ_4b:  return 0;
        case G_IM_SIZ_8b:  return 1;
        case G_IM_SIZ_16b: return 2;
        case G_IM_SIZ_32b: return 4;
    }
}

function PolyList(materialIndex)
{
    this.materialIndex = materialIndex;
    this.triangles = [];
}

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

GfxState.prototype.addTriangle = function(tri)
{
    var currentPolyList = this.polylists[this.polylists.length - 1];
    currentPolyList.triangles.push(tri);
}

GfxState.prototype.updateCurrentMaterial = function()
{
    var currentMaterial = this.getCurrentMaterial();

    if(currentMaterial == null)
    {
        this.materials.push(this.createMaterial());
        this.materialIndex = this.materials.length - 1;
        this.polylists.push(new PolyList(this.materialIndex));
        return;
    }

    var hash = this.generateMaterialHash();

    if(currentMaterial.hash == hash)
    {
        // material has not changed
        return;
    }

    var existingMaterialIndex = this.getMaterialIndex(hash);

    if(existingMaterialIndex != -1)
    {
        // material changed back to an existing material
        this.materialIndex = existingMaterialIndex;
        this.polylists.push(new PolyList(this.materialIndex));
    }
    else
    {
        this.materials.push(this.createMaterial());
        this.materialIndex = this.materials.length - 1;
        this.polylists.push(new PolyList(this.materialIndex));
    }
}

// used to check if a material has already been generated for the current gfx state
GfxState.prototype.generateMaterialHash = function()
{
    return this.dpImageAddress; // todo more
}

// create intermediate "material" to attach to the current polygon list
GfxState.prototype.createMaterial = function()
{
    var material = {};
    material.vertexColorsEnabled = false;
    material.vertexNormalsEnabled = false;
    material.imageIndex = this.getImageIndex(this.dpImageAddress);

    if(material.imageIndex == -1)
    {
        this.images.push(this.createRenderTileImage());
        material.imageIndex = this.images.length - 1;
    }

    material.hash = this.generateMaterialHash(); // +...

    return material;
}

GfxState.prototype.getImageIndex = function(address)
{
    for(var i = 0; i < this.images.length; i++)
    {
        if(this.images[i].address == address)
        {
            return i;
        }
    }
    return -1;
}

GfxState.prototype.getMaterialIndex = function(hash)
{
    for(var i = 0; i < this.materials.length; i++)
    {
        if(this.materials[i].hash == hash)
        {
            return i;
        }
    }
    return -1;
}

GfxState.prototype.createRenderTileImage = function()
{
    //for(var i = 0; i < )
    var imageData = null;
    var renderTile = this.dpTileDescriptors[0];
    var imageWidth = (renderTile.lrS >> 2) + 1;
    var imageHeight = (renderTile.lrT >> 2) + 1;
    var imageSize = imageWidth * imageHeight;

    if(renderTile.fmt == G_IM_FMT_CI && renderTile.siz == G_IM_SIZ_8b)
    {
        // todo check textlut for G_TT_RGBA16 or G_TT_IA16
        imageData = new Uint8Array(imageSize * 4);
        var tmemColorOffset = 256 * 8;
        var tmemCiOffset = renderTile.tmem * 8;

        for(var i = 0; i < imageSize; i++)
        {
            var ci = this.dpTextureMemory.getUint8(tmemCiOffset + i);
            var color16 = this.dpTextureMemory.getUint16(tmemColorOffset + ci*2);

            var r = (color16 >> 11) & 0x1F;
            var g = (color16 >> 6) & 0x1F;
            var b = (color16 >> 1) & 0x1F;
            var a = (color16 >> 0) & 0x01;

            const factor = 0xFF / 0x1F;

            imageData[i*4+0] = r * factor;
            imageData[i*4+1] = g * factor;
            imageData[i*4+2] = b * factor;
            imageData[i*4+3] = a * 255;
        }
        //material.imageData = imageData;
        //console.log(imageData);
    }
    else if(renderTile.fmt == G_IM_FMT_RGBA && renderTile.siz == G_IM_SIZ_16b)
    {
        imageData = new Uint8Array(imageSize * 4);
        var tmemOffset = renderTile.tmem * 8;

        if(imageSize <= 2048)
        for(var i = 0; i < imageSize; i++)
        {
            var color16 = this.dpTextureMemory.getUint16(tmemOffset + i*2);
            var r = (color16 >> 11) & 0x1F;
            var g = (color16 >> 6) & 0x1F;
            var b = (color16 >> 1) & 0x1F;
            var a = (color16 >> 0) & 0x01;
            const factor = 0xFF / 0x1F;
            imageData[i*4+0] = r * factor;
            imageData[i*4+1] = g * factor;
            imageData[i*4+2] = b * factor;
            imageData[i*4+3] = a * 255;
        }
    }
    else if(renderTile.fmt == G_IM_FMT_I && renderTile.siz == G_IM_SIZ_8b)
    {
        console.log("i8:", imageWidth, imageHeight);

        imageData = new Uint8Array(imageSize * 4);
        var tmemOffset = renderTile.tmem * 8;

        if(imageSize <= 2048)
        for(var i = 0; i < imageSize; i++)
        {
            var intensity8 = this.dpTextureMemory.getUint8(tmemOffset + i);
            imageData[i*4+0] = intensity8;
            imageData[i*4+1] = intensity8;
            imageData[i*4+2] = intensity8;
            imageData[i*4+3] = intensity8;
        }
    }
    else
    {
        console.log("need converter, fmt: " + renderTile.fmt + " siz: " + renderTile.siz);
    }

    return { address: this.dpImageAddress, data: imageData, width: imageWidth, height: imageHeight };
}

GfxState.prototype.getCurrentMaterial = function()
{
    if(this.materials.length == 0) return null;
    return this.materials[this.materialIndex];
}

GfxState.prototype.resetState = function()
{
    this.command = null;
    
    this.polylists = [];
    this.materials = [];
    this.images = []; // cached rgba32 images
    this.materialIndex = 0;

    this.spCommandAddress = 0;
    this.spSegments = new Array(16).fill(0);
    this.spReturnStack = new Array(16).fill(0);
    this.spReturnIndex = 0;
    this.spVertices = new Array(32);
    this.spMatrixStack = new Array(10);
    this.spMatrixIndex = 0;
    this.spProjectionMatrix = new THREE.Matrix4();
    this.dpImageAddress = 0;
    this.dpTextureMemory = new DataView(new ArrayBuffer(4096));
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

        var u = this.getS16(offset + 0x08) / 32;
        var v = this.getS16(offset + 0x0A) / 32;

        var r = this.getU8(offset + 0x0C) / 0xFF;
        var g = this.getU8(offset + 0x0D) / 0xFF;
        var b = this.getU8(offset + 0x0E) / 0xFF;

        var nx = this.getS8(offset + 0x0C) / 0x7F;
        var ny = this.getS8(offset + 0x0D) / 0x7F;
        var nz = this.getS8(offset + 0x0E) / 0x7F;

        var alpha = this.getU8(offset + 0x0F) / 0xFF;

        var color = new THREE.Color(r, g, b);
        var normal = new THREE.Vector3(nx, ny, nz);
        var position = new THREE.Vector3(x, y, z);
        var texcoords = new THREE.Vector2(u, v);
        position.applyMatrix4(this.spMatrixStack[this.spMatrixIndex]);

        this.spVertices[index + i] = new DmemVertex(position, texcoords, color, normal, alpha);
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

GfxState.prototype.getU8 = function(segmentOffset)
{
    var src = this._get(segmentOffset);
    return src.dv.getUint8(src.offset);
}

GfxState.prototype.getS8 = function(segmentOffset)
{
    var src = this._get(segmentOffset);
    return src.dv.getInt8(src.offset);
}