const G_IM_FMT_RGBA = 0;
const G_IM_FMT_YUV  = 1;
const G_IM_FMT_CI   = 2;
const G_IM_FMT_IA   = 3;
const G_IM_FMT_I    = 4;

const G_IM_SIZ_4b  = 0;
const G_IM_SIZ_8b  = 1;
const G_IM_SIZ_16b = 2;
const G_IM_SIZ_32b = 3;

// cmt/cms flags
const G_TX_MIRROR = 1;
const G_TX_NOMIRROR = 0;
const G_TX_WRAP = 0;
const G_TX_CLAMP = 2;

function SPVertex(position, texcoords, color, normal, alpha)
{
    this.position = position; // THREE.Vector3
    this.texcoords = texcoords; // THREE.Vector2
    this.color = color; // THREE.Color
    this.alpha = alpha; // float
    this.normal = normal; // THREE.Vector3
}

function SPLight()
{
    this.color = 0;
    this.colorCopy = 0;
    this.direction = {x: 0, y: 0, z: 0};
}

function DPTileDescriptor()
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

DPTileDescriptor.bytesPerTexel = function(siz)
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


    this.commandFunctions = new Array(256).fill(null);
    this.loghtm = "";
    this.resetState();
}

GfxState.prototype.log = function(htm)
{
    this.loghtm += htm;
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
    var hash = 0;

    hash = this.lastLoadBlock*1 +
           this.spGeometryMode.lighting*2 +
           this.spGeometryMode.shade*3 +
           this.dpTileDescriptors[0].on*4; // todo more

    //if(this.spGeometryMode.lighting)
    //{
    //    for(var j = 0; j < 8; j++)
    //    {
    //        hash += (this.spLights[j].color >>> 8)*5;
    //    }
    //}

    return hash;
}

