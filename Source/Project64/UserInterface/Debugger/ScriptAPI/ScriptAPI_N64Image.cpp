#include <stdafx.h>
#include "../ScriptAPI.h"
#include "N64Image.h"

#pragma warning(disable: 4702) // disable unreachable code warning

static CN64Image* GetThisImage(duk_context* ctx)
{
    duk_push_this(ctx);
    duk_get_prop_string(ctx, 0, DUK_HIDDEN_SYMBOL("N64IMAGE"));
    CN64Image* image = (CN64Image*)duk_get_pointer(ctx, -1);
    duk_pop_n(ctx, 2);

    if (image == nullptr)
    {
        duk_push_error_object(ctx, DUK_ERR_ERROR, "internal image object is null");
        return duk_throw(ctx);
    }

    return image;
}

void ScriptAPI::Define_N64Image(duk_context* ctx)
{
    const duk_function_list_entry methods[] = {
        { "toPNG", js_N64Image_toPNG, DUK_VARARGS },
        { "update", js_N64Image_update, 0 },
        { nullptr, 0 }
    };
    
    const duk_function_list_entry staticFuncs[] = {
        { "fromPNG", js_N64Image_static_fromPNG, DUK_VARARGS },
        { "format",  js_N64Image_static_format, DUK_VARARGS },
        { "bpp",     js_N64Image_static_bpp, DUK_VARARGS },
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

static void InitImageObjectProps(duk_context* ctx, duk_idx_t idx, CN64Image* image)
{
    idx = duk_normalize_index(ctx, idx);
    duk_push_pointer(ctx, image);
    duk_put_prop_string(ctx, idx, DUK_HIDDEN_SYMBOL("N64IMAGE"));

    duk_push_external_buffer(ctx);
    duk_config_buffer(ctx, -1, image->PixelData().data(), image->PixelData().size());
    duk_push_string(ctx, "pixels");
    duk_push_buffer_object(ctx, -2, 0, image->PixelData().size(), DUK_BUFOBJ_NODEJS_BUFFER);
    duk_remove(ctx, -3);
    duk_def_prop(ctx, idx, DUK_DEFPROP_HAVE_VALUE);

    if (image->UsesPalette())
    {
        duk_push_external_buffer(ctx);
        duk_config_buffer(ctx, -1, image->PaletteData().data(), image->PaletteData().size());
        duk_push_string(ctx, "palette");
        duk_push_buffer_object(ctx, -2, 0, image->PaletteData().size(), DUK_BUFOBJ_NODEJS_BUFFER);
        duk_remove(ctx, -3);
        duk_def_prop(ctx, idx, DUK_DEFPROP_HAVE_VALUE);
    }
    else
    {
        duk_push_string(ctx, "palette");
        duk_push_null(ctx);
        duk_def_prop(ctx, idx, DUK_DEFPROP_HAVE_VALUE);
    }

    duk_push_string(ctx, "format");
    duk_push_number(ctx, image->Format());
    duk_def_prop(ctx, idx, DUK_DEFPROP_HAVE_VALUE);
    
    duk_push_string(ctx, "width");
    duk_push_uint(ctx, image->Width());
    duk_def_prop(ctx, idx, DUK_DEFPROP_HAVE_VALUE);

    duk_push_string(ctx, "height");
    duk_push_uint(ctx, image->Height());
    duk_def_prop(ctx, idx, DUK_DEFPROP_HAVE_VALUE);

    duk_push_c_function(ctx, ScriptAPI::js_N64Image__finalizer, 1);
    duk_set_finalizer(ctx, idx);
}

duk_ret_t ScriptAPI::js_N64Image__constructor(duk_context* ctx)
{
    duk_idx_t nargs = duk_get_top(ctx);

    if (!duk_is_number(ctx, 0) ||
        !duk_is_number(ctx, 1) ||
        (nargs > 2 && !duk_is_number(ctx, 2)) ||
        (nargs > 3 && !duk_is_buffer_data(ctx, 3)) ||
        (nargs > 4 && !duk_is_buffer_data(ctx, 4)))
    {
        return ThrowInvalidArgsError(ctx);
    }

    if (!duk_is_constructor_call(ctx))
    {
        return DUK_RET_ERROR;
    }

    size_t pixelDataSize = 0, paletteDataSize = 0;

    size_t width = duk_get_uint(ctx, 0);
    size_t height = duk_get_uint(ctx, 1);
    int format = (nargs > 2) ? duk_get_int(ctx, 2) : IMG_RGBA32;
    void* pixelData = (nargs > 3) ? duk_get_buffer_data(ctx, 3, &pixelDataSize) : nullptr;
    void* paletteData = (nargs > 4) ? duk_get_buffer_data(ctx, 4, &paletteDataSize) : nullptr;

    // TODO do not make copies of pixelData and paletteData

    CN64Image* image = new CN64Image();
    int result = image->Init(format, width, height, pixelData, pixelDataSize, paletteData, paletteDataSize);

    if (result != N64IMG_OK)
    {
        duk_push_error_object(ctx, DUK_ERR_ERROR, "failed to initialize image (%s)",
            CN64Image::ResultCodeName(result));
        return duk_throw(ctx);
    }

    duk_push_this(ctx);
    InitImageObjectProps(ctx, -1, image);
    return 0;
}

duk_ret_t ScriptAPI::js_N64Image__finalizer(duk_context* ctx)
{
    duk_get_prop_string(ctx, 0, DUK_HIDDEN_SYMBOL("N64IMAGE"));
    CN64Image* image = (CN64Image*)duk_get_pointer(ctx, -1);
    if (image == nullptr)
    {
        return 0;
    }
    delete image;
    return 0;
}

duk_ret_t ScriptAPI::js_N64Image_static_fromPNG(duk_context* ctx)
{
    duk_idx_t nargs = duk_get_top(ctx);

    if (!duk_is_buffer_data(ctx, 0) ||
        (nargs > 1 && !duk_is_number(ctx, 1)))
    {
        return ThrowInvalidArgsError(ctx);
    }

    int format = IMG_RGBA32;
    
    if (nargs > 1)
    {
        format = duk_get_uint(ctx, 1);
    
        if (CN64Image::BitsPerPixel(format) == 0)
        {
            duk_push_error_object(ctx, DUK_RET_TYPE_ERROR, "invalid format");
            return duk_throw(ctx);
        }
    }

    size_t pngSize;
    uint8_t* pngData = (uint8_t*)duk_get_buffer_data(ctx, 0, &pngSize);

    CN64Image* image = new CN64Image();
    int result = image->Init(format, pngData, pngSize);
    if (result != N64IMG_OK)
    {
        duk_push_error_object(ctx, DUK_ERR_ERROR, "failed to initialize image (%s)",
            CN64Image::ResultCodeName(result), result);
        return duk_throw(ctx);
    }
    
    duk_push_object(ctx);
    duk_get_global_string(ctx, "N64Image");
    duk_get_prop_string(ctx, -1, "prototype");
    duk_set_prototype(ctx, -3);
    duk_pop(ctx);

    InitImageObjectProps(ctx, -1, image);
    return 1;
}

duk_ret_t ScriptAPI::js_N64Image_static_format(duk_context* ctx)
{
    duk_idx_t nargs = duk_get_top(ctx);

    if (!duk_is_number(ctx, 0) ||
        !duk_is_number(ctx, 1) ||
        (nargs > 2 && !duk_is_number(ctx, 2)))
    {
        return ThrowInvalidArgsError(ctx);
    }

    duk_uint_t gbiFmt = duk_get_uint(ctx, 0);
    duk_uint_t gbiSiz = duk_get_uint(ctx, 1);
    duk_uint_t gbiTlutFmt = (nargs > 2) ? duk_get_uint(ctx, 2) : G_TT_NONE;

    int format = (gbiFmt << 3) | gbiSiz | gbiTlutFmt;

    switch (format)
    {
    case IMG_RGBA16:
    case IMG_RGBA32:
    case IMG_CI4_RGBA16:
    case IMG_CI4_IA16:
    case IMG_CI8_RGBA16:
    case IMG_CI8_IA16:
    case IMG_IA4:
    case IMG_IA8:
    case IMG_IA16:
    case IMG_I4:
    case IMG_I8:
        duk_push_number(ctx, format);
        break;
    default:
        duk_push_number(ctx, -1);
    }

    return 1;
}

duk_ret_t ScriptAPI::js_N64Image_static_bpp(duk_context* ctx)
{
    if (!duk_is_number(ctx, 0))
    {
        return ThrowInvalidArgsError(ctx);
    }

    duk_uint_t format = duk_get_uint(ctx, 0);

    int bpp = 0;

    switch (format)
    {
    case G_IM_SIZ_4b: bpp = 4; break;
    case G_IM_SIZ_8b: bpp = 8; break;
    case G_IM_SIZ_16b: bpp = 16; break;
    case G_IM_SIZ_32b: bpp = 32; break;
    default: bpp = CN64Image::BitsPerPixel(format); break;
    }

    duk_push_number(ctx, bpp);
    return 1;
}

duk_ret_t ScriptAPI::js_N64Image_toPNG(duk_context* ctx)
{
    CN64Image* image = GetThisImage(ctx);

    std::vector<uint8_t> png;
    image->ToPNG(png);

    void* pngCopy = duk_push_buffer(ctx, png.size(), false);
    duk_push_buffer_object(ctx, -1, 0, png.size(), DUK_BUFOBJ_NODEJS_BUFFER);
    memcpy(pngCopy, png.data(), png.size());

    return 1;
}

duk_ret_t ScriptAPI::js_N64Image_update(duk_context* ctx)
{
    CN64Image* image = GetThisImage(ctx);
    int result = image->UpdateBitmap();
    if (result != N64IMG_OK)
    {
        duk_push_error_object(ctx, DUK_ERR_ERROR, "bitmap update failed (%s)",
            CN64Image::ResultCodeName(result));
    }
    return 0;
}
