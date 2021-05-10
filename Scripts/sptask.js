const MODEL_SCALE = 100;

const SP_TASK_STRUCT_ADDR = SP_DMEM_START + 0xFC0;

const OSTask = mem.typedef({
    type:             u32,
    flags:            u32,
    ucode_boot_addr:  u32,
    ucode_boot_size:  u32,
    ucode_addr:       u32,
    ucode_size:       u32,
    ucode_data_addr:  u32,
    ucode_data_size:  u32,
    dram_stack_addr:  u32,
    dram_stack_size:  u32,
    output_buff_addr: u32,
    output_buff_size: u32,
    data_addr:        u32,
    data_size:        u32,
    yield_data_addr:  u32,
    yield_data_size:  u32
});

var dlResult0 = null;
var dlResult1 = null;
var dumped = false;
var mouseX = 0;
var mouseY = 0;
var gShowWireframe = false;

events.onmousemove(function(e) {
    mouseX = e.x;
    mouseY = e.y;
});

events.onmouseup(function(e) {
    if(e.button == 2) {
        gShowWireframe = !gShowWireframe;
    }
})

events.onsptask(function(e) {
    var task = new OSTask(SP_TASK_STRUCT_ADDR);
    if (task.type == 1) {
        onGfxTask(task);
    }
    else if (task.type == 2) {
        onAudioTask(task);
    }
});

function drawWireframe(ctx)
{
    ctx.fillColor = 0x000000D0;
    ctx.fillrect(0, 0, 640, 480);

    ctx.strokeColor = 0xFFFFFF50;
    ctx.strokeWidth = 1;

    ctx.beginpath();
    for(var i = 0; i < dlResult1.faceHeap.length; i++)
    {
        var face = dlResult1.faceHeap[i];
        var v0 = dlResult1.vertexHeap[face[0]];
        var v1 = dlResult1.vertexHeap[face[1]];
        var v2 = dlResult1.vertexHeap[face[2]];
    
        var x0 = v0[0];
        var y0 = v0[1];
        var x1 = v1[0];
        var y1 = v1[1];
        var x2 = v2[0];
        var y2 = v2[1];
    
        ctx.moveto(x0, y0);
        ctx.lineto(x1, y1);
        ctx.lineto(x2, y2);
        ctx.lineto(x0, y0);
    }
    ctx.stroke();
}

function pointInTriangle(p, a, b, c)
{
    var ax = a[0];
    var ay = a[1];
    var x0 = b[0] - ax
    var y0 = b[1] - ay;
    var x1 = c[0] - ax
    var y1 = c[1] - ay;
    var x2 = p[0] - ax;
    var y2 = p[1] - ay;
    var den = x0 * y1 - x1 * y0;
    var v = (x2 * y1 - x1 * y2) / den;
    var w = (x0 * y2 - x2 * y0) / den;
    var u = 1.0 - v - w;
    return (u >= 0) && (v >= 0) && (u + v < 1);
}

function drawHotVertex(ctx)
{
    var nearestVertDist = 0;
    var hotVertIdx = -1;

    for(var i = 0; i < dlResult1.vertexHeap.length; i++) {
        var vertex = dlResult1.vertexHeap[i];
        if(!vertex)
            break;

        var dx = mouseX - vertex[0];
        var dy = mouseY - vertex[1];

        if(dx > 10 || dx < -10 || dy > 10 || dy < -10)
            continue;

        var d = Math.sqrt(dx*dx + dy*dy);

        if(hotVertIdx == -1 || d < nearestVertDist) {
            nearestVertDist = d;
            hotVertIdx = i;
        }
    }

    if(hotVertIdx >= 0)
    {
        var hotVertex = dlResult1.vertexHeap[hotVertIdx];
        ctx.fillColor = COLOR_RED;
        ctx.strokeColor = COLOR_BLACK;
        ctx.strokeWidth = 3;
        ctx.fillrect(hotVertex[0] - 2, hotVertex[1] - 2, 5, 5);
        ctx.print(hotVertex[0] + 5, hotVertex[1] - 10, hotVertex[3].hex());
    }
}

