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
#include "stdafx.h"
#include "ScriptSystem.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <mswsock.h>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")

mutex CScriptSystem::m_CtxMutex;

//HWND CScriptSystem::m_RenderWindow = NULL;
HFONT CScriptSystem::m_FontFamily = NULL;
HBRUSH CScriptSystem::m_FontColor = NULL;
HPEN CScriptSystem::m_FontOutline = NULL;

int CCallbackList::Add(void* heapptr, uint32_t tag = 0)
{
	int callbackId = m_nextCallbackId++;
	m_Callbacks.push_back({ callbackId, heapptr, tag });
	m_NumCallbacks = m_Callbacks.size();
	return callbackId;
}

void CCallbackList::InvokeById(int callbackId)
{
	for (int i = 0; i < m_NumCallbacks; i++)
	{
		if (m_Callbacks[i].callbackId == callbackId)
		{
			EnterCriticalSection(&CScriptSystem::m_CtxProtected);
			CScriptSystem::Invoke(m_Callbacks[i].heapptr);
			LeaveCriticalSection(&CScriptSystem::m_CtxProtected);
			return;
		}
	}
}

void CCallbackList::InvokeByTag(uint32_t tag)
{
	for (int i = 0; i < m_NumCallbacks; i++)
	{
		if (m_Callbacks[i].tag == tag)
		{
			EnterCriticalSection(&CScriptSystem::m_CtxProtected);
			CScriptSystem::Invoke(m_Callbacks[i].heapptr);
			LeaveCriticalSection(&CScriptSystem::m_CtxProtected);
			return;
		}
	}
}

// hooks: ui, read, write, exec

//duk_ret_t _NewWindow(duk_context* ctx);
//duk_ret_t _AddCtrl(duk_context* ctx);
//duk_ret_t _AddMessageHandler(duk_context* ctx);
/*
duk_ret_t _vmbase(duk_context* ctx)
{
	duk_push_uint(ctx, (uint32_t)g_MMU->Rdram());
	return 1;
}*/
duk_context* CScriptSystem::m_Ctx = NULL;

int CScriptSystem::m_NextStashIndex = 0;

vector<EVENTHOOK> CScriptSystem::m_Hooks;

CCallbackList CScriptSystem::m_ExecEvents;
CCallbackList CScriptSystem::m_ReadEvents;
CCallbackList CScriptSystem::m_WriteEvents;
CCallbackList CScriptSystem::m_WMEvents;

CRITICAL_SECTION CScriptSystem::m_CtxProtected;

HANDLE CScriptSystem::m_ioEventsThread;
HANDLE CScriptSystem::m_ioBasePort;
vector<IOListener*> CScriptSystem::m_ioListeners;

void CScriptSystem::Init()
{
	m_Ctx = duk_create_heap_default();
	
	const duk_function_list_entry _native[] = {
		{ "addCallback",   js_AddCallback,   DUK_VARARGS },
		{ "setGPRVal",     js_SetGPRVal,     DUK_VARARGS },
		{ "getGPRVal",     js_GetGPRVal,     DUK_VARARGS },
		{ "getRDRAMInt",   js_GetRDRAMInt,   DUK_VARARGS },
		{ "setRDRAMInt",   js_SetRDRAMInt,   DUK_VARARGS },
		{ "getRDRAMFloat", js_GetRDRAMFloat, DUK_VARARGS },
		{ "setRDRAMFloat", js_SetRDRAMFloat, DUK_VARARGS },
		{ "getRDRAMBlock", js_GetRDRAMBlock, DUK_VARARGS },
		{ "sockCreate",    js_ioSockCreate,  DUK_VARARGS },
		{ "sockListen",    js_ioSockListen,  DUK_VARARGS },
		{ "sockAccept",    js_ioSockAccept,  DUK_VARARGS },
		{ "write",         js_ioWrite,       DUK_VARARGS },
		{ "read",          js_ioRead,        DUK_VARARGS },
		{ "msgBox",        js_MsgBox,        DUK_VARARGS },
		{NULL, NULL, 0}
	};

	duk_push_object(m_Ctx);
	duk_put_global_string(m_Ctx, "_native");
	duk_get_global_string(m_Ctx, "_native");
	duk_put_function_list(m_Ctx, -1, _native);
	duk_pop(m_Ctx);
	
	RegisterCallbackList("exec", &m_ExecEvents);
	RegisterCallbackList("read", &m_ReadEvents);
	RegisterCallbackList("write", &m_WriteEvents);
	RegisterCallbackList("wm", &m_WMEvents);
	
	// Init winsock
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	m_ioBasePort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

	EvalFile("_api.js");
	EvalFile("_script.js");

	// screen print test
	//m_FontFamily = CreateFont(-13, 0, 0, 0,
	//	FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
	//	OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
	//	DEFAULT_QUALITY, FF_DONTCARE, "Courier New"
	//);
	//m_FontColor = CreateSolidBrush(RGB(0xFF, 0xFF, 0xFF));
	//m_FontOutline = CreatePen(PS_SOLID, 3, RGB(0, 0, 0));
	
	InitializeCriticalSection(&m_CtxProtected); // todo cleanup

	// Control of the duk ctx is transferred to m_ioEventsThread here
	m_ioEventsThread = CreateThread(NULL, 0, ioEventsProc, NULL, 0, NULL);
}

