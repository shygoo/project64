#pragma once

#include <stdafx.h>

class CScriptInstance;

class CScriptHook
{
private:
	typedef struct {
		CScriptInstance* scriptInstance;
		void* heapptr;
		uint32_t param;
		int callbackId;
		bool bOnce;
	} JSCALLBACK;

	int m_NextCallbackId;
	vector<JSCALLBACK> m_Callbacks;

public:
	CScriptHook();
	~CScriptHook();
	int Add(CScriptInstance* scriptInstance, void* heapptr, uint32_t tag = 0, bool bOnce = false);
	void InvokeAll();
	void InvokeById(int callbackId);
	void InvokeByParam(uint32_t tag);
	void RemoveById(int callbackId);
	void RemoveByParam(uint32_t tag);
	void RemoveByContext(CScriptInstance* scriptInstance);
	bool HasContext(CScriptInstance* scriptInstance);
	//bool HasTag(uint32_t tag);
};