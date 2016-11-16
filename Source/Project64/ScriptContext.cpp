#include "ScriptContext.h"

vector<CScriptContext*> CScriptContext::Cache;

CScriptContext::CScriptContext(char* path)
{
	Cache.push_back(this);
	m_Path = path;

	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)StartScriptProc, this, 0, NULL);
}

CScriptContext::~CScriptContext()
{
	CloseHandle(m_hThread);
	DeleteCriticalSection(&m_CriticalSection);
	duk_destroy_heap(m_Ctx);
	// delete from Cache
	// todo clear callbacks
	//free(jsContext);
	//m_Contexts.erase(m_Contexts.begin() + i);
}

DWORD CALLBACK CScriptContext::StartScriptProc(CScriptContext* _this)
{
	duk_context* ctx = _this->m_Ctx;

	duk_push_object(ctx);
	duk_put_global_string(ctx, "_native");
	duk_get_global_string(ctx, "_native");
	duk_put_function_list(ctx, -1, NativeFunctions);
	duk_pop(ctx);

	duk_peval_file(ctx, "_api.js");

	if (_this->m_Path != NULL)
	{
		duk_peval_file(ctx, _this->m_Path);
	}

	StartEventLoop(_this);
}


void CScriptContext::StartEventLoop(CScriptContext* _this)
{
	// todo while(listeners > 0)
	while (true)
	{
		OVERLAPPED_ENTRY usedOvlEntry;
		ULONG nUsedOverlaps;

		// Wait for an IO completion or async proc call
		BOOL status = GetQueuedCompletionStatusEx(
			_this->m_hIOCompletionPort,
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
		EnterCriticalSection(&_this->m_CriticalSection);

		_this->InvokeListenerEvent(lpListener);

		LeaveCriticalSection(&_this->m_CriticalSection);

		// Destroy listener
		_this->RemoveListenerByPtr(lpListener);
	}
}

void CScriptContext::InvokeListenerEvent(IOLISTENER* lpListener)
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
			RemoveFile(lpListener->fd);

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