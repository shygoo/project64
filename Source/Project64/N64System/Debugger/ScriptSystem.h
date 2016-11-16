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

typedef struct {
	duk_context* ctx;
	char* path;
	CRITICAL_SECTION criticalSection;
	HANDLE hThread;
	HANDLE hIOCompletionPort;
	vector<IOFD> ioFds;
	vector<IOLISTENER*> ioListeners;
	UINT ioNextListenerId;
	bool bActive; // event loop is running
} JSCONTEXT;

typedef struct {
	int      callbackId;
	void*    heapptr;
	uint32_t tag;
	duk_context* ctx;
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

class CScriptSystem {
public:
	CScriptSystem(CDebuggerUI* debugger);

	// Run a script in its own context/thread
	void RunScript(char* path);

	// Kill a script context/thread by its path
	void StopScript(char* path);

private:
	// Static list of existing contexts accessible by js_* functions
	static vector<JSCONTEXT*> m_Contexts;
	
	vector<EVENTHOOK> m_Hooks;

	CCallbackList m_ExecEvents;
	CCallbackList m_ReadEvents;
	CCallbackList m_WriteEvents;
	
public:

	static void RegisterCallbackList(const char* hookId, CCallbackList* cbList); // associate string id with callback list
	static CCallbackList* GetCallbackList(const char* hookId);

	//static void DrawTest();

	// Queue a routine to be called from the event loop thread
	

	static void SetScriptsWindow(CDebugScripts* m_ScriptsWindow);

	static void ConsolePrint(const char* text);

private:
	static CDebugScripts* m_ScriptsWindow;

	// Event loop
	//static HANDLE m_ioBasePort;
	
	static void ioDoEvent(IOLISTENER* lpListener);
	static IOLISTENER* ioAddListener(HANDLE fd, IOEVENTTYPE evt, void* jsCallback, void* data = NULL, int dataLen = 0);
	static void ioRemoveListenerByIndex(UINT index);
	static void ioRemoveListenerByPtr(IOLISTENER* lpListener);
	static void ioRemoveListenersByFd(HANDLE fd);

	static void ioAddFd(HANDLE fd, bool bSocket = false);
	static void ioCloseFd(HANDLE fd);
	static void ioRemoveFd(HANDLE fd);

	static HANDLE ioSockCreate();

	static HANDLE ioCreateExistingFile(const char* path);
	

};