/**************************************************************************

	WWWC

	Item.c

	Copyright (C) 1996-2006 by Nakashima Tomoaki. All rights reserved.
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

int UpdateItemFlag;

//外部参照
extern HINSTANCE g_hinst;				//アプリケーションのインスタンスハンドル
extern HWND WWWCWnd;
extern char CuDir[];
extern struct TP_PROTOCOL *tpProtocol;
extern int ProtocolCnt;
extern struct TP_TOOLS *ToolList;
extern int ToolListCnt;
extern HTREEITEM HiTestItem;
extern HTREEITEM RootItem;
extern HTREEITEM RecyclerItem;

extern int OpenReturnIcon;
extern int DefNoCheck;


/**************************************************************************
	Local Function Prototypes
**************************************************************************/

static int Item_Action(HWND hWnd, struct TPITEM *tpItemInfo, char *StrAction);
static void SetCopyItemInfo(struct TPITEM *tpItemInfo, HWND IconWnd, HWND InfoWnd);
static BOOL CALLBACK UpdateMsgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static int UpdateItemInfo(HWND hWnd, struct TPITEM *tpToItemInfo, struct TPITEM *tpFromItemInfo);
static BOOL Item_SelectDelete(HWND hWnd);
static BOOL FolderDelete(HWND hWnd);


/******************************************************************************

	FreeItemInfo

	アイテム情報の解放

******************************************************************************/

void FreeItemInfo(struct TPITEM *TmpItemInfo, BOOL ProtocolFreeFlag)
{
	char buf[BUFSIZE];
	int ProtocolIndex;

	if(ProtocolFreeFlag == TRUE){
		ProtocolIndex = GetProtocolIndex(TmpItemInfo->CheckURL);
		if(ProtocolIndex != -1 && (tpProtocol + ProtocolIndex)->lib != NULL){
			if((tpProtocol + ProtocolIndex)->Func_FreeItem == NULL){
				wsprintf(buf, "%sFreeItem", (tpProtocol + ProtocolIndex)->FuncHeader);
				(tpProtocol + ProtocolIndex)->Func_FreeItem = GetProcAddress((HMODULE)(tpProtocol + ProtocolIndex)->lib, buf);
			}
			if((tpProtocol + ProtocolIndex)->Func_FreeItem != NULL){
				(tpProtocol + ProtocolIndex)->Func_FreeItem(TmpItemInfo);
			}
		}
	}

	if(TmpItemInfo->Title != NULL) GlobalFree(TmpItemInfo->Title);
	if(TmpItemInfo->CheckURL != NULL) GlobalFree(TmpItemInfo->CheckURL);
	if(TmpItemInfo->Size != NULL) GlobalFree(TmpItemInfo->Size);
	if(TmpItemInfo->Date != NULL) GlobalFree(TmpItemInfo->Date);

	if(TmpItemInfo->CheckDate != NULL) GlobalFree(TmpItemInfo->CheckDate);
	if(TmpItemInfo->OldSize != NULL) GlobalFree(TmpItemInfo->OldSize);
	if(TmpItemInfo->OldDate != NULL) GlobalFree(TmpItemInfo->OldDate);

	if(TmpItemInfo->ViewURL != NULL) GlobalFree(TmpItemInfo->ViewURL);
	if(TmpItemInfo->Option1 != NULL) GlobalFree(TmpItemInfo->Option1);
	if(TmpItemInfo->Option2 != NULL) GlobalFree(TmpItemInfo->Option2);
	if(TmpItemInfo->Comment != NULL) GlobalFree(TmpItemInfo->Comment);
	if(TmpItemInfo->ErrStatus != NULL) GlobalFree(TmpItemInfo->ErrStatus);

	if(TmpItemInfo->DLLData1 != NULL) GlobalFree(TmpItemInfo->DLLData1);
	if(TmpItemInfo->DLLData2 != NULL) GlobalFree(TmpItemInfo->DLLData2);
}


/******************************************************************************

	FreeItemList

	アイテムリストの解放

******************************************************************************/

void FreeItemList(struct TPITEM **tpItemInfo, int ItemCnt, BOOL ProtocolFreeFlag)
{
	int i;

	//アイテム情報の解放
	for(i = 0;i < ItemCnt;i++){
		if((*tpItemInfo) == NULL){
			tpItemInfo++;
			continue;
		}

		FreeItemInfo(*tpItemInfo, ProtocolFreeFlag);
		GlobalFree(*tpItemInfo);
		*tpItemInfo = NULL;

		tpItemInfo++;
	}
}


/******************************************************************************

	GetMainItem

	本体のアイテムを取得

******************************************************************************/

struct TPITEM *GetMainItem(HWND hWnd, struct TPTREE *tpTreeInfo, struct TPITEM *SelItemInfo)
{
	struct TPITEM *tpItemInfo;
	int i;

	if(tpTreeInfo->ItemList == NULL){
		WaitCursor(TRUE);
		if(ReadTreeMem(hWnd, SelItemInfo->hItem) == FALSE){
			WaitCursor(FALSE);
			return NULL;
		}
		WaitCursor(FALSE);
	}

	if(tpTreeInfo->ItemList == NULL){
		return NULL;
	}

	//タイトルとURLが一致するアイテムを検索にする
	for(i = 0; i < tpTreeInfo->ItemListCnt; i++){
		tpItemInfo = *(tpTreeInfo->ItemList + i);
		if(tpItemInfo == NULL){
			continue;
		}
		if(lstrcmp(tpItemInfo->Title, SelItemInfo->Title) == 0 &&
			lstrcmp(tpItemInfo->CheckURL, SelItemInfo->CheckURL) == 0){
			return *(tpTreeInfo->ItemList + i);
		}
	}
	return NULL;
}


/******************************************************************************

	FindMainItem

	本体のアイテムを検索

******************************************************************************/

struct TPITEM *FindMainItem(HWND hWnd, struct TPITEM *SelItemInfo)
{
	struct TPTREE *tpTreeInfo;

	if(SelItemInfo == NULL){
		return NULL;
	}
	if(IsTreeItem(hWnd, SelItemInfo->hItem) == FALSE){
		return NULL;
	}
	//アイテム情報を取得
	tpTreeInfo = (struct TPTREE *)TreeView_GetlParam(GetDlgItem(hWnd, WWWC_TREE), SelItemInfo->hItem);
	if(tpTreeInfo == NULL){
		return NULL;
	}
	return GetMainItem(hWnd, tpTreeInfo, SelItemInfo);
}


/******************************************************************************

	Item_Action

	アイテムのプロトコル毎の動作を実行

******************************************************************************/

static int Item_Action(HWND hWnd, struct TPITEM *tpItemInfo, char *StrAction)
{
	char buf[BUFSIZE];
	char *p;
	int ProtocolIndex;
	int ret = 0;
	int i;

	ProtocolIndex = GetProtocolIndex(tpItemInfo->CheckURL);
	if(StrAction == NULL || (ProtocolIndex == -1 && lstrcmpi(StrAction, "open") == 0)){
		//関連付けで実行
		p = tpItemInfo->ViewURL;
		if(p == NULL || *p == '\0'){
			p = tpItemInfo->CheckURL;
		}
		if(p != NULL && *p != '\0'){
			ExecItemFile(hWnd, p, "", tpItemInfo, 0);
			ret = 1;
		}
	}else{
		//プロトコルを識別できない場合
		if(ProtocolIndex == -1 || tpProtocol[ProtocolIndex].lib == NULL){
			return 0;
		}
		//Actionの検索
		for(i = 0; i < tpProtocol[ProtocolIndex].tpMenuCnt; i++){
			if(lstrcmpi(tpProtocol[ProtocolIndex].tpMenu[i].Action, StrAction) == 0){
				break;
			}
		}
		if(i >= tpProtocol[ProtocolIndex].tpMenuCnt){
			return 0;
		}

		//実行
		if(tpProtocol[ProtocolIndex].Func_ExecItem == NULL){
			wsprintf(buf, "%sExecItem", tpProtocol[ProtocolIndex].FuncHeader);
			tpProtocol[ProtocolIndex].Func_ExecItem = GetProcAddress((HMODULE)tpProtocol[ProtocolIndex].lib, buf);
		}
		if(tpProtocol[ProtocolIndex].Func_ExecItem != NULL){
			ret = tpProtocol[ProtocolIndex].Func_ExecItem(hWnd, tpProtocol[ProtocolIndex].tpMenu[i].Action, tpItemInfo);
		}
	}
	if(WWWCWnd == NULL || IsWindow(WWWCWnd) == 0){
		//プロセスの強制終了
		exit(0);
	}
	return ret;
}


/******************************************************************************

	Item_Open

	アイテムを開く

******************************************************************************/

int Item_Open(HWND hWnd, int ID)
{
	struct TPITEM *tpItemInfo;
	HTREEITEM hItem;
	char *StrAction = NULL;
	int SelectItem;
	int ProtocolIndex;
	int ret;

	//フォーカスを持つアイテムのプロトコルを取得
	SelectItem = ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), -1, LVNI_FOCUSED);
	if(ListView_GetItemState(GetDlgItem(hWnd, WWWC_LIST), SelectItem, LVIS_SELECTED) != LVIS_SELECTED){
		SelectItem = ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), -1, LVNI_SELECTED);
	}

	tpItemInfo = (struct TPITEM *)ListView_GetlParam(GetDlgItem(hWnd, WWWC_LIST), SelectItem);
	if(tpItemInfo == NULL){
		//フォルダを開く
		hItem = TreeView_GetIndexToItem(GetDlgItem(hWnd, WWWC_TREE),
			TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)), SelectItem);
		if(hItem != NULL){
			TreeView_SelectItem(GetDlgItem(hWnd, WWWC_TREE), hItem);
		}
		return 1;
	}

	//Actionの取得
	ProtocolIndex = GetProtocolIndex(tpItemInfo->CheckURL);
	if(ProtocolIndex != -1 && ID != -1){
		StrAction = tpProtocol[ProtocolIndex].tpMenu[ID].Action;
	}

	SelectItem = -1;
	while(IsWindow(hWnd) != 0 &&
		(SelectItem = ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), SelectItem, LVNI_SELECTED)) != -1){
		tpItemInfo = (struct TPITEM *)ListView_GetlParam(GetDlgItem(hWnd, WWWC_LIST), SelectItem);
		if(tpItemInfo == NULL){
			continue;
		}
		ret = Item_Action(hWnd, tpItemInfo, StrAction);
		if(ret == 1 && OpenReturnIcon == 1){
			//アイコンの初期化
			SendMessage(hWnd, WM_ITEMINIT, 0, (LPARAM)tpItemInfo);
		}
	}
	ListView_RefreshItem(GetDlgItem(hWnd, WWWC_LIST));
	return 1;
}


