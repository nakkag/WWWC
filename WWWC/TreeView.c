/**************************************************************************

	WWWC

	TreeView.c

	Copyright (C) 1996-2008 by Nakashima Tomoaki. All rights reserved.
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

#define HISTORYCNT					100


/**************************************************************************
	Global Variables
**************************************************************************/

HTREEITEM RootItem;
HTREEITEM RecyclerItem;
HTREEITEM HistoryItem[HISTORYCNT + 1];
int HistoryIndex;
BOOL HistoryFlag;

//外部参照
extern HINSTANCE g_hinst;			//アプリケーションのインスタンスハンドル
extern HWND WWWCWnd;				//本体
extern HTREEITEM DgdpItem;
extern char CuDir[];
extern int gCheckFlag;
extern BOOL WWWCDropFlag;
extern UINT WWWC_ClipFormat;

extern char RootTitle[];
extern char RecyclerTitle[];
extern int SucceedFromParent;
extern int TvWndStyle;
extern char TvBkColor[];
extern int TvIconSize;

extern char Check[];
extern int CheckIndex;
extern char NoCheck[];
extern int NoCheckIndex;
extern char DirUP[];
extern int DirUPIndex;
extern char Dir[];
extern int DirIndex;
extern char DirUPchild[];
extern int DirUPchildIndex;
extern char CheckChild[];
extern int CheckChildIndex;
extern char Recycler[];
extern int RecyclerIndex;
extern char RecyclerFull[];
extern int RecyclerFullIndex;

extern char Inet[];
extern int InetIndex;
extern char DirOpen[];
extern int DirOpenIndex;

extern int DragItemIndex;


/**************************************************************************
	Local Function Prototypes
**************************************************************************/

static HTREEITEM TreeView_SetNewItem(HWND hTreeView, char *buf, HTREEITEM SetItem,
									 int Icon, int IconSel, HTREEITEM After);
static BOOL TreeView_SetItemText(HWND hTreeView, HTREEITEM hItem, char *buf);
static BOOL TreeView_SetIcon(HWND hTreeView, HTREEITEM hItem, int Icon, int SelIcon);
static void TreeView_SetParentIcon(HWND hWnd, HTREEITEM hItem, int Icon);
static void TreeView_DeleteParentIcon(HWND hWnd, HTREEITEM hItem, int Icon1, int Icon2);
static void TreeView_SetHistory(HTREEITEM NewItem);


/******************************************************************************

	CallTreeItem

	すべてのツリビューのアイテムに指定関数を実行

******************************************************************************/

void CallTreeItem(HWND hWnd, HTREEITEM hItem, FARPROC Func, long Param)
{
	HTREEITEM cItem;

	//関数の呼び出し
	Func(hWnd, hItem, Param);

	cItem = TreeView_GetChild(GetDlgItem(hWnd, WWWC_TREE), hItem);
	while(cItem != NULL){
		//再帰
		CallTreeItem(hWnd, cItem, Func, Param);
		cItem = TreeView_GetNextSibling(GetDlgItem(hWnd, WWWC_TREE), cItem);
	}
}


/******************************************************************************

	CreateTreeView

	ツリービューの作成

******************************************************************************/

HWND CreateTreeView(HWND hWnd)
{
	return CreateWindowEx(WS_EX_NOPARENTNOTIFY,
		WC_TREEVIEW, (LPSTR)NULL, WS_VISIBLE | WS_CHILD | WS_TABSTOP | TvWndStyle,
		0, 0, 0, 0, hWnd, (HMENU)WWWC_TREE, g_hinst, NULL);
}


/******************************************************************************

	TreeView_Initialize

	ツリービューの初期化

******************************************************************************/

void TreeView_Initialize(HWND hWnd)
{
	HIMAGELIST IconList;
	HIMAGELIST TmpIconList;
	HICON TmpIcon;

	IconList = ImageList_Create(TvIconSize, TvIconSize, ILC_COLOR16 | ILC_MASK, 0, 0);
	if(*TvBkColor != '\0'){
		ImageList_SetBkColor(IconList, strtol(TvBkColor, NULL, 0));
	}else{
		ImageList_SetBkColor(IconList, GetSysColor(COLOR_WINDOW));
	}
	//general
	ImageListIconAdd(IconList, IDI_ICON_CHECK, TvIconSize, Check, CheckIndex);
	ImageListIconAdd(IconList, IDI_ICON_NOCHECK, TvIconSize, NoCheck, NoCheckIndex);
	ImageList_SetOverlayImage(IconList, ICON_NOCHECK, 1);
	ImageListIconAdd(IconList, IDI_ICON_DIRUP, TvIconSize, DirUP, DirUPIndex);
	ImageListFileIconAdd(IconList, CuDir, 0, TvIconSize, Dir, DirIndex);
	ImageListIconAdd(IconList, IDI_ICON_UPCHILD, TvIconSize, DirUPchild, DirUPchildIndex);
	ImageListIconAdd(IconList, IDI_ICON_CHECKCHILD, TvIconSize, CheckChild, CheckChildIndex);

	TmpIconList = ImageList_Merge(IconList, ICON_DIR_CLOSE, IconList, ICON_CHECK, 0, 0);
	TmpIcon = ImageList_GetIcon(TmpIconList, 0, ILD_NORMAL);
	ImageList_AddIcon(IconList, TmpIcon);
	DestroyIcon(TmpIcon);
	ImageList_Destroy((void *)TmpIconList);

	TmpIconList = ImageList_Merge(IconList, ICON_DIR_CLOSE, IconList, ICON_DIRUP, 0, 0);
	TmpIcon = ImageList_GetIcon(TmpIconList, 0, ILD_NORMAL);
	ImageList_AddIcon(IconList, TmpIcon);
	DestroyIcon(TmpIcon);
	ImageList_Destroy((void *)TmpIconList);

	TmpIconList = ImageList_Merge(IconList, ICON_DIR_CLOSE, IconList, ICON_DIRUPCHILD, 0, 0);
	TmpIcon = ImageList_GetIcon(TmpIconList, 0, ILD_NORMAL);
	ImageList_AddIcon(IconList, TmpIcon);
	DestroyIcon(TmpIcon);
	ImageList_Destroy((void *)TmpIconList);

	TmpIconList = ImageList_Merge(IconList, ICON_DIR_CLOSE, IconList, ICON_CHECKCHILD, 0, 0);
	TmpIcon = ImageList_GetIcon(TmpIconList, 0, ILD_NORMAL);
	ImageList_AddIcon(IconList, TmpIcon);
	DestroyIcon(TmpIcon);
	ImageList_Destroy((void *)TmpIconList);

	ImageListIconAdd(IconList, IDI_ICON_RECYCLER, TvIconSize, Recycler, RecyclerIndex);
	ImageListIconAdd(IconList, IDI_ICON_RECYCLER_USE, TvIconSize, RecyclerFull, RecyclerFullIndex);

	//TreeView
	ImageListIconAdd(IconList, IDI_ICON_INET, TvIconSize, Inet, InetIndex);

	ImageListFileIconAdd(IconList, CuDir, SHGFI_OPENICON, TvIconSize, DirOpen, DirOpenIndex);

	TmpIconList = ImageList_Merge(IconList, ICON_DIR_OPEN, IconList, ICON_CHECK, 0, 0);
	TmpIcon = ImageList_GetIcon(TmpIconList, 0, ILD_NORMAL);
	ImageList_AddIcon(IconList, TmpIcon);
	DestroyIcon(TmpIcon);
	ImageList_Destroy((void *)TmpIconList);

	TmpIconList = ImageList_Merge(IconList, ICON_DIR_OPEN, IconList, ICON_DIRUP, 0, 0);
	TmpIcon = ImageList_GetIcon(TmpIconList, 0, ILD_NORMAL);
	ImageList_AddIcon(IconList, TmpIcon);
	DestroyIcon(TmpIcon);
	ImageList_Destroy((void *)TmpIconList);

	TmpIconList = ImageList_Merge(IconList, ICON_DIR_OPEN, IconList, ICON_DIRUPCHILD, 0, 0);
	TmpIcon = ImageList_GetIcon(TmpIconList, 0, ILD_NORMAL);
	ImageList_AddIcon(IconList, TmpIcon);
	DestroyIcon(TmpIcon);
	ImageList_Destroy((void *)TmpIconList);

	TmpIconList = ImageList_Merge(IconList, ICON_DIR_OPEN, IconList, ICON_CHECKCHILD, 0, 0);
	TmpIcon = ImageList_GetIcon(TmpIconList, 0, ILD_NORMAL);
	ImageList_AddIcon(IconList, TmpIcon);
	DestroyIcon(TmpIcon);
	ImageList_Destroy((void *)TmpIconList);

	TreeView_SetImageList(GetDlgItem(hWnd, WWWC_TREE), IconList, TVSIL_NORMAL);

	//ルートアイテム
	RootItem = TreeView_AllocItem(GetDlgItem(hWnd, WWWC_TREE), (HTREEITEM)TVI_ROOT,
		(HTREEITEM)TVI_LAST, RootTitle, ICON_INET, ICON_INET);
	GetDirInfo(GetDlgItem(hWnd, WWWC_TREE), CuDir, RootItem);

	//ごみ箱
	RecyclerItem = TreeView_AllocItem(GetDlgItem(hWnd, WWWC_TREE), (HTREEITEM)RootItem,
		(HTREEITEM)TVI_LAST, RecyclerTitle, ICON_DIR_RECYCLER, ICON_DIR_RECYCLER);

	//実ディレクトリ構造からフォルダの作成
	SetDirTree(GetDlgItem(hWnd, WWWC_TREE), CuDir, RootItem);

	//ごみ箱の状態を更新
	TreeView_SetIconState(hWnd, RecyclerItem, 0);
}


