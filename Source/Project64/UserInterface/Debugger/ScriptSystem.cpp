#include <sys/stat.h>

#include <stdafx.h>
#include "ScriptTypes.h"
#include "ScriptSystem.h"
#include "ScriptInstance.h"
#include "ScriptAPI.h"
#include "Debugger.h"

CScriptSystem::CScriptSystem(CDebuggerUI *debugger) :
    m_Debugger(debugger),
    m_NextAppCallbackId(0),
    m_AppCallbackCount(0)
{
    m_Cmd.id = CMD_IDLE;
    m_Cmd.hWakeEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    m_Cmd.hIdleEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
    m_hMutex = CreateMutex(NULL, FALSE, NULL);
    m_hThread = CreateThread(NULL, 0, ThreadProc, this, 0, NULL);
}

CScriptSystem::~CScriptSystem()
{
    m_Debugger->Debug_LogScriptsWindow("[ScriptSys]: shutting down\n");

    LockCommand(true);
    SetCommand(CMD_SHUTDOWN);
    UnlockCommand();
    WaitForSingleObject(m_hThread, INFINITE);
    CloseHandle(m_hThread);
    CloseHandle(m_hMutex);
    CloseHandle(m_Cmd.hWakeEvent);
    CloseHandle(m_Cmd.hIdleEvent);
}

bool CScriptSystem::StartScript(const char *name, const char *path)
{
    m_Debugger->Debug_LogScriptsWindow("[ScriptSys]: starting script...\n");

    if(!LockCommand())
    {
        m_Debugger->Debug_LogScriptsWindow("[ScriptSys]: error: START_SCRIPT timed out; system is busy\n");
        return false;
    }

    if(m_Instances.count(name) != 0)
    {
        
        m_Debugger->Debug_LogScriptsWindow(stdstr_f("[ScriptSys]: error: START_SCRIPT aborted; '%s' is already instanced\n", name).c_str());
        UnlockCommand();
        return false;
    }

    SetCommand(CMD_START_SCRIPT, name, path);
    UnlockCommand();
    return true;
}

bool CScriptSystem::StopScript(const char *name)
{
    if(!LockCommand())
    {
        m_Debugger->Debug_LogScriptsWindow("[ScriptSys]: error: STOP_SCRIPT timed out; system is busy\n");
        return false;
    }
    SetCommand(CMD_STOP_SCRIPT, name);
    UnlockCommand();
    return true;
}

bool CScriptSystem::Eval(const char *name, const char *code)
{
    if(!LockCommand())
    {
        m_Debugger->Debug_LogScriptsWindow("[ScriptSys]: error: EVAL timed out; system is busy\n");
        return false;
    }
    SetCommand(CMD_EVAL, name, code);
    UnlockCommand();
    return true;
}

jsstatus_t CScriptSystem::GetStatus(const char* name)
{
    if (WaitForSingleObject(m_hMutex, JS_EXEC_TIMEOUT) == WAIT_TIMEOUT)
    {
        m_Debugger->Debug_LogScriptsWindow("[ScriptSys]: error: status check operation timed out\n");
        return JS_STATUS_STOPPED;
    }

    if (m_Instances.count(name) != 0)
    {
        ReleaseMutex(m_hMutex);
        return JS_STATUS_STARTED;
    }
    else
    {
        ReleaseMutex(m_hMutex);
        return JS_STATUS_STOPPED;
    }
}

void CScriptSystem::_Invoke(jshook_id_t hookId, void* env)
{
    if(!LockCommand())
    {
        m_Debugger->Debug_LogScriptsWindow("[ScriptSys]: error: app callback invocation timed out; system is busy\n");
    }

    if(m_AppHooks.count(hookId) == 0 ||
       m_AppHooks[hookId].size() == 0)
    {
        UnlockCommand();
        return;
    }

    jscb_map_t& callbacks = m_AppHooks[hookId];

    jscb_map_t::iterator it;
    for(it = callbacks.begin(); it != callbacks.end(); it++)
    {
        jscallback_t& cb = it->second;
        if(env == NULL || cb.fnCondition(&cb, env))
        {
            cb.inst->RawCall(cb.heapptr, cb.fnPushArgs, env);
        }
    }

    SetCommand(CMD_SWEEP);
    UnlockCommand();
}

void CScriptSystem::SyncCall(CScriptInstance *inst, void *heapptr, jsargs_fn_t fnPushArgs, void *argsParam)
{
    if(!LockCommand())
    {
        m_Debugger->Debug_LogScriptsWindow("[ScriptSys]: error: worker callback invocation timed out; system is busy\n");
        return;
    }

    inst->RawCall(heapptr, fnPushArgs, argsParam);

    if(inst->GetRefCount() == 0)
    {
        SetCommand(CMD_STOP_SCRIPT, inst->Name().c_str());
    }

    UnlockCommand();
}