function drawHotFace(ctx)
{
    ctx.fillColor = COLOR_GREEN;
    var hotFaceIdx = -1;
    var hotFaceXYZ = [0,0,0];
    for(var i = dlResult1.faceHeap.length - 1; i >= 0; i--)
    {
        var face = dlResult1.faceHeap[i];
        var v0 = dlResult1.vertexHeap[face[0]];
        var v1 = dlResult1.vertexHeap[face[1]];
        var v2 = dlResult1.vertexHeap[face[2]];
        var x = (v0[0] + v1[0] + v2[0]) / 3;
        var y = (v0[1] + v1[1] + v2[1]) / 3;
        var z = (v0[2] + v1[2] + v2[2]) / 3;

        if(pointInTriangle([mouseX, mouseY], v0, v1, v2))
        {
            if(hotFaceIdx == -1 && hotFaceXYZ[2] < z)
            {
                hotFaceIdx = i;
                hotFaceXYZ = [x, y, z];
            }
        }
    }

    if(hotFaceIdx >= 0)
    {
        var face = dlResult1.faceHeap[hotFaceIdx];
        
        var v0 = dlResult1.vertexHeap[face[0]];
        var v1 = dlResult1.vertexHeap[face[1]];
        var v2 = dlResult1.vertexHeap[face[2]];
    
        var x0 = v0[0];
        var y0 = v0[1];
        var x1 = v1[0];
        var y1 = v1[1];
        var x2 = v2[0];
        var y2 = v2[1];

        ctx.fillColor = COLOR_GREEN;
        ctx.strokeColor = COLOR_GREEN;
        ctx.strokeWidth = 3;
        ctx.beginpath();
        ctx.moveto(x0, y0);
        ctx.lineto(x1, y1);
        ctx.lineto(x2, y2);
        ctx.lineto(x0, y0);
        ctx.stroke();

        ctx.fillrect(hotFaceXYZ[0] - 2, hotFaceXYZ[1] - 2, 5, 5);

        ctx.fillColor = COLOR_GREEN;
        ctx.strokeColor = COLOR_BLACK;
        ctx.print(hotFaceXYZ[0] + 5, hotFaceXYZ[1] - 10, face.debugAddr.hex());
    }
}

function Checkbox(x, y, label, cb)
{
    this.x = x;
    this.y = y;
    this.label = label;
    this.checked = false;
    var _this = this;

    this.cbId = events.onmouseup(function(e) {
        if(e.x >= _this.x && e.x < _this.x + 10 &&
           e.y >= _this.y && e.y < _this.y + 10)
        {
            gShowWireframe = !gShowWireframe;
            _this.checked = !_this.checked;
        }
    })
}

Checkbox.prototype.draw = function(ctx)
{
    ctx.fillColor = this.checked ? COLOR_BLACK : 0x00000080;
    ctx.fillrect(this.x, this.y, 10, 10);
    ctx.fillColor = this.checked ? COLOR_GREEN : 0x80808080;
    ctx.fillrect(this.x+2, this.y+2, 6, 6);
    ctx.strokeColor = this.checked ? COLOR_BLACK : 0x00000080
    ctx.fillColor = this.checked ? COLOR_WHITE : 0xFFFFFF80;
    ctx.print(this.x + 15, this.y - 2, this.label);
}

var wireframeCheckbox = new Checkbox(20, 100, "Wireframe");
var triInfoCheckbox = new Checkbox(20, 116, "Tri addr");
var vtxInfoCheckbox = new Checkbox(20, 132, "Vtx addr");

events.ondraw(function(e) {
    var ctx = e.drawingContext;
    ctx.fontSize = 14;
    ctx.fontFamily = "Consolas";
    ctx.fontWeight = "bold";
    var y = 100;

    function ny(){
        var t = y;
        y += 16;
        return t;
    }

    if(dlResult1 == null)
    {
        return;
    }

    var drawTimeStart = performance.now();

    if(wireframeCheckbox.checked) {
        drawWireframe(ctx);
    }
    
    if(vtxInfoCheckbox.checked) {
        drawHotVertex(ctx);
    }

    if(triInfoCheckbox.checked) {
        drawHotFace(ctx);
    }

    var drawTime = performance.now() - drawTimeStart;
    ctx.strokeWidth = 3;
    ctx.fillColor = COLOR_WHITE;
    ctx.strokeColor = COLOR_BLACK;
    ctx.print(20, 60, "drawtime " + drawTime.toFixed(2) + "ms");
    ctx.print(20, 76, "dlparser " + dlResult0.parseTime.toFixed(2) + "ms");

    wireframeCheckbox.draw(ctx);
    triInfoCheckbox.draw(ctx);
    vtxInfoCheckbox.draw(ctx);
    
});