/******************************************************************************

	TreeView_SetNewItem

	ツリービューアイテムの追加

******************************************************************************/

static HTREEITEM TreeView_SetNewItem(HWND hTreeView, char *buf, HTREEITEM SetItem,
									 int Icon, int IconSel, HTREEITEM After)
{
	TV_INSERTSTRUCT tvitn = { 0 };
	TV_ITEM tvit = { 0 };

	tvit.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
	tvit.cchTextMax = BUFSIZE - 1;
	tvitn.hInsertAfter = After;
	tvit.iImage = Icon;
	tvit.iSelectedImage = IconSel;
	tvit.hItem = NULL;
	tvit.state = 0;
	tvit.stateMask = 0;
	tvit.cChildren = 0;
	tvit.lParam = 0;
	tvit.pszText = buf;

	tvitn.hParent = SetItem;
	tvitn.item = tvit;
	return TreeView_InsertItem(hTreeView, &tvitn);
}


/******************************************************************************

	TreeView_AllocItem

	ツリー情報の確保

******************************************************************************/

HTREEITEM TreeView_AllocItem(HWND hTreeView, HTREEITEM pItem, HTREEITEM After, char *Title,
							 int Icon, int IconSel)
{
	struct TPTREE *tpTreeInfo;
	HTREEITEM NewItem;

	tpTreeInfo = (struct TPTREE *)GlobalAlloc(GPTR, sizeof(struct TPTREE));
	if(tpTreeInfo == NULL){
		abort();
	}
	tpTreeInfo->ItemList = NULL;
	tpTreeInfo->ItemListCnt = 0;
	tpTreeInfo->CheckFlag = 0;
	tpTreeInfo->MemFlag = 0;
	tpTreeInfo->Icon = 0;
	tpTreeInfo->CheckSt = 0;
	tpTreeInfo->AutoCheckSt = 1;
	tpTreeInfo->Comment = NULL;

	NewItem = TreeView_SetNewItem(hTreeView, Title, pItem, Icon, IconSel, (HTREEITEM)After);

	//ツリービューアイテムのlParamにツリー情報のアドレスをいれる
	TreeView_SetlParam(hTreeView, NewItem, (long)tpTreeInfo);

	return NewItem;
}


/******************************************************************************

	TreeView_NewFolderItem

	ツリービューに新しいフォルダを追加する

******************************************************************************/

BOOL TreeView_NewFolderItem(HWND hWnd)
{
	HTREEITEM hItem;
	char name[BUFSIZE];
	char buf[BUFSIZE];
	char path[BUFSIZE];
	int i;

	//新しいフォルダの名前を作成して実際にフォルダを作成する
	TreeView_GetPath(GetDlgItem(hWnd, WWWC_TREE),
		TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)), buf, CuDir);
	lstrcpy(name, NEWFOLDER);
	wsprintf(path, "%s\\%s", buf, name);

	//既にフォルダが存在している場合は番号を付ける
	i = 1;
	while(CreateDirectory(path, NULL) == FALSE){
		i++;
		wsprintf(name, "%s (%d)", NEWFOLDER, i);
		wsprintf(path, "%s\\%s", buf, name);
	}

	//ツリーにフォルダを追加する
	hItem = TreeView_AllocItem(GetDlgItem(hWnd, WWWC_TREE),
		TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)),
		(HTREEITEM)TVI_SORT, name, ICON_DIR_CLOSE, ICON_DIR_OPEN);

	if(SucceedFromParent == 1){
		//親情報の継承
		CopyAutoTime(hWnd, hItem, (long)TreeView_GetlParam(GetDlgItem(hWnd, WWWC_TREE),
			TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE))));
	}

	if(hItem == NULL){
		return FALSE;
	}

	InvalidateRect(GetDlgItem(hWnd, WWWC_TREE), NULL, FALSE);
	UpdateWindow(GetDlgItem(hWnd, WWWC_TREE));

	if(GetFocus() == GetDlgItem(hWnd, WWWC_TREE)){
		//ツリーにフォーカスがある場合は、追加したアイテムを選択してラベルを編集モードにする
		TreeView_SelectItem(GetDlgItem(hWnd, WWWC_TREE), hItem);
		TreeView_EditLabel(GetDlgItem(hWnd, WWWC_TREE), hItem);
		return TRUE;
	}
	if(GetFocus() == GetDlgItem(hWnd, WWWC_LIST)){
		//リストビューにフォーカスがある場合は、リストにフォルダを追加して編集モードにする
		ListView_RefreshFolder(hWnd);

		if((i = TreeView_GetListIndex(GetDlgItem(hWnd, WWWC_TREE), hItem)) != -1){
			//リストビューアイテムを編集モードにする
			ListView_EditLabel(GetDlgItem(hWnd, WWWC_LIST), i);
		}
	}
	return TRUE;
}


