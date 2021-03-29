#include <stdafx.h>
#include "ScriptTypes.h"
#include "ScriptInstance.h"
#include "ScriptAPI.h"

#include <sys/stat.h>

extern "C" {
    int DukTimeoutCheck(void* udata)
    {
        CScriptInstance* inst = (CScriptInstance*)udata;
        return (int)inst->IsTimedOut();
    }
}

CScriptInstance::CScriptInstance(CScriptSystem* sys, const char* name) :
    m_System(sys),
    m_Name(name),
    m_Ctx(NULL),
    m_RefCount(0),
    m_ExecTimeout(JS_EXEC_TIMEOUT),
    m_ExecStartTime(0),
    m_SourceCode(NULL)
{
}

CScriptInstance::~CScriptInstance()
{
    Cleanup();
}

std::string& CScriptInstance::Name()
{
    return m_Name;
}

CScriptSystem* CScriptInstance::System()
{
    return m_System;
}

CDebuggerUI* CScriptInstance::Debugger()
{
    return m_System->Debugger();
}

bool CScriptInstance::Run(const char* path)
{
    if(m_Ctx != NULL)
    {
        return false;
    }

    m_Ctx = duk_create_heap(NULL, NULL, NULL, this, NULL);

    if(m_Ctx == NULL)
    {
        goto error_cleanup;
    }

    struct stat statBuf;
    if(stat(path, &statBuf) != 0)
    {
        Debugger()->Debug_LogScriptsWindow(stdstr_f("[ScriptSys]: error: could not stat '%s'\n", path).c_str());
        goto error_cleanup;
    }

    m_SourceCode = new char[statBuf.st_size + 1];
    m_SourceCode[statBuf.st_size] = '\0';

    m_SourceFile.open(path, std::ios::in | std::ios::binary);
    if(!m_SourceFile.is_open())
    {
        Debugger()->Debug_LogScriptsWindow(stdstr_f("[ScriptSys]: error: could not open '%s'\n", path).c_str());
        goto error_cleanup;
    }

    m_SourceFile.read(m_SourceCode, statBuf.st_size);

    if((size_t)m_SourceFile.tellg() != statBuf.st_size)
    {
        Debugger()->Debug_LogScriptsWindow(stdstr_f("[ScriptSys]: error: failed to read '%s'\n", path).c_str());
        goto error_cleanup;
    }

    m_ExecStartTime = Timestamp();

    ScriptAPI::InitEnvironment(m_Ctx, this);

    duk_push_string(m_Ctx, m_Name.c_str());
    if(duk_pcompile_string_filename(m_Ctx, 0, m_SourceCode) != 0 ||
       duk_pcall(m_Ctx, 0) == DUK_EXEC_ERROR)
    {
        duk_get_prop_string(m_Ctx, -1, "stack");
        Debugger()->Debug_LogScriptsWindow(stdstr_f("%s\n", duk_safe_to_string(m_Ctx, -1)).c_str());
        duk_pop_n(m_Ctx, 2);
        goto error_cleanup;
    }

    duk_pop(m_Ctx);

    return true;

error_cleanup:
    Cleanup();
    return false;
}

size_t CScriptInstance::GetRefCount()
{
    return m_RefCount;
}

void CScriptInstance::IncRefCount()
{
    m_RefCount++;
}

void CScriptInstance::DecRefCount()
{
    if(m_RefCount > 0)
    {
        m_RefCount--;
    }
}

void CScriptInstance::RawCall(void *heapptr, jsargs_fn_t fnPushArgs, void *param)
{
    m_ExecStartTime = Timestamp();
    duk_push_heapptr(m_Ctx, heapptr);
    duk_idx_t nargs = fnPushArgs ? fnPushArgs(m_Ctx, param) : 0;

    if(duk_pcall(m_Ctx, nargs) == DUK_EXEC_ERROR)
    {
        duk_get_prop_string(m_Ctx, -1, "stack");
        Debugger()->Debug_LogScriptsWindow(stdstr_f("%s\n", duk_safe_to_string(m_Ctx, -1)).c_str());
        duk_pop(m_Ctx);
    }

    duk_pop(m_Ctx);
}

void CScriptInstance::SyncCall(void *heapptr, jsargs_fn_t fnPushArgs, void *param)
{
    m_System->SyncCall(this, heapptr, fnPushArgs, NULL);
}

/*
RawInput(const char* code)
{
    m_ExecStartTime = Timestamp();

}
*/

void CScriptInstance::RawEval(const char* code)
{
    duk_push_global_object(m_Ctx);
    duk_get_prop_string(m_Ctx, -1, DUK_HIDDEN_SYMBOL("INPUT_LISTENER"));
    if (duk_is_function(m_Ctx, -1))
    {
        m_ExecStartTime = Timestamp();
        duk_push_string(m_Ctx, code);
        if (duk_pcall(m_Ctx, 1) != DUK_EXEC_SUCCESS)
        {
            duk_get_prop_string(m_Ctx, -1, "stack");
            Debugger()->Debug_LogScriptsWindow(stdstr_f("%s\n", duk_safe_to_string(m_Ctx, -1)).c_str());
            duk_pop_n(m_Ctx, 3);
            return;
        }
        else
        {
            duk_pop_n(m_Ctx, 2);
            return;
        }
    }

    duk_pop_n(m_Ctx, 2);

    m_ExecStartTime = Timestamp();
    if(duk_peval_string(m_Ctx, code) == 0)
    {
        Debugger()->Debug_LogScriptsWindow(stdstr_f("%s\n", duk_safe_to_string(m_Ctx, -1)).c_str());
    }
    else
    {
        duk_get_prop_string(m_Ctx, -1, "stack");
        Debugger()->Debug_LogScriptsWindow(stdstr_f("%s\n", duk_safe_to_string(m_Ctx, -1)).c_str());
        duk_pop(m_Ctx);
    }
    duk_pop(m_Ctx);
}

void CScriptInstance::SetExecTimeout(size_t timeout)
{
    m_ExecTimeout = timeout;
}

bool CScriptInstance::IsTimedOut()
{
    if(m_ExecStartTime == 0 || m_ExecTimeout == 0)
    {
        return false;
    }

    uint64_t timeElapsed = Timestamp() - m_ExecStartTime;
    return (timeElapsed >= m_ExecTimeout);
}

uint64_t CScriptInstance::Timestamp()
{
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);

    ULARGE_INTEGER li;
    li.LowPart = ft.dwLowDateTime;
    li.HighPart = ft.dwHighDateTime;

    return li.QuadPart / 10000;
}

void CScriptInstance::Cleanup()
{
    if(m_Ctx != NULL)
    {
        duk_destroy_heap(m_Ctx);
        m_Ctx = NULL;
    }
    if(m_SourceCode != NULL)
    {
        delete[] m_SourceCode;
        m_SourceCode = NULL;
    }
    if(m_SourceFile.is_open())
    {
        m_SourceFile.close();
    }
}
