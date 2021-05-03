#include <stdafx.h>
#include "../ScriptAPI.h"
#include "../ScriptRenderWindow.h"

#pragma warning(disable: 4702)

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
        { "print", js_DrawingContext_print, 3 },
        { "fillrect", js_DrawingContext_fillrect, 4 },
        { "beginpath", js_DrawingContext_beginpath, 0 },
        { "moveto", js_DrawingContext_moveto, 2 },
        { "lineto", js_DrawingContext_lineto, 2 },
        { "stroke", js_DrawingContext_stroke, 0 },
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

    duk_push_string(ctx, "width");
    duk_push_c_function(ctx, js_DrawingContext__get_width, 0);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_GETTER);

    duk_push_string(ctx, "height");
    duk_push_c_function(ctx, js_DrawingContext__get_height, 0);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_GETTER);

    duk_push_string(ctx, "fillColor");
    duk_push_c_function(ctx, js_DrawingContext__get_fillColor, 0);
    duk_push_c_function(ctx, js_DrawingContext__set_fillColor, 1);
    duk_def_prop(ctx, -4, DUK_DEFPROP_HAVE_GETTER | DUK_DEFPROP_HAVE_SETTER);

    duk_push_string(ctx, "strokeColor");
    duk_push_c_function(ctx, js_DrawingContext__get_strokeColor, 0);
    duk_push_c_function(ctx, js_DrawingContext__set_strokeColor, 1);
    duk_def_prop(ctx, -4, DUK_DEFPROP_HAVE_GETTER | DUK_DEFPROP_HAVE_SETTER);

    duk_push_string(ctx, "strokeWidth");
    duk_push_c_function(ctx, js_DrawingContext__get_strokeWidth, 0);
    duk_push_c_function(ctx, js_DrawingContext__set_strokeWidth, 1);
    duk_def_prop(ctx, -4, DUK_DEFPROP_HAVE_GETTER | DUK_DEFPROP_HAVE_SETTER);

    duk_push_string(ctx, "fontFamily");
    duk_push_c_function(ctx, js_DrawingContext__get_fontFamily, 0);
    duk_push_c_function(ctx, js_DrawingContext__set_fontFamily, 1);
    duk_def_prop(ctx, -4, DUK_DEFPROP_HAVE_GETTER | DUK_DEFPROP_HAVE_SETTER);

    duk_push_string(ctx, "fontSize");
    duk_push_c_function(ctx, js_DrawingContext__get_fontSize, 0);
    duk_push_c_function(ctx, js_DrawingContext__set_fontSize, 1);
    duk_def_prop(ctx, -4, DUK_DEFPROP_HAVE_GETTER | DUK_DEFPROP_HAVE_SETTER);

    duk_push_string(ctx, "fontWeight");
    duk_push_c_function(ctx, js_DrawingContext__get_fontWeight, 0);
    duk_push_c_function(ctx, js_DrawingContext__set_fontWeight, 1);
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
    duk_push_uint(ctx, rw->GfxGetFillColor());
    return 1;
}

duk_ret_t ScriptAPI::js_DrawingContext__set_fillColor(duk_context* ctx)
{
    CScriptRenderWindow* rw = GetThisRW(ctx);

    if (!duk_is_number(ctx, 0))
    {
        return ThrowInvalidArgsError(ctx); // todo invalid assignment error
    }

    duk_double_t color = duk_get_number(ctx, 0);

    if (color < 0 || color > UINT_MAX)
    {
        return DUK_RET_RANGE_ERROR;
    }

    rw->GfxSetFillColor((uint32_t)color);
    return 0;
}

duk_ret_t ScriptAPI::js_DrawingContext__get_strokeColor(duk_context* ctx)
{
    CScriptRenderWindow* rw = GetThisRW(ctx);
    duk_push_number(ctx, rw->GfxGetStrokeColor());
    return 1;
}

duk_ret_t ScriptAPI::js_DrawingContext__set_strokeColor(duk_context* ctx)
{
    CScriptRenderWindow* rw = GetThisRW(ctx);

    if (!duk_is_number(ctx, 0))
    {
        return ThrowInvalidArgsError(ctx); // todo invalid assignment error
    }

    duk_double_t color = duk_get_number(ctx, 0);

    if (color < 0 || color > UINT_MAX)
    {
        return DUK_RET_RANGE_ERROR;
    }

    rw->GfxSetStrokeColor((uint32_t)color);
    return 0;
}

duk_ret_t ScriptAPI::js_DrawingContext__get_strokeWidth(duk_context* ctx)
{
    CScriptRenderWindow* rw = GetThisRW(ctx);
    duk_push_number(ctx, rw->GfxGetStrokeWidth());
    return 1;
}

