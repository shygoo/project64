#include <stdafx.h>
#include "../ScriptAPI.h"

static bool CbCond_CpuStep_ReadAddrBetween(jscallback_t* callback, void* condParam);
static bool CbCond_CpuStep_WriteAddrBetween(jscallback_t* callback, void* condParam);
static bool CbCond_CpuStep_PcBetween(jscallback_t* callback, void* condParam);
static bool CbCond_CpuStep_Opcode(jscallback_t* callback, void* condParam);
static bool CbCond_CpuStep_GprValue(jscallback_t* callback, void* condParam);

static duk_idx_t CbArgs_CpuStep_Pc(duk_context* ctx, void* argsParam);
static duk_idx_t CbArgs_CpuStep_Pc_ReadWriteAddr(duk_context* ctx, void* argsParam);
static duk_idx_t CbArgs_CpuStep_Pc_AffectedRegIndex(duk_context* ctx, void* argsParam);

static bool GetAddressOrAddressRange(duk_context* ctx, duk_idx_t idx, uint32_t* addrStart, uint32_t *addrEnd);

void ScriptAPI::Define_events(duk_context* ctx)
{
    const duk_function_list_entry funcs[] = {
        { "onexec",     js_events_onexec, 2 },
        { "onread",     js_events_onread, 2 },
        { "onwrite",    js_events_onwrite, 2 },
        { "ondraw",     js_events_ondraw, 1 },
        { "ongprvalue", js_events_ongprvalue, 4 },
        { "onopcode",   js_events_onopcode, DUK_VARARGS },
        { "remove",     js_events_remove, 1 },
        { NULL, NULL, 0 }
    };

    duk_push_global_object(ctx);
    duk_push_string(ctx, "events");
    duk_push_object(ctx);
    duk_put_function_list(ctx, -1, funcs);
    duk_freeze(ctx, -1);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_ENUMERABLE);
    duk_pop(ctx);
}

duk_ret_t ScriptAPI::js_events_onexec(duk_context* ctx)
{
    if(duk_get_top(ctx) != 2)
    {
        return DUK_RET_ERROR;
    }

    uint32_t addrStart, addrEnd;

    if(!GetAddressOrAddressRange(ctx, 0, &addrStart, &addrEnd))
    {
        return ThrowInvalidArgsError(ctx);
    }

    void* heapptr = duk_get_heapptr(ctx, 1);

    jscallback_t callback = {};
    callback.inst = GetInstance(ctx);
    callback.heapptr = heapptr;
    callback.fnPushArgs = CbArgs_CpuStep_Pc;
    callback.fnCondition = CbCond_CpuStep_PcBetween;
    callback.cond.addrStart = addrStart;
    callback.cond.addrEnd = addrEnd;

    jscb_id_t callbackId = AddCallback(ctx, JS_HOOK_CPUSTEP, callback);

    duk_pop_n(ctx, 2);
    duk_push_uint(ctx, callbackId);
    return 1;
}

duk_ret_t ScriptAPI::js_events_onread(duk_context* ctx){ return 0; }
duk_ret_t ScriptAPI::js_events_onwrite(duk_context* ctx){ return 0; }
duk_ret_t ScriptAPI::js_events_onopcode(duk_context* ctx){ return 0; }
duk_ret_t ScriptAPI::js_events_ongprvalue(duk_context* ctx){ return 0; }
duk_ret_t ScriptAPI::js_events_ondraw(duk_context* ctx){ return 0; }

duk_ret_t ScriptAPI::js_events_remove(duk_context* ctx)
{
    jscb_id_t callbackId = (jscb_id_t)duk_get_uint(ctx, -1);
    duk_pop(ctx);

    bool bRemoved = RemoveCallback(ctx, callbackId);

    duk_push_boolean(ctx, bRemoved);
    return 1;
}

// onread
bool CbCond_CpuStep_ReadAddrBetween(jscallback_t* callback, void* _env)
{
    jshook_env_cpustep_t* env = (jshook_env_cpustep_t*)_env;

    if(!env->bReadOp)
    {
        return false;
    }

    return (env->readWriteAddr >= callback->cond.addrStart &
            env->readWriteAddr <= callback->cond.addrEnd);
}

// onwrite
bool CbCond_CpuStep_WriteAddrBetween(jscallback_t* callback, void* _env)
{
    jshook_env_cpustep_t* env = (jshook_env_cpustep_t*)_env;

    if(!env->bWriteOp)
    {
        return false;
    }

    return (env->readWriteAddr >= callback->cond.addrStart &
            env->readWriteAddr <= callback->cond.addrEnd);
}

// onexec
bool CbCond_CpuStep_PcBetween(jscallback_t* callback, void* _env)
{
    jshook_env_cpustep_t* env = (jshook_env_cpustep_t*)_env;
    return (env->pc >= callback->cond.addrStart &
            env->pc <= callback->cond.addrEnd);
}

// onopcode
bool CbCond_CpuStep_Opcode(jscallback_t* callback, void* _env)
{
    jshook_env_cpustep_t* env = (jshook_env_cpustep_t*)_env;
    return callback->cond.opcode == (env->opcode & callback->cond.opcodeMask);
}

// ongprvalue
bool CbCond_CpuStep_GprValue(jscallback_t* callback, void* _env)
{
    jshook_env_cpustep_t* env = (jshook_env_cpustep_t*)_env;

    for(int i = 0; i < 32; i++)
    {
        if(callback->cond.regIndices & (1 << i))
        {
            if(env->gpr[i] == callback->cond.regValue)
            {
                env->outAffectedRegIndex = i;
                return true;
            }
        }
    }

    return false;
}

// onexec
duk_idx_t CbArgs_CpuStep_Pc(duk_context* ctx, void* _env)
{
    jshook_env_cpustep_t* env = (jshook_env_cpustep_t*)_env;
    duk_push_uint(ctx, env->pc);
    return 1;
}

// onread, onwrite
duk_idx_t CbArgs_CpuStep_Pc_ReadWriteAddr(duk_context* ctx, void* _env)
{
    jshook_env_cpustep_t* env = (jshook_env_cpustep_t*)_env;
    duk_push_uint(ctx, env->pc);
    duk_push_uint(ctx, env->readWriteAddr);
    return 2;
}

// ongprvalue
duk_idx_t CbArgs_CpuStep_Pc_AffectedRegIndex(duk_context* ctx, void* _env)
{
    jshook_env_cpustep_t* env = (jshook_env_cpustep_t*)_env;
    duk_push_uint(ctx, env->pc);
    duk_push_uint(ctx, env->outAffectedRegIndex);
    return 2;
}

bool GetAddressOrAddressRange(duk_context* ctx, duk_idx_t idx, uint32_t* addrStart, uint32_t *addrEnd)
{
    if(duk_is_number(ctx, idx))
    {
        uint32_t addr = duk_get_uint(ctx, idx);
        *addrStart = addr;
        *addrEnd = addr;
        return true;
    }

    if(duk_is_object(ctx, idx))
    {
        if(!duk_has_prop_string(ctx, idx, "start") ||
           !duk_has_prop_string(ctx, idx, "end"))
        {
            return false;
        }

        duk_get_prop_string(ctx, idx, "start");
        duk_get_prop_string(ctx, idx, "end");

        if(!duk_is_number(ctx, -2) ||
           !duk_is_number(ctx, -1))
        {
            duk_pop_n(ctx, 2);
            return false;
        }

        *addrStart = duk_get_uint(ctx, -2);
        *addrEnd = duk_get_uint(ctx, -1);
        duk_pop_n(ctx, 2);
        return true;
    }

    return false;
}