function onGfxTask(task) {

    dlResult1 = dlResult0;

    var dlistAddr = task.data_addr /*+ K0BASE*/;
    var ucodeAddr = task.ucode_addr + K0BASE;

    var dlp = new DLParser();
    dlp.ram = mem.getblock(K0BASE, mem.ramSize);
    dlp.parseF3D(dlistAddr);
    dlResult0 = dlp.result;

    if(!dumped)
    {
        dumpBasicOBJ("gfx_dump.obj", dlp.result);
        dumped = true;
    }
}

function dumpBasicOBJ(path, dlResult)
{
    var fd = fs.open(path, "wb");

    dlResult.vertexHeap.forEach(function(vertex) {
        fs.write(fd, "v " + vertex[0] + " " + vertex[1] + " " + vertex[2] + "\n");
    });

    dlResult.faceHeap.forEach(function(face) {
        fs.write(fd, "f " + (face[0]+1) + " " + (face[1]+1) + " " + (face[2]+1) + "\n");
    });

    fs.close(fd);

    console.log("wrote", path);
}

function onAudioTask(task) {
    // could implement alist parser here
}

/////////////////////////////

function DLParser()
{
    this.ram = null;
    this.result = {};
}



function loadMatrix(ram, offset, dst)
{
    for(var i = 0; i < 16; i++) {
        dst[i] = ((ram.readInt16BE(offset + i * 2) << 16) | ram.readUInt16BE(offset + 32 + i * 2)) / 0x10000;
    }
}

