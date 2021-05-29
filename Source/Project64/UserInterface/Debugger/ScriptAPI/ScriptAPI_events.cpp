#include <stdafx.h>
#include "../ScriptAPI.h"
#include "../OpInfo.h"

#pragma warning(disable: 4702) // disable unreachable code warning

using namespace ScriptAPI;

static bool CbCond_PcBetween(JSCallback* cb, void* env);
static bool CbCond_ReadAddrBetween(JSCallback* cb, void* env);
static bool CbCond_WriteAddrBetween(JSCallback* cb, void* env);
static bool CbCond_PcBetween_OpcodeEquals(JSCallback* cb, void* env);
static bool CbCond_PcBetween_GprValueEquals(JSCallback* cb, void* env);

static duk_idx_t CbArgs_GenericEventObject(duk_context* ctx, void* env);
static duk_idx_t CbArgs_ExecEventObject(duk_context* ctx, void* env);
static duk_idx_t CbArgs_ReadEventObject(duk_context* ctx, void* env);
static duk_idx_t CbArgs_WriteEventObject(duk_context* ctx, void* env);
static duk_idx_t CbArgs_OpcodeEventObject(duk_context* ctx, void* env);
static duk_idx_t CbArgs_RegValueEventObject(duk_context* ctx, void* env);
static duk_idx_t CbArgs_DrawEventObject(duk_context* ctx, void* env);
static duk_idx_t CbArgs_MouseEventObject(duk_context* ctx, void* env);
static duk_idx_t CbArgs_SPTaskEventObject(duk_context* ctx, void* env);
static duk_idx_t CbArgs_PIEventObject(duk_context* ctx, void* env);

static void CbFinish_KillDrawingContext(duk_context* ctx, void* env);

static bool GetAddressOrAddressRange(duk_context* ctx, duk_idx_t idx, uint32_t* addrStart, uint32_t *addrEnd);

static duk_ret_t ThrowNeedInterpreterError(duk_context* ctx);
static bool HaveInterpreter();

