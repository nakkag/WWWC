/**************************************************************************

	WWWC

	DragDrop.c

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

#define DROPFILE_NONE			0
#define DROPFILE_NEWITEM		1

#define FMT_NON					0
#define FMT_WWWCITEM			1
#define FMT_TEXT				2
#define FMT_HDROP				3

#define NEWITEMNAME				"新しいアイテム"

#define EMSG_ITEMREAD_TITLE		"アイテムの読み込み"
#define EMSG_ITEMREAD			"アイテム情報ファイルが開けませんでした。"


/**************************************************************************
	Global Variables
**************************************************************************/

char **DropFileNameList;
int DropFileCnt;
UINT MenuEffect;

//外部参照
extern char CuDir[];
extern char TempDir[];
extern struct TP_PROTOCOL *tpProtocol;
extern int ProtocolCnt;
extern HMENU hPOPUP;
extern UINT WWWC_ClipFormat;
extern HTREEITEM RecyclerItem;
extern int UpdateItemFlag;
extern int DragItemIndex;
extern int DefNoCheck;
extern int SucceedFromParent;


/**************************************************************************
	Local Function Prototypes
**************************************************************************/

static BOOL CreateDropFile(struct TPITEM *tpItemInfo, char *fPath, char *iPath, char *ret);
static void CreateTreeDropFiles(HWND hWnd, HTREEITEM hItem, char *fPath, char *iPath);
static BOOL DragDrop_MouseMove(HWND hWnd);
static HTREEITEM DragDrop_GetHitestFolder(HWND hWnd);
static UINT DragDrop_ShowMenu(HWND hWnd, BOOL defFlag);


/******************************************************************************

	DragDrop_SetDropFileMem

	ドロップファイルの作成

******************************************************************************/

HDROP DragDrop_SetDropFileMem(HWND hWnd)
{
	if(DropFileNameList == NULL){
		return NULL;
	}
	//ドロップファイルリストの作成
	return CreateDropFileMem(DropFileNameList, DropFileCnt);
}


/******************************************************************************

	CreateDropFile

	ドロップファイル用文字列の作成

******************************************************************************/

static BOOL CreateDropFile(struct TPITEM *tpItemInfo, char *fPath, char *iPath, char *ret)
{
	int ProtocolIndex;
	char buf[BUFSIZE];

	if(tpItemInfo == NULL){
		return FALSE;
	}
	ProtocolIndex = GetProtocolIndex(tpItemInfo->CheckURL);
	if(ProtocolIndex == -1 || (tpProtocol + ProtocolIndex)->lib == NULL){
		return FALSE;
	}
	if((tpProtocol + ProtocolIndex)->Func_CreateDropFile == NULL){
		wsprintf(buf, "%sCreateDropItem", (tpProtocol + ProtocolIndex)->FuncHeader);
		(tpProtocol + ProtocolIndex)->Func_CreateDropFile = GetProcAddress((HMODULE)(tpProtocol + ProtocolIndex)->lib, buf);
	}
	if((tpProtocol + ProtocolIndex)->Func_CreateDropFile == NULL){
		return FALSE;
	}
	return (tpProtocol + ProtocolIndex)->Func_CreateDropFile(tpItemInfo, fPath, iPath, ret);
}


/******************************************************************************

	CreateTreeDropFiles

	階層的にドロップファイルの文字列を作成

******************************************************************************/

static void CreateTreeDropFiles(HWND hWnd, HTREEITEM hItem, char *fPath, char *iPath)
{
	struct TPTREE *tpTreeInfo;
	char ItemPath[BUFSIZE];
	char buf[BUFSIZE];
	char tmp[BUFSIZE];
	HTREEITEM cItem;
	int ItemCnt, i;

	tpTreeInfo = (struct TPTREE *)TreeView_GetlParam(GetDlgItem(hWnd, WWWC_TREE), hItem);
	if(tpTreeInfo->ItemList == NULL){
		if(ReadTreeMem(hWnd, hItem) == FALSE){
			return;
		}
	}
	ItemCnt = tpTreeInfo->ItemListCnt;

	//アイテムのドロップファイル文字列を作成
	for(i = 0;i < ItemCnt;i++){
		if((*(tpTreeInfo->ItemList + i)) == NULL){
			continue;
		}
		CreateDropFile((*(tpTreeInfo->ItemList + i)), fPath, iPath, ItemPath);
	}

	//ドロップファイル用一時ディレクトリの作成
	cItem = TreeView_GetChild(GetDlgItem(hWnd, WWWC_TREE), hItem);
	while(cItem != NULL){
		TreeView_GetItemInfo(GetDlgItem(hWnd, WWWC_TREE), cItem, buf);
		wsprintf(tmp, "%s\\%s", iPath, buf);
		wsprintf(ItemPath, "%s\\%s", fPath, tmp);
		CreateDirectory(ItemPath, NULL);

		CreateTreeDropFiles(hWnd, cItem, fPath, tmp);

		cItem = TreeView_GetNextSibling(GetDlgItem(hWnd, WWWC_TREE), cItem);
	}
}


