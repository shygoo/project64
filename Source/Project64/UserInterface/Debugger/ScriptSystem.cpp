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
    m_hCmdEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    m_hThread = CreateThread(NULL, 0, ThreadProc, this, 0, NULL);
}

CScriptSystem::~CScriptSystem()
{
    PostCommand(CMD_SHUTDOWN);
    WaitForSingleObject(m_hThread, INFINITE);
    CloseHandle(m_hThread);
    CloseHandle(m_hCmdEvent);
}

jsstatus_t CScriptSystem::GetStatus(const char* name)
{
    CGuard guard(m_UIStateCS);
    if (m_UIInstanceStatus.count(name) == 0)
    {
        return JS_STATUS_STOPPED;
    }
    else
    {
        return m_UIInstanceStatus[name];
    }
}

void CScriptSystem::UpdateStatus(const char* name, jsstatus_t status)
{
    CGuard guard(m_UIStateCS);
    if (status == JS_STATUS_STOPPED)
    {
        m_UIInstanceStatus.erase(name);
    }
    else
    {
        m_UIInstanceStatus[name] = status;
    }
    m_Debugger->Debug_RefreshScriptsWindow();
}

void CScriptSystem::Log(const char* format, ...)
{
    CGuard guard(m_UIStateCS);

    va_list args;
    va_start(args, format);

    int size = vsnprintf(NULL, 0, format, args) + 1;
    char* str = new char[size];
    vsnprintf(str, size, format, args);

    stdstr formattedMsg = FixStringReturns(str) + "\r\n";
    
    m_Debugger->Debug_LogScriptsWindow(formattedMsg.c_str());
    m_UILog += formattedMsg;

    delete[] str;
    va_end(args);
}

void CScriptSystem::Print(const char* format, ...)
{
    CGuard guard(m_UIStateCS);

    va_list args;
    va_start(args, format);

    int size = vsnprintf(NULL, 0, format, args) + 1;
    char* str = new char[size];
    vsnprintf(str, size, format, args);
    
    stdstr formattedMsg = FixStringReturns(str);
    
    m_Debugger->Debug_LogScriptsWindow(formattedMsg.c_str());
    m_UILog += formattedMsg;

    delete[] str;
    va_end(args);
}

void CScriptSystem::ClearLog()
{
    CGuard guard(m_UIStateCS);
    m_UILog.clear();
    m_Debugger->Debug_ClearScriptsWindow();
}

stdstr CScriptSystem::GetLog()
{
    CGuard guard(m_UIStateCS);
    return stdstr(m_UILog);
}

void CScriptSystem::StartScript(const char *name, const char *path)
{
    PostCommand(CMD_START_SCRIPT, name, path);
}

void CScriptSystem::StopScript(const char *name)
{
    PostCommand(CMD_STOP_SCRIPT, name);
}

void CScriptSystem::Input(const char *name, const char *code)
{
    PostCommand(CMD_INPUT, name, code);
}

void CScriptSystem::Invoke(jshook_id_t hookId, void* env)
{
    CGuard guard(m_InstancesCS);

    if (m_AppCallbackHooks.count(hookId) == 0 ||
        m_AppCallbackHooks[hookId].size() == 0)
    {
        return;
    }

    bool bNeedSweep = false;
    jscb_map_t& callbacks = m_AppCallbackHooks[hookId];

    jscb_map_t::iterator it;
    for (it = callbacks.begin(); it != callbacks.end(); it++)
    {
        JSCallback& cb = it->second;
        cb.instance->ConditionalInvokeCallback(cb, env);
    }

    if (bNeedSweep)
    {
        PostCommand(CMD_SWEEP);
    }
}

void CScriptSystem::SyncCall(CScriptInstance *inst, void *heapptr, jsargs_fn_t fnPushArgs, void *argsParam)
{
    CGuard guard(m_InstancesCS);

    inst->RawCall(heapptr, fnPushArgs, argsParam);

    if(inst->GetRefCount() == 0)
    {
        PostCommand(CMD_STOP_SCRIPT, inst->Name().c_str());
    }
}

void CScriptSystem::PostCommand(jssyscmd_id_t id, stdstr paramA, stdstr paramB)
{
    CGuard guard(m_CmdQueueCS);
    jssys_cmd_t cmd = { id, paramA, paramB, NULL, NULL };
    m_CmdQueue.push_back(cmd);
    SetEvent(m_hCmdEvent);
}

