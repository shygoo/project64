#pragma once
// netdbg

class CDebugCommandsView : public CDebugDialog < CDebugCommandsView >
{
public:
	enum { IDD = IDD_Debugger_Commands };

	CDebugCommandsView(CDebuggerUI * debugger);
	virtual ~CDebugCommandsView(void);

	void ShowAddress(DWORD address, BOOL top);

	CListViewCtrl m_cmdList;
	CTabCtrl m_regTabs;

	//void SetHighlightedItem(int nItem);
	
private:
	static const int listLength;

	DWORD m_Address; // pc
	DWORD m_StartAddress;
	int m_ListHighlightedItem;

	LRESULT	OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT	OnClicked(WORD wNotifyCode, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled);
	LRESULT OnAddrChanged(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	//void                OnVScroll(int request, short Pos, HWND ctrl);
	LRESULT OnDestroy(void);
	LRESULT OnCustomDrawList(NMHDR* pNMHDR);

	BEGIN_MSG_MAP_EX(CDebugCommandsView)
		COMMAND_HANDLER(IDC_CMD_ADDR, EN_CHANGE, OnAddrChanged) // address input change
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_CODE_HANDLER(BN_CLICKED, OnClicked)
		NOTIFY_HANDLER_EX(IDC_CMD_LIST, NM_CUSTOMDRAW, OnCustomDrawList)
		MSG_WM_DESTROY(OnDestroy)
	END_MSG_MAP()
};

