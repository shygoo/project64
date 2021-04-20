#include <stdafx.h>
#include "../ScriptAPI.h"
#include "../ScriptRenderWindow.h"

static CScriptRenderWindow* GetThisRW(duk_context* ctx)
{
    duk_push_this(ctx);
    duk_get_prop_string(ctx, -1, DUK_HIDDEN_SYMBOL("srw"));
    void* p = duk_get_pointer(ctx, -1);

    if (p == nullptr)
    {
        duk_push_error_object(ctx, DUK_ERR_REFERENCE_ERROR, "internal drawing context expired");
        duk_throw(ctx);
        return nullptr;
    }

    duk_pop(ctx);
    return (CScriptRenderWindow*)p;
}

void ScriptAPI::Define_DrawingContext(duk_context* ctx)
{
    const duk_function_list_entry funcs[] = {
        { "color", js_DrawingContext_color, DUK_VARARGS },
        { "print", js_DrawingContext_print, 3 },
        { "fillrect", js_DrawingContext_fillrect, 4 },
        { "setdata", js_DrawingContext_setdata, 5 },
        { "getdata", js_DrawingContext_getdata, 4 },
        { nullptr, nullptr, 0 }
    };

    duk_push_global_object(ctx);
    duk_push_string(ctx, "DrawingContext");
    duk_push_c_function(ctx, js_DrawingContext__constructor, 3);
    duk_push_object(ctx);
    duk_put_function_list(ctx, -1, funcs);
    duk_freeze(ctx, -1);
    duk_put_prop_string(ctx, -2, "prototype");
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_ENUMERABLE);
    duk_pop(ctx);
}

duk_ret_t ScriptAPI::js_DrawingContext__constructor(duk_context* ctx)
{
    // TODO: set private constructor flag

    if (!duk_is_constructor_call(ctx))
    {
        return DUK_RET_ERROR;
    }

    duk_push_this(ctx);

    duk_push_string(ctx, "fillColor");
    duk_push_c_function(ctx, js_DrawingContext__get_fillColor, 0);
    duk_push_c_function(ctx, js_DrawingContext__set_fillColor, 1);
    duk_def_prop(ctx, -4, DUK_DEFPROP_HAVE_GETTER | DUK_DEFPROP_HAVE_SETTER);

    duk_push_string(ctx, "width");
    duk_push_c_function(ctx, js_DrawingContext__get_width, 0);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_GETTER);

    duk_push_string(ctx, "height");
    duk_push_c_function(ctx, js_DrawingContext__get_height, 0);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_GETTER);

    duk_push_string(ctx, "fontFamily");
    duk_push_c_function(ctx, js_DrawingContext__get_fontFamily, 0);
    duk_push_c_function(ctx, js_DrawingContext__set_fontFamily, 1);
    duk_def_prop(ctx, -4, DUK_DEFPROP_HAVE_GETTER | DUK_DEFPROP_HAVE_SETTER);

    duk_push_string(ctx, "fontSize");
    duk_push_c_function(ctx, js_DrawingContext__get_fontSize, 0);
    duk_push_c_function(ctx, js_DrawingContext__set_fontSize, 1);
    duk_def_prop(ctx, -4, DUK_DEFPROP_HAVE_GETTER | DUK_DEFPROP_HAVE_SETTER);

    duk_dup(ctx, 0); // ScriptRenderWindow*
    duk_put_prop_string(ctx, -2, DUK_HIDDEN_SYMBOL("srw"));
    return 0;
}

duk_ret_t ScriptAPI::js_DrawingContext__get_width(duk_context* ctx)
{
    CScriptRenderWindow* rw = GetThisRW(ctx);
    duk_push_number(ctx, rw->GetWidth());
    return 1;
}
duk_ret_t ScriptAPI::js_DrawingContext__get_height(duk_context* ctx)
{
    CScriptRenderWindow* rw = GetThisRW(ctx);
    duk_push_number(ctx, rw->GetHeight());
    return 1;
}

duk_ret_t ScriptAPI::js_DrawingContext__get_fillColor(duk_context* ctx)
{
    CScriptRenderWindow* rw = GetThisRW(ctx);
    // todo
    duk_push_uint(ctx, 1);
    return 1;
}

