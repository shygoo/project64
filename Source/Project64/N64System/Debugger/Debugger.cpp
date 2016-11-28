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
#include "stdafx.h"
#include "DebuggerUI.h"
#include "ScriptHook.h"

#include <Project64-core/N64System/Interpreter/InterpreterCPU.h>
#include <Project64-core/N64System/Interpreter/InterpreterOps.h>

CPj64Module _Module;

CDebuggerUI::CDebuggerUI () :
    m_MemoryDump(NULL),
    m_MemoryView(NULL),
    m_MemorySearch(NULL),
    m_DebugTLB(NULL),
    m_CommandsView(NULL),
	m_Scripts(NULL),
	m_Symbols(NULL),
	m_Breakpoints(NULL),
	m_ScriptSystem(NULL)
{
	g_Settings->RegisterChangeCB(GameRunning_InReset,this,(CSettings::SettingChangedFunc)GameReset);
	g_Debugger = this;

	m_Breakpoints = new CBreakpoints();
	m_ScriptSystem = new CScriptSystem(this);
}

CDebuggerUI::~CDebuggerUI (void)
{
	g_Settings->UnregisterChangeCB(GameRunning_InReset,this,(CSettings::SettingChangedFunc)GameReset);
    Debug_Reset();
	delete m_MemoryView;
	delete m_CommandsView;
	delete m_Scripts;
	delete m_ScriptSystem;
	delete m_Breakpoints;
	delete m_Symbols;
}

void CDebuggerUI::GameReset ( CDebuggerUI * _this )
{
	if (!g_Settings->LoadBool(GameRunning_InReset))
	{
		return;
	}
	_this->Debug_Reset();
}

void CDebuggerUI::Debug_Reset ( void )
{
    if (m_MemoryDump)
    {
        m_MemoryDump->HideWindow();
        delete m_MemoryDump;
        m_MemoryDump = NULL;
    }
    if (m_MemorySearch)
    {
        m_MemorySearch->HideWindow();
        delete m_MemorySearch;
        m_MemorySearch = NULL;
    }
    if (m_DebugTLB)
    {
        m_DebugTLB->HideWindow();
        delete m_DebugTLB;
        m_DebugTLB = NULL;
    }

	/*
	if (m_MemoryView)
	{
		m_MemoryView->HideWindow();
		delete m_MemoryView;
		m_MemoryView = NULL;
	}
	if (m_CommandsView)
	{
		m_CommandsView->HideWindow();
		delete m_CommandsView;
		m_CommandsView = NULL;
	}
	if (m_Scripts)
	{
		m_Scripts->HideWindow();
		delete m_Scripts;
		m_Scripts = NULL;
	}*/
}

void CDebuggerUI::Debug_ShowMemoryDump()
{
    if (g_MMU == NULL)
    {
        return;
    }
    if (m_MemoryDump == NULL)
    {
        m_MemoryDump = new CDumpMemory(this);
    }
    if (m_MemoryDump)
    {
        m_MemoryDump->ShowWindow();
    }
}

void CDebuggerUI::Debug_ShowMemoryWindow ( void )
{
    if (g_MMU == NULL)
    {
        return;
    }
    if (m_MemoryView == NULL)
    {
        m_MemoryView = new CDebugMemoryView(this);
    }
    if (m_MemoryView)
    {
        m_MemoryView->ShowWindow();
    }
}

void CDebuggerUI::Debug_ShowMemoryLocation ( uint32_t Address, bool VAddr )
{
    Debug_ShowMemoryWindow();
    if (m_MemoryView)
    {
        m_MemoryView->ShowAddress(Address,VAddr);
    }
}

void CDebuggerUI::Debug_ShowTLBWindow (void)
{
    if (g_MMU == NULL)
    {
        return;
    }
    if (m_DebugTLB == NULL)
    {
        m_DebugTLB = new CDebugTlb(this);
    }
    if (m_DebugTLB)
    {
        m_DebugTLB->ShowWindow();
    }
}

void CDebuggerUI::Debug_RefreshTLBWindow(void)
{
    if (m_DebugTLB)
    {
        m_DebugTLB->RefreshTLBWindow();
    }
}

void CDebuggerUI::Debug_ShowMemorySearch()
{
    if (m_MemorySearch == NULL)
    {
        m_MemorySearch = new CDebugMemorySearch(this);
    }
    if (m_MemorySearch)
    {
        m_MemorySearch->ShowWindow();
    }
}

void CDebuggerUI::Debug_ShowCommandsWindow()
{
	if (m_CommandsView == NULL)
	{
		m_CommandsView = new CDebugCommandsView(this);
	}
	m_CommandsView->ShowWindow();
}

void CDebuggerUI::Debug_ShowCommandsLocation(uint32_t address, bool top)
{
	Debug_ShowCommandsWindow();
	if (m_CommandsView)
	{
		m_CommandsView->ShowAddress(address, top);
	}
}

