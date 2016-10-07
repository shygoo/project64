/****************************************************************************
*                                                                           *
* Project64 - A Nintendo 64 emulator.                                      *
* http://www.pj64-emu.com/                                                  *
* Copyright (C) 2012 Project64. All rights reserved.                        *
*                                                                           *
* License:                                                                  *
* GNU/GPLv2 http://www.gnu.org/licenses/gpl-2.0.html                        *
*                                                                           *
****************************************************************************/

#pragma once

#include "stdafx.h"
#include <3rdParty/duktape/duktape.h>
#include <mutex>

typedef struct {
	int      callbackId;
	void*    heapptr;
	uint32_t tag;
} JSCALLBACK;

class CCallbackList {
private:
	//CCallbackList();
	int m_nextCallbackId;
	vector<JSCALLBACK> m_Callbacks;
	int m_NumCallbacks;
public:
	int Add(void* heapptr, uint32_t tag);
	void InvokeById(int callbackId);
	void InvokeByTag(uint32_t tag);
};

typedef struct {
	const char* hookId;
	CCallbackList* cbList;
} EVENTHOOK;


// js event loop
typedef enum {
	EVT_READ,
	EVT_WRITE,
	EVT_ACCEPT
} IOEVENTTYPE;

typedef struct {
	OVERLAPPED  ovl;
	IOEVENTTYPE evt;
	HANDLE      fd;
	HANDLE      childFd; // accepted socket
	UINT        id;
	void*       data;
	DWORD       dataLen; // changed to bytes transferred after event is fired
	void*       callback;
} IOListener;

class CScriptSystem {
private:
	CScriptSystem(CDebuggerUI* debugger);

public:
	static int m_NextStashIndex;
	static duk_context* m_Ctx;
	static mutex m_CtxMutex;

	static vector<EVENTHOOK> m_Hooks;
	static CCallbackList m_ExecEvents;
	static CCallbackList m_ReadEvents;
	static CCallbackList m_WriteEvents;
	static CCallbackList m_WMEvents;

	static void Init();
	static void RegisterCallbackList(const char* hookId, CCallbackList* cbList);
	static CCallbackList* GetCallbackList(const char* hookId);
	static void Eval(const char* jsCode);
	static void EvalFile(const char* jsPath);
	static void BindNativeFunction(const char* name, duk_c_function func);
	static void BindGlobalFunction(const char* name, duk_c_function func);

	//static void Stash(void* heapptr);
	//static void Unstash(void* heapptr);

	static void Invoke(void* heapptr);
	static void DrawTest();

	// Queue a routine to be called from the event loop thread
	static void QueueAPC(PAPCFUNC userProc, ULONG_PTR param = 0);

private:
	// Event loop
	static HANDLE m_ioBasePort;
	static char   m_ioBaseBuf[8192]; // io buffered here
	static int    m_ioBaseBufLen;
	static vector<IOListener*> m_ioListeners;
	static HANDLE m_ioEventsThread;

	static DWORD WINAPI ioEventsProc(void* param);
	static void ioDoEvent(IOListener* lpListener);

	static void   ioAddListener(HANDLE fd, IOEVENTTYPE evt, void* jsCallback, void* data = NULL, int dataLen = 0);

	static HANDLE ioCreateExistingFile(const char* path);
	static HANDLE ioCreateServer();
	static HANDLE ioSockCreate();
	static bool   ioSockListen(HANDLE fd, USHORT port);
	static bool   ioSockAccept(HANDLE fd, void* jsCallback);
	
	static duk_ret_t _ioCreateExistingFile(duk_context*);
	static duk_ret_t _ioCreateServer(duk_context*);

	static duk_ret_t _ioSockCreate(duk_context*);
	static duk_ret_t _ioSockListen(duk_context*);
	static duk_ret_t _ioSockAccept(duk_context*);

	static duk_ret_t _ioWrite(duk_context*);
	static duk_ret_t _ioRead(duk_context*);

	// Screen printing
	//static HWND   m_RenderWindow;
	static HFONT  m_FontFamily;
	static HBRUSH m_FontColor;
	static HPEN   m_FontOutline;
	
	//static vector<HANDLE> m_WorkerThreads;
	//static DWORD WINAPI ThreadProc(LPVOID lpDukProcHeapPtr);
	
	// Bound functions
	static duk_ret_t MsgBox          (duk_context* ctx); // (message, caption)
	static duk_ret_t AddCallback     (duk_context* ctx); // (hookId, callback, tag)
	static duk_ret_t GetGPRVal       (duk_context* ctx); // (regNum)
	static duk_ret_t SetGPRVal       (duk_context* ctx); // (regNum, value)
	static duk_ret_t GetRDRAMU8      (duk_context* ctx); // (address)
	static duk_ret_t GetRDRAMU16     (duk_context* ctx); // (address)
	static duk_ret_t GetRDRAMU32     (duk_context* ctx); // (address)
	static duk_ret_t SetRDRAMU8      (duk_context* ctx); // (address, value)
	static duk_ret_t SetRDRAMU16     (duk_context* ctx); // (address, value)
	static duk_ret_t SetRDRAMU32     (duk_context* ctx); // (address, value)

	//static duk_ret_t SockCreate      (duk_context* ctx);
	//static duk_ret_t SockListen      (duk_context* ctx);
	//static duk_ret_t SockAccept      (duk_context* ctx); // (serverSocket)   ; BLOCKING ; return client sock descriptor
	//static duk_ret_t CreateServer    (duk_context* ctx); // (port) ; return sock descriptor
	//static duk_ret_t SockReceiveN    (duk_context* ctx); // (socket, nBytes) ; BLOCKING
	//static duk_ret_t SockReceive     (duk_context* ctx); // (socket) ; receive max 4kb
	static duk_ret_t ConsoleLog      (duk_context* ctx);

	//static duk_ret_t Release         (duk_context* ctx); // set the mtsafe flag to false
};