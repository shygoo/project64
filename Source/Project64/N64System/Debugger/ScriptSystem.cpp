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

static BOOL ConnectEx(SOCKET s, const SOCKADDR* name, int namelen, PVOID lpSendBuffer,
	DWORD dwSendDataLength, LPDWORD lpdwBytesSent, LPOVERLAPPED lpOverlapped)
{
	LPFN_CONNECTEX ConnectExPtr = NULL;
	DWORD nBytes;
	GUID guid = WSAID_CONNECTEX;
	int fetched = WSAIoctl(s, SIO_GET_EXTENSION_FUNCTION_POINTER,
		(void*)&guid, sizeof(GUID),
		&ConnectExPtr, sizeof(LPFN_CONNECTEX),
		&nBytes, NULL, NULL);

	if (fetched == 0 && ConnectExPtr != NULL)
	{
		ConnectExPtr(s, name, namelen, lpSendBuffer, dwSendDataLength, lpdwBytesSent, lpOverlapped);
	}

	return false;
}

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

duk_context* CScriptSystem::m_Ctx = NULL;

vector<EVENTHOOK> CScriptSystem::m_Hooks;

CCallbackList CScriptSystem::m_ExecEvents;
CCallbackList CScriptSystem::m_ReadEvents;
CCallbackList CScriptSystem::m_WriteEvents;
CCallbackList CScriptSystem::m_WMEvents;

CRITICAL_SECTION CScriptSystem::m_CtxProtected;

HANDLE CScriptSystem::m_ioEventsThread;
HANDLE CScriptSystem::m_ioBasePort;
vector<IOLISTENER*> CScriptSystem::m_ioListeners;
UINT CScriptSystem::m_ioNextListenerId;

vector<IOFD> CScriptSystem::m_ioFds;

// native module test
//typedef int(__stdcall *ModuleFunction)(duk_context*);

void CScriptSystem::Init()
{
	m_Ctx = duk_create_heap_default();
	
	const duk_function_list_entry _native[] = {
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

	m_ioNextListenerId = 0;
	m_ioBasePort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

	EvalFile("_api.js");
	EvalFile("_script.js");
	
	InitializeCriticalSection(&m_CtxProtected); // todo cleanup

	// native module test
	/*
	HINSTANCE hNativeModule = LoadLibrary("test.dll");
	// FreeLibrary(hNativeModule)
	if (hNativeModule)
	{
		ModuleFunction DllMain = (ModuleFunction) GetProcAddress(hNativeModule, "DllMain");
		if (DllMain)
		{
			DllMain(m_Ctx);
		}
	}
	*/
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

void CScriptSystem::Invoke(void* heapptr)
{
	duk_push_heapptr(m_Ctx, heapptr);
	
	duk_int_t status = duk_pcall(m_Ctx, 0);
	
	if (status != DUK_EXEC_SUCCESS)
	{
		const char* msg = duk_safe_to_string(m_Ctx, -1);
		MessageBox(NULL, msg, "Script error", MB_OK | MB_ICONWARNING);
	}
	
	duk_pop(m_Ctx);
}

IOLISTENER* CScriptSystem::ioAddListener(HANDLE fd, IOEVENTTYPE evt, void* callback, void* data, int dataLen)
{
	IOLISTENER* lpListener = (IOLISTENER*) malloc(sizeof(IOLISTENER));
	//OVERLAPPED* lpOvl = (OVERLAPPED*)lpListener;
	memset(lpListener, 0x00, sizeof(IOLISTENER));
	
	lpListener->id = m_ioNextListenerId++;
	lpListener->eventType = evt;
	lpListener->fd = fd;
	lpListener->callback = callback;
	lpListener->data = data;
	lpListener->dataLen = dataLen;

	m_ioListeners.push_back(lpListener);

	return lpListener;
}

void CScriptSystem::ioRemoveListenerByIndex(UINT index)
{
	IOLISTENER* lpListener = m_ioListeners[index];

	if (lpListener->data != NULL)
	{
		free(lpListener->data);
	}

	CancelIoEx(lpListener->fd, (LPOVERLAPPED) lpListener);

	free(lpListener);

	m_ioListeners.erase(m_ioListeners.begin() + index);
}

// Free listener & its buffer, remove from list
void CScriptSystem::ioRemoveListenerByPtr(IOLISTENER* lpListener)
{
	for (UINT i = 0; i < m_ioListeners.size(); i++)
	{
		if (m_ioListeners[i] == lpListener)
		{
			ioRemoveListenerByIndex(i);
			break;
		}
	}
}

void CScriptSystem::ioRemoveListenersByFd(HANDLE fd)
{
	for (UINT i = 0; i < m_ioListeners.size(); i++)
	{
		if (m_ioListeners[i]->fd == fd)
		{
			ioRemoveListenerByIndex(i);
		}
	}
}

void CScriptSystem::ioDoEvent(IOLISTENER* lpListener)
{
	if (lpListener->callback == NULL)
	{
		return;
	}
	
	duk_push_heapptr(m_Ctx, lpListener->callback);

	int nargs = 0;

	switch (lpListener->eventType)
	{
	case EVT_READ:
		nargs = 1;
		if (lpListener->dataLen > 0)
		{
			void* data = duk_push_buffer(m_Ctx, lpListener->dataLen, false);
			memcpy(data, lpListener->data, lpListener->dataLen);
		}
		else
		{
			// handle must have closed, safe to untrack fd and remove all associated listeners
			ioRemoveFd(lpListener->fd);

			// pass null to callback
			duk_push_null(m_Ctx);
		}
		break;
	case EVT_WRITE:
		nargs = 1;
		duk_push_uint(m_Ctx, lpListener->dataLen); // num bytes written
		break;
	case EVT_ACCEPT:
		// pass client socket fd to callback
		nargs = 1;
		duk_push_uint(m_Ctx, (UINT)lpListener->childFd);
		break;
	case EVT_CONNECT:
		nargs = 0;
		break;
	}
	
	duk_int_t status = duk_pcall(m_Ctx, nargs);

	if (status != DUK_EXEC_SUCCESS)
	{
		const char* msg = duk_safe_to_string(m_Ctx, -1);
		MessageBox(NULL, msg, "Script error", MB_OK | MB_ICONWARNING);
	}
}

DWORD WINAPI CScriptSystem::ioEventsProc(void* param)
{
	UNREFERENCED_PARAMETER(param);

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
			int err = GetLastError();

			if (err == STATUS_USER_APC)
			{
				// Interrupted by an async proc call
				continue;
			}
			
			char errmsg[128];
			sprintf(errmsg, "GetQueuedCompletionStatus error (%d)", err);
			MessageBox(NULL, errmsg, "Error", MB_OK);
			break;
		}

		IOLISTENER* lpListener = (IOLISTENER*)lpUsedOvl;
		lpListener->dataLen = nBytesTransferred;

		// Protect from emulation thread (onexec, onread, etc)
		EnterCriticalSection(&m_CtxProtected);

		ioDoEvent(lpListener);

		LeaveCriticalSection(&m_CtxProtected);

		// Destroy listener
		ioRemoveListenerByPtr(lpListener);
	}

	return 0;
}