/******************************************************************************

	DragDrop_CreateTreeDropFiles

	ツリービューアイテムよりドロップファイルの文字列を作成

******************************************************************************/

BOOL DragDrop_CreateTreeDropFiles(HWND hWnd, HTREEITEM hItem, char *TempDirName)
{
	char DndPath[BUFSIZE];
	char ItemPath[BUFSIZE];
	char buf[BUFSIZE];

	WaitCursor(TRUE);

	DropFileCnt = 0;

	//ファイル名の配列を作成する
	if((DropFileNameList = (char **)GlobalAlloc(GPTR, sizeof(char *))) == NULL){
		ErrMsg(hWnd, GetLastError(), NULL);
		return FALSE;
	}

	//ドロップファイル用一時ディレクトリの作成
	wsprintf(DndPath, "%s\\%s", TempDir, TempDirName);
	if(GetDirSerch(DndPath) == TRUE){
		DeleteDirTree(DndPath, TRUE);
	}
	CreateDirectory(DndPath, NULL);

	//ドロップファイル用文字列の作成
	TreeView_GetItemInfo(GetDlgItem(hWnd, WWWC_TREE), hItem, buf);
	wsprintf(ItemPath, "%s\\%s", DndPath, buf);
	CreateDirectory(ItemPath, NULL);

	*DropFileNameList = (char *)GlobalAlloc(GPTR, lstrlen(ItemPath) + 1);
	if(*DropFileNameList == NULL){
		ErrMsg(hWnd, GetLastError(), NULL);
		GlobalFree(DropFileNameList);
		DropFileNameList = NULL;
		DropFileCnt = 0;
		return FALSE;
	}
	lstrcpy(*DropFileNameList, ItemPath);
	DropFileCnt = 1;

	//階層的にドロップファイルの作成
	CreateTreeDropFiles(hWnd, hItem, DndPath, buf);

	CallTreeItem(hWnd, hItem, (FARPROC)TreeView_FreeItem, 1);
	WaitCursor(FALSE);
	return TRUE;
}


/******************************************************************************

	DragDrop_CreateDropFiles

	リストビューの選択アイテムよりドロップファイルの文字列の作成

******************************************************************************/

