#include <stdafx.h>

#include "ScriptHook.h"
#include "ScriptInstance.h"

int CScriptHook::Add(CScriptInstance* scriptInstance, void* heapptr, uint32_t param, bool bOnce)
{
	JSCALLBACK jsCallback;
	jsCallback.scriptInstance = scriptInstance;
	jsCallback.heapptr = heapptr;
	jsCallback.callbackId = m_NextCallbackId;
	jsCallback.param = param;
	jsCallback.bOnce = bOnce;
	m_Callbacks.push_back(jsCallback);
	return m_NextCallbackId++;
}

void CScriptHook::InvokeById(int callbackId)
{
	int nCallbacks = m_Callbacks.size();
	for (int i = 0; i < nCallbacks; i++)
	{
		if (m_Callbacks[i].callbackId == callbackId)
		{
			m_Callbacks[i].scriptInstance->Invoke(m_Callbacks[i].heapptr);
			return;
		}
	}
}

void CScriptHook::InvokeByParam(uint32_t param)
{
	int nCallbacks = m_Callbacks.size();
	for (int i = 0; i < nCallbacks; i++)
	{
		if (m_Callbacks[i].param == param)
		{
			m_Callbacks[i].scriptInstance->Invoke(m_Callbacks[i].heapptr);
			return;
		}
	}
}

void CScriptHook::InvokeAll()
{
	int nCallbacks = m_Callbacks.size();
	for (int i = 0; i < nCallbacks; i++)
	{
		m_Callbacks[i].scriptInstance->Invoke(m_Callbacks[i].heapptr);
	}
}

void CScriptHook::RemoveById(int callbackId)
{
	int nCallbacks = m_Callbacks.size();
	for (int i = 0; i < nCallbacks; i++)
	{
		if (m_Callbacks[i].callbackId == callbackId)
		{
			m_Callbacks.erase(m_Callbacks.begin() + i);
			return;
		}
	}
}

void CScriptHook::RemoveByParam(uint32_t param)
{
	int nCallbacks = m_Callbacks.size();
	for (int i = 0; i < nCallbacks; i++)
	{
		if (m_Callbacks[i].param == param)
		{
			m_Callbacks.erase(m_Callbacks.begin() + i);
			return;
		}
	}
}

void CScriptHook::RemoveByInstance(CScriptInstance* scriptInstance)
{
	for (int i = 0; i < m_Callbacks.size(); i++)
	{
		if (m_Callbacks[i].scriptInstance == scriptInstance)
		{
			m_Callbacks.erase(m_Callbacks.begin() + i);
		}
	}
}

bool CScriptHook::HasContext(CScriptInstance* scriptInstance)
{
	for (int i = 0; i < m_Callbacks.size(); i++)
	{
		if (m_Callbacks[i].scriptInstance == scriptInstance)
		{
			return true;
		}
	}
	return false;
}

CScriptHook::CScriptHook()
{
}

CScriptHook::~CScriptHook()
{
	m_Callbacks.clear();
}