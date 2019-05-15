#pragma once

#include <stdafx.h>
#include "DebuggerUI.h"
#include "Util/DisplayListParser.h"

class CDebugDisplayList :
	public CDebugDialog<CDebugDisplayList>,
	public CDialogResize<CDebugDisplayList>,
	public CToolTipDialog<CDebugDisplayList>
{
public:
	enum { IDD = IDD_Debugger_DisplayList };

	enum {
		DisplayListCtrl_Col_PAddr,
		DisplayListCtrl_Col_SegOffset,
		DisplayListCtrl_Col_RawCommand,
		DisplayListCtrl_Col_Command,
		DisplayListCtrl_Col_Parameters
	};

	CDebugDisplayList(CDebuggerUI * debugger);
	virtual ~CDebugDisplayList(void);

	void Refresh(void);

private:
	CListViewCtrl m_DisplayListCtrl;
    CDisplayListParser m_DisplayListParser;
    CEdit m_StateTextbox;

	bool m_bRefreshPending;

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(void);
	LRESULT OnClicked(WORD wNotifyCode, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled);
    LRESULT OnListItemChanged(NMHDR* pNMHDR);

	//LRESULT OnActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	//LRESULT OnListDblClicked(NMHDR* pNMHDR);
	//LRESULT OnListItemChanged(NMHDR* pNMHDR);
	//LRESULT OnMeasureItem(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	//LRESULT OnScroll(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	//void    OnExitSizeMove(void);
	//void InterceptMouseWheel(WPARAM wParam, LPARAM lParam);
	//static CDebugCPULogView* _this;
	//static HHOOK hWinMessageHook;
	//static LRESULT CALLBACK HookProc(int nCode, WPARAM wParam, LPARAM lParam);
	//void ToggleLoggingEnabled(void);
	//void ShowRegStates(size_t stateIndex);
	//void Export(void);
	//int GetNumVisibleRows(CListViewCtrl& list);
	//bool MouseHovering(WORD ctrlId, int xMargin = 0, int yMargin = 0);

	BEGIN_MSG_MAP_EX(CDebugDisplayList)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_CODE_HANDLER(BN_CLICKED, OnClicked)
        NOTIFY_HANDLER_EX(IDC_LST_DLIST, LVN_ITEMCHANGED, OnListItemChanged)
		//NOTIFY_HANDLER_EX(IDC_CPU_LIST, NM_DBLCLK, OnListDblClicked)
		//MESSAGE_HANDLER(WM_MEASUREITEM, OnMeasureItem)
		//MSG_WM_EXITSIZEMOVE(OnExitSizeMove)
		CHAIN_MSG_MAP(CDialogResize<CDebugDisplayList>)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CDebugDisplayList)
        DLGRESIZE_CONTROL(IDC_LST_DLIST, DLSZ_SIZE_X | DLSZ_SIZE_Y)
        DLGRESIZE_CONTROL(IDC_BTN_REFRESH, DLSZ_MOVE_X | DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_EDIT_STATE, DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_TEX_PREVIEW, DLSZ_MOVE_Y)
	END_DLGRESIZE_MAP()

	BEGIN_TOOLTIP_MAP()
		//TOOLTIP(IDC_BUFFSIZE_EDIT, "Maximum number of states to keep (1024 = 416kB)") // sizeof(CPUState)
	END_TOOLTIP_MAP()
};