BOOL DragDrop_CreateDropFiles(HWND hWnd, char *TempDirName)
{
	struct TPITEM *tpItemInfo;
	HTREEITEM hItem;
	char DndPath[BUFSIZE];
	char ItemPath[BUFSIZE];
	char buf[BUFSIZE];
	int SelectItem;

	WaitCursor(TRUE);

	//ファイル名の配列を作成する
	DropFileNameList = (char **)GlobalAlloc(GPTR, sizeof(char *) *
		ListView_GetSelectedCount(GetDlgItem(hWnd, WWWC_LIST)));
	if(DropFileNameList == NULL){
		ErrMsg(hWnd, GetLastError(), NULL);
		return FALSE;
	}

	wsprintf(DndPath, "%s\\%s", TempDir, TempDirName);
	if(GetDirSerch(DndPath) == TRUE){
		DeleteDirTree(DndPath, TRUE);
	}

	CreateDirectory(DndPath, NULL);

	DropFileCnt = 0;
	SelectItem = -1;
	while((SelectItem = ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), SelectItem, LVNI_SELECTED)) != -1){
		tpItemInfo = (struct TPITEM *)ListView_GetlParam(GetDlgItem(hWnd, WWWC_LIST), SelectItem);
		if(tpItemInfo == NULL){
			hItem = TreeView_GetIndexToItem(GetDlgItem(hWnd, WWWC_TREE),
				TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)), SelectItem);
			if(hItem == RecyclerItem){
				continue;
			}

			TreeView_GetItemInfo(GetDlgItem(hWnd, WWWC_TREE), hItem, buf);
			wsprintf(ItemPath, "%s\\%s", DndPath, buf);
			CreateDirectory(ItemPath, NULL);

			*(DropFileNameList + DropFileCnt) = (char *)GlobalAlloc(GPTR, lstrlen(ItemPath) + 1);
			if(*(DropFileNameList + DropFileCnt) == NULL){
				ErrMsg(hWnd, GetLastError(), NULL);
				GlobalFree(DropFileNameList);
				DropFileNameList = NULL;
				DropFileCnt = 0;
				return FALSE;
			}
			lstrcpy(*(DropFileNameList + DropFileCnt), ItemPath);
			DropFileCnt++;

			CreateTreeDropFiles(hWnd, hItem, DndPath, buf);

		}else{
			if(CreateDropFile(tpItemInfo, DndPath, NULL, ItemPath) == FALSE){
				continue;
			}

			*(DropFileNameList + DropFileCnt) = (char *)GlobalAlloc(GPTR, lstrlen(ItemPath) + 1);
			if(*(DropFileNameList + DropFileCnt) == NULL){
				ErrMsg(hWnd, GetLastError(), NULL);
				GlobalFree(DropFileNameList);
				DropFileNameList = NULL;
				DropFileCnt = 0;
				return FALSE;
			}
			lstrcpy(*(DropFileNameList + DropFileCnt), ItemPath);
			DropFileCnt++;
		}
	}

	CallTreeItem(hWnd, TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)), (FARPROC)TreeView_FreeItem, 1);
	if(DropFileCnt == 0){
		GlobalFree(DropFileNameList);
		DropFileNameList = NULL;
	}
	WaitCursor(FALSE);
	return TRUE;
}


/******************************************************************************

	FreeDropFiles

	ドロップファイルの文字列を解放

******************************************************************************/

void FreeDropFiles(void)
{
	int i;

	//ファイル名の配列を解放する
	for(i = 0;i < DropFileCnt;i++){
		GlobalFree(*(DropFileNameList + i));
	}
	GlobalFree(DropFileNameList);

	DropFileCnt = 0;
}


/******************************************************************************

	DeleteTmpDropFiles

	ドロップファイル用の一時ディレクトリの削除

******************************************************************************/

void DeleteTmpDropFiles(char *TempDirName)
{
	char DndPath[BUFSIZE];

	wsprintf(DndPath, "%s\\%s", TempDir, TempDirName);
	if(GetDirSerch(DndPath) == TRUE){
		DeleteDirTree(DndPath, TRUE);
	}
	RemoveDirectory(DndPath);
}


/******************************************************************************

	DragDrop_MouseMove

	ウィンドウ内でマウスが移動したときにマウスの下のアイテムを
	ハイライト表示にする

******************************************************************************/

