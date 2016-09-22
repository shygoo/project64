
#include <3rdParty/duktape/duktape.h>
#include <Project64/N64System/Debugger/debugger.h>

int CScriptSystem::m_NextEventId = 0;

duk_context* CScriptSystem::m_Ctx = NULL;

std::vector<SCRIPTEVENT> CScriptSystem::m_ExecEvents;
std::vector<SCRIPTEVENT> CScriptSystem::m_ReadEvents;
std::vector<SCRIPTEVENT> CScriptSystem::m_WriteEvents;

void CScriptSystem::Eval(const char* jsCode) {
	duk_eval_string_noresult(m_Ctx, jsCode);
}

void CScriptSystem::InvokeExecEvents(uint32_t address)
{
	for (int i = 0; i < m_ExecEvents.size(); i++)
	{
		if (m_ExecEvents[i].tag == address)
		{
			InvokeEvent(m_ExecEvents[i].eventId);
		}
	}
}

void CScriptSystem::Init()
{
	m_Ctx = duk_create_heap_default();

	duk_push_c_function(m_Ctx, AddEvent, DUK_VARARGS);
	duk_put_global_string(m_Ctx, "_AddEvent");

	duk_eval_string_noresult(m_Ctx,
		"var R0 =  0, AT =  1, V0 =  2, V1 =  3, A0 =  4, A1 =  5, A2 =  6, A3 =  7,"
		"    T0 =  8, T1 =  9, T2 = 10, T3 = 11, T4 = 12, T5 = 13, T6 = 14, T7 = 15,"
		"    S0 = 16, S1 = 17, S2 = 18, S3 = 19, S4 = 20, S5 = 21, S6 = 22, S7 = 23,"
		"    T8 = 24, T9 = 25, K0 = 26, K1 = 27, GP = 28, SP = 29, FP = 30, RA = 31;"
		"function onexec(addr, fn){"
		"var eventId = _AddEvent('exec', fn, addr);"
		"}"
		"function rmevent(eventId){"
		"}"
	);
}

void CScriptSystem::InvokeEvent(int eventId)
{
	duk_push_global_stash(m_Ctx);
	duk_get_prop_index(m_Ctx, -1, eventId);
	duk_call(m_Ctx, 0);
	duk_pop_n(m_Ctx, 2);
}

duk_ret_t CScriptSystem::AddEvent(duk_context* ctx)
{
	int argc = duk_get_top(m_Ctx);

	if (argc != 3)
	{
		duk_push_number(m_Ctx, -1);
		return 1;
	}

	const char* hook = duk_to_string(m_Ctx, 0); // exec, read, write etc
	void* callback = duk_get_heapptr(m_Ctx, 1); // pointer to js function
	uint32_t tag = duk_to_uint32(m_Ctx, 2); // info associated with entry

											// Protect callback function from garbage collector
	duk_pop_n(m_Ctx, 3);
	duk_push_global_stash(m_Ctx);
	duk_push_heapptr(m_Ctx, callback);
	duk_put_prop_index(m_Ctx, -2, m_NextEventId);
	duk_pop_n(m_Ctx, 1);

	SCRIPTEVENT event = { m_NextEventId, callback, tag };

	// Push event struct to native hook
	if (strcmp("exec", hook) == 0)
	{
		m_ExecEvents.push_back(event);
	}
	else if (strcmp("write", hook) == 0)
	{
		m_WriteEvents.push_back(event);
	}
	else if (strcmp("read", hook) == 0)
	{
		m_ReadEvents.push_back(event);
	}

	// return event id (js)
	duk_push_number(m_Ctx, m_NextEventId);
	m_NextEventId++;
	return 1;
}
/*

int main(void)
{
CScriptSystem::Init();
CScriptSystem::Eval("onexec(0x802CB1C0, function(){print(123)})");
CScriptSystem::InvokeExecEvents(0x802CB1C0);
}
*/