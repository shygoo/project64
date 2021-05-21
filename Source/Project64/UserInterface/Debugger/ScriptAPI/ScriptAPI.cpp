#include <stdafx.h>
#include <dwrite.h>
#include "../ScriptAPI.h"
#include "N64Image.h"

#pragma warning(disable: 4702) // disable unreachable code warning

#define MODULES_DIR "Scripts/modules"

void ScriptAPI::InitEnvironment(duk_context* ctx, CScriptInstance* inst)
{
    duk_module_duktape_init(ctx);
    duk_get_global_string(ctx, "Duktape");
    duk_push_c_function(ctx, js_Duktape_modSearch, 4);
    duk_put_prop_string(ctx, -2, "modSearch");
    duk_pop(ctx);

    duk_push_pointer(ctx, inst);
    duk_put_global_string(ctx, HSYM_INSTANCE);

    duk_push_object(ctx); // callbackId => { hookId, callbackId, function }
    duk_put_global_string(ctx, HSYM_APPCALLBACKS);

    duk_push_object(ctx); // fd => { fp }
    duk_put_global_string(ctx, HSYM_OPENFILES);

    Define_asm(ctx);
    Define_console(ctx);
    Define_cpu(ctx);
    Define_debug(ctx);
    Define_events(ctx);
    Define_fs(ctx);
    Define_mem(ctx);
    Define_pj64(ctx);
    Define_script(ctx);
    
    Define_alert(ctx);
    Define_exec(ctx);
    Define_RGBA(ctx);
    
    Define_AddressRange(ctx);
    Define_DrawingContext(ctx);
    Define_N64Image(ctx);
    //Define_Server(ctx);
    
    Define_Number_prototype_hex(ctx);
    DefineGlobalConstants(ctx);

    if(duk_get_top(ctx) > 0) 
    {
        inst->System()->Log("[SCRIPTSYS]: warning: duktape stack is dirty after API init");
    }
}