void CScriptSystem::QueueAPC(PAPCFUNC userProc, ULONG_PTR param)
{
	if(m_ioEventsThread != NULL)
	{
		QueueUserAPC(userProc, m_ioEventsThread, param);
	}
}

void CScriptSystem::Eval(const char* jsCode)
{
	int result = duk_peval_string(m_Ctx, jsCode);
	const char* msg = duk_safe_to_string(m_Ctx, -1);
	if (result != 0)
	{
		MessageBox(NULL, msg, "Script error", MB_OK | MB_ICONWARNING);
	}
	duk_pop(m_Ctx);
}

void CScriptSystem::EvalFile(const char* jsPath)
{
	int result = duk_peval_file(m_Ctx, jsPath);
	const char* msg = duk_safe_to_string(m_Ctx, -1);
	if (result != 0)
	{
		MessageBox(NULL, msg, jsPath, MB_OK | MB_ICONWARNING);
	}
	duk_pop(m_Ctx);
}

void CScriptSystem::BindGlobalFunction(const char* name, duk_c_function func)
{
	duk_push_c_function(m_Ctx, func, DUK_VARARGS);
	duk_put_global_string(m_Ctx, name);
}

void CScriptSystem::RegisterCallbackList(const char* hookId, CCallbackList* cbList)
{
	EVENTHOOK eventHook = { hookId, cbList };
	m_Hooks.push_back(eventHook);
}

CCallbackList* CScriptSystem::GetCallbackList(const char* hookId)
{
	int size = m_Hooks.size();
	for (int i = 0; i < size; i++)
	{
		if (strcmp(m_Hooks[i].hookId, hookId) == 0)
		{
			return m_Hooks[i].cbList;
		}
	}
	return NULL;
}

/*
// Protect js object/function from garbage collector
void CScriptSystem::Stash(void* heapptr)
{
	duk_push_global_stash(m_Ctx);
	duk_push_heapptr(m_Ctx, heapptr);
	duk_put_prop_index(m_Ctx, -2, m_NextStashIndex);
	duk_pop_n(m_Ctx, 1);
}*/

void CScriptSystem::Invoke(void* heapptr)
{
	duk_push_heapptr(m_Ctx, heapptr);
	duk_call(m_Ctx, 0);
	duk_pop(m_Ctx);
}

void CScriptSystem::DrawTest()
{
	HWND renderWindow = (HWND)g_Plugins->MainWindow()->GetWindowHandle();

	HDC hdc = GetDC(renderWindow);

	SelectObject(hdc, m_FontFamily);
	SelectObject(hdc, m_FontColor);
	SelectObject(hdc, m_FontOutline);
	
	SetBkMode(hdc, TRANSPARENT);

	SetTextColor(hdc, RGB(0xFF, 0xFF, 0xFF));

	char str[256];
	strcpy(str, "screen print test");

	RECT rect;
	rect.top = 50;
	rect.left = 50;

	BeginPath(hdc);
	TextOut(hdc, 50, 50, str, strlen(str));
	EndPath(hdc);
	StrokePath(hdc);
	AbortPath(hdc);

	TextOut(hdc, 50, 50, str, strlen(str));

	ReleaseDC(renderWindow, hdc);
}

