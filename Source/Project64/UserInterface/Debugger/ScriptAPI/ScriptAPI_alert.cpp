#include <stdafx.h>
#include <windows.h>
#include "../ScriptAPI.h"

void ScriptAPI::Define_alert(duk_context* ctx)
{
    duk_push_global_object(ctx);
    duk_push_string(ctx, "alert");
    duk_push_c_function(ctx, js_alert, DUK_VARARGS);
    duk_freeze(ctx, -1);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_ENUMERABLE);
    duk_pop(ctx);
}

duk_ret_t ScriptAPI::js_alert(duk_context* ctx)
{
    CheckArgs(ctx, { Arg_Any, Arg_OptAny });
    duk_idx_t nargs = duk_get_top(ctx);

    const char* message = duk_safe_to_string(ctx, 0);
    const char* caption = (nargs == 2) ? duk_safe_to_string(ctx, 1) : "";

    HWND mainWindow = (HWND)g_Plugins->MainWindow()->GetWindowHandle();
    MessageBoxA(mainWindow, message, caption, MB_OK);
    return 0;
}