/******************************************************************************

	Item_DefaultAction

	アイテムのデフォルトの動作を実行

******************************************************************************/

BOOL Item_DefaultOpen(HWND hWnd, struct TPITEM *tpItemInfo)
{
	int ProtocolIndex;
	int pMenuCnt;
	int i, j;
	int ret;

	//プロトコルなし
	ProtocolIndex = GetProtocolIndex(tpItemInfo->CheckURL);
	if(ProtocolIndex == -1 || tpProtocol[ProtocolIndex].tpMenu == NULL){
		ret = Item_Action(hWnd, tpItemInfo, NULL);
		if(ret == 1 && OpenReturnIcon == 1){
			//アイコンの初期化
			SendMessage(hWnd, WM_ITEMINIT, 0, (LPARAM)tpItemInfo);
		}
		return TRUE;
	}

	//ツール
	for(j = 0; j < ToolListCnt; j++){
		if((ToolList[j].Action & TOOL_EXEC_ITEMMENU) == 0 ||
			strlistcmp(tpProtocol[ProtocolIndex].title, ToolList[j].Protocol, ',') == FALSE ||
			lstrcmp(ToolList[j].title, "-") == 0){
			continue;
		}
		if((ToolList[j].Action & TOOL_EXEC_MENUDEFAULT) != 0){
			if(str_match("*.dll", ToolList[j].FileName) == TRUE){
				//DLLの実行
				DllToolExec(hWnd, j, &tpItemInfo, 1, TOOL_EXEC_ITEMMENU, 0);
			}else{
				//ツールに設定されたファイルを実行
				ExecItemFile(hWnd, ToolList[j].FileName, ToolList[j].CommandLine, tpItemInfo,
					ToolList[j].Action & TOOL_EXEC_SYNC);
			}
			if(ToolList[j].Action & TOOL_EXEC_INITITEM){
				//アイコンの初期化
				SendMessage(hWnd, WM_ITEMINIT, 0, (LPARAM)tpItemInfo);
			}
			return TRUE;
		}
	}

	//プロトコル
	pMenuCnt = tpProtocol[ProtocolIndex].tpMenuCnt;
	if(pMenuCnt == 0){
		return FALSE;
	}
	for(i = 0; i < pMenuCnt; i++){
		if(lstrcmp(tpProtocol[ProtocolIndex].tpMenu[i].name, "-") == 0){
			continue;
		}
		if(tpProtocol[ProtocolIndex].tpMenu[i].Default == TRUE){
			ret = Item_Action(hWnd, tpItemInfo, tpProtocol[ProtocolIndex].tpMenu[i].Action);
			if(ret == 1 && OpenReturnIcon == 1){
				//アイコンの初期化
				SendMessage(hWnd, WM_ITEMINIT, 0, (LPARAM)tpItemInfo);
			}
			return TRUE;
		}
	}
	return FALSE;
}


/******************************************************************************

	Item_ContentCopy

	アイテム情報をコピーする

******************************************************************************/

void Item_ContentCopy(struct TPITEM *NewItemInfo, struct TPITEM *TmpItemInfo)
{
	//構造体のコピー
	CopyMemory(NewItemInfo, TmpItemInfo, sizeof(struct TPITEM));

	//アイテム情報のコピー
	NewItemInfo->Title = AllocCopy(TmpItemInfo->Title);
	NewItemInfo->CheckURL = AllocCopy(TmpItemInfo->CheckURL);
	NewItemInfo->Size = AllocCopy(TmpItemInfo->Size);
	NewItemInfo->Date = AllocCopy(TmpItemInfo->Date);
	NewItemInfo->CheckDate = AllocCopy(TmpItemInfo->CheckDate);
	NewItemInfo->OldSize = AllocCopy(TmpItemInfo->OldSize);
	NewItemInfo->OldDate = AllocCopy(TmpItemInfo->OldDate);
	NewItemInfo->ViewURL = AllocCopy(TmpItemInfo->ViewURL);
	NewItemInfo->Option1 = AllocCopy(TmpItemInfo->Option1);
	NewItemInfo->Option2 = AllocCopy(TmpItemInfo->Option2);
	NewItemInfo->Comment = AllocCopy(TmpItemInfo->Comment);
	NewItemInfo->ErrStatus = AllocCopy(TmpItemInfo->ErrStatus);
	NewItemInfo->DLLData1 = AllocCopy(TmpItemInfo->DLLData1);
	NewItemInfo->DLLData2 = AllocCopy(TmpItemInfo->DLLData2);
}


/******************************************************************************

	Item_Copy

	アイテム情報のコピーを作成する

******************************************************************************/

struct TPITEM *Item_Copy(struct TPITEM *TmpItemInfo)
{
	struct TPITEM *NewItemInfo;

	//新しいアイテム情報の確保
	NewItemInfo = GlobalAlloc(GPTR, sizeof(struct TPITEM));
	if(NewItemInfo == NULL){
		return NULL;
	}
	Item_ContentCopy(NewItemInfo, TmpItemInfo);
	NewItemInfo->RefreshFlag = FALSE;
	return NewItemInfo;
}


/******************************************************************************

	Item_CopyMainContent

	アイテムリストと本体のアイテムの内容の同期

******************************************************************************/

void Item_CopyMainContent(HWND hWnd, struct TPITEM **tpItemList, int cnt)
{
	struct TPITEM *tpItemInfo;
	int i, st, ist;

	for(i = 0; i < cnt; i++){
		if((*(tpItemList + i)) == NULL){
			continue;
		}
		tpItemInfo = FindMainItem(hWnd, (*(tpItemList + i)));
		if(tpItemInfo == NULL || tpItemInfo->IconStatus == ST_CHECK){
			continue;
		}
		st = (*(tpItemList + i))->Status;
		ist = (*(tpItemList + i))->IconStatus;

		FreeItemInfo((*(tpItemList + i)), FALSE);
		Item_ContentCopy((*(tpItemList + i)), tpItemInfo);

		(*(tpItemList + i))->Status = st;
		(*(tpItemList + i))->IconStatus = ist;
	}
}


/******************************************************************************

	Item_CopyMainContent

	アイテムリストの再描画フラグが立っているものを本体にコピーする

******************************************************************************/

void Item_CopyMainRefreshContent(HWND hWnd, struct TPITEM **tpItemList, int cnt)
{
	struct TPITEM *tpItemInfo;
	int i, st, ist;

	for(i = 0; i < cnt; i++){
		if((*(tpItemList + i)) == NULL || (*(tpItemList + i))->RefreshFlag == FALSE){
			continue;
		}
		tpItemInfo = FindMainItem(hWnd, (*(tpItemList + i)));
		if(tpItemInfo == NULL || tpItemInfo->IconStatus == ST_CHECK){
			continue;
		}
		st = tpItemInfo->Status;
		ist = tpItemInfo->IconStatus;

		FreeItemInfo(tpItemInfo, TRUE);
		Item_ContentCopy(tpItemInfo, (*(tpItemList + i)));

		tpItemInfo->Status = st;
		tpItemInfo->IconStatus = ist;
	}
}


/******************************************************************************

	Item_Select

	アイテム情報からファイルに存在しないアイテムを削除する

******************************************************************************/