duk_ret_t ScriptAPI::js_DrawingContext__set_fillColor(duk_context* ctx)
{
    CScriptRenderWindow* rw = GetThisRW(ctx);

    if (!duk_is_number(ctx, 0))
    {
        return ThrowInvalidArgsError(ctx);
    }

    duk_double_t color = duk_get_number(ctx, 0);

    if (color < 0 || color > UINT_MAX)
    {
        return DUK_RET_RANGE_ERROR;
    }

    uint32_t rgba32 = (uint32_t)color;
    float r = (float)((rgba32 >> 24) & 0xFF) / 255.0f;
    float g = (float)((rgba32 >> 16) & 0xFF) / 255.0f;
    float b = (float)((rgba32 >>  8) & 0xFF) / 255.0f;
    float a = (float)((rgba32 >>  0) & 0xFF) / 255.0f;

    rw->GfxSetFillColor(r, g, b, a);

    return 0;
}

duk_ret_t ScriptAPI::js_DrawingContext__get_fontFamily(duk_context* ctx)
{
    CScriptRenderWindow* rw = GetThisRW(ctx);
    duk_push_string(ctx, "todo");
    return 1;
}

duk_ret_t ScriptAPI::js_DrawingContext__set_fontFamily(duk_context* ctx)
{
    CScriptRenderWindow* rw = GetThisRW(ctx);
    if (!duk_is_string(ctx, 0))
    {
        return ThrowInvalidArgsError(ctx);
    }

    const char* fontFamily = duk_get_string(ctx, 0);
    rw->GfxSetFontFamily(stdstr(fontFamily).ToUTF16().c_str());
    return 0;
}

duk_ret_t ScriptAPI::js_DrawingContext__get_fontSize(duk_context* ctx)
{
    duk_push_string(ctx, "todo");
    return 1;
}

duk_ret_t ScriptAPI::js_DrawingContext__set_fontSize(duk_context* ctx)
{
    CScriptRenderWindow* rw = GetThisRW(ctx);
    if (!duk_is_number(ctx, 0))
    {
        return ThrowInvalidArgsError(ctx);
    }

    duk_double_t fontSize = duk_get_number(ctx, 0);
    rw->GfxSetFontSize((float)fontSize);
    return 0;
}

// TODO this should be static
duk_ret_t ScriptAPI::js_DrawingContext_color(duk_context* ctx)
{
    duk_idx_t nargs = duk_get_top(ctx);

    if (!duk_is_number(ctx, 0) ||
        !duk_is_number(ctx, 1) ||
        !duk_is_number(ctx, 2) ||
        (nargs == 4 && !duk_is_number(ctx, 3)))
    {
        return ThrowInvalidArgsError(ctx);
    }

    duk_double_t r = duk_get_number(ctx, 0);
    duk_double_t g = duk_get_number(ctx, 1);
    duk_double_t b = duk_get_number(ctx, 2);
    duk_double_t a = (nargs == 4) ? duk_get_number(ctx, 3) : 1.0;

    if (r < 0 || r > 255 ||
        g < 0 || g > 255 ||
        b < 0 || b > 255 ||
        a < 0 || a > 1.0)
    {
        return DUK_RET_RANGE_ERROR;
    }

    uint32_t rgba32 = (uint32_t)r << 24 | (uint32_t)g << 16 |
                      (uint32_t)b << 8 | (uint32_t)(255 * a);

    duk_push_uint(ctx, rgba32);
    return 1;
}

duk_ret_t ScriptAPI::js_DrawingContext_print(duk_context* ctx)
{
    CScriptRenderWindow* rw = GetThisRW(ctx);

    int x = duk_get_number(ctx, 0);
    int y = duk_get_number(ctx, 1);
    const char* text = duk_safe_to_string(ctx, 2);

    rw->GfxDrawText(x, y, stdstr(text).ToUTF16().c_str());

    return 0;
}

duk_ret_t ScriptAPI::js_DrawingContext_fillrect(duk_context* ctx)
{
    CScriptRenderWindow* rw = GetThisRW(ctx);
    
    if (!duk_is_number(ctx, 0) ||
        !duk_is_number(ctx, 1) ||
        !duk_is_number(ctx, 2) ||
        !duk_is_number(ctx, 3))
    {
        return ThrowInvalidArgsError(ctx);
    }

    duk_push_this(ctx);
    duk_get_prop_string(ctx, -1, "fillColor");
    uint32_t fillColor = duk_get_uint(ctx, -1);

    int x = duk_get_number(ctx, 0);
    int y = duk_get_number(ctx, 1);
    int w = duk_get_number(ctx, 2);
    int h = duk_get_number(ctx, 3);

    rw->GfxFillRect(x, y, x + w, y + h);

    return 0;
}
duk_ret_t ScriptAPI::js_DrawingContext_setdata(duk_context* ctx)
{
    CScriptRenderWindow* rw = GetThisRW(ctx);

    return 0;
}

duk_ret_t ScriptAPI::js_DrawingContext_getdata(duk_context* ctx)
{
    CScriptRenderWindow* rw = GetThisRW(ctx);

    return 0;
}
