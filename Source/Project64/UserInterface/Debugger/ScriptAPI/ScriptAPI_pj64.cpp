#include <stdafx.h>
#include "../ScriptAPI.h"

void ScriptAPI::Define_pj64(duk_context* ctx)
{
    const duk_function_list_entry funcs[] = {
        { "open",  js_pj64_open,  1 },
        { "close", js_pj64_close, 0 },
        { NULL, NULL, 0 }
    };

    duk_push_global_object(ctx);
    duk_push_string(ctx, "pj64");
    duk_push_object(ctx);
    duk_put_function_list(ctx, -1, funcs);
    duk_freeze(ctx, -1);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_ENUMERABLE);
    duk_pop(ctx);
}

duk_ret_t ScriptAPI::js_pj64_open(duk_context* ctx)
{
    CScriptInstance* inst = GetInstance(ctx);

    if (duk_get_top(ctx) != 1 || !duk_is_string(ctx, 0))
    {
        return ThrowInvalidArgsError(ctx);
    }

    const char* romPath = duk_get_string(ctx, 0);

    stdstr ext = CPath(romPath).GetExtension();
    if ((_stricmp(ext.c_str(), "ndd") != 0) && (_stricmp(ext.c_str(), "d64") != 0))
    {
        if (!g_BaseSystem->RunFileImage(romPath))
        {
            return DUK_RET_ERROR;
        }
    }
    else
    {
        if (!g_BaseSystem->RunDiskImage(romPath))
        {
            return DUK_RET_ERROR;
        }
    }

    // TODO: Block until rom is actually loaded

    return 0;
}

duk_ret_t ScriptAPI::js_pj64_close(duk_context* ctx)
{
    if (g_BaseSystem)
    {
        g_BaseSystem->CloseCpu();
    }
    //m_Gui->SaveWindowLoc();
    //if (UISettingsLoadBool(Setting_EnableDiscordRPC))
    //{
    //    CDiscord::Update(false);
    //}

    // TODO: Block until rom is actually closed

    return 0;
}
