#include <stdafx.h>
#include "../ScriptAPI.h"

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
        { "f32", F32 },
        { "f64", F64 },
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

jscb_id_t ScriptAPI::AddCallback(duk_context* ctx, jshook_id_t hookId, jscallback_t& callback)
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
    return bExists;
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