/******************************************************************************

	TreeView_FreeItem

	ツリービューアイテムに関連付けられたアイテム情報の解放

******************************************************************************/

void CALLBACK TreeView_FreeItem(HWND hWnd, HTREEITEM hItem, long Param)
{
	struct TPTREE *tpTreeInfo;
	int i;

	if(Param == 1){
		WaitCursor(TRUE);
		//アイテム情報の保存
		SaveTreeMem(hWnd, GetDlgItem(hWnd, WWWC_TREE), hItem);
		WaitCursor(FALSE);
	}

	//現在開いているフォルダの場合は関数を抜ける
	if(Param != 2 && hItem == TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE))){
		return;
	}

	//ツリー情報の取得
	tpTreeInfo = (struct TPTREE *)TreeView_GetlParam(GetDlgItem(hWnd, WWWC_TREE), hItem);
	if(tpTreeInfo == NULL){
		return;
	}
	if(tpTreeInfo->ItemList == NULL){
		return;
	}

	//フォルダのプロパティを開いているなどの状況
	if(tpTreeInfo->MemFlag > 0){
		return;
	}
	tpTreeInfo->MemFlag = 0;

	//チェック中の場合は関数を抜ける
	for(i = 0;i < tpTreeInfo->ItemListCnt;i++){
		if((*(tpTreeInfo->ItemList + i)) == NULL){
			continue;
		}
		if((*(tpTreeInfo->ItemList + i))->IconStatus == ST_CHECK ||
			(*(tpTreeInfo->ItemList + i))->IconStatus == ST_NOCHECK){
			return;
		}
	}

	//アイテムリストの解放
	FreeItemList(tpTreeInfo->ItemList, tpTreeInfo->ItemListCnt, TRUE);
	GlobalFree(tpTreeInfo->ItemList);
	tpTreeInfo->ItemList = NULL;
	tpTreeInfo->ItemListCnt = 0;
}


/******************************************************************************

	TreeView_FreeTreeItem

	ツリー情報の解放

******************************************************************************/

void CALLBACK TreeView_FreeTreeItem(HWND hWnd, HTREEITEM hItem, long Param)
{
	struct TPTREE *tpTreeInfo;

	//ツリー情報の取得
	tpTreeInfo = (struct TPTREE *)TreeView_GetlParam(GetDlgItem(hWnd, WWWC_TREE), hItem);
	if(tpTreeInfo == NULL){
		return;
	}
	tpTreeInfo->MemFlag = 0;

	//ツリー情報の取得
	if(Param == 0){
		SaveTreeMem(hWnd, GetDlgItem(hWnd, WWWC_TREE), hItem);
	}

	//アイテムリストの解放
	if(tpTreeInfo->ItemList != NULL){
		FreeItemList(tpTreeInfo->ItemList, tpTreeInfo->ItemListCnt, TRUE);
		GlobalFree(tpTreeInfo->ItemList);
		tpTreeInfo->ItemList = NULL;
		tpTreeInfo->ItemListCnt = 0;
	}

	//自動チェック情報の解放
	if(tpTreeInfo->tpCheckTime != NULL){
		GlobalFree(tpTreeInfo->tpCheckTime);
		tpTreeInfo->tpCheckTime = NULL;
		tpTreeInfo->tpCheckTimeCnt = 0;
	}

	//フォルダのコメントの解放
	if(tpTreeInfo->Comment != NULL){
		GlobalFree(tpTreeInfo->Comment);
		tpTreeInfo->Comment = NULL;
	}

	//ツリー情報の解放
	GlobalFree(tpTreeInfo);
	TreeView_SetlParam(GetDlgItem(hWnd, WWWC_TREE), hItem, (long)NULL);
}


/******************************************************************************

	TreeView_DeleteTreeInfo

	ツリービューアイテムの削除

******************************************************************************/

void TreeView_DeleteTreeInfo(HWND hWnd, HTREEITEM hItem)
{
	HTREEITEM pItem;
	char buf[BUFSIZE];

	//実ディレクトリの削除
	TreeView_GetPath(GetDlgItem(hWnd, WWWC_TREE), hItem, buf, CuDir);
	DeleteDirTree(buf, FALSE);
	RemoveDirectory(buf);

	//フォルダの削除
	CallTreeItem(hWnd, hItem, (FARPROC)TreeView_FreeTreeItem, 1);
	pItem = TreeView_GetParent(GetDlgItem(hWnd, WWWC_TREE), hItem);
	TreeView_DeleteItem(GetDlgItem(hWnd, WWWC_TREE), hItem);
	TreeView_SetIconState(hWnd, pItem, 0);
}


/******************************************************************************

	TreeView_AllExpand

	アイテムを展開する

******************************************************************************/

void CALLBACK TreeView_AllExpand(HWND hWnd, HTREEITEM hItem, long Param)
{
	struct TPTREE *tpTreeInfo;

	if(Param == 1){
		TreeView_Expand(GetDlgItem(hWnd, WWWC_TREE), hItem, TVE_EXPAND);
	}else{
		//ツリー情報の取得
		tpTreeInfo = (struct TPTREE *)TreeView_GetlParam(GetDlgItem(hWnd, WWWC_TREE), hItem);
		if(tpTreeInfo == NULL){
			return;
		}
		if(tpTreeInfo->Expand != 0){
			TreeView_Expand(GetDlgItem(hWnd, WWWC_TREE), hItem, TVE_EXPAND);
		}
	}
}


/******************************************************************************

	TreeView_FindItemPath

	パスからアイテムを検索

******************************************************************************/

