#include <stdafx.h>
#include "../ScriptAPI.h"
#include <Project64/UserInterface/Debugger/debugger.h>
#include <Project64/UserInterface/Debugger/DebugMMU.h>
#include <stdio.h>
#include <string>

#pragma warning(disable: 4702)

void ScriptAPI::Define_mem(duk_context *ctx)
{
    #define MEM_PROXY_FUNCTIONS(T) { \
        { "get", js_mem__get<T>, 2 }, \
        { "set", js_mem__set<T>, 3 }, \
        { NULL, NULL, 0 } \
    }

    const struct {
        const char *key;
        const duk_function_list_entry functions[3];
    } proxies[] = {
        { "u32", MEM_PROXY_FUNCTIONS(uint32_t) },
        { "u16", MEM_PROXY_FUNCTIONS(uint16_t) },
        { "u8",  MEM_PROXY_FUNCTIONS(uint8_t) },
        { "s32", MEM_PROXY_FUNCTIONS(int32_t) },
        { "s16", MEM_PROXY_FUNCTIONS(int16_t) },
        { "s8",  MEM_PROXY_FUNCTIONS(int8_t) },
        { "f64", MEM_PROXY_FUNCTIONS(double) },
        { "f32", MEM_PROXY_FUNCTIONS(float) },
        { NULL, NULL }
    };

    const duk_function_list_entry funcs[] = {
        { "getblock",   js_mem_getblock, 2 },
        { "setblock",   js_mem_setblock, DUK_VARARGS },
        { "getstring",  js_mem_getstring, DUK_VARARGS },
        { "setstring",  js_mem_setblock, DUK_VARARGS },
        { "bindvar",    js_mem_bindvar, 4 },
        { "bindvars",   js_mem_bindvars, 2 },
        { "bindstruct", js_mem_bindstruct, 3 },
        { "typedef",    js_mem_typedef, 1 },
        { NULL, NULL, 0 }
    };

    duk_push_global_object(ctx);
    duk_push_string(ctx, "mem");
    duk_push_object(ctx);

    for(int i = 0; proxies[i].key != NULL; i++)
    {
        duk_push_string(ctx, proxies[i].key);
        duk_push_object(ctx);
        duk_push_object(ctx);
        duk_put_function_list(ctx, -1, proxies[i].functions);
        duk_push_proxy(ctx, 0);
        duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_ENUMERABLE);
    }

    duk_push_string(ctx, "ramSize");
    duk_push_c_function(ctx, js_mem__get_ramsize, 0);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_GETTER | DUK_DEFPROP_SET_ENUMERABLE);

    duk_push_string(ctx, "romSize");
    duk_push_c_function(ctx, js_mem__get_romsize, 0);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_GETTER | DUK_DEFPROP_SET_ENUMERABLE);

    duk_put_function_list(ctx, -1, funcs);
    duk_freeze(ctx, -1);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_ENUMERABLE);
    duk_pop(ctx);
}

template <class T>
duk_ret_t ScriptAPI::js_mem__get(duk_context *ctx)
{
    CScriptInstance *inst = GetInstance(ctx);

    uint32_t addr = (uint32_t)duk_to_number(ctx, 1);
    
    T value;
    if(inst->Debugger()->DebugLoad_VAddr<T>(addr, value))
    {
        duk_push_number(ctx, value);
        return 1;
    }

    return ThrowMemoryError(ctx, addr);
}

template <class T>
duk_ret_t ScriptAPI::js_mem__set(duk_context *ctx)
{
    CScriptInstance *inst = GetInstance(ctx);

    uint32_t addr = (uint32_t)duk_to_number(ctx, 1);
    T value = (T)duk_to_number(ctx, 2);

    if(inst->Debugger()->DebugStore_VAddr<T>(addr, value))
    {
        duk_push_true(ctx);
        return 1;
    }

    return ThrowMemoryError(ctx, addr);
}

