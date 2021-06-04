#include <stdafx.h>
#include "../ScriptAPI.h"
#include <Project64/UserInterface/DiscordRPC.h>

using namespace ScriptAPI;

static void CpuRunningChanged(void *data);
static void Define_rominfo(duk_context* ctx, duk_idx_t obj_idx);

void ScriptAPI::Define_pj64(duk_context* ctx)
{
    const duk_function_list_entry funcs[] = {
        { "open", js_pj64_open, DUK_VARARGS },
        { "close", js_pj64_close, DUK_VARARGS },
        //{ "savestate", js_pj64_savestate, DUK_VARARGS },
        //{ "loadstate", js_pj64_loadstate, DUK_VARARGS }
        { nullptr, nullptr, 0 }
    };

    duk_push_global_object(ctx);
    duk_push_string(ctx, "pj64");
    duk_push_object(ctx);
    duk_put_function_list(ctx, -1, funcs);

    Define_rominfo(ctx, -1);

    duk_freeze(ctx, -1);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_ENUMERABLE);
    duk_pop(ctx);
}

void Define_rominfo(duk_context* ctx, duk_idx_t obj_idx)
{
    obj_idx = duk_normalize_index(ctx, obj_idx);

    static const DukPropListEntry props[] = {
        { "loaded", DukGetter(js_pj64_romInfo_loaded) },
        { "goodName", DukGetter(js_pj64_romInfo_goodName) },
        { "fileName", DukGetter(js_pj64_romInfo_fileName) },
        { "filePath", DukGetter(js_pj64_romInfo_filePath) },
        { "headerCrc1", DukGetter(js_pj64_romInfo_headerCrc1) },
        { "headerCrc2", DukGetter(js_pj64_romInfo_headerCrc2) },
        { "headerName", DukGetter(js_pj64_romInfo_headerName) },
        { "headerMediaFormat", DukGetter(js_pj64_romInfo_headerMediaFormat) },
        { "headerId", DukGetter(js_pj64_romInfo_headerId) },
        { "headerCountryCode", DukGetter(js_pj64_romInfo_headerCountryCode) },
        { "headerVersion", DukGetter(js_pj64_romInfo_headerVersion) },
        { nullptr }
    };

    duk_push_string(ctx, "rominfo");
    duk_push_object(ctx);
    DukPutPropList(ctx, -1, props);
    duk_freeze(ctx, -1);
    duk_def_prop(ctx, obj_idx, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_ENUMERABLE);
}

