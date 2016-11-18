#include <stdafx.h>

#include "ScriptHook.h"
#include "ScriptContext.h"

int CScriptHook::Add(CScriptContext* scriptContext, void* heapptr, uint32_t tag)
{
	m_Callbacks.push_back({ scriptContext, heapptr, tag, m_NextCallbackId });
	return m_NextCallbackId++;
}

void CScriptHook::InvokeById(int callbackId)
{
	int nCallbacks = m_Callbacks.size();
	for (int i = 0; i < nCallbacks; i++)
	{
		if (m_Callbacks[i].callbackId == callbackId)
		{
			m_Callbacks[i].scriptContext->Invoke(m_Callbacks[i].heapptr);
			return;
		}
	}
}

void CScriptHook::InvokeByTag(uint32_t tag)
{
	int nCallbacks = m_Callbacks.size();
	for (int i = 0; i < nCallbacks; i++)
	{
		if (m_Callbacks[i].tag == tag)
		{
			m_Callbacks[i].scriptContext->Invoke(m_Callbacks[i].heapptr);
			return;
		}
	}
}

void CScriptHook::InvokeAll()
{
	int nCallbacks = m_Callbacks.size();
	for (int i = 0; i < nCallbacks; i++)
	{
		m_Callbacks[i].scriptContext->Invoke(m_Callbacks[i].heapptr);
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

void CScriptHook::RemoveByTag(uint32_t tag)
{
	int nCallbacks = m_Callbacks.size();
	for (int i = 0; i < nCallbacks; i++)
	{
		if (m_Callbacks[i].tag == tag)
		{
			m_Callbacks.erase(m_Callbacks.begin() + i);
			return;
		}
	}
}

void CScriptHook::RemoveByContext(CScriptContext* scriptContext)
{
	for (int i = 0; i < m_Callbacks.size(); i++)
	{
		if (m_Callbacks[i].scriptContext == scriptContext)
		{
			m_Callbacks.erase(m_Callbacks.begin() + i);
		}
	}
}

bool CScriptHook::HasContext(CScriptContext* scriptContext)
{
	for (int i = 0; i < m_Callbacks.size(); i++)
	{
		if (m_Callbacks[i].scriptContext == scriptContext)
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