void ScriptAPI::Define_events(duk_context* ctx)
{
    const duk_function_list_entry funcs[] = {
        { "onexec",       js_events_onexec, 2 },
        { "onread",       js_events_onread, 2 },
        { "onwrite",      js_events_onwrite, 2 },
        { "ongprvalue",   js_events_ongprvalue, 4 },
        { "onopcode",     js_events_onopcode, 4 },
        { "ondraw",       js_events_ondraw, 1 },
        { "onpifread",    js_events_onpifread, 1 },
        { "onsptask",     js_events_onsptask, 1 },
        { "onpidma",      js_events_onpidma, 1 },
        { "onmouseup",    js_events_onmouseup, 1 },
        { "onmousedown",  js_events_onmousedown, 1 },
        { "onmousemove",  js_events_onmousemove, 1 },
        { "remove",       js_events_remove, 1 },
        { nullptr, nullptr, 0 }
    };

    duk_push_global_object(ctx);
    duk_push_string(ctx, "events");
    duk_push_object(ctx);
    duk_put_function_list(ctx, -1, funcs);
    duk_freeze(ctx, -1);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_ENUMERABLE);
    duk_pop(ctx);

    const char* dummyConstructorNames[] = {
        "GenericEvent",
        "CPUExecEvent",
        "CPUReadWriteEvent",
        "CPUOpcodeEvent",
        "CPURegValueEvent",
        "SPTaskEvent",
        "PIEvent",
        "DrawEvent",
        nullptr
    };

    DefineGlobalDummyConstructors(ctx, dummyConstructorNames);

    duk_number_list_entry mouseEventStaticProps[] = {
        { "NONE", -1 },
        { "LEFT", 0 },
        { "MIDDLE", 1 },
        { "RIGHT", 2 },
        { nullptr, 0 }
    };

    duk_push_global_object(ctx);
    duk_push_string(ctx, "MouseEvent");
    PushNewDummyConstructor(ctx, false);
    duk_put_number_list(ctx, -1, mouseEventStaticProps);
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
    if(!GetAddressOrAddressRange(ctx, 0, &addrStart, &addrEnd) ||
       !duk_is_function(ctx, 1))
    {
        return ThrowInvalidArgsError(ctx);
    }

    JSCallback cb(GetInstance(ctx), duk_get_heapptr(ctx, 1),
        CbCond_PcBetween, CbArgs_ExecEventObject);
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
    if (!GetAddressOrAddressRange(ctx, 0, &addrStart, &addrEnd) ||
        !duk_is_function(ctx, 1))
    {
        return ThrowInvalidArgsError(ctx);
    }

    JSCallback cb(GetInstance(ctx), duk_get_heapptr(ctx, 1),
        CbCond_ReadAddrBetween, CbArgs_ReadEventObject);
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
    if (!GetAddressOrAddressRange(ctx, 0, &addrStart, &addrEnd) ||
        !duk_is_function(ctx, 1))
    {
        return ThrowInvalidArgsError(ctx);
    }

    JSCallback cb(GetInstance(ctx), duk_get_heapptr(ctx, 1),
        CbCond_WriteAddrBetween, CbArgs_WriteEventObject);
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

    uint32_t addrStart, addrEnd;
    if (!GetAddressOrAddressRange(ctx, 0, &addrStart, &addrEnd) ||
        !duk_is_number(ctx, 1) ||
        !duk_is_number(ctx, 2) ||
        !duk_is_function(ctx, 3))
    {
        return ThrowInvalidArgsError(ctx);
    }

    uint32_t opcode = duk_get_uint(ctx, 1);
    uint32_t mask = duk_get_uint(ctx, 2);

    JSCallback cb(GetInstance(ctx), duk_get_heapptr(ctx, 3),
        CbCond_PcBetween_OpcodeEquals, CbArgs_OpcodeEventObject);
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
    if (!GetAddressOrAddressRange(ctx, 0, &addrStart, &addrEnd) ||
        !duk_is_number(ctx, 1) ||
        !duk_is_number(ctx, 2) ||
        !duk_is_function(ctx, 3))
    {
        return ThrowInvalidArgsError(ctx);
    }

    JSCallback cb(GetInstance(ctx), duk_get_heapptr(ctx, 3),
        CbCond_PcBetween_GprValueEquals, CbArgs_RegValueEventObject);
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
    if (!duk_is_function(ctx, 0))
    {
        return ThrowInvalidArgsError(ctx);
    }

    jscb_id_t callbackId = AddCallback(ctx, 0, JS_HOOK_GFXUPDATE, nullptr,
        CbArgs_DrawEventObject, CbFinish_KillDrawingContext);
    duk_push_uint(ctx, callbackId);
    return 1;
}

duk_ret_t ScriptAPI::js_events_onpifread(duk_context* ctx)
{
    if (!duk_is_function(ctx, 0))
    {
        return ThrowInvalidArgsError(ctx);
    }

    jscb_id_t callbackId = AddCallback(ctx, 0, JS_HOOK_PIFREAD, nullptr, CbArgs_GenericEventObject);
    duk_push_uint(ctx, callbackId);
    return 1;
}

duk_ret_t ScriptAPI::js_events_onsptask(duk_context* ctx)
{
    if (!duk_is_function(ctx, 0))
    {
        return ThrowInvalidArgsError(ctx);
    }

    jscb_id_t callbackId = AddCallback(ctx, 0, JS_HOOK_RSPTASK, nullptr, CbArgs_SPTaskEventObject);
    duk_push_uint(ctx, callbackId);
    return 1;
}

duk_ret_t ScriptAPI::js_events_onpidma(duk_context* ctx)
{
    if (!duk_is_function(ctx, 0))
    {
        return ThrowInvalidArgsError(ctx);
    }

    jscb_id_t callbackId = AddCallback(ctx, 0, JS_HOOK_PIDMA, nullptr, CbArgs_PIEventObject);
    duk_push_uint(ctx, callbackId);
    return 1;
}

