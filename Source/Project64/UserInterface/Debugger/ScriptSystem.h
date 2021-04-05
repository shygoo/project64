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

    typedef struct
    {
        int type;
        std::string message;
    } jslogitem_t;

    CriticalSection m_CS;
    CDebuggerUI    *m_Debugger;
    HANDLE          m_hThread;
    jshook_map_t    m_AppHooks;
    jscb_id_t       m_NextAppCallbackId;
    size_t          m_AppCallbackCount;
    jsinst_map_t    m_Instances;
    
    struct
    {
        jssyscmd_id_t id;
        std::string   paramA;
        std::string   paramB;
        HANDLE        hWakeEvent;
        HANDLE        hIdleEvent;
    } m_Cmd;

    CriticalSection m_UIStateCS;
    jsinst_status_map_t m_InstanceStatus;
    stdstr m_Log;

public:
    CScriptSystem(CDebuggerUI* debugger);
    ~CScriptSystem();

    CDebuggerUI* Debugger();

    bool StartScript(const char* name, const char* path);
    bool StopScript(const char* name);
    bool Input(const char* name, const char* code);

    jsstatus_t GetStatus(const char* name);
    void UpdateStatus(const char* name, jsstatus_t status);
    void Log(const char* format, ...);
    void Print(const char* format, ...);
    void ClearLog();
    stdstr GetLog();

    void SyncCall(CScriptInstance *inst, void *heapptr, jsargs_fn_t fnPushArgs = NULL, void *param = NULL);

    void Invoke(jshook_id_t hookId, void* env);
    jscb_id_t RawAddCallback(jshook_id_t hookId, JSCallback& callback);
    bool RawRemoveCallback(jshook_id_t hookId, jscb_id_t callbackId);
    
private:
    void SetCommand(jssyscmd_id_t cmd, const char* paramA = NULL, const char* paramB = NULL);

    static DWORD WINAPI ThreadProc(void* _this);
    void ThreadProc();

    void OnStartScript(const char* name, const char* path);
    void OnStopScript(const char* name);
    void OnInput(const char* name, const char* code);
    void OnSweep(bool bIfDone);

    bool RawRemoveInstance(const char* key);

    static stdstr FixStringReturns(const char* str);
};