HANDLE CScriptSystem::ioCreateExistingFile(const char* path)
{
	HANDLE fd = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ,
		NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	ioAddFd(fd);
	return fd;
}

HANDLE CScriptSystem::ioSockCreate()
{
	HANDLE fd = (HANDLE)WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	ioAddFd(fd, true);
	return fd;
}

void CScriptSystem::ioAddFd(HANDLE fd, bool bSocket)
{
	IOFD iofd;
	iofd.fd = fd;
	iofd.iocp = CreateIoCompletionPort(fd, m_ioBasePort, (ULONG_PTR)fd, 0);
	iofd.bSocket = bSocket;
	m_ioFds.push_back(iofd);
}

void CScriptSystem::ioCloseFd(HANDLE fd)
{
	// Causes EVT_READ with length 0
	// Not safe to remove listeners until then

	for (uint32_t i = 0; i < m_ioFds.size(); i++)
	{
		IOFD iofd = m_ioFds[i];
		if (iofd.fd != fd)
		{
			continue;
		}
		
		// Close file handle
		if (iofd.bSocket)
		{
			closesocket((SOCKET)iofd.fd);
		}
		else
		{
			CloseHandle(iofd.fd);
		}
		break;
	}
}

void CScriptSystem::ioRemoveFd(HANDLE fd)
{
	// Stop tracking an fd and remove all of its listeners
	for (uint32_t i = 0; i < m_ioFds.size(); i++)
	{
		IOFD iofd = m_ioFds[i];
		if (iofd.fd != fd)
		{
			continue;
		}

		m_ioFds.erase(m_ioFds.begin() + i);
		ioRemoveListenersByFd(fd);
		break;
	}
}

duk_ret_t CScriptSystem::js_ioSockConnect(duk_context* ctx)
{
	HANDLE fd = (HANDLE) duk_get_uint(ctx, 0);
	const char* ipStr = duk_to_string(ctx, 1);
	USHORT port = (USHORT)duk_get_uint(ctx, 2);
	void* callback = duk_get_heapptr(ctx, 3);
	
	char ipBytes[sizeof(uint32_t)];
	sscanf(ipStr, "%hhu.%hhu.%hhu.%hhu", &ipBytes[0], &ipBytes[1], &ipBytes[2], &ipBytes[3]);
	
	SOCKADDR_IN serverAddr;
	serverAddr.sin_addr.s_addr = *(uint32_t*)ipBytes;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_family = AF_INET;
	
	SOCKADDR_IN clientAddr;
	clientAddr.sin_addr.s_addr = INADDR_ANY;
	clientAddr.sin_port = 0;
	clientAddr.sin_family = AF_INET;

	::bind((SOCKET)fd, (SOCKADDR*)&clientAddr, sizeof(SOCKADDR));

	IOLISTENER* lpListener = ioAddListener(fd, EVT_CONNECT, callback);
	ConnectEx((SOCKET)fd, (SOCKADDR*)&serverAddr, sizeof(SOCKADDR), NULL, 0, NULL, (LPOVERLAPPED)lpListener);

	duk_pop_n(ctx, 4);
	return 1;
}