HTREEITEM TreeView_FindItemPath(HWND hTreeView, HTREEITEM hItem, char *path)
{
	HTREEITEM cItem;
	char *p, *r;
	char buf[BUFSIZE];

	if(*path == '\0'){
		return NULL;
	}

	p = path;

	//ルートの場合
	if((*p == '\\' && *(p + 1) == '\\') || (*p == '/' && *(p + 1) == '/')){
		for(p = path + 2; *p != '\0'; p++){
			if(IsDBCSLeadByte((BYTE)*p) == FALSE){
				if(*p == '\\' || *p == '/'){
					break;
				}
			}else{
				p++;
			}
		}
	}

	if(*p == '\\' || *p == '/'){
		p++;
	}
	if(*p == '\0'){
		return hItem;
	}

	//パスを展開
	for(r = buf; *p != '\0'; p++, r++){
		if(IsDBCSLeadByte((BYTE)*p) == FALSE){
			if(*p == '\\' || *p == '/'){
				break;
			}
			*r = *p;
		}else{
			*(r++) = *(p++);
			*r = *p;
		}
	}
	*r = '\0';

	//名前からアイテムを取得
	cItem = TreeView_CheckName(hTreeView, hItem, buf);
	if(cItem == NULL){
		return NULL;
	}

	if(*p != '\0'){
		//再帰
		return TreeView_FindItemPath(hTreeView, cItem, p);
	}
	return cItem;
}


/******************************************************************************

	FindTreeItem

	アイテムか存在するか調べる

******************************************************************************/

void CALLBACK FindTreeItem(HWND hWnd, HTREEITEM hItem, long Param)
{
	HTREEITEM pItem;

	pItem = *((HTREEITEM *)Param);

	if(pItem == hItem){
		*((HTREEITEM *)Param) = NULL;
	}
}
BOOL IsTreeItem(HWND hWnd, HTREEITEM hItem)
{
	HTREEITEM RetItem;

	RetItem = hItem;
	CallTreeItem(WWWCWnd, RootItem, (FARPROC)FindTreeItem, (long)&RetItem);
	if(RetItem != NULL){
		return FALSE;
	}
	return TRUE;
}


/******************************************************************************

	TreeView_CheckName

	指定の名前のツリービューアイテムのハンドルを取得

******************************************************************************/

HTREEITEM TreeView_CheckName(HWND hTreeView, HTREEITEM hItem, char *str)
{
	HTREEITEM cItem;
	char buf[BUFSIZE];

	cItem = TreeView_GetChild(hTreeView, hItem);
	while(cItem != NULL){
		TreeView_GetItemInfo(hTreeView, cItem, buf);
		if(lstrcmpi(str, buf) == 0){
			return cItem;
		}
		cItem = TreeView_GetNextSibling(hTreeView, cItem);
	}
	return NULL;
}


/******************************************************************************

	TreeView_GetChildCount

	ツリービューアイテムの子供数を取得

******************************************************************************/

int TreeView_GetChildCount(HWND hTreeView, HTREEITEM hItem)
{
	HTREEITEM cItem;
	int ret = 0;

	//ツリービューに表示されているフォルダの数
	cItem = TreeView_GetChild(hTreeView, hItem);
	while(cItem != NULL){
		ret++;
		cItem = TreeView_GetNextSibling(hTreeView, cItem);
	}
	return ret;
}


/******************************************************************************

	TreeView_IsRecyclerItem

	ツリービューアイテムがごみ箱かごみ箱の中のアイテムなのかチェックする

******************************************************************************/

BOOL TreeView_IsRecyclerItem(HWND hTreeView, HTREEITEM hItem)
{
	HTREEITEM cItem;

	cItem = hItem;
	while(cItem != RootItem){
		if(cItem == RecyclerItem){
			//ごみ箱の場合
			return TRUE;
		}
		cItem = TreeView_GetParent(hTreeView, cItem);
	}
	return FALSE;
}


/******************************************************************************

	TreeView_GetPath

	ツリービューアイテムのパスを取得する

******************************************************************************/

void TreeView_GetPath(HWND hTreeView, HTREEITEM hItem, char *ret, char *RootString)
{
	HTREEITEM pItem;
	char buf[BUFSIZE];
	char tvName[BUFSIZE];
	char work[BUFSIZE];

	//ルートアイテムの場合はそのままパスを返す
	if(hItem == RootItem){
		lstrcpy(ret, RootString);
		return;
	}

	if(TreeView_GetItemInfo(hTreeView, hItem, buf) == -1){
		*ret = '\0';
		return;
	}
	pItem = TreeView_GetParent(hTreeView, hItem);
	while(pItem != RootItem){
		TreeView_GetItemInfo(hTreeView, pItem, tvName);
		wsprintf(work, "%s\\%s", tvName, buf);
		lstrcpy(buf, work);

		pItem = TreeView_GetParent(hTreeView, pItem);
	}
	//指定の文字列と結合
	wsprintf(ret, "%s\\%s", RootString, buf);
}


/******************************************************************************

	TreeView_SetItemText

	アイテムのタイトルを設定

******************************************************************************/

static BOOL TreeView_SetItemText(HWND hTreeView, HTREEITEM hItem, char *buf)
{
	TV_ITEM tvItem = { 0 };

	tvItem.mask = TVIF_TEXT;
	tvItem.hItem = hItem;
	tvItem.cchTextMax = BUFSIZE - 1;
	tvItem.pszText = buf;

	return TreeView_SetItem(hTreeView, &tvItem);
}


/******************************************************************************

	TreeView_SetName

	ディレクトリ名とツリービューアイテムの名前を設定する

******************************************************************************/

BOOL TreeView_SetName(HWND hWnd, HTREEITEM hItem, char *NewName)
{
	char buf[BUFSIZE];
	char Tmpbuf[BUFSIZE];
	char OldFileName[BUFSIZE];
	char NewFileName[BUFSIZE];

	//不正なファイル名の場合はエラーを出す
	if(FileNameCheck(NewName) == FALSE){
		MessageBox(hWnd, EMSG_CHANGEFILENAME, EMSG_CHANGEFILENAME_TITLE, MB_ICONSTOP);
		return FALSE;
	}

	//パスを取得する
	*buf = '\0';
	TreeView_GetPath(GetDlgItem(hWnd, WWWC_TREE),
		TreeView_GetParent(GetDlgItem(hWnd, WWWC_TREE), hItem), buf, CuDir);

	//古いパスと新しいパスを作成する
	TreeView_GetItemInfo(GetDlgItem(hWnd, WWWC_TREE), hItem, Tmpbuf);
	wsprintf(OldFileName, "%s\\%s", buf, Tmpbuf);
	wsprintf(NewFileName, "%s\\%s", buf, NewName);

	//ファイル名を変更する
	if(MoveFile(OldFileName, NewFileName) == FALSE){
		//失敗した場合はエラーを出力する
		ErrMsg(hWnd, GetLastError(), EMSG_CHANGEFILENAME_TITLE);
		return FALSE;
	}

	//ツリーのタイトルを変更してソートする
	TreeView_SetItemText(GetDlgItem(hWnd, WWWC_TREE), hItem, NewName);
	TreeView_SortChildren(GetDlgItem(hWnd, WWWC_TREE),
		TreeView_GetParent(GetDlgItem(hWnd, WWWC_TREE), hItem), 0);

	//Undoバッファにセット
	SetTVTitleToUndo(hItem, Tmpbuf);
	return TRUE;
}


