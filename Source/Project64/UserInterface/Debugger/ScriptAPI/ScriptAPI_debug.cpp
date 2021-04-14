#include <stdafx.h>
#include "../ScriptAPI.h"
#include <Project64-core/Settings/DebugSettings.h>

void ScriptAPI::Define_debug(duk_context* ctx)
{
    const duk_function_list_entry funcs[] = {
        { "breakhere", js_debug_breakhere, 0 },
        { "skip", js_debug_skip, 0 },
        { "resume", js_debug_resume, 0 },
        { "showmemory", js_debug_showmemory, 1 },
        { "showcommands", js_debug_showcommands, 1 },
        { nullptr, nullptr, 0 }
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

duk_ret_t ScriptAPI::js_debug_skip(duk_context* /*ctx*/)
{
    g_Settings->SaveBool(Debugger_SkipOp, true);
    return 0;
}

duk_ret_t ScriptAPI::js_debug_showmemory(duk_context* ctx)
{
    if (!duk_is_number(ctx, 0))
    {
        return ThrowInvalidArgsError(ctx);
    }

    GetInstance(ctx)->Debugger()->Debug_ShowMemoryLocation(duk_get_uint(ctx, 0), true);
    return 0;
}

duk_ret_t ScriptAPI::js_debug_showcommands(duk_context* ctx)
{
    if (!duk_is_number(ctx, 0))
    {
        return ThrowInvalidArgsError(ctx);
    }

    GetInstance(ctx)->Debugger()->Debug_ShowCommandsLocation(duk_get_uint(ctx, 0), true);
    return 0;
}

duk_ret_t ScriptAPI::js_debug_resume(duk_context* ctx)
{
    g_Settings->SaveBool(Debugger_SteppingOps, false);
    if (CDebugSettings::WaitingForStep())
    {
        GetInstance(ctx)->Debugger()->StepEvent().Trigger();
    }
    return 0;
}