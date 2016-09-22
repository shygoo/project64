#pragma once

#include "stdafx.h"

#include <3rdParty/duktape/duk_config.h>
#include <3rdParty/duktape/duktape.h>


typedef struct {
	int eventId;
	void* callback;
	uint32_t tag;
} SCRIPTEVENT;

class CScriptSystem {
private:
	CScriptSystem();

public:
	static int m_NextEventId;

	static duk_context* m_Ctx;

	static std::vector<SCRIPTEVENT> m_ExecEvents;
	static std::vector<SCRIPTEVENT> m_ReadEvents;
	static std::vector<SCRIPTEVENT> m_WriteEvents;

	static void Init();

	static void Eval(const char* jsCode);

	static void InvokeEvent(int eventId);
	static void InvokeExecEvents(uint32_t address);

	// called from scripts
	static duk_ret_t AddEvent(duk_context* ctx);
	static duk_ret_t GPR(duk_context* ctx);
};