BOOL Item_Select(HWND hWnd, HTREEITEM hItem)
{
	struct TPTREE *tpTreeInfo;
	struct TPITEM **tpItemList;
	struct TPITEM **tpTmpItemList;
	struct TPITEM **tpFileItemList;
	struct TPITEM *tpItemInfo;
	char TmpBuf[BUFSIZE];
	char SourcePath[BUFSIZE];
	char *p, *r;
	int ItemCnt, NewItemCnt, FileItemCnt;
	int i, j, n;
	int ListItemCnt;

	//ツリービューからアイテムリスト取得
	tpTreeInfo = (struct TPTREE *)TreeView_GetlParam(GetDlgItem(hWnd, WWWC_TREE), hItem);
	if(tpTreeInfo == NULL){
		return FALSE;
	}
	//アイテム情報を読み込む
	if(tpTreeInfo->ItemList == NULL){
		return FALSE;
	}
	ItemCnt = tpTreeInfo->ItemListCnt;

	//ファイルよりアイテムリストを読み込む
	TreeView_GetPath(GetDlgItem(hWnd, WWWC_TREE), hItem, TmpBuf, CuDir);
	wsprintf(SourcePath, "%s\\"DATAFILENAME, TmpBuf);
	tpFileItemList = ReadItemList(SourcePath, &FileItemCnt, NULL);
	if(tpFileItemList == NULL){
		return FALSE;
	}

	//アイテムリストの確保
	tpItemList = (struct TPITEM **)GlobalAlloc(GPTR, sizeof(struct TPITEM *) * ItemCnt);
	if(tpItemList == NULL){
		return FALSE;
	}

	//作業用アイテムリスト
	tpTmpItemList = (struct TPITEM **)GlobalAlloc(GPTR, sizeof(struct TPITEM *) * FileItemCnt);
	if(tpTmpItemList == NULL){
		GlobalFree(tpItemList);
		return FALSE;
	}
	CopyMemory(tpTmpItemList, tpFileItemList, sizeof(struct TPITEM *) * FileItemCnt);

	ListView_SetRedraw(GetDlgItem(hWnd, WWWC_LIST), LDF_NODRAW);

	NewItemCnt = 0;
	//ファイルに存在するアイテム以外は削除を行う
	for(i = 0;i < ItemCnt;i++){
		if(*(tpTreeInfo->ItemList + i) == NULL){
			continue;
		}
		for(j = 0;j < FileItemCnt;j++){
			if(*(tpTmpItemList + j) == NULL){
				continue;
			}
			//タイトルの比較
			p = ((*(tpTreeInfo->ItemList + i))->Title == NULL) ? "" : (*(tpTreeInfo->ItemList + i))->Title;
			r = ((*(tpTmpItemList + j))->Title == NULL) ? "" : (*(tpTmpItemList + j))->Title;
			if(lstrcmp(p, r) != 0){
				continue;
			}
			//URLの比較
			p = ((*(tpTreeInfo->ItemList + i))->CheckURL == NULL) ? "" : (*(tpTreeInfo->ItemList + i))->CheckURL;
			r = ((*(tpTmpItemList + j))->CheckURL == NULL) ? "" : (*(tpTmpItemList + j))->CheckURL;
			if(lstrcmp(p, r) != 0){
				continue;
			}

			//待避
			*(tpItemList + NewItemCnt) = *(tpTreeInfo->ItemList + i);
			NewItemCnt++;
			*(tpTreeInfo->ItemList + i) = NULL;

			//作業用アイテムリストのアイテムへのポインタをクリア
			//（次回比較しないようにするため）
			*(tpTmpItemList + j) = NULL;
			break;
		}

		//アイテムの解放
		if(*(tpTreeInfo->ItemList + i) != NULL){
			if(hItem == TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE))){
				//現在表示されているフォルダの場合アイテムを削除する
				ListItemCnt = ListView_GetItemCount(GetDlgItem(hWnd, WWWC_LIST));
				for(n = 0;n < ListItemCnt;n++){
					tpItemInfo = (struct TPITEM *)ListView_GetlParam(GetDlgItem(hWnd, WWWC_LIST), n);
					if(tpItemInfo == *(tpTreeInfo->ItemList + i)){
						ListView_DeleteItem(GetDlgItem(hWnd, WWWC_LIST), n);
					}
				}
			}

			FreeItemInfo(*(tpTreeInfo->ItemList + i), TRUE);
			GlobalFree(*(tpTreeInfo->ItemList + i));
			*(tpTreeInfo->ItemList + i) = NULL;
		}
	}

	ListView_SetRedraw(GetDlgItem(hWnd, WWWC_LIST), LDF_REDRAW);

	//旧アイテムリストの解放
	GlobalFree(tpTreeInfo->ItemList);

	tpTreeInfo->ItemListCnt = NewItemCnt;
	//アイテムリストの確保
	tpTreeInfo->ItemList = (struct TPITEM **)GlobalAlloc(GPTR, sizeof(struct TPITEM *) * NewItemCnt);
	if(tpItemList == NULL){
		abort();
	}
	for(i = 0;i < NewItemCnt;i++){
		*(tpTreeInfo->ItemList + i) = *(tpItemList + i);
	}

	GlobalFree(tpItemList);
	GlobalFree(tpTmpItemList);

	//ファイルのアイテムリストを解放する
	FreeItemList(tpFileItemList, FileItemCnt, FALSE);
	GlobalFree(tpFileItemList);

	TreeView_FreeItem(hWnd, hItem, 1);
	return TRUE;
}


/******************************************************************************

	Item_ProtocolSelect

	プロトコルにマッチするアイテムの抽出

******************************************************************************/

struct TPITEM **Item_ProtocolSelect(struct TPITEM **tpItemList, int *cnt, char *protocol)
{
	struct TPITEM **RetItemList;
	struct TPITEM *tpItemInfo;
	int sCnt = *cnt;
	int ProtocolIndex;
	int i;

	*cnt = 0;
	for(i = 0; i < sCnt; i++){
		tpItemInfo = *(tpItemList + i);
		if(tpItemInfo == NULL){
			continue;
		}
		ProtocolIndex = GetProtocolIndex(tpItemInfo->CheckURL);
		if(strlistcmp((ProtocolIndex == -1) ? "" : tpProtocol[ProtocolIndex].title, protocol, ',') == FALSE){
			continue;
		}
		(*cnt)++;
	}
	//アイテムリストの確保
	RetItemList = (struct TPITEM **)GlobalAlloc(GPTR, sizeof(struct TPITEM *) * *cnt);
	if(RetItemList == NULL){
		return NULL;
	}
	//アイテムリストに選択アイテムを設定
	*cnt = 0;
	for(i = 0; i < sCnt; i++){
		tpItemInfo = *(tpItemList + i);
		if(tpItemInfo == NULL){
			continue;
		}
		ProtocolIndex = GetProtocolIndex(tpItemInfo->CheckURL);
		if(strlistcmp((ProtocolIndex == -1) ? "" : tpProtocol[ProtocolIndex].title, protocol, ',') == FALSE){
			continue;
		}
		*(RetItemList + *cnt) = tpItemInfo;
		(*cnt)++;
	}
	return RetItemList;
}


/******************************************************************************

	SetCopyItemInfo

	コピーアイテムの情報を表示

******************************************************************************/

static void SetCopyItemInfo(struct TPITEM *tpItemInfo, HWND IconWnd, HWND InfoWnd)
{
	HICON hIcon;
	char Size[BUFSIZE];
	char Date[BUFSIZE];
	char buf[BUFSIZE];
	int i, iIndex;

	if(tpItemInfo->CheckURL != NULL){
		//アイコンの設定
		i = GetProtocolIndex(tpItemInfo->CheckURL);
		if(tpItemInfo->Status & ST_ERROR){
			iIndex = ICON_ERR;

		}else if(tpItemInfo->Status & ST_TIMEOUT){
			iIndex = ICON_TIMEOUT;

		}else if(tpItemInfo->Status & ST_UP){
			iIndex = (i == -1) ? ICON_UP : (tpProtocol + i)->Icon + 3;

		}else{
			iIndex = (i == -1) ? ICON_NOICON : (tpProtocol + i)->Icon;
		}
		hIcon = ImageList_GetIcon(ListView_GetImageList(GetDlgItem(WWWCWnd, WWWC_LIST), LVSIL_NORMAL), iIndex, 0);
		SendMessage(IconWnd, STM_SETICON, (WPARAM)hIcon, 0);
	}
	//サイズ、更新日の設定
	*Size = '\0';
	if(tpItemInfo->Size != NULL){
		lstrcpyn(Size, tpItemInfo->Size, BUFSIZE - 1);
	}
	*Date = '\0';
	if(tpItemInfo->Date != NULL){
		lstrcpyn(Date, tpItemInfo->Date, BUFSIZE - 1);
	}
	wsprintf(buf, STR_UPDATEINFO, Size, Date);
	SetWindowText(InfoWnd, buf);
}


/******************************************************************************

	UpdateMsgProc

	コピーの上書き確認を行うウィンドウプロシージャ

******************************************************************************/

static BOOL CALLBACK UpdateMsgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HICON hIcon;
	char buf[BUFSIZE * 2];
	char FolderName[BUFSIZE];
	struct TPITEM **tpMsgItemList;

	switch (uMsg)
	{
	case WM_INITDIALOG:
		tpMsgItemList = (struct TPITEM **)lParam;
		//タイトル
		if((*tpMsgItemList)->Title != NULL && (*tpMsgItemList)->hItem != NULL){
			TreeView_GetItemInfo(GetDlgItem(WWWCWnd, WWWC_TREE), (*tpMsgItemList)->hItem, FolderName);
			wsprintf(buf, STR_UPDATEMSG, FolderName, (*tpMsgItemList)->Title);
			SetWindowText(GetDlgItem(hDlg, IDC_STATIC_COPYMSG_TITLE), buf);
		}

		if((*tpMsgItemList)->CheckURL != NULL){
			SendDlgItemMessage(hDlg, IDC_EDIT_COPYMSG_URL, WM_SETTEXT, 0, (LPARAM)(*tpMsgItemList)->CheckURL);
		}

		//コピー先
		SetCopyItemInfo(*tpMsgItemList, GetDlgItem(hDlg, IDC_STATIC_COPYMSG_TOICON),
			GetDlgItem(hDlg, IDC_STATIC_COPYMSG_TO));
		//コピー元
		SetCopyItemInfo(*(tpMsgItemList + 1), GetDlgItem(hDlg, IDC_STATIC_COPYMSG_FROMICON),
			GetDlgItem(hDlg, IDC_STATIC_COPYMSG_FROM));
		break;

	case WM_CLOSE:
		//アイコンの解放
		hIcon = (HICON)SendDlgItemMessage(hDlg, IDC_STATIC_COPYMSG_TOICON, STM_GETICON, 0, 0);
		DestroyIcon(hIcon);
		hIcon = (HICON)SendDlgItemMessage(hDlg, IDC_STATIC_COPYMSG_FROMICON, STM_GETICON, 0, 0);
		DestroyIcon(hIcon);

		EndDialog(hDlg, TRUE);
		break;

	case WM_COMMAND:
		switch(wParam)
		{
		case IDOK:
			UpdateItemFlag = UF_COPY;
			SendMessage(hDlg, WM_CLOSE, 0, 0);
			break;

		case ID_ALLYES:
			UpdateItemFlag = UF_ALLCOPY;
			SendMessage(hDlg, WM_CLOSE, 0, 0);
			break;

		case IDNO:
			UpdateItemFlag = UF_NOCOPY;
			SendMessage(hDlg, WM_CLOSE, 0, 0);
			break;

		case ID_ALLNO:
			UpdateItemFlag = UF_ALLNOCOPY;
			SendMessage(hDlg, WM_CLOSE, 0, 0);
			break;

		case IDCANCEL:
			UpdateItemFlag = UF_CANCEL;
			SendMessage(hDlg, WM_CLOSE, 0, 0);
			break;
		}
		break;

	default:
		return(FALSE);
	}
	return(TRUE);
}


/******************************************************************************

	UpdateItemInfo

	アイテムの上書き処理

******************************************************************************/

