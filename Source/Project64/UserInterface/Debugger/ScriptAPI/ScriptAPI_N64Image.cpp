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

duk_ret_t ScriptAPI::N64Image__constructor(duk_context* ctx)
{
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