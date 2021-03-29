#include <windows.h>
#include <fstream>
#include <map>

#include "ScriptTypes.h"
#include "debugger.h"

#pragma once

class CScriptSystem
{
    typedef std::map<jsname_t, CScriptInstance*> jsinst_map_t;
    typedef std::map<jscb_id_t, jscallback_t> jscb_map_t;
    typedef std::map<jshook_id_t, jscb_map_t> jshook_map_t;

    CDebuggerUI   *m_Debugger;
    HANDLE         m_hMutex;
    HANDLE         m_hThread;
    jsinst_map_t   m_Instances;
    jshook_map_t   m_AppHooks;
    jscb_id_t      m_NextAppCallbackId;
    size_t         m_AppCallbackCount;
    
    struct
    {
        jssyscmd_id_t id;
        std::string   paramA;
        std::string   paramB;
        HANDLE        hWakeEvent;
        HANDLE        hIdleEvent;
    } m_Cmd;

public:
    CScriptSystem(CDebuggerUI *debugger);
    ~CScriptSystem();
    bool StartScript(const char *name, const char *path);
    bool StopScript(const char *name);
    bool Eval(const char *name, const char *code);

    jsstatus_t GetStatus(const char* name);

    void SyncCall(CScriptInstance *inst, void *heapptr, jsargs_fn_t fnPushArgs = NULL, void *param = NULL);

    inline void Invoke(jshook_id_t hookId, void *env)
    {
        // lock omitted here for speed, should be okay
        if(m_AppCallbackCount != 0)
        {
            _Invoke(hookId, env);
        }
    }

    // private to ScriptAPI
    jscb_id_t RawAddCallback(jshook_id_t hookId, jscallback_t& callback);
    bool RawRemoveCallback(jshook_id_t hookId, jscb_id_t callbackId);

    CDebuggerUI* Debugger();

private:
    void _Invoke(jshook_id_t hookId, void *env);

    bool LockCommand(bool bWaitInfinite = false);
    void UnlockCommand();
    void SetCommand(jssyscmd_id_t cmd, const char *paramA = "", const char *paramB = "");

    static DWORD WINAPI ThreadProc(void *_this);
    void ThreadProc();

    void OnStartScript(const char *key, const char *path);
    void OnStopScript(const char *key);
    void OnEval(const char *key, const char *code);
    void OnSweep(bool bIfDone);

    bool RawRemoveInstance(const char *key);
};
