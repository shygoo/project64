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
#include "InterpreterDebug.h"

#include <Project64-core/N64System/Mips/RegisterClass.h>
#include <Project64-core/N64System/SystemGlobals.h>
#include <Project64-core/N64System/Mips/OpcodeName.h>

CDebuggerUI* CInterpreterDebug::m_DebuggerUI = NULL;

BOOL CInterpreterDebug::m_Debugging = FALSE;
BOOL CInterpreterDebug::m_Skipping = FALSE;

vector<uint32_t> CInterpreterDebug::m_RBP;
vector<uint32_t> CInterpreterDebug::m_WBP;
vector<uint32_t> CInterpreterDebug::m_EBP;

int CInterpreterDebug::m_nRBP = 0;
int CInterpreterDebug::m_nWBP = 0;
int CInterpreterDebug::m_nEBP = 0;

void CInterpreterDebug::DbgInit(CDebuggerUI* debuggerUI)
{
	m_DebuggerUI = debuggerUI;
}

void CInterpreterDebug::Pause(uint32_t address)
{
	KeepDebugging();
	m_DebuggerUI->Debug_ShowCommandsLocation(address, FALSE);
	g_System->Pause();
}

void CInterpreterDebug::Resume()
{
	g_System->ExternalEvent(SysEvent_ResumeCPU_FromMenu);
}

BOOL CInterpreterDebug::isDebugging()
{
	return m_Debugging;
}

void CInterpreterDebug::KeepDebugging()
{
	m_Debugging = TRUE;
}

void CInterpreterDebug::StopDebugging()
{
	m_Debugging = FALSE;
}

void CInterpreterDebug::Skip()
{
	m_Skipping = TRUE;
}

BOOL CInterpreterDebug::RBPExists(uint32_t address)
{
	for (int i = 0; i < m_nRBP; i++)
	{
		if (m_RBP[i] == address)
		{
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CInterpreterDebug::WBPExists(uint32_t address)
{
	for (int i = 0; i < m_nWBP; i++)
	{
		if (m_WBP[i] == address)
		{
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CInterpreterDebug::EBPExists(uint32_t address)
{
	for (int i = 0; i < m_nEBP; i++)
	{
		if (m_EBP[i] == address)
		{
			return TRUE;
		}
	}
	return FALSE;
}

void CInterpreterDebug::RBPAdd(uint32_t address)
{
	m_RBP.push_back(address);
	m_nRBP = m_RBP.size();
}

void CInterpreterDebug::WBPAdd(uint32_t address)
{
	m_WBP.push_back(address);
	m_nWBP = m_WBP.size();
}

void CInterpreterDebug::EBPAdd(uint32_t address)
{
	m_EBP.push_back(address);
	m_nEBP = m_EBP.size();
}

void CInterpreterDebug::RBPRemove(uint32_t address)
{
	for (int i = 0; i < m_nRBP; i++)
	{
		if (m_RBP[i] == address)
		{
			m_RBP.erase(m_RBP.begin() + i);
			m_nRBP = m_RBP.size();
			return;
		}
	}
}

void CInterpreterDebug::WBPRemove(uint32_t address)
{
	for (int i = 0; i < m_nWBP; i++)
	{
		if (m_WBP[i] == address)
		{
			m_WBP.erase(m_WBP.begin() + i);
			m_nWBP = m_WBP.size();
			return;
		}
	}
}

void CInterpreterDebug::EBPRemove(uint32_t address)
{
	for (int i = 0; i < m_nEBP; i++)
	{
		if (m_EBP[i] == address)
		{
			m_EBP.erase(m_EBP.begin() + i);
			m_nEBP = m_EBP.size();
			return;
		}
	}
}

void CInterpreterDebug::RBPClear()
{
	m_RBP.clear();
	m_nRBP = 0;
}

void CInterpreterDebug::WBPClear()
{
	m_WBP.clear();
	m_nWBP = 0;
}

void CInterpreterDebug::EBPClear()
{
	m_EBP.clear();
	m_nEBP = 0;
}

void CInterpreterDebug::BPClear()
{
	RBPClear();
	WBPClear();
	EBPClear();
}