/******************************************************************************

	TreeView_GetItemInfo

	アイテムのタイトルとアイコンを取得

******************************************************************************/

int TreeView_GetItemInfo(HWND hTreeView, HTREEITEM hItem, char *buf)
{
	TV_ITEM tvItem = { 0 };

	tvItem.mask = TVIF_TEXT | TVIF_IMAGE;
	tvItem.hItem = hItem;
	tvItem.cchTextMax = BUFSIZE - 1;
	tvItem.pszText = buf;
	tvItem.iImage = 0;
	tvItem.iSelectedImage = 0;

	if(TreeView_GetItem(hTreeView, &tvItem) == FALSE){
		return -1;
	}
	return tvItem.iImage;
}


/******************************************************************************

	TreeView_GetListIndex

	アイテムの位置を取得

******************************************************************************/

int TreeView_GetListIndex(HWND hTreeView, HTREEITEM hItem)
{
	HTREEITEM pItem, cItem;
	int i;

	pItem = TreeView_GetParent(hTreeView, hItem);
	cItem = TreeView_GetChild(hTreeView, pItem);
	i = 0;
	while(cItem != NULL){
		if(cItem == hItem){
			return i;
		}
		cItem = TreeView_GetNextSibling(hTreeView, cItem);
		i++;
	}
	return -1;
}


/******************************************************************************

	TreeView_GetIndexToItem

	指定位置のアイテムを取得

******************************************************************************/

HTREEITEM TreeView_GetIndexToItem(HWND hTreeView, HTREEITEM hItem, int Index)
{
	HTREEITEM cItem;
	int i;

	i = 0;
	cItem = TreeView_GetChild(hTreeView, hItem);
	while(cItem != NULL){
		if(Index == i){
			return cItem;
		}
		i++;
		cItem = TreeView_GetNextSibling(hTreeView, cItem);
	}
	return NULL;
}


/******************************************************************************

	TreeView_GetHiTestItem

	マウスの下のアイテムを取得

******************************************************************************/

HTREEITEM TreeView_GetHiTestItem(HWND hTreeView)
{
	TV_HITTESTINFO tvht = { 0 };
	POINT apos;
	RECT TvRect;

	if(hTreeView == NULL){
		return NULL;
	}

	//マウスの位置を取得
	GetCursorPos((LPPOINT)&apos);
	GetWindowRect(hTreeView, (LPRECT)&TvRect);
	apos.x = apos.x - TvRect.left;
	apos.y = apos.y - TvRect.top;

	tvht.pt = apos;
	tvht.flags = TVHT_NOWHERE;
	tvht.hItem = NULL;
	return TreeView_HitTest(hTreeView, &tvht);
}


/******************************************************************************

	TreeView_SetlParam

	アイテムに情報を関連つける

******************************************************************************/

BOOL TreeView_SetlParam(HWND hTreeView, HTREEITEM hItem, long lParam)
{
	TV_ITEM tvit = { 0 };

	tvit.mask = TVIF_PARAM;
	tvit.hItem = hItem;
	tvit.lParam = (LPARAM)lParam;
	return TreeView_SetItem(hTreeView, &tvit);
}


/******************************************************************************

	TreeView_GetlParam

	アイテムに関連付けられた情報の取得

******************************************************************************/

void *TreeView_GetlParam(HWND hTreeView, HTREEITEM hItem)
{
	TV_ITEM tvit = { 0 };

	if(hItem == NULL){
		return NULL;
	}
	tvit.mask = TVIF_PARAM;
	tvit.hItem = hItem;
	tvit.lParam = 0;

	if(TreeView_GetItem(hTreeView, &tvit) == TRUE && tvit.lParam != 0){
		return (void *)tvit.lParam;
	}
	return NULL;
}


/******************************************************************************

	TreeView_SetState

	アイテムの状態の設定

******************************************************************************/

BOOL TreeView_SetState(HWND hTreeView, HTREEITEM hItem, int State, int mask)
{
	TV_ITEM tvit = { 0 };

	tvit.mask = TVIF_STATE;
	tvit.hItem = hItem;
	tvit.state = State;
	tvit.stateMask = mask;
	return TreeView_SetItem(hTreeView, &tvit);
}


/******************************************************************************

	TreeView_GetState

	アイテムの状態の取得

******************************************************************************/

int TreeView_GetState(HWND hTreeView, HTREEITEM hItem, int mask)
{
	TV_ITEM tvit = { 0 };

	tvit.mask = TVIF_STATE;
	tvit.hItem = hItem;
	tvit.state = 0;
	tvit.stateMask = mask;

	TreeView_GetItem(hTreeView, &tvit);
	return tvit.state;
}


/******************************************************************************

	TreeView_SetIcon

	アイテムのアイコンを設定

******************************************************************************/

static BOOL TreeView_SetIcon(HWND hTreeView, HTREEITEM hItem, int Icon, int SelIcon)
{
	TV_ITEM tvit = { 0 };

	tvit.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE;
	tvit.hItem = hItem;
	tvit.iImage = Icon;
	tvit.iSelectedImage = SelIcon;

	return TreeView_SetItem(hTreeView, &tvit);
}


/******************************************************************************

	TreeView_UpdateList

	ツリービューのアイテムに対応したリストビューアイテムを更新

******************************************************************************/

BOOL TreeView_UpdateList(HWND hWnd, HTREEITEM hItem)
{
	int ListIndex;

	//親ウィンドウが選択フォルダではない場合は関数を抜ける
	if(TreeView_GetParent(GetDlgItem(hWnd, WWWC_TREE), hItem) !=
		TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE))){
		return FALSE;
	}
	//リストビューのインデックスを取得
	ListIndex = TreeView_GetListIndex(GetDlgItem(hWnd, WWWC_TREE), hItem);

	ListView_SetItemText(GetDlgItem(hWnd, WWWC_LIST), ListIndex, 0, LPSTR_TEXTCALLBACK);
	ListView_RedrawItems(GetDlgItem(hWnd, WWWC_LIST), ListIndex, ListIndex);
	UpdateWindow(GetDlgItem(hWnd, WWWC_LIST));
	return TRUE;
}


/******************************************************************************

	SetItemIcon

	アイテムの状態に応じてアイコンを設定する

******************************************************************************/

