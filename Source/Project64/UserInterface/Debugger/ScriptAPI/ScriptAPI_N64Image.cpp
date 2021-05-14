#include <stdafx.h>
#include <3rdParty/png/png.h>
#include "../ScriptAPI.h"

static int BitsPerPixel(int format);
static int PaletteColorCount(int format);
static bool UsesPalette(int format);
static unsigned int GetTexel(int format, void* data, size_t nPixel);
static uint32_t ColorToRGBA32(int srcFormat, unsigned int color);
static uint32_t GetColor(int format, size_t nPixel, void* data, void* palette = nullptr);

static void MakePNG(uint8_t* rgba32, size_t width, size_t height, std::vector<uint8_t>& buffer);
static void PngWriteData(png_structp png_ptr, png_bytep data, png_size_t length);
static void PngOutputFlushNoop(png_structp png_ptr);

void ScriptAPI::Define_N64Image(duk_context* ctx)
{
    const duk_function_list_entry methods[] = {
        { "toPNG", js_N64Image_toPNG, DUK_VARARGS },
        { nullptr, 0 }
    };
    
    const duk_function_list_entry staticFuncs[] = {
        { "fromPNG", js_N64Image_fromPNG, DUK_VARARGS },
        { nullptr, 0 }
    };

    duk_push_global_object(ctx);
    duk_push_string(ctx, "N64Image");
    duk_push_c_function(ctx, js_N64Image__constructor, DUK_VARARGS);
    duk_put_function_list(ctx, -1, staticFuncs);
    duk_push_object(ctx);
    duk_put_function_list(ctx, -1, methods);
    duk_freeze(ctx, -1);
    duk_put_prop_string(ctx, -2, "prototype");
    duk_freeze(ctx, -1);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_ENUMERABLE);
    duk_pop(ctx);
}

duk_ret_t ScriptAPI::js_N64Image__constructor(duk_context* ctx)
{
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
        duk_push_error_object(ctx, DUK_RET_TYPE_ERROR, "invalid format");
        return duk_throw(ctx);
    }

    size_t requiredDataByteLength = (numPixels * bpp) / 8;

    if (dataByteLength != requiredDataByteLength)
    {
        duk_push_error_object(ctx, DUK_RET_TYPE_ERROR, "invalid data size (%d); expected %d",
            dataByteLength, requiredDataByteLength);
        return duk_throw(ctx);
    }

    if (UsesPalette(format))
    {
        if (!duk_is_buffer_data(ctx, 4))
        {
            duk_push_error_object(ctx, DUK_RET_TYPE_ERROR, "palette argument is missing");
            return duk_throw(ctx);
        }

        paletteData = (uint8_t*)duk_get_buffer_data(ctx, 4, &paletteByteLength);
        size_t numColors = paletteByteLength / sizeof(uint16_t);

        for (size_t i = 0; i < numPixels; i++)
        {
            if (GetTexel(format, data, i) >= numColors)
            {
                duk_push_error_object(ctx, DUK_RET_RANGE_ERROR, "color index out of bounds");
                return duk_throw(ctx);
            }
        }
    }

    // TODO detect non-grayscale texel with intensity format and throw error 

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

    if (format == IMG_RGBA32)
    {
        duk_push_null(ctx);
    }
    else
    {
        size_t rgba32ByteLength = numPixels * 4;
        uint8_t* rgba32 = (uint8_t*)duk_push_buffer(ctx, rgba32ByteLength, false);

        for (size_t nPixel = 0; nPixel < numPixels; nPixel++)
        {
            uint32_t color = GetColor(format, nPixel, data, paletteData);
            uint32_t* pPixelOut = (uint32_t*)(rgba32 + nPixel * 4);
            *pPixelOut = _byteswap_ulong(color);
        }
    }

    duk_put_prop_string(ctx, -2, DUK_HIDDEN_SYMBOL("rgba32"));

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

    duk_put_prop_string(ctx, -2, "palette");
    return 0;
}

duk_ret_t ScriptAPI::js_N64Image_toPNG(duk_context* ctx)
{
    duk_push_this(ctx);

    duk_get_prop_string(ctx, -1, "format");
    int format = duk_get_int(ctx, -1);
    duk_pop(ctx);

    duk_get_prop_string(ctx, -1, "width");
    duk_uint_t width = duk_get_uint(ctx, -1);
    duk_pop(ctx);

    duk_get_prop_string(ctx, -1, "height");
    duk_uint_t height = duk_get_uint(ctx, -1);
    duk_pop(ctx);

    uint8_t *rgba32;

    CScriptInstance* inst = GetInstance(ctx);

    duk_size_t dummy;
    if (format == IMG_RGBA32)
    {
        duk_get_prop_string(ctx, -1, "data");
    }
    else if (duk_has_prop_string(ctx, -1, DUK_HIDDEN_SYMBOL("rgba32")))
    {
        duk_get_prop_string(ctx, -1, DUK_HIDDEN_SYMBOL("rgba32"));
    }
    else
    {
        return DUK_RET_ERROR;
    }

    rgba32 = (uint8_t*)duk_get_buffer_data(ctx, -1, &dummy);
    
    if (rgba32 == nullptr)
    {
        return DUK_RET_ERROR;
    }

    std::vector<uint8_t> pngData;
    MakePNG(rgba32, width, height, pngData);

    void* jsPngData = duk_push_buffer(ctx, width * height * 4, false);
    memcpy(jsPngData, &pngData[0], pngData.size());

    return 1;
}

