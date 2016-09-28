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

#pragma comment(lib, "Ws2_32.lib")

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

	RegisterCallbackList("exec", &m_ExecEvents);
	RegisterCallbackList("read", &m_ReadEvents);
	RegisterCallbackList("write", &m_WriteEvents);
	RegisterCallbackList("wm", &m_WMEvents);

	BindGlobalFunction("_AddCallback", AddCallback);
	BindGlobalFunction("_SetGPRVal", SetGPRVal);
	BindGlobalFunction("_GetGPRVal", GetGPRVal);
	BindGlobalFunction("_GetRDRAMU8", GetRDRAMU8);
	BindGlobalFunction("_GetRDRAMU16", GetRDRAMU16);
	BindGlobalFunction("_GetRDRAMU32", GetRDRAMU32);
	BindGlobalFunction("_SetRDRAMU8", SetRDRAMU8);
	BindGlobalFunction("_SetRDRAMU16", SetRDRAMU16);
	BindGlobalFunction("_SetRDRAMU32", SetRDRAMU32);
	BindGlobalFunction("alert", alert);
	//BindGlobalFunction("_NewWindow", _NewWindow);
	//BindGlobalFunction("_AddCtrl", _AddCtrl);
	//BindGlobalFunction("_AddMessageHandler", _AddMessageHandler);
	BindGlobalFunction("_CreateThread", CreateThread);
	BindGlobalFunction("_TerminateThread", TerminateThread);
	BindGlobalFunction("_SuspendThread", SuspendThread);
	BindGlobalFunction("_ResumeThread", ResumeThread);
	BindGlobalFunction("_Sleep", Sleep);

	BindGlobalFunction("_CreateServer", CreateServer);
	BindGlobalFunction("_ReceiveBytes", ReceiveBytes);
	BindGlobalFunction("_SockAccept", SockAccept);

	EvalFile("_api.js");

	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

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

	return callbackId;
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

duk_ret_t CScriptSystem::alert(duk_context* ctx)
{
	int argc = duk_get_top(ctx);
	const char* msg = duk_to_string(ctx, 0);
	MessageBox(NULL, msg, "", MB_OK);
	duk_pop_n(ctx, 1);
	duk_push_boolean(ctx, 1);
	return 1;
}

DWORD WINAPI CScriptSystem::ThreadProc(LPVOID lpDukProcHeapPtr)
{
	Invoke(lpDukProcHeapPtr);
	return 0;
}

duk_ret_t CScriptSystem::CreateThread(duk_context* ctx)
{
	void* lpDukProcHeapPtr = duk_get_heapptr(ctx, 0);
	duk_pop(ctx);
	HANDLE hWorkerThread = ::CreateThread(NULL, 0, ThreadProc, lpDukProcHeapPtr, 0, NULL);
	duk_push_uint(ctx, (UINT)hWorkerThread);
	return 1;
}

duk_ret_t CScriptSystem::TerminateThread(duk_context* ctx)
{
	HANDLE hThread = (HANDLE)duk_get_uint(ctx, 0);
	duk_pop(ctx);
	::TerminateThread(hThread, 0);
	CloseHandle(hThread);
	return 1;
}

duk_ret_t CScriptSystem::SuspendThread(duk_context* ctx)
{
	HANDLE hThread = (HANDLE)duk_get_uint(ctx, 0);
	duk_pop(ctx);
	::SuspendThread(hThread);
	return 1;
}

duk_ret_t CScriptSystem::ResumeThread(duk_context* ctx)
{
	HANDLE hThread = (HANDLE)duk_get_uint(ctx, 0);
	duk_pop(ctx);
	::ResumeThread(hThread);
	return 1;
}

duk_ret_t CScriptSystem::Sleep(duk_context* ctx)
{
	DWORD milliseconds = duk_get_uint(ctx, 0);
	duk_pop(ctx);
	::Sleep(milliseconds);
	return 1;
}

duk_ret_t CScriptSystem::CreateServer(duk_context* ctx)
{
	USHORT port = (USHORT)duk_get_uint(ctx, 0);
	duk_pop(ctx);

	SOCKET sock;
	struct sockaddr_in server;

	sock = socket(AF_INET, SOCK_STREAM, 0);

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(port);

	if (::bind(sock, (struct sockaddr*) &server, sizeof(server))) {
		duk_push_int(ctx, INVALID_SOCKET);
		return 1;
	}

	listen(sock, 3);
	duk_push_int(ctx, sock);
	return 1;
}

duk_ret_t CScriptSystem::ReceiveBytes(duk_context* ctx) {
	// (socketd, nBytes)

	SOCKET socket = (SOCKET)duk_get_int(ctx, 0);
	int nBytes = duk_get_int(ctx, 1);

	duk_pop_n(ctx, 2);

	char* bytes = (char*)malloc(nBytes);
	uint64_t bytesRead = 0;

	while (bytesRead < nBytes) {
		int n = recv(socket, bytes + bytesRead, nBytes - bytesRead, 0);
		if (n < 0) {
			return n;
		}
		bytesRead += n;
	}
	
	void* jsBuf = duk_push_buffer(ctx, nBytes, 0);
	memcpy(jsBuf, bytes, nBytes);
	free(bytes);

	return 1;
}

duk_ret_t CScriptSystem::SockAccept(duk_context* ctx) {
	// (serversocketd)

	SOCKET serverSocket = (SOCKET)duk_get_int(ctx, 0);
	duk_pop(ctx);

	struct sockaddr_in client;
	int addrSize = sizeof(struct sockaddr_in);

	SOCKET clientSocket = accept(serverSocket, (struct sockaddr*)&client, &addrSize);
	
	duk_push_int(ctx, (int)clientSocket);

	return 1;
}

/******************** windows **********************/

/*
static void DrawTest() {
	HWND renderWindow = (HWND)g_Plugins->MainWindow()->GetWindowHandle();
	HDC hdc = GetDC(renderWindow);

	HFONT monoFont = CreateFont(-13, 0, 0, 0,
		FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, FF_DONTCARE, "Courier New"
	);
	HBRUSH whiteBrush = CreateSolidBrush(RGB(0xFF, 0xFF, 0xFF));
	HPEN pen = CreatePen(PS_SOLID, 3, RGB(0, 0, 0));

	SelectObject(hdc, monoFont);
	SelectObject(hdc, pen);
	SelectObject(hdc, whiteBrush);

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