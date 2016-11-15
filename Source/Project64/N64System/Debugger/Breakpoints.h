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

class CBreakpoints {
private:
	

public:
	CBreakpoints();

	BOOL m_Debugging;
	BOOL m_Skipping;
	std::vector<uint32_t> m_RBP;
	std::vector<uint32_t> m_WBP;
	std::vector<uint32_t> m_EBP;

	int m_nRBP;
	int m_nWBP;
	int m_nEBP;

	void Pause();
	void Resume();
	void Skip();

	BOOL isDebugging();
	void KeepDebugging();
	void StopDebugging();
	inline BOOL isSkipping()
	{
		BOOL ret = m_Skipping;
		m_Skipping = FALSE;
		return ret;
	}

	bool RBPAdd(uint32_t address);
	void RBPRemove(uint32_t address);
	void RBPToggle(uint32_t address);
	void RBPClear();

	bool WBPAdd(uint32_t address);
	void WBPRemove(uint32_t address);
	void WBPToggle(uint32_t address);
	void WBPClear();

	bool EBPAdd(uint32_t address);
	void EBPRemove(uint32_t address);
	void EBPToggle(uint32_t address);
	void EBPClear();
	
	void BPClear();

	// inlines

	inline BOOL RBPExists(uint32_t address)
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

	inline BOOL WBPExists(uint32_t address)
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

	inline BOOL EBPExists(uint32_t address)
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