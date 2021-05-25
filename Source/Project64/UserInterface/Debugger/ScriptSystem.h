#include <windows.h>
#include <fstream>
#include <map>

#include "ScriptTypes.h"
#include "debugger.h"

#pragma once

class CScriptSystem
{
    typedef std::map<jsname_t, CScriptInstance*> jsinst_map_t;
    typedef std::map<jscb_id_t, JSCallback> jscb_map_t;
    typedef std::map<jshook_id_t, jscb_map_t> jshook_map_t;
    typedef std::map<jsname_t, jsstatus_t> jsinst_status_map_t;
    typedef std::vector<jssys_cmd_t> jssys_cmd_queue_t;

    HANDLE              m_hThread;

    CriticalSection     m_CmdQueueCS;
    jssys_cmd_queue_t   m_CmdQueue;
    HANDLE              m_hCmdEvent;

    CriticalSection     m_InstancesCS;
    jsinst_map_t        m_Instances;
    jshook_map_t        m_AppCallbackHooks;
    jscb_id_t           m_NextAppCallbackId;
    size_t              m_AppCallbackCount;

    CriticalSection     m_UIStateCS;
    jsinst_status_map_t m_UIInstanceStatus;
    stdstr              m_UILog;

    CDebuggerUI*        m_Debugger;

    std::set<std::string> m_AutorunSet;

public:
    CScriptSystem(CDebuggerUI* debugger);
    ~CScriptSystem();

    CDebuggerUI* Debugger();

    void StartScript(const char* name, const char* path);
    void StopScript(const char* name);
    void Input(const char* name, const char* code);

    jsstatus_t GetStatus(const char* name);
    void UpdateStatus(const char* name, jsstatus_t status);
    void Log(const char* format, ...);
    void Print(const char* format, ...);
    void ClearLog();
    stdstr GetLog();

    void SyncCall(CScriptInstance *inst, void *heapptr, jsargs_fn_t fnPushArgs = nullptr, void *param = nullptr);

    bool HaveCallbacks(jshook_id_t hookId);
    void Invoke(jshook_id_t hookId, void* env);
    void DoMouseEvent(jshook_id_t hookId, int x, int y, int button = -1);
    jscb_id_t RawAddCallback(jshook_id_t hookId, JSCallback& callback);
    bool RawRemoveCallback(jshook_id_t hookId, jscb_id_t callbackId);

    void ExecAutorunSet();
    std::set<std::string>& AutorunSet();
    void LoadAutorunSet();
    void SaveAutorunSet();
    
private:
    void PostCommand(jssyscmd_id_t id, stdstr paramA = "", stdstr paramB = "");
    //void PostCommandSync(jssyscmd_id_t id, void* paramC);

    static DWORD WINAPI ThreadProc(void* _this);
    void ThreadProc();

    void OnStartScript(const char* name, const char* path);
    void OnStopScript(const char* name);
    void OnInput(const char* name, const char* code);
    void OnSweep(bool bIfDone);

    bool RawRemoveInstance(const char* key);

    static stdstr FixStringReturns(const char* str);
};
