#include <stdafx.h>
#include "../ScriptAPI.h"

void ScriptAPI::Define_asm(duk_context *ctx)
{
    const duk_function_list_entry funcs[] = {
        { "gprname", js_asm_gprname, 1 },
        { NULL, NULL, 0 }
    };

    duk_push_global_object(ctx);
    duk_push_string(ctx, "asm");
    duk_push_object(ctx);
    duk_put_function_list(ctx, -1, funcs);
    duk_freeze(ctx, -1);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_ENUMERABLE);
    duk_pop(ctx);
}

duk_ret_t ScriptAPI::js_asm_gprname(duk_context* ctx)
{
    const char* names[32] = {
        "r0", "at", "v0", "v1", "a0", "a1", "a2", "a3",
        "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
        "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
        "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra"
    };

    if (!duk_is_number(ctx, 0))
    {
        return ThrowInvalidArgsError(ctx);
    }

    duk_uint_t idx = duk_get_uint(ctx, 0);

    if (idx < 32)
    {
        duk_push_string(ctx, names[idx]);
    }
    else
    {
        return DUK_RET_RANGE_ERROR;
    }

    return 1;
}