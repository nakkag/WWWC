/**************************************************************************

	WWWC

	Check.c

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

#define TITLE_TIMEFORMAT	"[%s %s]"


/**************************************************************************
	Global Variables
**************************************************************************/

struct TPITEM **CheckItemList;
static int gCheckType = 0;
BOOL ErrCheckFlag = FALSE;
static int UPcnt;

//外部参照
extern HTREEITEM RootItem;
extern HWND UpWnd;
extern HWND AniWnd;
extern HMENU hPOPUP;
extern int gCheckFlag;
extern HICON TrayIcon_Main;
extern HICON TrayIcon_Chaeck;
extern HICON TrayIcon_Up;
extern HICON TrayIcon_Main_Win;
extern HICON TrayIcon_Chaeck_Win;
extern HICON StCheckIcon;
extern BOOL UpIconFlag;
extern BOOL CmdCheckEnd;
extern BOOL CmdNoUpCheckEnd;

extern struct TPITEM **UpItemList;
extern int UpItemListCnt;
extern struct TP_PROTOCOL *tpProtocol;
extern int ProtocolCnt;
extern struct TP_TOOLS *ToolList;
extern int ToolListCnt;

extern int TrayIcon;
extern char WinTitle[];
extern int CheckUPItemClear;
extern int ClearTime;
extern int UPSnd;
extern int NoUpMsg;
extern char WaveFile[];
extern int CheckMax;
extern int ReturnIcon;

extern struct TPLVCOLUMN *ColumnInfo;
extern struct TPLVCOLUMN *SortColInfo;
extern int LvSortFlag;
extern int LvAutoSort;
extern int LvCheckEndAutoSort;


/**************************************************************************
	Local Function Prototypes
**************************************************************************/

static int GetHandleToIndex(HANDLE hHnd);
static int GetSocketToIndex(int Soc);
static int GetItemInfoToIndex(struct TPITEM *tpItemInfo);
static int ListCheckCount(HWND hWnd);
static int TreeCheckCount(HWND hWnd, HTREEITEM hItem, int CheckFlag);
static void ListCheckIni(HWND hWnd, BOOL TreeFlag);
static void CALLBACK TreeCheckIni(HWND hWnd, HTREEITEM hItem, long Param);
static void CALLBACK FindCheckTree(HWND hWnd, HTREEITEM hItem, long Param);
static void CALLBACK FindCheckNoCheckTree(HWND hWnd, HTREEITEM hItem, long Param);
static void SetCheckDateTime(struct TPITEM *tpItemInfo);
static int CheckStartItem(HWND hWnd, struct TPITEM *tpItemInfo, int ProtocolIndex, int CheckListIndex);
static void CALLBACK CheckStartFolder(HWND hWnd, HTREEITEM hItem, long Param);
static void CancelItem(HWND hWnd, struct TPITEM *tpItemInfo);
static void NotifyItemCheckEnd(HWND hWnd, struct TPITEM *tpItemInfo);


/******************************************************************************

	GetHandleToIndex

	ホスト情報ハンドルからアイテムを検索

******************************************************************************/

static int GetHandleToIndex(HANDLE hHnd)
{
	int i;

	for(i = 0;i < CheckMax;i++){
		if(*(CheckItemList + i) == NULL){
			continue;
		}
		if((*(CheckItemList + i))->hGetHost1 == hHnd || (*(CheckItemList + i))->hGetHost2 == hHnd){
			return i;
		}
	}
	return -1;
}


/******************************************************************************

	GetSocketToIndex

	ソケットからアイテムを検索

******************************************************************************/

static int GetSocketToIndex(int Soc)
{
	int i;

	for(i = 0;i < CheckMax;i++){
		if(*(CheckItemList + i) == NULL){
			continue;
		}
		if((*(CheckItemList + i))->Soc1 == Soc || (*(CheckItemList + i))->Soc2 == Soc){
			return i;
		}
	}
	return -1;
}


/******************************************************************************

	GetItemInfoToIndex

	チェックアイテム情報からアイテムを検索

******************************************************************************/

static int GetItemInfoToIndex(struct TPITEM *tpItemInfo)
{
	int i;

	for(i = 0;i < CheckMax;i++){
		if(*(CheckItemList + i) == NULL){
			continue;
		}
		if((*(CheckItemList + i)) == tpItemInfo){
			return i;
		}
	}
	return -1;
}


/******************************************************************************

	ListCheckCount

	リストのチェック数の取得

******************************************************************************/

static int ListCheckCount(HWND hWnd)
{
	struct TPTREE *tpTreeInfo;
	struct TPITEM *tpItemInfo;
	HTREEITEM hItem;
	int cnt = 0;
	int i;

	//ツリー情報を取得
	tpTreeInfo = (struct TPTREE *)TreeView_GetlParam(GetDlgItem(hWnd, WWWC_TREE),
		TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)));
	if(tpTreeInfo == NULL || tpTreeInfo->ItemList == NULL){
		return 0;
	}

	i = -1;
	while((i = ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), i, LVNI_SELECTED)) != -1){
		tpItemInfo = (struct TPITEM *)ListView_GetlParam(GetDlgItem(hWnd, WWWC_LIST), i);
		if(tpItemInfo == NULL){
			hItem = TreeView_GetIndexToItem(GetDlgItem(hWnd, WWWC_TREE), TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)), i);
			cnt += TreeCheckCount(hWnd, hItem, CHECKINI_CHECK);
		}else{
			if(tpItemInfo->IconStatus == ST_DEFAULT){
				cnt++;
			}
		}
	}
	return cnt;
}


/******************************************************************************

	TreeCheckCount

	チェックするアイテム数の取得

******************************************************************************/

static int TreeCheckCount(HWND hWnd, HTREEITEM hItem, int CheckFlag)
{
	struct TPTREE *tpTreeInfo;
	int cnt = 0;
	int i;

	//ツリー情報を取得
	if((tpTreeInfo = (struct TPTREE *)TreeView_GetlParam(GetDlgItem(hWnd, WWWC_TREE), hItem)) == NULL ||
		(CheckFlag != CHECKINI_TREECHECK && TreeView_IsRecyclerItem(GetDlgItem(hWnd, WWWC_TREE), hItem) == TRUE) ||	//ごみ箱
		(CheckFlag != CHECKINI_CHECK && tpTreeInfo->CheckSt == 1)){
		return 0;
	}

	//アイテムリストの読み込み
	if(tpTreeInfo->ItemList == NULL){
		WaitCursor(TRUE);
		ReadTreeMem(hWnd, hItem);
		WaitCursor(FALSE);
	}
	if(tpTreeInfo->ItemList == NULL){
		return 0;
	}

	for(i = 0;i < tpTreeInfo->ItemListCnt;i++){
		if((*(tpTreeInfo->ItemList + i)) == NULL ||
			(*(tpTreeInfo->ItemList + i))->CheckSt == 1 ||
			(*(tpTreeInfo->ItemList + i))->IconStatus != ST_DEFAULT){
			continue;
		}
		cnt++;
	}
	return cnt;
}


