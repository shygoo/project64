#include <stdafx.h>
#include "../ScriptAPI.h"
#include <Project64/UserInterface/DiscordRPC.h>

static void CpuRunningChanged(void *data);

void ScriptAPI::Define_pj64(duk_context* ctx)
{
    const duk_function_list_entry funcs[] = {
        { "open", js_pj64_open, 1 },
        { "close", js_pj64_close, 0 },
        //{ "savestate", js_pj64_savestate, 1 },
        //{ "loadstate", js_pj64_loadstate, 1 }
        { nullptr, nullptr, 0 }
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
    if (duk_get_top(ctx) != 1 || !duk_is_string(ctx, 0))
    {
        return ThrowInvalidArgsError(ctx);
    }

    const char* romPath = duk_get_string(ctx, 0);

    SyncEvent cpuRunningChangedEvent;
    g_Settings->RegisterChangeCB(GameRunning_CPU_Running, &cpuRunningChangedEvent, CpuRunningChanged);

    bool oldAutoStartSetting = g_Settings->LoadBool(Setting_AutoStart);
    g_Settings->SaveBool(Setting_AutoStart, true);
    bool bLoaded = false;

    stdstr ext = CPath(romPath).GetExtension();
    if ((_stricmp(ext.c_str(), "ndd") != 0) && (_stricmp(ext.c_str(), "d64") != 0))
    {
        bLoaded = g_BaseSystem->RunFileImage(romPath);
    }
    else
    {
        bLoaded = g_BaseSystem->RunDiskImage(romPath);
    }

    if (bLoaded)
    {
        cpuRunningChangedEvent.IsTriggered(SyncEvent::INFINITE_TIMEOUT);
        g_Settings->UnregisterChangeCB(GameRunning_CPU_Running, &cpuRunningChangedEvent, CpuRunningChanged);
    }

    g_Settings->SaveBool(Setting_AutoStart, oldAutoStartSetting);
    duk_push_boolean(ctx, bLoaded);
    return 1;
}

duk_ret_t ScriptAPI::js_pj64_close(duk_context* /*ctx*/)
{
    if (g_BaseSystem)
    {
        // TODO: this is buggy
        g_BaseSystem->ExternalEvent(SysEvent_CloseCPU);
    }

    if (UISettingsLoadBool(Setting_EnableDiscordRPC))
    {
        CDiscord::Update(false);
    }

    return 0;
}

/*
TODO: Can't implement these because CN64System::SaveState() doesn't allow arbitrary paths

duk_ret_t ScriptAPI::js_pj64_savestate(duk_context* ctx)
{
    if (!duk_is_string(ctx, 0))
    {
        return ThrowInvalidArgsError(ctx);
    }

    const char* path = duk_get_string(ctx, 0);
    g_Settings->SaveString(GameRunning_InstantSaveFile, path);
    g_BaseSystem->ExternalEvent(SysEvent_SaveMachineState);
    return 0;
}

duk_ret_t ScriptAPI::js_pj64_loadstate(duk_context* ctx)
{
    return 0;
}
*/

static void CpuRunningChanged(void *data)
{
    SyncEvent* cpuRunningChangedEvent = (SyncEvent*)data;
    if (g_Settings->LoadBool(GameRunning_CPU_Running))
    {
        cpuRunningChangedEvent->Trigger();
    }
}