static int UpdateItemInfo(HWND hWnd, struct TPITEM *tpToItemInfo, struct TPITEM *tpFromItemInfo)
{
	#define MSGITEMCNT		2
	struct TPITEM **tpMsgItemList;

	//すべて上書き
	if(UpdateItemFlag == UF_ALLCOPY){
		return 1;
	}
	//すべて上書きしない
	if(UpdateItemFlag == UF_ALLNOCOPY){
		return -1;
	}

	tpMsgItemList = (struct TPITEM **)GlobalAlloc(GPTR, sizeof(struct TPITEM *) * MSGITEMCNT);
	if(tpMsgItemList == NULL){
		ErrMsg(hWnd, GetLastError(), NULL);
		UpdateItemFlag = UF_CANCEL;
		return -1;
	}
	*(tpMsgItemList) = tpToItemInfo;			//コピー先
	*(tpMsgItemList + 1) = tpFromItemInfo;		//コピー元

	WaitCursor(FALSE);
	//上書き確認ダイアログの表示
	DialogBoxParam(g_hinst, MAKEINTRESOURCE(IDD_DIALOG_COPYMSG), hWnd, UpdateMsgProc, (LPARAM)tpMsgItemList);
	WaitCursor(TRUE);

	GlobalFree(tpMsgItemList);

	//すべて上書きしない、キャンセル
	if(UpdateItemFlag == UF_ALLNOCOPY || UpdateItemFlag == UF_CANCEL){
		return -1;
	}
	//上書きしない
	if(UpdateItemFlag == UF_NOCOPY){
		UpdateItemFlag = UF_COPY;
		return -1;
	}
	return 1;
}


/******************************************************************************

	Item_Add

	アイテムリストにアイテム情報を追加

******************************************************************************/

int Item_Add(HWND hWnd, HTREEITEM hItem, struct TPITEM *NewItemInfo)
{
	struct TPTREE *tpTreeInfo;
	struct TPITEM **tpItemList;
	struct TPITEM *tpListViewItemInfo;
	char *p, *r;
	int ItemCnt, i;
	int ret = 0;
	int ListItemCnt;
	int j;

	//アイテムリスト取得
	tpTreeInfo = (struct TPTREE *)TreeView_GetlParam(GetDlgItem(hWnd, WWWC_TREE), hItem);
	if(tpTreeInfo == NULL){
		return -1;
	}
	//アイテム情報を読み込む
	if(tpTreeInfo->ItemList == NULL){
		if(ReadTreeMem(hWnd, hItem) == FALSE){
			return -1;
		}
	}

	ItemCnt = tpTreeInfo->ItemListCnt;

	//アイテムリストの確保
	tpItemList = (struct TPITEM **)GlobalAlloc(GPTR, sizeof(struct TPITEM *) * (ItemCnt + 1));
	if(tpItemList == NULL){
		return -1;
	}

	//アイテム情報のコピー
	for(i = 0; i < ItemCnt; i++){
		if(UpdateItemFlag == UF_NOMSG || ret == 1 || *(tpTreeInfo->ItemList + i) == NULL){
			*(tpItemList + i) = *(tpTreeInfo->ItemList + i);
			continue;
		}
		//タイトルの比較
		p = ((*(tpTreeInfo->ItemList + i))->Title == NULL) ? "" : (*(tpTreeInfo->ItemList + i))->Title;
		r = (NewItemInfo->Title == NULL) ? "" : NewItemInfo->Title;
		if(lstrcmp(p, r) != 0){
			*(tpItemList + i) = *(tpTreeInfo->ItemList + i);
			continue;
		}
		//URLの比較
		p = ((*(tpTreeInfo->ItemList + i))->CheckURL == NULL) ? "" : (*(tpTreeInfo->ItemList + i))->CheckURL;
		r = (NewItemInfo->CheckURL == NULL) ? "" : NewItemInfo->CheckURL;
		if(lstrcmp(p, r) != 0){
			*(tpItemList + i) = *(tpTreeInfo->ItemList + i);
			continue;
		}

		//上書き確認
		if(UpdateItemInfo(hWnd, *(tpTreeInfo->ItemList + i), NewItemInfo) == -1){
			GlobalFree(tpItemList);
			return -1;
		}

		//チェック中のアイテム
		if((*(tpTreeInfo->ItemList + i))->IconStatus == ST_CHECK){
			GlobalFree(tpItemList);
			UpdateItemFlag = UF_CANCEL;
			MessageBox(hWnd, EMSG_CHECKUPDATE, EMSG_CHECKUPDATE_TITLE, MB_ICONEXCLAMATION);
			return -1;
		}

		//現在表示されているフォルダの場合はアイテムに関連つけられたアイテム情報を変更する
		if(hItem == TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE))){
			ListItemCnt = ListView_GetItemCount(GetDlgItem(hWnd, WWWC_LIST));
			for(j = 0; j < ListItemCnt; j++){
				tpListViewItemInfo = (struct TPITEM *)ListView_GetlParam(GetDlgItem(hWnd, WWWC_LIST), j);
				if(tpListViewItemInfo == NULL){
					continue;
				}
				if(tpListViewItemInfo != *(tpTreeInfo->ItemList + i)){
					continue;
				}
				ListView_SetlParam(GetDlgItem(hWnd, WWWC_LIST), j, (long)NewItemInfo);
				//チェックする/しない設定
				ListView_SetItemState(GetDlgItem(hWnd, WWWC_LIST), j,
					((NewItemInfo->CheckSt == 1) ? INDEXTOOVERLAYMASK(ICON_ST_NOCHECK) : 0), LVIS_OVERLAYMASK);
				ListView_SetItemState(GetDlgItem(hWnd, WWWC_LIST), j, LVIS_SELECTED, LVIS_SELECTED);
				break;
			}
		}

		//アイテムの上書きを行う
		FreeItemInfo(*(tpTreeInfo->ItemList + i), TRUE);
		GlobalFree(*(tpTreeInfo->ItemList + i));
		*(tpItemList + i) = NewItemInfo;
		(*(tpItemList + i))->hItem = hItem;

		ret = 1;
	}
	if(ret == 0){
		//新規アイテムの追加
		*(tpItemList + i) = NewItemInfo;
		(*(tpItemList + i))->hItem = hItem;

		if(hItem == TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE))){
			ListView_InsertItemEx(GetDlgItem(hWnd, WWWC_LIST), (char*)LPSTR_TEXTCALLBACK, I_IMAGECALLBACK,
				(long)(*(tpItemList + i)), -1);
			//チェックする/しない設定
			ListView_SetItemState(GetDlgItem(hWnd, WWWC_LIST),
				ListView_GetItemCount(GetDlgItem(hWnd, WWWC_LIST)) - 1,
				(((*(tpItemList + i))->CheckSt == 1) ? INDEXTOOVERLAYMASK(ICON_ST_NOCHECK) : 0), LVIS_OVERLAYMASK);
			ListView_SetItemState(GetDlgItem(hWnd, WWWC_LIST),
				ListView_GetItemCount(GetDlgItem(hWnd, WWWC_LIST)) - 1, LVIS_SELECTED, LVIS_SELECTED);
		}
	}else{
		*(tpItemList + i) = NULL;
	}

	//旧アイテムリストの解放
	GlobalFree(tpTreeInfo->ItemList);
	tpTreeInfo->ItemList = tpItemList;
	tpTreeInfo->ItemListCnt = ItemCnt + 1;
	return ret;
}


/******************************************************************************

	Item_UrlAdd

	URLからアイテムを作成

******************************************************************************/

BOOL Item_UrlAdd(HWND hWnd, char *Title, char *buf, int NameFlag, HTREEITEM hItem)
{
	struct TPITEM *NewItemInfo;
	HTREEITEM InItem;
	char *url, *p, *r;
	int InsertIndex = -1;

	InItem = hItem;
	if(InItem == NULL){
		InItem = TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE));
	}

	url = (char *)GlobalAlloc(GMEM_FIXED, lstrlen(buf) + 1);
	if(url == NULL){
		return FALSE;
	}

	p = buf;
	while(*p != '\0'){
		for(; *p == '\r' || *p == '\n'; p++);
		if(*p == '\0'){
			break;
		}
		for(r = url; *p != '\0'; p++){
			if(IsDBCSLeadByte((BYTE)*p) == TRUE){
				//２バイトコードの場合
				*(r++) = *(p++);
				*(r++) = *p;
				continue;
			}
			if(*p == '\r' || *p == '\n') break;
			*(r++) = *p;
		}
		*r = '\0';

		//新しいアイテム情報の確保
		NewItemInfo = (struct TPITEM *)GlobalAlloc(GPTR, sizeof(struct TPITEM));
		if(NewItemInfo == NULL){
			return FALSE;
		}
		NewItemInfo->iSize = sizeof(struct TPITEM);
		NewItemInfo->hItem = InItem;

		//タイトルとURLを設定
		NewItemInfo->Title = (char *)GlobalAlloc(GMEM_FIXED, lstrlen(Title) + 1);
		if(NewItemInfo->Title == NULL){
			GlobalFree(NewItemInfo);
			GlobalFree(url);
			return FALSE;
		}
		lstrcpy(NewItemInfo->Title, Title);
		NewItemInfo->CheckURL = (char *)GlobalAlloc(GMEM_FIXED, lstrlen(url) + 1);
		if(NewItemInfo->CheckURL == NULL){
			FreeItemInfo(NewItemInfo, FALSE);
			GlobalFree(NewItemInfo);
			GlobalFree(url);
			return FALSE;
		}
		lstrcpy(NewItemInfo->CheckURL, url);
		NewItemInfo->CheckSt = DefNoCheck;

		//アイテムの追加
		UpdateItemFlag = UF_NOMSG;
		if(Item_Add(hWnd, InItem, NewItemInfo) == -1){
			FreeItemInfo(NewItemInfo, FALSE);
			GlobalFree(NewItemInfo);
			GlobalFree(url);
			return FALSE;
		}
		if(InsertIndex == -1){
			InsertIndex = ListView_GetItemCount(GetDlgItem(hWnd, WWWC_LIST)) - 1;
		}
	}
	GlobalFree(url);

	if(NameFlag == 0 && InsertIndex != -1){
		if(InsertIndex == ListView_GetItemCount(GetDlgItem(hWnd, WWWC_LIST)) - 1){
			SetFocus(GetDlgItem(hWnd, WWWC_LIST));
			//追加アイテムの選択
			ListView_SetItemState(GetDlgItem(hWnd, WWWC_LIST), InsertIndex, LVIS_SELECTED, LVIS_SELECTED);
			ListView_EnsureVisible(GetDlgItem(hWnd, WWWC_LIST), InsertIndex, TRUE);
			//名前の変更
			ListView_EditLabel(GetDlgItem(hWnd, WWWC_LIST), InsertIndex);
		}else{
			ListView_SetItemState(GetDlgItem(hWnd, WWWC_LIST), -1, 0, LVIS_SELECTED);
			ListView_SetItemState(GetDlgItem(hWnd, WWWC_LIST), InsertIndex,
				LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
			ListView_EnsureVisible(GetDlgItem(hWnd, WWWC_LIST), InsertIndex, TRUE);
			for(InsertIndex++; InsertIndex < ListView_GetItemCount(GetDlgItem(hWnd, WWWC_LIST)); InsertIndex++){
				ListView_SetItemState(GetDlgItem(hWnd, WWWC_LIST), InsertIndex, LVIS_SELECTED, LVIS_SELECTED);
			}
		}
	}
	TreeView_FreeItem(hWnd, InItem, 1);
	return TRUE;
}


