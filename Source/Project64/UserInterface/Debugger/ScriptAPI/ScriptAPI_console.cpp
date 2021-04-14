#include <stdafx.h>
#include <windows.h>
#include "../ScriptAPI.h"

void ScriptAPI::Define_console(duk_context* ctx)
{
    const duk_function_list_entry funcs[] = {
        { "print", js_console_print, DUK_VARARGS },
        { "log", js_console_log, DUK_VARARGS },
        { "clear", js_console_clear, 0 },
        { "listen", js_console_listen, 1 },
        { nullptr, nullptr, 0 }
    };

    duk_push_global_object(ctx);
    duk_push_string(ctx, "console");
    duk_push_object(ctx);
    duk_put_function_list(ctx, -1, funcs);
    duk_freeze(ctx, -1);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_ENUMERABLE);
    duk_pop(ctx);
}

duk_ret_t ScriptAPI::js_console_print(duk_context* ctx)
{
    const char* str = duk_safe_to_string(ctx, 0);
    GetInstance(ctx)->System()->Print("%s", str);
    return 0;
}

duk_ret_t ScriptAPI::js_console_log(duk_context* ctx)
{
    duk_idx_t nargs = duk_get_top(ctx);

    stdstr str;

    for (duk_idx_t n = 0; n < nargs; n++)
    {
        if (n != 0)
        {
            str += " ";
        }

        str += duk_safe_to_string(ctx, n - nargs);
    }

    GetInstance(ctx)->System()->Log("%s", str.c_str());
    return 0;
}

duk_ret_t ScriptAPI::js_console_clear(duk_context* ctx)
{
    GetInstance(ctx)->System()->ClearLog();
    return 0;
}

duk_ret_t ScriptAPI::js_console_listen(duk_context* ctx)
{
    CScriptInstance* inst = GetInstance(ctx);

    duk_push_global_object(ctx);
    duk_bool_t haveListener = duk_has_prop_string(ctx, -1, DUK_HIDDEN_SYMBOL("INPUT_LISTENER"));

    if (duk_is_function(ctx, 0))
    {
        duk_pull(ctx, 0);
        duk_put_prop_string(ctx, -2, DUK_HIDDEN_SYMBOL("INPUT_LISTENER"));
        if (!haveListener)
        {
            inst->IncRefCount();
        }
        return 0;
    }
    else if (duk_is_undefined(ctx, 0))
    {
        if (haveListener)
        {
            duk_del_prop_string(ctx, -1, DUK_HIDDEN_SYMBOL("INPUT_LISTENER"));
            inst->DecRefCount();
        }
        return 0;
    }

    return ThrowInvalidArgsError(ctx);
}