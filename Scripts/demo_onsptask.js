const ui = require('ui.js');
const dlutil = require('dlutil.js')
const math3d = require('math3d.dll');
const vec2 = math3d.vec2;
const vec3 = math3d.vec3;
const vec4 = math3d.vec4;
const mat4 = math3d.mat4;

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

var screenWidth = 320;
var screenHeight = 240;
var dlResult0 = null;
var dlResult1 = null;
var dumped = false;
var gMouseX = 0;
var gMouseY = 0;

events.onsptask(function(e) {
    var task = new OSTask(SP_TASK_STRUCT_ADDR);
    if (task.type == 1) {
        onGfxTask(task);
    }
    else if (task.type == 2) {
        onAudioTask(task);
    }
});

function onGfxTask(task) {

    dlResult1 = dlResult0;

    var dlistAddr = task.data_addr /*+ K0BASE*/;
    var ucodeAddr = task.ucode_addr + K0BASE;

    var dlp = new dlutil.DLParser();
    dlp.ram = mem.getblock(K0BASE, mem.ramSize);
    dlp.parseF3D(dlistAddr, screenWidth, screenHeight);
    dlResult0 = dlp.result;

    if(!dumped)
    {
        dumpBasicOBJ("gfx_dump.obj", dlp.result);
        dumped = true;
    }
}

function onAudioTask(task) {
    // could implement alist parser here
}

function drawWireframe(ctx)
{
    ctx.fillColor = 0x000000D0;
    ctx.fillrect(0, 0, ctx.width, ctx.height);

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

function drawHotVertex(ctx)
{
    var nearestVertDist = 0;
    var hotVertIdx = -1;

    for(var i = 0; i < dlResult1.vertexHeap.length; i++) {
        var vertex = dlResult1.vertexHeap[i];
        if(!vertex)
            break;

        var dx = gMouseX - vertex[0];
        var dy = gMouseY - vertex[1];

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
        ctx.drawtext(hotVertex[0] + 5, hotVertex[1] - 10, hotVertex[3].hex());
    }
}

var gHotFaceAddr = 0;

function drawHotFace(ctx)
{
    var highlightFaceIdx = -1;
    var bestPointDepth = 0;

    var mousePoint2 = vec2.fromValues(gMouseX, gMouseY);

    for(var i = dlResult1.faceHeap.length - 1; i >= 0; i--)
    {
        var face = dlResult1.faceHeap[i];
        var v0 = dlResult1.vertexHeap[face[0]];
        var v1 = dlResult1.vertexHeap[face[1]];
        var v2 = dlResult1.vertexHeap[face[2]];

        var bcCoords3 = vec3.create();
        math3d.barycentric(bcCoords3, mousePoint2, v0, v1, v2);

        //var weights = triBarycentricCoords([gMouseX, gMouseY], v0, v1, v2);
        if(math3d.bcInside(bcCoords3))
        {
            var w3 = vec3.fromValues(v0[3], v1[3], v2[3]);
            var pointDepth = vec3.dot(w3, bcCoords3);
            //var pointDepth = interpolate(v0[3], v1[3], v2[3], weights);
            if(highlightFaceIdx == -1 || (pointDepth > 1 && pointDepth < bestPointDepth))
            {
                highlightFaceIdx = i;
                bestPointDepth = pointDepth;
            }
        }
    }

    ctx.fillColor = COLOR_GREEN;

    if(highlightFaceIdx >= 0)
    {
        var face = dlResult1.faceHeap[highlightFaceIdx];
        
        var v0 = dlResult1.vertexHeap[face[0]];
        var v1 = dlResult1.vertexHeap[face[1]];
        var v2 = dlResult1.vertexHeap[face[2]];
    
        var x0 = v0[0];
        var y0 = v0[1];
        var x1 = v1[0];
        var y1 = v1[1];
        var x2 = v2[0];
        var y2 = v2[1];

        ctx.fillColor = RGBA(COLOR_GREEN, 0.20);
        ctx.strokeColor = COLOR_GREEN;
        ctx.strokeWidth = 1;
        ctx.beginpath();
        ctx.moveto(x0, y0);
        ctx.lineto(x1, y1);
        ctx.lineto(x2, y2);
        ctx.lineto(x0, y0);
        ctx.stroke();
        ctx.fill();

        ctx.fillColor = COLOR_GREEN;
        ctx.strokeColor = COLOR_BLACK;
        ctx.strokeWidth = 3;

        //ctx.fillrect(hotFaceXYZ[0] - 1, hotFaceXYZ[1] - 1, 3, 3);
        //ctx.drawtext(hotFaceXYZ[0] + 5, hotFaceXYZ[1] - 10, face.debugAddr.hex());

        gHotFaceAddr = face.debugAddr;
    }
}

var wireframeCheckbox = new ui.CheckboxCtrl(20, 100, "Wireframe");
var triInfoCheckbox = new ui.CheckboxCtrl(20, 116, "Tri addr");
var vtxInfoCheckbox = new ui.CheckboxCtrl(20, 132, "Vtx addr");

var controls = new ui.ScreenCtrlPane();
controls.add(wireframeCheckbox);
controls.add(triInfoCheckbox);
controls.add(vtxInfoCheckbox);

events.onmouseup(function(e) {
    if(controls.click(e.x, e.y)) {
        return;
    }
    if(triInfoCheckbox.checked) {
        debug.showmemory(gHotFaceAddr);
    }
});

events.onmousemove(function(e) {
    gMouseX = e.x;
    gMouseY = e.y;
});

events.ondraw(function(e) {
    var ctx = e.drawingContext;
    screenWidth = ctx.width;
    screenHeight = ctx.height;
    ctx.fontSize = 14;
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

    controls.draw(ctx);

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
    ctx.drawtext(20, 60, "drawtime " + drawTime.toFixed(2) + "ms");
    ctx.drawtext(20, 76, "dlparser " + dlResult0.parseTime.toFixed(2) + "ms");

    wireframeCheckbox.draw(ctx);
    triInfoCheckbox.draw(ctx);
    vtxInfoCheckbox.draw(ctx);
});

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



/////////////////////////////