static BOOL DragDrop_MouseMove(HWND hWnd)
{
	struct TPITEM *tpItemInfo;
	POINT pt;
	HWND hPointWnd;
	HTREEITEM hItem;
	int iItem;

	GetCursorPos(&pt);
	hPointWnd = WindowFromPoint(pt);

	if(hPointWnd == GetDlgItem(hWnd, WWWC_TREE)){			//TreeView
		ListView_SetItemState(GetDlgItem(hWnd, WWWC_LIST), -1, 0, LVIS_DROPHILITED);

		//マウスの下のTreeViewアイテムのハンドルを取得
		hItem = TreeView_GetHiTestItem(GetDlgItem(hWnd, WWWC_TREE));
		if(hItem == NULL){
			KillTimer(hWnd, TIMER_TREEEXPAND);
			TreeView_SelectDropTarget(GetDlgItem(hWnd, WWWC_TREE), NULL);
			return FALSE;
		}

		if((HTREEITEM)SendDlgItemMessage(hWnd, WWWC_TREE, TVM_GETNEXTITEM, TVGN_DROPHILITE, 0) == hItem){
			return TRUE;
		}

		//マウスの下のアイテムをハイライト状態にする
		TreeView_SelectDropTarget(GetDlgItem(hWnd, WWWC_TREE), hItem);

		//アイテムを画面上に表示させる
		if(TreeView_GetFirstVisible(GetDlgItem(hWnd, WWWC_TREE)) == hItem){
			TreeView_EnsureVisible(GetDlgItem(hWnd, WWWC_TREE),
				TreeView_GetPrevVisible(GetDlgItem(hWnd, WWWC_TREE), hItem));
		}else{
			TreeView_EnsureVisible(GetDlgItem(hWnd, WWWC_TREE), hItem);
		}

		//一定時間同じアイテムの場合は展開を行う
		KillTimer(hWnd, TIMER_TREEEXPAND);
		SetTimer(hWnd, TIMER_TREEEXPAND, 700, NULL);
		return TRUE;

	}else if(hPointWnd == GetDlgItem(hWnd, WWWC_LIST)){	//ListView
		KillTimer(hWnd, TIMER_TREEEXPAND);
		TreeView_SelectDropTarget(GetDlgItem(hWnd, WWWC_TREE), NULL);

		//マウスの下のListViewアイテムのインデックスを取得
		iItem = ListView_GetHiTestItem(GetDlgItem(hWnd, WWWC_LIST));
		if(iItem == -1){
			ListView_SetItemState(GetDlgItem(hWnd, WWWC_LIST), -1, 0, LVIS_DROPHILITED);
			return TRUE;
		}

		if(DragItemIndex != -1){
			if(ListView_GetItemState(GetDlgItem(hWnd, WWWC_LIST), iItem, LVIS_SELECTED) == LVIS_SELECTED){
				ListView_SetItemState(GetDlgItem(hWnd, WWWC_LIST), -1, 0, LVIS_DROPHILITED);
				return TRUE;
			}
		}

		//既にハイライトされている場合
		if(ListView_GetItemState(GetDlgItem(hWnd, WWWC_LIST), iItem, LVIS_DROPHILITED) == LVIS_DROPHILITED){
			return TRUE;
		}

		tpItemInfo = (struct TPITEM *)ListView_GetlParam(GetDlgItem(hWnd, WWWC_LIST), iItem);
		if(tpItemInfo != NULL){
			//フォルダではない場合はハイライトを行わない
			ListView_SetItemState(GetDlgItem(hWnd, WWWC_LIST), -1, 0, LVIS_DROPHILITED);

			//アイテムを画面上に表示させる
			ListView_EnsureVisible(GetDlgItem(hWnd, WWWC_LIST), iItem - 1, TRUE);
			ListView_EnsureVisible(GetDlgItem(hWnd, WWWC_LIST), iItem + 1, TRUE);
			return TRUE;
		}

		//マウスの下のアイテムをハイライト状態にする
		ListView_SetItemState(GetDlgItem(hWnd, WWWC_LIST), -1, 0, LVIS_DROPHILITED);
		ListView_SetItemState(GetDlgItem(hWnd, WWWC_LIST), iItem, LVIS_DROPHILITED, LVIS_DROPHILITED);

		//アイテムを画面上に表示させる
		ListView_EnsureVisible(GetDlgItem(hWnd, WWWC_LIST), iItem - 1, TRUE);
		ListView_EnsureVisible(GetDlgItem(hWnd, WWWC_LIST), iItem + 1, TRUE);
		return TRUE;

	}else{
		KillTimer(hWnd, TIMER_TREEEXPAND);

		TreeView_SelectDropTarget(GetDlgItem(hWnd, WWWC_TREE), NULL);
		ListView_SetItemState(GetDlgItem(hWnd, WWWC_LIST), -1, 0, LVIS_DROPHILITED);
	}
	return FALSE;
}


/******************************************************************************

	DragDrop_GetHitestFolder

	マウスの下のフォルダアイテムのハンドルを取得する

******************************************************************************/

static HTREEITEM DragDrop_GetHitestFolder(HWND hWnd)
{
	struct TPITEM *tpItemInfo;
	POINT pt;
	HWND hPointWnd;
	HTREEITEM hItem;
	int iItem;

	GetCursorPos(&pt);
	hPointWnd = WindowFromPoint(pt);

	if(hPointWnd == GetDlgItem(hWnd, WWWC_TREE)){			//TreeView
		hItem = TreeView_GetHiTestItem(GetDlgItem(hWnd, WWWC_TREE));
		if(hItem == NULL){
			return NULL;
		}

	}else if(hPointWnd == GetDlgItem(hWnd, WWWC_LIST)){	//ListView
		iItem = ListView_GetHiTestItem(GetDlgItem(hWnd, WWWC_LIST));
		if(iItem == -1){
			return TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE));
		}

		tpItemInfo = (struct TPITEM *)ListView_GetlParam(GetDlgItem(hWnd, WWWC_LIST), iItem);
		//フォルダではない場合は NULL を返す
		if(tpItemInfo != NULL){
			return TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE));
		}
		hItem = TreeView_GetIndexToItem(GetDlgItem(hWnd, WWWC_TREE),
			TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)), iItem);

	}else{
		return NULL;
	}
	return hItem;
}


