var gP1AnalogX = 0;
var gP1AnalogY = 0;
var gP1Buttons = 0;

events.onpifread(function() {
    gP1Buttons = mem.u16[PIF_RAM_START + 4];
    gP1AnalogX = mem.s8[PIF_RAM_START + 6];
    gP1AnalogY = mem.s8[PIF_RAM_START + 7];
});

var buttonColors = {15: 0x4466FFFF, 14: 0x00FF00FF, 12: 0xFF0000FF,
    0: 0xFFFF00FF, 1: 0xFFFF00FF, 2: 0xFFFF00FF, 3: 0xFFFF00FF};

function getButtonColor(nButton)
{
    return (nButton in buttonColors) ? buttonColors[nButton] : 0xDDDDDDDD;
}

function getButtonColorEmpty(nButton)
{
    var color = getButtonColor(nButton);
    color = ((color & 0xFFFFFF00) >>> 0) + 0x20;
    return color;
}

events.ondraw(function(e) {
    var ctx = e.drawingContext;

    var radius = 0x7F / 4;

    var boxMiddleX = 10 + radius;
    var boxMiddleY = 100;

    var x = boxMiddleX + (gP1AnalogX / 4) - 4;
    var y = boxMiddleY - (gP1AnalogY / 4) - 4;

    var boxX = boxMiddleX - radius;
    var boxY = boxMiddleY - radius;
    var boxWidth = radius * 2;
    var boxHeight = radius * 2;

    ctx.fillColor = RGBA(0, 0, 0, 0.5);
    ctx.fillrect(boxX, boxY, boxWidth, boxHeight)

    ctx.fillColor = RGBA(255,0,0,0.9)
    ctx.fillrect(boxX, boxMiddleY, boxWidth, 1);
    ctx.fillrect(boxMiddleX, boxY, 1, boxHeight);

    ctx.fillColor = RGBA(255, 255, 255, 0.8);
    ctx.fillrect(x, y, 9, 9);
    //ctx.fillColor = RGBA(255, 255, 255, 0.9);
    //ctx.fillrect(x+2, y+2, 5, 5);

    //drawTextStuff(ctx)

    //console.log(ctx.width, ctx.height);



    ctx.fillColor = 0x0000007F;
    var buttonPaneX = boxX + boxWidth + 9
    var buttonPaneY = boxMiddleY - 11;
    ctx.fillrect(buttonPaneX, buttonPaneY, 8 * 10 + 1, 21);

    for(var i = 0; i < 16; i++) {
        var b = (gP1Buttons >> i) & 1;
        //var clrFilled

        ctx.fillColor = getButtonColorEmpty(i);
        ctx.fillrect(boxX + boxWidth + (i % 8) * 10 + 10, boxMiddleY - 10 + ((i/8)|0)*10, 9, 9);

        if(b) {
            if(i in buttonColors) ctx.fillColor = buttonColors[i];
            else ctx.fillColor = 0xFFFFFFFF;
            ctx.fillColor = getButtonColor(i);
            ctx.fillrect(boxX + boxWidth + (i % 8) * 10 + 10, boxMiddleY - 10 + ((i/8)|0)*10, 9, 9);
        }
    }
});

function drawTextStuff(ctx)
{
    //console.log(ctx.strokeColor)

    ctx.strokeWidth = 3;
    ctx.strokeColor = 0x000000FF;
    ctx.fontFamily = "courier new";
    ctx.fontWeight = "bold";
    ctx.fontSize = 18;

    ctx.fontWeight = "bold";

    // ctx.setfont({ family: 'segoe ui', size: 16, weight: 'bold' });
    // ctx.setfont('segoe ui', 16, 'bold');

    //ctx.fillColor = 0x0000007F;
    //ctx.fillrect(10, 140, 150, 72);

    var y = 145;

    function nextY()
    {
        var t = y;
        y += 14;
        return t;
    }

    ctx.fillColor = 0xFFFFFFFF;
    ctx.drawtext(15, nextY(), "X:   " + mem.f32[0x8033B1AC].toFixed(2));
    ctx.drawtext(15, nextY(), "Y:   " + mem.f32[0x8033B1B0].toFixed(2));
    ctx.drawtext(15, nextY(), "Z:   " + mem.f32[0x8033B1B4].toFixed(2));
    ctx.drawtext(15, nextY(), "SPD: " + mem.f32[0x8033B1C4].toFixed(2));

    
}

