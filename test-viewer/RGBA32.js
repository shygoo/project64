function RGBA32(r, g, b, a)
{
    return (r << 24 | g << 16 | b << 8 | a) >>> 0;
}

RGBA32.fromRGBA16 = function(texel)
{
    const factor = 0xFF / 0x1F;
    var r = (((texel >> 11) & 0x1F) * factor) | 0;
    var g = (((texel >>  6) & 0x1F) * factor) | 0;
    var b = (((texel >>  1) & 0x1F) * factor) | 0;
    var a = (texel & 1) * 255;
    return RGBA32(r, g, b, a);
}

RGBA32.fromIA16 = function(texel)
{
    var i = (texel >> 8);
    var a = (texel & 0xFF);
    return RGBA32(i, i, i, a);
}

RGBA32.fromI8 = function(texel)
{
    return RGBA32(texel, texel, texel, texel);
}

RGBA32.fromIA8 = function(texel)
{
    var i = (texel >> 4) * 17;
    var a = (texel & 0x0F) * 17;
    return RGBA32(i, i, i, a);
}

RGBA32.fromI4 = function(texel)
{
    var i = texel * 17;
    return RGBA32(i, i, i, i);
}

RGBA32.fromIA4 = function(texel)
{
    const factor = 255 / 7;
    var i = ((texel >> 1) * factor) | 0;
    var a = (texel & 1) * 255;
    return RGBA32(i, i, i, a);
}