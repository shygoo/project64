#include <stdafx.h>
#include "../ScriptAPI.h"
#include "../ScriptRenderWindow.h"
#include "N64Image.h"

#pragma warning(disable: 4702) // disable unreachable code warning

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
        { "drawtext", js_DrawingContext_drawtext, 3 },
        { "measuretext", js_DrawingContext_measuretext, 1},
        { "drawimage", js_DrawingContext_drawimage, DUK_VARARGS },
        { "fillrect", js_DrawingContext_fillrect, 4 },
        { "strokerect", js_DrawingContext_strokerect, 4 },
        { "beginpath", js_DrawingContext_beginpath, 0 },
        { "moveto", js_DrawingContext_moveto, 2 },
        { "lineto", js_DrawingContext_lineto, 2 },
        { "stroke", js_DrawingContext_stroke, 0 },
        { "fill", js_DrawingContext_fill, 0 },
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
    if (!duk_is_constructor_call(ctx) ||
        !PrivateCallAllowed(ctx))
    {
        return ThrowNotCallableError(ctx);
    }

    const DukPropListEntry props[] = {
        { DUK_HIDDEN_SYMBOL("srw"), DukDupIndex(0) },
        { "pointer", DukGetter(js_DrawingContext__get_pointer) },
        { "width", DukGetter(js_DrawingContext__get_width) },
        { "height", DukGetter(js_DrawingContext__get_height) },
        { "fillColor", DukGetterSetter (
            js_DrawingContext__get_fillColor,
            js_DrawingContext__set_fillColor) },
        { "strokeColor", DukGetterSetter (
            js_DrawingContext__get_strokeColor,
            js_DrawingContext__set_strokeColor) },
        { "strokeWidth", DukGetterSetter (
            js_DrawingContext__get_strokeWidth,
            js_DrawingContext__set_strokeWidth) },
        { "fontFamily", DukGetterSetter (
            js_DrawingContext__get_fontFamily,
            js_DrawingContext__set_fontFamily) },
        { "fontSize", DukGetterSetter (
            js_DrawingContext__get_fontSize,
            js_DrawingContext__set_fontSize) },
        { "fontWeight", DukGetterSetter (
            js_DrawingContext__get_fontWeight,
            js_DrawingContext__set_fontWeight) },
        { nullptr }
    };

    duk_push_this(ctx);
    DukPutPropList(ctx, -1, props);
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
        return ThrowInvalidAssignmentError(ctx);
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
        return ThrowInvalidAssignmentError(ctx);
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
        return ThrowInvalidAssignmentError(ctx);
    }

    rw->GfxSetStrokeWidth((float)duk_get_number(ctx, 0));

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

    CScriptRenderWindow::FontWeight weight = rw->GfxGetFontWeight();
    const char* weightName = "";

    switch (weight)
    {
    case CScriptRenderWindow::GFW_LIGHT:
        weightName = "light";
        break;
    case CScriptRenderWindow::GFW_NORMAL:
        weightName = "normal";
        break;
    case CScriptRenderWindow::GFW_BOLD:
        weightName = "bold";
        break;
    default:
        weightName = "";
    }

    duk_push_string(ctx, weightName);
    return 1;
}

duk_ret_t ScriptAPI::js_DrawingContext__set_fontWeight(duk_context* ctx)
{
    CScriptRenderWindow* rw = GetThisRW(ctx);
    if (!duk_is_string(ctx, 0))
    {
        return ThrowInvalidAssignmentError(ctx);
    }

    const char* weightName = duk_get_string(ctx, 0);

    static const std::map<std::string, CScriptRenderWindow::FontWeight> weights = {
        { "light",  CScriptRenderWindow::GFW_LIGHT },
        { "normal", CScriptRenderWindow::GFW_NORMAL },
        { "bold",   CScriptRenderWindow::GFW_BOLD }
    };

    if (weights.count(weightName) == 0)
    {
        duk_push_error_object(ctx, DUK_ERR_ERROR, "invalid font weight");
        return duk_throw(ctx);
    }

    rw->GfxSetFontWeight(weights.at(weightName));
    return 0;
}

duk_ret_t ScriptAPI::js_DrawingContext__get_pointer(duk_context* ctx)
{
    CScriptRenderWindow* rw = GetThisRW(ctx);
    duk_push_pointer(ctx, rw->GetRenderTarget());
    return 1;
}