/*
void CScriptSystem::PostCommandSync(jssyscmd_id_t id, void* paramC)
{
    HANDLE hDoneEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    {
        CGuard guard(m_CmdQueueCS);
        jssys_cmd_t cmd = { id, "", "", paramC, hDoneEvent };
        m_CmdQueue.push_back(cmd);
        SetEvent(m_hCmdEvent);
    }

    WaitForSingleObject(hDoneEvent, INFINITE);
    CloseHandle(hDoneEvent);
}
*/

DWORD CScriptSystem::ThreadProc(void *_this)
{
    ((CScriptSystem *)_this)->ThreadProc();
    return 0;
}

void CScriptSystem::ThreadProc()
{
    bool bRunning = true;
    std::vector<jssys_cmd_t> queue;

    while(bRunning)
    {
        WaitForSingleObject(m_hCmdEvent, INFINITE);

        {
            CGuard guard(m_CmdQueueCS);
            queue = m_CmdQueue;
            m_CmdQueue.clear();
        }

        for (size_t i = 0; i < queue.size(); i++)
        {
            jssys_cmd_t& cmd = queue[i];

            switch (cmd.id)
            {
            case CMD_START_SCRIPT:
                OnStartScript(cmd.paramA.c_str(), cmd.paramB.c_str());
                break;
            case CMD_STOP_SCRIPT:
                OnStopScript(cmd.paramA.c_str());
                break;
            case CMD_INPUT:
                OnInput(cmd.paramA.c_str(), cmd.paramB.c_str());
                break;
            case CMD_SWEEP:
                OnSweep(true);
                break;
            case CMD_SHUTDOWN:
                OnSweep(false);
                bRunning = false;
                break;
            }

            if (cmd.hDoneEvent != NULL)
            {
                SetEvent(cmd.hDoneEvent);
            }
        }
    }
}

void CScriptSystem::OnStartScript(const char *name, const char *path)
{
    CGuard guard(m_InstancesCS);

    if (m_Instances.count(name) != 0)
    {
        Log("[SCRIPTSYS]: error: START_SCRIPT aborted; '%s' is already instanced", name);
    }

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
    CGuard guard(m_InstancesCS);

    if (m_Instances.count(name) == 0)
    {
        Log("[SCRIPTSYS]: error: STOP_SCRIPT aborted; instance '%s' does not exist", name);
        return;
    }

    UpdateStatus(name, JS_STATUS_STOPPED);
    RawRemoveInstance(name);
}

void CScriptSystem::OnInput(const char *name, const char *code)
{
    CGuard guard(m_InstancesCS);

    if(m_Instances.count(name) == 0)
    {
        Log("[SCRIPTSYS]: error: INPUT aborted; instance '%s' does not exist", name);
        return;
    }

    CScriptInstance* inst = m_Instances[name];

    inst->RawInput(code);

    if(inst->GetRefCount() == 0)
    {
        UpdateStatus(name, JS_STATUS_STOPPED);
        RawRemoveInstance(name);
    }
}

void CScriptSystem::OnSweep(bool bIfDone)
{
    CGuard guard(m_InstancesCS);

    jsinst_map_t::iterator it = m_Instances.begin();
    while(it != m_Instances.end())
    {
        CScriptInstance*& inst = it->second;
        if(!bIfDone || inst->GetRefCount() == 0)
        {
            UpdateStatus(inst->Name().c_str(), JS_STATUS_STOPPED);
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

jscb_id_t CScriptSystem::RawAddCallback(jshook_id_t hookId, JSCallback& callback)
{
    if(hookId >= JS_NUM_APP_HOOKS)
    {
        return JS_INVALID_CALLBACK;
    }

    callback.id = m_NextAppCallbackId;
    m_AppCallbackHooks[hookId][m_NextAppCallbackId] = callback;
    m_AppCallbackCount++;
    return m_NextAppCallbackId++;
}

bool CScriptSystem::RawRemoveCallback(jshook_id_t hookId, jscb_id_t callbackId)
{
    if(m_AppCallbackHooks[hookId].count(callbackId) == 0)
    {
        return false;
    }

    m_AppCallbackHooks[hookId].erase(callbackId);
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
