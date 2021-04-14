#include <stdafx.h>
#include "../ScriptAPI.h"

static void* GetPtr(duk_context* ctx)
{
    duk_push_this(ctx);
    duk_get_prop_string(ctx, -1, DUK_HIDDEN_SYMBOL("ptr"));
    void* p = duk_get_pointer(ctx, -1);
    duk_pop(ctx);
    return p;
}

static duk_ret_t ThrowContextExpiredError(duk_context* ctx)
{
    duk_push_error_object(ctx, DUK_ERR_REFERENCE_ERROR, "internal drawing context expired");
    return duk_throw(ctx);
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

    duk_push_uint(ctx, 0xFFFFFFFF);
    duk_put_prop_string(ctx, -2, "fillColor"); // todo make nonconfigurable

    duk_dup(ctx, 0); // hdc, TODO: make this raw frame data
    duk_put_prop_string(ctx, -2, DUK_HIDDEN_SYMBOL("ptr"));
    return 0;
}

duk_ret_t ScriptAPI::js_DrawingContext__get_width(duk_context* ctx) { return 0; }
duk_ret_t ScriptAPI::js_DrawingContext__get_height(duk_context* ctx) { return 0; }
duk_ret_t ScriptAPI::js_DrawingContext__get_fillColor(duk_context* ctx) { return 0; }
duk_ret_t ScriptAPI::js_DrawingContext__set_fillColor(duk_context* ctx) { return 0; }

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
    void* ptr = GetPtr(ctx);
    if (ptr == nullptr)
    {
        ThrowContextExpiredError(ctx);
    }



    return 0;
}

duk_ret_t ScriptAPI::js_DrawingContext_fillrect(duk_context* ctx)
{
    void* ptr = GetPtr(ctx);
    if (ptr == nullptr)
    {
        return ThrowContextExpiredError(ctx);
    }
    
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

    //GetInstance(ctx)->System()->Log("fillColor: %08X", fillColor);

    int x = duk_get_number(ctx, 0);
    int y = duk_get_number(ctx, 1);
    int w = duk_get_number(ctx, 2);
    int h = duk_get_number(ctx, 3);

    // alpha must be zero for now

    HDC hdc = (HDC)ptr;
    HBRUSH hbr = CreateSolidBrush(DUK_BSWAP32(fillColor));
    CRect rc(x, y, x + w, y + h);
    FillRect(hdc, &rc, hbr);
    DeleteObject(hbr);

    return 0;
}
duk_ret_t ScriptAPI::js_DrawingContext_setdata(duk_context* ctx)
{
    void* ptr = GetPtr(ctx);
    if (ptr == nullptr)
    {
        ThrowContextExpiredError(ctx);
    }

    return 0;
}

duk_ret_t ScriptAPI::js_DrawingContext_getdata(duk_context* ctx)
{
    void* ptr = GetPtr(ctx);
    if (ptr == nullptr)
    {
        ThrowContextExpiredError(ctx);
    }

    return 0;
}