duk_ret_t ScriptAPI::js_DrawingContext_drawtext(duk_context* ctx)
{
    CScriptRenderWindow* rw = GetThisRW(ctx);

    float x = (float)duk_get_number(ctx, 0);
    float y = (float)duk_get_number(ctx, 1);
    const char* text = duk_safe_to_string(ctx, 2);

    rw->GfxDrawText(x, y, stdstr(text).ToUTF16().c_str());

    return 0;
}

duk_ret_t ScriptAPI::js_DrawingContext_measuretext(duk_context* ctx)
{
    CScriptRenderWindow* rw = GetThisRW(ctx);

    const char* text = duk_safe_to_string(ctx, 0);

    CScriptRenderWindow::TextMetrics textMetrics;
    rw->GfxMeasureText(stdstr(text).ToUTF16().c_str(), &textMetrics);

    duk_push_object(ctx);
    
    const DukPropListEntry props[] = {
        { "left", DukUInt(textMetrics.left) },
        { "top", DukUInt(textMetrics.top) },
        { "width", DukUInt(textMetrics.width) },
        { "height", DukUInt(textMetrics.height) },
        { nullptr }
    };

    DukPutPropList(ctx, -1, props);
    return 1;
}

duk_ret_t ScriptAPI::js_DrawingContext_drawimage(duk_context* ctx)
{
    CScriptRenderWindow* rw = GetThisRW(ctx);

    if (!duk_is_number(ctx, 0) ||
        !duk_is_number(ctx, 1) ||
        !duk_is_object(ctx, 2))
    {
        return ThrowInvalidArgsError(ctx);
    }

    float dx = (float)duk_get_number(ctx, 0);
    float dy = (float)duk_get_number(ctx, 1);
    duk_get_prop_string(ctx, 2, DUK_HIDDEN_SYMBOL("N64IMAGE"));
    CN64Image* image = (CN64Image*)duk_get_pointer(ctx, -1);

    if (image == nullptr)
    {
        return ThrowInvalidArgsError(ctx);
    }

    float dw = (float)duk_get_number_default(ctx, 3, image->Width());
    float dh = (float)duk_get_number_default(ctx, 4, image->Height());
    float sx = (float)duk_get_number_default(ctx, 5, 0);
    float sy = (float)duk_get_number_default(ctx, 6, 0);
    float sw = (float)duk_get_number_default(ctx, 7, dw);
    float sh = (float)duk_get_number_default(ctx, 8, dh);

    std::vector<uint8_t>& bitmap = image->Bitmap();
    rw->GfxDrawImage(&bitmap[0], bitmap.size(), image->Width(), image->Height(), sw, sh, sx, sy, dw, dh, dx, dy);

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

    float x = (float)duk_get_number(ctx, 0);
    float y = (float)duk_get_number(ctx, 1);
    float width = (float)duk_get_number(ctx, 2);
    float height = (float)duk_get_number(ctx, 3);

    rw->GfxFillRect(x, y, x + width, y + height);

    return 0;
}

duk_ret_t ScriptAPI::js_DrawingContext_strokerect(duk_context* ctx)
{
    CScriptRenderWindow* rw = GetThisRW(ctx);

    if (!duk_is_number(ctx, 0) ||
        !duk_is_number(ctx, 1) ||
        !duk_is_number(ctx, 2) ||
        !duk_is_number(ctx, 3))
    {
        return ThrowInvalidArgsError(ctx);
    }

    float x = (float)duk_get_number(ctx, 0);
    float y = (float)duk_get_number(ctx, 1);
    float width = (float)duk_get_number(ctx, 2);
    float height = (float)duk_get_number(ctx, 3);

    rw->GfxStrokeRect(x, y, x + width, y + height);

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

    float x = (float)duk_get_number(ctx, 0);
    float y = (float)duk_get_number(ctx, 1);
    
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

    float x = (float)duk_get_number(ctx, 0);
    float y = (float)duk_get_number(ctx, 1);

    rw->GfxLineTo(x, y);

    return 0;
}

duk_ret_t ScriptAPI::js_DrawingContext_stroke(duk_context* ctx)
{
    CScriptRenderWindow* rw = GetThisRW(ctx);
    rw->GfxStroke();
    return 0;
}

duk_ret_t ScriptAPI::js_DrawingContext_fill(duk_context* ctx)
{
    CScriptRenderWindow* rw = GetThisRW(ctx);
    rw->GfxFill();
    return 0;
}
