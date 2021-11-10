/**************************************************************************

	WWWC

	SelectFolder.c

	Copyright (C) 1996-2003 by Nakashima Tomoaki. All rights reserved.
		http://www.nakka.com/
		nakka@nakka.com

**************************************************************************/


/**************************************************************************
	Include Files
**************************************************************************/

#define _INC_OLE
#include <windows.h>
#undef	_INC_OLE

#include "General.h"


/**************************************************************************
	Define
**************************************************************************/


/**************************************************************************
	Global Variables
**************************************************************************/

static HWND folWnd;

//外部参照
extern HINSTANCE g_hinst;				//アプリケーションのインスタンスハンドル
extern HWND WWWCWnd;					//本体
extern HTREEITEM RootItem;


/**************************************************************************
	Local Function Prototypes
**************************************************************************/

static void CALLBACK FindTreeItem(HWND hWnd, HTREEITEM hItem, long Param);
static void CALLBACK CreateTreeItem(HWND hWnd, HTREEITEM hItem, long Param);
static void CreateTree(HWND hDlg, HTREEITEM hItem);
static BOOL CALLBACK SelectFolderProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);


/******************************************************************************

	FindTreeItem

	本体のアイテムから選択ツリーのアイテムを検索

******************************************************************************/

static void CALLBACK FindTreeItem(HWND hWnd, HTREEITEM hItem, long Param)
{
	if(TreeView_GetlParam(GetDlgItem(hWnd, WWWC_TREE), hItem) == (HTREEITEM)*((long *)Param)){
		(HTREEITEM)*((long *)Param) = hItem;
	}
}


/******************************************************************************

	CreateTreeItem

	本体のツリーアイテムを選択ツリーに追加

******************************************************************************/

static void CALLBACK CreateTreeItem(HWND hWnd, HTREEITEM hItem, long Param)
{
	TV_INSERTSTRUCT tvitn;
	TV_ITEM tvit;
	HTREEITEM pItem;
	char buf[BUFSIZE];
	long l;

	tvit.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_STATE;
	tvit.hItem = hItem;
	tvit.cchTextMax = BUFSIZE - 1;
	tvit.pszText = buf;
	tvit.iImage = 0;
	tvit.iSelectedImage = 0;
	tvit.state = 0;
	tvit.stateMask = TVIS_OVERLAYMASK;

	if(TreeView_GetItem(GetDlgItem(hWnd, WWWC_TREE), &tvit) == FALSE){
		return;
	}
	tvit.lParam = (LPARAM)hItem;
	tvit.hItem = NULL;
	tvit.cChildren = 0;

	if(tvit.iImage == ICON_DIR_CLOSE_CH || tvit.iImage == ICON_DIR_CLOSE_CHECKCHILD){
		tvit.iImage = ICON_DIR_CLOSE;
	}
	if(tvit.iSelectedImage == ICON_DIR_OPEN_CH || tvit.iSelectedImage == ICON_DIR_OPEN_CHECKCHILD){
		tvit.iSelectedImage = ICON_DIR_OPEN;
	}

	if(hItem == RootItem){
		tvitn.hParent = (HTREEITEM)TVI_ROOT;
	}else{
		pItem = TreeView_GetParent(GetDlgItem(hWnd, WWWC_TREE), hItem);
		l = (long)pItem;
		CallTreeItem(folWnd, TreeView_GetRoot(GetDlgItem(folWnd, WWWC_TREE)), (FARPROC)FindTreeItem, (long)&l);
		if(l == (long)pItem){
			return;
		}
		tvitn.hParent = (HTREEITEM)l;
	}
	tvitn.item = tvit;

	pItem = TreeView_InsertItem(GetDlgItem(folWnd, WWWC_TREE), &tvitn);

	if((HTREEITEM)Param == hItem){
		TreeView_SelectItem(GetDlgItem(folWnd, WWWC_TREE), pItem);
	}
}


