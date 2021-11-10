/**************************************************************************

	WWWC

	UpMessage.c

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
#include "OleDragDrop.h"


/**************************************************************************
	Define
**************************************************************************/

#define DEF_RIGHT						120		//デフォルトの位置
#define DEF_BOTTOM						140

#define LIST_TOP						20		//リストビューの高さ

#define BTN_LEFT						100		//ボタンの位置
#define BTN_RIGHT						88
#define BTN_BOTTOM						23
#define BTN_VIEW_TOP					10
#define BTN_WAIT_TOP					40
#define BTN_INFO_TOP					70

#define UPINFO_VIEWSPEED				20		//詳細を展開する速さ

#define DnD_CLIPFORMAT_CNT				3
#define MENU_MAX						1000


/**************************************************************************
	Global Variables
**************************************************************************/

struct TPLVCOLUMN *upColumnInfo = NULL;
struct TPITEM **UpItemList;
int UpItemListCnt = 0;

static struct TPITEM **ViewUpItemList;
static int ViewUpItemListCnt = 0;
static BOOL SizeFlag;
static BOOL MaxWndFlag;

static WNDPROC BannerWindowProcedure;

//外部参照
extern HINSTANCE g_hinst;				//アプリケーションのインスタンスハンドル
extern HWND WWWCWnd;					//本体
extern HWND FocusWnd;
extern HWND UpWnd;
extern HTREEITEM RootItem;
extern HTREEITEM DgdpItem;
extern struct TP_PROTOCOL *tpProtocol;
extern struct TPLVCOLUMN *SortColInfo;
extern BOOL WWWCDropFlag;
extern UINT WWWC_ClipFormat;
extern HMENU hPOPUP;
extern struct TP_TOOLS *ToolList;
extern int ToolListCnt;
extern int gCheckFlag;
extern HFONT ListFont;

extern int LvSpaceNextFocus;

extern int UPSnd;
extern char WaveFile[];
extern int UPMsg;
extern int NoUpMsg;
extern int CheckUPItemClear;
extern int CheckUPItemAutoSort;
extern int ClearTime;
extern int UPMsgTop;
extern int UPActive;
extern int UPAni;
extern int UPMsgExpand;
extern int UPWinLeft;
extern int UPWinTop;
extern int UPWinRight;
extern int UPWinBottom;
extern int UPWinPosSave;
extern int UPWinSizeSave;
extern int UPWinExpandCenter;
extern int LvUPExStyle;
extern int LvUPSortFlag;
extern int LvUPColSize[];
extern int LvUPColCnt;
extern char LvUPColumn[];

extern int OpenReturnIcon;
extern int DnDReturnIcon;


/**************************************************************************
	Local Function Prototypes
**************************************************************************/

static BOOL CopyUPItem(void);
static void FreeViewUpItemList(void);
static BOOL ListView_UpGetDispItem(HWND hWnd, LV_ITEM *lvItem);
static LRESULT UpListViewHeaderNotifyProc(HWND hWnd, LPARAM lParam, struct TPLVCOLUMN *ColInfo);
static void InitializeUpMessage(HWND hDlg);
static void CloseUpMessage(HWND hDlg);
static void ShowUpMessage(HWND hDlg);
static void SizeInitUpMessage(HWND hDlg, int Size, int Pos, int Expand);
static void SetUpMessageControls(HWND hDlg);
static void SizeUpMessage(HWND hDlg, WPARAM wParam);
static void ViewUpInfo(HWND hDlg);
static void SetBannerSubClass(HWND hWnd);
static void DelBannerSubClass(HWND hWnd);
static LRESULT CALLBACK SubClassBannerProc(HWND hWnd, UINT msg, WPARAM wParam,LPARAM lParam);


/******************************************************************************

	AddUpItem

	更新アイテム情報にアイテム情報を追加

******************************************************************************/

BOOL AddUpItem(struct TPITEM *NewItemInfo)
{
	struct TPITEM **tpItemInfo;
	int i;

	if(UpItemList == NULL){
		UpItemListCnt = 0;
		return FALSE;
	}

	//重複アイテム情報のチェック
	for(i = 0; i < UpItemListCnt; i++){
		if((*(UpItemList + i)) == NULL){
			continue;
		}
		//既に同じアイテムが存在している場合は上書きを行う
		if((*(UpItemList + i))->hItem == NewItemInfo->hItem &&
			lstrcmp((*(UpItemList + i))->Title, NewItemInfo->Title) == 0 &&
			lstrcmp((*(UpItemList + i))->CheckURL, NewItemInfo->CheckURL) == 0){

			FreeItemInfo(*(UpItemList + i), FALSE);
			GlobalFree(*(UpItemList + i));
			*(UpItemList + i) = Item_Copy(NewItemInfo);
			return FALSE;
		}
	}

	tpItemInfo = (struct TPITEM **)GlobalAlloc(GPTR,
		sizeof(struct TPITEM *) * (UpItemListCnt + 1));
	if(tpItemInfo == NULL){
		return FALSE;
	}

	CopyMemory(tpItemInfo, UpItemList, sizeof(struct TPITEM *) * UpItemListCnt);
	//コピーの作成
	*(tpItemInfo + UpItemListCnt) = Item_Copy(NewItemInfo);

	GlobalFree(UpItemList);
	UpItemList = tpItemInfo;

	UpItemListCnt++;
	return TRUE;
}


/******************************************************************************

	CopyUPItem

	更新アイテム情報を表示アイテム情報に移す

******************************************************************************/

static BOOL CopyUPItem(void)
{
	struct TPITEM **tpItemInfo;
	int DecCnt, i, j;

	if(UpItemList == NULL){
		UpItemListCnt = 0;
		return FALSE;
	}

	//削除済みカウント
	DecCnt = 0;

	//重複アイテム情報のチェック
	for(j = 0; j < UpItemListCnt; j++){
		if((*(UpItemList + j)) == NULL){
			continue;
		}
		for(i = 0; i < ViewUpItemListCnt; i++){
			if((*(ViewUpItemList + i)) == NULL){
				continue;
			}
			//既に同じアイテムが存在している場合は上書きを行う
			if((*(ViewUpItemList + i))->hItem == (*(UpItemList + j))->hItem &&
				lstrcmp((*(ViewUpItemList + i))->Title, (*(UpItemList + j))->Title) == 0 &&
				lstrcmp((*(ViewUpItemList + i))->CheckURL, (*(UpItemList + j))->CheckURL) == 0){

				FreeItemInfo(*(ViewUpItemList + i), FALSE);
				GlobalFree(*(ViewUpItemList + i));

				*(ViewUpItemList + i) = *(UpItemList + j);

				*(UpItemList + j) = NULL;
				DecCnt++;
				break;
			}
		}
	}

	//表示アイテム情報の作成
	tpItemInfo = (struct TPITEM **)GlobalAlloc(GPTR,
		sizeof(struct TPITEM *) * (ViewUpItemListCnt + UpItemListCnt - DecCnt));
	if(tpItemInfo == NULL){
		return FALSE;
	}
	for(i = 0; i < ViewUpItemListCnt; i++){
		*(tpItemInfo + i) = *(ViewUpItemList + i);
	}
	for(j = 0; j < UpItemListCnt; j++){
		if((*(UpItemList + j)) == NULL){
			continue;
		}
		*(tpItemInfo + i) = *(UpItemList + j);
		i++;
	}
	if(ViewUpItemList != NULL){
		GlobalFree(ViewUpItemList);
	}
	ViewUpItemList = tpItemInfo;
	ViewUpItemListCnt = ViewUpItemListCnt + UpItemListCnt - DecCnt;

	GlobalFree(UpItemList);
	UpItemList = NULL;
	UpItemListCnt = 0;
	return TRUE;
}


/******************************************************************************

	FreeViewUpItemList

	表示UPアイテムの解放

******************************************************************************/

