#include <stdafx.h>
#include "../ScriptAPI.h"

void ScriptAPI::Define_N64Image(duk_context* ctx)
{
    const duk_function_list_entry methods[] = {
        { "toPNG", N64Image_toPNG, DUK_VARARGS },
        { nullptr, 0 }
    };
    
    const duk_function_list_entry staticFuncs[] = {
        { "fromPNG", N64Image_fromPNG, DUK_VARARGS },
        { nullptr, 0 }
    };

    duk_push_global_object(ctx);
    duk_push_c_function(ctx, N64Image__constructor, DUK_VARARGS);
    duk_put_function_list(ctx, -1, staticFuncs);
    duk_push_object(ctx);
    duk_put_function_list(ctx, -1, methods);
    duk_freeze(ctx, -1);
    duk_put_prop_string(ctx, -2, "prototype");
    duk_freeze(ctx, -1);
    duk_def_prop(ctx, -2, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_ENUMERABLE);
    duk_pop(ctx);
}

static int BitsPerPixel(int format)
{
    using namespace ScriptAPI;

    switch (format)
    {
    case IMG_I4:
    case IMG_IA4:
    case IMG_CI4_RGBA16:
    case IMG_CI4_IA16:
        return 4;
    case IMG_I8:
    case IMG_IA8:
    case IMG_CI8_RGBA16:
    case IMG_CI8_IA16:
        return 8;
    case IMG_IA16:
    case IMG_RGBA16:
        return 16;
    case IMG_RGBA32:
        return 32;
    }

    return 0;
}

static int PaletteColorCount(int format)
{
    using namespace ScriptAPI;

    switch (format)
    {
    case IMG_CI4_RGBA16:
    case IMG_CI4_IA16:
        return 16;
    case IMG_CI8_RGBA16:
    case IMG_CI8_IA16:
        return 256;
    }

    return 0;
}

static bool NeedsPalette(int format)
{
    using namespace ScriptAPI;

    switch (format)
    {
    case IMG_CI4_RGBA16:
    case IMG_CI4_IA16:
    case IMG_CI8_RGBA16:
    case IMG_CI8_IA16:
        return true;
    }

    return false;
}

static unsigned int GetTexel(int format, void* data, size_t nPixel)
{
    int bpp = BitsPerPixel(format);
    size_t byteOffset = (bpp * nPixel) / 8;
    void* pTexel = (uint8_t*)data + byteOffset;

    switch (bpp)
    {
    case 4:
        if (nPixel % 2)
        {
            return (*(uint8_t*)pTexel >> 4) & 0x0F;
        }
        else
        {
            return *(uint8_t*)pTexel & 0x0F;
        }
    case 8:
        return *(uint8_t*)pTexel;
    case 16:
        return _byteswap_ushort(*(uint16_t*)pTexel);
    case 32:
        return _byteswap_ushort(*(uint32_t*)pTexel);
    }

    return 0;
}

duk_ret_t ScriptAPI::N64Image__constructor(duk_context* ctx)
{
    /*
    new N64Image(format, width, height, data[, palette])
    new N64Image(format: number, width: number, height: number, data: Buffer, palette?: Buffer)
    */
    
    if (!duk_is_number(ctx, 0) ||
        !duk_is_number(ctx, 1) ||
        !duk_is_number(ctx, 2) ||
        !duk_is_buffer_data(ctx, 3))
    {
        return ThrowInvalidArgsError(ctx);
    }

    duk_uint_t format = duk_get_int(ctx, 0);
    duk_uint_t width = duk_get_uint(ctx, 1);
    duk_uint_t height = duk_get_uint(ctx, 2);

    size_t numPixels = width * height;

    size_t dataByteLength = 0, paletteByteLength = 0;
    uint8_t* data = (uint8_t*)duk_get_buffer_data(ctx, 3, &dataByteLength);
    uint8_t* paletteData = nullptr;

    int bpp = BitsPerPixel(format);

    if (bpp == 0)
    {
        return DUK_RET_TYPE_ERROR; // todo invalid format error
    }

    size_t requiredDataByteLength = (numPixels * bpp) / 8;

    if (dataByteLength != requiredDataByteLength)
    {
        return DUK_RET_RANGE_ERROR; // todo invalid data size, expected x bytes
    }

    if (NeedsPalette(format))
    {
        if (!duk_is_buffer_data(ctx, 4))
        {
            return DUK_RET_ERROR; // todo palette argument is required for xx format
        }

        paletteData = (uint8_t*)duk_get_buffer_data(ctx, 4, &paletteByteLength);
        size_t numColors = paletteByteLength / sizeof(uint16_t);

        for (size_t i = 0; i < numPixels; i++)
        {
            if (GetTexel(format, data, i) >= numColors)
            {
                return DUK_RET_RANGE_ERROR; // image data contains out-of-bounds color index
            }
        }
    }

    duk_push_this(ctx);
    duk_push_uint(ctx, format);
    duk_put_prop_string(ctx, -2, "format");
    duk_push_uint(ctx, width);
    duk_put_prop_string(ctx, -2, "width");
    duk_push_uint(ctx, height);
    duk_put_prop_string(ctx, -2, "height");
    
    void* dataCopy = duk_push_buffer(ctx, dataByteLength, false);
    memcpy(dataCopy, data, dataByteLength);
    duk_put_prop_string(ctx, -2, "data");

    // todo always store hidden rgba32
    //duk_put_prop_string(ctx, -2, DUK_HIDDEN_SYMBOL("nativeData"));

    if (paletteData)
    {
        int maxColors = PaletteColorCount(format);
        size_t paletteCopySize = min(paletteByteLength, maxColors);
        void* paletteCopy = duk_push_buffer(ctx, paletteCopySize, false);
        memcpy(paletteCopy, paletteData, paletteCopySize);
    }
    else
    {
        duk_push_null(ctx);
    }

    duk_put_prop_string(ctx, -2, "paletteData");
    return 0;
}

duk_ret_t ScriptAPI::N64Image_toPNG(duk_context* ctx)
{
    return 1;
}

duk_ret_t ScriptAPI::N64Image_fromPNG(duk_context* ctx)
{
    return 1;
}