void CScriptSystem::ioAddListener(HANDLE fd, IOEVENTTYPE evt, void* callback, void* data, int dataLen, bool bSocket)
{
	IOListener* lpListener = (IOListener*) malloc(sizeof(IOListener));
	OVERLAPPED* lpOvl = (OVERLAPPED*)lpListener;
	*lpListener = { 0 };

	m_ioListeners.push_back(lpListener);
	
	lpListener->evt = evt;
	lpListener->fd = fd;
	lpListener->callback = callback;
	lpListener->data = data;
	lpListener->dataLen = dataLen;
	lpListener->bSocket = bSocket;

	switch (evt)
	{
	case EVT_READ:
		{
			bool status = ReadFile(fd, lpListener->data, lpListener->dataLen, NULL, lpOvl);
			if (status == false && GetLastError() != ERROR_IO_PENDING)
			{
				MessageBox(NULL, "readex error", "", MB_OK);
			}
			break;
		}
	case EVT_WRITE:
		WriteFile(fd, lpListener->data, lpListener->dataLen, NULL, lpOvl);
		break;

	case EVT_ACCEPT:
		// Get client socket ready
		lpListener->childFd = ioSockCreate();
		AcceptEx(
			(SOCKET)fd,
			(SOCKET)lpListener->childFd,
			lpListener->data, // local and remote SOCKADDR
			0,
			sizeof(SOCKADDR_IN) + 16,
			sizeof(SOCKADDR_IN) + 16,
			NULL,
			lpOvl
		);
		break;
	}
}

// Free listener & its buffer, remove from list
void CScriptSystem::ioRemoveListener(IOListener* lpListener)
{
	if (lpListener->data != NULL)
	{
		free(lpListener->data);
	}

	free(lpListener);

	UINT len = m_ioListeners.size();
	for (UINT i = 0; i < len; i++)
	{
		if (m_ioListeners[i] == lpListener)
		{
			m_ioListeners.erase(m_ioListeners.begin() + i);
			break;
		}
	}
}

void CScriptSystem::ioDoEvent(IOListener* lpListener)
{
	switch (lpListener->evt)
	{
	case EVT_READ:
		if (lpListener->dataLen == 0)
		{
			if (lpListener->bSocket)
			{
				closesocket((SOCKET)lpListener->fd);
			}
			else
			{
				CloseHandle(lpListener->fd);
			}
			break;
		}
		if (lpListener->callback != NULL)
		{
			duk_push_heapptr(m_Ctx, lpListener->callback);
			//duk_push_external_buffer(m_Ctx);
			//duk_config_buffer(m_Ctx, -1, lpListener->data, lpListener->dataLen);
			void* data = duk_push_buffer(m_Ctx, lpListener->dataLen, false);
			memcpy(data, lpListener->data, lpListener->dataLen);

			duk_call(m_Ctx, 1);
			duk_pop(m_Ctx);
		}
		break;

	case EVT_WRITE:
		if (lpListener->callback != NULL)
		{
			duk_push_heapptr(m_Ctx, lpListener->callback);
			duk_call(m_Ctx, 0);
			duk_pop(m_Ctx);
		}
		break;

	case EVT_ACCEPT:
		// pass client socket fd to callback
		if (lpListener->callback != NULL)
		{
			duk_push_heapptr(m_Ctx, lpListener->callback);
			duk_push_uint(m_Ctx, (UINT)lpListener->childFd);
			duk_call(m_Ctx, 1);
			duk_pop(m_Ctx);
		}
		break;
	}
}

