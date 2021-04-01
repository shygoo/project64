#include <stdafx.h>
#include "../ScriptAPI.h"


void ScriptAPI::Define_debug(duk_context* ctx)
{
    const duk_function_list_entry funcs[] = {
        { "breakhere", js_debug_breakhere, 1 },
        { NULL, NULL, 0 }
    };

    duk_push_global_object(ctx);
    duk_push_string(ctx, "debug");
    duk_push_object(ctx);
    duk_put_function_list(ctx, -1, funcs);
    duk_freeze(ctx, -1);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_ENUMERABLE);
    duk_pop(ctx);
}

duk_ret_t ScriptAPI::js_debug_breakhere(duk_context* /*ctx*/)
{
    g_Settings->SaveBool(Debugger_SteppingOps, true);
    return 0;
}