void ScriptAPI::DefineGlobalConstants(duk_context* ctx)
{
    const duk_number_list_entry numbers[] = {
        { "u8",   U8 },
        { "u16", U16 },
        { "u32", U32 },
        { "s8",   S8 },
        { "s16", S16 },
        { "s32", S32 },
        { "f32", F32 },
        { "f64", F64 },

        { "s64", S64 },
        { "u64", U64 },

        { "GPR_R0", GPR_R0 },
        { "GPR_AT", GPR_AT },
        { "GPR_V0", GPR_V0 },
        { "GPR_V1", GPR_V1 },
        { "GPR_A0", GPR_A0 },
        { "GPR_A1", GPR_A1 },
        { "GPR_A2", GPR_A2 },
        { "GPR_A3", GPR_A3 },
        { "GPR_T0", GPR_T0 },
        { "GPR_T1", GPR_T1 },
        { "GPR_T2", GPR_T2 },
        { "GPR_T3", GPR_T3 },
        { "GPR_T4", GPR_T4 },
        { "GPR_T5", GPR_T5 },
        { "GPR_T6", GPR_T6 },
        { "GPR_T7", GPR_T7 },
        { "GPR_S0", GPR_S0 },
        { "GPR_S1", GPR_S1 },
        { "GPR_S2", GPR_S2 },
        { "GPR_S3", GPR_S3 },
        { "GPR_S4", GPR_S4 },
        { "GPR_S5", GPR_S5 },
        { "GPR_S6", GPR_S6 },
        { "GPR_S7", GPR_S7 },
        { "GPR_T8", GPR_T8 },
        { "GPR_T9", GPR_T9 },
        { "GPR_K0", GPR_K0 },
        { "GPR_K1", GPR_K1 },
        { "GPR_GP", GPR_GP },
        { "GPR_SP", GPR_SP },
        { "GPR_FP", GPR_FP },
        { "GPR_RA", GPR_RA },
        //{ "GPR_S8", GPR_S8 },
        { "GPR_ANY", 0xFFFFFFFF },

        { "RDRAM_CONFIG_REG", 0xA3F00000 },
        { "RDRAM_DEVICE_TYPE_REG", 0xA3F00000 },
        { "RDRAM_DEVICE_ID_REG", 0xA3F00004 },
        { "RDRAM_DELAY_REG", 0xA3F00008 },
        { "RDRAM_MODE_REG", 0xA3F0000C },
        { "RDRAM_REF_INTERVAL_REG", 0xA3F00010 },
        { "RDRAM_REF_ROW_REG", 0xA3F00014 },
        { "RDRAM_RAS_INTERVAL_REG", 0xA3F00018 },
        { "RDRAM_MIN_INTERVAL_REG", 0xA3F0001C },
        { "RDRAM_ADDR_SELECT_REG", 0xA3F00020 },
        { "RDRAM_DEVICE_MANUF_REG", 0xA3F00024 },
        { "SP_MEM_ADDR_REG", 0xA4040000 },
        { "SP_DRAM_ADDR_REG", 0xA4040004 },
        { "SP_RD_LEN_REG", 0xA4040008 },
        { "SP_WR_LEN_REG", 0xA404000C },
        { "SP_STATUS_REG", 0xA4040010 },
        { "SP_DMA_FULL_REG", 0xA4040014 },
        { "SP_DMA_BUSY_REG", 0xA4040018 },
        { "SP_SEMAPHORE_REG", 0xA404001C },
        { "SP_PC_REG", 0xA4080000 },
        { "SP_IBIST_REG", 0xA4080004 },
        { "DPC_START_REG", 0xA4100000 },
        { "DPC_END_REG", 0xA4100004 },
        { "DPC_CURRENT_REG", 0xA4100008 },
        { "DPC_STATUS_REG", 0xA410000C },
        { "DPC_CLOCK_REG", 0xA4100010 },
        { "DPC_BUFBUSY_REG", 0xA4100014 },
        { "DPC_PIPEBUSY_REG", 0xA4100018 },
        { "DPC_TMEM_REG", 0xA410001C },
        { "DPS_TBIST_REG", 0xA4200000 },
        { "DPS_TEST_MODE_REG", 0xA4200004 },
        { "DPS_BUFTEST_ADDR_REG", 0xA4200008 },
        { "DPS_BUFTEST_DATA_REG", 0xA420000C },
        { "MI_INIT_MODE_REG", 0xA4300000 },
        { "MI_MODE_REG", 0xA4300000 },
        { "MI_VERSION_REG", 0xA4300004 },
        { "MI_NOOP_REG", 0xA4300004 },
        { "MI_INTR_REG", 0xA4300008 },
        { "MI_INTR_MASK_REG", 0xA430000C },
        { "VI_STATUS_REG", 0xA4400000 },
        { "VI_CONTROL_REG", 0xA4400000 },
        { "VI_ORIGIN_REG", 0xA4400004 },
        { "VI_DRAM_ADDR_REG", 0xA4400004 },
        { "VI_WIDTH_REG", 0xA4400008 },
        { "VI_H_WIDTH_REG", 0xA4400008 },
        { "VI_INTR_REG", 0xA440000C },
        { "VI_V_INTR_REG", 0xA440000C },
        { "VI_CURRENT_REG", 0xA4400010 },
        { "VI_V_CURRENT_LINE_REG", 0xA4400010 },
        { "VI_BURST_REG", 0xA4400014 },
        { "VI_TIMING_REG", 0xA4400014 },
        { "VI_V_SYNC_REG", 0xA4400018 },
        { "VI_H_SYNC_REG", 0xA440001C },
        { "VI_LEAP_REG", 0xA4400020 },
        { "VI_H_SYNC_LEAP_REG", 0xA4400020 },
        { "VI_H_START_REG", 0xA4400024 },
        { "VI_H_VIDEO_REG", 0xA4400024 },
        { "VI_V_START_REG", 0xA4400028 },
        { "VI_V_VIDEO_REG", 0xA4400028 },
        { "VI_V_BURST_REG", 0xA440002C },
        { "VI_X_SCALE_REG", 0xA4400030 },
        { "VI_Y_SCALE_REG", 0xA4400034 },
        { "AI_DRAM_ADDR_REG", 0xA4500000 },
        { "AI_LEN_REG", 0xA4500004 },
        { "AI_CONTROL_REG", 0xA4500008 },
        { "AI_STATUS_REG", 0xA450000C },
        { "AI_DACRATE_REG", 0xA4500010 },
        { "AI_BITRATE_REG", 0xA4500014 },
        { "PI_DRAM_ADDR_REG", 0xA4600000 },
        { "PI_CART_ADDR_REG", 0xA4600004 },
        { "PI_RD_LEN_REG", 0xA4600008 },
        { "PI_WR_LEN_REG", 0xA460000C },
        { "PI_STATUS_REG", 0xA4600010 },
        { "PI_BSD_DOM1_LAT_REG", 0xA4600014 },
        { "PI_BSD_DOM1_PWD_REG", 0xA4600018 },
        { "PI_BSD_DOM1_PGS_REG", 0xA460001C },
        { "PI_BSD_DOM1_RLS_REG", 0xA4600020 },
        { "PI_BSD_DOM2_LAT_REG", 0xA4600024 },
        { "PI_BSD_DOM2_PWD_REG", 0xA4600028 },
        { "PI_BSD_DOM2_PGS_REG", 0xA460002C },
        { "PI_BSD_DOM2_RLS_REG", 0xA4600030 },
        { "RI_MODE_REG", 0xA4700000 },
        { "RI_CONFIG_REG", 0xA4700004 },
        { "RI_CURRENT_LOAD_REG", 0xA4700008 },
        { "RI_SELECT_REG", 0xA470000C },
        { "RI_REFRESH_REG", 0xA4700010 },
        { "RI_COUNT_REG", 0xA4700010 },
        { "RI_LATENCY_REG", 0xA4700014 },
        { "RI_RERROR_REG", 0xA4700018 },
        { "RI_WERROR_REG", 0xA470001C },
        { "SI_DRAM_ADDR_REG", 0xA4800000 },
        { "SI_PIF_ADDR_RD64B_REG", 0xA4800004 },
        { "SI_PIF_ADDR_WR64B_REG", 0xA4800010 },
        { "SI_STATUS_REG", 0xA4800018 },

        { "PIF_ROM_START", 0xBFC00000 },
        { "PIF_RAM_START", 0xBFC007C0 },

        { "SP_DMEM_START", 0xA4000000 },
        { "SP_IMEM_START", 0xA4001000 },

        { "KUBASE", 0x00000000 },
        { "K0BASE", 0x80000000 },
        { "K1BASE", 0xA0000000 },
        { "K2BASE", 0xC0000000 },

        { "UT_VEC", 0x80000000 },
        { "R_VEC", 0xBFC00000 },
        { "XUT_VEC", 0x80000080 },
        { "ECC_VEC", 0x80000100 },
        { "E_VEC", 0x80000180 },
            
        { "M_GFXTASK", 1 },
        { "M_AUDTASK", 2 },
        { "OS_READ", 0 },
        { "OS_WRITE", 1 },
        
        { "COLOR_BLACK",   0x000000FF },
        { "COLOR_WHITE",   0xFFFFFFFF },
        { "COLOR_GRAY",    0x808080FF },
        { "COLOR_RED",     0xFF0000FF },
        { "COLOR_GREEN",   0x00FF00FF },
        { "COLOR_BLUE",    0x0000FFFF },
        { "COLOR_YELLOW",  0xFFFF00FF },
        { "COLOR_CYAN",    0x00FFFFFF },
        { "COLOR_MAGENTA", 0xFF00FFFF },

        { "IMG_I4", IMG_I4 },
        { "IMG_I8", IMG_I8 },
        { "IMG_IA4", IMG_IA4 },
        { "IMG_IA8", IMG_IA8 },
        { "IMG_IA16", IMG_IA16 },
        { "IMG_RGBA16", IMG_RGBA16 },
        { "IMG_RGBA32", IMG_RGBA32 },
        { "IMG_CI8_RGBA16", IMG_CI8_RGBA16 },
        { "IMG_CI4_RGBA16", IMG_CI4_RGBA16 },
        { "IMG_CI8_IA16", IMG_CI8_IA16 },
        { "IMG_CI4_IA16", IMG_CI4_IA16 },

        { "G_IM_FMT_RGBA", G_IM_FMT_RGBA },
        { "G_IM_FMT_YUV",  G_IM_FMT_YUV },
        { "G_IM_FMT_CI",   G_IM_FMT_CI },
        { "G_IM_FMT_IA",   G_IM_FMT_IA },
        { "G_IM_FMT_I",    G_IM_FMT_I },

        { "G_IM_SIZ_4b",  G_IM_SIZ_4b },
        { "G_IM_SIZ_8b",  G_IM_SIZ_8b },
        { "G_IM_SIZ_16b", G_IM_SIZ_16b },
        { "G_IM_SIZ_32b", G_IM_SIZ_32b },

        { "G_TT_NONE",    G_TT_NONE },
        { "G_TT_RGBA16",  G_TT_RGBA16 },
        { "G_TT_IA16",    G_TT_IA16 },

        { nullptr, 0 },
    };

    duk_push_global_object(ctx);
    duk_put_number_list(ctx, -1, numbers);
    duk_pop(ctx);
}