DWORD WINAPI CScriptSystem::ioEventsProc(void* param)
{
	while (1)
	{
		OVERLAPPED_ENTRY usedOvlEntry;
		ULONG nUsedOverlaps;

		// Wait for an IO completion or async proc call
		BOOL status = GetQueuedCompletionStatusEx(
			m_ioBasePort,
			&usedOvlEntry,
			1,
			&nUsedOverlaps,
			INFINITE,
			TRUE
		);

		LPOVERLAPPED lpUsedOvl = usedOvlEntry.lpOverlapped;
		DWORD nBytesTransferred = usedOvlEntry.dwNumberOfBytesTransferred;

		if (!status)
		{
			if (GetLastError() == STATUS_USER_APC)
			{
				// Interrupted by an async proc call
				continue;
			}
			
			char errmsg[128];
			sprintf(errmsg, "GetQueuedCompletionStatus error (%d)", GetLastError());
			MessageBox(NULL, errmsg, "Error", MB_OK);
			break;
		}

		IOListener* lpListener = (IOListener*)lpUsedOvl;
		lpListener->dataLen = nBytesTransferred;

		// Protect from emulation thread (onexec, onread, etc)
		EnterCriticalSection(&m_CtxProtected);

		ioDoEvent(lpListener);

		LeaveCriticalSection(&m_CtxProtected);

		// Destroy listener
		ioRemoveListener(lpListener);
	}

	return 0;
}

HANDLE CScriptSystem::ioCreateExistingFile(const char* path)
{
	HANDLE fd = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ,
		NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	CreateIoCompletionPort(fd, m_ioBasePort, (ULONG_PTR)fd, 0);
	return fd;
}

HANDLE CScriptSystem::ioSockCreate()
{
	HANDLE fd = (HANDLE)WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	CreateIoCompletionPort(fd, m_ioBasePort, (ULONG_PTR)fd, 0);
	return fd;
}

duk_ret_t CScriptSystem::js_ioSockCreate(duk_context* ctx)
{
	HANDLE fd = ioSockCreate();
	duk_pop_n(ctx, duk_get_top(ctx));
	duk_push_int(ctx, (int)fd);
	return 1;
}

duk_ret_t CScriptSystem::js_ioSockListen(duk_context* ctx)
{
	HANDLE fd = (HANDLE)duk_get_uint(ctx, 0);
	USHORT port = duk_get_uint(ctx, 1);
	duk_pop_n(ctx, 2);

	SOCKADDR_IN serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(port);
	if (::bind((SOCKET)fd, (SOCKADDR*)&serverAddr, sizeof(serverAddr)))
	{
		duk_push_boolean(ctx, false);
		return 1;
	}
	bool listenOkay = listen((SOCKET)fd, 3) == 0;
	duk_push_boolean(ctx, listenOkay);

	return 1;
}

duk_ret_t CScriptSystem::js_ioSockAccept(duk_context* ctx)
{
	HANDLE fd = (HANDLE)duk_get_uint(ctx, 0);
	void* jsCallback = duk_get_heapptr(ctx, 1);
	duk_pop_n(ctx, 2);

	void* data = malloc(sizeof(SOCKADDR) * 4); // issue?
	ioAddListener(fd, EVT_ACCEPT, jsCallback, data, 0, true);
	return 1;
}

duk_ret_t CScriptSystem::js_ioRead(duk_context* ctx)
{
	// (fd, bufferSize, callback)
	HANDLE fd = (HANDLE)duk_get_uint(ctx, 0);
	size_t bufferSize = duk_get_uint(ctx, 1);
	void* jsCallback = duk_get_heapptr(ctx, 2);
	duk_pop_n(ctx, 3);

	void* data = malloc(bufferSize); // freed after event is fired

	// TEMP bSocket true
	ioAddListener(fd, EVT_READ, jsCallback, data, bufferSize, true);
	return 1;
}

duk_ret_t CScriptSystem::js_ioWrite(duk_context* ctx)
{
	HANDLE fd = (HANDLE)duk_get_uint(ctx, 0);
	duk_size_t dataLen;
	void* jsData = duk_to_buffer(ctx, 1, &dataLen);
	void* jsCallback = duk_get_heapptr(ctx, 2);

	char* data = (char*)malloc(dataLen + 1); // freed after event is fired
	memcpy(data, jsData, dataLen);
	data[dataLen] = '\0';
	
	duk_pop_n(ctx, 3);
	// TEMP bSocket true
	ioAddListener(fd, EVT_WRITE, jsCallback, data, dataLen, true);
	return 1;
}

