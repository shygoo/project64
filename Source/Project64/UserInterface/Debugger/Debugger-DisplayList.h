#pragma once

#include <stdafx.h>
#include "DebuggerUI.h"
#include "Util/DisplayListParser.h"

typedef struct
{
	COLORREF fgColor;
	COLORREF bgColor;
} list_entry_colors_t;

class CDebugDisplayList :
	public CDebugDialog<CDebugDisplayList>,
	public CDialogResize<CDebugDisplayList>,
	public CToolTipDialog<CDebugDisplayList>
{
public:
	enum { IDD = IDD_Debugger_DisplayList };

	enum {
		DisplayListCtrl_Col_VAddr,
		DisplayListCtrl_Col_SegOffset,
		DisplayListCtrl_Col_RawCommand,
		DisplayListCtrl_Col_Command,
		DisplayListCtrl_Col_Parameters
	};

	CDebugDisplayList(CDebuggerUI * debugger);
	virtual ~CDebugDisplayList(void);

	void Refresh(void);

private:
    bool m_bRefreshPending;
	std::vector<list_entry_colors_t> m_ListColors;
	std::vector<dram_resource_t> m_RamResources;
	
	CListViewCtrl m_DisplayListCtrl;
    CDisplayListParser m_DisplayListParser;
    CEdit m_StateTextbox;
	CStatic m_StatusText;

    CTreeViewCtrl m_ResourceTreeCtrl;
    CImageList m_TreeImageList;

    HTREEITEM m_hTreeDisplayLists;
    HTREEITEM m_hTreeSegments;
    HTREEITEM m_hTreeImages;
    HTREEITEM m_hTreeVertices;
    HTREEITEM m_hTreeMatrices;
    HTREEITEM m_hTreeViewports;
    HTREEITEM m_hTreeLights;
    
    void ResetResourceTreeCtrl(void);

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(void);
	LRESULT OnClicked(WORD wNotifyCode, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled);
    LRESULT OnListItemChanged(NMHDR* pNMHDR);
    LRESULT OnResourceTreeSelChanged(NMHDR* pNMHDR);
	LRESULT OnCustomDrawList(NMHDR* pNMHDR);
	LRESULT OnMeasureItem(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);

	//LRESULT OnListDblClicked(NMHDR* pNMHDR);
	//void    OnExitSizeMove(void);
	//void ShowRegStates(size_t stateIndex);
	//void Export(void);

	BEGIN_MSG_MAP_EX(CDebugDisplayList)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_CODE_HANDLER(BN_CLICKED, OnClicked)
        NOTIFY_HANDLER_EX(IDC_LST_DLIST, LVN_ITEMCHANGED, OnListItemChanged)
        NOTIFY_HANDLER_EX(IDC_TREE_RESOURCES, TVN_SELCHANGED, OnResourceTreeSelChanged)
		NOTIFY_HANDLER_EX(IDC_LST_DLIST, NM_CUSTOMDRAW, OnCustomDrawList)
		MESSAGE_HANDLER(WM_MEASUREITEM, OnMeasureItem)
		//NOTIFY_HANDLER_EX(IDC_CPU_LIST, NM_DBLCLK, OnListDblClicked)
		//MSG_WM_EXITSIZEMOVE(OnExitSizeMove)
		CHAIN_MSG_MAP(CDialogResize<CDebugDisplayList>)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CDebugDisplayList)
        DLGRESIZE_CONTROL(IDC_LST_DLIST, DLSZ_SIZE_X | DLSZ_SIZE_Y)
        DLGRESIZE_CONTROL(IDC_BTN_REFRESH, DLSZ_MOVE_X)
        DLGRESIZE_CONTROL(IDC_GRP_STATE, DLSZ_SIZE_X | DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_EDIT_STATE, DLSZ_SIZE_X | DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_GRP_RESOURCES, DLSZ_MOVE_X | DLSZ_SIZE_Y)
        DLGRESIZE_CONTROL(IDC_TREE_RESOURCES, DLSZ_MOVE_X | DLSZ_SIZE_Y)
        DLGRESIZE_CONTROL(IDC_TEX_PREVIEW, DLSZ_MOVE_X | DLSZ_MOVE_Y)
	END_DLGRESIZE_MAP()

	BEGIN_TOOLTIP_MAP()
	END_TOOLTIP_MAP()
};
