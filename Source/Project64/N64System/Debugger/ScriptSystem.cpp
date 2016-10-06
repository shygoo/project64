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
			CScriptSystem::Invoke(m_Callbacks[i].heapptr);
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
			CScriptSystem::Invoke(m_Callbacks[i].heapptr);
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

void CScriptSystem::Init()
{
	m_Ctx = duk_create_heap_default();

	duk_push_object(m_Ctx);
	duk_put_global_string(m_Ctx, "_native");

	RegisterCallbackList("exec", &m_ExecEvents);
	RegisterCallbackList("read", &m_ReadEvents);
	RegisterCallbackList("write", &m_WriteEvents);
	RegisterCallbackList("wm", &m_WMEvents);

	BindNativeFunction("addCallback", AddCallback);
	BindNativeFunction("setGPRVal", SetGPRVal);
	BindNativeFunction("getGPRVal", GetGPRVal);

	BindNativeFunction("getRDRAMU8", GetRDRAMU8);
	BindNativeFunction("getRDRAMU16", GetRDRAMU16);
	BindNativeFunction("getRDRAMU32", GetRDRAMU32);
	BindNativeFunction("setRDRAMU8", SetRDRAMU8);
	BindNativeFunction("setRDRAMU16", SetRDRAMU16);
	BindNativeFunction("setRDRAMU32", SetRDRAMU32);

	BindNativeFunction("sockCreate", _ioSockCreate);
	BindNativeFunction("sockListen", _ioSockListen);
	BindNativeFunction("sockAccept", _ioSockAccept);
	BindNativeFunction("addListener", _ioAddListener);
	BindNativeFunction("write", _ioWrite);
	BindNativeFunction("read", _ioRead);
	//BindNativeFunction("sockReceive", SockReceive);
	
	BindNativeFunction("msgBox", MsgBox);
	
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


	//HANDLE fd = ioSockCreate();
	//ioSockListen(fd, 80);
	//ioSockAccept(fd, NULL);

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
		MessageBox(NULL, msg, "Script error", MB_OK);
	}
	duk_pop(m_Ctx);
}

void CScriptSystem::EvalFile(const char* jsPath)
{
	int result = duk_peval_file(m_Ctx, jsPath);
	const char* msg = duk_safe_to_string(m_Ctx, -1);
	if (result != 0)
	{
		MessageBox(NULL, msg, jsPath, MB_OK);
	}
	duk_pop(m_Ctx);
}

void CScriptSystem::BindGlobalFunction(const char* name, duk_c_function func)
{
	duk_push_c_function(m_Ctx, func, DUK_VARARGS);
	duk_put_global_string(m_Ctx, name);
}