CScriptInstance* ScriptAPI::GetInstance(duk_context* ctx)
{
    duk_get_global_string(ctx, HSYM_INSTANCE);
    CScriptInstance* instance = (CScriptInstance*)duk_get_pointer(ctx, -1);
    duk_pop(ctx);
    return instance;
}

jscb_id_t ScriptAPI::AddCallback(duk_context* ctx, duk_idx_t callbackIdx, jshook_id_t hookId,
    jscond_fn_t fnCondition, jsargs_fn_t fnPushArgs, jsfn_t fnFinish)
{
    JSCallback cb(GetInstance(ctx), duk_get_heapptr(ctx, callbackIdx), fnCondition, fnPushArgs, fnFinish);
    return AddCallback(ctx, hookId, cb);
}

jscb_id_t ScriptAPI::AddCallback(duk_context* ctx, jshook_id_t hookId, JSCallback& callback)
{
    CScriptInstance* inst = GetInstance(ctx);
    jscb_id_t callbackId = inst->System()->RawAddCallback(hookId, callback);

    if(callbackId == JS_INVALID_CALLBACK)
    {
        inst->System()->Log("[SCRIPTSYS]: error: callback was not added");
        return JS_INVALID_CALLBACK;
    }

    duk_get_global_string(ctx, HSYM_APPCALLBACKS);

    duk_push_object(ctx);
    duk_push_number(ctx, hookId);
    duk_put_prop_string(ctx, -2, "hookId");
    duk_push_number(ctx, callbackId);
    duk_put_prop_string(ctx, -2, "callbackId");
    duk_push_heapptr(ctx, callback.heapptr);
    duk_put_prop_string(ctx, -2, "func");

    duk_push_c_function(ctx, CallbackFinalizer, 1);
    duk_set_finalizer(ctx, -2);

    duk_put_prop_index(ctx, -2, callbackId);

    duk_pop(ctx);

    inst->IncRefCount();

    return callbackId;
}

