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
#include "InterpreterDBG.h"

#include <Project64-core/N64System/Mips/RegisterClass.h>
#include <Project64-core/N64System/SystemGlobals.h>
#include <Project64-core/N64System/Mips/OpcodeName.h>

CDebuggerUI* CInterpreterDBG::m_DebuggerUI = NULL;

BOOL CInterpreterDBG::m_Paused = FALSE;
BOOL CInterpreterDBG::m_Debugging = FALSE;

vector<uint32_t> CInterpreterDBG::m_RBP;
vector<uint32_t> CInterpreterDBG::m_WBP;
vector<uint32_t> CInterpreterDBG::m_EBP;

int CInterpreterDBG::m_nRBP = 0;
int CInterpreterDBG::m_nWBP = 0;
int CInterpreterDBG::m_nEBP = 0;

void CInterpreterDBG::DbgInit(CDebuggerUI* debuggerUI)
{
	m_DebuggerUI = debuggerUI;
}

void CInterpreterDBG::PauseHere(uint32_t address)
{
	char notification[64];
	sprintf(notification, "%s (%08X)", g_Lang->GetString(MSG_CPU_PAUSED).c_str(), address);
	g_Notify->DisplayMessage(5, notification);
	
	m_DebuggerUI->Debug_ShowCommandsLocation(address, FALSE);
	
	m_Paused = TRUE;
	
	// block the calling thread
	while (m_Paused)
	{
		Sleep(20);
	}
}

void CInterpreterDBG::KeepDebugging()
{
	m_Debugging = TRUE;
}

void CInterpreterDBG::Resume()
{
	g_Notify->DisplayMessage(5, MSG_CPU_RESUMED);
	m_Paused = FALSE;
}

BOOL CInterpreterDBG::RBPExists(uint32_t address)
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

BOOL CInterpreterDBG::WBPExists(uint32_t address)
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

BOOL CInterpreterDBG::EBPExists(uint32_t address)
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

void CInterpreterDBG::RBPAdd(uint32_t address)
{
	m_RBP.push_back(address);
	m_nRBP = m_RBP.size();
}

void CInterpreterDBG::WBPAdd(uint32_t address)
{
	m_WBP.push_back(address);
	m_nWBP = m_WBP.size();
}

void CInterpreterDBG::EBPAdd(uint32_t address)
{
	m_EBP.push_back(address);
	m_nEBP = m_EBP.size();
}

void CInterpreterDBG::RBPRemove(uint32_t address)
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

void CInterpreterDBG::WBPRemove(uint32_t address)
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

void CInterpreterDBG::EBPRemove(uint32_t address)
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

void CInterpreterDBG::RBPClear()
{
	m_RBP.clear();
	m_nRBP = 0;
}

void CInterpreterDBG::WBPClear()
{
	m_WBP.clear();
	m_nWBP = 0;
}

void CInterpreterDBG::EBPClear()
{
	m_EBP.clear();
	m_nEBP = 0;
}

void CInterpreterDBG::BPClear()
{
	RBPClear();
	WBPClear();
	EBPClear();
}
