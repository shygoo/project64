#include "ScriptTypes.h"
#include "ScriptSystem.h"
#include "ScriptInstance.h"

#pragma once

namespace ScriptAPI
{
    // ScriptAPI
    void InitEnvironment(duk_context* ctx, CScriptInstance* inst);
    void DefineGlobalConstants(duk_context* ctx);
    
    CScriptInstance* GetInstance(duk_context* ctx);
        
    jscb_id_t AddCallback(duk_context* ctx, jshook_id_t hookId, jscallback_t& callback);
    bool RemoveCallback(duk_context* ctx, jscb_id_t callbackId);
    duk_ret_t CallbackFinalizer(duk_context* ctx);

    duk_ret_t ThrowInvalidArgsError(duk_context* ctx);

    // ScriptAPI_events
    void Define_events(duk_context* ctx);
    duk_ret_t js_events_onexec(duk_context* ctx);
    duk_ret_t js_events_onread(duk_context* ctx); // *
    duk_ret_t js_events_onwrite(duk_context* ctx); // *
    duk_ret_t js_events_onopcode(duk_context* ctx); // *
    duk_ret_t js_events_ongprvalue(duk_context* ctx); // *
    duk_ret_t js_events_ondraw(duk_context* ctx); // *
    //duk_ret_t js_events_oncommand(duk_context* ctx);
    duk_ret_t js_events_remove(duk_context* ctx);

    // ScriptAPI_console
    void Define_console(duk_context* ctx);
    duk_ret_t js_console_print(duk_context* ctx);
    duk_ret_t js_console_log(duk_context* ctx);
    duk_ret_t js_console_clear(duk_context* ctx);

    // ScriptAPI_mem
    void Define_mem(duk_context* ctx);
    template <class T> duk_ret_t js_mem__get(duk_context* ctx);
    template <class T> duk_ret_t js_mem__set(duk_context* ctx);
    duk_ret_t js_mem_getblock(duk_context* ctx);
    duk_ret_t js_mem_getstring(duk_context* ctx);
    duk_ret_t js_mem_bindvar(duk_context* ctx);
    duk_ret_t js_mem__boundget(duk_context* ctx);
    duk_ret_t js_mem__boundset(duk_context* ctx);
    duk_ret_t js_mem_bindvars(duk_context* ctx);
    duk_ret_t js_mem_bindstruct(duk_context* ctx);
    duk_ret_t js_mem_typedef(duk_context* ctx);
    duk_ret_t js_mem__type_constructor(duk_context* ctx);
    enum mem_type_t { U8, U16, U32, S8, S16, S32, F32, F64 };
    size_t MemTypeSize(mem_type_t t);
    duk_ret_t ThrowMemoryError(duk_context* ctx, uint32_t address);

    // ScriptAPI_Server
    //void Define_Server(duk_context* ctx);
    //duk_ret_t js_Server__constructor(duk_context* ctx);
    //duk_ret_t js_Server__finalizer(duk_context* ctx);
    //duk_ret_t js_Server_address(duk_context* ctx);
    //duk_ret_t js_Server_on(duk_context* ctx);
    //duk_ret_t js_Server_listen(duk_context* ctx);

    // ScriptAPI_script
    void Define_script(duk_context* ctx);
    duk_ret_t js_script_keepalive(duk_context* ctx);
    duk_ret_t js_script_timeout(duk_context* ctx);
    duk_ret_t js_script_listen(duk_context* ctx);

    // ScriptAPI_Number_hex
    void Define_Number_prototype_hex(duk_context* ctx);
    duk_ret_t js_Number_prototype_hex(duk_context* ctx);