/******************************************************************************

	DragDrop_ShowMenu

	ドラッグ＆ドロップ用のメニューを表示する

******************************************************************************/

static UINT DragDrop_ShowMenu(HWND hWnd, BOOL defFlag)
{
	UINT ret;
	UINT MenuEffect;

	//デフォルトのメニュー項目を設定
	if(defFlag == TRUE){
		SetMenuDefaultItem(GetSubMenu(hPOPUP, MENU_POP_DGDP), ID_MENU_DGDP_COPY, 0);
	}else{
		SetMenuDefaultItem(GetSubMenu(hPOPUP, MENU_POP_DGDP), ID_MENU_DGDP_CUT, 0);
	}

	ret = ShowMenuCommand(hWnd, hPOPUP, MENU_POP_DGDP);
	switch(ret)
	{
	case ID_MENU_DGDP_CUT:
		MenuEffect = DROPEFFECT_MOVE;
		break;

	case ID_MENU_DGDP_COPY:
		MenuEffect = DROPEFFECT_COPY;
		break;

	default:
		MenuEffect = DROPEFFECT_NONE;
		break;
	}
	return MenuEffect;
}


/******************************************************************************

	DragDrop_GetDropItem

	ドロップアイテム情報文字列よりアイテムを作成

******************************************************************************/

BOOL DragDrop_GetDropItem(HWND hWnd, HGLOBAL hMem, int dwEffect, HTREEITEM hItem)
{
	struct TPTREE *tpTreeInfo;
	POINT pt;
	HWND hPointWnd;
	char *buf;
	int ret;

	if(hMem == NULL){
		return FALSE;
	}

	WaitCursor(TRUE);

	GetCursorPos(&pt);
	hPointWnd = WindowFromPoint(pt);
	if(hItem == NULL){
		if(hPointWnd == GetDlgItem(hWnd, WWWC_LIST)){
			hItem = TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE));
		}else{
			WaitCursor(FALSE);
			return FALSE;
		}
	}

	SetFocus(hPointWnd);

	tpTreeInfo = (struct TPTREE *)TreeView_GetlParam(GetDlgItem(hWnd, WWWC_TREE), hItem);
	if(tpTreeInfo == NULL){
		WaitCursor(FALSE);
		return FALSE;
	}
	//アイテム情報を読み込む
	if(tpTreeInfo->ItemList == NULL){
		if(ReadTreeMem(hWnd, hItem) == FALSE){
			MessageBox(hWnd, EMSG_ITEMREAD, EMSG_ITEMREAD_TITLE, MB_ICONSTOP);
			WaitCursor(FALSE);
			return FALSE;
		}
	}

	//アイテム情報文字列からアイテムを作成
	if((buf = GlobalLock(hMem)) == NULL){
		WaitCursor(FALSE);
		return FALSE;
	}
	ret = Clipboard_Get_WF_String(hWnd, hItem, buf, dwEffect);
	GlobalUnlock(hMem);

	TreeView_SetIconState(hWnd, hItem, 0);
	TreeView_FreeItem(hWnd, hItem, 1);

	WaitCursor(FALSE);
	return ret;
}


/******************************************************************************

	DragDrop_GetDropItemFiles

	ドロップされたファイルをプロトコルDLLに処理させる

******************************************************************************/