/******************************************************************************

	ItemCheckIni

	アイテムのチェックの初期化

******************************************************************************/

void ItemCheckIni(HWND hWnd, struct TPITEM *tpItemInfo, BOOL no_err)
{
	char buf[BUFSIZE];
	int ProtocolIndex;
	int ret;

	if(tpItemInfo == NULL || tpItemInfo->IconStatus != ST_DEFAULT){
		return;
	}

	//エラーアイテムのみチェックの場合
	if(ErrCheckFlag == TRUE && no_err == FALSE){
		if(tpItemInfo->Status != ST_ERROR && tpItemInfo->Status != ST_TIMEOUT){
			return;
		}
	}

	ProtocolIndex = GetProtocolIndex(tpItemInfo->CheckURL);
	if(ProtocolIndex == -1 || (tpProtocol + ProtocolIndex)->lib == NULL){
		return;
	}

	if(ReturnIcon == 1){
		Item_Initialize(hWnd, tpItemInfo, TRUE);
	}
	//アイテムの初期化
	if((tpProtocol + ProtocolIndex)->Func_Initialize == NULL){
		wsprintf(buf, "%sInitialize", (tpProtocol + ProtocolIndex)->FuncHeader);
		(tpProtocol + ProtocolIndex)->Func_Initialize = GetProcAddress((HMODULE)(tpProtocol + ProtocolIndex)->lib, buf);
	}
	if((tpProtocol + ProtocolIndex)->Func_Initialize != NULL){
		ret = (tpProtocol + ProtocolIndex)->Func_Initialize(hWnd, tpItemInfo);
	}
	tpItemInfo->IconStatus = ST_NOCHECK;
}


/******************************************************************************

	ListCheckIni

	リストのアイテムの初期化

******************************************************************************/

static void ListCheckIni(HWND hWnd, BOOL TreeFlag)
{
	struct TPTREE *tpTreeInfo;
	struct TPITEM *tpItemInfo;
	HTREEITEM hItem;
	int i;

	//ツリー情報を取得
	tpTreeInfo = (struct TPTREE *)TreeView_GetlParam(GetDlgItem(hWnd, WWWC_TREE),
		TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)));
	if(tpTreeInfo == NULL || tpTreeInfo->ItemList == NULL){
		return;
	}

	//フォルダにチェック中フラグをセット
	tpTreeInfo->CheckFlag = 1;

	//リストビューの選択項目のチェック
	i = -1;
	while((i = ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), i, LVNI_SELECTED)) != -1){
		tpItemInfo = (struct TPITEM *)ListView_GetlParam(GetDlgItem(hWnd, WWWC_LIST), i);
		if(tpItemInfo == NULL){
			hItem = TreeView_GetIndexToItem(GetDlgItem(hWnd, WWWC_TREE), TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)), i);
			if(TreeFlag == TRUE){
				CallTreeItem(hWnd, hItem, (FARPROC)TreeCheckIni, CHECKINI_TREECHECK);
			}else{
				TreeCheckIni(hWnd, hItem, CHECKINI_CHECK);
			}
		}else{
			ItemCheckIni(hWnd, tpItemInfo, FALSE);
		}
	}
}


/******************************************************************************

	FolderCheckIni

	フォルダのチェックの初期化

******************************************************************************/

void FolderCheckIni(HWND hWnd, HTREEITEM hItem)
{
	struct TPTREE *tpTreeInfo;
	int i;

	//ツリー情報を取得
	tpTreeInfo = (struct TPTREE *)TreeView_GetlParam(GetDlgItem(hWnd, WWWC_TREE), hItem);
	if(tpTreeInfo == NULL ||
		tpTreeInfo->CheckFlag != 1 ||
		tpTreeInfo->ItemList == NULL){
		return;
	}

	//フォルダ内の全てのアイテムをチェック
	for(i = 0;i < tpTreeInfo->ItemListCnt;i++){
		if((*(tpTreeInfo->ItemList + i)) == NULL ||
			(*(tpTreeInfo->ItemList + i))->CheckSt == 1){
			continue;
		}
		ItemCheckIni(hWnd, (*(tpTreeInfo->ItemList + i)), FALSE);
	}
}


/******************************************************************************

	TreeCheckIni

	ツリーのアイテムの初期化

******************************************************************************/

static void CALLBACK TreeCheckIni(HWND hWnd, HTREEITEM hItem, long Param)
{
	struct TPTREE *tpTreeInfo;

	//ツリー情報を取得
	if((tpTreeInfo = (struct TPTREE *)TreeView_GetlParam(GetDlgItem(hWnd, WWWC_TREE), hItem)) == NULL ||
		(Param != CHECKINI_TREECHECK && TreeView_IsRecyclerItem(GetDlgItem(hWnd, WWWC_TREE), hItem) == TRUE) ||	//ごみ箱
		(Param != CHECKINI_CHECK && tpTreeInfo->CheckSt == 1) ||				//チェックしない設定のフォルダ
		(Param == CHECKINI_AUTOALLCHECK && tpTreeInfo->AutoCheckSt == 0)){		//自動チェックでかつデフォルトのチェックタイムを使用する
		return;
	}

	//フォルダにチェック中フラグをセット
	tpTreeInfo->CheckFlag = 1;

	//アイテムが存在しない場合は抜ける
	if(tpTreeInfo->ItemList == NULL){
		return;
	}

	//フォルダ内の全てのアイテムをチェック
	FolderCheckIni(hWnd, hItem);
}


/******************************************************************************

	CheckIniProc

	チェックの初期化

******************************************************************************/

