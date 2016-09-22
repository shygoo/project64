#pragma once

#include "stdafx.h"
#include <3rdParty/duktape/duktape.h>

typedef struct {
	int eventId;
	void* callback;
	uint32_t tag;
} SCRIPTEVENT;

typedef struct {
	const char* hook;
	std::vector<SCRIPTEVENT>* events;
} SCRIPTHOOK;

class CScriptSystem {
private:
	CScriptSystem();

public:
	static int m_NextEventId;

	static duk_context* m_Ctx;

	static std::vector<SCRIPTEVENT> m_ExecEvents;
	static std::vector<SCRIPTEVENT> m_ReadEvents;
	static std::vector<SCRIPTEVENT> m_WriteEvents;

	static std::vector<SCRIPTHOOK> m_ScriptHooks;

	static void RegisterHook(const char* hook, std::vector<SCRIPTEVENT>* events);

	static bool GetEvents(const char* hook, std::vector<SCRIPTEVENT>** events);

	static void Init();

	static void Eval(const char* jsCode);
	static void EvalFile(const char* jsPath);

	static void InvokeEvent(int eventId);
	static void InvokeExecEvents(uint32_t address);
	static void InvokeReadEvents(uint32_t address);
	static void InvokeWriteEvents(uint32_t address);

	static void RegisterGlobalFunction(const char* name, duk_c_function func);

private:
	// called from scripts
	static duk_ret_t AddEvent(duk_context* ctx);
	static duk_ret_t GetGPRVal(duk_context* ctx);
	static duk_ret_t SetGPRVal(duk_context* ctx);
	static duk_ret_t GetRDRAMU8(duk_context* ctx);
	static duk_ret_t SetRDRAMU8(duk_context* ctx);
};