duk_ret_t CScriptSystem::js_AddCallback(duk_context* ctx)
{
	const char* hookId;
	void* heapptr;
	uint32_t tag;

	int argc = duk_get_top(ctx);

	tag = 0;
	if (argc == 3)
	{
		tag = duk_get_uint(ctx, 2);
		duk_pop(ctx);
	}

	hookId = duk_get_string(ctx, 0);
	heapptr = duk_get_heapptr(ctx, 1);
	duk_pop_n(ctx, 2);

	//Stash(heapptr);

	int callbackId = -1;

	CCallbackList* cbList = GetCallbackList(hookId);
	if (cbList != NULL)
	{
		callbackId = cbList->Add(heapptr, tag);
	}

	duk_push_int(ctx, callbackId);
	return 1;
}

duk_ret_t CScriptSystem::js_GetGPRVal(duk_context* ctx)
{
	int regnum = duk_to_int(ctx, 0);
	duk_pop_n(ctx, 1);
	duk_push_uint(ctx, g_Reg->m_GPR[regnum].UW[0]);
	return 1;
}

duk_ret_t CScriptSystem::js_SetGPRVal(duk_context* ctx)
{
	int regnum = duk_to_int(ctx, 0);
	uint32_t val = duk_to_uint32(ctx, 1);
	duk_pop_n(ctx, 1);
	g_Reg->m_GPR[regnum].UW[0] = val;
	duk_push_uint(ctx, val);
	return 1;
}

duk_ret_t CScriptSystem::js_GetRDRAMInt(duk_context* ctx)
{
	uint32_t address = duk_to_uint32(ctx, 0);
	int bitwidth = duk_to_int(ctx, 1);
	bool bSigned = duk_to_boolean(ctx, 2);

	duk_pop_n(ctx, 3);
	
	if (g_MMU == NULL)
	{
		goto return_err;
	}
	
	DWORD retval;

	switch (bitwidth)
	{
	case 8:
		{
			uint8_t val;
			if (!g_MMU->LB_VAddr(address, val))
			{
				goto return_err;
			}
			retval = bSigned ? (char)val : val;
			break;
		}
	case 16:
		{
			uint16_t val;
			if (!g_MMU->LH_VAddr(address, val))
			{
				goto return_err;
			}
			retval = bSigned ? (short)val : val;
			break;
		}
	case 32:
		{
			uint32_t val;
			if (!g_MMU->LW_VAddr(address, val))
			{
				goto return_err;
			}
			retval = bSigned ? (int)val : val;
			break;
		}
	default:
		goto return_err;
	}
	
	if (bSigned)
	{
		duk_push_int(ctx, retval);
	}
	else
	{
		duk_push_uint(ctx, retval);
	}
	
	return 1;

	return_err:
	duk_push_boolean(ctx, false);
	return 1;
}

duk_ret_t CScriptSystem::js_SetRDRAMInt(duk_context* ctx)
{
	uint32_t address = duk_to_uint32(ctx, 0);
	int bitwidth = duk_to_int(ctx, 1);
	DWORD newValue = duk_to_uint32(ctx, 2);

	duk_pop_n(ctx, 3);

	if (g_MMU == NULL)
	{
		goto return_err;
	}

	switch (bitwidth)
	{
	case 8:
		if (!g_MMU->SB_VAddr(address, newValue))
		{
			goto return_err;
		}
		goto return_ok;
	case 16:
		if (!g_MMU->SH_VAddr(address, newValue))
		{
			goto return_err;
		}
		goto return_ok;
	case 32:
		if (!g_MMU->SW_VAddr(address, newValue))
		{
			goto return_err;
		}
		goto return_ok;
	default:
		goto return_err;
	}

	return_ok:
	duk_push_boolean(ctx, true);
	return 1;

	return_err:
	duk_push_boolean(ctx, false);
	return 1;
}