BOOL CheckIniProc(HWND hWnd, HTREEITEM hItem, int CheckFlag, int CheckType)
{
	struct TPTREE *tpTreeInfo;
	char fDay[BUFSIZE];
	char fTime[BUFSIZE];
	char buf[BUFSIZE];
	int cnt = 0;
	int i;
	BOOL ReloadFlag = FALSE;

	//チェック可能なアイテム数を取得
	switch(CheckFlag)
	{
	case CHECKINI_CHECK:
		//単体チェック
		if(GetFocus() == GetDlgItem(hWnd, WWWC_TREE) || hItem != TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)) ||
			ListView_GetSelectedCount(GetDlgItem(hWnd, WWWC_LIST)) == 0){
			if(hItem == NULL){
				return FALSE;
			}
			cnt = TreeCheckCount(hWnd, hItem, CheckFlag);

		}else{
			cnt = ListCheckCount(hWnd);
		}
		break;

	case CHECKINI_AUTOCHECK:
		//自動チェック
		if(hItem == NULL){
			return FALSE;
		}
		cnt = TreeCheckCount(hWnd, hItem, CheckFlag);
		break;

	case CHECKINI_AUTOALLCHECK:
	case CHECKINI_ALLCHECK:
	case CHECKINI_TREECHECK:
	case CHECKINI_DLLCHECK:
	default:
		cnt = 1;
		break;
	}
	if(cnt == 0){
		//チェックするアイテム無し
		if(hItem == NULL){
			TreeView_FreeItem(hWnd, hItem, 1);
		}
		return FALSE;
	}

	gCheckType |= CheckType;

	if(gCheckFlag != 1){
		//フォルダの内容を保存
		SendMessage(hWnd, WM_FOLDER_SAVE, 0, 0);

		//チェック開始時に実行するツールを実行する
		for(i = 0;i < ToolListCnt;i++){
			if((ToolList[i].Action & TOOL_EXEC_CHECKSTART) != 0){
				if(str_match("*.dll", ToolList[i].FileName) == TRUE){
					//ツールが -1 を返した場合はチェックを行わない
					if(DllToolExec(hWnd, i, NULL, -1, TOOL_EXEC_CHECKSTART, gCheckType) == -1){
						return FALSE;
					}
				}else{
					ExecItemFile(hWnd, ToolList[i].FileName, ToolList[i].CommandLine, NULL,
						ToolList[i].Action & TOOL_EXEC_SYNC);
				}
				if((ToolList[i].Action & TOOL_EXEC_SAVEFOLDER) != 0){
					ReloadFlag = TRUE;
				}
			}
		}
		if(ReloadFlag == TRUE){
			//フォルダの内容を読み直す
			SendMessage(hWnd, WM_FOLDER_LOAD, 0, 0);
		}
	}

	//チェックのタイプにより初期化の方法を変える
	switch(CheckFlag)
	{
	case CHECKINI_CHECK:
		//単体チェック
		if(GetFocus() == GetDlgItem(hWnd, WWWC_TREE) || hItem != TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)) ||
			ListView_GetSelectedCount(GetDlgItem(hWnd, WWWC_LIST)) == 0){
			if(hItem == NULL){
				return FALSE;
			}
			TreeCheckIni(hWnd, hItem, CheckFlag);

		}else{
			ListCheckIni(hWnd, FALSE);
		}
		break;

	case CHECKINI_AUTOCHECK:
		//自動チェック
		TreeCheckIni(hWnd, hItem, CheckFlag);
		break;

	case CHECKINI_AUTOALLCHECK:
	case CHECKINI_ALLCHECK:
		//すべてのアイテムをチェック
		CallTreeItem(hWnd, RootItem, (FARPROC)TreeCheckIni, CheckFlag);
		break;

	case CHECKINI_TREECHECK:
		//階層チェック
		if(GetFocus() == GetDlgItem(hWnd, WWWC_TREE) || hItem != TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)) ||
			ListView_GetSelectedCount(GetDlgItem(hWnd, WWWC_LIST)) == 0){
			TreeCheckIni(hWnd, hItem, CheckFlag);
			CallTreeItem(hWnd, hItem, (FARPROC)TreeCheckIni, CheckFlag);
		}else{
			ListCheckIni(hWnd, TRUE);
		}
		break;

	case CHECKINI_DLLCHECK:
		if(hItem == NULL){
			return FALSE;
		}
		tpTreeInfo = (struct TPTREE *)TreeView_GetlParam(GetDlgItem(hWnd, WWWC_TREE), hItem);
		if(tpTreeInfo == NULL){
			return FALSE;
		}
		tpTreeInfo->CheckFlag = 1;
		break;
	}

	//チェックアイテムリストの確保
	if(CheckItemList == NULL){
		CheckItemList = (struct TPITEM **)GlobalAlloc(GPTR, sizeof(struct TPITEM *) * CheckMax);
		if(CheckItemList == NULL){
			ErrMsg(hWnd, GetLastError(), NULL);
			return FALSE;
		}
		for(i = 0;i < CheckMax;i++){
			*(CheckItemList + i) = NULL;
		}
		UPcnt = 0;
	}
	//更新アイテムリストの確保
	if(UpItemList == NULL){
		UpItemList = (struct TPITEM **)GlobalAlloc(GPTR, sizeof(struct TPITEM *) * 0);
		if(UpItemList == NULL){
			ErrMsg(hWnd, GetLastError(), NULL);
			return FALSE;
		}
		UpItemListCnt = 0;
	}

	//ウィンドウタイトルの設定
	GetDateTime(fDay, fTime);
	wsprintf(WinTitle, TITLE_TIMEFORMAT, fDay, fTime);
	SetWinTitle(hWnd);

	//タスクトレイの設定
	GetWindowText(hWnd, buf, BUFSIZE - 1);
	if(TrayIcon == 1 && TrayMessage(hWnd, NIM_MODIFY, TRAY_ID, TrayIcon_Chaeck, buf) == FALSE){
		TrayMessage(hWnd, NIM_ADD, TRAY_ID, TrayIcon_Chaeck, buf);
	}
	SetClassLong(hWnd, GCL_HICON, (long)TrayIcon_Chaeck_Win);

	//既にチェック中の場合
	if(gCheckFlag == 1){
		SetTimer(hWnd, TIMER_CHECK, NEXTCHECK_INTERVAL, NULL);		//アイテムチェック用タイマー
		return TRUE;
	}

	//チェック開始時更新アイテムを解放する設定
	if(ClearTime == 0){
		switch(CheckUPItemClear)
		{
		//無条件
		case 0:
			SendMessage(UpWnd, WM_UP_FREE, 0, 0);
			break;

		//ウィンドウが表示状態のとき
		case 1:
			if(IsWindowVisible(UpWnd) == 0){
				SendMessage(UpWnd, WM_UP_FREE, 0, 0);
			}
			break;

		//空にしない
		case 2:
			break;
		}
	}

	//チェック中フラグのセット
	gCheckFlag = 1;

	//メニューの状態を設定
	SetEnableMenu(hWnd);
	SetEnableToolMenu(hWnd);
	SetEnableToolItemMenu(GetSubMenu(hPOPUP, MENU_POP_ITEM));

	//ステータスバーの設定
	SendDlgItemMessage(hWnd, WWWC_SB, SB_SETTEXT, (WPARAM)0 | 0, (LPARAM)"チェックを開始");
	//ステータスバーにアイコンを設定
	SendDlgItemMessage(hWnd, WWWC_SB, WM_USER + 15, (WPARAM)0 | 0, (LPARAM)StCheckIcon);

	//チェック用タイマーの開始
	SetTimer(AniWnd, TIMER_ANI, CHECKANI_INTERVAL, NULL);		//アニメーション用タイマー
	SetTimer(hWnd, TIMER_CHECK, NEXTCHECK_INTERVAL, NULL);		//アイテムチェック用タイマー
	SetTimer(hWnd, TIMER_CHECKTIMEOUT, TIMEOUT_INTERVAL, NULL);	//タイムアウト用タイマー
	return TRUE;
}