void CScriptSystem::BindNativeFunction(const char* name, duk_c_function func)
{
	duk_get_global_string(m_Ctx, "_native");
	duk_push_c_function(m_Ctx, func, DUK_VARARGS);
	duk_put_prop_string(m_Ctx, 0, name);
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

duk_ret_t CScriptSystem::AddCallback(duk_context* ctx)
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

duk_ret_t CScriptSystem::GetGPRVal(duk_context* ctx)
{
	int regnum = duk_to_int(ctx, 0);
	duk_pop_n(ctx, 1);
	duk_push_uint(ctx, g_Reg->m_GPR[regnum].UW[0]);
	return 1;
}

duk_ret_t CScriptSystem::SetGPRVal(duk_context* ctx)
{
	int regnum = duk_to_int(ctx, 0);
	uint32_t val = duk_to_uint32(ctx, 1);
	duk_pop_n(ctx, 1);
	g_Reg->m_GPR[regnum].UW[0] = val;
	duk_push_uint(ctx, val);
	return 1;
}

duk_ret_t CScriptSystem::GetRDRAMU8(duk_context* ctx)
{
	uint32_t address = duk_to_uint32(ctx, 0);
	duk_pop(ctx);
	uint8_t val = 0;
	if (g_MMU != NULL)
	{
		g_MMU->LB_VAddr(address, val);
	}
	duk_push_int(ctx, val);
	return 1;
}

duk_ret_t CScriptSystem::GetRDRAMU16(duk_context* ctx)
{
	uint32_t address = duk_to_uint32(ctx, 0);
	duk_pop(ctx);
	uint16_t val = 0;
	if (g_MMU != NULL)
	{
		g_MMU->LH_VAddr(address, val);
	}
	duk_push_int(ctx, val);
	return 1;
}

duk_ret_t CScriptSystem::GetRDRAMU32(duk_context* ctx)
{
	uint32_t address = duk_to_uint32(ctx, 0);
	duk_pop(ctx);
	uint32_t val = 0;
	if (g_MMU != NULL)
	{
		g_MMU->LW_VAddr(address, val);
	}
	duk_push_int(ctx, val);
	return 1;
}

duk_ret_t CScriptSystem::SetRDRAMU8(duk_context* ctx)
{
	uint32_t address = duk_to_uint32(ctx, 0);
	uint8_t val = duk_to_int(ctx, 1);
	duk_pop_n(ctx, 2);
	if (g_MMU != NULL)
	{
		g_MMU->SB_VAddr(address, val);
	}
	duk_push_int(ctx, val);
	return 1;
}

duk_ret_t CScriptSystem::SetRDRAMU16(duk_context* ctx)
{
	uint32_t address = duk_to_uint32(ctx, 0);
	uint16_t val = duk_to_int(ctx, 1);
	duk_pop_n(ctx, 2);
	if (g_MMU != NULL)
	{
		g_MMU->SH_VAddr(address, val);
	}
	duk_push_int(ctx, val);
	return 1;
}

duk_ret_t CScriptSystem::SetRDRAMU32(duk_context* ctx)
{
	uint32_t address = duk_to_uint32(ctx, 0);
	uint32_t val = duk_to_int(ctx, 1);
	duk_pop_n(ctx, 2);
	if (g_MMU != NULL)
	{
		g_MMU->SW_VAddr(address, val);
	}
	duk_push_int(ctx, val);
	return 1;
}

duk_ret_t CScriptSystem::MsgBox(duk_context* ctx)
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

HANDLE CScriptSystem::m_ioEventsThread;
HANDLE CScriptSystem::m_ioBasePort;
char   CScriptSystem::m_ioBaseBuf[8192]; // io buffered here
int    CScriptSystem::m_ioBaseBufLen;
vector<IOListener> CScriptSystem::m_ioListeners;

void CScriptSystem::ioAddListener(HANDLE fd, EVENTTYPE evt, void* callback, void* data, int dataLen)
{
	int len = m_ioListeners.size();
	bool foundReplacement = false;

	IOListener* lpListener;

	for (int i = 0; i < len; i++)
	{
		// replace dead listener
		if (m_ioListeners[i].finished == true)
		{
			lpListener = &m_ioListeners[i];
			*lpListener = { 0 };
			foundReplacement = true;
			break;
		}
	}

	if (!foundReplacement)
	{
		m_ioListeners.push_back({ 0 });
		lpListener = &m_ioListeners[m_ioListeners.size() - 1];
	}
	
	OVERLAPPED* lpOvl = (OVERLAPPED*)lpListener;

	lpListener->evt = evt;
	lpListener->fd = fd;
	lpListener->callback = callback;
	lpListener->data = data;
	lpListener->dataLen = dataLen;

	switch (evt)
	{
	case EVT_READ:
		ReadFile(fd, lpListener->data, lpListener->dataLen, NULL, lpOvl);
		break;

	case EVT_WRITE:
		WriteFile(fd, lpListener->data, lpListener->dataLen, NULL, lpOvl);
		break;

	case EVT_ACCEPT:
		// client socket
		lpListener->childFd = ioSockCreate();
		AcceptEx(
			(SOCKET)fd,
			(SOCKET)lpListener->childFd,
			m_ioBaseBuf,
			0,
			sizeof(SOCKADDR_IN) + 16,
			sizeof(SOCKADDR_IN) + 16,
			NULL,
			lpOvl
		);
		break;
	}
}

void CScriptSystem::ioDoEvent(IOListener* lpListener)
{
	switch (lpListener->evt)
	{
	case EVT_READ:
		MessageBox(NULL, "read fired", "read", MB_OK);
		duk_push_heapptr(m_Ctx, lpListener->callback);
		duk_push_external_buffer(m_Ctx);
		duk_config_buffer(m_Ctx, -1, lpListener->data, lpListener->dataLen);
		duk_call(m_Ctx, 1);
		duk_pop(m_Ctx);
		break;

	case EVT_WRITE:
		// todo pass num bytes transferred to callback
		duk_push_heapptr(m_Ctx, lpListener->callback);
		duk_call(m_Ctx, 0);
		duk_pop(m_Ctx);
		break;

	case EVT_ACCEPT:
		// pass client socket fd to callback
		duk_push_heapptr(m_Ctx, lpListener->callback);
		duk_push_uint(m_Ctx, (UINT)lpListener->childFd);
		duk_call(m_Ctx, 1);
		duk_pop(m_Ctx);
		break;
	}
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
	HANDLE iocp = CreateIoCompletionPort(fd, m_ioBasePort, (ULONG_PTR)fd, 0);
	return fd;
}

bool CScriptSystem::ioSockListen(HANDLE fd, USHORT port)
{
	SOCKET sock = (SOCKET)fd;
	SOCKADDR_IN serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(port);
	if (::bind(sock, (SOCKADDR*) &serverAddr, sizeof(serverAddr)))
	{
		MessageBox(NULL, "couldnt bind socket", "tes", MB_OK);
		return false;
	}
	listen(sock, 3);
	return true;
}

bool CScriptSystem::ioSockAccept(HANDLE fd, void* jsCallback)
{
	ioAddListener(fd, EVT_ACCEPT, jsCallback);
	return true;
}

DWORD WINAPI CScriptSystem::ioEventsProc(void* param)
{
	while (1)
	{
		//MessageBox(NULL, "Events proc running", "ok", MB_OK);

		OVERLAPPED_ENTRY usedOvlEntry;
		ULONG nUsedOverlaps;

		LPOVERLAPPED& lpUsedOvl = usedOvlEntry.lpOverlapped;
		DWORD& nBytesTransferred = usedOvlEntry.dwNumberOfBytesTransferred;

		// Wait for an IO completion or async proc call
		BOOL status = GetQueuedCompletionStatusEx(
			m_ioBasePort,
			&usedOvlEntry,
			1,
			&nUsedOverlaps,
			INFINITE,
			TRUE
		);
		
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

		HANDLE fd = lpListener->fd;
		EVENTTYPE evt = lpListener->evt;
		
		// Protect from emulation thread (onexec, onread, etc)
		m_CtxMutex.lock();

		ioDoEvent(lpListener);

		if (lpListener->data != NULL)
		{
			free(lpListener->data);
			lpListener->finished = false;
		}

		m_CtxMutex.unlock();

		int lastListenerIdx = m_ioListeners.size();
		//todo peel off dead listeners
	}

	return 0;
}

duk_ret_t CScriptSystem::_ioAddListener(duk_context* ctx)
{
	HANDLE fd = (HANDLE)duk_get_uint(ctx, 0);
	EVENTTYPE evt = (EVENTTYPE)duk_get_int(ctx, 1);
	void* jsCallback = duk_get_heapptr(ctx, 2);
	duk_pop_n(ctx, 3);
	ioAddListener(fd, evt, jsCallback);
	return 1;
}

duk_ret_t CScriptSystem::_ioSockCreate(duk_context* ctx)
{
	duk_pop_n(ctx, duk_get_top(ctx));
	duk_push_int(ctx, (int)ioSockCreate());
	return 1;
}

duk_ret_t CScriptSystem::_ioSockListen(duk_context* ctx)
{
	//MessageBox(NULL, "sock listening", "ok", MB_OK);
	HANDLE fd = (HANDLE)duk_get_uint(ctx, 0);
	USHORT port = duk_get_uint(ctx, 1);
	duk_pop_n(ctx, 2);
	ioSockListen(fd, port);
	return 1;
}

duk_ret_t CScriptSystem::_ioSockAccept(duk_context* ctx)
{
	//MessageBox(NULL, "sock accepting", "ok", MB_OK);
	HANDLE fd = (HANDLE)duk_get_uint(ctx, 0);
	void* jsCallback = duk_get_heapptr(ctx, 1);
	duk_pop_n(ctx, 2);
	ioSockAccept(fd, jsCallback);
	return 1;
}

duk_ret_t CScriptSystem::_ioRead(duk_context* ctx)
{
	// (fd, bufferSize, callback)
	HANDLE fd = (HANDLE)duk_get_uint(ctx, 0);
	size_t bufferSize = duk_get_uint(ctx, 1);
	void* jsCallback = duk_get_heapptr(ctx, 2);
	duk_pop_n(ctx, 3);

	void* data = malloc(bufferSize); // freed after event is fired

	ioAddListener(fd, EVT_READ, jsCallback, data, bufferSize);
	return 1;
}

duk_ret_t CScriptSystem::_ioWrite(duk_context* ctx)
{
	HANDLE fd = (HANDLE)duk_get_uint(ctx, 0);

	duk_size_t dataLen;
	void* jsData = duk_to_buffer(ctx, 1, &dataLen);

	void* jsCallback = duk_get_heapptr(ctx, 2);

	char* data = (char*)malloc(dataLen + 1); // freed after event is fired
	memcpy(data, jsData, dataLen);
	data[dataLen] = '\0';
	
	duk_pop_n(ctx, 3);
	// need buffer & size
	ioAddListener(fd, EVT_WRITE, jsCallback, data, dataLen);
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