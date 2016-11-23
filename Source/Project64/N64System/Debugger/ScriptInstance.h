#pragma once

#include "stdafx.h"
#include <3rdParty/duktape/duktape.h>

#include <winsock2.h>
#include <ws2tcpip.h>
#include <mswsock.h>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")

class CScriptSystem;

class CScriptInstance
{
	typedef enum {
		EVENT_READ,
		EVENT_WRITE,
		EVENT_ACCEPT,
		EVENT_CONNECT
	} IOEVENTTYPE;

	typedef struct {
		OVERLAPPED  ovl;
		IOEVENTTYPE eventType;
		HANDLE      fd;
		HANDLE      childFd; // accepted socket
		bool        bSocket;
		UINT        id;
		void*       data;
		DWORD       dataLen; // changed to bytes transferred after event is fired
		void*       callback;
	} IOLISTENER;

	// Wrapper for file/socket descriptor and completion port
	typedef struct {
		HANDLE fd;
		HANDLE iocp;
		bool bSocket;
	} IOFD;
	
	typedef enum {
		EVENT_STATUS_OK,
		EVENT_STATUS_INTERRUPTED,
		EVENT_STATUS_ERROR
	} EVENT_STATUS;

public:

	typedef enum {
		STATE_STARTED, // initial evaluation & execution
		STATE_RUNNING, // event loop running with pending events
		STATE_STOPPED,  // no pending events
		STATE_INVALID
	} INSTANCE_STATE;

	CScriptInstance(CScriptSystem* scriptSystem);
	~CScriptInstance();

	void Start(char* path);
	void Invoke(void* heapptr);
	INSTANCE_STATE GetState();

private:
	duk_context*        m_Ctx;
	duk_context*        DukContext();
	char*               m_TempPath;
	
	HANDLE              m_hThread;
	HANDLE              m_hIOCompletionPort;
	CRITICAL_SECTION    m_CriticalSection;

	vector<IOFD>        m_Files;
	vector<IOLISTENER*> m_Listeners;
	UINT                m_NextListenerId;
	
	CScriptSystem*      m_ScriptSystem;
	CScriptSystem*      ScriptSystem();

	INSTANCE_STATE      m_State;
	
	static DWORD CALLBACK StartScriptProc(CScriptInstance* _this);
	void StartEventLoop();
	bool HaveEvents();
	EVENT_STATUS WaitForEvent(IOLISTENER** lpListener);

	void QueueAPC(PAPCFUNC userProc, ULONG_PTR param = 0);

	void AddFile(HANDLE fd, bool bSocket = false);
	void CloseFile(HANDLE fd);
	void RemoveFile(HANDLE fd);
	HANDLE CreateSocket();
	
	IOLISTENER* AddListener(HANDLE fd, IOEVENTTYPE evt, void* jsCallback, void* data = NULL, int dataLen = 0);
	void RemoveListenerByIndex(UINT index);
	void RemoveListenerByPtr(IOLISTENER* lpListener);
	void RemoveListenersByFd(HANDLE fd);
	void InvokeListenerCallback(IOLISTENER* lpListener);
	
	const char* Eval(const char* jsCode);
	const char* EvalFile(const char* jsPath);
	
	// Lookup list of CScriptInstance instances for static js_* functions
	static vector<CScriptInstance*> Cache;
	static void CacheInstance(CScriptInstance* _this);
	static void UncacheInstance(CScriptInstance* _this);
	static CScriptInstance* FetchInstance(duk_context* ctx);
	
	// Bound functions (_native object)
	static duk_ret_t js_ioSockCreate   (duk_context*);
	static duk_ret_t js_ioSockListen   (duk_context*);
	static duk_ret_t js_ioSockAccept   (duk_context*); // async
	static duk_ret_t js_ioSockConnect  (duk_context*); // async
	static duk_ret_t js_ioRead         (duk_context*); // async
	static duk_ret_t js_ioWrite        (duk_context*); // async
	static duk_ret_t js_ioClose        (duk_context*); // (fd) ; file or socket
	static duk_ret_t js_MsgBox         (duk_context*); // (message, caption)
	static duk_ret_t js_AddCallback    (duk_context*); // (hookId, callback, tag) ; external events
	static duk_ret_t js_GetGPRVal      (duk_context*); // (regNum)
	static duk_ret_t js_SetGPRVal      (duk_context*); // (regNum, value)
	static duk_ret_t js_GetRDRAMInt    (duk_context*); // (address, bitwidth, signed)
	static duk_ret_t js_SetRDRAMInt    (duk_context*); // (address, bitwidth, signed, newValue)
	static duk_ret_t js_GetRDRAMFloat  (duk_context*); // (address, bDouble)
	static duk_ret_t js_SetRDRAMFloat  (duk_context*); // (address, bDouble, newValue)
	static duk_ret_t js_GetRDRAMBlock  (duk_context*); // (address, nBytes) ; returns Buffer
	static duk_ret_t js_GetRDRAMString (duk_context*); // fetch zero terminated string from memory
	static duk_ret_t js_ConsolePrint   (duk_context*);
	static duk_ret_t js_ConsoleClear   (duk_context*);

	static constexpr duk_function_list_entry NativeFunctions[] =
	{
		{ "addCallback",    js_AddCallback,    DUK_VARARGS },
		{ "setGPRVal",      js_SetGPRVal,      DUK_VARARGS },
		{ "getGPRVal",      js_GetGPRVal,      DUK_VARARGS },
		{ "getRDRAMInt",    js_GetRDRAMInt,    DUK_VARARGS },
		{ "setRDRAMInt",    js_SetRDRAMInt,    DUK_VARARGS },
		{ "getRDRAMFloat",  js_GetRDRAMFloat,  DUK_VARARGS },
		{ "setRDRAMFloat",  js_SetRDRAMFloat,  DUK_VARARGS },
		{ "getRDRAMBlock",  js_GetRDRAMBlock,  DUK_VARARGS },
		{ "getRDRAMString", js_GetRDRAMString, DUK_VARARGS },
		{ "sockCreate",     js_ioSockCreate,   DUK_VARARGS },
		{ "sockListen",     js_ioSockListen,   DUK_VARARGS },
		{ "sockAccept",     js_ioSockAccept,   DUK_VARARGS },
		{ "sockConnect",    js_ioSockConnect,  DUK_VARARGS },
		{ "close",          js_ioClose,        DUK_VARARGS },
		{ "write",          js_ioWrite,        DUK_VARARGS },
		{ "read",           js_ioRead,         DUK_VARARGS },
		{ "msgBox",         js_MsgBox,         DUK_VARARGS },
		{ "consolePrint",   js_ConsolePrint,   DUK_VARARGS },
		{ "consoleClear",   js_ConsoleClear,   DUK_VARARGS },
		{ NULL, NULL, 0 }
	};
};