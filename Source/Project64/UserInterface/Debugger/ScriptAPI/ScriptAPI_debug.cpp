#include <stdafx.h>
#include "../ScriptAPI.h"
#include <Project64-core/Settings/DebugSettings.h>

void ScriptAPI::Define_debug(duk_context* ctx)
{
    const duk_function_list_entry funcs[] = {
        { "breakhere", js_debug_breakhere, DUK_VARARGS },
        { "step", js_debug_step, DUK_VARARGS },
        { "skip", js_debug_skip, DUK_VARARGS },
        { "resume", js_debug_resume, DUK_VARARGS },
        { "showmemory", js_debug_showmemory, DUK_VARARGS },
        { "showcommands", js_debug_showcommands, DUK_VARARGS },
        { nullptr, nullptr, 0 }
    };

    duk_push_global_object(ctx);
    duk_push_string(ctx, "debug");
    duk_push_object(ctx);
    
    duk_push_string(ctx, "paused");
    duk_push_c_function(ctx, js_debug__get_paused, 0);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_GETTER);

    duk_put_function_list(ctx, -1, funcs);
    duk_freeze(ctx, -1);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_ENUMERABLE);
    duk_pop(ctx);
}

duk_ret_t ScriptAPI::js_debug_breakhere(duk_context* ctx)
{
    CheckArgs(ctx, { Arg_Boolean });

    if (duk_get_top(ctx) > 0 && duk_get_boolean(ctx, 0) == 1)
    {
        g_Settings->SaveBool(Debugger_SilentBreak, true);
    }
    g_Settings->SaveBool(Debugger_SteppingOps, true);
    return 0;
}

duk_ret_t ScriptAPI::js_debug_step(duk_context* ctx)
{
    CheckArgs(ctx, {});

    if (g_Settings->LoadBool(Debugger_SteppingOps) && 
        CDebugSettings::WaitingForStep())
    {
        g_Settings->SaveBool(Debugger_SilentBreak, true);
        GetInstance(ctx)->Debugger()->StepEvent().Trigger();
    }
    return 0;
}

duk_ret_t ScriptAPI::js_debug_skip(duk_context* ctx)
{
    CheckArgs(ctx, {});

    g_Settings->SaveBool(Debugger_SkipOp, true);

    if (g_Settings->LoadBool(Debugger_SteppingOps) &&
        CDebugSettings::WaitingForStep())
    {
        GetInstance(ctx)->Debugger()->StepEvent().Trigger();
    }
    return 0;
}

duk_ret_t ScriptAPI::js_debug_showmemory(duk_context* ctx)
{
    CheckArgs(ctx, { Arg_Number, Arg_OptBoolean });

    uint32_t address = duk_get_uint(ctx, 0);
    bool bPhysical = duk_get_boolean_default(ctx, 1, 0) ? true : false;

    GetInstance(ctx)->Debugger()->Debug_ShowMemoryLocation(duk_get_uint(ctx, 0), !bPhysical);
    return 0;
}

duk_ret_t ScriptAPI::js_debug_showcommands(duk_context* ctx)
{
    CheckArgs(ctx, { Arg_Number });
    GetInstance(ctx)->Debugger()->Debug_ShowCommandsLocation(duk_get_uint(ctx, 0), true);
    return 0;
}

duk_ret_t ScriptAPI::js_debug_resume(duk_context* ctx)
{
    CheckArgs(ctx, {});

    g_Settings->SaveBool(Debugger_SteppingOps, false);

    if (CDebugSettings::WaitingForStep())
    {
        GetInstance(ctx)->Debugger()->StepEvent().Trigger();
    }
    return 0;
}

duk_ret_t ScriptAPI::js_debug__get_paused(duk_context* ctx)
{
    duk_push_boolean(ctx, CDebugSettings::WaitingForStep() && g_Settings->LoadBool(Debugger_SteppingOps));
    return 1;
}