duk_ret_t CScriptSystem::js_GetRDRAMFloat(duk_context* ctx)
{
	int argc = duk_get_top(ctx);
	
	uint32_t address = duk_to_uint32(ctx, 0);
	bool bDouble = false;

	if (argc > 1)
	{
		bDouble = duk_get_boolean(ctx, 1);
	}

	duk_pop_n(ctx, argc);

	if (g_MMU == NULL)
	{
		goto return_err;
	}

	if (!bDouble)
	{
		uint32_t rawValue;
		if (!g_MMU->LW_VAddr(address, rawValue))
		{
			goto return_err;
		}

		float floatValue;
		memcpy(&floatValue, &rawValue, sizeof(float));
		
		duk_push_number(ctx, (double)floatValue);
		return 1;
	}

	uint64_t rawValue;
	if (!g_MMU->LD_VAddr(address, rawValue))
	{
		goto return_err;
	}

	double floatValue;
	memcpy(&floatValue, &rawValue, sizeof(double));
	
	duk_push_number(ctx, floatValue);
	return 1;

return_err:
	duk_push_boolean(ctx, false);
	return 1;
}

duk_ret_t CScriptSystem::js_SetRDRAMFloat(duk_context* ctx)
{
	int argc = duk_get_top(ctx);

	uint32_t address = duk_to_uint32(ctx, 0);
	double value = duk_get_number(ctx, 1);
	bool bDouble = false;

	if (argc > 2)
	{
		bDouble = duk_get_boolean(ctx, 2);
	}

	duk_pop_n(ctx, argc);

	if (g_MMU == NULL)
	{
		goto return_err;
	}

	if (!bDouble)
	{
		float floatValue = (float)value;
		uint32_t rawValue;
		memcpy(&rawValue, &floatValue, sizeof(float));
		if (!g_MMU->SW_VAddr(address, rawValue))
		{
			goto return_err;
		}
	}
	else
	{
		double floatValue = (double)value;
		uint64_t rawValue;
		memcpy(&rawValue, &floatValue, sizeof(double));
		if (!g_MMU->SD_VAddr(address, rawValue))
		{
			goto return_err;
		}
	}

	duk_push_boolean(ctx, true);
	return 1;

return_err:
	duk_push_boolean(ctx, false);
	return 1;
}

duk_ret_t CScriptSystem::js_GetRDRAMBlock(duk_context* ctx)
{
	uint32_t address = duk_get_uint(ctx, 0);
	uint32_t size = duk_get_uint(ctx, 1);

	duk_pop_n(ctx, 2);

	uint8_t* block = (uint8_t*) duk_push_buffer(m_Ctx, size, false);

	for (uint32_t i = 0; i < size; i++)
	{
		g_MMU->LB_VAddr(address + i, block[i]);
	}

	return 1;
}


duk_ret_t CScriptSystem::js_MsgBox(duk_context* ctx)
{
	int argc = duk_get_top(ctx);

	const char* msg = duk_to_string(ctx, 0);
	const char* caption = "";

	if (argc > 1)
	{
		caption = duk_to_string(ctx, 1);
	}

	MessageBox(NULL, msg, caption, MB_OK);

	duk_pop_n(ctx, argc);
	duk_push_boolean(ctx, 1);
	return 1;
}

