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
    m_hThread = CreateThread(NULL, 0, ThreadProc, this, 0, NULL);
}

CScriptSystem::~CScriptSystem()
{
    m_CS.enter();
    SetCommand(CMD_SHUTDOWN);
    m_CS.leave();

    WaitForSingleObject(m_hThread, INFINITE);
    CloseHandle(m_hThread);
    CloseHandle(m_Cmd.hWakeEvent);
}

void CScriptSystem::Log(const char* message)
{
    CGuard guard(m_LogCS);
    stdstr formattedMsg = FixStringReturns(message) + "\r\n";
    m_Log += formattedMsg;
    m_Debugger->Debug_LogScriptsWindow(formattedMsg.c_str());
}

void CScriptSystem::Print(const char* message)
{
    CGuard guard(m_LogCS);
    stdstr formattedMsg = FixStringReturns(message);
    m_Log += formattedMsg;
    m_Debugger->Debug_LogScriptsWindow(formattedMsg.c_str());
}

void CScriptSystem::ClearLog()
{
    CGuard guard(m_LogCS);
    m_Log.clear();
    m_Debugger->Debug_ClearScriptsWindow();
}

stdstr CScriptSystem::GetLog()
{
    CGuard guard(m_LogCS);
    return stdstr(m_Log);
}

bool CScriptSystem::StartScript(const char *name, const char *path)
{
    CGuard guard(m_CS);
    if (m_Instances.count(name) != 0)
    {
        Log(stdstr_f("[ScriptSys]: START_SCRIPT aborted; '%s' is already instanced\n", name).c_str());
        return false;
    }

    SetCommand(CMD_START_SCRIPT, name, path);
    return true;
}

bool CScriptSystem::StopScript(const char *name)
{
    CGuard guard(m_CS);
    SetCommand(CMD_STOP_SCRIPT, name);
    return true;
}

bool CScriptSystem::Eval(const char *name, const char *code)
{
    CGuard guard(m_CS);
    SetCommand(CMD_EVAL, name, code);
    return true;
}

void CScriptSystem::UpdateStatus(const char* name, jsstatus_t status)
{
    CGuard guard(m_InstanceStatusCS);
    if (status == JS_STATUS_STOPPED)
    {
        m_InstanceStatus.erase(name);
    }
    else
    {
        m_InstanceStatus[name] = status;
    }
    m_Debugger->Debug_RefreshScriptsWindow();
}

jsstatus_t CScriptSystem::GetStatus(const char* name)
{
    CGuard guard(m_InstanceStatusCS);
    if (m_InstanceStatus.count(name) == 0)
    {
        return JS_STATUS_STOPPED;
    }
    else
    {
        return m_InstanceStatus[name];
    }
}

void CScriptSystem::_Invoke(jshook_id_t hookId, void* env)
{
    CGuard guard(m_CS);

    if(m_AppHooks.count(hookId) == 0 ||
       m_AppHooks[hookId].size() == 0)
    {
        return;
    }

    bool bDidExec = false;
    jscb_map_t& callbacks = m_AppHooks[hookId];

    jscb_map_t::iterator it;
    for(it = callbacks.begin(); it != callbacks.end(); it++)
    {
        jscallback_t& cb = it->second;
        if(env == NULL || cb.fnCondition(&cb, env))
        {
            cb.inst->RawCall(cb.heapptr, cb.fnPushArgs, env);
            bDidExec = true;
        }
    }

    SetCommand(bDidExec ? CMD_SWEEP : CMD_IDLE);
}

void CScriptSystem::SyncCall(CScriptInstance *inst, void *heapptr, jsargs_fn_t fnPushArgs, void *argsParam)
{
    CGuard guard(m_CS);

    inst->RawCall(heapptr, fnPushArgs, argsParam);

    if(inst->GetRefCount() == 0)
    {
        SetCommand(CMD_STOP_SCRIPT, inst->Name().c_str());
    }
}

void CScriptSystem::SetCommand(jssyscmd_id_t cmd, const char *paramA, const char *paramB)
{
    m_Cmd.id = cmd;
    m_Cmd.paramA = paramA;
    m_Cmd.paramB = paramB;
    SetEvent(m_Cmd.hWakeEvent);
}

DWORD CScriptSystem::ThreadProc(void *_this)
{
    ((CScriptSystem *)_this)->ThreadProc();
    return 0;
}

void CScriptSystem::ThreadProc()
{
    bool bRunning = true;

    while(bRunning)
    {
        WaitForSingleObject(m_Cmd.hWakeEvent, INFINITE);
        CGuard guard(m_CS);

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
        ResetEvent(m_Cmd.hWakeEvent);
    }
}

void CScriptSystem::OnStartScript(const char *name, const char *path)
{
    CScriptInstance *inst = new CScriptInstance(this, name);
    
    UpdateStatus(name, JS_STATUS_STARTING);

    if(inst->Run(path) && inst->GetRefCount() > 0)
    {
        m_Instances[name] = inst;
        UpdateStatus(name, JS_STATUS_STARTED);
    }
    else
    {
        UpdateStatus(name, JS_STATUS_STOPPED);
        delete inst;
    }
}

void CScriptSystem::OnStopScript(const char *name)
{
    UpdateStatus(name, JS_STATUS_STOPPED);
    RawRemoveInstance(name);
}

void CScriptSystem::OnEval(const char *name, const char *code)
{
    if(m_Instances.count(name) == 0)
    {
        Log(stdstr_f("[ScriptSys]: error: eval aborted; instance '%s' does not exist\n", name).c_str());
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
    
    m_AppHooks[hookId].erase(callbackId);
    m_AppCallbackCount--;
    return true;
}

CDebuggerUI* CScriptSystem::Debugger()
{
    return m_Debugger;
}

stdstr CScriptSystem::FixStringReturns(const char* str)
{
    stdstr fstr = str;
    size_t pos = 0;
    while ((pos = fstr.find("\n", pos)) != stdstr::npos)
    {
        fstr.replace(pos, 1, "\r\n");
        pos += 2;
    }
    return fstr;
}