/******************************************************************************

	FindCheckTree

	チェック中のアイテムを検索する

******************************************************************************/

static void CALLBACK FindCheckTree(HWND hWnd, HTREEITEM hItem, long Param)
{
	struct TPTREE *tpTreeInfo;
	int *ret = (int *)Param;
	int i;

	if(*ret == 1 ||
		(tpTreeInfo = (struct TPTREE *)TreeView_GetlParam(GetDlgItem(hWnd, WWWC_TREE), hItem)) == NULL ||
		tpTreeInfo->CheckFlag == 0 || tpTreeInfo->ItemList == NULL){
		return;
	}

	//チェック中のアイテムの検索
	for(i = 0;i < tpTreeInfo->ItemListCnt;i++){
		if((*(tpTreeInfo->ItemList + i)) == NULL){
			continue;
		}
		if((*(tpTreeInfo->ItemList + i))->IconStatus == ST_CHECK){
			*ret = 1;
			return;
		}
	}
}
int FindCheckItem(HWND hWnd, HTREEITEM hItem)
{
	int ret = 0;

	CallTreeItem(hWnd, hItem, (FARPROC)FindCheckTree, (long)&ret);
	return ret;
}


/******************************************************************************

	FindCheckNoCheckTree

	チェック中かチェック待機中のアイテムを検索する

******************************************************************************/

static void CALLBACK FindCheckNoCheckTree(HWND hWnd, HTREEITEM hItem, long Param)
{
	struct TPTREE *tpTreeInfo;
	int *ret = (int *)Param;

	//既にチェック中かチェック待機中のアイテムが見付かっている場合
	if(*ret == 1 ||
		(tpTreeInfo = (struct TPTREE *)TreeView_GetlParam(GetDlgItem(hWnd, WWWC_TREE), hItem)) == NULL ||
		tpTreeInfo->CheckFlag == 0){
		return;
	}
	*ret = 1;
}
int FindCheckNoCheckItem(HWND hWnd, HTREEITEM hItem)
{
	int ret = 0;

	CallTreeItem(hWnd, hItem, (FARPROC)FindCheckNoCheckTree, (long)&ret);
	return ret;
}


/******************************************************************************

	SetCheckDateTime

	チェック開始時間の取得

******************************************************************************/

static void SetCheckDateTime(struct TPITEM *tpItemInfo)
{
	char fDay[BUFSIZE];
	char fTime[BUFSIZE];

	//現在の日時を取得する
	GetDateTime(fDay, fTime);

	if(tpItemInfo->CheckDate != NULL){
		GlobalFree(tpItemInfo->CheckDate);
	}
	tpItemInfo->CheckDate = (char *)GlobalAlloc(GPTR, lstrlen(fDay) + lstrlen(fTime) + 2);
	if(tpItemInfo->CheckDate == NULL){
		return;
	}

	wsprintf(tpItemInfo->CheckDate, "%s %s", fDay, fTime);
}


/******************************************************************************

	CheckStartItem

	アイテムのチェックの開始

******************************************************************************/

static int CheckStartItem(HWND hWnd, struct TPITEM *tpItemInfo, int ProtocolIndex, int CheckListIndex)
{
	char buf[BUFSIZE];
	int ret;

	tpItemInfo->IconStatus = ST_CHECK;
	SetCheckDateTime(tpItemInfo);

	if((tpProtocol + ProtocolIndex)->lib == NULL){
		tpItemInfo->Status = (tpItemInfo->Status & ST_UP) ? ST_UP | ST_ERROR : ST_ERROR;
		tpItemInfo->IconStatus = ST_DEFAULT;
		return -1;
	}

	//プロトコルDLLのチェック開始の関数を呼ぶ
	if((tpProtocol + ProtocolIndex)->Func_Start == NULL){
		wsprintf(buf, "%sStart", (tpProtocol + ProtocolIndex)->FuncHeader);
		(tpProtocol + ProtocolIndex)->Func_Start = GetProcAddress((HMODULE)(tpProtocol + ProtocolIndex)->lib, buf);
	}
	if((tpProtocol + ProtocolIndex)->Func_Start == NULL){
		tpItemInfo->Status = (tpItemInfo->Status & ST_UP) ? ST_UP | ST_ERROR : ST_ERROR;
		tpItemInfo->IconStatus = ST_DEFAULT;
		return -1;
	}
	ret = (tpProtocol + ProtocolIndex)->Func_Start(hWnd, tpItemInfo);
	switch(ret)
	{
	//エラー
	case CHECK_ERROR:
		tpItemInfo->Status = (tpItemInfo->Status & ST_UP) ? ST_UP | ST_ERROR : ST_ERROR;
		tpItemInfo->IconStatus = ST_DEFAULT;
		TreeView_SetIconState(hWnd, tpItemInfo->hItem, 0);
		NotifyItemCheckEnd(hWnd, tpItemInfo);
		return -1;

	//チェック開始
	case CHECK_SUCCEED:
		tpItemInfo->IconStatus = ST_CHECK;
		*(CheckItemList + CheckListIndex) = tpItemInfo;
		break;

	//チェック待ち
	case CHECK_NO:
		tpItemInfo->IconStatus = ST_NOCHECK;
		break;

	//チェック終了
	case CHECK_END:
		tpItemInfo->IconStatus = ST_DEFAULT;
		TreeView_SetIconState(hWnd, tpItemInfo->hItem, 0);
		NotifyItemCheckEnd(hWnd, tpItemInfo);
		return -1;
	}
	TreeView_SetIconState(hWnd, tpItemInfo->hItem, 0);
	return 0;
}