DLParser.prototype.parseF3D = function(dlistAddr) {
    var parseTimeStart = performance.now();

    var dlAddress = dlistAddr;
    var ram = this.ram;

    var w0, w1;
    
    var spVertices = new Array(16);
    var spJumpStack = new Uint32Array(16);
    var spSegments = new Uint32Array(16);
    var spJumpStackIndex = 0;
    var spMatrices = new Array(16);
    var spMatrixIndex = 0;
    var spProjectionMatrix = mat4.create();
    var spFinalMatrix = mat4.create();

    var screenMtx = mat4.create();
    screenMtx[0] = 320;
    screenMtx[5] = -240;
    screenMtx[12] = 320;
    screenMtx[13] = 240;

    var scratchMtx = mat4.create();

    for(var i = 0; i < 16; i++) {
        spSegments[i] = 0;
    }

    for(var i = 0; i < 64; i++) {
        // (only loading xyz)
        spVertices[i] = vec3.create();
    }

    for(var i = 0; i < spMatrices.length; i++) {
        spMatrices[i] = mat4.create();
    }

    // G_VTX
    var numv, vidx, vaddr, vend, nv, curv,
        ix, iy, iz, ox, oy, oz, ow;

    // G_TRI
    var v0, v1, v2;

    // G_MTX
    var mtxflags;

    // (this reduces call time)
    const getu32 = ram.readUInt32BE.bind(ram);
    const setu32 = ram.writeUInt32BE.bind(ram);
    const gets16 = ram.readInt16BE.bind(ram);
    const vec3_transformMat4 = vec3.transformMat4;
    const mat4_mul = mat4.mul;

    // result
    var numCommands = 0;
    var numTriangles = 0;
    var numVertices = 0;
    var vertexHeap = new Array(5000);
    var vhidx = 0;

    var faceHeap = [];
    var faceIdx = 0;

    for(var done = false; !done;) {
        w0 = getu32(dlAddress);
        w1 = getu32(dlAddress + 4);
        dlAddress += 8;
        
        switch(w0 >>> 24)
        {
            case 0x04: {
                numv = ((w0 >> 20) & 0x0F) + 1;
                vidx = (w0 >> 16) & 0x0F;
                vaddr = spSegments[(w1 >>> 24) & 0x0F] + (w1 & 0x00FFFFFF);
                vend = vaddr + numv * 16;
                nv = 0;

                while(vaddr < vend) {
                    curv = spVertices[vidx + nv];
                    curv[0] = gets16(vaddr);
                    curv[1] = gets16(vaddr + 2);
                    curv[2] = gets16(vaddr+ 4);
                    vec3_transformMat4(curv, curv, spFinalMatrix);
                    curv.debugAddr = K0BASE + vaddr;
                    vaddr += 16;
                    nv++;
                }
                //numVertices += numv;
            } break;
            case 0xBF: {
                v0 = spVertices[((w1 >>> 16) & 0xFF) / 0x0A];
                v1 = spVertices[((w1 >>>  8) & 0xFF) / 0x0A];
                v2 = spVertices[((w1 >>>  0) & 0xFF) / 0x0A];

                var x0 = v0[0], y0 = v0[1], z0 = v0[2];
                var x1 = v1[0], y1 = v1[1], z1 = v1[2];
                var x2 = v2[0], y2 = v2[1], z2 = v2[2];

                // todo test w
                if(z0 > 1 || z1 > 1 || z2 > 1)
                    break;

                if(x0 < 0 && x1 < 0 && x2 < 0) continue;
                if(x0 >= 640 && x1 >= 640 && x2 >= 640) continue;
                if(y0 < 0 && y1 < 0 && y2 < 0) continue;
                if(y0 >= 480 && y1 >= 480 && y2 >= 480) continue;

                vertexHeap[vhidx] = [ x0, y0, z0, v0.debugAddr ];
                vertexHeap[vhidx+1] = [ x1, y1, z1, v1.debugAddr ];
                vertexHeap[vhidx+2] = [ x2, y2, z2, v2.debugAddr ];
                faceHeap[faceIdx] = [vhidx, vhidx + 1, vhidx + 2];
                faceHeap[faceIdx].debugAddr = K0BASE + dlAddress - 8;
                vhidx += 3;
                faceIdx++;
                //numTriangles++;
            } break;
            case 0x01: {
                var maddr = spSegments[(w1 >>> 24) & 0x0F] + (w1 & 0x00FFFFFF);
                mtxflags = (w0 >>> 16) & 0xFF;
                
                if(mtxflags & 0x01) { // G_MTX_PROJECTION
                    if(mtxflags & 0x02) { // G_MTX_LOAD
                        loadMatrix(ram, maddr, spProjectionMatrix);
                    }
                    else {
                        loadMatrix(ram, maddr, scratchMtx);
                        mat4_mul(spProjectionMatrix, spProjectionMatrix, scratchMtx);
                    }
                }
                else { // G_MTX_MODELVIEW
                    var curMtx = spMatrices[spMatrixIndex];

                    if(mtxflags & 0x04) { // G_MTX_PUSH
                        spMatrixIndex++;
                    }

                    if(mtxflags & 0x02) { // G_MTX_LOAD
                        loadMatrix(ram, maddr, spMatrices[spMatrixIndex]);
                    }
                    else { // G_MTX_MUL
                        loadMatrix(ram, maddr, scratchMtx);
                        mat4_mul(spMatrices[spMatrixIndex], curMtx, scratchMtx);
                    }

                    mat4_mul(spFinalMatrix, spMatrices[spMatrixIndex], spProjectionMatrix);
                    mat4_mul(spFinalMatrix, spFinalMatrix, screenMtx);
                }
            } break;
            case 0xBD: {
                spMatrixIndex--;
            } break;
            case 0x06: {
                if((w0 & 0x00FF0000) >>> 16 != 0x01) {
                    spJumpStack[spJumpStackIndex++] = dlAddress;
                }
                dlAddress = spSegments[(w1 >>> 24) & 0x0F] + (w1 & 0x00FFFFFF);
            } break;
            case 0xB8: {
                if(spJumpStackIndex > 0) {
                    dlAddress = spJumpStack[--spJumpStackIndex];
                }
                else {
                    done = true;
                }
            } break;
            case 0xBC: {
                if((w0 & 0xFF) == 0x06) {
                    spSegments[((w0 & 0xFF00) >>> 8) / 4] = w1 & 0x1FFFFFFF;
                }
            } break;
        }
        //numCommands++;
    }

    this.result.parseTime = performance.now() - parseTimeStart;
    this.result.numCommands = numCommands;
    this.result.numTriangles = numTriangles;
    this.result.numVertices = numVertices;
    this.result.faceHeap = faceHeap;
    this.result.vertexHeap = vertexHeap;
}

var mouseQueue = { x: 0, y: 0, queued: false };

events.onmouseup(function(e) {
    mouseQueue.x = e.x;
    mouseQueue.y = e.y;
    mouseQueue.queued = true;
});