/******************************************************************************

	Item_ListAdd

	アイテムリストに他のアイテムリストを追加

******************************************************************************/

BOOL Item_ListAdd(HWND hWnd, struct TPITEM **NewItemInfo, int cnt)
{
	int i;

	if(NewItemInfo == NULL || cnt == 0){
		return FALSE;
	}

	ListView_SetItemState(GetDlgItem(hWnd, WWWC_LIST), -1, 0, LVIS_SELECTED);

	UpdateItemFlag = UF_COPY;
	for(i = 0;i < cnt;i++){
		if((*(NewItemInfo + i)) == NULL){
			continue;
		}
		//アイテムの追加
		(*(NewItemInfo + i))->hItem = TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE));
		(*(NewItemInfo + i))->CheckSt = DefNoCheck;
		if(Item_Add(hWnd, TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)), *(NewItemInfo + i)) != -1){
			*(NewItemInfo + i) = NULL;
		}

		if(UpdateItemFlag == UF_CANCEL){
			break;
		}
	}
	FreeItemList(NewItemInfo, cnt, FALSE);

	//選択の先頭アイテムにフォーカスを与える
	if(ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), -1, LVNI_SELECTED) != -1){
		ListView_SetItemState(GetDlgItem(hWnd, WWWC_LIST),
			ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), -1, LVNI_SELECTED),
			LVIS_FOCUSED, LVIS_FOCUSED);

		ListView_EnsureVisible(GetDlgItem(hWnd, WWWC_LIST),
			ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), -1, LVNI_SELECTED), TRUE);
	}
	return TRUE;
}


/******************************************************************************

	Item_Delete

	アイテム情報の削除

******************************************************************************/

BOOL Item_Delete(struct TPTREE *tpTreeInfo, struct TPITEM *tpItemInfo)
{
	struct TPITEM **tmpItemInfo;
	int ItemCnt, i, j;

	//削除するアイテム情報を含まないリストを新規に作成
	ItemCnt = tpTreeInfo->ItemListCnt;
	tmpItemInfo = (struct TPITEM **)GlobalAlloc(GPTR, sizeof(struct TPITEM *) * (ItemCnt - 1));
	if(tmpItemInfo == NULL){
		return FALSE;
	}
	j = 0;
	for(i = 0;i < ItemCnt;i++){
		if(*(tpTreeInfo->ItemList + i) != NULL && *(tpTreeInfo->ItemList + i) == tpItemInfo){
			FreeItemInfo(*(tpTreeInfo->ItemList + i), TRUE);
			GlobalFree(*(tpTreeInfo->ItemList + i));
			*(tpTreeInfo->ItemList + i) = NULL;
		}else{
			*(tmpItemInfo + j) = *(tpTreeInfo->ItemList + i);
			j++;
		}
	}
	//アイテム情報の解放
	GlobalFree(tpTreeInfo->ItemList);

	tpTreeInfo->ItemList = tmpItemInfo;
	tpTreeInfo->ItemListCnt = ItemCnt - 1;
	return TRUE;
}


/******************************************************************************

	Item_SelectDelete

	リストビューで選択されているアイテムを削除

******************************************************************************/

static BOOL Item_SelectDelete(HWND hWnd)
{
	struct TPTREE *tpTreeInfo;
	struct TPITEM *tpItemInfo;
	HTREEITEM hItem;
	int SelectItem;

	tpTreeInfo = (struct TPTREE *)TreeView_GetlParam(GetDlgItem(hWnd, WWWC_TREE),
		TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)));
	if(tpTreeInfo == NULL){
		return FALSE;
	}
	WaitCursor(TRUE);
	//選択アイテムが存在している間ループ
	while((SelectItem = ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), -1, LVNI_SELECTED)) != -1){
		tpItemInfo = (struct TPITEM *)ListView_GetlParam(GetDlgItem(hWnd, WWWC_LIST), SelectItem);
		if(tpItemInfo == NULL){
			//フォルダの場合は下の階層まで削除する
			hItem = TreeView_GetIndexToItem(GetDlgItem(hWnd, WWWC_TREE),
				TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)), SelectItem);

			TreeView_DeleteTreeInfo(hWnd, hItem);
			ListView_DeleteItem(GetDlgItem(hWnd, WWWC_LIST), SelectItem);
		}else{
			//アイテム情報の削除
			Item_Delete(tpTreeInfo, tpItemInfo);
			ListView_DeleteItem(GetDlgItem(hWnd, WWWC_LIST), SelectItem);
		}
	}
	WaitCursor(FALSE);
	return TRUE;
}


/******************************************************************************

	Item_FlagDelete

	削除フラグの付いたアイテムの削除

******************************************************************************/

#if 0
BOOL Item_FlagDelete(struct TPTREE *tpTreeInfo)
{
	struct TPITEM **tmpItemInfo;
	int ItemCnt, i, j;

	ItemCnt = 0;
	for(i = 0; i < tpTreeInfo->ItemListCnt; i++){
		if(*(tpTreeInfo->ItemList + i) == NULL){
			continue;
		}
		if((*(tpTreeInfo->ItemList + i))->DeleteFlag == TRUE &&
			(*(tpTreeInfo->ItemList + i))->IconStatus != ST_CHECK){
			continue;
		}
		ItemCnt++;
	}
	if(tpTreeInfo->ItemListCnt == ItemCnt){
		return TRUE;
	}

	tmpItemInfo = (struct TPITEM **)GlobalAlloc(GPTR, sizeof(struct TPITEM *) * ItemCnt);
	if(tmpItemInfo == NULL){
		return FALSE;
	}
	j = 0;
	for(i = 0; i < tpTreeInfo->ItemListCnt; i++){
		if(*(tpTreeInfo->ItemList + i) == NULL){
			continue;
		}
		if((*(tpTreeInfo->ItemList + i))->DeleteFlag == TRUE &&
			(*(tpTreeInfo->ItemList + i))->IconStatus != ST_CHECK){
			FreeItemInfo(*(tpTreeInfo->ItemList + i), TRUE);
			GlobalFree(*(tpTreeInfo->ItemList + i));
			*(tpTreeInfo->ItemList + i) = NULL;
			continue;
		}
		(*(tpTreeInfo->ItemList + i))->DeleteFlag = FALSE;
		*(tmpItemInfo + j) = *(tpTreeInfo->ItemList + i);
		j++;
	}
	//アイテム情報の解放
	GlobalFree(tpTreeInfo->ItemList);

	tpTreeInfo->ItemList = tmpItemInfo;
	tpTreeInfo->ItemListCnt = ItemCnt;
	return TRUE;
}
#endif

/******************************************************************************

	Item_Create

	アイテムの新規作成

******************************************************************************/

BOOL Item_Create(HWND hWnd, int ProtocolIndex)
{
	FARPROC Func_Property;
	struct TPTREE *tpTreeInfo;
	struct TPITEM *NewItemInfo;
	HTREEITEM hItem;
	char buf[BUFSIZE];
	char *p;
	int ret;
	int i;

	if(tpProtocol[ProtocolIndex].lib == NULL){
		return FALSE;
	}
	wsprintf(buf, "%sProperty", tpProtocol[ProtocolIndex].FuncHeader);
	Func_Property = GetProcAddress((HMODULE)tpProtocol[ProtocolIndex].lib, buf);
	if(Func_Property == NULL){
		return FALSE;
	}
	hItem = TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE));
	tpTreeInfo = (struct TPTREE *)TreeView_GetlParam(GetDlgItem(hWnd, WWWC_TREE), hItem);
	if(tpTreeInfo == NULL){
		return FALSE;
	}
	if(tpTreeInfo->ItemList == NULL){
		return FALSE;
	}

	NewItemInfo = (struct TPITEM *)GlobalAlloc(GPTR, sizeof(struct TPITEM));
	if(NewItemInfo == NULL){
		return FALSE;
	}
	NewItemInfo->iSize = sizeof(struct TPITEM);
	NewItemInfo->hItem = hItem;
	NewItemInfo->CheckSt = DefNoCheck;

	//アイテムメモリを解放してしまわないようにメモリフラグをカウント
	tpTreeInfo->MemFlag++;
	//アイテムのプロパティを表示
	ret = Func_Property(hWnd, NewItemInfo);
	if(IsTreeItem(hWnd, hItem) == FALSE){
		FreeItemInfo(NewItemInfo, FALSE);
		GlobalFree(NewItemInfo);
		return FALSE;
	}
	tpTreeInfo->MemFlag--;

	if(ret == -1){
		GlobalFree(NewItemInfo);
		return FALSE;
	}

	if(NewItemInfo->Comment != NULL){
		//コメントの改行コードを変換
		p = CodeToEsc(NewItemInfo->Comment);
		GlobalFree(NewItemInfo->Comment);
		NewItemInfo->Comment = p;
	}

	//アイテムの追加
	UpdateItemFlag = UF_NOMSG;
	if(Item_Add(hWnd, hItem, NewItemInfo) == -1){
		FreeItemInfo(NewItemInfo, FALSE);
		GlobalFree(NewItemInfo);
		return FALSE;
	}

	if(hItem == TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE))){
		SetFocus(GetDlgItem(hWnd, WWWC_LIST));

		i = ListView_GetMemToIndex(GetDlgItem(hWnd, WWWC_LIST), NewItemInfo);
		ListView_SetItemState(GetDlgItem(hWnd, WWWC_LIST), i,
			((NewItemInfo->CheckSt == 1) ? INDEXTOOVERLAYMASK(ICON_ST_NOCHECK) : 0), LVIS_OVERLAYMASK);
		ListView_RedrawItems(GetDlgItem(hWnd, WWWC_LIST), i, i);
		UpdateWindow(GetDlgItem(hWnd, WWWC_LIST));

		ListView_SetItemState(GetDlgItem(hWnd, WWWC_LIST), i,
			LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		ListView_EnsureVisible(GetDlgItem(hWnd, WWWC_LIST), i, TRUE);
	}
	TreeView_FreeItem(hWnd, hItem, 1);
	return TRUE;
}


