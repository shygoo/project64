#pragma once
// netdbg

class CListViewCtrlDbg :
	public CWindowImpl<CListViewCtrlDbg, CListViewCtrl>,
	public CCustomDraw<CListViewCtrlDbg>
{

public:
	void SetHighlightedItem(int nItem);

	DWORD OnPrePaint(int idCtrl, LPNMCUSTOMDRAW lpNMCustomDraw);
	DWORD OnItemPrePaint(int idCtrl, LPNMCUSTOMDRAW lpNMCustomDraw);

private:
	BEGIN_MSG_MAP(CListViewCtrlDbg)
		CHAIN_MSG_MAP(CCustomDraw<CListViewCtrlDbg>)
	END_MSG_MAP()

	int m_highlightedItem;
};

//////////

class CDebugCommandsView :
	public CDebugDialog < CDebugCommandsView >
{
public:
	enum { IDD = IDD_Debugger_Commands };

	CDebugCommandsView(CDebuggerUI * debugger);
	virtual ~CDebugCommandsView(void);

	void ShowAddress(DWORD address);

	CListViewCtrlDbg m_cmdList;
	CTabCtrl m_regTabs;

private:
	BEGIN_MSG_MAP_EX(CDebugCommandsView)
		//CHAIN_MSG_MAP_MEMBER(m_cmdList)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_CODE_HANDLER(BN_CLICKED, OnClicked)
		MSG_WM_DESTROY(OnDestroy)
		// MSG_WM_VSCROLL(OnVScroll)
	END_MSG_MAP()

	LRESULT				OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT				OnClicked(WORD wNotifyCode, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled);
	//void				OnAddrChanged(UINT Code, int id, HWND ctl);
	//void                OnVScroll(int request, short Pos, HWND ctrl);
	LRESULT             OnDestroy(void);

public:

};