BOOL SetItemIcon(HWND hWnd, HTREEITEM hItem)
{
	struct TPTREE *tpTreeInfo;
	char buf[BUFSIZE];
	int Icon;

	if(hItem == RootItem || hItem == RecyclerItem){
		return FALSE;
	}
	tpTreeInfo = (struct TPTREE *)TreeView_GetlParam(GetDlgItem(hWnd, WWWC_TREE), hItem);
	if(tpTreeInfo == NULL){
		return FALSE;
	}

	//現在のアイコンを取得
	Icon = TreeView_GetItemInfo(GetDlgItem(hWnd, WWWC_TREE), hItem, buf);

	//チェック中
	if(tpTreeInfo->Icon & TREEICON_CH){
		if(Icon == ICON_DIR_CLOSE_CH){
			return FALSE;
		}
		TreeView_SetIcon(GetDlgItem(hWnd, WWWC_TREE), hItem,
			ICON_DIR_CLOSE_CH, ICON_DIR_OPEN_CH);
		TreeView_UpdateList(hWnd, hItem);
		return TRUE;
	}
	//子供がチェック中
	if(tpTreeInfo->Icon & TREEICON_CHECKCHILD){
		if(Icon == ICON_DIR_CLOSE_CHECKCHILD){
			return FALSE;
		}
		TreeView_SetIcon(GetDlgItem(hWnd, WWWC_TREE), hItem,
			ICON_DIR_CLOSE_CHECKCHILD, ICON_DIR_OPEN_CHECKCHILD);
		TreeView_UpdateList(hWnd, hItem);
		return TRUE;
	}
	//UPアイコンあり
	if(tpTreeInfo->Icon & TREEICON_UP){
		if(Icon == ICON_DIR_CLOSE_UP){
			return FALSE;
		}
		TreeView_SetIcon(GetDlgItem(hWnd, WWWC_TREE), hItem,
			ICON_DIR_CLOSE_UP, ICON_DIR_OPEN_UP);
		TreeView_UpdateList(hWnd, hItem);
		return TRUE;
	}
	//子供がUPアイコンあり
	if(tpTreeInfo->Icon & TREEICON_UPCHILD){
		if(Icon == ICON_DIR_CLOSE_UPCHILD){
			return FALSE;
		}
		TreeView_SetIcon(GetDlgItem(hWnd, WWWC_TREE), hItem,
			ICON_DIR_CLOSE_UPCHILD, ICON_DIR_OPEN_UPCHILD);
		TreeView_UpdateList(hWnd, hItem);
		return TRUE;
	}

	//標準アイコン
	if(Icon != ICON_DIR_CLOSE){
		TreeView_SetIcon(GetDlgItem(hWnd, WWWC_TREE), hItem,
			ICON_DIR_CLOSE, ICON_DIR_OPEN);
		TreeView_UpdateList(hWnd, hItem);
		return TRUE;
	}
	return FALSE;
}


/******************************************************************************

	TreeView_SetParentIcon

	親アイテムの状態を設定する

******************************************************************************/

static void TreeView_SetParentIcon(HWND hWnd, HTREEITEM hItem, int Icon)
{
	struct TPTREE *TmptpTreeInfo;
	HTREEITEM pItem;

	pItem = TreeView_GetParent(GetDlgItem(hWnd, WWWC_TREE), hItem);
	while(pItem != RootItem && pItem != RecyclerItem){
		TmptpTreeInfo = (struct TPTREE *)TreeView_GetlParam(GetDlgItem(hWnd, WWWC_TREE), pItem);
		if(TmptpTreeInfo != NULL){
			if((TmptpTreeInfo->Icon & Icon) != 0){
				break;
			}
			//状態を設定
			TmptpTreeInfo->Icon |= Icon;
			SetItemIcon(hWnd, pItem);
		}
		pItem = TreeView_GetParent(GetDlgItem(hWnd, WWWC_TREE), pItem);
	}
}


/******************************************************************************

	TreeView_DeleteParentIcon

	親アイテムの状態を削除する

******************************************************************************/

static void TreeView_DeleteParentIcon(HWND hWnd, HTREEITEM hItem, int Icon1, int Icon2)
{
	struct TPTREE *TmptpTreeInfo;
	HTREEITEM pItem, cItem;

	pItem = hItem;

	while(pItem != RootItem && pItem != RecyclerItem){
		cItem = TreeView_GetChild(GetDlgItem(hWnd, WWWC_TREE), pItem);
		while(cItem != NULL){
			TmptpTreeInfo = (struct TPTREE *)TreeView_GetlParam(GetDlgItem(hWnd, WWWC_TREE), cItem);
			if(TmptpTreeInfo != NULL &&
				((TmptpTreeInfo->Icon & Icon1) || (TmptpTreeInfo->Icon & Icon2))){
				break;
			}
			cItem = TreeView_GetNextSibling(GetDlgItem(hWnd, WWWC_TREE), cItem);
		}
		if(cItem != NULL){
			break;
		}

		TmptpTreeInfo = (struct TPTREE *)TreeView_GetlParam(GetDlgItem(hWnd, WWWC_TREE), pItem);
		if(TmptpTreeInfo != NULL && TmptpTreeInfo->Icon & Icon2){
			//状態を削除
			TmptpTreeInfo->Icon ^= Icon2;
			SetItemIcon(hWnd, pItem);
		}
		pItem = TreeView_GetParent(GetDlgItem(hWnd, WWWC_TREE), pItem);
	}
}


/******************************************************************************

	TreeView_SetIconState

	ツリビューアイテムのアイコン状態を設定する

******************************************************************************/

