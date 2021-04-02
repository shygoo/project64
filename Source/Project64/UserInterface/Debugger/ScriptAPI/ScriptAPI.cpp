#include <stdafx.h>
#include "../ScriptAPI.h"

#pragma warning(disable: 4702)

void ScriptAPI::InitEnvironment(duk_context *ctx, CScriptInstance* inst)
{
    duk_push_global_object(ctx);

    duk_push_pointer(ctx, inst);
    duk_put_prop_string(ctx, -2, DUK_HIDDEN_SYMBOL("INSTANCE"));
    duk_push_object(ctx); // callbackId => { hookId, callbackId, function }
    duk_put_prop_string(ctx, -2, DUK_HIDDEN_SYMBOL("APPCALLBACKS"));
    duk_push_object(ctx); // fd => { fp }
    duk_put_prop_string(ctx, -2, DUK_HIDDEN_SYMBOL("FILES"));

    duk_pop(ctx);

    Define_script(ctx);
    Define_console(ctx);
    Define_events(ctx);
    Define_mem(ctx);
    Define_fs(ctx);
    Define_AddressRange(ctx);
    Define_asm(ctx);
    Define_registers(ctx);
    Define_debug(ctx);

    //Define_Server(ctx);
    Define_Number_prototype_hex(ctx);
    DefineGlobalConstants(ctx);

    if(duk_get_top(ctx) > 0) 
    {
        printf("[ScriptSys]: warning: duk stack dirty after API init\n");
    }
}

void ScriptAPI::DefineGlobalConstants(duk_context *ctx)
{
    const duk_number_list_entry numbers[] = {
        { "u8",   U8 },
        { "u16", U16 },
        { "u32", U32 },
        { "s8",   S8 },
        { "s16", S16 },
        { "s32", S32 },
        { "float", F32 },
        { "double", F64 },

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
        { NULL, 0 },
    };

    duk_push_global_object(ctx);
    duk_put_number_list(ctx, -1, numbers);
    duk_pop(ctx);
}

CScriptInstance* ScriptAPI::GetInstance(duk_context *ctx)
{
    duk_push_global_object(ctx);
    duk_get_prop_string(ctx, -1, DUK_HIDDEN_SYMBOL("INSTANCE"));
    CScriptInstance* instance = (CScriptInstance*)duk_get_pointer(ctx, -1);
    duk_pop_n(ctx, 2);
    return instance;
}

jscb_id_t ScriptAPI::AddCallback(duk_context* ctx, jshook_id_t hookId, JSCallback& callback)
{
    CScriptInstance* inst = GetInstance(ctx);
    jscb_id_t callbackId = inst->System()->RawAddCallback(hookId, callback);

    if(callbackId == JS_INVALID_CALLBACK)
    {
        printf("invalid callback id was generated\n");
        return JS_INVALID_CALLBACK;
    }

    duk_push_global_object(ctx);
    duk_get_prop_string(ctx, -1, DUK_HIDDEN_SYMBOL("APPCALLBACKS"));

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

    duk_pop_n(ctx, 2);

    inst->IncRefCount();

    return callbackId;
}

bool ScriptAPI::RemoveCallback(duk_context* ctx, jscb_id_t callbackId)
{
    duk_push_global_object(ctx);
    duk_get_prop_string(ctx, -1, DUK_HIDDEN_SYMBOL("APPCALLBACKS"));
    duk_bool_t bExists = duk_has_prop_index(ctx, -1, callbackId);

    if(bExists)
    {
        // will invoke CallbackFinalizer
        duk_del_prop_index(ctx, -1, callbackId); 
    }

    duk_pop_n(ctx, 2);
    return bExists != 0;
}

duk_ret_t ScriptAPI::CallbackFinalizer(duk_context *ctx)
{
    CScriptInstance *inst = ScriptAPI::GetInstance(ctx);

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

duk_ret_t ScriptAPI::ThrowInvalidArgsError(duk_context* ctx)
{
    duk_push_error_object(ctx, DUK_ERR_TYPE_ERROR, "invalid arguments");
    return duk_throw(ctx);  
}