duk_ret_t ScriptAPI::js_mem_getblock(duk_context *ctx)
{
    CScriptInstance *inst = GetInstance(ctx);

    if(!duk_is_number(ctx, 0) || !duk_is_number(ctx, 1))
    {
        return ThrowInvalidArgsError(ctx);
    }

    duk_uint_t addr = duk_to_uint(ctx, 0);
    duk_uint_t length = duk_to_uint(ctx, 1);

    uint8_t *data = (uint8_t*)duk_push_fixed_buffer(ctx, length);
    duk_push_buffer_object(ctx, -1, 0, length, DUK_BUFOBJ_NODEJS_BUFFER);

    for(size_t i = 0; i < length; i++)
    {
        uint8_t byte;
        if(inst->Debugger()->DebugLoad_VAddr<uint8_t>(addr + i, byte))
        {
            data[i] = byte;
        }
        else
        {
            return ThrowMemoryError(ctx, addr);
        }
    }

    return 1;
}

duk_ret_t ScriptAPI::js_mem_getstring(duk_context *ctx)
{
    CScriptInstance *inst = GetInstance(ctx);

    duk_idx_t nargs = duk_get_top(ctx);

    if(nargs < 1 ||
       !duk_is_number(ctx, 0) ||
       (nargs > 1 && !duk_is_number(ctx, 1)))
    {
        return ThrowInvalidArgsError(ctx);
    }

    duk_uint_t addr = duk_to_uint(ctx, 0);
    duk_uint_t maxLength = nargs > 1 ? duk_to_uint(ctx, 1) : 0xFFFFFFFF;
    size_t length = 0;

    for(size_t i = 0; i < maxLength; i++)
    {
        char c;
        if(!inst->Debugger()->DebugLoad_VAddr<char>(addr + i, c))
        {
            return ThrowMemoryError(ctx, addr);
        }

        if(c == 0)
        {
            break;
        }

        length++;
    }

    char *str = new char[length + 1];
    str[length] = '\0';

    for(size_t i = 0; i < length; i++)
    {
        if(!inst->Debugger()->DebugLoad_VAddr<char>(addr + i, str[i]))
        {
            delete[] str;
            return ThrowMemoryError(ctx, addr);
        }
    }

    duk_push_string(ctx, str);
    delete[] str;
    return 1;
}

duk_ret_t ScriptAPI::js_mem_setblock(duk_context *ctx) // address, data, maxLength
{
    CScriptInstance *inst = GetInstance(ctx);
    CDebuggerUI *debugger = inst->Debugger();

    duk_idx_t nargs = duk_get_top(ctx);

    if(nargs < 2 || nargs > 3 ||
       !duk_is_number(ctx, 0) ||
       (nargs == 3 && !duk_is_number(ctx, 2)))
    {
        return ThrowInvalidArgsError(ctx);
    }
    
    char* data;
    duk_size_t dataSize, length;

    uint32_t address = duk_get_uint(ctx, 0);

    if (duk_is_buffer_data(ctx, 1))
    {
        data = (char*)duk_get_buffer_data(ctx, 1, &dataSize);
    }
    else if(duk_is_string(ctx, 1))
    {
        data = (char*)duk_get_lstring(ctx, 1, &dataSize);
    }
    else
    {
        return ThrowInvalidArgsError(ctx);
    }

    if (nargs == 3)
    {
        duk_double_t l = duk_get_number(ctx, 2);

        if (l < 0 || l > dataSize)
        {
            return DUK_RET_RANGE_ERROR;
        }

        length = (duk_size_t)l;
    }
    else
    {
        length = dataSize;
    }

    for (size_t i = 0; i < length; i++)
    {
        if (!debugger->DebugStore_VAddr(address + i, data[i]))
        {
            return ThrowMemoryError(ctx, address + i);
        }
    }

    return 0;
}