bool CScriptSystem::LockCommand(bool bWaitInfinite)
{
    HANDLE handles[] = { m_hMutex, m_Cmd.hIdleEvent };
    if(bWaitInfinite)
    {
        WaitForMultipleObjects(2, handles, TRUE, INFINITE);
        return true;
    }
    return WaitForMultipleObjects(2, handles, TRUE, JS_EXEC_TIMEOUT) != WAIT_TIMEOUT;
}

void CScriptSystem::UnlockCommand()
{
    ResetEvent(m_Cmd.hIdleEvent);
    SetEvent(m_Cmd.hWakeEvent);
    ReleaseMutex(m_hMutex);
}

void CScriptSystem::SetCommand(jssyscmd_id_t cmd, const char *paramA, const char *paramB)
{
    m_Cmd.id = cmd;
    m_Cmd.paramA = paramA;
    m_Cmd.paramB = paramB;
}

DWORD CScriptSystem::ThreadProc(void *_this)
{
    ((CScriptSystem *)_this)->ThreadProc();
    return 0;
}

void CScriptSystem::ThreadProc()
{
    HANDLE handles[] = { m_Cmd.hWakeEvent, m_hMutex };
    bool bRunning = true;

    while(bRunning)
    {
        WaitForMultipleObjects(2, handles, TRUE, INFINITE);

        switch(m_Cmd.id)
        {
        case CMD_START_SCRIPT:
            OnStartScript(m_Cmd.paramA.c_str(), m_Cmd.paramB.c_str());
            break;
        case CMD_STOP_SCRIPT:
            OnStopScript(m_Cmd.paramA.c_str());
            break;
        case CMD_EVAL:
            OnEval(m_Cmd.paramA.c_str(), m_Cmd.paramB.c_str());
            break;
        case CMD_SWEEP:
            OnSweep(true);
            break;
        case CMD_SHUTDOWN:
            OnSweep(false);
            bRunning = false;
            break;
        }

        m_Cmd.id = CMD_IDLE;
        SetEvent(m_Cmd.hIdleEvent);
        ResetEvent(m_Cmd.hWakeEvent);
        ReleaseMutex(m_hMutex);
    }
}

void CScriptSystem::OnStartScript(const char *name, const char *path)
{
    CScriptInstance *inst = new CScriptInstance(this, name);
    
    if(inst->Run(path) && inst->GetRefCount() > 0)
    {
        // SendMessage(m_hScriptWnd, WM_JS_STATUS, (WPARAM)STATUS_STARTED, (LPARAM)name);
        m_Instances[name] = inst;
    }
    else
    {
        delete inst;
    }
}

void CScriptSystem::OnStopScript(const char *name)
{
    RawRemoveInstance(name);
}

void CScriptSystem::OnEval(const char *name, const char *code)
{
    if(m_Instances.count(name) == 0)
    {
        m_Debugger->Debug_LogScriptsWindow(stdstr_f("[ScriptSys]: error: eval aborted; instance '%s' does not exist\n", name).c_str());
        return;
    }

    CScriptInstance* inst = m_Instances[name];

    inst->RawEval(code);

    if(inst->GetRefCount() == 0)
    {
        RawRemoveInstance(name);
    }
}

void CScriptSystem::OnSweep(bool bIfDone)
{
    jsinst_map_t::iterator it = m_Instances.begin();
    while(it != m_Instances.end())
    {
        CScriptInstance*& inst = it->second;
        if(!bIfDone || inst->GetRefCount() == 0)
        {
            delete inst;
            m_Instances.erase(it++);
        }
        else
        {
            it++;
        }
    }
}

bool CScriptSystem::RawRemoveInstance(const char *name)
{
    if(m_Instances.count(name) == 0)
    {
        return false;
    }

    CScriptInstance*& inst = m_Instances[name];
    delete inst;
    m_Instances.erase(name);
    return true;
}

jscb_id_t CScriptSystem::RawAddCallback(jshook_id_t hookId, jscallback_t& callback)
{
    if(hookId >= JS_NUM_APP_HOOKS)
    {
        return JS_INVALID_CALLBACK;
    }

    m_Debugger->Debug_LogScriptsWindow(stdstr_f("[ScriptSys]: '%s' added callback %d to hook %d\n", callback.inst->Name().c_str(), m_NextAppCallbackId, hookId).c_str());

    m_AppHooks[hookId][m_NextAppCallbackId] = callback;
    m_AppCallbackCount++;
    return m_NextAppCallbackId++;
}

bool CScriptSystem::RawRemoveCallback(jshook_id_t hookId, jscb_id_t callbackId)
{
    if(m_AppHooks[hookId].count(callbackId) == 0)
    {
        return false;
    }

    m_Debugger->Debug_LogScriptsWindow(stdstr_f("[ScriptSys]: '%s' removed callback %d from hook %d\n", m_AppHooks[hookId][callbackId].inst->Name().c_str(), callbackId, hookId).c_str());

    m_AppHooks[hookId].erase(callbackId);
    m_AppCallbackCount--;
    return true;
}

CDebuggerUI* CScriptSystem::Debugger()
{
    return m_Debugger;
}