bool ScriptAPI::RemoveCallback(duk_context* ctx, jscb_id_t callbackId)
{
    duk_get_global_string(ctx, HSYM_APPCALLBACKS);
    duk_bool_t bExists = duk_has_prop_index(ctx, -1, callbackId);

    if(bExists)
    {
        // will invoke CallbackFinalizer
        duk_del_prop_index(ctx, -1, callbackId); 
    }

    duk_pop(ctx);
    return bExists != 0;
}

duk_ret_t ScriptAPI::CallbackFinalizer(duk_context* ctx)
{
    CScriptInstance* inst = ScriptAPI::GetInstance(ctx);

    duk_get_prop_string(ctx, 0, "hookId");
    duk_get_prop_string(ctx, 0, "callbackId");

    jshook_id_t hookId = (jshook_id_t)duk_get_uint(ctx, -2);
    jscb_id_t callbackId = (jscb_id_t)duk_get_uint(ctx, -1);
    duk_pop_n(ctx, 2);

    bool bRemoved = inst->System()->RawRemoveCallback(hookId, callbackId);

    if(bRemoved)
    {
        inst->DecRefCount();
    }
    
    return 0;
}

duk_ret_t ScriptAPI::js_Duktape_modSearch(duk_context* ctx)
{
    if (!duk_is_string(ctx, 0))
    {
        return ThrowInvalidArgsError(ctx);
    }

    const char* id = duk_get_string(ctx, 0);

    CFile file(stdstr_f(MODULES_DIR "/%s", id).c_str(), CFile::modeRead);

    if (!file.IsOpen())
    {
        return 0;
    }

    uint32_t length = file.GetLength();

    char* sourceCode = new char[length + 1];
    sourceCode[length] = '\0';

    if (file.Read(sourceCode, length) != length)
    {
        delete[] sourceCode;
        return 0;
    }

    duk_push_string(ctx, sourceCode);
    delete[] sourceCode;
    return 1;
}