static void FreeViewUpItemList(void)
{
	if(ViewUpItemList == NULL){
		return;
	}
	//UPアイテムのメモリを解放
	FreeItemList(ViewUpItemList, ViewUpItemListCnt, FALSE);
	GlobalFree(ViewUpItemList);
	ViewUpItemList = NULL;
	ViewUpItemListCnt = 0;
}


/******************************************************************************

	ListView_UpGetDispItem

	リストビューのアイテム情報要求

******************************************************************************/

static BOOL ListView_UpGetDispItem(HWND hWnd, LV_ITEM *lvItem)
{
	struct TPITEM *tpItemInfo;
	char tmp1[BUFSIZE];
	char tmp2[BUFSIZE];
	int i;

	tpItemInfo = (struct TPITEM *)ListView_GetlParam(GetDlgItem(hWnd, WWWC_LIST), lvItem->iItem);
	if(tpItemInfo == NULL){
		return FALSE;
	}

	//テキスト設定
	if(lvItem->mask & LVIF_TEXT){
		if((upColumnInfo + lvItem->iSubItem)->p == 0){
			//アイテムが格納されているフォルダのパス
			if(IsTreeItem(WWWCWnd, tpItemInfo->hItem) == TRUE){
				TreeView_GetItemInfo(GetDlgItem(WWWCWnd, WWWC_TREE), RootItem, tmp1);
				wsprintf(tmp2, "\\\\%s", tmp1);
				TreeView_GetPath(GetDlgItem(WWWCWnd, WWWC_TREE), tpItemInfo->hItem, lvItem->pszText, tmp2);
			}

		}else if((char *)*((long *)((long)tpItemInfo + (upColumnInfo + lvItem->iSubItem)->p)) != NULL){
			//アイテム情報とカラム情報からテキストを設定
			lstrcpyn(lvItem->pszText,
				(char *)*((long *)((long)tpItemInfo + (upColumnInfo + lvItem->iSubItem)->p)), BUFSIZE - 1);

		}else{
			*(lvItem->pszText) = '\0';
		}
	}

	if((lvItem->mask & LVIF_IMAGE) == 0){
		return FALSE;
	}

	//アイコン情報設定
	i = GetProtocolIndex(tpItemInfo->CheckURL);
	//アイテムのアイコン
	if(tpItemInfo->Status & ST_UP){
		lvItem->iImage = (i == -1) ? ICON_UP : (tpProtocol + i)->Icon + 3;
	}else{
		lvItem->iImage = (i == -1) ? ICON_NOICON : (tpProtocol + i)->Icon;
	}
	return FALSE;
}


/******************************************************************************

	SetSubItemClipboardData

	ListViewアイテムをクリップボードにコピー

******************************************************************************/

BOOL SetSubItemClipboardData(HWND hWnd)
{
	HANDLE hMem, hMemText, hMemDrop;
	int ErrCode;

	//クリップボードを開く
	if(OpenClipboard(hWnd) == FALSE){
		ErrMsg(hWnd, GetLastError(), NULL);
		return FALSE;
	}
	if(EmptyClipboard() == FALSE){
		ErrCode = GetLastError();
		CloseClipboard();
		ErrMsg(hWnd, ErrCode, NULL);
		return FALSE;
	}

	//フォルダ情報
	hMem = Clipboard_Set_WF_ItemList(hWnd, FLAG_COPY, NULL);
	if(hMem == NULL){
		CloseClipboard();
		return FALSE;
	}
	//独自フォーマットの設定
	if(SetClipboardData(WWWC_ClipFormat, hMem) == NULL){
		ErrCode = GetLastError();
		CloseClipboard();
		ErrMsg(hWnd, ErrCode, NULL);
		return FALSE;
	}

	//ドロップファイル
	if(DragDrop_CreateDropFiles(hWnd, DnD_DirName) == FALSE){
		return FALSE;
	}
	if((hMemDrop = DragDrop_SetDropFileMem(hWnd)) != NULL){
		SetClipboardData(CF_HDROP, hMemDrop);
	}
	FreeDropFiles();

	//フォルダのパス
	hMemText = Clipboard_Set_TEXT(hWnd, NULL);
	if(hMemText != NULL){
		SetClipboardData(CF_TEXT, hMemText);
	}
	CloseClipboard();
	return TRUE;
}


/******************************************************************************

	StartDragSubItem

	ListViewアイテムのドラッグ＆ドロップ

******************************************************************************/

void StartDragSubItem(HWND hWnd)
{
	UINT cf[DnD_CLIPFORMAT_CNT];
	int ret;
	int i;
	int cfcnt;
	BOOL NoFileFlag;

	DgdpItem = NULL;

	//Altキーが押されている場合はファイルを扱わない
	NoFileFlag = (GetAsyncKeyState(VK_MENU) < 0) ? TRUE : FALSE;

	if(NoFileFlag == FALSE){
		if(DragDrop_CreateDropFiles(hWnd, DnD_DirName) == FALSE){
			return;
		}
	}

	WWWCDropFlag = FALSE;

	//クリップボードフォーマットの設定
	cf[0] = WWWC_ClipFormat;
	cf[1] = CF_TEXT;
	cf[2] = CF_HDROP;
	cfcnt = (NoFileFlag == FALSE) ? CLIPFORMAT_CNT : CLIPFORMAT_CNT - 1;

	ret = OLE_IDropSource_Start(hWnd, WM_DATAOBJECT_GETDATA, cf, cfcnt,
		DROPEFFECT_COPY | DROPEFFECT_LINK);

	if(NoFileFlag == FALSE){
		FreeDropFiles();
	}

	//アイテムがWWWC以外にコピーされた場合
	if(ret != -1 && ret != DROPEFFECT_NONE && WWWCDropFlag == FALSE){
		switch(DnDReturnIcon)
		{
		case 0:			//初期化しない
			break;

		case 1:			//選択が一件の場合のみアイコンを初期化
			if(ListView_GetSelectedCount(GetDlgItem(hWnd, WWWC_LIST)) != 1){
				break;
			}
			if((i = ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), -1, LVNI_SELECTED)) == -1){
				break;
			}
			InitSubIcon(hWnd, i);
			SetTrayInitIcon(WWWCWnd);
			break;

		case 2:			//アイコンを初期化
			i = -1;
			while((i = ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), i, LVNI_SELECTED)) != -1){
				InitSubIcon(hWnd, i);
			}
			SetTrayInitIcon(WWWCWnd);
			break;
		}
		RefreshListView(hWnd);
		CallTreeItem(WWWCWnd, RootItem, (FARPROC)TreeView_FreeItem, 1);
	}
}


/******************************************************************************

	UpListViewHeaderNotifyProc

	UPメッセージのリストビューヘッダメッセージ

******************************************************************************/

static LRESULT UpListViewHeaderNotifyProc(HWND hWnd, LPARAM lParam, struct TPLVCOLUMN *ColInfo)
{
	HD_NOTIFY *phd = (HD_NOTIFY *)lParam;

	switch(phd->hdr.code)
	{
	case HDN_ITEMCLICK:			//ヘッダがクリックされたのでソートを行う
		WaitCursor(TRUE);

		//ソート
		LvUPSortFlag = (ABS(LvUPSortFlag) == (phd->iItem + 1))
			? (LvUPSortFlag * -1) : (phd->iItem + 1);
		SortColInfo = ColInfo;
		ListView_SortItems(GetDlgItem(hWnd, WWWC_LIST), CompareFunc, LvUPSortFlag);

		//アイテム情報の再設定
		GlobalFree(ViewUpItemList);
		ViewUpItemListCnt = 0;
		ViewUpItemList = ListView_SetListToMem(hWnd, &ViewUpItemListCnt);

		WaitCursor(FALSE);
		break;
	}
	return FALSE;
}


/******************************************************************************

	InitSubIcon

	UPメッセージと本体のアイコンの初期化

******************************************************************************/