void DragDrop_GetDropItemFiles(HWND hWnd, char *buf, HTREEITEM hItem)
{
	struct TPITEM *tpItemInfo;
	char funcName[BUFSIZE];
	char *p, *r;
	int ProtocolIndex;
	int pIndex;
	int ret;

	//拡張子を抽出
	p = buf;
	r = NULL;
	while(*p != '\0'){
		if(IsDBCSLeadByte((BYTE)*p) == TRUE){
			p++;
		}else if(*p == '.'){
			r = p;
		}
		p++;
	}
	if(r == NULL){
		return;
	}
	r++;
	//拡張子に対応したプロトコルを取得
	ProtocolIndex = -1;
	for(pIndex = 0;pIndex < ProtocolCnt;pIndex++){
		if(*(tpProtocol[pIndex].FileType) != '\0' && strlistcmp(r, tpProtocol[pIndex].FileType, '\t') == TRUE){
			ProtocolIndex = pIndex;
			break;
		}
	}
	if(ProtocolIndex == -1){
		struct TPITEM **tpFromItemInfo;
		int FromCnt, i;

		if(lstrcmpi(r, "dll") == 0){
			SelectDll(hWnd, buf);
			return;
		}

		if(lstrcmpi(r, "dat") != 0){
			return;
		}

		//転送元のファイルからアイテムリストを取得する
		tpFromItemInfo = ReadItemList(buf, &FromCnt, NULL);
		if(tpFromItemInfo == NULL){
			return;
		}
		UpdateItemFlag = UF_COPY;
		//転送先にアイテムを追加する
		for(i = 0;i < FromCnt;i++){
			if(*(tpFromItemInfo + i) == NULL){
				continue;
			}

			//アイテムの追加
			if(Item_Add(hWnd, hItem, *(tpFromItemInfo + i)) != -1){
				*(tpFromItemInfo + i) = NULL;
			}
			//コピーがキャンセルされている場合
			if(UpdateItemFlag == UF_CANCEL){
				break;
			}
		}
		FreeItemList(tpFromItemInfo, FromCnt, FALSE);
		GlobalFree(tpFromItemInfo);

		TreeView_SetIconState(hWnd, hItem, 0);
		TreeView_FreeItem(hWnd, hItem, 1);
		return;
	}
	//ドロップファイルを処理する関数を呼ぶ
	if((tpProtocol + ProtocolIndex)->Func_DropFile == NULL){
		wsprintf(funcName, "%sDropFile", (tpProtocol + ProtocolIndex)->FuncHeader);
		(tpProtocol + ProtocolIndex)->Func_DropFile = GetProcAddress((HMODULE)(tpProtocol + ProtocolIndex)->lib, funcName);
	}
	if((tpProtocol + ProtocolIndex)->Func_DropFile == NULL){
		return;
	}
	tpItemInfo = (struct TPITEM *)GlobalAlloc(GPTR, sizeof(struct TPITEM));
	if(tpItemInfo == NULL){
		return;
	}
	tpItemInfo->iSize = sizeof(struct TPITEM);
	tpItemInfo->hItem = hItem;
	tpItemInfo->CheckSt = DefNoCheck;
	ret = (tpProtocol + ProtocolIndex)->Func_DropFile(hWnd, r, buf, tpItemInfo);

	switch(ret)
	{
	case DROPFILE_NEWITEM:		//アイテムの追加
		if(Item_Add(hWnd, hItem, tpItemInfo) == -1){
			FreeItemInfo(tpItemInfo, FALSE);
			GlobalFree(tpItemInfo);
		}
		break;

	case DROPFILE_NONE:
	default:
		FreeItemInfo(tpItemInfo, FALSE);
		GlobalFree(tpItemInfo);
		break;
	}
}


/******************************************************************************

	DragDrop_GetDropTree

	ドロップされたディレクトリを階層的に処理する

******************************************************************************/

