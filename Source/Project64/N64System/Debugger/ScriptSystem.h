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
	EVT_ACCEPT,
	EVT_CONNECT
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
	//void*       closedCallback; // call if EVT_READ and nBytes == 0
} IOLISTENER;

typedef struct {
	HANDLE fd;
	HANDLE iocp;
	bool bSocket;
} IOFD;

class CScriptSystem {
private:
	CScriptSystem(CDebuggerUI* debugger);

public:
	static int m_NextStashIndex;
	static duk_context* m_Ctx;
	static mutex m_CtxMutex;

	static CRITICAL_SECTION m_CtxProtected;

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

	static void Invoke(void* heapptr);
	//static void DrawTest();

	// Queue a routine to be called from the event loop thread
	static void QueueAPC(PAPCFUNC userProc, ULONG_PTR param = 0);

private:
	
	// Event loop
	static HANDLE m_ioBasePort;
	
	static vector<IOFD> m_ioFds;

	static vector<IOLISTENER*> m_ioListeners;
	static UINT m_ioNextListenerId;
	
	static HANDLE m_ioEventsThread;

	static DWORD WINAPI ioEventsProc(void* param);
	static void ioDoEvent(IOLISTENER* lpListener);

	static IOLISTENER* ioAddListener(HANDLE fd, IOEVENTTYPE evt, void* jsCallback, void* data = NULL, int dataLen = 0);
	static void   ioRemoveListenerByIndex(UINT index);
	static void   ioRemoveListenerByPtr(IOLISTENER* lpListener);
	static void   ioRemoveListenersByFd(HANDLE fd);

	static void   ioAddFd(HANDLE fd, bool bSocket = false);
	static void   ioCloseFd(HANDLE fd);
	
	static HANDLE ioSockCreate();

	static HANDLE ioCreateExistingFile(const char* path);
	static duk_ret_t _ioCreateExistingFile(duk_context*);
	
	// Screen printing
	//static HWND   m_RenderWindow;
	static HFONT  m_FontFamily;
	static HBRUSH m_FontColor;
	static HPEN   m_FontOutline;
	
	// Bound functions (_native object)
	static duk_ret_t js_ioSockCreate (duk_context*);
	static duk_ret_t js_ioSockListen (duk_context*);
	static duk_ret_t js_ioSockAccept (duk_context*);
	static duk_ret_t js_ioSockConnect(duk_context*);

	static duk_ret_t js_ioRead       (duk_context*);
	static duk_ret_t js_ioWrite      (duk_context*);
	static duk_ret_t js_ioClose      (duk_context* ctx); // (fd) ; file or socket

	static duk_ret_t js_MsgBox       (duk_context*); // (message, caption)
	
	static duk_ret_t js_AddCallback  (duk_context*); // (hookId, callback, tag) ; external events

	static duk_ret_t js_GetGPRVal    (duk_context*); // (regNum)
	static duk_ret_t js_SetGPRVal    (duk_context*); // (regNum, value)

	static duk_ret_t js_GetRDRAMInt  (duk_context*); // (address, bitwidth, signed)
	static duk_ret_t js_SetRDRAMInt  (duk_context*); // (address, bitwidth, signed, newValue)
	static duk_ret_t js_GetRDRAMFloat(duk_context*); // (address, bDouble)
	static duk_ret_t js_SetRDRAMFloat(duk_context*); // (address, bDouble, newValue)
	static duk_ret_t js_GetRDRAMBlock(duk_context*); // (address, nBytes) ; returns Buffer
	
	//static duk_ret_t ConsoleLog      (duk_context*);
};