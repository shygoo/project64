#pragma once

#include <stdafx.h>

class CScriptContext;

class CScriptHook
{
private:
	typedef struct {
		CScriptContext* scriptContext;
		void* heapptr;
		uint32_t tag;
		int callbackId;
	} JSCALLBACK;

	int m_NextCallbackId;
	vector<JSCALLBACK> m_Callbacks;

public:
	CScriptHook();
	~CScriptHook();
	int Add(CScriptContext* scriptContext, void* heapptr, uint32_t tag = 0);
	void InvokeAll();
	void InvokeById(int callbackId);
	void InvokeByTag(uint32_t tag);
	void RemoveById(int callbackId);
	void RemoveByTag(uint32_t tag);
	void RemoveByContext(CScriptContext* scriptContext);
	bool HasContext(CScriptContext* scriptContext);
	//bool HasTag(uint32_t tag);
};