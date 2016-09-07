#pragma once
// netdbg

class CDebugCommandsView :
	public CDebugDialog < CDebugCommandsView >
{
public:
	enum { IDD = IDD_Debugger_Commands };

	CDebugCommandsView(CDebuggerUI * debugger);
	virtual ~CDebugCommandsView(void);

	void ShowAddress(DWORD Address, bool VAddr);

private:
	BEGIN_MSG_MAP_EX(CDebugCommandsView)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_CODE_HANDLER(BN_CLICKED, OnClicked)
		// COMMAND_HANDLER_EX(IDC_ADDR_EDIT, EN_CHANGE, OnAddrChanged)
		// NOTIFY_HANDLER_EX(IDC_MEM_DETAILS, LCN_MODIFIED, OnMemoryModified)
		MSG_WM_DESTROY(OnDestroy)
		// MSG_WM_VSCROLL(OnVScroll)
		END_MSG_MAP()

	LRESULT				OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT				OnClicked(WORD wNotifyCode, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled);
	//void				OnAddrChanged(UINT Code, int id, HWND ctl);
	//void                OnVScroll(int request, short Pos, HWND ctrl);
	//LRESULT             OnMemoryModified(LPNMHDR lpNMHDR);
	LRESULT             OnDestroy(void);

	//void Insert_MemoryLineDump(int LineNumber);
	//void RefreshMemory(bool ResetCompare);

	//enum { MemoryToDisplay = 0x100 };

	//CEditNumber   m_MemAddr;
	//CListCtrl   * m_MemoryList;
	//
	//DWORD         m_DataStartLoc;
	//bool          m_DataVAddrr;
	//BYTE          m_CurrentData[MemoryToDisplay];
	//bool          m_DataValid[MemoryToDisplay];
	//
	//DWORD         m_CompareStartLoc;
	//bool          m_CompareVAddrr;
	//BYTE          m_CompareData[MemoryToDisplay];
	//bool          m_CompareValid[MemoryToDisplay];
public:
	/*
	BEGIN_MSG_MAP(CDebugCommandsView)
		NOTIFY_HANDLER(IDC_COMMANDS_LIST, LVN_ITEMCHANGED, OnLvnItemchangedCommandsList)
	END_MSG_MAP()
	LRESULT OnLvnItemchangedCommandsList(int /*idCtrl, LPNMHDR pNMHDR, BOOL& /*bHandled);*/


};