duk_ret_t ScriptAPI::js_events_onmouseup(duk_context* ctx)
{
    if (!duk_is_function(ctx, 0))
    {
        return ThrowInvalidArgsError(ctx);
    }

    jscb_id_t callbackId = AddCallback(ctx, 0, JS_HOOK_MOUSEUP, nullptr, CbArgs_MouseEventObject);
    duk_push_uint(ctx, callbackId);
    return 1;
}

duk_ret_t ScriptAPI::js_events_onmousedown(duk_context* ctx)
{
    if (!duk_is_function(ctx, 0))
    {
        return ThrowInvalidArgsError(ctx);
    }

    jscb_id_t callbackId = AddCallback(ctx, 0, JS_HOOK_MOUSEDOWN, nullptr, CbArgs_MouseEventObject);
    duk_push_uint(ctx, callbackId);
    return 1;
}

duk_ret_t ScriptAPI::js_events_onmousemove(duk_context* ctx)
{
    if (!duk_is_function(ctx, 0))
    {
        return ThrowInvalidArgsError(ctx);
    }

    jscb_id_t callbackId = AddCallback(ctx, 0, JS_HOOK_MOUSEMOVE, nullptr, CbArgs_MouseEventObject);
    duk_push_uint(ctx, callbackId);
    return 1;
}

duk_ret_t ScriptAPI::js_events_remove(duk_context* ctx)
{
    if (!duk_is_number(ctx, 0))
    {
        return ThrowInvalidArgsError(ctx);
    }

    jscb_id_t callbackId = (jscb_id_t)duk_get_uint(ctx, 0);

    if (!RemoveCallback(ctx, callbackId))
    {
        duk_push_error_object(ctx, DUK_ERR_REFERENCE_ERROR, "invalid callback ID");
        return duk_throw(ctx);
    }

    return 0;
}

bool CbCond_ReadAddrBetween(JSCallback* cb, void* _env)
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

bool CbCond_WriteAddrBetween(JSCallback* cb, void* _env)
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

bool CbCond_PcBetween(JSCallback* cb, void* _env)
{
    jshook_env_cpustep_t* env = (jshook_env_cpustep_t*)_env;
    return (env->pc >= cb->params.addrStart &&
            env->pc <= cb->params.addrEnd);
}

bool CbCond_PcBetween_OpcodeEquals(JSCallback* cb, void* _env)
{
    if (!CbCond_PcBetween(cb, _env))
    {
        return false;
    }

    jshook_env_cpustep_t* env = (jshook_env_cpustep_t*)_env;
    return cb->params.opcode == (env->opInfo.m_OpCode.Hex & cb->params.opcodeMask);
}