BOOL InitSubIcon(HWND hDlg, int Index)
{
	struct TPITEM *tpItemInfo;
	struct TPITEM *SelItemInfo;

	SelItemInfo = (struct TPITEM *)ListView_GetlParam(GetDlgItem(hDlg, WWWC_LIST), Index);
	if(SelItemInfo == NULL){
		return FALSE;
	}
	//アイテム情報を取得
	tpItemInfo = FindMainItem(WWWCWnd, SelItemInfo);
	if(tpItemInfo != NULL){
		Item_Initialize(WWWCWnd, tpItemInfo, FALSE);
		TreeView_SetIconState(WWWCWnd, SelItemInfo->hItem, 0);
		TreeView_FreeItem(WWWCWnd, SelItemInfo->hItem, 1);
	}
	Item_Initialize(hDlg, SelItemInfo, FALSE);
	return TRUE;
}


/******************************************************************************

	FindInitSubIcon

	UPメッセージからアイテムを検索してアイコンの初期化

******************************************************************************/

BOOL FindInitSubIcon(HWND hDlg, struct TPITEM *tpItemInfo)
{
	struct TPITEM *TmpItemInfo;
	int cnt;
	int i;

	cnt = ListView_GetItemCount(GetDlgItem(hDlg, WWWC_LIST));
	for(i = 0; i < cnt; i++){
		TmpItemInfo = (struct TPITEM *)ListView_GetlParam(GetDlgItem(hDlg, WWWC_LIST), i);
		if(TmpItemInfo == NULL){
			continue;
		}
		if(tpItemInfo == TmpItemInfo || (tpItemInfo->hItem == TmpItemInfo->hItem &&
			lstrcmp(tpItemInfo->Title, TmpItemInfo->Title) == 0 &&
			lstrcmp(tpItemInfo->CheckURL, TmpItemInfo->CheckURL) == 0)){
			InitSubIcon(hDlg, i);
			return TRUE;
		}
	}
	return FALSE;
}


/******************************************************************************

	MainItemSelect

	メッセージとメインウィンドウのリストビューとの選択アイテムの同期

******************************************************************************/

BOOL MainItemSelect(HWND hWnd)
{
	struct TPITEM *tpItemInfo;
	struct TPITEM *SelItemInfo;
	int i;

	//フォーカスを持つアイテムの取得
	if((i = ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), -1, LVNI_FOCUSED)) == -1){
		return FALSE;
	}
	SelItemInfo = (struct TPITEM *)ListView_GetlParam(GetDlgItem(hWnd, WWWC_LIST), i);
	if(SelItemInfo == NULL || IsTreeItem(WWWCWnd, SelItemInfo->hItem) == FALSE){
		MessageBox(hWnd, EMSG_NOMAINITEM, EMSG_MAINSHOW_TITLE, MB_ICONSTOP);
		return FALSE;
	}
	//本体ツリーの選択
	TreeView_SelectItem(GetDlgItem(WWWCWnd, WWWC_TREE), SelItemInfo->hItem);
	FocusWnd = GetDlgItem(WWWCWnd, WWWC_LIST);
	//本体ウィンドウの表示
	SendMessage(WWWCWnd, WM_COMMAND, ID_MENUITEM_REWINDOW, 0);

	//本体のアイテム情報を取得
	tpItemInfo = FindMainItem(WWWCWnd, SelItemInfo);
	if(tpItemInfo == NULL){
		return FALSE;
	}

	//全てのアイテムの選択状態を解除
	ListView_SetItemState(GetDlgItem(WWWCWnd, WWWC_LIST), -1, 0, LVIS_SELECTED);
	//指定アイテムを選択状態にする
	i = ListView_GetMemToIndex(GetDlgItem(WWWCWnd, WWWC_LIST), tpItemInfo);
	if(i != -1){
		ListView_SetItemState(GetDlgItem(WWWCWnd, WWWC_LIST), i,
			LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		ListView_EnsureVisible(GetDlgItem(WWWCWnd, WWWC_LIST), i, TRUE);
	}else{
		MessageBox(hWnd, EMSG_NOMAINITEM, EMSG_MAINSHOW_TITLE, MB_ICONSTOP);
		return FALSE;
	}
	return TRUE;
}


/******************************************************************************

	MainItemProp

	本体のアイテムのプロパティを表示

******************************************************************************/

BOOL MainItemProp(HWND hWnd)
{
	struct TPTREE *tpTreeInfo;
	struct TPITEM *SelItemInfo;
	struct TPITEM *tpItemInfo;
	struct TPITEM *PropItemInfo;
	char *p;
	int i;
	BOOL ret;

	if(ListView_GetSelectedCount(GetDlgItem(hWnd, WWWC_LIST)) <= 0){
		return FALSE;
	}
	//フォーカスを持つアイテムの取得
	i = ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), -1, LVNI_FOCUSED);
	if(ListView_GetItemState(GetDlgItem(hWnd, WWWC_LIST), i, LVIS_SELECTED) != LVIS_SELECTED){
		i = ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), -1, LVNI_SELECTED);
	}
	SelItemInfo = (struct TPITEM *)ListView_GetlParam(GetDlgItem(hWnd, WWWC_LIST), i);
	if(SelItemInfo == NULL || IsTreeItem(WWWCWnd, SelItemInfo->hItem) == FALSE){
		MessageBox(hWnd, EMSG_NOMAINITEM, EMSG_PROP_TITLE, MB_ICONSTOP);
		return FALSE;
	}

	//本体のアイテム情報を取得
	tpTreeInfo = (struct TPTREE *)TreeView_GetlParam(GetDlgItem(WWWCWnd, WWWC_TREE),
		SelItemInfo->hItem);
	if(tpTreeInfo == NULL){
		return FALSE;
	}
	tpItemInfo = GetMainItem(WWWCWnd, tpTreeInfo, SelItemInfo);
	if(tpItemInfo == NULL){
		MessageBox(hWnd, EMSG_NOMAINITEM, EMSG_PROP_TITLE, MB_ICONSTOP);
		return FALSE;
	}

	//プロパティ用アイテムの作成
	PropItemInfo = Item_Copy(tpItemInfo);
	if(PropItemInfo == NULL){
		return FALSE;
	}

	if(PropItemInfo->Comment != NULL){
		//コメントの改行コードを変換
		EscToCode(PropItemInfo->Comment);
	}

	//プロパティ表示
	tpTreeInfo->MemFlag++;
	ret = ShowItemProp(hWnd, PropItemInfo);
	if(IsTreeItem(WWWCWnd, PropItemInfo->hItem) == FALSE){
		FreeItemInfo(PropItemInfo, FALSE);
		GlobalFree(PropItemInfo);
		return FALSE;
	}
	tpTreeInfo->MemFlag--;
	if(ret == FALSE){
		FreeItemInfo(PropItemInfo, FALSE);
		GlobalFree(PropItemInfo);
		TreeView_FreeItem(WWWCWnd, SelItemInfo->hItem, 1);
		return FALSE;
	}

	if(PropItemInfo->Comment != NULL){
		//コメントの改行コードを変換
		p = CodeToEsc(PropItemInfo->Comment);
		GlobalFree(PropItemInfo->Comment);
		PropItemInfo->Comment = p;
	}

	//Subウィンドウのアイテムに反映
	i = ListView_GetMemToIndex(GetDlgItem(hWnd, WWWC_LIST), SelItemInfo);
	if(i != -1){
		FreeItemInfo(SelItemInfo, FALSE);
		Item_ContentCopy(SelItemInfo, PropItemInfo);

		ListView_SetItemText(GetDlgItem(hWnd, WWWC_LIST), i, 0, LPSTR_TEXTCALLBACK);
		ListView_RedrawItems(GetDlgItem(hWnd, WWWC_LIST), i, i);
		UpdateWindow(GetDlgItem(hWnd, WWWC_LIST));
	}

	//本体のアイテムに反映
	for(i = 0; i < tpTreeInfo->ItemListCnt; i++){
		if(tpItemInfo == *(tpTreeInfo->ItemList + i)){
			FreeItemInfo(tpItemInfo, TRUE);
			CopyMemory(tpItemInfo, PropItemInfo, sizeof(struct TPITEM));
			break;
		}
	}
	if(i >= tpTreeInfo->ItemListCnt){
		FreeItemInfo(PropItemInfo, FALSE);
		GlobalFree(PropItemInfo);
		TreeView_FreeItem(WWWCWnd, SelItemInfo->hItem, 1);
		MessageBox(hWnd, EMSG_NOMAINITEM, EMSG_PROP_TITLE, MB_ICONSTOP);
		return FALSE;
	}

	//本体のリストビューの更新
	if(tpItemInfo->hItem == TreeView_GetSelection(GetDlgItem(WWWCWnd, WWWC_TREE))){
		i = ListView_GetMemToIndex(GetDlgItem(WWWCWnd, WWWC_LIST), tpItemInfo);
		if(i != -1){
			ListView_SetItemText(GetDlgItem(WWWCWnd, WWWC_LIST), i, 0, LPSTR_TEXTCALLBACK);
			ListView_SetItemState(GetDlgItem(WWWCWnd, WWWC_LIST), i,
				((tpItemInfo->CheckSt == 1) ? INDEXTOOVERLAYMASK(ICON_ST_NOCHECK) : 0), LVIS_OVERLAYMASK);
			ListView_RedrawItems(GetDlgItem(WWWCWnd, WWWC_LIST), i, i);
			UpdateWindow(GetDlgItem(WWWCWnd, WWWC_LIST));
		}
	}
	GlobalFree(PropItemInfo);
	TreeView_FreeItem(WWWCWnd, SelItemInfo->hItem, 1);
	return TRUE;
}


