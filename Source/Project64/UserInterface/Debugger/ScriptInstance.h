#include "ScriptTypes.h"
#include "ScriptSystem.h"

#pragma once

class CScriptInstance
{
    jsname_t       m_Name;
    CScriptSystem* m_System;
    duk_context*   m_Ctx;
    size_t         m_RefCount;
    uint64_t       m_ExecTimeout;
    uint64_t       m_ExecStartTime;
    std::ifstream  m_SourceFile;
    char*          m_SourceCode;
    jscb_id_t      m_CurExecCallbackId;

public:
    CScriptInstance(CScriptSystem* sys, const char* name);
    ~CScriptInstance();

    jsname_t&      Name();
    CScriptSystem* System();
    CDebuggerUI*   Debugger();
    jscb_id_t      CallbackId();

    bool           Run(const char* path);

    size_t         GetRefCount();
    void           IncRefCount();
    void           DecRefCount();

    void           ConditionalInvokeCallback(JSCallback& cb, void *_hookEnv);

    void           RawCall(void* heapptr, jsargs_fn_t fnPushArgs, void* param);
    void           SyncCall(void* heapptr, jsargs_fn_t fnPushArgs, void* param);
    void           RawInput(const char* code);

    void SetExecTimeout(uint64_t timeout);
    bool IsTimedOut();

private:
    static uint64_t Timestamp();
    void Cleanup();
};
