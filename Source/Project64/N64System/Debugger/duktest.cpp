#include "stdafx.h"
#include <Project64/N64System/Debugger/duktest.h>
#include <3rdParty/duktape/duktape.h>

duk_ret_t _alert(duk_context* ctx)
{
	int argc = duk_get_top(ctx);
	const char* msg = duk_to_string(ctx, 0);
	MessageBox(NULL, msg, "", MB_OK);
	duk_pop_n(ctx, 1);
	duk_push_boolean(ctx, 1);
	return 1;
}

int CScriptSystem::m_NextEventId = 0;

duk_context* CScriptSystem::m_Ctx = NULL;

std::vector<SCRIPTEVENT> CScriptSystem::m_ExecEvents;
std::vector<SCRIPTEVENT> CScriptSystem::m_ReadEvents;
std::vector<SCRIPTEVENT> CScriptSystem::m_WriteEvents;

std::vector<SCRIPTHOOK> CScriptSystem::m_ScriptHooks;

void CScriptSystem::Eval(const char* jsCode)
{
	duk_eval_string_noresult(m_Ctx, jsCode);
}

void CScriptSystem::EvalFile(const char* jsPath)
{
	duk_eval_file_noresult(m_Ctx, jsPath);
}

void CScriptSystem::InvokeExecEvents(uint32_t address)
{
	for (uint32_t i = 0; i < m_ExecEvents.size(); i++)
	{
		if (m_ExecEvents[i].tag == address)
		{
			InvokeEvent(m_ExecEvents[i].eventId);
		}
	}
}

void CScriptSystem::InvokeReadEvents(uint32_t address)
{
	for (uint32_t i = 0; i < m_ReadEvents.size(); i++)
	{
		if (m_ReadEvents[i].tag == address)
		{
			InvokeEvent(m_ReadEvents[i].eventId);
		}
	}
}

void CScriptSystem::InvokeWriteEvents(uint32_t address)
{
	for (uint32_t i = 0; i < m_WriteEvents.size(); i++)
	{
		if (m_WriteEvents[i].tag == address)
		{
			InvokeEvent(m_WriteEvents[i].eventId);
		}
	}
}

void CScriptSystem::Init()
{
	RegisterHook("exec", &m_ExecEvents);
	RegisterHook("read", &m_ReadEvents);
	RegisterHook("write", &m_WriteEvents);

	m_Ctx = duk_create_heap_default();

	BindGlobalFunction("_AddEvent", AddEvent);
	BindGlobalFunction("_SetGPRVal", SetGPRVal);
	BindGlobalFunction("_GetGPRVal", GetGPRVal);
	BindGlobalFunction("_GetRDRAMU8", GetRDRAMU8);
	BindGlobalFunction("_SetRDRAMU8", SetRDRAMU8);
	BindGlobalFunction("alert", _alert);

	EvalFile("_api.js");
}

void CScriptSystem::BindGlobalFunction(const char* name, duk_c_function func) {
	duk_push_c_function(m_Ctx, func, DUK_VARARGS);
	duk_put_global_string(m_Ctx, name);
}

void CScriptSystem::InvokeEvent(int eventId)
{
	duk_push_global_stash(m_Ctx);
	duk_get_prop_index(m_Ctx, -1, eventId);
	duk_call(m_Ctx, 0);
	duk_pop_n(m_Ctx, 2);
}

void CScriptSystem::RegisterHook(const char* hook, std::vector<SCRIPTEVENT>* events)
{
	SCRIPTHOOK scriptHook = { hook, events };
	m_ScriptHooks.push_back(scriptHook);
}

bool CScriptSystem::GetEvents(const char* hook, std::vector<SCRIPTEVENT>** events)
{
	for (uint32_t i = 0; i < m_ScriptHooks.size(); i++)
	{
		if (strcmp(m_ScriptHooks[i].hook, hook) == 0)
		{
			*events = m_ScriptHooks[i].events;
			return true;
		}
	}
	return false;
}

duk_ret_t CScriptSystem::AddEvent(duk_context* ctx)
{
	int argc = duk_get_top(ctx);

	if (argc != 3)
	{
		duk_push_boolean(ctx, false);
		return 1;
	}

	const char* hook = duk_to_string(ctx, 0); // exec, read, write etc
	void* callback = duk_get_heapptr(ctx, 1); // pointer to js function
	uint32_t tag = duk_to_uint32(ctx, 2); // info associated with entry
	duk_pop_n(ctx, 3);

	// Protect callback function from garbage collector
	duk_push_global_stash(ctx);
	duk_push_heapptr(ctx, callback);
	duk_put_prop_index(ctx, -2, m_NextEventId);
	duk_pop_n(ctx, 1);

	SCRIPTEVENT scriptEvent = { m_NextEventId, callback, tag };

	// Get events list associated with hook and push event to it

	std::vector<SCRIPTEVENT>* events;
	if (GetEvents(hook, &events))
	{
		// Register event and return id
		events->push_back(scriptEvent);
		duk_push_number(ctx, m_NextEventId);
		m_NextEventId++;
		return 1;
	}
	
	duk_push_boolean(ctx, false);
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
	g_MMU->LB_VAddr(address, val);
	duk_push_int(ctx, val);
	return 1;
}

duk_ret_t CScriptSystem::SetRDRAMU8(duk_context* ctx)
{
	uint32_t address = duk_to_uint32(ctx, 0);
	uint8_t val = duk_to_int(ctx, 1);
	duk_pop_n(ctx, 2);
	g_MMU->SB_VAddr(address, val);
	duk_push_int(ctx, val);
	return 1;
}