/******************************************************************************

	CheckStartFolder

	フォルダごとのチェックの開始

******************************************************************************/

static void CALLBACK CheckStartFolder(HWND hWnd, HTREEITEM hItem, long Param)
{
	struct TPTREE *tpTreeInfo;
	int ProtocolIndex;
	int i, ccnt = 0;
	BOOL CheckFlag = FALSE;

	if(CheckItemList == NULL || *(CheckItemList + Param) != NULL ||
		(tpTreeInfo = (struct TPTREE *)TreeView_GetlParam(GetDlgItem(hWnd, WWWC_TREE), hItem)) == NULL ||
		tpTreeInfo->CheckFlag == 0){
		return;
	}

	//アイテムリストの読み込み
	if(tpTreeInfo->ItemList == NULL){
		WaitCursor(TRUE);
		ReadTreeMem(hWnd, hItem);
		WaitCursor(FALSE);
	}
	if(tpTreeInfo->ItemList == NULL){
		tpTreeInfo->CheckFlag = 0;
		return;
	}

	//全てのアイテムに対してチェック開始処理を行う
	for(i = 0;i < tpTreeInfo->ItemListCnt;i++){
		if((*(tpTreeInfo->ItemList + i)) == NULL){
			continue;
		}
		ProtocolIndex = GetProtocolIndex((*(tpTreeInfo->ItemList + i))->CheckURL);
		if((*(tpTreeInfo->ItemList + i))->IconStatus == ST_CHECK){
			CheckFlag = TRUE;
			ccnt++;
			if(tpTreeInfo->CheckMax > 0 && tpTreeInfo->CheckMax <= ccnt){
				break;
			}

		}else if(ProtocolIndex != -1 && (*(tpTreeInfo->ItemList + i))->IconStatus == ST_NOCHECK){
			//チェック開始
			if(CheckStartItem(hWnd, (*(tpTreeInfo->ItemList + i)), ProtocolIndex, Param) == -1){
				continue;
			}
			CheckFlag = TRUE;
			if(*(CheckItemList + Param) != NULL){
				break;
			}
		}
	}
	//チェック可能アイテムが存在しなかった場合はフォルダのチェックを終了する
	if(CheckFlag == FALSE){
		tpTreeInfo->CheckFlag = 0;
		TreeView_FreeItem(hWnd, hItem, 1);
		SetEnableMenu(hWnd);
	}
}


/******************************************************************************

	CheckProc

	チェック処理

******************************************************************************/

int CheckProc(HWND hWnd)
{
	int j, ret;
	int ListIndex;

	if(CheckItemList == NULL){
		return 0;
	}
	//チェックリストの空きを検索
	for(j = 0;j < CheckMax;j++){
		if(*(CheckItemList + j) == NULL){
			break;
		}
	}
	if(j >= CheckMax){
		KillTimer(hWnd, TIMER_CHECK);
		return 0;
	}

	//チェックするアイテムを検索してチェックを開始
	CallTreeItem(hWnd, RootItem, (FARPROC)CheckStartFolder, (long)j);
	if(CheckItemList == NULL){
		return 0;
	}

	if(*(CheckItemList + j) == NULL){		//チェックすべきアイテムが無かった場合
		KillTimer(hWnd, TIMER_CHECK);

		//現在チェック中のアイテムを検索
		for(j = 0;j < CheckMax;j++){
			if(*(CheckItemList + j) != NULL){
				break;
			}
		}
		if(j < CheckMax){
			return 0;
		}

		//チェック可能なフォルダを検索
		ret = FindCheckNoCheckItem(hWnd, RootItem);
		if(ret != 0){
			return 0;
		}

		//チェック終了
		InvalidateRect(GetDlgItem(hWnd, WWWC_LIST), NULL, FALSE);
		UpdateWindow(GetDlgItem(hWnd, WWWC_LIST));

		WaitCursor(TRUE);
		CheckEndProc(hWnd);
		WaitCursor(FALSE);

	}else{
		//現在選択しているフォルダのアイテムの場合はアイテムの表示更新を行う
		if((*(CheckItemList + j))->hItem == TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE))){
			ListIndex = ListView_GetMemToIndex(GetDlgItem(hWnd, WWWC_LIST), *(CheckItemList + j));
			ListView_RedrawItems(GetDlgItem(hWnd, WWWC_LIST), ListIndex, ListIndex);
			UpdateWindow(GetDlgItem(hWnd, WWWC_LIST));
			SetEnableMenu(hWnd);
		}
	}
	return 0;
}


/******************************************************************************

	ResultCheckStatus

	チェック結果により状態を変更

******************************************************************************/

BOOL ResultCheckStatus(HWND hWnd, struct TPITEM *tpItemInfo, int ret)
{
	switch(ret)
	{
	case CHECK_ERROR:
		//エラー
		tpItemInfo->Status = (tpItemInfo->Status & ST_UP) ? ST_UP | ST_ERROR : ST_ERROR;
		tpItemInfo->IconStatus = ST_DEFAULT;
		NotifyItemCheckEnd(hWnd, tpItemInfo);
		break;

	case CHECK_SUCCEED:
		//正常・待機
		return TRUE;

	case CHECK_NO:
		//チェック待ち
		tpItemInfo->IconStatus = ST_NOCHECK;
		break;

	case CHECK_END:
		//チェック終了
	default:
		tpItemInfo->IconStatus = ST_DEFAULT;
		NotifyItemCheckEnd(hWnd, tpItemInfo);
		break;
	}
	return FALSE;
}


/******************************************************************************

	InitCheckItemList

	アイテムチェック終了時のチェックアイテムリストの初期化

******************************************************************************/

void InitCheckItemList(HWND hWnd, struct TPITEM *tpItemInfo)
{
	int ListIndex;
	int i;

	//ツリービューとリストビューの表示更新
	TreeView_SetIconState(hWnd, tpItemInfo->hItem, 0);
	if(tpItemInfo->hItem == TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE))){
		ListIndex = ListView_GetMemToIndex(GetDlgItem(hWnd, WWWC_LIST), tpItemInfo);
		ListView_RedrawItems(GetDlgItem(hWnd, WWWC_LIST), ListIndex, ListIndex);
		UpdateWindow(GetDlgItem(hWnd, WWWC_LIST));
	}

	//チェックアイテムリストの初期化
	if(CheckItemList == NULL){
		return;
	}
	for(i = 0; i < CheckMax; i++){
		if(*(CheckItemList + i) == NULL){
			continue;
		}
		if(tpItemInfo == *(CheckItemList + i)){
			*(CheckItemList + i) = NULL;
			break;
		}
	}

	//次のチェックアイテム
	SetTimer(hWnd, TIMER_CHECK, NEXTCHECK_INTERVAL, NULL);
}