duk_ret_t ScriptAPI::js_mem__boundget(duk_context *ctx)
{
    CDebuggerUI *debugger = GetInstance(ctx)->Debugger();

    uint32_t addr = duk_get_uint(ctx, 0);
    duk_int_t type = duk_get_int(ctx, 1);

    union {
        uint8_t u8;
        uint16_t u16;
        uint32_t u32;
        int8_t s8;
        int16_t s16;
        int32_t s32;
        float f32;
        double f64;
    } retval;

    #define MEM_BOUNDGET_TRY(addr, T, result, dukpush) \
        if(debugger->DebugLoad_VAddr<T>(addr, result)) { dukpush(ctx, result); } \
        else { goto memory_error; }
    
    switch(type)
    {
    case U8:
        MEM_BOUNDGET_TRY(addr, uint8_t, retval.u8, duk_push_uint);
        return 1;
    case U16:
        MEM_BOUNDGET_TRY(addr, uint16_t, retval.u16, duk_push_uint);
        return 1;
    case U32:
        MEM_BOUNDGET_TRY(addr, uint32_t, retval.u32, duk_push_uint);
        return 1;
    case S8:
        MEM_BOUNDGET_TRY(addr, int8_t, retval.s8, duk_push_int);
        return 1;
    case S16:
        MEM_BOUNDGET_TRY(addr, int16_t, retval.s16, duk_push_int);
        return 1;
    case S32:
        MEM_BOUNDGET_TRY(addr, int32_t, retval.s32, duk_push_int);
        return 1;
    case F32:
        MEM_BOUNDGET_TRY(addr, float, retval.f32, duk_push_number);
        return 1;
    case F64:
        MEM_BOUNDGET_TRY(addr, double, retval.f64, duk_push_number);
        return 1;
    }

memory_error:
    return ThrowMemoryError(ctx, addr);
}

duk_ret_t ScriptAPI::js_mem__boundset(duk_context *ctx)
{
    CDebuggerUI *debugger = GetInstance(ctx)->Debugger();

    uint32_t addr = duk_get_uint(ctx, 0);
    duk_int_t type = duk_get_int(ctx, 1);

    #define MEM_BOUNDSET_TRY(addr, T, value) \
        if(debugger->DebugStore_VAddr<T>(addr, value)) { return 1; } \
        else { goto memory_error; }

    switch(type)
    {
    case U8:
        MEM_BOUNDSET_TRY(addr, uint8_t, duk_get_uint(ctx, 2) & 0xFF);
        break;
    case U16:
        MEM_BOUNDSET_TRY(addr, uint16_t, duk_get_uint(ctx, 2) & 0xFFFF);
        break;
    case U32:
        MEM_BOUNDSET_TRY(addr, uint32_t, duk_get_uint(ctx, 2));
        break;
    case S8:
        MEM_BOUNDSET_TRY(addr, int8_t, duk_get_int(ctx, 2) & 0xFF);
        break;
    case S16:
        MEM_BOUNDSET_TRY(addr, int16_t, duk_get_int(ctx, 2) & 0xFFFF);
        break;
    case S32:
        MEM_BOUNDSET_TRY(addr, int32_t, duk_get_int(ctx, 2));
        break;
    case F32:
        MEM_BOUNDSET_TRY(addr, float, (float)duk_get_number(ctx, 2));
        break;
    case F64:
        MEM_BOUNDSET_TRY(addr, double, duk_get_number(ctx, 2));
        break;
    }

    return 0;

memory_error:
    return ThrowMemoryError(ctx, addr);
}

duk_ret_t ScriptAPI::js_mem_bindvar(duk_context *ctx)
{
    if(!duk_is_object(ctx, 0) ||
       !duk_is_number(ctx, 1) ||
       !duk_is_string(ctx, 2) ||
       !duk_is_number(ctx, 3))
    {
        return DUK_RET_TYPE_ERROR;
    }

    duk_uint_t addr = duk_get_uint(ctx, 1);
    const char* key = duk_get_string(ctx, 2);
    duk_int_t type = duk_get_int(ctx, 3);

    duk_push_string(ctx, key);

    // [ ... js_mem__boundget ] -> [ ... js_mem__boundget.bind(obj, addr, type) ]
    duk_push_c_function(ctx, js_mem__boundget, DUK_VARARGS);
    duk_push_string(ctx, "bind");
    duk_dup(ctx, 0);
    duk_push_uint(ctx, addr);
    duk_push_int(ctx, type);
    duk_call_prop(ctx, -5, 3);
    duk_remove(ctx, -2);

    duk_push_c_function(ctx, js_mem__boundset, DUK_VARARGS);
    duk_push_string(ctx, "bind");
    duk_dup(ctx, 0);
    duk_push_uint(ctx, addr);
    duk_push_int(ctx, type);
    duk_call_prop(ctx, -5, 3);
    duk_remove(ctx, -2);

    duk_def_prop(ctx, 0, DUK_DEFPROP_HAVE_GETTER | DUK_DEFPROP_HAVE_SETTER);

    return 0;
}