/******************************************************************************

	InitializeUpMessage

	UPメッセージの初期化

******************************************************************************/

static void InitializeUpMessage(HWND hDlg)
{
	HICON hIcon;
	HICON hIconS;

	ViewUpItemList = NULL;
	ViewUpItemListCnt = 0;

	hIcon = ImageList_GetIcon(
		ListView_GetImageList(GetDlgItem(WWWCWnd, WWWC_LIST), LVSIL_NORMAL),
		ICON_UP, 0);
	hIconS = ImageList_GetIcon(
		ListView_GetImageList(GetDlgItem(WWWCWnd, WWWC_LIST), LVSIL_SMALL),
		ICON_UP, 0);

	SendMessage(hDlg, WM_SETICON, (WPARAM)TRUE, (LPARAM)hIcon);
	SendMessage(hDlg, WM_SETICON, (WPARAM)FALSE, (LPARAM)hIconS);
	SendDlgItemMessage(hDlg, IDC_STATIC_ICON, STM_SETICON, (WPARAM)hIcon, 0);

	SizeFlag = TRUE;
	SizeInitUpMessage(hDlg, UPWinSizeSave, UPWinPosSave, UPMsgExpand);
	SizeFlag = FALSE;

	if(UPMsgTop == 1){
		SetWindowPos(hDlg, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
	}

	//コントロールの位置をセット
	SetUpMessageControls(hDlg);

	//リストビューを初期化
	SetWindowLong(GetDlgItem(hDlg, WWWC_LIST), GWL_STYLE,
		GetWindowLong(GetDlgItem(hDlg, WWWC_LIST), GWL_STYLE) | LVS_SHOWSELALWAYS);
	//リストビューの拡張スタイルを設定
	SendDlgItemMessage(hDlg, WWWC_LIST, LVM_SETEXTENDEDLISTVIEWSTYLE, 0,
		LvUPExStyle | SendDlgItemMessage(hDlg, WWWC_LIST, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0));
	//リストビューのフォント設定
	if(ListFont != NULL){
		SendMessage(GetDlgItem(hDlg, WWWC_LIST), WM_SETFONT, (WPARAM)ListFont, MAKELPARAM(TRUE, 0));
	}

	LvUPColCnt = ListView_AddColumn(GetDlgItem(hDlg, WWWC_LIST), LvUPColumn, LvUPColSize, upColumnInfo);

	ListView_SetImageList(GetDlgItem(hDlg, WWWC_LIST),
		ListView_GetImageList(GetDlgItem(WWWCWnd, WWWC_LIST), LVSIL_NORMAL), LVSIL_NORMAL);
	ListView_SetImageList(GetDlgItem(hDlg, WWWC_LIST),
		ListView_GetImageList(GetDlgItem(WWWCWnd, WWWC_LIST), LVSIL_SMALL), LVSIL_SMALL);
}


/******************************************************************************

	CloseUpMessage

	UPメッセージの終了処理

******************************************************************************/

static void CloseUpMessage(HWND hDlg)
{
	HICON hIcon;
	HICON hIconS;
	int i;

	hIcon = (HICON)SendMessage(hDlg, WM_GETICON, TRUE, 0);
	hIconS = (HICON)SendMessage(hDlg, WM_GETICON, FALSE, 0);
	DestroyIcon(hIcon);
	DestroyIcon(hIconS);

	ListView_DeleteAllItems(GetDlgItem(hDlg, WWWC_LIST));

	//リストビューのカラムのサイズを取得
	for(i = 0; i < LvUPColCnt; i++){
		LvUPColSize[i] = ListView_GetColumnWidth(GetDlgItem(hDlg, WWWC_LIST), i);
	}

	FreeViewUpItemList();
	UpWnd = NULL;

	DestroyWindow(hDlg);
}


/******************************************************************************

	ShowUpMessage

	UPメッセージの表示

******************************************************************************/

static void ShowUpMessage(HWND hDlg)
{
	char buf[BUFSIZE];
	char fDay[BUFSIZE];
	char fTime[BUFSIZE];
	int i;
	int UpItemCnt;
	BOOL UpMsgFlag;

	//更新アイテムの存在を示すフラグ
	UpMsgFlag = (UpItemListCnt == 0) ? TRUE : FALSE;

	if(ClearTime == 1){		//更新時アイテムを解放する設定
		switch(CheckUPItemClear)
		{
		//無条件
		case 0:
			//前回の更新アイテム情報を解放
			SendMessage(hDlg, WM_UP_FREE, 0, 0);
			break;

		//ウィンドウが表示状態のとき
		case 1:
			if(IsWindowVisible(hDlg) == 0){
				//前回の更新アイテム情報を解放
				SendMessage(hDlg, WM_UP_FREE, 0, 0);
			}
			break;

		//空にしない
		case 2:
			break;
		}
	}

	//ウィンドウが表示されていない場合はサイズの初期化を行う
	if(IsWindowVisible(hDlg) == 0){
		SendMessage(hDlg, WM_UP_WININI, 0, 0);
	}

	//更新メッセージ表示
	if(NoUpMsg == 1 || UPMsg == 1){
		ShowWindow(hDlg, SW_SHOWNOACTIVATE);
		if(UPMsgTop == 0){
			//非アクティブで前面に持ってくる
			SetWindowPos(hDlg, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
			SetWindowPos(hDlg, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
		}
	}

	//リストビューのアイテムをすべて消去
	ListView_DeleteAllItems(GetDlgItem(hDlg, WWWC_LIST));

	//UPアイテムのメモリを作成
	CopyUPItem();
	ListView_SetItemCount(GetDlgItem(hDlg, WWWC_LIST), ViewUpItemListCnt);

	//リストビューにアイテムを追加（コールバックアイテム）
	UpItemCnt = 0;
	for(i = 0; i < ViewUpItemListCnt; i++){
		if((*(ViewUpItemList + i)) == NULL){
			continue;
		}
		ListView_InsertItemEx(GetDlgItem(hDlg, WWWC_LIST),
			(char*)LPSTR_TEXTCALLBACK, I_IMAGECALLBACK,
			(long)*(ViewUpItemList + i), -1);
		UpItemCnt++;
	}

	//自動ソート
	if(CheckUPItemAutoSort == 1){
		SortColInfo = upColumnInfo;
		ListView_SortItems(GetDlgItem(hDlg, WWWC_LIST), CompareFunc, LvUPSortFlag);

		//アイテム情報の再設定
		GlobalFree(ViewUpItemList);
		ViewUpItemListCnt = 0;
		ViewUpItemList = ListView_SetListToMem(hDlg, &ViewUpItemListCnt);
	}
	ListView_SetItemState(GetDlgItem(hDlg, WWWC_LIST), 0, LVIS_FOCUSED, LVIS_FOCUSED);

	//更新メッセージを表示
	GetDateTime(fDay, fTime);
	if(UpMsgFlag == TRUE){
		//更新アイテムが存在しない場合
		if(UpItemCnt == 0){
			//表示が0件の場合
			wsprintf(buf, STR_ZEROMSG, fDay, fTime);
		}else{
			//表示がある場合
			wsprintf(buf, STR_NOTUPMSG, UpItemCnt, fDay, fTime);
		}
	}else{
		//更新アイテムが存在する場合
		wsprintf(buf, STR_UPMSG, UpItemCnt, fDay, fTime);
	}
	SetWindowText(GetDlgItem(hDlg, IDC_STATIC_MSG), buf);

	//アクティブにする設定
	if(UPMsg == 1 && UPActive == 1){
		ShowWindow(hDlg, SW_SHOW);
		_SetForegroundWindow(hDlg);
	}
}


/******************************************************************************

	SizeInitUpMessage

	UPメッセージのサイズの初期化

******************************************************************************/

static void SizeInitUpMessage(HWND hDlg, int Size, int Pos, int Expand)
{
	RECT DesktopRect, WindowRect, lvRect, StRect, BnRect, WinRec;
	int i, BnSize;

	GetWindowRect(GetDesktopWindow(), (LPRECT)&DesktopRect);	//デスクトップのサイズ
	GetWindowRect(hDlg, (LPRECT)&WindowRect);				//ウィンドウのサイズ
	GetWindowRect(GetDlgItem(hDlg, IDC_STATIC_MSG), (LPRECT)&StRect);

	GetWindowRect(GetDlgItem(hDlg, IDC_STATIC_BANNER), (LPRECT)&BnRect);
	BnSize = (BnRect.bottom - BnRect.top < 32) ? 32 : BnRect.bottom - BnRect.top;

	if(Size == 1 && (UPWinBottom != 0 && UPWinRight != 0)){
		//前回のサイズに表示
		SetWindowPos(hDlg, 0, 0, 0, UPWinRight, WindowRect.bottom - WindowRect.top,
			SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
	}else{
		UPWinBottom = DEF_BOTTOM;

		i = StRect.right - WindowRect.left + GetSystemMetrics(SM_CYSIZEFRAME) + DEF_RIGHT;
		SetWindowPos(hDlg, 0, 0, 0, i, WindowRect.bottom - WindowRect.top,
			SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
	}

	GetWindowRect(hDlg, (LPRECT)&WindowRect);				//ウィンドウのサイズ
	GetClientRect(hDlg, (LPRECT)&WinRec);
	GetWindowRect(GetDlgItem(hDlg, WWWC_LIST), (LPRECT)&lvRect);

	SetWindowPos(GetDlgItem(hDlg, WWWC_LIST), 0, 0,StRect.bottom - StRect.top + BnSize + LIST_TOP,
		WinRec.right, UPWinBottom, SWP_NOACTIVATE | SWP_NOZORDER);
	GetWindowRect(GetDlgItem(hDlg, WWWC_LIST), (LPRECT)&lvRect);

	//ウィンドウの縦サイズをセット
	i = lvRect.top - WindowRect.top + GetSystemMetrics(SM_CYSIZEFRAME);
	if(Expand == 1){
		//ウィンドウを展開状態にする場合は、ボタンの名前を変更
		SetWindowText(GetDlgItem(hDlg, IDC_BUTTON_INFO), BTN_UPINFO_OFF);
		//ウィンドウの縦サイズにリストビューのサイズを加える
		i = lvRect.bottom - WindowRect.top + GetSystemMetrics(SM_CYSIZEFRAME);
		ShowWindow(GetDlgItem(hDlg, WWWC_LIST), SW_SHOW);
	}else{
		SetWindowText(GetDlgItem(hDlg, IDC_BUTTON_INFO), BTN_UPINFO_ON);
		ShowWindow(GetDlgItem(hDlg, WWWC_LIST), SW_HIDE);
	}

	//センタリングのために一時ウィンドウサイズ変更
	if(UPWinExpandCenter == 1){
		SetWindowPos(hDlg, 0, 0, 0,
			WindowRect.right - WindowRect.left,
			lvRect.bottom - WindowRect.top + GetSystemMetrics(SM_CYSIZEFRAME),
			SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
	}else{
		SetWindowPos(hDlg, 0, 0, 0, WindowRect.right - WindowRect.left, i,
			SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
	}
	GetWindowRect(hDlg, (LPRECT)&WindowRect);				//ウィンドウのサイズ

	if(Pos == 1 && (UPWinLeft != 0 && UPWinTop != 0)){
		//前回の表示位置に表示
		SetWindowPos(hDlg, 0, UPWinLeft, UPWinTop, WindowRect.right - WindowRect.left, i,
			SWP_NOACTIVATE | SWP_NOZORDER);
	}else{
		//センタリングしてデフォルトのサイズで表示
		SetWindowPos(hDlg, 0,
			(DesktopRect.right / 2) - ((WindowRect.right - WindowRect.left) / 2),
			(DesktopRect.bottom / 2) - ((WindowRect.bottom - WindowRect.top) / 2),
			WindowRect.right - WindowRect.left, i, SWP_NOACTIVATE | SWP_NOZORDER);
	}
}


/******************************************************************************

	SetUpMessageControls

	UPメッセージ内のコントロールのサイズ・位置を変更

******************************************************************************/

static void SetUpMessageControls(HWND hDlg)
{
	RECT WinRec, WindowRect, lvRect, StRect, BnRect;
	char buf[BUFSIZE];
	int BnSize;

	GetClientRect(hDlg, (LPRECT)&WinRec);

	//ボタンの位置設定
	MoveWindow(GetDlgItem(hDlg, IDC_BUTTON_VIEW),
		WinRec.right - BTN_LEFT, BTN_VIEW_TOP, BTN_RIGHT, BTN_BOTTOM, TRUE);
	MoveWindow(GetDlgItem(hDlg, IDC_BUTTON_WAIT),
		WinRec.right - BTN_LEFT, BTN_WAIT_TOP, BTN_RIGHT, BTN_BOTTOM, TRUE);
	MoveWindow(GetDlgItem(hDlg, IDC_BUTTON_INFO),
		WinRec.right - BTN_LEFT, BTN_INFO_TOP, BTN_RIGHT, BTN_BOTTOM, TRUE);

	//ウィンドウの高さ設定
	GetWindowText(GetDlgItem(hDlg, IDC_BUTTON_INFO), buf, BUFSIZE - 1);
	if(lstrcmp(buf, BTN_UPINFO_OFF) == 0){
		GetWindowRect(GetDlgItem(hDlg, IDC_STATIC_MSG), (LPRECT)&StRect);
		GetWindowRect(GetDlgItem(hDlg, IDC_STATIC_BANNER), (LPRECT)&BnRect);
		BnSize = (BnRect.bottom - BnRect.top < 32) ? 32 : BnRect.bottom - BnRect.top;

		MoveWindow(GetDlgItem(hDlg, WWWC_LIST), 0, StRect.bottom - StRect.top + BnSize + LIST_TOP,
			WinRec.right, WinRec.bottom - (StRect.bottom - StRect.top + BnSize + LIST_TOP), TRUE);
	}else{
		GetWindowRect(hDlg, (LPRECT)&WindowRect);
		GetWindowRect(GetDlgItem(hDlg, WWWC_LIST), (LPRECT)&lvRect);
		SetWindowPos(hDlg, 0, 0, 0,
			WindowRect.right - WindowRect.left,
			lvRect.top - WindowRect.top + GetSystemMetrics(SM_CYSIZEFRAME),
			SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
	}
	UpdateWindow(GetDlgItem(hDlg, WWWC_LIST));

	//メッセージの再描画
	InvalidateRect(GetDlgItem(hDlg, IDC_STATIC_MSG), NULL, FALSE);
	UpdateWindow(GetDlgItem(hDlg, IDC_STATIC_MSG));

	//ボタンの再描画
	InvalidateRect(GetDlgItem(hDlg, IDC_BUTTON_VIEW), NULL, FALSE);
	UpdateWindow(GetDlgItem(hDlg, IDC_BUTTON_VIEW));
	InvalidateRect(GetDlgItem(hDlg, IDC_BUTTON_WAIT), NULL, FALSE);
	UpdateWindow(GetDlgItem(hDlg, IDC_BUTTON_WAIT));
	InvalidateRect(GetDlgItem(hDlg, IDC_BUTTON_INFO), NULL, FALSE);
	UpdateWindow(GetDlgItem(hDlg, IDC_BUTTON_INFO));
}


/******************************************************************************

	SizeUpMessage

	UPメッセージのサイズ変更

******************************************************************************/

static void SizeUpMessage(HWND hDlg, WPARAM wParam)
{
	RECT WindowRect, lvRect;
	char buf[BUFSIZE];

	if(SizeFlag == TRUE){
		return;
	}
	GetWindowText(GetDlgItem(hDlg, IDC_BUTTON_INFO), buf, BUFSIZE - 1);
	if(wParam == SIZE_RESTORED && MaxWndFlag == TRUE && lstrcmp(buf, BTN_UPINFO_OFF) == 0){
		//詳細表示で元のサイズに戻った場合
		MaxWndFlag = FALSE;
		GetWindowRect(hDlg, (LPRECT)&WindowRect);
		GetWindowRect(GetDlgItem(hDlg, WWWC_LIST), (LPRECT)&lvRect);
		SetWindowPos(GetDlgItem(hDlg, WWWC_LIST), 0, 0, 0,
			lvRect.right - lvRect.left, UPWinBottom, SWP_NOZORDER | SWP_NOMOVE);
		GetWindowRect(GetDlgItem(hDlg, WWWC_LIST), (LPRECT)&lvRect);
		SetWindowPos(hDlg, 0, 0, 0,
			WindowRect.right - WindowRect.left,
			lvRect.bottom - WindowRect.top + GetSystemMetrics(SM_CYSIZEFRAME),
			SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);

	}else if(wParam == SIZE_RESTORED && MaxWndFlag == TRUE && lstrcmp(buf, BTN_UPINFO_ON) == 0){
		//詳細非表示で元のサイズに戻った場合
		MaxWndFlag = FALSE;
		GetWindowRect(GetDlgItem(hDlg, WWWC_LIST), (LPRECT)&lvRect);
		SetWindowPos(GetDlgItem(hDlg, WWWC_LIST), 0, 0, 0,
			lvRect.right - lvRect.left, UPWinBottom, SWP_NOZORDER | SWP_NOMOVE);

	}else if(wParam == SIZE_MAXIMIZED && MaxWndFlag == FALSE){
		//最大化された場合
		MaxWndFlag = TRUE;
		SetWindowText(GetDlgItem(hDlg, IDC_BUTTON_INFO), BTN_UPINFO_OFF);
		ShowWindow(GetDlgItem(hDlg, WWWC_LIST), SW_SHOW);
	}
	//コントロールの位置をセット
	SetUpMessageControls(hDlg);
}


/******************************************************************************

	RefreshListView

	UPメッセージと本体の描画の更新

******************************************************************************/

void RefreshListView(HWND hDlg)
{
	InvalidateRect(GetDlgItem(WWWCWnd, WWWC_LIST), NULL, FALSE);
	UpdateWindow(GetDlgItem(WWWCWnd, WWWC_LIST));
	InvalidateRect(GetDlgItem(hDlg, WWWC_LIST), NULL, FALSE);
	UpdateWindow(GetDlgItem(hDlg, WWWC_LIST));
	CallTreeItem(WWWCWnd, RootItem, (FARPROC)TreeView_FreeItem, 1);

	ListView_RefreshItem(GetDlgItem(WWWCWnd, WWWC_LIST));
	ListView_RefreshItem(GetDlgItem(hDlg, WWWC_LIST));

	SetTrayInitIcon(WWWCWnd);
}


/******************************************************************************

	ViewUpInfo

	詳細の表示状態を切り替える

******************************************************************************/

static void ViewUpInfo(HWND hDlg)
{
	RECT WindowRect, lvRect;
	char buf[BUFSIZE];
	int i, j;

	GetWindowRect(hDlg, (LPRECT)&WindowRect);					//ウィンドウのサイズ
	GetWindowRect(GetDlgItem(hDlg, WWWC_LIST), (LPRECT)&lvRect);	//リストビューのサイズ

	GetWindowText(GetDlgItem(hDlg, IDC_BUTTON_INFO), buf, BUFSIZE - 1);
	if(lstrcmp(buf, BTN_UPINFO_OFF) == 0){
		//詳細を隠す
		SetWindowText(GetDlgItem(hDlg, IDC_BUTTON_INFO), BTN_UPINFO_ON);
		ShowWindow(GetDlgItem(hDlg, WWWC_LIST), SW_HIDE);

		//ウィンドウの縦サイズをセット
		i = lvRect.top - WindowRect.top + GetSystemMetrics(SM_CYSIZEFRAME);

	}else{
		//詳細を表示
		//ウィンドウを展開状態にする場合は、ボタンの名前を変更
		SetWindowText(GetDlgItem(hDlg, IDC_BUTTON_INFO), BTN_UPINFO_OFF);
		ShowWindow(GetDlgItem(hDlg, WWWC_LIST), SW_SHOW);

		//徐々に表示
		i = lvRect.bottom - WindowRect.top + GetSystemMetrics(SM_CYSIZEFRAME);
		if(UPAni == 1){
			SizeFlag = TRUE;
			for(j = (WindowRect.bottom - WindowRect.top); j <= i; j += UPINFO_VIEWSPEED){
				MoveWindow(hDlg, WindowRect.left, WindowRect.top, WindowRect.right - WindowRect.left, j, TRUE);
				UpdateWindow(GetDlgItem(hDlg, WWWC_LIST));
			}
			SizeFlag = FALSE;
		}
	}
	//ウィンドウのサイズを設定
	SetWindowPos(hDlg, 0, 0, 0,WindowRect.right - WindowRect.left, i,
		SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
	SetUpMessageControls(hDlg);
}


/******************************************************************************

	ExeAction

	プロトコル毎の処理およびツールの実行

******************************************************************************/

void ExeAction(HWND hDlg, WPARAM wParam)
{
	struct TPITEM **ToolItemList = NULL;
	struct TPITEM **TmpItemList;
	int i;
	int id;

	//プロトコル毎の処理
	if(LOWORD(wParam) >= ID_MENU_ACTION && LOWORD(wParam) < (ID_MENU_ACTION + MENU_MAX)){
		Item_Open(hDlg, LOWORD(wParam) - ID_MENU_ACTION);
		return;
	}

	id = LOWORD(wParam) - ID_MENU_TOOL_ACTION;
	if(LOWORD(wParam) < ID_MENU_TOOL_ACTION || LOWORD(wParam) >= (ID_MENU_TOOL_ACTION + MENU_MAX)){
		return;
	}

	//指定プロトコルのアイテムを抽出
	TmpItemList = ListView_SelectItemToMem(GetDlgItem(hDlg, WWWC_LIST), &i);
	if(TmpItemList != NULL){
		ToolItemList = Item_ProtocolSelect(TmpItemList, &i, ToolList[id].Protocol);
		GlobalFree(TmpItemList);
	}
	if(ToolItemList == NULL){
		return;
	}
	//本体のアイテムと同期
	WaitCursor(TRUE);
	Item_CopyMainContent(WWWCWnd, ToolItemList, i);
	WaitCursor(FALSE);

	//ツールの実行
	SubItemExecTool(hDlg, id, ToolItemList, i, TOOL_EXEC_ITEMMENU, 0);

	WaitCursor(TRUE);
	//更新フラグが立っているアイテムを本体に書き戻す
	Item_CopyMainRefreshContent(WWWCWnd, ToolItemList, i);
	if(ToolList[id].Action & TOOL_EXEC_INITITEM){
		//アイコンの初期化
		Item_MainItemIni(WWWCWnd, hDlg, ToolItemList, i);
	}
	//アイテム情報の解放
	GlobalFree(ToolItemList);
	CallTreeItem(WWWCWnd, RootItem, (FARPROC)TreeView_FreeItem, 1);

	//表示の更新
	RefreshListView(hDlg);
	WaitCursor(FALSE);
}


/******************************************************************************

	SetBannerSubClass

	ウィンドウのサブクラス化

******************************************************************************/

static void SetBannerSubClass(HWND hWnd)
{
	BannerWindowProcedure = (WNDPROC)SetWindowLong(hWnd, GWL_WNDPROC, (long)SubClassBannerProc);
}


/******************************************************************************

	DelBannerSubClass

	ウィンドウクラスを標準のものに戻す

******************************************************************************/

static void DelBannerSubClass(HWND hWnd)
{
	SetWindowLong(hWnd, GWL_WNDPROC, (long)BannerWindowProcedure);
	BannerWindowProcedure = NULL;
}


/******************************************************************************

	SubClassBannerProc

	ウィンドウのサブクラス化

******************************************************************************/

static LRESULT CALLBACK SubClassBannerProc(HWND hWnd, UINT msg, WPARAM wParam,LPARAM lParam)
{
	switch(msg)
	{
	case WM_MOUSEMOVE:
		SetCursor(LoadCursor(g_hinst, MAKEINTRESOURCE(IDC_CURSOR_HAND)));
		return 0;
	}
	return CallWindowProc(BannerWindowProcedure, hWnd, msg, wParam, lParam);
}


/******************************************************************************

	UpMessageProc

	UPメッセージのウィンドウプロシージャ

******************************************************************************/

BOOL CALLBACK UpMessageProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	RECT WindowRect, lvRect;
	char buf[BUFSIZE];
	int i;

	switch (uMsg)
	{
	case WM_INITDIALOG:
		InitializeUpMessage(hDlg);
		SetBannerSubClass(GetDlgItem(hDlg, IDC_STATIC_BANNER));
		break;

	case WM_UP_WININI:
		if(UPMsg == 1 && (IsZoomed(hDlg) != 0 || IsIconic(hDlg) != 0)){
			ShowWindow(hDlg, SW_RESTORE);
		}

		//サイズの初期化
		SizeFlag = TRUE;
		SizeInitUpMessage(hDlg, UPWinSizeSave, UPWinPosSave, UPMsgExpand);
		SizeFlag = FALSE;

		//コントロールの位置をセット
		SetUpMessageControls(hDlg);
		break;

	case WM_UP_INIT:
		ShowUpMessage(hDlg);
		break;

	case WM_UP_FREE:
		SetWindowText(GetDlgItem(hDlg, IDC_STATIC_MSG), STR_NOUPMSG);
		ListView_DeleteAllItems(GetDlgItem(hDlg, WWWC_LIST));
		FreeViewUpItemList();
		break;

	case WM_UP_CLOSE:
		DelBannerSubClass(GetDlgItem(hDlg, IDC_STATIC_BANNER));
		CloseUpMessage(hDlg);
		break;

	case WM_SIZE:
		SizeUpMessage(hDlg, wParam);
		break;

	case WM_SIZING:
		//詳細が隠されている場合はサイズを制限
		GetWindowText(GetDlgItem(hDlg, IDC_BUTTON_INFO), buf, BUFSIZE - 1);
		if(lstrcmp(buf, BTN_UPINFO_ON) == 0){
			GetWindowRect(GetDlgItem(hDlg, WWWC_LIST), (LPRECT)&lvRect);
			((LPRECT)lParam)->bottom = lvRect.top + GetSystemMetrics(SM_CYSIZEFRAME);
		}
		return FALSE;

	case WM_EXITSIZEMOVE:
		if(IsWindowVisible(hDlg) != 0 && IsIconic(hDlg) == 0 && IsZoomed(hDlg) == 0){
			//ウィンドウの位置を保存
			GetWindowRect(hDlg, (LPRECT)&WindowRect);
			GetWindowRect(GetDlgItem(hDlg, WWWC_LIST), (LPRECT)&lvRect);
			UPWinLeft = WindowRect.left;
			UPWinTop = WindowRect.top;
			UPWinRight = WindowRect.right - WindowRect.left;
			UPWinBottom = lvRect.bottom - lvRect.top;
		}
		break;

	case WM_CLOSE:
		SetFocus(GetDlgItem(hDlg, IDC_BUTTON_VIEW));
		ShowWindow(hDlg, SW_HIDE);
		break;

	case WM_TIMER:
		switch(wParam)
		{
		//次のアイテムにフォーカスを移動する
		case TIMER_NEXTFOCUS:
			KillTimer(hDlg, wParam);
			ListView_SetItemState(GetDlgItem(hDlg, WWWC_LIST),
				ListView_GetNextItem(GetDlgItem(hDlg, WWWC_LIST), -1, LVNI_FOCUSED) + 1,
				LVIS_FOCUSED, LVIS_FOCUSED);
			ListView_EnsureVisible(GetDlgItem(hDlg, WWWC_LIST),
				ListView_GetNextItem(GetDlgItem(hDlg, WWWC_LIST), -1, LVNI_FOCUSED), TRUE);
			break;
		}
		break;

	case WM_NOTIFY:
		//リストビューヘッダコントロール
		if(((NMHDR *)lParam)->hwndFrom == GetWindow(GetDlgItem(hDlg, WWWC_LIST), GW_CHILD)){
			return UpListViewHeaderNotifyProc(hDlg, lParam, upColumnInfo);
		}
		//リストビュー
		if(((NMHDR *)lParam)->hwndFrom != GetDlgItem(hDlg, WWWC_LIST)){
			return FALSE;
		}
		return ListView_NotifyProc(hDlg, lParam);

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_STATIC_BANNER:
			ExecItemFile(hDlg, BANNER_URL, "", NULL, 0);
			break;

		case ID_KEY_RETURN:
			if(GetFocus() != GetDlgItem(hDlg, WWWC_LIST)){
				//フォーカスの持つボタンを選択
				SendMessage(hDlg, WM_COMMAND, GetDlgCtrlID(GetFocus()), 0);
				break;
			}
			//プロトコル毎のメニューを設定
			DeleteProtocolMenuItem(GetSubMenu(hPOPUP, MENU_POP_UPITEM));
			SetProtocolItemMenu(hDlg, GetSubMenu(hPOPUP, MENU_POP_UPITEM), TRUE, TRUE);
			SetEnableToolItemMenu(GetSubMenu(hPOPUP, MENU_POP_UPITEM));
			if((i = GetMenuDefaultItem(GetSubMenu(hPOPUP, MENU_POP_UPITEM), 0, 0)) == -1){
				break;
			}
			SendMessage(hDlg, WM_COMMAND, i, 0);
			break;

		case ID_KEY_RBUTTON:
			if(GetFocus() != GetDlgItem(hDlg, WWWC_LIST)){
				break;
			}
			if(ListView_GetSelectedCount(GetDlgItem(hDlg, WWWC_LIST)) <= 0){
				ShowMenu(hDlg, hPOPUP, MENU_POP_UPMSG);
				break;
			}
			//アイテムメニューの表示
			EnableMenuItem(GetSubMenu(hPOPUP, MENU_POP_UPITEM), ID_MENUITEM_UPVIEW, MF_ENABLED);
			EnableMenuItem(GetSubMenu(hPOPUP, MENU_POP_UPITEM), ID_MENUITEM_COPY, MF_ENABLED);
			EnableMenuItem(GetSubMenu(hPOPUP, MENU_POP_UPITEM), ID_MENUITEM_INITICON_POP, MF_ENABLED);
			EnableMenuItem(GetSubMenu(hPOPUP, MENU_POP_UPITEM), ID_MENUITEM_PROP, MF_ENABLED);
			DeleteProtocolMenuItem(GetSubMenu(hPOPUP, MENU_POP_UPITEM));
			SetProtocolItemMenu(hDlg, GetSubMenu(hPOPUP, MENU_POP_UPITEM), TRUE, TRUE);
			SetEnableToolItemMenu(GetSubMenu(hPOPUP, MENU_POP_UPITEM));
			ShowMenu(hDlg, hPOPUP, MENU_POP_UPITEM);
			break;

		case ID_KEY_ESC:
		//待機
		case IDC_BUTTON_WAIT:
			SendMessage(hDlg, WM_CLOSE, 0, 0);
			break;

		//参照
		case IDC_BUTTON_VIEW:
			if(ViewUpItemListCnt != 0 && ViewUpItemList != NULL && *ViewUpItemList != NULL
				&& IsTreeItem(WWWCWnd, (*ViewUpItemList)->hItem) == TRUE){
				//一番目のアイテムのフォルダを選択
				TreeView_SelectItem(GetDlgItem(WWWCWnd, WWWC_TREE),
					(*ViewUpItemList)->hItem);
			}
			//本体ウィンドウの表示
			SendMessage(WWWCWnd, WM_COMMAND, ID_MENUITEM_REWINDOW, 0);
			ShowWindow(hDlg, SW_HIDE);
			break;

		//詳細
		case IDC_BUTTON_INFO:
			//詳細の表示の切り替え
			ViewUpInfo(hDlg);
			break;

		//開く
		case ID_MENU_ACTION_OPEM:
			Item_Open(hDlg, -1);
			break;

		//全て選択
		case ID_MENUITEM_ALLSELECT:
			SetFocus(GetDlgItem(hDlg, WWWC_LIST));
			ListView_SetItemState(GetDlgItem(hDlg, WWWC_LIST), -1, LVIS_SELECTED, LVIS_SELECTED);
			break;

		//更新アイテムを選択
		case ID_MENUITEM_UPSELECT:
			SetFocus(GetDlgItem(hDlg, WWWC_LIST));
			ListView_UpSelectItem(GetDlgItem(hDlg, WWWC_LIST));
			break;

		//表示
		case ID_MENUITEM_UPVIEW:
			//本体のアイコンを表示
			MainItemSelect(hDlg);
			break;

		//コピー
		case ID_MENUITEM_COPY:
			SetSubItemClipboardData(hDlg);
			break;

		//全てのアイコンを初期化
		case ID_MENUITEM_ALLINITICON:
			WaitCursor(TRUE);
			for(i = 0; i < ListView_GetItemCount(GetDlgItem(hDlg, WWWC_LIST)); i++){
				InitSubIcon(hDlg, i);
			}
			RefreshListView(hDlg);
			WaitCursor(FALSE);
			break;

		//アイコンの初期化
		case ID_MENUITEM_INITICON:
		case ID_MENUITEM_INITICON_POP:
			if(ListView_GetSelectedCount(GetDlgItem(hDlg, WWWC_LIST)) <= 0){
				break;
			}
			WaitCursor(TRUE);
			i = -1;
			while((i = ListView_GetNextItem(GetDlgItem(hDlg, WWWC_LIST), i, LVNI_SELECTED)) != -1){
				InitSubIcon(hDlg, i);
			}
			RefreshListView(hDlg);
			WaitCursor(FALSE);
			break;

		//クリア
		case ID_MENUITEM_UPITEMCLEAR:
			SendMessage(hDlg, WM_UP_FREE, 0, 0);
			break;

		//プロパティ
		case ID_MENUITEM_PROP:
			MainItemProp(hDlg);
			break;

		default:
			ExeAction(hDlg, wParam);
			break;
		}
		break;

	//リストビューのイベント
	case WM_LV_EVENT:
		switch(wParam)
		{
		case LVN_GETDISPINFO:
			return ListView_UpGetDispItem(hDlg, &(((LV_DISPINFO *)lParam)->item));

		case LVN_BEGINDRAG:
		case LVN_BEGINRDRAG:
			StartDragSubItem(hDlg);
			break;

		case LVN_KEYDOWN:
			if(LvSpaceNextFocus == 1 && ((LV_KEYDOWN *)lParam)->wVKey == VK_SPACE){
				SetTimer(hDlg, TIMER_NEXTFOCUS, 1, NULL);
			}
			break;

		case NM_CLICK:
			if(((SendDlgItemMessage(hDlg, WWWC_LIST, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0) & LVS_EX_TRACKSELECT) == 0 ||
				ListView_GetSelectedCount(GetDlgItem(hDlg, WWWC_LIST)) != 1 ||
				ListView_MouseSelectItem(GetDlgItem(hDlg, WWWC_LIST)) == FALSE)){
				break;
			}
			SendMessage(hDlg, WM_COMMAND, ID_KEY_RETURN, 0);
			break;

		case NM_DBLCLK:
			if((SendDlgItemMessage(hDlg, WWWC_LIST, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0) & LVS_EX_TRACKSELECT) != 0){
				break;
			}
			SendMessage(hDlg, WM_COMMAND, ID_KEY_RETURN, 0);
			break;

		case NM_RCLICK:
			SendMessage(hDlg, WM_COMMAND, ID_KEY_RBUTTON, 0);
			break;
		}
		break;

	//リストビューアイテムの初期化
	case WM_LV_INITICON:
		InitSubIcon(hDlg, wParam);
		RefreshListView(hDlg);
		break;

	//ドラッグ＆ドロップのデータ要求
	case WM_DATAOBJECT_GETDATA:
		//WWWCアイテムフォーマット
		if(wParam == WWWC_ClipFormat){
			*((HGLOBAL *)lParam) = Clipboard_Set_WF_ItemList(hDlg, FLAG_COPY, NULL);
			WWWCDropFlag = TRUE;
			break;
		}

		WWWCDropFlag = FALSE;
		switch(wParam)
		{
		case CF_TEXT:	//テキスト
			*((HGLOBAL *)lParam) = Clipboard_Set_TEXT(hDlg, NULL);
			break;

		case CF_HDROP:	//ドロップファイル
			*((HGLOBAL *)lParam) = DragDrop_SetDropFileMem(hDlg);
			break;
		}
		break;

	//DLLからのメッセージを本体に転送
	case WM_GETVERSION:
	case WM_GETMAINWINDOW:
	case WM_GETUPWINDOW:
	case WM_GETFINDWINDOW:
	case WM_GETCHECKLIST:
	case WM_GETCHECKLISTCNT:
	case WM_GETMAINITEM:
	case WM_WWWC_GETINI:
	case WM_WWWC_PUTINI:
	case WM_FOLDER_SAVE:
	case WM_FOLDER_LOAD:
	case WM_FOLDER_GETPATH:
	case WM_FOLDER_GETWWWCPATH:
	case WM_FOLDER_SELECT:
		return SendMessage(WWWCWnd, uMsg, wParam, lParam);

	case WM_ITEMEXEC:
		return Item_DefaultOpen(hDlg, (struct TPITEM *)lParam);

	case WM_ITEMINIT:
		if(FindInitSubIcon(hDlg, (struct TPITEM *)lParam) == FALSE){
			return SendMessage(WWWCWnd, uMsg, wParam, lParam);
		}
		RefreshListView(hDlg);
		break;

	default:
		return FALSE;
	}
	return TRUE;
}
/* End of source */