// create intermediate "material" to attach to the current polygon list
GfxState.prototype.createMaterial = function()
{
    var material = {};

    //var gmLighting = this.spGeometryMode.lighting;
    //var gmShade = this.spGeometryMode.shade;
    
    var renderMode = (this.dpOtherModeL >> 3) & 0b1111111111111;
    material.bFog = !!this.spGeometryMode.fog;
    material.bDecal = ((renderMode >> 7) & 3) == 3;
    //material.vertexColorsEnabled = (gmLighting == 0) && (gmShade == 1);
    //material.vertexNormalsEnabled = (gmLighting == 1) && (gmShade == 1);
    
    //if(gmLighting)
    //{
    //    material.numLights = this.spNumLights;
    //    material.lights = Object.assign({}, this.spLights);
    //}

    var renderTile = this.dpTileDescriptors[0];

    if(renderTile.on)
    {
        material.imageIndex = this.getImageIndex(this.lastLoadBlock);

        if(material.imageIndex == -1)
        {
            material.cmT = renderTile.cmT;
            material.cmS = renderTile.cmS;
            material.scaleS = renderTile.scaleS / 0x10000;
            material.scaleT = renderTile.scaleT / 0x10000;
    
            this.images.push(this.createRenderTileImage());
            material.imageIndex = this.images.length - 1;
        }
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
    var renderTile = this.dpTileDescriptors[0];
    var imageWidth = (renderTile.lrS >> 2) + 1;
    var imageHeight = (renderTile.lrT >> 2) + 1;
    var imageSize = imageWidth * imageHeight;
    var imageData = new Uint8Array(imageSize * 4);
    var imageDV = new DataView(imageData.buffer);

    if(renderTile.siz != G_IM_SIZ_4b && imageSize > 2048 ||
       renderTile.siz == G_IM_SIZ_4b && imageSize > 4096)
    {
        console.log("image too big?", imageWidth, imageHeight);
        imageData = null;
        return { address: this.lastLoadBlock, data: imageData, width: imageWidth, height: imageHeight };
    }

    var tmemOffset = renderTile.tmem * 8;

    var convertFunc = null;

    function convertCI8(ci)
    {
        var tmemColorOffset = 256 * 8; 
        var color = this.dpTextureMemory.getUint16(tmemColorOffset + ci*2);
        return RGBA32.fromRGBA16(color); // todo check TEXTLUT for format
    }

    function convertCI4(ci)
    {
        var tmemColorOffset = 256 * 8; // todo check tile descriptor's palette number
        var color = this.dpTextureMemory.getUint16(tmemColorOffset + ci*2);
        return RGBA32.fromRGBA16(color); // todo check TEXTLUT for format
    }

    if(renderTile.siz == G_IM_SIZ_8b)
    {
        switch(renderTile.fmt)
        {
        case G_IM_FMT_I:  convertFunc = RGBA32.fromI8; break;
        case G_IM_FMT_IA: convertFunc = RGBA32.fromIA8; break;
        case G_IM_FMT_CI: convertFunc = convertCI8.bind(this); break;
        }

        for(var i = 0; i < imageSize; i++)
        {
            var texel = this.dpTextureMemory.getUint8(tmemOffset + i);
            imageDV.setUint32(i*4, convertFunc(texel));
        }
    }
    else if(renderTile.siz == G_IM_SIZ_16b)
    {
        switch(renderTile.fmt)
        {
        case G_IM_FMT_RGBA: convertFunc = RGBA32.fromRGBA16; break;
        case G_IM_FMT_IA:   convertFunc = RGBA32.fromIA16; break;
        }

        for(var i = 0; i < imageSize; i++)
        {
            var texel = this.dpTextureMemory.getUint16(tmemOffset + i*2);
            imageDV.setUint32(i*4, convertFunc(texel));
        }
    }
    else if(renderTile.siz == G_IM_SIZ_4b)
    {
        switch(renderTile.fmt)
        {
        case G_IM_FMT_I:  convertFunc = RGBA32.fromI4; break;
        case G_IM_FMT_IA: convertFunc = RGBA32.fromIA4; break;
        case G_IM_FMT_CI: convertFunc = convertCI4.bind(this); break;
        case G_IM_FMT_RGBA: convertFunc = convertCI4.bind(this); break; // micro machines
        }

        for(var i = 0; i < imageSize / 2; i++)
        {
            var pair = this.dpTextureMemory.getUint8(tmemOffset + i);
            var texel0 = (pair >> 4) & 0x0F;
            var texel1 = (pair >> 0) & 0x0F;
            imageDV.setUint32(i*8 + 0, convertFunc(texel0));
            imageDV.setUint32(i*8 + 4, convertFunc(texel1));
        }
    }
    else
    {
        console.log("need converter, fmt: " + renderTile.fmt + " siz: " + renderTile.siz);
        imageData = null;
    }

    return { address: this.lastLoadBlock, data: imageData, width: imageWidth, height: imageHeight };
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
    this.spSegments = new Array(16).fill(0); // addresses when using main memory
    this.spSegmentBuffers = new Array(16).fill(null); // overrides spSegments
    this.spReturnStack = new Array(16).fill(0);
    this.spReturnIndex = 0;
    this.spVertices = new Array(32);
    this.spMatrixStack = new Array(10);
    this.spMatrixIndex = 0;
    this.spProjectionMatrix = new THREE.Matrix4();
    this.dpImageAddress = 0;
    this.dpTextureMemory = new DataView(new ArrayBuffer(4096));
    this.dpTileDescriptors = new Array(8);
    this.dpOtherModeL = 0;
    this.dpOtherModeH = 0;
    this.spLights = new Array(8);
    this.spNumLights = 0;

    this.spGeometryMode = {
        zbuffer: 0,
        shade: 0,
        cull_front: 0,
        cull_back: 0,
        fog: 0,
        lighting: 0,
        texture_gen: 0,
        texture_gen_linear: 0,
        shading_smooth: 0,
        clipping: 0
    };

    for(var i = 0; i < this.spLights.length; i++)
    {
        this.spLights[i] = new SPLight();
    }

    for(var i = 0; i < this.dpTileDescriptors.length; i++)
    {
        this.dpTileDescriptors[i] = new DPTileDescriptor();
    }

    for(var i = 0; i < this.spMatrixStack.length; i++)
    {
        this.spMatrixStack[i] = new THREE.Matrix4();
    }

    for(var i = 0; i < this.spVertices.length; i++)
    {
        this.spVertices[i] = new SPVertex();
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

        function hex(n)
        {
            var s = n.toString(16).toUpperCase();
            while(s.length < 8) s = "0" + s;
            return s;
        }

        this.log(hex(this.spCommandAddress-8) + ": " + hex(this.command.w0) + " " + hex(this.command.w1) + "<br>\n");

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

GfxState.prototype.importOpsFromChecksum = function(checksum)
{
    var dpOps = GfxOps.RDP;
    var spOps = null;
    var spOpsPatch = null;

    const checksums = [
        [ 0x3A1CBAC3, GfxOps.Fast3D, null], // sm64
        [ 0xEE47381B, GfxOps.F3DEX,  null], // micro machines 64 turbo
        [ 0x8805FFEA, GfxOps.F3DEX,  GfxOps.Patch_F3DEX_095 ], // mk64
        [ 0x5D3099F1, GfxOps.F3DEX2, null], // zelda
    ];

    for(var i = 0; i < checksums.length; i++)
    {
        var row = checksums[i];
        if(checksum == row[0])
        {
            spOps = row[1];
            spOpsPatch = row[2];
            break;
        }
    }

    this.importOps(dpOps, spOps, spOpsPatch);
}

GfxState.prototype.loadLight = function(segmentOffset, index)
{
    if(index < 0 || index > 7)
    {
        return;
    }

    var light = new SPLight();

    light.color = this.getU32(segmentOffset + 0x00);
    light.colorCopy = this.getU32(segmentOffset + 0x04);
    light.direction.x = this.getS8(segmentOffset + 0x08);
    light.direction.y = this.getS8(segmentOffset + 0x09);
    light.direction.z = this.getS8(segmentOffset + 0x0A);

    this.spLights[index] = light;

    console.log(light.color.toString(16), light.colorCopy.toString(16), light.direction);
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

        var alpha = this.getU8(offset + 0x0F) / 0xFF;

        var color = new THREE.Color(1, 1, 1);
        var normal = new THREE.Vector3(0, 0, 0);

        if(this.spGeometryMode.shade)
        {
            if(this.spGeometryMode.lighting)
            {
                var nx = this.getS8(offset + 0x0C) / 0x7F;
                var ny = this.getS8(offset + 0x0D) / 0x7F;
                var nz = this.getS8(offset + 0x0E) / 0x7F;
                normal = new THREE.Vector3(nx, ny, nz);
                color = new THREE.Color(this.spLights[0].color >>> 8);

                for(var j = 0; j < this.spNumLights; j++)
                {
                    var lx = this.spLights[1+j].direction.x / 0x7F;
                    var ly = this.spLights[1+j].direction.y / 0x7F;
                    var lz = this.spLights[1+j].direction.z / 0x7F;
                    var lNormal = new THREE.Vector3(lx, ly, lz);
                    var dp = normal.dot(lNormal);

                    if(dp >= 0)
                    {
                        // todo proper calculation
                        color = new THREE.Color(this.spLights[1].color >>> 8);
                    }
                }
            }
            else
            {
                var r = this.getU8(offset + 0x0C) / 0xFF;
                var g = this.getU8(offset + 0x0D) / 0xFF;
                var b = this.getU8(offset + 0x0E) / 0xFF;
                color = new THREE.Color(r, g, b);
            }
        }
        
        var position = new THREE.Vector3(x, y, z);
        var texcoords = new THREE.Vector2(u, v);
        position.applyMatrix4(this.spMatrixStack[this.spMatrixIndex]);

        this.spVertices[index + i] = new SPVertex(position, texcoords, color, normal, alpha);
    }
}

GfxState.prototype.loadMatrix = function(segmentOffset, bPush, bLoad, bProj)
{
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