duk_ret_t ScriptAPI::ThrowInvalidArgsError(duk_context* ctx)
{
    duk_push_error_object(ctx, DUK_ERR_TYPE_ERROR, "invalid argument(s)");
    return duk_throw(ctx);  
}

duk_ret_t ScriptAPI::ThrowInvalidAssignmentError(duk_context* ctx)
{
    duk_push_error_object(ctx, DUK_ERR_TYPE_ERROR, "invalid assignment");
    return duk_throw(ctx);
}

duk_ret_t ScriptAPI::ThrowNotCallableError(duk_context* ctx)
{
    duk_push_error_object(ctx, DUK_ERR_TYPE_ERROR, "not callable");
    return duk_throw(ctx);
}

void ScriptAPI::AllowPrivateCall(duk_context* ctx, bool bAllow)
{
    duk_push_boolean(ctx, (duk_bool_t)bAllow);
    duk_put_global_string(ctx, HSYM_PRIVATECALL);
}

bool ScriptAPI::PrivateCallAllowed(duk_context* ctx)
{
    if (!duk_get_global_string(ctx, HSYM_PRIVATECALL))
    {
        return false;
    }

    bool bAllowed = duk_get_boolean(ctx, -1);
    duk_pop(ctx);
    return bAllowed;
}

duk_ret_t ScriptAPI::js_DummyConstructor(duk_context* ctx)
{
    return ThrowNotCallableError(ctx);
}

void ScriptAPI::PushNewDummyConstructor(duk_context* ctx, bool bFrozen)
{
    duk_push_c_function(ctx, js_DummyConstructor, 0);
    duk_push_object(ctx);
    duk_put_prop_string(ctx, -2, "prototype");

    if (bFrozen)
    {
        duk_freeze(ctx, -1);
    }
}

void ScriptAPI::DefineGlobalDummyConstructors(duk_context* ctx, const char* constructorNames[], bool bFreeze)
{
    duk_push_global_object(ctx);

    for (size_t i = 0;; i++)
    {
        if (constructorNames[i] == nullptr)
        {
            break;
        }
        duk_push_string(ctx, constructorNames[i]);
        PushNewDummyConstructor(ctx, bFreeze);
        duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_ENUMERABLE);
    }

    duk_pop(ctx);
}

void ScriptAPI::SetDummyConstructor(duk_context* ctx, duk_idx_t obj_idx, const char* globalConstructorName)
{
    obj_idx = duk_normalize_index(ctx, obj_idx);
    duk_get_global_string(ctx, globalConstructorName);
    duk_get_prop_string(ctx, -1, "prototype");
    duk_set_prototype(ctx, obj_idx);
    duk_pop(ctx);
}