duk_ret_t ScriptAPI::js_pj64_open(duk_context* ctx)
{
    CheckArgs(ctx, { Arg_String });

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
    CheckArgs(ctx, { Arg_String });

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

duk_ret_t ScriptAPI::js_pj64_romInfo_loaded(duk_context* ctx)
{
    duk_push_boolean(ctx, (duk_bool_t)g_Settings->LoadBool(GameRunning_CPU_Running));
    return 1;
}

duk_ret_t ScriptAPI::js_pj64_romInfo_goodName(duk_context* ctx)
{
    if (!g_Settings->LoadBool(GameRunning_CPU_Running))
    {
        duk_push_string(ctx, "");
        return 1;
    }

    duk_push_string(ctx, g_Settings->LoadStringVal(Rdb_GoodName).c_str());
    return 1;
}

duk_ret_t ScriptAPI::js_pj64_romInfo_fileName(duk_context* ctx)
{
    if (!g_Settings->LoadBool(GameRunning_CPU_Running))
    {
        duk_push_string(ctx, "");
        return 1;
    }

    duk_push_string(ctx, CPath(g_Settings->LoadStringVal(Game_File)).GetNameExtension().c_str());
    return 1;
}

duk_ret_t ScriptAPI::js_pj64_romInfo_filePath(duk_context* ctx)
{
    if (!g_Settings->LoadBool(GameRunning_CPU_Running))
    {
        duk_push_string(ctx, "");
        return 1;
    }

    duk_push_string(ctx, g_Settings->LoadStringVal(Game_File).c_str());
    return 1;
}

duk_ret_t ScriptAPI::js_pj64_romInfo_headerCrc1(duk_context* ctx)
{
    if (!g_Settings->LoadBool(GameRunning_CPU_Running))
    {
        duk_push_uint(ctx, 0);
        return 1;
    }

    CDebuggerUI* debugger = GetInstance(ctx)->Debugger();
    uint32_t crc1 = 0;
    debugger->DebugLoad_VAddr<uint32_t>(0xB0000010, crc1);
    duk_push_uint(ctx, crc1);
    return 1;
}

duk_ret_t ScriptAPI::js_pj64_romInfo_headerCrc2(duk_context* ctx)
{
    if (!g_Settings->LoadBool(GameRunning_CPU_Running))
    {
        duk_push_uint(ctx, 0);
        return 1;
    }

    CDebuggerUI* debugger = GetInstance(ctx)->Debugger();
    uint32_t crc2 = 0;
    debugger->DebugLoad_VAddr<uint32_t>(0xB0000014, crc2);
    duk_push_uint(ctx, crc2);
    return 1;
}

duk_ret_t ScriptAPI::js_pj64_romInfo_headerName(duk_context* ctx)
{
    if (!g_Settings->LoadBool(GameRunning_CPU_Running))
    {
        duk_push_string(ctx, "");
        return 1;
    }

    CDebuggerUI* debugger = GetInstance(ctx)->Debugger();
    char headerName[0x15] = "";
    for (size_t i = 0; i < 0x14; i++)
    {
        debugger->DebugLoad_VAddr<char>(0xB0000020 + i, headerName[i]);
    }
    duk_push_string(ctx, headerName);
    duk_trim(ctx, -1);
    return 1;
}

duk_ret_t ScriptAPI::js_pj64_romInfo_headerMediaFormat(duk_context* ctx)
{
    if (!g_Settings->LoadBool(GameRunning_CPU_Running))
    {
        duk_push_uint(ctx, 0);
        return 1;
    }

    CDebuggerUI* debugger = GetInstance(ctx)->Debugger();
    uint32_t mediaFormat = 0;
    debugger->DebugLoad_VAddr<uint32_t>(0xB0000038, mediaFormat);
    duk_push_uint(ctx, mediaFormat);
    return 1;
}

duk_ret_t ScriptAPI::js_pj64_romInfo_headerId(duk_context* ctx)
{
    if (!g_Settings->LoadBool(GameRunning_CPU_Running))
    {
        duk_push_string(ctx, "");
        return 1;
    }

    CDebuggerUI* debugger = GetInstance(ctx)->Debugger();
    char headerId[3] = "";
    debugger->DebugLoad_VAddr<char>(0xB000003C, headerId[0]);
    debugger->DebugLoad_VAddr<char>(0xB000003D, headerId[1]);
    duk_push_string(ctx, headerId);
    return 1;
}

duk_ret_t ScriptAPI::js_pj64_romInfo_headerCountryCode(duk_context* ctx)
{
    if (!g_Settings->LoadBool(GameRunning_CPU_Running))
    {
        duk_push_string(ctx, "");
        return 1;
    }

    CDebuggerUI* debugger = GetInstance(ctx)->Debugger();
    char countryCode[2] = "";
    debugger->DebugLoad_VAddr<char>(0xB000003E, countryCode[0]);
    duk_push_string(ctx, countryCode);
    return 1;
}

duk_ret_t ScriptAPI::js_pj64_romInfo_headerVersion(duk_context* ctx)
{
    if (!g_Settings->LoadBool(GameRunning_CPU_Running))
    {
        duk_push_uint(ctx, 0);
        return 1;
    }

    CDebuggerUI* debugger = GetInstance(ctx)->Debugger();
    uint8_t headerVersion = 0;
    debugger->DebugLoad_VAddr<uint8_t>(0xB000003F, headerVersion);
    duk_push_uint(ctx, headerVersion);
    return 1;
}

static void CpuRunningChanged(void *data)
{
    SyncEvent* cpuRunningChangedEvent = (SyncEvent*)data;
    if (g_Settings->LoadBool(GameRunning_CPU_Running))
    {
        cpuRunningChangedEvent->Trigger();
    }
}