void CDebuggerUI::Debug_ShowScriptsWindow()
{
	if (m_Scripts == NULL)
	{
		m_Scripts = new CDebugScripts(this);
	}
	m_Scripts->ShowWindow();
}

void CDebuggerUI::Debug_RefreshScriptsWindow()
{
	//m_Scripts->RefreshList();
	//m_Scripts->Invalidate(true); // repaint (updates script list colors)
}

void CDebuggerUI::Debug_ShowSymbolsWindow()
{
	if (m_Symbols == NULL)
	{
		m_Symbols = new CDebugSymbols(this);
	}
	m_Symbols->ShowWindow();
}

void CDebuggerUI::Debug_ShowModalAddBreakpoint(void)
{
	if (m_AddBreakpoint == NULL)
	{
		m_AddBreakpoint = new CDebugAddBreakpoint(this);
	}
	m_AddBreakpoint->ShowWindow();
}

CBreakpoints* CDebuggerUI::Breakpoints()
{
	return m_Breakpoints;
}

CScriptSystem* CDebuggerUI::ScriptSystem()
{
	return m_ScriptSystem;
}

CDebugScripts* CDebuggerUI::ScriptConsole()
{
	return m_Scripts;
}

void CDebuggerUI::BreakpointHit()
{
	Debug_ShowCommandsLocation(g_Reg->m_PROGRAM_COUNTER, false);
	m_Breakpoints->Pause();
}

// CDebugger implementation

void CDebuggerUI::TLBChanged()
{
	Debug_RefreshTLBWindow();
}

// Called from the interpreter core at the beginning of every CPU step
// Returns false when the instruction should be skipped
bool CDebuggerUI::CPUStepStarted()
{
	uint32_t PROGRAM_COUNTER = g_Reg->m_PROGRAM_COUNTER;
	uint32_t JumpToLocation = R4300iOp::m_JumpToLocation;
	
	m_ScriptSystem->HookCPUExec()->InvokeByParam(PROGRAM_COUNTER);
	
	// PC breakpoints

	if (m_Breakpoints->EBPExists(PROGRAM_COUNTER))
	{
		BreakpointHit();
		return !m_Breakpoints->isSkipping();
	}

	// Memory breakpoints

	OPCODE Opcode = R4300iOp::m_Opcode;
	uint32_t op = Opcode.op;

	if (op >= R4300i_LDL && op <= R4300i_SD && op != R4300i_CACHE) // Read and write instructions
	{
		uint32_t memoryAddress = g_Reg->m_GPR[Opcode.base].UW[0] + (int16_t)Opcode.offset;

		if ((op <= R4300i_LWU || (op >= R4300i_LL && op <= R4300i_LD))) // Read instructions
		{
			m_ScriptSystem->HookCPURead()->InvokeByParam(memoryAddress);
			
			if (m_Breakpoints->RBPExists(memoryAddress))
			{
				BreakpointHit();
				return !m_Breakpoints->isSkipping();
			}
		}
		else // Write instructions
		{
			m_ScriptSystem->HookCPUWrite()->InvokeByParam(memoryAddress);

			if (m_Breakpoints->WBPExists(memoryAddress))
			{
				BreakpointHit();
				return !m_Breakpoints->isSkipping();
			}
			
			// Catch cart -> rdram dma
			if (memoryAddress == 0xA460000C) // PI_WR_LEN_REG
			{
				uint32_t dmaAddr = g_Reg->PI_DRAM_ADDR_REG | 0x80000000;
				uint32_t dmaLen = g_Reg->m_GPR[Opcode.rt].UW[0] + 1; // Assume u32 for now
				uint32_t endAddr = dmaAddr + dmaLen;

				for (int i = 0; i < m_Breakpoints->m_nWBP; i++)
				{
					uint32_t wbpAddr = m_Breakpoints->m_WBP[i];
					if (wbpAddr >= dmaAddr && wbpAddr < endAddr)
					{
						BreakpointHit();
						//m_CommandsView->ShowWindow();
						//m_CommandsView->ShowPIRegTab();
						//CBreakpoints::Pause();
						return !m_Breakpoints->isSkipping();
					}
				}
			}
		}
	}

	if (!m_Breakpoints->isDebugging())
	{
		return !m_Breakpoints->isSkipping();
	}

	if (R4300iOp::m_NextInstruction != JUMP)
	{
		BreakpointHit();
		return !m_Breakpoints->isSkipping();
	}

	if (JumpToLocation == PROGRAM_COUNTER + 4)
	{
		// Only pause on delay slots when branch isn't taken
		BreakpointHit();
		return !m_Breakpoints->isSkipping();
	}

	return !m_Breakpoints->isSkipping();
}