/******************************************************************************

	GethostMsg

	ホスト情報取得イベント

******************************************************************************/

int GethostMsg(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	struct TPITEM *tpItemInfo;
	char buf[BUFSIZE];
	int Index;
	int ProtocolIndex;
	int ret;

	Index = GetHandleToIndex((HANDLE)wParam);
	if(Index == -1){
		return 0;
	}
	tpItemInfo = *(CheckItemList + Index);

	//プロトコルDLLのホスト情報取得関数を呼ぶ
	ProtocolIndex = GetProtocolIndex(tpItemInfo->CheckURL);
	if(ProtocolIndex == -1){
		tpItemInfo->Status = (tpItemInfo->Status & ST_UP) ? ST_UP | ST_ERROR : ST_ERROR;
		tpItemInfo->IconStatus = ST_DEFAULT;
		//チェックアイテムリストの初期化
		InitCheckItemList(hWnd, tpItemInfo);
		return 0;
	}
	if((tpProtocol + ProtocolIndex)->Func_Gethost == NULL){
		wsprintf(buf, "%sGethost", (tpProtocol + ProtocolIndex)->FuncHeader);
		(tpProtocol + ProtocolIndex)->Func_Gethost = GetProcAddress((HMODULE)(tpProtocol + ProtocolIndex)->lib, buf);
	}
	if((tpProtocol + ProtocolIndex)->Func_Gethost == NULL){
		tpItemInfo->Status = (tpItemInfo->Status & ST_UP) ? ST_UP | ST_ERROR : ST_ERROR;
		tpItemInfo->IconStatus = ST_DEFAULT;
	}else{
		ret = (tpProtocol + ProtocolIndex)->Func_Gethost(hWnd, wParam, lParam, tpItemInfo);
		if(ResultCheckStatus(hWnd, tpItemInfo, ret) == TRUE){
			return 0;
		}
	}
	//チェックアイテムリストの初期化
	InitCheckItemList(hWnd, tpItemInfo);
	return 0;
}


/******************************************************************************

	SelectMsg

	Selectイベント

******************************************************************************/

int SelectMsg(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	struct TPITEM *tpItemInfo;
	char buf[BUFSIZE];
	int Index;
	int ProtocolIndex;
	int ret;

	Index = GetSocketToIndex((int)wParam);
	if(Index == -1){
		return 0;
	}
	tpItemInfo = *(CheckItemList + Index);

	//プロトコルDLLのSelect処理関数を呼ぶ
	ProtocolIndex = GetProtocolIndex(tpItemInfo->CheckURL);
	if(ProtocolIndex == -1){
		tpItemInfo->Status = (tpItemInfo->Status & ST_UP) ? ST_UP | ST_ERROR : ST_ERROR;
		tpItemInfo->IconStatus = ST_DEFAULT;
		InitCheckItemList(hWnd, tpItemInfo);
		return 0;
	}
	if((tpProtocol + ProtocolIndex)->Func_Select == NULL){
		wsprintf(buf, "%sSelect", (tpProtocol + ProtocolIndex)->FuncHeader);
		(tpProtocol + ProtocolIndex)->Func_Select = GetProcAddress((HMODULE)(tpProtocol + ProtocolIndex)->lib, buf);
	}
	if((tpProtocol + ProtocolIndex)->Func_Select == NULL){
		tpItemInfo->Status = (tpItemInfo->Status & ST_UP) ? ST_UP | ST_ERROR : ST_ERROR;
		tpItemInfo->IconStatus = ST_DEFAULT;
	}else{
		ret = (tpProtocol + ProtocolIndex)->Func_Select(hWnd, wParam, lParam, tpItemInfo);
		if(ResultCheckStatus(hWnd, tpItemInfo, ret) == TRUE){
			return 0;
		}
	}
	//チェックアイテムリストの初期化
	InitCheckItemList(hWnd, tpItemInfo);
	return 0;
}


/******************************************************************************

	TimeoutItem

	タイムアウト用タイマー処理

******************************************************************************/

void TimeoutItem(HWND hWnd)
{
	char buf[BUFSIZE];
	int i;
	int ProtocolIndex;
	int ret;

	if(CheckItemList == NULL){
		return;
	}

	for(i = 0;i < CheckMax;i++){
		if(*(CheckItemList + i) == NULL){
			continue;
		}
		ProtocolIndex = GetProtocolIndex((*(CheckItemList + i))->CheckURL);
		if(ProtocolIndex == -1){
			continue;
		}
		if((tpProtocol + ProtocolIndex)->lib == NULL){
			continue;
		}

		if((tpProtocol + ProtocolIndex)->Func_Timer == NULL){
			wsprintf(buf, "%sTimer", (tpProtocol + ProtocolIndex)->FuncHeader);
			(tpProtocol + ProtocolIndex)->Func_Timer = GetProcAddress((HMODULE)(tpProtocol + ProtocolIndex)->lib, buf);
		}
		if((tpProtocol + ProtocolIndex)->Func_Timer != NULL){
			ret = (tpProtocol + ProtocolIndex)->Func_Timer(hWnd, *(CheckItemList + i));
			if(CheckItemList == NULL){
				return;
			}
			if(*(CheckItemList + i) == NULL){
				continue;
			}
			if(ResultCheckStatus(hWnd, *(CheckItemList + i), ret) == TRUE){
				continue;
			}
			InitCheckItemList(hWnd, *(CheckItemList + i));
		}
	}
}


/******************************************************************************

	CancelItem

	アイテムのキャンセル

******************************************************************************/