void CALLBACK TreeView_SetIconState(HWND hWnd, HTREEITEM hItem, long Param)
{
	struct TPTREE *tpTreeInfo;
	int i, ItemCnt, Icon;
	int UPFlag = 0;
	char buf[BUFSIZE];

	//ルートアイテムの場合は関数を抜ける
	if(hItem == RootItem){
		return;
	}

	//ごみ箱の場合はごみ箱の中身によってアイコンを変化させる
	if(hItem == RecyclerItem){
		Icon = TreeView_GetItemInfo(GetDlgItem(hWnd, WWWC_TREE), hItem, buf);
		if(CheckRecycler(hWnd) == FALSE){
			if(Icon != ICON_DIR_RECYCLER){
				TreeView_SetIcon(GetDlgItem(hWnd, WWWC_TREE), hItem,
					ICON_DIR_RECYCLER, ICON_DIR_RECYCLER);
				Icon = -1;
			}
		}else{
			if(Icon != ICON_DIR_RECYCLER_USE){
				TreeView_SetIcon(GetDlgItem(hWnd, WWWC_TREE), hItem,
					ICON_DIR_RECYCLER_USE, ICON_DIR_RECYCLER_USE);
				Icon = -1;
			}
		}
		if(Icon == -1 && TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)) == RootItem){
			i = TreeView_GetListIndex(GetDlgItem(hWnd, WWWC_TREE), RecyclerItem);
			ListView_RedrawItems(GetDlgItem(hWnd, WWWC_LIST), i, i);
			UpdateWindow(GetDlgItem(hWnd, WWWC_LIST));
		}
		return;
	}

	//ツリーアイテム情報の取得
	tpTreeInfo = (struct TPTREE *)TreeView_GetlParam(GetDlgItem(hWnd, WWWC_TREE), hItem);
	if(tpTreeInfo == NULL){
		TreeView_SetIcon(GetDlgItem(hWnd, WWWC_TREE), hItem, ICON_DIR_CLOSE, ICON_DIR_OPEN);
		return;
	}

	//アイテム情報が読み込まれていない場合は読み込む
	if(tpTreeInfo->ItemList == NULL){
		if(ReadTreeMem(hWnd, hItem) == FALSE){
			TreeView_SetIcon(GetDlgItem(hWnd, WWWC_TREE), hItem, ICON_DIR_CLOSE, ICON_DIR_OPEN);
			return;
		}
	}

	//ツリーのアイテムの情報を取得
	Icon = TreeView_GetItemInfo(GetDlgItem(hWnd, WWWC_TREE), hItem, buf);

	ItemCnt = tpTreeInfo->ItemListCnt;
	for(i = 0;i < ItemCnt;i++){
		if((*(tpTreeInfo->ItemList + i)) == NULL){
			continue;
		}

		//チェック中のアイテム
		if((*(tpTreeInfo->ItemList + i))->IconStatus == ST_CHECK){
			tpTreeInfo->Icon |= TREEICON_CH;
			SetItemIcon(hWnd, hItem);

			//親をチェックありアイコンにする
			TreeView_SetParentIcon(hWnd, hItem, TREEICON_CHECKCHILD);
			return;
		}

		//UPアイテム
		if((*(tpTreeInfo->ItemList + i))->Status == ST_UP){
			//UPアイテム存在フラグをセット
			UPFlag = 1;
			//チェック中ではない場合はループを抜ける
			if(gCheckFlag == 0){
				break;
			}
		}
	}

	//チェック中フラグ
	if(tpTreeInfo->Icon & TREEICON_CH){
		//チェック中フラグを無くす
		tpTreeInfo->Icon ^= TREEICON_CH;

		TreeView_DeleteParentIcon(hWnd,
			TreeView_GetParent(GetDlgItem(hWnd, WWWC_TREE), hItem),
			TREEICON_CH, TREEICON_CHECKCHILD);
	}

	//UPアイテムが存在する場合
	if(UPFlag == 1){
		//UPアイコンに変更
		tpTreeInfo->Icon |= TREEICON_UP;
		SetItemIcon(hWnd, hItem);

		//親をUPありアイコンにする
		TreeView_SetParentIcon(hWnd, hItem, TREEICON_UPCHILD);
		return;
	}

	if(tpTreeInfo->Icon & TREEICON_UP){
		//UPアイコンの除去
		tpTreeInfo->Icon ^= TREEICON_UP;
		//親のアイコンを変更
		TreeView_DeleteParentIcon(hWnd,
			TreeView_GetParent(GetDlgItem(hWnd, WWWC_TREE), hItem),
			TREEICON_UP, TREEICON_UPCHILD);

	}else if(tpTreeInfo->Icon & TREEICON_UPCHILD){
		//子供の確認
		TreeView_DeleteParentIcon(hWnd, hItem, TREEICON_UP, TREEICON_UPCHILD);
	}

	//アイコンの変更
	SetItemIcon(hWnd, hItem);
}


/******************************************************************************

	TreeView_NoFileDelete

	対応するフォルダが存在しないアイテムを削除する

******************************************************************************/

void CALLBACK TreeView_NoFileDelete(HWND hWnd, HTREEITEM hItem, long Param)
{
	HTREEITEM cItem, TmpItem;
	char path[BUFSIZE];

	TreeView_GetPath(GetDlgItem(hWnd, WWWC_TREE), hItem, path, CuDir);
	if(GetDirSerch(path) == FALSE && FindCheckItem(hWnd, hItem) == 0 && FindPropItem(hWnd, hItem) == 0){
		CallTreeItem(hWnd, hItem, (FARPROC)TreeView_FreeTreeItem, 1);
		TreeView_DeleteItem(GetDlgItem(hWnd, WWWC_TREE), hItem);
		return;
	}

	cItem = TreeView_GetChild(GetDlgItem(hWnd, WWWC_TREE), hItem);
	while(cItem != NULL){
		//再帰
		TmpItem = TreeView_GetNextSibling(GetDlgItem(hWnd, WWWC_TREE), cItem);
		TreeView_NoFileDelete(hWnd, cItem, Param);
		cItem = TmpItem;
	}

}


/******************************************************************************

	TreeView_StartDragItem

	ツリビューアイテムのドラッグ＆ドロップ開始

******************************************************************************/

void TreeView_StartDragItem(HWND hWnd, HTREEITEM hItem)
{
	HTREEITEM pItem;
	UINT cf[CLIPFORMAT_CNT];
	int Effect;
	int ret;
	int cfcnt;
	BOOL NoFileFlag;

	DgdpItem = hItem;
	if(DgdpItem == NULL || DgdpItem == RootItem){
		DgdpItem = NULL;
		return;
	}

	//Altキーが押されている場合はファイルを扱わない
	NoFileFlag = (GetAsyncKeyState(VK_MENU) < 0) ? TRUE : FALSE;

	//フォルダ情報の保存
	SaveTreeMem(hWnd, GetDlgItem(hWnd, WWWC_TREE), DgdpItem);

	Effect = DROPEFFECT_COPY;
	if(FindCheckItem(hWnd, DgdpItem) != 1){
		Effect |= DROPEFFECT_MOVE;
	}
	if(FindPropItem(hWnd, DgdpItem) != 1){
		Effect |= DROPEFFECT_MOVE;
	}

	WWWCDropFlag = FALSE;

	if(NoFileFlag == FALSE){
		if(DragDrop_CreateTreeDropFiles(hWnd, DgdpItem, DnD_DirName) == FALSE){
			return;
		}
	}

	DragItemIndex = -1;

	cf[0] = WWWC_ClipFormat;
	cf[1] = CF_TEXT;
	cf[2] = CF_HDROP;
	cfcnt = (NoFileFlag == FALSE) ? CLIPFORMAT_CNT : CLIPFORMAT_CNT - 1;

	ret = OLE_IDropSource_Start(hWnd, WM_DATAOBJECT_GETDATA, cf, cfcnt, Effect);

	if(ret != -1 && ret == DROPEFFECT_MOVE && WWWCDropFlag == TRUE){
		pItem = TreeView_GetParent(GetDlgItem(hWnd, WWWC_TREE), DgdpItem);
		TreeView_NoFileDelete(hWnd, DgdpItem, 0);

		if(pItem == TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE))){
			ListView_RefreshFolder(hWnd);
		}
		TreeView_SetIconState(hWnd, pItem, 0);
		CallTreeItem(hWnd, pItem, (FARPROC)TreeView_FreeItem, 1);

	}else{
		CallTreeItem(hWnd, DgdpItem, (FARPROC)TreeView_FreeItem, 1);
	}
	if(NoFileFlag == FALSE){
		FreeDropFiles();
	}
}