duk_ret_t ScriptAPI::js_DrawingContext__set_strokeWidth(duk_context* ctx)
{
    CScriptRenderWindow* rw = GetThisRW(ctx);

    if (!duk_is_number(ctx, 0))
    {
        return ThrowInvalidArgsError(ctx); // todo invalid assignment error
    }

    rw->GfxSetStrokeWidth(duk_get_number(ctx, 0));

    return 0;
}

duk_ret_t ScriptAPI::js_DrawingContext__get_fontFamily(duk_context* ctx)
{
    CScriptRenderWindow* rw = GetThisRW(ctx);
    stdstr fontFamily = rw->GfxGetFontFamily();
    duk_push_string(ctx, fontFamily.c_str());
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
    CScriptRenderWindow* rw = GetThisRW(ctx);
    duk_push_number(ctx, rw->GfxGetFontSize());
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

duk_ret_t ScriptAPI::js_DrawingContext__get_fontWeight(duk_context* ctx)
{
    CScriptRenderWindow* rw = GetThisRW(ctx);

    DWRITE_FONT_WEIGHT weight = rw->GfxGetFontWeight();
    const char* weightName = "";

    switch (weight)
    {
    case DWRITE_FONT_WEIGHT_LIGHT:
        weightName = "light";
        break;
    case DWRITE_FONT_WEIGHT_NORMAL:
        weightName = "normal";
        break;
    case DWRITE_FONT_WEIGHT_BOLD:
        weightName = "bold";
        break;
    }

    duk_push_string(ctx, weightName);
    return 1;
}

duk_ret_t ScriptAPI::js_DrawingContext__set_fontWeight(duk_context* ctx)
{
    CScriptRenderWindow* rw = GetThisRW(ctx);
    if (!duk_is_string(ctx, 0))
    {
        return ThrowInvalidArgsError(ctx); // todo invalid assignment error
    }

    const char* weightName = duk_get_string(ctx, 0);

    const std::map<std::string, DWRITE_FONT_WEIGHT> weights = {
        { "light", DWRITE_FONT_WEIGHT_LIGHT },
        { "normal", DWRITE_FONT_WEIGHT_NORMAL },
        { "bold", DWRITE_FONT_WEIGHT_BOLD }
    };

    if (weights.count(weightName) == 0)
    {
        return DUK_RET_ERROR; // todo invalid font weight error
    }

    rw->GfxSetFontWeight(weights.at(weightName));
    return 0;
}

duk_ret_t ScriptAPI::js_DrawingContext_print(duk_context* ctx)
{
    CScriptRenderWindow* rw = GetThisRW(ctx);

    duk_double_t x = duk_get_number(ctx, 0);
    duk_double_t y = duk_get_number(ctx, 1);
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

    duk_double_t x = duk_get_number(ctx, 0);
    duk_double_t y = duk_get_number(ctx, 1);
    duk_double_t width = duk_get_number(ctx, 2);
    duk_double_t height = duk_get_number(ctx, 3);

    rw->GfxFillRect(x, y, x + width, y + height);

    return 0;
}

duk_ret_t ScriptAPI::js_DrawingContext_beginpath(duk_context* ctx)
{
    CScriptRenderWindow* rw = GetThisRW(ctx);
    rw->GfxBeginPath();
    return 0;
}

duk_ret_t ScriptAPI::js_DrawingContext_moveto(duk_context* ctx)
{
    CScriptRenderWindow* rw = GetThisRW(ctx);

    if (!duk_is_number(ctx, 0) ||
        !duk_is_number(ctx, 1))
    {
        return ThrowInvalidArgsError(ctx);
    }

    float x = duk_get_number(ctx, 0);
    float y = duk_get_number(ctx, 1);
    
    rw->GfxMoveTo(x, y);

    return 0;
}

duk_ret_t ScriptAPI::js_DrawingContext_lineto(duk_context* ctx)
{
    CScriptRenderWindow* rw = GetThisRW(ctx);

    if (!duk_is_number(ctx, 0) ||
        !duk_is_number(ctx, 1))
    {
        return ThrowInvalidArgsError(ctx);
    }

    float x = duk_get_number(ctx, 0);
    float y = duk_get_number(ctx, 1);

    rw->GfxLineTo(x, y);

    return 0;
}

duk_ret_t ScriptAPI::js_DrawingContext_stroke(duk_context* ctx)
{
    CScriptRenderWindow* rw = GetThisRW(ctx);
    rw->GfxStroke();
    return 0;
}