/******************** windows **********************/
/*

// win32 bindings

enum HandlerType {
	MESSAGES = 0,
	NOTIFICATIONS = 1,
	COMMANDS = 2
};

typedef struct {
	HandlerType handlerType;
	HWND parentWnd;
	UINT code;
	WORD ctrl;
	int eventId;
	int handlerId;
} MESSAGEHANDLER;

std::vector<MESSAGEHANDLER> messageHandlers;

int nextHandlerId = 0;

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int nMessageHandlers = messageHandlers.size();
	
	for (int i = 0; i < nMessageHandlers; i++)
	{
		MESSAGEHANDLER handler = messageHandlers[i];

		if (handler.parentWnd != hWnd)
		{
			continue;
		}

		if (uMsg == WM_COMMAND && messageHandlers[i].handlerType == COMMANDS)
		{
			if (handler.code == HIWORD(wParam) && handler.ctrl == LOWORD(wParam))
			{
				CScriptSystem::InvokeEvent(handler.eventId);
			}
		}
		else if (uMsg == WM_NOTIFY && messageHandlers[i].handlerType == NOTIFICATIONS)
		{
			NMHDR* lpNMHDR = (NMHDR*)lParam;
			if (handler.code == lpNMHDR->code && handler.ctrl == lpNMHDR->idFrom)
			{
				CScriptSystem::InvokeEvent(handler.eventId);
			}
		}
		else if (messageHandlers[i].handlerType == MESSAGES)
		{
			if (handler.code == uMsg)
			{
				CScriptSystem::InvokeEvent(handler.eventId);
			}
		}
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

duk_ret_t _NewWindow(duk_context* ctx) {

	WNDCLASSEX wcx;

	wcx.cbSize = sizeof(wcx);
	wcx.style = CS_HREDRAW | CS_VREDRAW;
	wcx.lpfnWndProc = MainWndProc;
	wcx.cbClsExtra = 0;
	wcx.cbWndExtra = 0;
	wcx.hInstance = GetModuleHandle(NULL);
	wcx.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wcx.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcx.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wcx.lpszMenuName = "MainMenu";
	wcx.lpszClassName = "MainWClass";
	wcx.hIconSm = (HICON)LoadImage(GetModuleHandle(NULL),
		MAKEINTRESOURCE(5),
		IMAGE_ICON,
		GetSystemMetrics(SM_CXSMICON),
		GetSystemMetrics(SM_CYSMICON),
		LR_DEFAULTCOLOR);

	RegisterClassEx(&wcx);

	HWND hWnd = CreateWindow(
		"MainWClass",
		"test",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		500, 100,
		NULL,
		NULL,
		NULL, // hInstance
		NULL
	);

	ShowWindow(hWnd, SW_SHOW);

	duk_push_int(ctx, (int)hWnd);
	return 1;
}

UINT currentMenuId = 1;

duk_ret_t _AddCtrl(duk_context* ctx)
{
	// in hwnd, wndclass, position
	// out menuId
	HWND hWnd = (HWND)duk_get_int(ctx, 0);
	const char* wndClass = duk_get_string(ctx, 1);
	const char* lpWindowName = duk_get_string(ctx, 2);
	int xPos = duk_get_int(ctx, 3);
	int yPos = duk_get_int(ctx, 4);
	int width = duk_get_int(ctx, 5);
	int height = duk_get_int(ctx, 6);
	duk_pop_n(ctx, 7);

	HWND hwndButton = CreateWindowA(
		wndClass,  // Predefined class;
		"OK",      // Button text 
		WS_TABSTOP | WS_VISIBLE | WS_CHILD, //| BS_DEFPUSHBUTTON,  // Styles 
		xPos,         // x position 
		yPos,         // y position 
		width,        // Button width
		height,       // Button height
		hWnd,		// Parent window
		(HMENU)currentMenuId,       // No menu.
		(HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE),
		NULL);      // Pointer not needed.

	duk_push_uint(ctx, currentMenuId);
	currentMenuId++;
	return 1;
}

duk_ret_t _AddMessageHandler(duk_context* ctx) {
	// in type, hParentWnd, code, ctrlId, eventId
	// ret handlerId
	HandlerType type = (HandlerType)duk_get_int(ctx, 0);
	HWND hParentWnd = (HWND)duk_get_uint(ctx, 1);
	UINT code = duk_get_uint(ctx, 2);
	UINT ctrl = duk_get_uint(ctx, 3);
	int eventId = duk_get_int(ctx, 4);
	duk_pop_n(ctx, 5);

	MESSAGEHANDLER handler = {type, hParentWnd, code, ctrl, eventId, nextHandlerId};
	nextHandlerId++;

	messageHandlers.push_back(handler);

	duk_push_int(ctx, handler.handlerId);
	return 1;
}

*/