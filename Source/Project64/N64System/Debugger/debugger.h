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

class CDumpMemory;
class CDebugMemoryView;
class CDebugMemorySearch;
class CDebugTlb;
class CDebugCommandsView;
class CDebugScripts;

__interface CDebugger
{
    virtual void TLBChanged ( void ) = 0;
    virtual void BreakpointHit ( void ) = 0;
	virtual void CPUStepStarted ( void ) = 0;
};

class CDebuggerUI :
	public CDebugger
{
    CDumpMemory        * m_MemoryDump;
    CDebugMemoryView   * m_MemoryView;
    CDebugMemorySearch * m_MemorySearch;
    CDebugTlb          * m_DebugTLB;
	CDebugCommandsView * m_CommandsView;
	CDebugScripts      * m_Scripts;

protected:
    CDebuggerUI();
    virtual ~CDebuggerUI();

	void TLBChanged ( void );
	void BreakpointHit ( void );
	void CPUStepStarted ( void );

public:
    void Debug_Reset                ( void );
    void Debug_ShowMemoryDump       ( void );
    void Debug_ShowMemoryWindow     ( void );
    void Debug_ShowMemoryLocation   ( uint32_t Address, bool VAddr );
    void Debug_ShowMemorySearch     ( void );
    void Debug_ShowTLBWindow        ( void );
    void Debug_RefreshTLBWindow     ( void );
    void Debug_ShowCommandsWindow   ( void );
    void Debug_ShowCommandsLocation ( uint32_t address, bool top );
	void Debug_ShowScriptsWindow    ( void );
    
    static void GameReset ( CDebuggerUI * _this );
};