static bool CbCond_PcBetween_GprValueEquals(JSCallback* cb, void* _env)
{
    if (!CbCond_PcBetween(cb, _env))
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

duk_idx_t CbArgs_GenericEventObject(duk_context* ctx, void* /*_env*/)
{
    CScriptInstance* inst = GetInstance(ctx);
    duk_push_object(ctx);
    SetDummyConstructor(ctx, -1, "GenericEvent");
    
    const DukPropListEntry props[] = {
        { "callbackId", DukUInt(inst->CallbackId()) },
        { nullptr }
    };

    DukPutPropList(ctx, -1, props);
    duk_freeze(ctx, -1);
    return 1;
}

duk_idx_t CbArgs_ExecEventObject(duk_context* ctx, void* _env)
{
    CScriptInstance* inst = GetInstance(ctx);
    jshook_env_cpustep_t* env = (jshook_env_cpustep_t*)_env;
    duk_push_object(ctx);
    SetDummyConstructor(ctx, -1, "CPUExecEvent");
    
    const DukPropListEntry props[] = {
        { "callbackId", DukUInt(inst->CallbackId()) },
        { "pc", DukUInt(env->pc) },
        { nullptr }
    };

    DukPutPropList(ctx, -1, props);
    duk_freeze(ctx, -1);
    return 1;
}

duk_idx_t CbArgs_ReadEventObject(duk_context* ctx, void* _env)
{
    CScriptInstance* inst = GetInstance(ctx);
    CDebuggerUI* debugger = inst->Debugger();
    jshook_env_cpustep_t* env = (jshook_env_cpustep_t*)_env;
    
    uint32_t address = env->opInfo.GetLoadStoreAddress();

    uint8_t op = env->opInfo.m_OpCode.op;
    uint8_t rt = env->opInfo.m_OpCode.rt;
    bool bFPU = (op == R4300i_LWC1 || op == R4300i_LDC1);

    duk_push_object(ctx);
    SetDummyConstructor(ctx, -1, "CPUReadWriteEvent");

    const DukPropListEntry props[] = {
        { "callbackId", DukUInt(inst->CallbackId()) },
        { "pc", DukUInt(env->pc) },
        { "address", DukUInt(address) },
        { "reg", DukUInt(rt) },
        { "fpu", DukBoolean(bFPU) },
        { nullptr }
    };

    DukPutPropList(ctx, -1, props);

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
    
    switch (env->opInfo.m_OpCode.op)
    {
    case R4300i_LB:
        debugger->DebugLoad_VAddr(address, value.s8);
        duk_push_int(ctx, value.s8);
        duk_push_int(ctx, S8);
        break;
    case R4300i_LBU:
        debugger->DebugLoad_VAddr(address, value.u8);
        duk_push_uint(ctx, value.u8);
        duk_push_int(ctx, U8);
        break;
    case R4300i_LH:
        debugger->DebugLoad_VAddr(address, value.s16);
        duk_push_int(ctx, value.s16);
        duk_push_int(ctx, S16);
        break;
    case R4300i_LHU:
        debugger->DebugLoad_VAddr(address, value.u16);
        duk_push_uint(ctx, value.u16);
        duk_push_int(ctx, U16);
        break;
    case R4300i_LL:
    case R4300i_LW:
        debugger->DebugLoad_VAddr(address, value.s32);
        duk_push_int(ctx, value.s32);
        duk_push_int(ctx, S32);
        break;
    case R4300i_LWU:
        debugger->DebugLoad_VAddr(address, value.u32);
        duk_push_uint(ctx, value.u32);
        duk_push_int(ctx, U32);
        break;
    case R4300i_LWC1:
        debugger->DebugLoad_VAddr(address, value.f32);
        duk_push_number(ctx, value.f32);
        duk_push_int(ctx, F32);
        break;
    case R4300i_LDC1:
        debugger->DebugLoad_VAddr(address, value.f64);
        duk_push_number(ctx, value.f64);
        duk_push_int(ctx, F64);
        break;
    case R4300i_LD:
        debugger->DebugLoad_VAddr(address, value.u64);
        duk_push_number(ctx, (duk_double_t)(value.u64 & 0xFFFFFFFF));
        duk_push_int(ctx, U64);
        bNeedUpper32 = true;
        break;
    case R4300i_LDL:
        {
        int shift = (address & 7) * 8;
        uint64_t mask = ~(((uint64_t)-1) << shift);
        debugger->DebugLoad_VAddr(address & ~7, value.u64);
        value.u64 = (g_Reg->m_GPR[rt].DW & mask) + (value.u64 << shift);
        duk_push_number(ctx, (duk_double_t)(value.u64 & 0xFFFFFFFF));
        duk_push_int(ctx, U64);
        bNeedUpper32 = true;
        }
        break;
    case R4300i_LDR:
        {
        int shift = 56 - ((address & 7) * 8);
        uint64_t mask = ~(((uint64_t)-1) >> shift);
        debugger->DebugLoad_VAddr(address & ~7, value.u64);
        value.u64 = (g_Reg->m_GPR[rt].DW & mask) + (value.u64 >> shift);
        duk_push_number(ctx, (duk_double_t)(value.u64 & 0xFFFFFFFF));
        duk_push_int(ctx, U64);
        bNeedUpper32 = true;
        }
        break;
    case R4300i_LWL:
        {
        int shift = (address & 3) * 8;
        uint32_t mask = ~(((uint32_t)-1) << shift);
        debugger->DebugLoad_VAddr(address & ~3, value.s32);
        value.s32 = (g_Reg->m_GPR[rt].W[0] & mask) + (value.s32 << shift);
        duk_push_number(ctx, value.s32);
        duk_push_int(ctx, S32);
        }
        break;
    case R4300i_LWR:
        {
        int shift = 24 - ((address & 3) * 8);
        uint32_t mask = ~(((uint32_t)-1) >> shift);
        debugger->DebugLoad_VAddr(address & ~3, value.s32);
        value.s32 = (g_Reg->m_GPR[rt].W[0] & mask) + (value.s32 >> shift);
        duk_push_number(ctx, value.s32);
        duk_push_int(ctx, S32);
        }
        break;
    default:
        duk_push_number(ctx, 0);
        duk_push_number(ctx, 0);
        break;
    }

    duk_put_prop_string(ctx, -3, "valueType");
    duk_put_prop_string(ctx, -2, "value");

    if (bNeedUpper32)
    {
        duk_push_number(ctx, (duk_double_t)(value.u64 >> 32));
        duk_put_prop_string(ctx, -2, "valueHi");
    }

    duk_freeze(ctx, -1);
    return 1;
}

duk_idx_t CbArgs_WriteEventObject(duk_context* ctx, void* _env)
{
    CScriptInstance* inst = GetInstance(ctx);
    CDebuggerUI* debugger = inst->Debugger();
    jshook_env_cpustep_t* env = (jshook_env_cpustep_t*)_env;

    uint32_t address = env->opInfo.GetLoadStoreAddress();

    uint8_t op = env->opInfo.m_OpCode.op;
    uint8_t rt = env->opInfo.m_OpCode.rt;
    bool bFPU = (op == R4300i_SWC1 || op == R4300i_SDC1);

    duk_push_object(ctx);
    SetDummyConstructor(ctx, -1, "CPUReadWriteEvent");

    const DukPropListEntry props[] = {
        { "callbackId", DukUInt(inst->CallbackId()) },
        { "pc", DukUInt(env->pc) },
        { "address", DukUInt(address) },
        { "reg", DukUInt(rt) },
        { "fpu", DukBoolean(bFPU) },
        { nullptr }
    };

    DukPutPropList(ctx, -1, props);

    bool bNeedUpper32 = false;
    uint64_t value64 = 0;

    switch (env->opInfo.m_OpCode.op)
    {
    case R4300i_SB:
        duk_push_int(ctx, g_Reg->m_GPR[rt].B[0]);
        duk_push_int(ctx, S8);
        break;
    case R4300i_SH:
        duk_push_int(ctx, g_Reg->m_GPR[rt].HW[0]);
        duk_push_int(ctx, S16);
        break;
    case R4300i_SW:
        duk_push_int(ctx, g_Reg->m_GPR[rt].W[0]);
        duk_push_int(ctx, S32);
        break;
    case R4300i_SWC1:
        duk_push_number(ctx, *g_Reg->m_FPR_S[rt]);
        duk_push_int(ctx, F32);
        break;
    case R4300i_SDC1:
        duk_push_number(ctx, *g_Reg->m_FPR_D[rt]);
        duk_push_int(ctx, F64);
        break;
    case R4300i_SD:
        duk_push_number(ctx, g_Reg->m_GPR[rt].UW[0]);
        duk_push_int(ctx, U64);
        bNeedUpper32 = true;
        break;
    case R4300i_SWL:
        {
        int shift = (address & 3) * 8;
        uint32_t mask = ~(((uint32_t)-1) >> shift);
        uint32_t value;
        debugger->DebugLoad_VAddr(address & ~3, value);
        value = (value & mask) + (g_Reg->m_GPR[rt].UW[0] >> shift);
        duk_push_number(ctx, value);
        duk_push_int(ctx, S32);
        }
        break;
    case R4300i_SWR:
        {
        int shift = 24 - ((address & 3) * 8);
        uint32_t mask = ~(((uint32_t)-1) << shift);
        uint32_t value;
        debugger->DebugLoad_VAddr(address & ~3, value);
        value = (value & mask) + (g_Reg->m_GPR[rt].UW[0] >> shift);
        duk_push_number(ctx, value);
        duk_push_int(ctx, S32);
        }
        break;
    case R4300i_SDL:
        {
        int shift = (address & 7) * 8;
        uint64_t mask = ~(((uint64_t)-1) >> shift);
        debugger->DebugLoad_VAddr(address & ~7, value64);
        value64 = (value64 & mask) + (g_Reg->m_GPR[rt].UDW >> shift);
        duk_push_number(ctx, (duk_double_t)(value64 & 0xFFFFFFFF));
        duk_push_int(ctx, U64);
        }
    case R4300i_SDR:
    {
        int shift = 56 - ((address & 7) * 8);
        uint64_t mask = ~(((uint64_t)-1) << shift);
        debugger->DebugLoad_VAddr(address & ~7, value64);
        value64 = (value64 & mask) + (g_Reg->m_GPR[rt].UDW >> shift);
        duk_push_number(ctx, (duk_double_t)(value64 & 0xFFFFFFFF));
        duk_push_int(ctx, U64);
    }
    default:
        duk_push_number(ctx, 0);
        duk_push_number(ctx, 0);
        break;
    }

    duk_put_prop_string(ctx, -3, "valueType");
    duk_put_prop_string(ctx, -2, "value");

    if (bNeedUpper32)
    {
        duk_push_number(ctx, (duk_double_t)(value64 >> 32));
        duk_put_prop_string(ctx, -2, "valueHi");
    }

    duk_freeze(ctx, -1);
    
    return 1;
}

duk_idx_t CbArgs_OpcodeEventObject(duk_context* ctx, void* _env)
{
    CScriptInstance* inst = GetInstance(ctx);
    jshook_env_cpustep_t* env = (jshook_env_cpustep_t*)_env;
    duk_push_object(ctx);
    SetDummyConstructor(ctx, -1, "CPUOpcodeEvent");

    const DukPropListEntry props[] = {
        { "callbackId", DukUInt(inst->CallbackId()) },
        { "pc", DukUInt(env->pc) },
        { "opcode", DukUInt(env->opInfo.m_OpCode.Hex) },
        { nullptr }
    };

    DukPutPropList(ctx, -1, props);
    duk_freeze(ctx, -1);
    return 1;
}

duk_idx_t CbArgs_RegValueEventObject(duk_context* ctx, void* _env)
{
    CScriptInstance* inst = GetInstance(ctx);
    jshook_env_cpustep_t* env = (jshook_env_cpustep_t*)_env;
    duk_push_object(ctx);
    SetDummyConstructor(ctx, -1, "CPURegValueEvent");

    const DukPropListEntry props[] = {
        { "callbackId", DukUInt(inst->CallbackId()) },
        { "pc", DukUInt(env->pc) },
        { "value", DukUInt(g_Reg->m_GPR[env->outAffectedRegIndex].UW[0]) },
        { "reg", DukUInt(env->outAffectedRegIndex) },
        { nullptr }
    };

    DukPutPropList(ctx, -1, props);
    duk_freeze(ctx, -1);
    return 1;
}

duk_idx_t CbArgs_DrawEventObject(duk_context* ctx, void* _env)
{
    CScriptInstance* inst = GetInstance(ctx);
    jshook_env_gfxupdate_t* env = (jshook_env_gfxupdate_t*)_env;

    duk_push_object(ctx);
    SetDummyConstructor(ctx, -1, "DrawEvent");

    duk_push_uint(ctx, inst->CallbackId());
    duk_put_prop_string(ctx, -2, "callbackId");

    duk_get_global_string(ctx, "DrawingContext");
    duk_push_pointer(ctx, env->scriptRenderWindow);
    AllowPrivateCall(ctx, true);
    duk_new(ctx, 1);
    AllowPrivateCall(ctx, false);

    duk_dup(ctx, -1);
    duk_put_global_string(ctx, HSYM_CURDRAWINGCTX);
    env->jsDrawingContext = duk_get_heapptr(ctx, -1);

    duk_put_prop_string(ctx, -2, "drawingContext");

    duk_freeze(ctx, -1);

    return 1;
}

void CbFinish_KillDrawingContext(duk_context* ctx, void* _env)
{
    jshook_env_gfxupdate_t* env = (jshook_env_gfxupdate_t*)_env;
    duk_push_heapptr(ctx, env->jsDrawingContext);
    duk_push_pointer(ctx, nullptr);
    duk_put_prop_string(ctx, -2, DUK_HIDDEN_SYMBOL("srw"));
    duk_pop(ctx);
}

static duk_idx_t CbArgs_MouseEventObject(duk_context* ctx, void* _env)
{
    CScriptInstance* inst = GetInstance(ctx);
    jshook_env_mouse_t* env = (jshook_env_mouse_t*)_env;
    duk_push_object(ctx);
    SetDummyConstructor(ctx, -1, "MouseEvent");

    const DukPropListEntry props[] = {
        { "callbackId", DukUInt(inst->CallbackId()) },
        { "button", DukInt(env->button) },
        { "x", DukInt(env->x) },
        { "y", DukInt(env->y) },
        { nullptr }
    };
    
    DukPutPropList(ctx, -1, props);
    duk_freeze(ctx, -1);
    return 1;
}

static duk_idx_t CbArgs_SPTaskEventObject(duk_context* ctx, void* _env)
{
    CScriptInstance* inst = GetInstance(ctx);
    jshook_env_sptask_t* env = (jshook_env_sptask_t*)_env;
    duk_push_object(ctx);
    SetDummyConstructor(ctx, -1, "SPTaskEvent");

    const DukPropListEntry props[] = {
        { "callbackId",        DukUInt(inst->CallbackId()) },
        { "taskType",          DukUInt(env->taskType) },
        { "taskFlags",         DukUInt(env->taskFlags) },
        { "ucodeBootAddress",  DukUInt(env->ucodeBootAddress | 0x80000000) },
        { "ucodeBootSize",     DukUInt(env->ucodeBootSize) },
        { "ucodeAddress",      DukUInt(env->ucodeAddress | 0x80000000) },
        { "ucodeSize",         DukUInt(env->ucodeSize) },
        { "ucodeDataAddress",  DukUInt(env->ucodeDataAddress | 0x80000000) },
        { "ucodeDataSize",     DukUInt(env->ucodeDataSize) },
        { "dramStackAddress",  DukUInt(env->dramStackAddress | 0x80000000) },
        { "dramStackSize",     DukUInt(env->dramStackSize) },
        { "outputBuffAddress", DukUInt(env->outputBuffAddress | 0x80000000) },
        { "outputBuffSize",    DukUInt(env->outputBuffSize) },
        { "dataAddress",       DukUInt(env->dataAddress | 0x80000000) },
        { "dataSize",          DukUInt(env->dataSize) },
        { "yieldDataAddress",  DukUInt(env->yieldDataAddress | 0x80000000) },
        { "yieldDataSize",     DukUInt(env->yieldDataSize) },
        { nullptr }
    };

    DukPutPropList(ctx, -1, props);
    duk_freeze(ctx, -1);
    return 1;
}

static duk_idx_t CbArgs_PIEventObject(duk_context* ctx, void* _env)
{
    CScriptInstance* inst = GetInstance(ctx);
    jshook_env_pidma_t* env = (jshook_env_pidma_t*)_env;
    duk_push_object(ctx);
    SetDummyConstructor(ctx, -1, "PIEvent");

    const DukPropListEntry props[] = {
        { "callbackId",  DukUInt(inst->CallbackId()) },
        { "direction",   DukUInt(env->direction) },
        { "dramAddress", DukUInt(env->dramAddress | 0x80000000) },
        { "cartAddress", DukUInt(env->cartAddress | 0xA0000000) },
        { "length",      DukUInt(env->length + 1) },
        { nullptr }
    };

    DukPutPropList(ctx, -1, props);
    duk_freeze(ctx, -1);
    return 1;
}

bool GetAddressOrAddressRange(duk_context* ctx, duk_idx_t idx, uint32_t* addrStart, uint32_t* addrEnd)
{
    if(duk_is_number(ctx, idx))
    {
        if (abs(duk_get_number(ctx, idx)) > 0xFFFFFFFF)
        {
            return false;
        }

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
