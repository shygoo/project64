/****************************************************************************
*                                                                           *
* Project64 - A Nintendo 64 emulator.                                      *
* http://www.pj64-emu.com/                                                  *
* Copyright (C) 2012 Project64. All rights reserved.                        *
*                                                                           *
* License:                                                                  *
* GNU/GPLv2 http://www.gnu.org/licenses/gpl-2.0.html                        *
*                                                                           *
****************************************************************************/

#pragma once

#include <stdafx.h>
#include <3rdParty/duktape/duktape.h>

class CScriptHook;
class CScriptContext;

class CScriptSystem
{
public:
	CScriptSystem(CDebuggerUI* debugger);
	~CScriptSystem();
	// Run a script in its own context/thread
	void RunScript(char* path);

	// Kill a script context/thread by its path
	void StopScript(char* path);

private:
	typedef struct {
		const char* hookId;
		CScriptHook* cbList;
	} HOOKENTRY;

	vector<HOOKENTRY> m_Hooks;

	CScriptHook* m_HookCPUExec;
	CScriptHook* m_HookCPURead;
	CScriptHook* m_HookCPUWrite;

	void RegisterHook(const char* hookId, CScriptHook* cbList); // associate string id with callback list
	void UnregisterHooks();

public:
	// Returns true if any of the script hooks have callbacks for scriptContext

	bool HaveCallbacksForContext(CScriptContext* scriptContext);

	CScriptHook* GetHook(const char* hookId);
	
	inline CScriptHook* HookCPUExec()
	{
		return m_HookCPUExec;
	}

	inline CScriptHook* HookCPURead()
	{
		return m_HookCPURead;
	}

	inline CScriptHook* HookCPUWrite()
	{
		return m_HookCPUWrite;
	}
};