    // ScriptAPI_fs
    void Define_fs(duk_context* ctx);
    duk_ret_t js_fs_open(duk_context* ctx);
    duk_ret_t js_fs_close(duk_context* ctx);
    duk_ret_t js_fs_write(duk_context* ctx);
    duk_ret_t js_fs_writeFile(duk_context* ctx);
    duk_ret_t js_fs_read(duk_context* ctx);
    duk_ret_t js_fs_readFile(duk_context* ctx);
    duk_ret_t js_fs_fstat(duk_context* ctx);
    duk_ret_t js_fs_stat(duk_context* ctx);
    duk_ret_t js_fs_unlink(duk_context* ctx);
    duk_ret_t js_fs_mkdir(duk_context* ctx);
    duk_ret_t js_fs_rmdir(duk_context* ctx);
    duk_ret_t js_fs_readdir(duk_context* ctx);
    duk_ret_t js_fs_Stats__constructor(duk_context* ctx);
    duk_ret_t js_fs_Stats_isDirectory(duk_context* ctx);
    duk_ret_t js_fs_Stats_isFile(duk_context* ctx);

    // ScriptAPI_AddressRange
    void Define_AddressRange(duk_context* ctx);
    duk_ret_t js_AddressRange__constructor(duk_context* ctx);

    // ScriptAPI_debug
    //void Define_debug(duk_context* ctx);
    //duk_ret_t js_debug_breakhere(duk_context* ctx);

    // ScriptAPI_asm
    //void Define_asm(duk_context* ctx);
    //duk_ret_t js_asm_gprname(duk_context* ctx);

    // ScriptAPI_alert
    //void Define_alert(duk_context* ctx);
    //duk_ret_t js_alert(duk_context* ctx);

    // ScriptAPI_registers (gpr, ugpr, fpr, dfpr, cop0)
    //void Define_registers(duk_context* ctx);
    //duk_ret_t js_gpr_get(duk_context* ctx);
    //duk_ret_t js_gpr_set(duk_context* ctx);
    //duk_ret_t js_ugpr_get(duk_context* ctx);
    //duk_ret_t js_ugpr_set(duk_context* ctx);
    //duk_ret_t js_fpr_get(duk_context* ctx);
    //duk_ret_t js_fpr_set(duk_context* ctx);
    //duk_ret_t js_dfpr_get(duk_context* ctx);
    //duk_ret_t js_dfpr_set(duk_context* ctx);

    // ScriptAPI_screen
    //duk_ret_t js_screen(duk_context* ctx);

    enum {
        R0, AT, V0, V1, A0, A1, A2, A3, T0, T1, T2, T3, T4, T5, T6, T7,
        S0, S1, S2, S3, S4, S5, S6, S7, T8, T9, K0, K1, GP, SP, FP, RA,
        //S8 = FP
    };

    enum {
        GPR_R0 = (1 << R0),
        GPR_AT = (1 << AT),
        GPR_V0 = (1 << V0),
        GPR_V1 = (1 << V1),
        GPR_A0 = (1 << A0),
        GPR_A1 = (1 << A1),
        GPR_A2 = (1 << A2),
        GPR_A3 = (1 << A3),
        GPR_T0 = (1 << T0),
        GPR_T1 = (1 << T1),
        GPR_T2 = (1 << T2),
        GPR_T3 = (1 << T3),
        GPR_T4 = (1 << T4),
        GPR_T5 = (1 << T5),
        GPR_T6 = (1 << T6),
        GPR_T7 = (1 << T7),
        GPR_S0 = (1 << S0),
        GPR_S1 = (1 << S1),
        GPR_S2 = (1 << S2),
        GPR_S3 = (1 << S3),
        GPR_S4 = (1 << S4),
        GPR_S5 = (1 << S5),
        GPR_S6 = (1 << S6),
        GPR_S7 = (1 << S7),
        GPR_T8 = (1 << T8),
        GPR_T9 = (1 << T9),
        GPR_K0 = (1 << K0),
        GPR_K1 = (1 << K1),
        GPR_GP = (1 << GP),
        GPR_SP = (1 << SP),
        GPR_FP = (1 << FP),
        GPR_RA = (1 << RA),
        //GPR_S8 = (1 << S8),
        GPR_ANY = 0xFFFFFFFF
    };
};