duk_ret_t CScriptSystem::js_ioClose(duk_context* ctx)
{
	HANDLE fd = (HANDLE)duk_get_uint(ctx, 0);
	duk_pop(ctx);
	ioCloseFd(fd);
	return 1;
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
	USHORT port = (USHORT)duk_get_uint(ctx, 1);
	
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
	
	void* data = malloc(sizeof(SOCKADDR) * 4); // issue?

	IOLISTENER* lpListener = ioAddListener(fd, EVT_ACCEPT, jsCallback, data, 0);

	lpListener->childFd = ioSockCreate();

	AcceptEx(
		(SOCKET)fd,
		(SOCKET)lpListener->childFd,
		lpListener->data, // local and remote SOCKADDR
		0,
		sizeof(SOCKADDR_IN) + 16,
		sizeof(SOCKADDR_IN) + 16,
		NULL,
		(LPOVERLAPPED) lpListener
	);
	
	duk_pop_n(ctx, 2);
	return 1;
}

duk_ret_t CScriptSystem::js_ioRead(duk_context* ctx)
{
	// (fd, bufferSize, callback)
	HANDLE fd = (HANDLE)duk_get_uint(ctx, 0);
	size_t bufferSize = duk_get_uint(ctx, 1);
	void* jsCallback = duk_get_heapptr(ctx, 2);
	
	void* data = malloc(bufferSize); // freed after event is fired
	
	IOLISTENER* lpListener = ioAddListener(fd, EVT_READ, jsCallback, data, bufferSize);
	BOOL status = ReadFile(fd, lpListener->data, lpListener->dataLen, NULL, (LPOVERLAPPED) lpListener);
	
	if (status == false && GetLastError() != ERROR_IO_PENDING)
	{
		MessageBox(NULL, "readex error", "", MB_OK);
	}
	
	duk_pop_n(ctx, 3);
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
	
	IOLISTENER* lpListener = ioAddListener(fd, EVT_WRITE, jsCallback, data, dataLen);
	WriteFile(fd, lpListener->data, lpListener->dataLen, NULL, (LPOVERLAPPED)lpListener);

	duk_pop_n(ctx, 3);
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
	
	int callbackId = -1;

	CCallbackList* cbList = GetCallbackList(hookId);
	if (cbList != NULL)
	{
		callbackId = cbList->Add(heapptr, tag);
	}
	
	duk_pop_n(ctx, 2);

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
	g_Reg->m_GPR[regnum].UW[0] = val;
	duk_pop_n(ctx, 1);
	duk_push_uint(ctx, val);
	return 1;
}

duk_ret_t CScriptSystem::js_GetRDRAMInt(duk_context* ctx)
{
	uint32_t address = duk_to_uint32(ctx, 0);
	int bitwidth = duk_to_int(ctx, 1);
	duk_bool_t bSigned = duk_to_boolean(ctx, 2);

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
		if (!g_MMU->SB_VAddr(address, (uint8_t)newValue))
		{
			goto return_err;
		}
		goto return_ok;
	case 16:
		if (!g_MMU->SH_VAddr(address, (uint16_t)newValue))
		{
			goto return_err;
		}
		goto return_ok;
	case 32:
		if (!g_MMU->SW_VAddr(address, (uint32_t)newValue))
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
	duk_bool_t bDouble = false;

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
	duk_bool_t bDouble = false;

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

// Return zero-terminated string from ram
duk_ret_t CScriptSystem::js_GetRDRAMString(duk_context* ctx)
{
	// (address)
	uint32_t address = duk_get_uint(ctx, 0);

	uint8_t test = 0xFF;
	int len = 0;

	// determine length of string
	while (g_MMU->LB_VAddr(address + len, test) && test != 0) // todo protect from ram overrun
	{
		len++;
	}
	
	uint8_t* str = (uint8_t*)malloc(len);
	
	for (int i = 0; i < len; i++)
	{
		str[i] = g_MMU->LB_VAddr(address + i, str[i]);
	}
	
	duk_pop(ctx);
	duk_push_string(ctx, (char*)str);
	free(str); // duk creates internal copy
	return 1;
}