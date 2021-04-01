#include <stdafx.h>
#include "../ScriptAPI.h"

#pragma warning(disable: 4702)

static bool CbCond_CpuStep_ReadAddrBetween(JSCallback* cb, void* env);
static bool CbCond_CpuStep_WriteAddrBetween(JSCallback* cb, void* env);
static bool CbCond_CpuStep_PcBetween(JSCallback* cb, void* env);
static bool CbCond_CpuStep_PcBetween_Opcode(JSCallback* cb, void* env);
static bool CbCond_CpuStep_PcBetween_GprValue(JSCallback* cb, void* env);

static duk_idx_t CbArgs_CpuStep_ReadWriteAddr(duk_context* ctx, void* env);
static duk_idx_t CbArgs_CpuStep_AffectedRegIndex(duk_context* ctx, void* env);

static bool GetAddressOrAddressRange(duk_context* ctx, duk_idx_t idx, uint32_t* addrStart, uint32_t *addrEnd);

static duk_ret_t ThrowNeedInterpreterError(duk_context* ctx);
static bool HaveInterpreter();

void ScriptAPI::Define_events(duk_context* ctx)
{
    const duk_function_list_entry funcs[] = {
        { "onexec",     js_events_onexec, 2 },
        { "onread",     js_events_onread, 2 },
        { "onwrite",    js_events_onwrite, 2 },
        { "ongprvalue", js_events_ongprvalue, 4 },
        { "onopcode",   js_events_onopcode, DUK_VARARGS },
        { "ondraw",     js_events_ondraw, 1 },
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
    if (!HaveInterpreter())
    {
        return ThrowNeedInterpreterError(ctx);
    }

    uint32_t addrStart, addrEnd;
    if(duk_get_top(ctx) != 2 ||
       !GetAddressOrAddressRange(ctx, 0, &addrStart, &addrEnd) ||
       !duk_is_function(ctx, 1))
    {
        return ThrowInvalidArgsError(ctx);
    }

    JSCallback cb(GetInstance(ctx), duk_get_heapptr(ctx, 1),
        CbCond_CpuStep_PcBetween, NULL);
    cb.params.addrStart = addrStart;
    cb.params.addrEnd = addrEnd;

    jscb_id_t callbackId = AddCallback(ctx, JS_HOOK_CPUSTEP, cb);

    duk_push_uint(ctx, callbackId);
    return 1;
}

duk_ret_t ScriptAPI::js_events_onread(duk_context* ctx)
{
    if (!HaveInterpreter())
    {
        return ThrowNeedInterpreterError(ctx);
    }

    uint32_t addrStart, addrEnd;
    if (duk_get_top(ctx) != 2 ||
        !GetAddressOrAddressRange(ctx, 0, &addrStart, &addrEnd) ||
        !duk_is_function(ctx, 1))
    {
        return ThrowInvalidArgsError(ctx);
    }

    JSCallback cb(GetInstance(ctx), duk_get_heapptr(ctx, 1),
        CbCond_CpuStep_ReadAddrBetween, CbArgs_CpuStep_ReadWriteAddr);
    cb.params.addrStart = addrStart;
    cb.params.addrEnd = addrEnd;

    jscb_id_t callbackId = AddCallback(ctx, JS_HOOK_CPUSTEP, cb);

    duk_push_uint(ctx, callbackId);
    return 1;
}

duk_ret_t ScriptAPI::js_events_onwrite(duk_context* ctx)
{
    if (!HaveInterpreter())
    {
        return ThrowNeedInterpreterError(ctx);
    }

    uint32_t addrStart, addrEnd;
    if (duk_get_top(ctx) != 2 ||
        !GetAddressOrAddressRange(ctx, 0, &addrStart, &addrEnd) ||
        !duk_is_function(ctx, 1))
    {
        return ThrowInvalidArgsError(ctx);
    }

    JSCallback cb(GetInstance(ctx), duk_get_heapptr(ctx, 1),
        CbCond_CpuStep_WriteAddrBetween, CbArgs_CpuStep_ReadWriteAddr);
    cb.params.addrStart = addrStart;
    cb.params.addrEnd = addrEnd;

    jscb_id_t callbackId = AddCallback(ctx, JS_HOOK_CPUSTEP, cb);

    duk_push_uint(ctx, callbackId);
    return 1;
}

duk_ret_t ScriptAPI::js_events_onopcode(duk_context* ctx)
{
    if (!HaveInterpreter())
    {
        return ThrowNeedInterpreterError(ctx);
    }

    duk_idx_t nargs = duk_get_top(ctx);
    duk_idx_t cbidx = (nargs == 4) ? 3 : 2;

    uint32_t addrStart, addrEnd;
    if ((nargs < 3 || nargs > 4) ||
        !GetAddressOrAddressRange(ctx, 0, &addrStart, &addrEnd) ||
        !duk_is_number(ctx, 1) ||
        (nargs == 4 && !duk_is_number(ctx, 2)) ||
        !duk_is_function(ctx, cbidx))
    {
        return ThrowInvalidArgsError(ctx);
    }

    uint32_t opcode = duk_get_uint(ctx, 1);
    uint32_t mask = 0xFFFFFFFF;

    if (nargs == 4)
    {
        mask = duk_get_uint(ctx, 2);
    }

    JSCallback cb(GetInstance(ctx), duk_get_heapptr(ctx, cbidx),
        CbCond_CpuStep_PcBetween_Opcode, NULL);
    cb.params.addrStart = addrStart;
    cb.params.addrEnd = addrEnd;
    cb.params.opcode = opcode;
    cb.params.opcodeMask = mask;

    jscb_id_t callbackId = AddCallback(ctx, JS_HOOK_CPUSTEP, cb);

    duk_push_uint(ctx, callbackId);
    return 1;
}

duk_ret_t ScriptAPI::js_events_ongprvalue(duk_context* ctx)
{
    if (!HaveInterpreter())
    {
        return ThrowNeedInterpreterError(ctx);
    }

    uint32_t addrStart, addrEnd;
    if (duk_get_top(ctx) != 4 ||
        !GetAddressOrAddressRange(ctx, 0, &addrStart, &addrEnd) ||
        !duk_is_number(ctx, 1) ||
        !duk_is_number(ctx, 2) ||
        !duk_is_function(ctx, 3))
    {
        return ThrowInvalidArgsError(ctx);
    }

    JSCallback cb(GetInstance(ctx), duk_get_heapptr(ctx, 3),
        CbCond_CpuStep_PcBetween_GprValue, CbArgs_CpuStep_AffectedRegIndex);
    cb.params.addrStart = addrStart;
    cb.params.addrEnd = addrEnd;
    cb.params.regIndices = duk_get_uint(ctx, 1);
    cb.params.regValue = duk_get_uint(ctx, 2);

    jscb_id_t callbackId = AddCallback(ctx, JS_HOOK_CPUSTEP, cb);

    duk_push_uint(ctx, callbackId);
    return 1;
}

duk_ret_t ScriptAPI::js_events_ondraw(duk_context* ctx)
{
    return 0;
}

duk_ret_t ScriptAPI::js_events_remove(duk_context* ctx)
{
    if (!duk_is_number(ctx, 0))
    {
        return ThrowInvalidArgsError(ctx);
    }

    jscb_id_t callbackId = (jscb_id_t)duk_get_uint(ctx, -1);
    duk_pop(ctx);

    if (!RemoveCallback(ctx, callbackId))
    {
        duk_push_error_object(ctx, DUK_ERR_ERROR, "invalid callback ID");
        return duk_throw(ctx);
    }

    return 0;
}

// onread
bool CbCond_CpuStep_ReadAddrBetween(JSCallback* cb, void* _env)
{
    jshook_env_cpustep_t* env = (jshook_env_cpustep_t*)_env;

    if(!env->bReadOp)
    {
        return false;
    }

    return (env->readWriteAddr >= cb->params.addrStart &&
            env->readWriteAddr <= cb->params.addrEnd);
}

// onwrite
bool CbCond_CpuStep_WriteAddrBetween(JSCallback* cb, void* _env)
{
    jshook_env_cpustep_t* env = (jshook_env_cpustep_t*)_env;

    if(!env->bWriteOp)
    {
        return false;
    }

    return (env->readWriteAddr >= cb->params.addrStart &&
            env->readWriteAddr <= cb->params.addrEnd);
}

// onexec
bool CbCond_CpuStep_PcBetween(JSCallback* cb, void* _env)
{
    jshook_env_cpustep_t* env = (jshook_env_cpustep_t*)_env;
    return (env->pc >= cb->params.addrStart &&
            env->pc <= cb->params.addrEnd);
}

// onopcode
bool CbCond_CpuStep_PcBetween_Opcode(JSCallback* cb, void* _env)
{
    if (!CbCond_CpuStep_PcBetween(cb, _env))
    {
        return false;
    }

    jshook_env_cpustep_t* env = (jshook_env_cpustep_t*)_env;
    return cb->params.opcode == (env->opcode & cb->params.opcodeMask);
}

// ongprvalue
static bool CbCond_CpuStep_PcBetween_GprValue(JSCallback* cb, void* _env)
{
    if (!CbCond_CpuStep_PcBetween(cb, _env))
    {
        return false;
    }

    jshook_env_cpustep_t* env = (jshook_env_cpustep_t*)_env;

    for(int i = 0; i < 32; i++)
    {
        if(cb->params.regIndices & (1 << i))
        {
            if(g_Reg->m_GPR[i].UW[0] == cb->params.regValue)
            {
                env->outAffectedRegIndex = i;
                return true;
            }
        }
    }

    return false;
}

// onread, onwrite
duk_idx_t CbArgs_CpuStep_ReadWriteAddr(duk_context* ctx, void* _env)
{
    jshook_env_cpustep_t* env = (jshook_env_cpustep_t*)_env;
    duk_push_uint(ctx, env->readWriteAddr);

    /*
    * todo if int write pass g_Reg->m_GPR[(env->opcode >> 16) & 0x1F]
    * if float write pass g_Reg->m_FPR_S/D[etc]
    * add 'wantUnsigned' argument to onread/onwrite?
    */

    return 1;
}

// ongprvalue
duk_idx_t CbArgs_CpuStep_AffectedRegIndex(duk_context* ctx, void* _env)
{
    jshook_env_cpustep_t* env = (jshook_env_cpustep_t*)_env;
    duk_push_uint(ctx, env->outAffectedRegIndex);
    return 1;
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

static duk_ret_t ThrowNeedInterpreterError(duk_context* ctx)
{
    duk_push_error_object(ctx, DUK_ERR_ERROR, "this feature requires the interpreter core");
    return duk_throw(ctx);
}

static bool HaveInterpreter()
{
    if (!g_Settings->LoadBool(Setting_ForceInterpreterCPU) &&
        (CPU_TYPE)g_Settings->LoadDword(Game_CpuType) != CPU_Interpreter)
    {
        return false;
    }

    return true;
}
