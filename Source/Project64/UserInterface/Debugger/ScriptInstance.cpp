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
    m_SourceCode(NULL),
    m_CurExecCallbackId(JS_INVALID_CALLBACK)
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

jscb_id_t CScriptInstance::CallbackId()
{
    return m_CurExecCallbackId;
}

bool CScriptInstance::Run(const char* path)
{
    if(m_Ctx != NULL)
    {
        return false;
    }

    m_Ctx = duk_create_heap(NULL, NULL, NULL, this, NULL);
    duk_module_duktape_init(m_Ctx);

    if(m_Ctx == NULL)
    {
        goto error_cleanup;
    }

    struct stat statBuf;
    if(stat(path, &statBuf) != 0)
    {
        m_System->Log("[SCRIPTSYS]: error: could not stat '%s'", path);
        goto error_cleanup;
    }

    m_SourceCode = new char[statBuf.st_size + 1];
    m_SourceCode[statBuf.st_size] = '\0';

    m_SourceFile.open(path, std::ios::in | std::ios::binary);
    if(!m_SourceFile.is_open())
    {
        m_System->Log("[SCRIPTSYS]: error: could not open '%s'", path);
        goto error_cleanup;
    }

    m_SourceFile.read(m_SourceCode, statBuf.st_size);

    if((size_t)m_SourceFile.tellg() != statBuf.st_size)
    {
        m_System->Log("[SCRIPTSYS]: error: could not read '%s'", path);
        goto error_cleanup;
    }

    m_ExecStartTime = Timestamp();

    ScriptAPI::InitEnvironment(m_Ctx, this);

    duk_push_string(m_Ctx, m_Name.c_str());
    if(duk_pcompile_string_filename(m_Ctx, 0, m_SourceCode) != 0 ||
       duk_pcall(m_Ctx, 0) == DUK_EXEC_ERROR)
    {
        duk_get_prop_string(m_Ctx, -1, "stack");
        m_System->Log("%s", duk_safe_to_string(m_Ctx, -1));
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
        m_System->Log("%s", duk_safe_to_string(m_Ctx, -1));
        duk_pop(m_Ctx);
    }

    duk_pop(m_Ctx);
}

void CScriptInstance::ConditionalInvokeCallback(JSCallback& cb, void* _hookEnv)
{
    if (cb.Condition != NULL && !cb.Condition(&cb, _hookEnv))
    {
        return;
    }

    m_CurExecCallbackId = cb.id;
    RawCall(cb.heapptr, cb.PushArguments, _hookEnv);
    m_CurExecCallbackId = JS_INVALID_CALLBACK;
}

void CScriptInstance::SyncCall(void *heapptr, jsargs_fn_t fnPushArgs, void* /*param*/)
{
    m_System->SyncCall(this, heapptr, fnPushArgs, NULL);
}

void CScriptInstance::RawInput(const char* code)
{
    m_System->Log("> %s", code);

    duk_push_global_object(m_Ctx);
    duk_get_prop_string(m_Ctx, -1, DUK_HIDDEN_SYMBOL("INPUT_LISTENER"));
    if (duk_is_function(m_Ctx, -1))
    {
        m_ExecStartTime = Timestamp();
        duk_push_string(m_Ctx, code);
        if (duk_pcall(m_Ctx, 1) != DUK_EXEC_SUCCESS)
        {
            duk_get_prop_string(m_Ctx, -1, "stack");
            m_System->Log("%s", duk_safe_to_string(m_Ctx, -1));
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
    duk_push_string(m_Ctx, stdstr_f("<eval:%s>", m_Name.c_str()).c_str());
    if (duk_pcompile_string_filename(m_Ctx, 0, code) != 0 ||
        duk_pcall(m_Ctx, 0) != DUK_EXEC_SUCCESS)
    {
        duk_get_prop_string(m_Ctx, -1, "stack");
        m_System->Log("%s", duk_safe_to_string(m_Ctx, -1));
        duk_pop(m_Ctx);
    }
    else
    {
        m_System->Log("%s", duk_safe_to_string(m_Ctx, -1));
    }
    duk_pop(m_Ctx);
}

void CScriptInstance::SetExecTimeout(uint64_t timeout)
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