/******************************************************************************

	Item_Property

	アイテムのプロパティを表示

******************************************************************************/

BOOL Item_Property(HWND hWnd)
{
	struct TPTREE *tpTreeInfo;
	struct TPITEM *tpItemInfo;
	struct TPITEM *PropItemInfo;
	HTREEITEM hItem;
	char *p;
	int ret;
	int i;

	//ツリーにフォーカスがある場合はフォルダのプロパティを表示
	if(GetFocus() == GetDlgItem(hWnd, WWWC_TREE)){
		ViewFolderProperties(hWnd, HiTestItem);
		return TRUE;
	}
	//アイテムが選択されていない場合は現在のフォルダのプロパティを表示
	if(ListView_GetSelectedCount(GetDlgItem(hWnd, WWWC_LIST)) <= 0){
		ViewFolderProperties(hWnd, TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)));
		return TRUE;
	}

	hItem = TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE));
	tpTreeInfo = (struct TPTREE *)TreeView_GetlParam(GetDlgItem(hWnd, WWWC_TREE), hItem);
	if(tpTreeInfo == NULL){
		return FALSE;
	}

	i = ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), -1, LVNI_FOCUSED);
	if(ListView_GetItemState(GetDlgItem(hWnd, WWWC_LIST), i, LVIS_SELECTED) != LVIS_SELECTED){
		i = ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), -1, LVNI_SELECTED);
	}
	tpItemInfo = (struct TPITEM *)ListView_GetlParam(GetDlgItem(hWnd, WWWC_LIST), i);
	if(tpItemInfo == NULL){
		hItem = TreeView_GetIndexToItem(GetDlgItem(hWnd, WWWC_TREE),
			TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)), i);
		if(hItem == NULL){
			return FALSE;
		}
		//フォルダのプロパティ
		ViewFolderProperties(hWnd, hItem);
		return TRUE;
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
	if(IsTreeItem(hWnd, hItem) == FALSE){
		FreeItemInfo(PropItemInfo, FALSE);
		GlobalFree(PropItemInfo);
		return FALSE;
	}
	tpTreeInfo->MemFlag--;
	if(ret == FALSE){
		FreeItemInfo(PropItemInfo, FALSE);
		GlobalFree(PropItemInfo);
		TreeView_FreeItem(hWnd, hItem, 1);
		return FALSE;
	}

	if(PropItemInfo->Comment != NULL){
		//コメントの改行コードを変換
		p = CodeToEsc(PropItemInfo->Comment);
		GlobalFree(PropItemInfo->Comment);
		PropItemInfo->Comment = p;
	}

	//アイテムに反映
	for(i = 0; i < tpTreeInfo->ItemListCnt; i++){
		if(tpItemInfo == *(tpTreeInfo->ItemList + i)){
			PropItemInfo->IconStatus = tpItemInfo->IconStatus;
			PropItemInfo->Soc1 = tpItemInfo->Soc1;
			PropItemInfo->Soc2 = tpItemInfo->Soc2;
			PropItemInfo->hGetHost1 = tpItemInfo->hGetHost1;
			PropItemInfo->hGetHost2 = tpItemInfo->hGetHost2;
			PropItemInfo->Param1 = tpItemInfo->Param1;
			PropItemInfo->Param2 = tpItemInfo->Param2;
			PropItemInfo->Param3 = tpItemInfo->Param3;
			PropItemInfo->Param4 = tpItemInfo->Param4;
			PropItemInfo->user1 = tpItemInfo->user1;
			PropItemInfo->user2 = tpItemInfo->user2;
			PropItemInfo->RefreshFlag = tpItemInfo->RefreshFlag;

			FreeItemInfo(tpItemInfo, TRUE);
			CopyMemory(tpItemInfo, PropItemInfo, sizeof(struct TPITEM));
			break;
		}
	}
	if(i >= tpTreeInfo->ItemListCnt){
		FreeItemInfo(PropItemInfo, FALSE);
		GlobalFree(PropItemInfo);
		TreeView_FreeItem(hWnd, hItem, 1);
		MessageBox(hWnd, EMSG_NOPROPITEM, EMSG_PROP_TITLE, MB_ICONSTOP);
		return FALSE;
	}

	//リストビューの更新
	if(hItem == TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE))){
		i = ListView_GetMemToIndex(GetDlgItem(hWnd, WWWC_LIST), tpItemInfo);
		ListView_SetItemText(GetDlgItem(hWnd, WWWC_LIST), i, 0, LPSTR_TEXTCALLBACK);
		ListView_SetItemState(GetDlgItem(hWnd, WWWC_LIST), i,
			((tpItemInfo->CheckSt == 1) ? INDEXTOOVERLAYMASK(ICON_ST_NOCHECK) : 0), LVIS_OVERLAYMASK);
		ListView_RedrawItems(GetDlgItem(hWnd, WWWC_LIST), i, i);
		UpdateWindow(GetDlgItem(hWnd, WWWC_LIST));
	}
	GlobalFree(PropItemInfo);
	TreeView_FreeItem(hWnd, hItem, 1);
	return ret;
}


/******************************************************************************

	Item_UpCount

	フォルダ内のUPアイコンの数の取得

******************************************************************************/

int Item_UpCount(HWND hWnd, HTREEITEM hItem)
{
	struct TPTREE *tpTreeInfo;
	int i, ret = 0;

	tpTreeInfo = (struct TPTREE *)TreeView_GetlParam(GetDlgItem(hWnd, WWWC_TREE), hItem);
	if(tpTreeInfo == NULL){
		return 0;
	}

	//アイテムリストの取得
	if(tpTreeInfo->ItemList == NULL){
		ReadTreeMem(hWnd, hItem);
	}
	if(tpTreeInfo->ItemList == NULL){
		return 0;
	}
	//フォルダ内のUPアイテムの数をカウント
	for(i = 0;i < tpTreeInfo->ItemListCnt;i++){
		if((*(tpTreeInfo->ItemList + i)) == NULL){
			continue;
		}
		if((*(tpTreeInfo->ItemList + i))->Status == ST_UP){
			ret++;
		}
	}
	TreeView_FreeItem(hWnd, hItem, 0);
	return ret;
}


/******************************************************************************

	Item_Initialize

	アイコンの初期化

******************************************************************************/

int Item_Initialize(HWND hWnd, struct TPITEM *tpItemInfo, BOOL InitIconFlag)
{
	char buf[BUFSIZE];
	int ProtocolIndex;
	int ret = -1;

	if(tpItemInfo == NULL || tpItemInfo->Status == ST_DEFAULT){
		return -1;
	}
	if(InitIconFlag == TRUE || tpItemInfo->Status == ST_UP){
		//初期化
		tpItemInfo->Status = ST_DEFAULT;
	}else{
		//エラーの状態を除去
		if(tpItemInfo->Status & ST_ERROR) tpItemInfo->Status ^= ST_ERROR;
		if(tpItemInfo->Status & ST_TIMEOUT) tpItemInfo->Status ^= ST_TIMEOUT;
	}

	ProtocolIndex = GetProtocolIndex(tpItemInfo->CheckURL);
	if(ProtocolIndex == -1){
		return -1;
	}
	if(tpProtocol[ProtocolIndex].lib == NULL){
		return -1;
	}
	if(tpProtocol[ProtocolIndex].Func_InitItem == NULL){
		//プロトコルDLLのアイテム初期化関数を呼ぶ
		wsprintf(buf, "%sInitItem", tpProtocol[ProtocolIndex].FuncHeader);
		tpProtocol[ProtocolIndex].Func_InitItem = GetProcAddress((HMODULE)tpProtocol[ProtocolIndex].lib, buf);
	}
	if(tpProtocol[ProtocolIndex].Func_InitItem != NULL){
		ret = tpProtocol[ProtocolIndex].Func_InitItem(hWnd, tpItemInfo);
	}
	return ret;
}


/******************************************************************************

	ListItemIni

	リストビューの選択アイテムのアイコンの初期化

******************************************************************************/

void ListItemIni(HWND hWnd)
{
	struct TPITEM *tpItemInfo;
	HTREEITEM hItem;
	int i;

	hItem = TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE));

	if(ListView_GetSelectedCount(GetDlgItem(hWnd, WWWC_LIST)) == 0){
		TreeItemIni(hWnd, hItem, 0);
		return;
	}

	//選択アイテムを初期化
	i = -1;
	while((i = ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), i, LVNI_SELECTED)) != -1){
		tpItemInfo = (struct TPITEM *)ListView_GetlParam(GetDlgItem(hWnd, WWWC_LIST), i);
		if(tpItemInfo == NULL){
			//フォルダの場合は再帰する
			TreeItemIni(hWnd, TreeView_GetIndexToItem(GetDlgItem(hWnd, WWWC_TREE), hItem, i), 0);
			TreeView_SetIconState(hWnd, TreeView_GetIndexToItem(GetDlgItem(hWnd, WWWC_TREE), hItem, i), 0);
			TreeView_FreeItem(hWnd, TreeView_GetIndexToItem(GetDlgItem(hWnd, WWWC_TREE), hItem, i), 1);
		}else{
			//アイコンを初期化
			Item_Initialize(hWnd, tpItemInfo, FALSE);
		}
	}
	TreeView_SetIconState(hWnd, hItem, 0);
}


