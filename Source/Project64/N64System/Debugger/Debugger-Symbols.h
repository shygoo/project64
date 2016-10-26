/****************************************************************************
*                                                                           *
* Project64 - A Nintendo 64 emulator.                                       *
* http://www.pj64-emu.com/                                                  *
* Copyright (C) 2012 Project64. All rights reserved.                        *
*                                                                           *
* License:                                                                  *
* GNU/GPLv2 http://www.gnu.org/licenses/gpl-2.0.html                        *
*                                                                           *
****************************************************************************/

#pragma once
#include "DebuggerUI.h"

typedef struct {
	int type;
	char* type_str;
	uint32_t address;
	char* address_str;
	char* name;
	char* description;
	int id;
} SYMBOLENTRY;

// todo maybe add char* ownerName and use a TreeView

typedef struct {
	const char* typeName;
	int typeVal;
} SYMBOLTYPE;



class CDebugSymbols : public CDebugDialog<CDebugSymbols>
{
private:
	typedef enum {
		ERR_SUCCESS,
		ERR_INVALID_TYPE,
		ERR_INVALID_ADDR,
		ERR_INVALID_NAME,
		ERR_MISSING_FIELDS,
	} ParseError;

	static constexpr char* symbolTypes[] = {
		"code",
		"data",
		"u8",
		"u16",
		"u32",
		"u64",
		"s8",
		"s16",
		"s32",
		"s64",
		"float",
		"double",
		NULL
	};

	static int GetTypeNumber(const char* typeName);

public:
	enum { IDD = IDD_Debugger_Symbols };

	CDebugSymbols(CDebuggerUI * debugger);
	//virtual ~CDebugScripts(void);

	CListViewCtrl m_SymbolsListView;

	CFile m_SymFileHandle;
	char* m_SymFileBuffer;
	
	vector<SYMBOLENTRY> m_Symbols;
	int m_NextSymbolId;

	void LoadGameSymbols();
	void ParseErrorAlert(char* message, int lineNumber);

	SYMBOLENTRY* GetSymbolEntryById(int id);

	LRESULT OnDestroy(void)
	{
		return 0;
	}

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClicked(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	BEGIN_MSG_MAP_EX(CDebugSymbols)
		COMMAND_CODE_HANDLER(BN_CLICKED, OnClicked)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MSG_WM_DESTROY(OnDestroy)
	END_MSG_MAP()
};