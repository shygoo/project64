#include <stdafx.h>
#include "../ScriptAPI.h"

void ScriptAPI::Define_RGBA(duk_context* ctx)
{
    duk_push_global_object(ctx);
    duk_push_string(ctx, "RGBA");
    duk_push_c_function(ctx, js_RGBA, DUK_VARARGS);
    duk_freeze(ctx, -1);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_ENUMERABLE);
    duk_pop(ctx);
}

duk_ret_t ScriptAPI::js_RGBA(duk_context* ctx)
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
        (uint32_t)b << 8 | (uint32_t)roundf(255 * a);

    duk_push_uint(ctx, rgba32);
    return 1;
}