/******************************************************************************

	Item_ItemListIni

	アイテムリストの全アイテムを初期化

******************************************************************************/

void Item_ItemListIni(HWND hWnd, struct TPITEM **tpItemList, int cnt)
{
	int i;

	for(i = 0; i < cnt; i++){
		if((*(tpItemList + i)) != NULL){
			//アイコンの初期化
			Item_Initialize(hWnd, (*(tpItemList + i)), FALSE);
		}
	}
}


/******************************************************************************

	Item_MainItemIni

	アイテムリストから本体のアイテムを検索して初期化

******************************************************************************/

void Item_MainItemIni(HWND hMainWnd, HWND hWnd, struct TPITEM **tpItemList, int cnt)
{
	struct TPITEM *tpItemInfo;
	int i;

	for(i = 0; i < cnt; i++){
		tpItemInfo = FindMainItem(hMainWnd, (*(tpItemList + i)));
		if(tpItemInfo != NULL){
			//アイコンの初期化
			Item_Initialize(hMainWnd, tpItemInfo, FALSE);
			TreeView_SetIconState(hMainWnd, (*(tpItemList + i))->hItem, 0);
		}
		Item_Initialize(hWnd, (*(tpItemList + i)), FALSE);
	}
}


/******************************************************************************

	TreeItemIni

	フォルダ内の全てのアイテムのアイコンの初期化

******************************************************************************/

void CALLBACK TreeItemIni(HWND hWnd, HTREEITEM hItem, long Param)
{
	struct TPTREE *tpTreeInfo;
	int i;

	tpTreeInfo = (struct TPTREE *)TreeView_GetlParam(GetDlgItem(hWnd, WWWC_TREE), hItem);
	if(tpTreeInfo == NULL){
		return;
	}

	//アイテムリストの取得
	if(tpTreeInfo->ItemList == NULL){
		ReadTreeMem(hWnd, hItem);
	}
	if(tpTreeInfo->ItemList == NULL){
		return;
	}
	//フォルダ内の全てのアイテムを初期化
	for(i = 0;i < tpTreeInfo->ItemListCnt;i++){
		if((*(tpTreeInfo->ItemList + i)) == NULL){
			continue;
		}
		Item_Initialize(hWnd, (*(tpTreeInfo->ItemList + i)), FALSE);
	}

	TreeView_SetIconState(hWnd, hItem, 0);
	TreeView_FreeItem(hWnd, hItem, 1);
}


/******************************************************************************

	Item_ChangeState

	アイテムの状態の変更

******************************************************************************/

BOOL Item_ChangeState(HWND hWnd, int Status)
{
	struct TPITEM *tpItemInfo;
	int i;

	if(GetFocus() == GetDlgItem(hWnd, WWWC_TREE)){
		return FALSE;
	}

	i = -1;
	while((i = ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), i, LVNI_SELECTED)) != -1){
		tpItemInfo = (struct TPITEM *)ListView_GetlParam(GetDlgItem(hWnd, WWWC_LIST), i);
		if(tpItemInfo != NULL){
			//アイテムの状態追加
			tpItemInfo->Status |= Status;
			ListView_RedrawItems(GetDlgItem(hWnd, WWWC_LIST), i, i);
		}
	}
	//表示更新
	UpdateWindow(GetDlgItem(hWnd, WWWC_LIST));
	TreeView_SetIconState(hWnd, TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)), 0);
	TreeView_FreeItem(hWnd, TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)), 1);
	SetSbText(hWnd);
	return TRUE;
}


/******************************************************************************

	Item_SwitchCheckState

	アイテム／フォルダのチェックフラグの切り替え

******************************************************************************/

BOOL Item_SwitchCheckState(HWND hWnd, HTREEITEM hItem)
{
	struct TPTREE *tpTreeInfo;
	struct TPITEM *tpItemInfo;
	int i;
	HTREEITEM cItem;
	char buf[BUFSIZE];
	UINT status;

	if(GetFocus() == GetDlgItem(hWnd, WWWC_TREE)){
		tpTreeInfo = (struct TPTREE *)TreeView_GetlParam(GetDlgItem(hWnd, WWWC_TREE), hItem);
		if(tpTreeInfo == NULL){
			return FALSE;
		}

		//切り替え
		tpTreeInfo->CheckSt = (tpTreeInfo->CheckSt == 0) ? 1 : 0;
		//状態変更
		status = (tpTreeInfo->CheckSt == 1) ? INDEXTOOVERLAYMASK(ICON_ST_NOCHECK) : 0;
		TreeView_SetState(GetDlgItem(hWnd, WWWC_TREE), hItem, status, TVIS_OVERLAYMASK);
		if(TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)) ==
			TreeView_GetParent(GetDlgItem(hWnd, WWWC_TREE), hItem)){
			//リストビューに反映
			ListView_SetItemState(GetDlgItem(hWnd, WWWC_LIST),
				TreeView_GetListIndex(GetDlgItem(hWnd, WWWC_TREE), hItem), status, LVIS_OVERLAYMASK);
		}

		//フォルダ情報の保存
		TreeView_GetPath(GetDlgItem(hWnd, WWWC_TREE), hItem, buf, CuDir);
		PutDirInfo(GetDlgItem(hWnd, WWWC_TREE), buf, hItem);
		return TRUE;
	}

	i = -1;
	while((i = ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), i, LVNI_SELECTED)) != -1){
		tpItemInfo = (struct TPITEM *)ListView_GetlParam(GetDlgItem(hWnd, WWWC_LIST), i);
		if(tpItemInfo == NULL){
			cItem = TreeView_GetIndexToItem(GetDlgItem(hWnd, WWWC_TREE),
				TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)), i);
			tpTreeInfo = (struct TPTREE *)TreeView_GetlParam(GetDlgItem(hWnd, WWWC_TREE), cItem);
			if(tpTreeInfo == NULL){
				return FALSE;
			}

			//切り替え
			tpTreeInfo->CheckSt = (tpTreeInfo->CheckSt == 0) ? 1 : 0;
			//状態変更
			status = (tpTreeInfo->CheckSt == 1) ? INDEXTOOVERLAYMASK(ICON_ST_NOCHECK) : 0;
			TreeView_SetState(GetDlgItem(hWnd, WWWC_TREE), cItem, status, TVIS_OVERLAYMASK);
			ListView_SetItemState(GetDlgItem(hWnd, WWWC_LIST), i, status, LVIS_OVERLAYMASK);

			//フォルダ情報の保存
			TreeView_GetPath(GetDlgItem(hWnd, WWWC_TREE), cItem, buf, CuDir);
			PutDirInfo(GetDlgItem(hWnd, WWWC_TREE), buf, cItem);

		}else{
			//切り替え
			tpItemInfo->CheckSt = (tpItemInfo->CheckSt == 0) ? 1 : 0;
			//状態変更
			status = (tpItemInfo->CheckSt == 1) ? INDEXTOOVERLAYMASK(ICON_ST_NOCHECK) : 0;
			ListView_SetItemState(GetDlgItem(hWnd, WWWC_LIST), i, status, LVIS_OVERLAYMASK);
		}
	}
	return TRUE;
}


/******************************************************************************

	FolderDelete

	フォルダの削除

******************************************************************************/

static BOOL FolderDelete(HWND hWnd)
{
	HTREEITEM hItem, pItem;
	char tmp[BUFSIZE];
	char msg[BUFSIZE * 2];
	BOOL ShiftFlag = FALSE;
	HANDLE hMem;

	if(GetAsyncKeyState(VK_SHIFT) < 0){
		//シフトキーが押されている
		ShiftFlag = TRUE;
	}

	hItem = HiTestItem;
	if(hItem == RootItem || hItem == RecyclerItem){
		return FALSE;
	}

	//確認メッセージ
	TreeView_GetItemInfo(GetDlgItem(hWnd, WWWC_TREE), hItem, tmp);
	wsprintf(msg, QMSG_FOLDERDELETE, tmp);
	if(MessageBox(hWnd, msg, QMSG_DELETE_TITLE, MB_ICONEXCLAMATION | MB_YESNO) == IDNO){
		return FALSE;
	}

	if(FindCheckItem(hWnd, hItem) == 1){
		MessageBox(hWnd, EMSG_FOLDERCHECK, EMSG_FOLDERDELETE_TITLE, MB_ICONEXCLAMATION);
		return FALSE;
	}
	if(FindPropItem(hWnd, hItem) == 1){
		MessageBox(hWnd, EMSG_FOLDERPROP, EMSG_FOLDERDELETE_TITLE, MB_ICONEXCLAMATION);
		return FALSE;
	}

	//フォルダ情報の保存
	SaveTreeMem(hWnd, GetDlgItem(hWnd, WWWC_TREE), hItem);

	pItem = TreeView_GetParent(GetDlgItem(hWnd, WWWC_TREE), hItem);
	if(ShiftFlag == FALSE && TreeView_IsRecyclerItem(GetDlgItem(hWnd, WWWC_TREE), hItem) == FALSE){
		//ごみ箱に移動
		hMem = Clipboard_Set_WF_ItemList(hWnd, FLAG_CUT, hItem);
		if(hMem == NULL){
			MessageBox(hWnd, EMSG_FOLDERNODELETE, EMSG_FOLDERDELETE_TITLE, MB_ICONEXCLAMATION);
			return FALSE;
		}
		UpdateItemFlag = UF_NOMSG;

		if(DragDrop_GetDropItem(hWnd, hMem, DND_RECY, RecyclerItem) != FALSE){
			TreeView_NoFileDelete(hWnd, hItem, 0);
			ListView_RefreshFolder(hWnd);
		}
		GlobalFree(hMem);

	}else{
		//削除
		TreeView_DeleteTreeInfo(hWnd, hItem);
	}
	TreeView_SetIconState(hWnd, pItem, 0);
	TreeView_SetIconState(hWnd, RecyclerItem, 0);
	return TRUE;
}


/******************************************************************************

	DeleteItem

	アイテムの削除

******************************************************************************/