/******************************************************************************

	CreateTree

	本体のツリーを選択ツリーにコピー

******************************************************************************/

static void CreateTree(HWND hDlg, HTREEITEM hItem)
{
	CallTreeItem(WWWCWnd, RootItem, (FARPROC)CreateTreeItem, (long)hItem);
	TreeView_Expand(GetDlgItem(hDlg, WWWC_TREE), TreeView_GetRoot(GetDlgItem(folWnd, WWWC_TREE)), TVE_EXPAND);
}


/******************************************************************************

	SelectFolderProc

	フォルダ選択ウィンドウプロシージャ

******************************************************************************/

static BOOL CALLBACK SelectFolderProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HTREEITEM hItem;
	char *ret;
	char tmp1[BUFSIZE];
	char tmp2[BUFSIZE];

	switch (uMsg)
	{
	case WM_INITDIALOG:
		folWnd = hDlg;

		SetWindowLong(hDlg, GWL_USERDATA, lParam);
		ret = (char *)lParam;
		if(ret == NULL){
			EndDialog(hDlg, FALSE);
			break;
		}

		TreeView_SetImageList(GetDlgItem(hDlg, WWWC_TREE),
			TreeView_GetImageList(GetDlgItem(WWWCWnd, WWWC_TREE), TVSIL_NORMAL), TVSIL_NORMAL);

		hItem = TreeView_FindItemPath(GetDlgItem(WWWCWnd, WWWC_TREE), RootItem, ret);
		if(hItem == NULL){
			hItem = RootItem;
		}
		CreateTree(hDlg, hItem);
		break;

	case WM_CLOSE:
		EndDialog(hDlg, FALSE);
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDCANCEL:
			SendMessage(hDlg, WM_CLOSE, 0, 0);
			break;

		case IDOK:
			ret = (char *)GetWindowLong(hDlg, GWL_USERDATA);
			if(ret == NULL){
				EndDialog(hDlg, FALSE);
				break;
			}

			TreeView_GetItemInfo(GetDlgItem(WWWCWnd, WWWC_TREE), RootItem, tmp1);
			wsprintf(tmp2, "\\\\%s", tmp1);
			TreeView_GetPath(GetDlgItem(WWWCWnd, WWWC_TREE),
				(HTREEITEM)TreeView_GetlParam(GetDlgItem(hDlg, WWWC_TREE), TreeView_GetSelection(GetDlgItem(hDlg, WWWC_TREE))),
				ret, tmp2);

			EndDialog(hDlg, TRUE);
			break;
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}


/******************************************************************************

	SelectFolder

	フォルダ選択ウィンドウの表示

******************************************************************************/

BOOL SelectFolder(HWND hWnd, char *ret)
{
	return DialogBoxParam(g_hinst, MAKEINTRESOURCE(IDD_DIALOG_SELECTFOLDER), hWnd, SelectFolderProc, (LPARAM)ret);
}

/******************************************************************************

	SelectFolderItem

	フォルダ選択ウィンドウの表示

******************************************************************************/

HTREEITEM SelectFolderItem(HWND hWnd, HTREEITEM hItem)
{
	HTREEITEM hRetItem;
	char buf[BUFSIZE];
	char tmp[BUFSIZE];

	TreeView_GetItemInfo(GetDlgItem(hWnd, WWWC_TREE), RootItem, buf);
	wsprintf(tmp, "\\\\%s", buf);
	TreeView_GetPath(GetDlgItem(hWnd, WWWC_TREE), hItem, buf, tmp);

	if(DialogBoxParam(g_hinst, MAKEINTRESOURCE(IDD_DIALOG_SELECTFOLDER), hWnd, SelectFolderProc, (LPARAM)buf) == TRUE){
		hRetItem = TreeView_FindItemPath(GetDlgItem(hWnd, WWWC_TREE), RootItem, buf);
		return ((hRetItem != NULL) ? hRetItem : RootItem);
	}
	return NULL;
}
/* End of source */