static void CancelItem(HWND hWnd, struct TPITEM *tpItemInfo)
{
	char buf[BUFSIZE];
	int Index;
	int ProtocolIndex;
	int ListIndex;
	int ret;

	if(tpItemInfo == NULL){
		return;
	}
	if(tpItemInfo->IconStatus != ST_CHECK){
		tpItemInfo->IconStatus = ST_DEFAULT;
		return;
	}

	Index = GetItemInfoToIndex(tpItemInfo);

	ProtocolIndex = GetProtocolIndex(tpItemInfo->CheckURL);
	if(ProtocolIndex == -1){
		tpItemInfo->IconStatus = ST_DEFAULT;
		*(CheckItemList + Index) = NULL;
		return;
	}
	if((tpProtocol + ProtocolIndex)->lib == NULL){
		tpItemInfo->IconStatus = ST_DEFAULT;
		*(CheckItemList + Index) = NULL;
		return;
	}

	//プロトコルDLLのチェックのキャンセル関数を呼出す
	if((tpProtocol + ProtocolIndex)->Func_Cancel == NULL){
		wsprintf(buf, "%sCancel", (tpProtocol + ProtocolIndex)->FuncHeader);
		(tpProtocol + ProtocolIndex)->Func_Cancel = GetProcAddress((HMODULE)(tpProtocol + ProtocolIndex)->lib, buf);
	}
	if((tpProtocol + ProtocolIndex)->Func_Cancel != NULL){
		ret = (tpProtocol + ProtocolIndex)->Func_Cancel(hWnd, tpItemInfo);
	}

	tpItemInfo->IconStatus = ST_DEFAULT;
	if(CheckItemList == NULL){
		return;
	}
	*(CheckItemList + Index) = NULL;

	//表示されているアイテムの場合は表示の更新を行う
	if(tpItemInfo->hItem == TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE))){
		ListIndex = ListView_GetMemToIndex(GetDlgItem(hWnd, WWWC_LIST), tpItemInfo);
		if(tpItemInfo->RefreshFlag = TRUE){
			ListView_SetItemText(GetDlgItem(hWnd, WWWC_LIST), ListIndex, 0, LPSTR_TEXTCALLBACK);
		}
		ListView_RedrawItems(GetDlgItem(hWnd, WWWC_LIST), ListIndex, ListIndex);
		UpdateWindow(GetDlgItem(hWnd, WWWC_LIST));
	}
	tpItemInfo->RefreshFlag = FALSE;
}


/******************************************************************************

	ListCancelItem

	リストアイテムのキャンセル

******************************************************************************/

void ListCancelItem(HWND hWnd)
{
	struct TPTREE *tpTreeInfo;
	struct TPITEM *tpItemInfo;
	HTREEITEM hItem;
	int i;

	hItem = TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE));

	if(ListView_GetSelectedCount(GetDlgItem(hWnd, WWWC_LIST)) == 0){
		TreeCancelItem(hWnd, hItem, 0);
		return;
	}

	//選択されているアイテムのキャンセルを行う
	i = -1;
	while((i = ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), i, LVNI_SELECTED)) != -1){
		tpItemInfo = (struct TPITEM *)ListView_GetlParam(GetDlgItem(hWnd, WWWC_LIST), i);
		if(tpItemInfo != NULL){
			CancelItem(hWnd, tpItemInfo);
			tpItemInfo->IconStatus = ST_DEFAULT;

		}else{
			//フォルダの場合は再帰する
			TreeCancelItem(hWnd, TreeView_GetIndexToItem(GetDlgItem(hWnd, WWWC_TREE), hItem, i), 0);
		}
	}

	tpTreeInfo = (struct TPTREE *)TreeView_GetlParam(GetDlgItem(hWnd, WWWC_TREE), hItem);
	if(tpTreeInfo == NULL){
		return;
	}
	if(tpTreeInfo->ItemList == NULL){
		tpTreeInfo->CheckFlag = 0;
		return;
	}

	//チェック中のアイテムを検索する
	for(i = 0;i < tpTreeInfo->ItemListCnt;i++){
		if((*(tpTreeInfo->ItemList + i)) == NULL){
			continue;
		}
		if((*(tpTreeInfo->ItemList + i))->IconStatus != ST_DEFAULT){
			break;
		}
	}
	if(i >= tpTreeInfo->ItemListCnt){
		//チェック中のアイテムが無い場合
		tpTreeInfo->CheckFlag = 0;
		TreeView_SetIconState(hWnd, hItem, 0);
	}
}


/******************************************************************************

	TreeCancelItem

	階層的にアイテムのキャンセル

******************************************************************************/

void CALLBACK TreeCancelItem(HWND hWnd, HTREEITEM hItem, long Param)
{
	struct TPTREE *tpTreeInfo;
	int i;

	tpTreeInfo = (struct TPTREE *)TreeView_GetlParam(GetDlgItem(hWnd, WWWC_TREE), hItem);
	if(tpTreeInfo == NULL){
		return;
	}
	if(tpTreeInfo->ItemList == NULL){
		tpTreeInfo->CheckFlag = 0;

		TreeView_SetIconState(hWnd, hItem, 0);
		return;
	}

	//アイテムリストの中のアイテムすべてにキャンセルを行う
	for(i = 0;i < tpTreeInfo->ItemListCnt;i++){
		if((*(tpTreeInfo->ItemList + i)) == NULL){
			continue;
		}
		CancelItem(hWnd, (*(tpTreeInfo->ItemList + i)));
		(*(tpTreeInfo->ItemList + i))->IconStatus = ST_DEFAULT;
	}
	tpTreeInfo->CheckFlag = 0;

	TreeView_SetIconState(hWnd, hItem, 0);
	TreeView_FreeItem(hWnd, hItem, 1);
}


/******************************************************************************

	CheckEndItem

	アイテムのチェック結果通知

******************************************************************************/

int CheckEndItem(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	int ListIndex;

	if(((struct TPITEM *)lParam) == NULL){
		return 0;
	}

	((struct TPITEM *)lParam)->IconStatus = ST_DEFAULT;
	switch(wParam)
	{
	//更新あり
	case ST_UP:
		((struct TPITEM *)lParam)->Status = ST_UP;
		UPcnt++;
		AddUpItem((struct TPITEM *)lParam);
		break;

	case ST_DEFAULT:
	case ST_ERROR:
	case ST_TIMEOUT:
		((struct TPITEM *)lParam)->Status = (((struct TPITEM *)lParam)->Status & ST_UP) ? ST_UP | wParam : wParam;
		break;
	}

	TreeView_SetIconState(hWnd, ((struct TPITEM *)lParam)->hItem, 0);

	if(((struct TPITEM *)lParam)->hItem == TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE))){
		ListIndex = ListView_GetMemToIndex(GetDlgItem(hWnd, WWWC_LIST), ((struct TPITEM *)lParam));
		if(((struct TPITEM *)lParam)->RefreshFlag = TRUE){
			ListView_SetItemText(GetDlgItem(hWnd, WWWC_LIST), ListIndex, 0, LPSTR_TEXTCALLBACK);
		}
		ListView_RedrawItems(GetDlgItem(hWnd, WWWC_LIST), ListIndex, ListIndex);
		UpdateWindow(GetDlgItem(hWnd, WWWC_LIST));
	}
	((struct TPITEM *)lParam)->RefreshFlag = FALSE;
	return 0;
}


