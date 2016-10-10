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

#include <Project64-core/N64System/Interpreter/InterpreterCPU.h>
#include <Project64-core/N64System/Interpreter/InterpreterOps.h>

CPj64Module _Module;

CDebuggerUI::CDebuggerUI () :
    m_MemoryDump(NULL),
    m_MemoryView(NULL),
    m_MemorySearch(NULL),
    m_DebugTLB(NULL),
    m_CommandsView(NULL),
	m_Scripts(NULL)
{
	g_Settings->RegisterChangeCB(GameRunning_InReset,this,(CSettings::SettingChangedFunc)GameReset);
	g_Debugger = this;
}

CDebuggerUI::~CDebuggerUI (void)
{
	g_Settings->UnregisterChangeCB(GameRunning_InReset,this,(CSettings::SettingChangedFunc)GameReset);
    Debug_Reset();
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
    if (m_MemoryView)
    {
        m_MemoryView->HideWindow();
        delete m_MemoryView;
        m_MemoryView = NULL;
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
	/*if (m_CommandsView)
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

void CDebuggerUI::TLBChanged()
{
	Debug_RefreshTLBWindow();
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

void CDebuggerUI::BreakpointHit()
{
	if (m_CommandsView)
	{
		Debug_ShowCommandsLocation(g_Reg->m_PROGRAM_COUNTER, false);
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

void CDebuggerUI::CPUStepStarted()
{
	uint32_t PROGRAM_COUNTER = g_Reg->m_PROGRAM_COUNTER;
	uint32_t JumpToLocation = R4300iOp::m_JumpToLocation;

	CScriptSystem::m_ExecEvents.InvokeByTag(PROGRAM_COUNTER);

	// PC breakpoints

	if (CInterpreterDebug::EBPExists(PROGRAM_COUNTER))
	{
		CInterpreterDebug::Pause();
		return;
	}

	// Memory breakpoints

	OPCODE Opcode = R4300iOp::m_Opcode;
	uint32_t op = Opcode.op;

	if (op >= R4300i_LDL && op <= R4300i_SD && op != R4300i_CACHE) // Read and write instructions
	{
		uint32_t memoryAddress = g_Reg->m_GPR[Opcode.base].UW[0] + (int16_t)Opcode.offset;

		if ((op <= R4300i_LWU || (op >= R4300i_LL && op <= R4300i_LD))) // Read instructions
		{
			CScriptSystem::m_ReadEvents.InvokeByTag(memoryAddress);

			if (CInterpreterDebug::RBPExists(memoryAddress))
			{
				CInterpreterDebug::Pause();
				return;
			}
		}
		else // Write instructions
		{
			CScriptSystem::m_WriteEvents.InvokeByTag(memoryAddress);

			if (CInterpreterDebug::WBPExists(memoryAddress))
			{
				CInterpreterDebug::Pause();
				return;
			}
		}
	}

	if (!CInterpreterDebug::isDebugging())
	{
		return;
	}

	if (R4300iOp::m_NextInstruction != JUMP)
	{
		CInterpreterDebug::Pause();
		return;
	}

	if (JumpToLocation == PROGRAM_COUNTER + 4)
	{
		// Only pause on delay slots when branch isn't taken
		CInterpreterDebug::Pause();
		return;
	}
}