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

class CInterpreterDebug {
private:
	CInterpreterDebug();

public:
	static BOOL m_Debugging;
	static BOOL m_Skipping;
	static std::vector<uint32_t> m_RBP;
	static std::vector<uint32_t> m_WBP;
	static std::vector<uint32_t> m_EBP;

	static int m_nRBP;
	static int m_nWBP;
	static int m_nEBP;

	static void Pause();
	static void Resume();
	static void Skip();

	static BOOL isDebugging();
	static void KeepDebugging();
	static void StopDebugging();

	static void RBPAdd(uint32_t address);
	static void RBPRemove(uint32_t address);
	static void RBPClear();

	static void WBPAdd(uint32_t address);
	static void WBPRemove(uint32_t address);
	static void WBPClear();

	static void EBPAdd(uint32_t address);
	static void EBPRemove(uint32_t address);
	static void EBPClear();

	static void BPClear();

	// inlines

	static inline BOOL RBPExists(uint32_t address)
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

	static inline BOOL WBPExists(uint32_t address)
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

	static inline BOOL EBPExists(uint32_t address)
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

};