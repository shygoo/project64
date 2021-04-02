#include <stdafx.h>
#include "../ScriptAPI.h"
#include "../OpInfo.h"

#pragma warning(disable: 4702)

static bool CbCond_CpuStep_ReadAddrBetween(JSCallback* cb, void* env);
static bool CbCond_CpuStep_WriteAddrBetween(JSCallback* cb, void* env);
static bool CbCond_CpuStep_PcBetween(JSCallback* cb, void* env);
static bool CbCond_CpuStep_PcBetween_Opcode(JSCallback* cb, void* env);
static bool CbCond_CpuStep_PcBetween_GprValue(JSCallback* cb, void* env);

static duk_idx_t CbArgs_CpuStep_ReadEventObject(duk_context* ctx, void* env);
static duk_idx_t CbArgs_CpuStep_WriteAddr_Value(duk_context* ctx, void* env);

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
        CbCond_CpuStep_ReadAddrBetween, CbArgs_CpuStep_ReadEventObject);
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
        CbCond_CpuStep_WriteAddrBetween, CbArgs_CpuStep_WriteAddr_Value);
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

    if(!env->opInfo.IsLoadCommand())
    {
        return false;
    }

    uint32_t addr = env->opInfo.GetLoadStoreAddress();

    return (addr >= cb->params.addrStart &&
            addr <= cb->params.addrEnd);
}

// onwrite
bool CbCond_CpuStep_WriteAddrBetween(JSCallback* cb, void* _env)
{
    jshook_env_cpustep_t* env = (jshook_env_cpustep_t*)_env;

    if(!env->opInfo.IsStoreCommand())
    {
        return false;
    }

    uint32_t addr = env->opInfo.GetLoadStoreAddress();

    return (addr >= cb->params.addrStart &&
            addr <= cb->params.addrEnd);
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
    return cb->params.opcode == (env->opInfo.m_OpCode.Hex & cb->params.opcodeMask);
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

// onread
duk_idx_t CbArgs_CpuStep_ReadEventObject(duk_context* ctx, void* _env)
{
    jshook_env_cpustep_t* env = (jshook_env_cpustep_t*)_env;
    CDebuggerUI* debugger = ScriptAPI::GetInstance(ctx)->Debugger();

    uint32_t address = env->opInfo.GetLoadStoreAddress();

    duk_push_object(ctx); // CPUReadWriteEvent

    duk_push_uint(ctx, address);
    duk_put_prop_string(ctx, -2, "address");

    union {
        uint8_t u8;
        int8_t s8;
        uint16_t u16;
        int16_t s16;
        uint32_t u32;
        int32_t s32;
        float f32;
        double f64;
        uint64_t u64;
    } value = {0};
    
    bool bNeedUpper32 = false;
    bool bCop1 = false;

    switch (env->opInfo.m_OpCode.op)
    {
    case R4300i_LB:
        debugger->DebugLoad_VAddr(address, value.s8);
        duk_push_int(ctx, value.s8);
        duk_push_int(ctx, ScriptAPI::S8);
        break;
    case R4300i_LBU:
        debugger->DebugLoad_VAddr(address, value.u8);
        duk_push_uint(ctx, value.u8);
        duk_push_int(ctx, ScriptAPI::U8);
        break;
    case R4300i_LH:
        debugger->DebugLoad_VAddr(address, value.s16);
        duk_push_int(ctx, value.s16);
        duk_push_int(ctx, ScriptAPI::S16);
        break;
    case R4300i_LHU:
        debugger->DebugLoad_VAddr(address, value.u16);
        duk_push_uint(ctx, value.u16);
        duk_push_int(ctx, ScriptAPI::U16);
        break;
    case R4300i_LW:
        debugger->DebugLoad_VAddr(address, value.s32);
        duk_push_int(ctx, value.s32);
        duk_push_int(ctx, ScriptAPI::S32);
        break;
    case R4300i_LWU:
        debugger->DebugLoad_VAddr(address, value.u32);
        duk_push_uint(ctx, value.u32);
        duk_push_int(ctx, ScriptAPI::U32);
        break;
    case R4300i_LWC1:
        debugger->DebugLoad_VAddr(address, value.f32);
        duk_push_number(ctx, value.f32);
        duk_push_int(ctx, ScriptAPI::F32);
        bCop1 = true;
        break;
    case R4300i_LDC1:
        debugger->DebugLoad_VAddr(address, value.f64);
        duk_push_number(ctx, value.f64);
        duk_push_int(ctx, ScriptAPI::F64);
        bCop1 = true;
        break;
    case R4300i_LD:
        debugger->DebugLoad_VAddr(address, value.u64);
        duk_push_number(ctx, value.u64 & 0xFFFFFFFF);
        duk_push_int(ctx, ScriptAPI::U64);
        bNeedUpper32 = true;
        break;
    //R4300i_LL
    //R4300i_LDL
    //R4300i_LDR
    //R4300i_LWL
    //R4300i_LWR
    default:
        duk_push_number(ctx, 0);
        break;
    }

    duk_put_prop_string(ctx, -3, "type");
    duk_put_prop_string(ctx, -2, "value");

    if (bNeedUpper32)
    {
        duk_push_number(ctx, value.u64 >> 32);
        duk_put_prop_string(ctx, -2, "valueHi");
    }

    duk_push_boolean(ctx, bCop1);
    duk_put_prop_string(ctx, -2, "fpu");

    duk_push_number(ctx, env->opInfo.m_OpCode.rt);
    duk_put_prop_string(ctx, -2, "reg");

    duk_freeze(ctx, -1);

    return 1;
}

// onwrite
duk_idx_t CbArgs_CpuStep_WriteAddr_Value(duk_context* ctx, void* _env)
{
    jshook_env_cpustep_t* env = (jshook_env_cpustep_t*)_env;
    duk_push_uint(ctx, env->opInfo.GetLoadStoreAddress());

    //COpInfo info();

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
