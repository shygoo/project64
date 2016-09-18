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

#include <stdint.h>
#include <Project64/N64System/Debugger/DebuggerUI.h>

class CInterpreterDebug {
private:
	CInterpreterDebug();
	static CDebuggerUI* m_DebuggerUI;
	
public:
	static BOOL m_Paused;
	static BOOL m_Debugging;
	static vector<uint32_t> m_RBP;
	static vector<uint32_t> m_WBP;
	static vector<uint32_t> m_EBP;

	static int m_nRBP;
	static int m_nWBP;
	static int m_nEBP;

	static void Pause(uint32_t address);
	static void Resume();

	static void DbgInit(CDebuggerUI* debuggerUI);
	static BOOL isDebugging();
	static void KeepDebugging();
	static void StopDebugging();

	static BOOL RBPExists(uint32_t address);
	static void RBPAdd(uint32_t address);
	static void RBPRemove(uint32_t address);
	static void RBPClear();

	static BOOL WBPExists(uint32_t address);
	static void WBPAdd(uint32_t address);
	static void WBPRemove(uint32_t address);
	static void WBPClear();

	static BOOL EBPExists(uint32_t address);
	static void EBPAdd(uint32_t address);
	static void EBPRemove(uint32_t address);
	static void EBPClear();

	static void BPClear();
};