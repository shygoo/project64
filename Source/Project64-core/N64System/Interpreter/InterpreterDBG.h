#pragma once

#include <stdint.h>
#include <Project64/N64System/Debugger/DebuggerUI.h>

class CInterpreterDBG {
private:
	CInterpreterDBG();
	static CDebuggerUI* m_DebuggerUI;
	
public:
	static BOOL m_Paused;
	static BOOL m_Debugging;
	static vector<DWORD> m_RBP;
	static vector<DWORD> m_WBP;
	static vector<DWORD> m_EBP;

	static int m_nRBP;
	static int m_nWBP;
	static int m_nEBP;

	static void DbgInit(CDebuggerUI* debuggerUI);
	static void PauseHere(uint32_t address);
	static void KeepDebugging();
	static void Resume();

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