duk_ret_t ScriptAPI::js_mem_bindvars(duk_context *ctx)
{
    if(!duk_is_object(ctx, 0) ||
       !duk_is_array(ctx, 1))
    {
        return DUK_RET_TYPE_ERROR;
    }

    duk_size_t length = duk_get_length(ctx, 1);

    for(duk_uarridx_t i = 0; i < length; i++)
    {
        duk_get_prop_index(ctx, 1, i);
        if(!duk_is_array(ctx, -1) ||
           duk_get_length(ctx, -1) != 3)
        {
            return DUK_RET_TYPE_ERROR;
        }

        duk_push_c_function(ctx, js_mem_bindvar, 4);
        duk_dup(ctx, 0);
        duk_get_prop_index(ctx, -3, 0);
        duk_get_prop_index(ctx, -4, 1);
        duk_get_prop_index(ctx, -5, 2);

        if(duk_pcall(ctx, 4) != DUK_EXEC_SUCCESS)
        {
            return duk_throw(ctx);
        }

        duk_pop_n(ctx, 1);
    }

    duk_dup(ctx, 0);
    return 1;
}

duk_ret_t ScriptAPI::js_mem_bindstruct(duk_context *ctx)
{
    if(!duk_is_object(ctx, 0) ||
       !duk_is_number(ctx, 1) ||
       !duk_is_object(ctx, 2))
    {
        return DUK_RET_TYPE_ERROR;
    }

    uint32_t curAddr = duk_get_uint(ctx, 1);

    duk_enum(ctx, 2, DUK_ENUM_OWN_PROPERTIES_ONLY);

    while(duk_next(ctx, -1, 1))
    {
        //const char *name = duk_get_string(ctx, -2);
        mem_type_t type = (mem_type_t)duk_get_int(ctx, -1);

        duk_push_c_function(ctx, js_mem_bindvar, 4);
        duk_dup(ctx, 0);
        duk_push_uint(ctx, curAddr);
        duk_pull(ctx, -5);
        duk_pull(ctx, -5);

        if(duk_pcall(ctx, 4) != DUK_EXEC_SUCCESS)
        {
            return duk_throw(ctx);
        }

        duk_pop(ctx);

        curAddr += MemTypeSize(type);
    }

    duk_dup(ctx, 0);

    return 1;
}

duk_ret_t ScriptAPI::js_mem__type_constructor(duk_context* ctx)
{
    if(!duk_is_object(ctx, 0) || !duk_is_number(ctx, 1))
    {
        return DUK_RET_TYPE_ERROR;
    }

    duk_push_c_function(ctx, js_mem_bindstruct, 3);
    duk_push_this(ctx);
    duk_pull(ctx, 1);
    duk_pull(ctx, 0);

    if(duk_pcall(ctx, 3) != DUK_EXEC_SUCCESS)
    {
        return duk_throw(ctx);
    }

    return 0;
}

duk_ret_t ScriptAPI::js_mem_typedef(duk_context* ctx)
{
    if(!duk_is_object(ctx, 0))
    {
        return DUK_RET_TYPE_ERROR;
    }

    duk_push_c_function(ctx, js_mem__type_constructor, DUK_VARARGS);
    duk_push_string(ctx, "bind");
    duk_push_null(ctx);
    duk_dup(ctx, 0);
    duk_call_prop(ctx, -4, 2);
    return 1;
}

duk_ret_t ScriptAPI::js_mem__get_ramsize(duk_context* ctx)
{
    duk_push_number(ctx, g_MMU ? g_MMU->RdramSize() : 0);
    return 1;
}

duk_ret_t ScriptAPI::js_mem__get_romsize(duk_context* ctx)
{
    duk_push_number(ctx, g_Rom ? g_Rom->GetRomSize() : 0);
    return 1;
}

size_t ScriptAPI::MemTypeSize(mem_type_t t)
{
    switch(t)
    {
    case U8:
    case S8:
        return 1;
    case U16:
    case S16:
        return 2;
    case U32:
    case S32:
    case F32:
        return 4;
    case F64:
        return 8;
    }
    return 0;
}

duk_ret_t ScriptAPI::ThrowMemoryError(duk_context* ctx, uint32_t address)
{
    duk_push_error_object(ctx, DUK_ERR_ERROR, "memory error (can't access 0x%08X)", address);
    return duk_throw(ctx);
}