/******************************************************************************

	TreeView_SelItemChanging

	ツリービューアイテムの選択変更チェック

******************************************************************************/

BOOL TreeView_SelItemChanging(HWND hWnd, HTREEITEM NewItem, HTREEITEM OldItem)
{
	if(NewItem == NULL){
		return FALSE;
	}
	WaitCursor(TRUE);

	//ファイルからリストの情報をメモリに読み込む
	if(ReadTreeMem(hWnd, NewItem) == FALSE){
		WaitCursor(FALSE);
		MessageBox(hWnd, EMSG_DIRMOVE, EMSG_DIRMOVE_TITLE, MB_ICONSTOP);
		if(OldItem == NULL){
			return FALSE;
		}
		return TRUE;
	}
	WaitCursor(FALSE);
	return FALSE;
}


/******************************************************************************

	TreeView_SelItemChanged

	ツリービューアイテムの選択変更

******************************************************************************/

BOOL TreeView_SelItemChanged(HWND hWnd, HTREEITEM NewItem, HTREEITEM OldItem)
{
	//リストビューのアイテムをすべて削除する
	if(NewItem == NULL){
		ListView_DeleteAllItems(GetDlgItem(hWnd, WWWC_LIST));
		return FALSE;
	}

	WaitCursor(TRUE);

	//リストビューにアイテムを表示する
	ListView_ShowItem(hWnd, NewItem);
	ListView_SetItemState(GetDlgItem(hWnd, WWWC_LIST), 0, LVIS_FOCUSED, LVIS_FOCUSED);

	//ツリーのアイコンを設定する
	TreeView_SetIconState(hWnd, NewItem, 0);

	//前回選択されていたアイテムのメモリを解放する（保存モード）
	if(OldItem != NULL){
		TreeView_FreeItem(hWnd, OldItem, 1);
	}

	SetProtocolMenu(hWnd);
	SetSbText(hWnd);
	TreeView_SetHistory(NewItem);

	WaitCursor(FALSE);
	return FALSE;
}


/******************************************************************************

	TreeView_SetHistory

	TreeViewアイテムを履歴に追加

******************************************************************************/

static void TreeView_SetHistory(HTREEITEM NewItem)
{
	int i;

	if(HistoryFlag == TRUE) return;
	if(HistoryIndex < HISTORYCNT){
		HistoryIndex++;
		HistoryItem[HistoryIndex] = NewItem;
		for(i = HistoryIndex + 1; i < HISTORYCNT; i++){
			HistoryItem[i] = NULL;
		}
		return;
	}
	for(i = 0; i < HISTORYCNT; i++){
		HistoryItem[i] = HistoryItem[i + 1];
	}
	HistoryItem[HISTORYCNT] = NewItem;
}


/******************************************************************************

	TreeView_CheckHistory

	移動可能かチェック

******************************************************************************/

BOOL TreeView_CheckHistory(HWND hWnd, BOOL NextFlag)
{
	HTREEITEM hItem;
	int i;

	hItem = TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE));
	i = HistoryIndex;

	if(NextFlag == TRUE){
		for(i = HistoryIndex + 1; i < HISTORYCNT &&
			(HistoryItem[i] == NULL || HistoryItem[i] == hItem ||
			IsTreeItem(hWnd, HistoryItem[i]) == FALSE); i++);
		if(i >= HISTORYCNT) return FALSE;
	}else{
		for(i = HistoryIndex - 1; i > 0 &&
			(HistoryItem[i] == NULL || HistoryItem[i] == hItem ||
			IsTreeItem(hWnd, HistoryItem[i]) == FALSE); i--);
		if(i <= 0) return FALSE;
	}
	return TRUE;
}


/******************************************************************************

	TreeView_PrevHistory

	前に戻る

******************************************************************************/

BOOL TreeView_PrevHistory(HWND hWnd)
{
	HTREEITEM hItem;
	int i;
	BOOL ret;

	hItem = TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE));
	for(i = HistoryIndex - 1; i > 0 &&
		(HistoryItem[i] == NULL || HistoryItem[i] == hItem ||
		IsTreeItem(hWnd, HistoryItem[i]) == FALSE); i--);
	if(i <= 0) return FALSE;

	HistoryFlag = TRUE;
	ret = (BOOL)TreeView_SelectItem(GetDlgItem(hWnd, WWWC_TREE), HistoryItem[i]);
	HistoryFlag = FALSE;
	HistoryIndex = i;

	if(ret == FALSE){
		return TreeView_PrevHistory(hWnd);
	}
	return TRUE;
}


/******************************************************************************

	TreeView_NextHistory

	次に進む

******************************************************************************/

BOOL TreeView_NextHistory(HWND hWnd)
{
	HTREEITEM hItem;
	int i;
	BOOL ret;

	hItem = TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE));
	for(i = HistoryIndex + 1; i < HISTORYCNT &&
		(HistoryItem[i] == NULL || HistoryItem[i] == hItem ||
		IsTreeItem(hWnd, HistoryItem[i]) == FALSE); i++);
	if(i >= HISTORYCNT) return FALSE;

	HistoryFlag = TRUE;
	ret = (BOOL)TreeView_SelectItem(GetDlgItem(hWnd, WWWC_TREE), HistoryItem[i]);
	HistoryFlag = FALSE;
	HistoryIndex = i;

	if(ret == FALSE){
		return TreeView_NextHistory(hWnd);
	}
	return TRUE;
}


/******************************************************************************

	TreeView_NotifyProc

	ツリービューイベント

******************************************************************************/

LRESULT TreeView_NotifyProc(HWND hWnd, LPARAM lParam)
{
	TV_DISPINFO *ptv = (TV_DISPINFO *)lParam;
	NM_TREEVIEW *Tv = (NM_TREEVIEW *)lParam;
	NMHDR *CForm = (NMHDR *)lParam;

	switch(ptv->hdr.code)
	{
	case TVN_BEGINLABELEDIT:
	case TVN_ENDLABELEDIT:
		return SendMessage(hWnd, WM_TV_EVENT, ptv->hdr.code, lParam);
	}

	switch(Tv->hdr.code)
	{
	case TVN_BEGINDRAG:
	case TVN_BEGINRDRAG:
	case TVN_SELCHANGING:
	case TVN_SELCHANGED:
		return SendMessage(hWnd, WM_TV_EVENT, Tv->hdr.code, lParam);
	}

	switch(CForm->code)
	{
	case NM_SETFOCUS:
	case NM_RCLICK:
		return SendMessage(hWnd, WM_TV_EVENT, CForm->code, lParam);
	}
	return FALSE;
}
/* End of source */