void DragDrop_GetDropTree(HWND hWnd, char *Path, HTREEITEM hItem)
{
	WIN32_FIND_DATA FindData;
	HANDLE hFindFile;
	char sPath[BUFSIZE];
	char fName[BUFSIZE];
	char buf[BUFSIZE];
	HTREEITEM cItem;

	if(GetPathToFilename(Path, fName) == FALSE){
		return;
	}

	//フォルダ名からTreeViewアイテムを取得
	if((cItem = TreeView_CheckName(GetDlgItem(hWnd, WWWC_TREE), hItem, fName)) == NULL){
		//追加先のフォルダを含むパスの場合はエラー
		TreeView_GetPath(GetDlgItem(hWnd, WWWC_TREE), hItem, buf, CuDir);
		if(lstrcmpni(Path, buf, lstrlen(Path)) == 0){
			char Msg[BUFSIZE * 2];

			wsprintf(Msg, EMSG_FOLDERCOPY_SUB, fName);
			MessageBox(hWnd, Msg, EMSG_FOLDERCOPY_TITLE, MB_ICONSTOP);
			return;
		}

		//フォルダの追加
		cItem = TreeView_AllocItem(GetDlgItem(hWnd, WWWC_TREE), hItem,
			(HTREEITEM)TVI_SORT, fName, ICON_DIR_CLOSE, ICON_DIR_OPEN);
		if(SucceedFromParent == 1){
			//親情報の継承
			CopyAutoTime(hWnd, cItem, (long)TreeView_GetlParam(GetDlgItem(hWnd, WWWC_TREE), hItem));
		}

		TreeView_GetPath(GetDlgItem(hWnd, WWWC_TREE), cItem, buf, CuDir);
		if(GetDirSerch(buf) == FALSE){
			//実フォルダ作成
			CreateDirectory(buf, NULL);
		}
	}

	InvalidateRect(GetDlgItem(hWnd, WWWC_TREE), NULL, FALSE);
	UpdateWindow(GetDlgItem(hWnd, WWWC_TREE));

	wsprintf(sPath, "%s\\*", Path);

	if((hFindFile = FindFirstFile(sPath, &FindData)) == INVALID_HANDLE_VALUE){
		return;
	}

	do{
		if(UpdateItemFlag == UF_CANCEL){
			break;
		}

		wsprintf(buf, "%s\\%s", Path, FindData.cFileName);

		if((FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0){
			DragDrop_GetDropItemFiles(hWnd, buf, cItem);
			continue;
		}

		if(lstrcmp(FindData.cFileName, ".") == 0 || lstrcmp(FindData.cFileName, "..") == 0){
			continue;
		}
		//再帰
		DragDrop_GetDropTree(hWnd, buf, cItem);
	} while(FindNextFile(hFindFile, &FindData) == TRUE);

	FindClose(hFindFile);
}


/******************************************************************************

	DragDrop_GetDropFiles

	ドロップファイルよりアイテムの作成

******************************************************************************/

void DragDrop_GetDropFiles(HWND hWnd, HDROP hDrop, HTREEITEM hItem)
{
	char *buf;
	int index;
	int len;
	int i, j;

	WaitCursor(TRUE);

	ListView_SetItemState(GetDlgItem(hWnd, WWWC_LIST), -1, 0, LVIS_SELECTED);

	index = ListView_GetItemCount(GetDlgItem(hWnd, WWWC_LIST));

	i = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
	for(j = 0;j < i;j++){
		if(UpdateItemFlag == UF_CANCEL){
			break;
		}
		len = DragQueryFile(hDrop, j, NULL, 0);
		buf = GlobalAlloc(GPTR, len + 1);
		if(buf == NULL){
			break;
		}
		DragQueryFile(hDrop, j, buf, len + 1);

		if(GetDirSerch(buf) == TRUE){
			DragDrop_GetDropTree(hWnd, buf, hItem);
		}else{
			DragDrop_GetDropItemFiles(hWnd, buf, hItem);
		}
		GlobalFree(buf);
	}

	ListView_RefreshFolder(hWnd);
	//追加アイテムの選択
	if(hItem == TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE))){
		if(ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), -1, LVNI_SELECTED) != -1){
			ListView_SetItemState(GetDlgItem(hWnd, WWWC_LIST),
				ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), -1, LVNI_SELECTED),
				LVIS_SELECTED | LVIS_FOCUSED, LVIS_FOCUSED | LVIS_SELECTED);
			ListView_EnsureVisible(GetDlgItem(hWnd, WWWC_LIST),
				ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), -1, LVNI_SELECTED), TRUE);
		}
	}

	TreeView_SetIconState(hWnd, hItem, 0);
	CallTreeItem(hWnd, hItem, (FARPROC)TreeView_FreeItem, 1);
	WaitCursor(FALSE);
}


/******************************************************************************

	DragDrop_NotifyProc

	ドラッグメッセージ

******************************************************************************/