/******************************************************************************

	NotifyItemCheckEnd

	アイテムのチェック終了通知

******************************************************************************/

static void NotifyItemCheckEnd(HWND hWnd, struct TPITEM *tpItemInfo)
{
	char buf[BUFSIZE];
	int ProtocolIndex;

	//チェック終了通知
	ProtocolIndex = GetProtocolIndex(tpItemInfo->CheckURL);
	if(ProtocolIndex != -1 && (tpProtocol + ProtocolIndex)->lib != NULL){
		if((tpProtocol + ProtocolIndex)->Func_ItemCheckEnd == NULL){
			wsprintf(buf, "%sItemCheckEnd", (tpProtocol + ProtocolIndex)->FuncHeader);
			(tpProtocol + ProtocolIndex)->Func_ItemCheckEnd = GetProcAddress((HMODULE)(tpProtocol + ProtocolIndex)->lib, buf);
		}
		if((tpProtocol + ProtocolIndex)->Func_ItemCheckEnd != NULL){
			(tpProtocol + ProtocolIndex)->Func_ItemCheckEnd(hWnd, tpItemInfo);
		}
	}
}


/******************************************************************************

	CheckEndProc

	全チェックの終了

******************************************************************************/

void CheckEndProc(HWND hWnd)
{
	struct TPTREE *tpTreeInfo;
	struct TPITEM **ToolItemList = NULL;
	char buf[BUFSIZE];
	HICON vIcon;
	int i, j;
	BOOL InitFlag = FALSE;

	KillTimer(hWnd, TIMER_CHECK);
	KillTimer(hWnd, TIMER_CHECKTIMEOUT);

	gCheckFlag = 0;

	if(CheckItemList == NULL){
		return;
	}
	GlobalFree(CheckItemList);
	CheckItemList = NULL;

	//メニューの状態の設定
	SetEnableMenu(hWnd);
	SetEnableToolMenu(hWnd);
	SetEnableToolItemMenu(GetSubMenu(hPOPUP, MENU_POP_ITEM));

	//ステータスバーのアイコンを消去
	SendDlgItemMessage(hWnd, WWWC_SB, WM_USER + 15, (WPARAM)0 | 0, (LPARAM)NULL);
	SetSbText(hWnd);

	//リストビューをソート
	if(LvAutoSort == 1 && LvCheckEndAutoSort == 1){
		tpTreeInfo = (struct TPTREE *)TreeView_GetlParam(GetDlgItem(hWnd, WWWC_TREE),
			TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)));
		if(tpTreeInfo != NULL){
			//アイテムをソート
			WaitCursor(TRUE);
			SortColInfo = ColumnInfo;
			ListView_SortItems(GetDlgItem(hWnd, WWWC_LIST), CompareFunc, LvSortFlag);

			GlobalFree(tpTreeInfo->ItemList);
			tpTreeInfo->ItemListCnt = 0;
			tpTreeInfo->ItemList = ListView_SetListToMem(hWnd, &tpTreeInfo->ItemListCnt);
			WaitCursor(FALSE);
		}
	}

	//チェック終了時に実行するツール
	for(i = 0;i < ToolListCnt;i++){
		if((ToolList[i].Action & TOOL_EXEC_CHECKEND) != 0 &&
			((UPcnt != 0 && (ToolList[i].Action & TOOL_EXEC_CHECKENDUP) != 0) ||
			(UPcnt == 0 && (ToolList[i].Action & TOOL_EXEC_CHECKENDNOUP) != 0) ||
			((ToolList[i].Action & TOOL_EXEC_CHECKENDUP) == 0 &&
			(ToolList[i].Action & TOOL_EXEC_CHECKENDNOUP) == 0))){

			//指定プロトコルのアイテムを抽出
			j = UpItemListCnt;
			ToolItemList = Item_ProtocolSelect(UpItemList, &j, ToolList[i].Protocol);
			if(ToolItemList != NULL){
				//ツールの実行
				SubItemExecTool(hWnd, i, ToolItemList, j, TOOL_EXEC_CHECKEND, gCheckType);
				if(ToolList[i].Action & TOOL_EXEC_INITITEM){
					//アイコンの初期化
					Item_MainItemIni(hWnd, hWnd, ToolItemList, j);
					InitFlag = TRUE;
				}
				GlobalFree(ToolItemList);
			}
		}
	}
	gCheckType = 0;

	//ツール実行後にアイコンの初期化を行う設定の処理
	WaitCursor(TRUE);
	//アイテム情報の解放
	CallTreeItem(hWnd, RootItem, (FARPROC)TreeView_FreeItem, 1);

	//表示の更新
	if(InitFlag == TRUE){
		InvalidateRect(GetDlgItem(hWnd, WWWC_LIST), NULL, FALSE);
		UpdateWindow(GetDlgItem(hWnd, WWWC_LIST));
	}
	WaitCursor(FALSE);

	//更新アイテムありの場合
	if(NoUpMsg == 1 || UPcnt > 0){
		//サウンドの再生
		if(UPSnd == 1){
			if(*WaveFile == '\0' || sndPlaySound(WaveFile, SND_ASYNC | SND_NODEFAULT) == FALSE){
				MessageBeep(MB_ICONASTERISK);
			}
		}
		//更新メッセージウィンドウの初期化
		if(CmdCheckEnd == FALSE || CmdNoUpCheckEnd == TRUE){
			SendMessage(UpWnd, WM_UP_INIT, 0, 0);
		}
	}

	//アイコンの設定
	if(InitFlag == FALSE && (UpIconFlag == TRUE || UPcnt > 0) && GetForegroundWindow() != hWnd){
		vIcon = TrayIcon_Up;
		UpIconFlag = TRUE;
	}else{
		vIcon = TrayIcon_Main;
	}
	GetWindowText(hWnd, buf, BUFSIZE - 1);
	if(TrayIcon == 1 && TrayMessage(hWnd, NIM_MODIFY, TRAY_ID, vIcon, buf) == FALSE){
		TrayMessage(hWnd, NIM_ADD, TRAY_ID, vIcon, buf);
	}
	SetClassLong(hWnd, GCL_HICON, (long)TrayIcon_Main_Win);

	if(CmdCheckEnd == TRUE){
		if(UPcnt > 0 && CmdNoUpCheckEnd == TRUE){
			CmdCheckEnd = FALSE;
			CmdNoUpCheckEnd = FALSE;
			return;
		}
		SendMessage(hWnd, WM_CLOSE, 0, 0);
	}
}
/* End of source */