BOOL DeleteItem(HWND hWnd)
{
	struct TPTREE *tpTreeInfo;
	struct TPITEM *tpItemInfo;
	HTREEITEM hItem;
	int SelectItem;
	char tmp[BUFSIZE];
	char msg[BUFSIZE * 2];
	BOOL ShiftFlag = FALSE;
	BOOL CheckFlag = FALSE;
	HANDLE hMem;

	//ツリーにフォーカスがある場合はフォルダの削除
	if(GetFocus() == GetDlgItem(hWnd, WWWC_TREE)){
		return FolderDelete(hWnd);
	}

	if(GetAsyncKeyState(VK_SHIFT) < 0){
		//シフトキーが押されている場合
		ShiftFlag = TRUE;
	}

	if(ListView_GetSelectedCount(GetDlgItem(hWnd, WWWC_LIST)) <= 0){
		return FALSE;
	}

	tpTreeInfo = (struct TPTREE *)TreeView_GetlParam(GetDlgItem(hWnd, WWWC_TREE),
		TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)));
	if(tpTreeInfo == NULL){
		return FALSE;
	}

	if(ListView_GetSelectedCount(GetDlgItem(hWnd, WWWC_LIST)) == 1){
		//一件選択の場合は詳細な確認メッセージを表示
		tpItemInfo = (struct TPITEM *)ListView_GetlParam(GetDlgItem(hWnd, WWWC_LIST),
			ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), -1, LVNI_SELECTED));
		if(tpItemInfo == NULL){
			ListView_GetItemText(GetDlgItem(hWnd, WWWC_LIST),
				ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), -1, LVNI_SELECTED),
				0, tmp, BUFSIZE - 1);
			wsprintf(msg, QMSG_FOLDERDELETE, tmp);
		}else{
			wsprintf(msg, QMSG_ITEMDELETE, tpItemInfo->Title);
		}
		if(MessageBox(hWnd, msg, QMSG_DELETE_TITLE, MB_ICONEXCLAMATION | MB_YESNO) == IDNO){
			return FALSE;
		}
	}else{
		//確認メッセージ
		wsprintf(msg, QMSG_MANYITEMDELETE,
			ListView_GetSelectedCount(GetDlgItem(hWnd, WWWC_LIST)));
		if(MessageBox(hWnd, msg, QMSG_DELETE_TITLE, MB_ICONEXCLAMATION | MB_YESNO) == IDNO){
			return FALSE;
		}
	}

	//チェック中のアイテムがある場合はエラーを表示
	SelectItem = -1;
	while((SelectItem = ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), SelectItem, LVNI_SELECTED)) != -1){
		tpItemInfo = (struct TPITEM *)ListView_GetlParam(GetDlgItem(hWnd, WWWC_LIST), SelectItem);
		if(tpItemInfo == NULL){
			hItem = TreeView_GetIndexToItem(GetDlgItem(hWnd, WWWC_TREE),
				TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)), SelectItem);
			if(FindCheckItem(hWnd, hItem) == 1){
				CheckFlag = TRUE;
				break;
			}
			if(FindPropItem(hWnd, hItem) == 1){
				MessageBox(hWnd, EMSG_FOLDERPROP, EMSG_FOLDERDELETE_TITLE, MB_ICONEXCLAMATION);
				return FALSE;
			}
		}else{
			if(tpItemInfo->IconStatus == ST_CHECK){
				CheckFlag = TRUE;
				break;
			}
		}
	}
	if(CheckFlag == TRUE){
		MessageBox(hWnd, EMSG_ITEMCHECK, EMSG_ITEMDELETE_TITLE, MB_ICONEXCLAMATION);
		return FALSE;
	}

	WaitCursor(TRUE);
	//フォルダ情報の保存
	SaveTreeMem(hWnd, GetDlgItem(hWnd, WWWC_TREE), TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)));

	if(ShiftFlag == FALSE && TreeView_IsRecyclerItem(GetDlgItem(hWnd, WWWC_TREE),
		TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE))) == FALSE){
		//ごみ箱に移動するアイテムを文字列にする
		hMem = Clipboard_Set_WF_ItemList(hWnd, FLAG_CUT, NULL);
		if(hMem == NULL){
			WaitCursor(FALSE);
			MessageBox(hWnd, EMSG_ITEMNODELETE, EMSG_ITEMDELETE_TITLE, MB_ICONEXCLAMATION);
			return FALSE;
		}
		UpdateItemFlag = UF_NOMSG;

		//ごみ箱に移動する
		if(DragDrop_GetDropItem(hWnd, hMem, DND_RECY, RecyclerItem) != FALSE){
			TreeView_NoFileDelete(hWnd, TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)), 0);
			ListView_RefreshFolder(hWnd);
			Item_Select(hWnd, TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)));
		}
		GlobalFree(hMem);
	}else{
		//削除
		Item_SelectDelete(hWnd);
	}
	TreeView_SetIconState(hWnd, TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)), 0);
	TreeView_SetIconState(hWnd, RecyclerItem, 0);
	WaitCursor(FALSE);
	return TRUE;
}


/******************************************************************************

	CheckRecycler

	ごみ箱の状態をチェック

******************************************************************************/

BOOL CheckRecycler(HWND hWnd)
{
	struct TPTREE *tpTreeInfo;
	char buf[BUFSIZE];
	char FileName[BUFSIZE];
	int i;

	if(TreeView_GetChild(GetDlgItem(hWnd, WWWC_TREE), RecyclerItem) != NULL){
		return TRUE;
	}

	tpTreeInfo = (struct TPTREE *)TreeView_GetlParam(GetDlgItem(hWnd, WWWC_TREE), RecyclerItem);
	if(tpTreeInfo == NULL){
		return FALSE;
	}
	if(tpTreeInfo->ItemList != NULL){
		for(i = 0;i < tpTreeInfo->ItemListCnt;i++){
			if((*(tpTreeInfo->ItemList + i)) == NULL){
				continue;
			}
			//ごみ箱にアイテムが存在する
			return TRUE;
		}
	}

	TreeView_GetPath(GetDlgItem(hWnd, WWWC_TREE), RecyclerItem, buf, CuDir);
	wsprintf(FileName, "%s\\"DATAFILENAME, buf);
	if (GetFileSerchSize(FileName) > 0) {
		//ごみ箱にアイテムが存在する
		return TRUE;
	}
	return FALSE;
}


/******************************************************************************

	ClearRecycler

	ごみ箱を空にする

******************************************************************************/

BOOL ClearRecycler(HWND hWnd, BOOL Flag)
{
	struct TPTREE *tpTreeInfo;
	char buf[BUFSIZE];
	int ListIndex;

	//ごみ箱のチェック
	if(CheckRecycler(hWnd) == FALSE){
		return FALSE;
	}

	//確認メッセージ
	if(Flag == FALSE && MessageBox(hWnd, QMSG_CLEARRECYCLER, QMSG_CLEARRECYCLER_TITLE,
		MB_ICONEXCLAMATION | MB_YESNO) == IDNO){
		return FALSE;
	}

	if(FindCheckItem(hWnd, RecyclerItem) == 1){
		if(Flag == FALSE){
			MessageBox(hWnd, EMSG_CLEARRECYCLER, EMSG_CLEARRECYCLER_TITLE, MB_ICONEXCLAMATION);
		}
		return FALSE;
	}
	if(FindPropItem(hWnd, RecyclerItem) == 1){
		if(Flag == FALSE){
			MessageBox(hWnd, EMSG_PROPRECYCLER, EMSG_CLEARRECYCLER_TITLE, MB_ICONEXCLAMATION);
		}
		return FALSE;
	}

	//現在選択されているフォルダがごみ箱の中の場合はごみ箱を選択する
	if(TreeView_IsRecyclerItem(GetDlgItem(hWnd, WWWC_TREE), TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE))) == TRUE){
		TreeView_SelectItem(GetDlgItem(hWnd, WWWC_TREE), RecyclerItem);
	}

	//ごみ箱をフォルダごと削除
	TreeView_GetPath(GetDlgItem(hWnd, WWWC_TREE), RecyclerItem, buf, CuDir);
	DeleteDirTree(buf, FALSE);
	RemoveDirectory(buf);
	CallTreeItem(hWnd, RecyclerItem, (FARPROC)TreeView_FreeTreeItem, 1);

	while(TreeView_GetChild(GetDlgItem(hWnd, WWWC_TREE), RecyclerItem) != NULL){
		TreeView_DeleteItem(GetDlgItem(hWnd, WWWC_TREE),
			TreeView_GetChild(GetDlgItem(hWnd, WWWC_TREE), RecyclerItem));
	}

	//ごみ箱の作成
	tpTreeInfo = (struct TPTREE *)GlobalAlloc(GPTR, sizeof(struct TPTREE));
	if(tpTreeInfo == NULL){
		ErrMsg(hWnd, GetLastError(), NULL);
		return FALSE;
	}
	tpTreeInfo->ItemList = NULL;
	tpTreeInfo->ItemListCnt = 0;
	tpTreeInfo->CheckFlag = 0;
	tpTreeInfo->MemFlag = 0;
	tpTreeInfo->Icon = 0;
	tpTreeInfo->CheckSt = 0;
	tpTreeInfo->AutoCheckSt = 1;
	tpTreeInfo->Comment = NULL;
	TreeView_SetlParam(GetDlgItem(hWnd, WWWC_TREE), RecyclerItem, (long)tpTreeInfo);

	TreeView_SetIconState(hWnd, RecyclerItem, 0);

	//表示更新
	if(TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)) == RootItem){
		ListIndex = TreeView_GetListIndex(GetDlgItem(hWnd, WWWC_TREE), RecyclerItem);
		ListView_RedrawItems(GetDlgItem(hWnd, WWWC_LIST), ListIndex, ListIndex);
		UpdateWindow(GetDlgItem(hWnd, WWWC_LIST));

	}else if(TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)) == RecyclerItem){
		ListView_DeleteAllItems(GetDlgItem(hWnd, WWWC_LIST));
		InvalidateRect(GetDlgItem(hWnd, WWWC_LIST), NULL, TRUE);
		UpdateWindow(GetDlgItem(hWnd, WWWC_LIST));
	}
	return TRUE;
}
/* End of source */
