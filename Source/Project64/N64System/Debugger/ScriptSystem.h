#pragma once

#include "stdafx.h"
#include <3rdParty/duktape/duktape.h>

typedef struct {
	int      callbackId;
	void*    heapptr;
	uint32_t tag;
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

class CScriptSystem {
private:
	CScriptSystem();

public:
	static int m_NextStashIndex;
	static duk_context* m_Ctx;
	static vector<EVENTHOOK> m_Hooks;
	static CCallbackList m_ExecEvents;
	static CCallbackList m_ReadEvents;
	static CCallbackList m_WriteEvents;
	static CCallbackList m_WMEvents;

	static void Init();
	static void RegisterCallbackList(const char* hookId, CCallbackList* cbList);
	static CCallbackList* GetCallbackList(const char* hookId);
	static void Eval(const char* jsCode);
	static void EvalFile(const char* jsPath);
	static void BindGlobalFunction(const char* name, duk_c_function func);
	static void Stash(void* heapptr);
	static void Unstash(void* heapptr);

	static void Invoke(void* heapptr);

private:
	// Bound functions

	static duk_ret_t AddCallback(duk_context* ctx); // ()
	static duk_ret_t GetGPRVal(duk_context* ctx);
	static duk_ret_t SetGPRVal(duk_context* ctx);
	static duk_ret_t GetRDRAMU8(duk_context* ctx);
	static duk_ret_t GetRDRAMU16(duk_context* ctx);
	static duk_ret_t GetRDRAMU32(duk_context* ctx);
	static duk_ret_t SetRDRAMU8(duk_context* ctx);
	static duk_ret_t SetRDRAMU16(duk_context* ctx);
	static duk_ret_t SetRDRAMU32(duk_context* ctx);
	static duk_ret_t Thread(duk_context* ctx);
	static duk_ret_t alert(duk_context* ctx);
};