long DragDrop_NotifyProc(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	LPIDROPTARGET_NOTIFY pdtn;
	HTREEITEM DnDItem;
	char *p;
	static int DnDKeySt,DnDCur;
	int tmp;

	pdtn = (LPIDROPTARGET_NOTIFY)lParam;

	switch(wParam){
	case IDROPTARGET_NOTIFY_DRAGENTER:
		DnDKeySt = pdtn->grfKeyState;
		DnDCur = FMT_NON;
		if(pdtn->cfFormat == WWWC_ClipFormat){
			DnDCur = FMT_WWWCITEM;
		}
		switch(pdtn->cfFormat)
		{
		case CF_TEXT:
			p = GlobalLock(pdtn->hMem);
			if(GetProtocolIndex(p) != -1){
				DnDCur = FMT_TEXT;
			}
			GlobalUnlock(pdtn->hMem);
			break;

		case CF_HDROP:
			DnDCur = FMT_HDROP;
			break;
		}

	case IDROPTARGET_NOTIFY_DRAGOVER:
		if(DnDCur != FMT_TEXT && DragDrop_MouseMove(hWnd) == FALSE && DnDCur != FMT_HDROP){
			pdtn->dwEffect = DROPEFFECT_NONE;
			break;
		}
		switch(DnDCur){
		case FMT_NON:
			pdtn->dwEffect = DROPEFFECT_NONE;
			break;

		case FMT_WWWCITEM:
			pdtn->dwEffect = DROPEFFECT_COPY;
			if(!(pdtn->grfKeyState & MK_CONTROL)){
				pdtn->dwEffect |= DROPEFFECT_MOVE;
			}

			tmp = ListView_GetHiTestItem(GetDlgItem(hWnd, WWWC_LIST));
			if(tmp == -1 || (struct TPITEM *)ListView_GetlParam(GetDlgItem(hWnd, WWWC_LIST), tmp) != NULL){
				break;
			}
			//同一フォルダの場合は禁止カーソルにする
			if(DragItemIndex != -1){
				if(ListView_GetItemState(GetDlgItem(hWnd, WWWC_LIST), tmp, LVIS_SELECTED) == LVIS_SELECTED){
					pdtn->dwEffect = DROPEFFECT_NONE;
				}
			}
			break;

		case FMT_TEXT:
			pdtn->dwEffect = DROPEFFECT_COPY | DROPEFFECT_LINK;
			break;

		case FMT_HDROP:
			pdtn->dwEffect = DROPEFFECT_COPY;
			break;
		}
		break;

	case IDROPTARGET_NOTIFY_DRAGLEAVE:
		TreeView_Select(GetDlgItem(hWnd, WWWC_TREE), NULL, TVGN_DROPHILITE);
		ListView_SetItemState(GetDlgItem(hWnd, WWWC_LIST), -1, 0, LVIS_DROPHILITED);
		KillTimer(hWnd, TIMER_TREEEXPAND);
		break;

	case IDROPTARGET_NOTIFY_DROP:
		KillTimer(hWnd, TIMER_TREEEXPAND);

		DnDItem = DragDrop_GetHitestFolder(hWnd);
		UpdateItemFlag = UF_COPY;

		if(pdtn->cfFormat == WWWC_ClipFormat){
			if(DnDKeySt & MK_RBUTTON){
				pdtn->dwEffect = DragDrop_ShowMenu(hWnd, pdtn->dwEffect == DROPEFFECT_COPY);
			}
			if(pdtn->dwEffect == DROPEFFECT_NONE){
				TreeView_Select(GetDlgItem(hWnd, WWWC_TREE), NULL, TVGN_DROPHILITE);
				ListView_SetItemState(GetDlgItem(hWnd, WWWC_LIST), -1, 0, LVIS_DROPHILITED);
				break;
			}
			tmp = (pdtn->dwEffect == DROPEFFECT_MOVE) ? DND_MOVE : DND_COPY;
			if(DnDItem == RecyclerItem){
				UpdateItemFlag = UF_NOMSG;
				tmp |= DND_RECY;
			}
			_SetForegroundWindow(hWnd);
			if(DragDrop_GetDropItem(hWnd, pdtn->hMem, tmp, DnDItem) == FALSE){
				pdtn->dwEffect = DROPEFFECT_NONE;
			}

		}else{
			switch(pdtn->cfFormat)
			{
			case CF_TEXT:
				p = GlobalLock(pdtn->hMem);
				if(GetProtocolIndex(p) != -1){
					_SetForegroundWindow(hWnd);
					Item_UrlAdd(hWnd, NEWITEMNAME, p, 0, NULL);
				}
				GlobalUnlock(pdtn->hMem);
				break;

			case CF_HDROP:
				if(DnDItem == NULL){
					DnDItem = TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE));
				}
				_SetForegroundWindow(hWnd);
				DragDrop_GetDropFiles(hWnd, pdtn->hMem, DnDItem);
				DragFinish(pdtn->hMem);
				break;
			}
		}
		TreeView_Select(GetDlgItem(hWnd, WWWC_TREE), NULL, TVGN_DROPHILITE);
		ListView_SetItemState(GetDlgItem(hWnd, WWWC_LIST), -1, 0, LVIS_DROPHILITED);
		break;
	}
	return 0;
}
/* End of source */