duk_ret_t ScriptAPI::js_N64Image_fromPNG(duk_context* ctx)
{
    return 1;
}

struct ImgFormatInfo {
    int bitsPerPixel;
    int paletteColorCount;
};

static const std::map<int, ImgFormatInfo> m_FormatInfo = {
    { ScriptAPI::IMG_I4,         { 4,  0 } },
    { ScriptAPI::IMG_IA4,        { 4,  0 } },
    { ScriptAPI::IMG_I8,         { 8,  0 } },
    { ScriptAPI::IMG_IA8,        { 8,  0 } },
    { ScriptAPI::IMG_RGBA16,     { 16, 0 } },
    { ScriptAPI::IMG_RGBA32,     { 32, 0 } },
    { ScriptAPI::IMG_CI4_RGBA16, { 4,  16 } },
    { ScriptAPI::IMG_CI4_IA16,   { 4,  16 } },
    { ScriptAPI::IMG_CI8_RGBA16, { 8,  256 } },
    { ScriptAPI::IMG_CI8_IA16,   { 8,  256 } },
};

static int BitsPerPixel(int format)
{
    if (m_FormatInfo.count(format))
    {
        return m_FormatInfo.at(format).bitsPerPixel;
    }
    return 0;
}

static int PaletteColorCount(int format)
{
    if (m_FormatInfo.count(format))
    {
        return m_FormatInfo.at(format).paletteColorCount;
    }
    return 0;
}

static bool UsesPalette(int format)
{
    if (m_FormatInfo.count(format))
    {
        return m_FormatInfo.at(format).paletteColorCount > 0;
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

static uint32_t ColorToRGBA32(int srcFormat, unsigned int color)
{
    using namespace ScriptAPI;

    float r, g, b, a;

    switch (srcFormat)
    {
    case IMG_RGBA32:
        return color;
    case IMG_RGBA16:
    case IMG_CI8_RGBA16:
    case IMG_CI4_RGBA16:
        {
            r = ((color >> 11) & 0x1F) / 31.0f;
            g = ((color >> 6) & 0x1F) / 31.0f;
            b = ((color >> 1) & 0x1F) / 31.0f;
            a = color & 1;
        }
        break;
    case IMG_IA16:
    case IMG_CI8_IA16:
    case IMG_CI4_IA16:
        {
            r = g = b = (color >> 8) / 255.0f;
            a = (color & 0xFF) / 255.0f;
        }
        break;
    case IMG_I4:
        {
            r = g = b = color / 15.0f;
            a = 1;
        }
        break;
    case IMG_IA4:
        {
            r = g = b = (color >> 1) / 7.0f;
            a = (color & 1);
        }
        break;
    case IMG_I8:
        {
            r = g = b = color / 255.0f;
            a = 1;
        }
        break;
    case IMG_IA8:
        {
            r = g = b = (color >> 4) / 15.0f;
            a = (color & 0x0F) / 15.0f;
        }
        break;
    }

    return (uint8_t)(255 * r) << 24 |
           (uint8_t)(255 * g) << 16 |
           (uint8_t)(255 * b) << 8 |
           (uint8_t)(255 * a);
}

static uint32_t GetColor(int format, size_t nPixel, void* data, void* palette)
{
    unsigned int texel = GetTexel(format, data, nPixel);

    if (UsesPalette(format))
    {
        if (palette == nullptr)
        {
            return 0;
        }
        void* pColor = (char*)palette + texel;
        unsigned int color = _byteswap_ushort(*(uint16_t*)pColor);
        return ColorToRGBA32(format, color);
    }

    return ColorToRGBA32(format, texel);
}

static void PngWriteData(png_structp png_ptr, png_bytep data, png_size_t length)
{
    std::vector<uint8_t>* buffer = (std::vector<uint8_t>*)png_get_io_ptr(png_ptr);
    buffer->insert(buffer->end(), &data[0], &data[length]);
}

static void PngOutputFlushNoop(png_structp png_ptr) { }

static void MakePNG(uint8_t* rgba32, size_t width, size_t height, std::vector<uint8_t>& buffer)
{
    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);

    if (!png_ptr)
    {
        png_destroy_write_struct(&png_ptr, nullptr);
        return;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);

    if (!info_ptr)
    {
        png_destroy_write_struct(&png_ptr, nullptr);
        return;
    }

    if (setjmp(png_jmpbuf(png_ptr)))
    {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        return;
    }

    png_set_IHDR(png_ptr, info_ptr, width, height,
        8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    png_set_write_fn(png_ptr, &buffer, PngWriteData, PngOutputFlushNoop);
    png_write_info(png_ptr, info_ptr);

    size_t rowSize = width * 4;
    for (size_t nRow = 0; nRow < height; nRow++)
    {
        png_write_row(png_ptr, &rgba32[nRow * rowSize]);
    }
    
    png_destroy_write_struct(&png_ptr, &info_ptr);
}