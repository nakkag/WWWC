/**************************************************************************

	WWWC

	ClipBoard.c

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

static HTREEITEM CutItem;
static BOOL TreeEditFlag;
static BOOL CutCntFlag;
static HWND ClipNext;

static struct TPITEM **tpDeleteItemList;
static int tpDeleteItemListCnt;

//外部参照
extern char CuDir[];
extern HWND TbWnd;
extern HMENU hPOPUP;
extern struct TP_PROTOCOL *tpProtocol;
extern int ProtocolCnt;
extern HTREEITEM RootItem;
extern HTREEITEM RecyclerItem;
extern HTREEITEM HiTestItem;
extern UINT WWWC_ClipFormat;
extern int UpdateItemFlag;


/**************************************************************************
	Local Function Prototypes
**************************************************************************/
static HANDLE Clipboard_Set_WF_FolderPath(HWND hWnd, int mode, HTREEITEM ClipItem);
static HANDLE Clipboard_Set_TEXT_FolderPath(HWND hWnd, HTREEITEM hItem);
static HANDLE Clipboard_Set_TEXT_ItemInfo(struct TPITEM *tpItemInfo);
static BOOL Clipboard_SetTreeData(HWND hWnd, int mode);
static BOOL CopyFolder(HWND hWnd, HTREEITEM hItem, char *FromPath, char *Name, int mode, int DnDFlag);
static BOOL CopyItemInfo(HWND hWnd, HTREEITEM hItem, struct TPITEM *NewItemInfo, char *SourcePath, int mode);
static int StringToItem(HWND hWnd, HTREEITEM hItem, char *Data, int DnDFlag);


/******************************************************************************

	Clipboard_SetChain

	クリップボードビューアチェーンにウィンドウをセットする

******************************************************************************/

void Clipboard_SetChain(HWND hWnd)
{
	ClipNext = SetClipboardViewer(hWnd);
}


/******************************************************************************

	Clipboard_DeleteChain

	クリップボードビューアチェーンにウィンドウを削除する

******************************************************************************/

void Clipboard_DeleteChain(HWND hWnd)
{
	if(ClipNext != NULL){
		ChangeClipboardChain(hWnd, ClipNext);
		ClipNext = NULL;
	}
}


/******************************************************************************

	Clipboard_ChangeChain

	クリップボードビューアチェーンの変更設定

******************************************************************************/

void Clipboard_ChangeChain(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if((HWND)wParam == ClipNext){
		ClipNext = (HWND)lParam;

	}else if(ClipNext != NULL){
		SendMessage(ClipNext, uMsg, wParam, lParam);
	}
}


/******************************************************************************

	Clipboard_Draw

	クリップボードの内容の変化

******************************************************************************/

void Clipboard_Draw(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HTREEITEM pItem;
	int ret;

	if(ClipNext != NULL){
		SendMessage(ClipNext, uMsg, wParam, lParam);
	}
	ret = (Clipboard_CheckFormat() <= 0) ? 1 : 0;
	EnableMenuItem(GetSubMenu(GetMenu(hWnd), MENU_HWND_EDIT), ID_MENUITEM_PASTE, ret);
	EnableMenuItem(GetSubMenu(hPOPUP, MENU_POP_FOLDER), ID_MENUITEM_PASTE, ret);
	SendMessage(TbWnd, TB_ENABLEBUTTON, ID_MENUITEM_PASTE, (LPARAM) MAKELONG(!(ret), 0));

	if(CutCntFlag == FALSE && CutItem != NULL){
		//既に削除されたフォルダをツリービューから削除
		pItem = TreeView_GetParent(GetDlgItem(hWnd, WWWC_TREE), CutItem);

		TreeView_NoFileDelete(hWnd, CutItem, 0);
		ListView_RefreshFolder(hWnd);
		if(TreeEditFlag == FALSE){
			//既に削除されたアイテムを削除
			Item_Select(hWnd, CutItem);
			TreeView_SetIconState(hWnd, CutItem, 0);
		}else{
			TreeView_SetIconState(hWnd, pItem, 0);
		}
		//切り取りマークの解除
		Clipboard_DeleteCutStatus(hWnd);

		CutItem = NULL;
		TreeEditFlag = FALSE;
	}
	CutCntFlag = FALSE;
}


/******************************************************************************

	Clipboard_DeleteCutStatus

	切り取りマークの除去

******************************************************************************/

BOOL Clipboard_DeleteCutStatus(HWND hWnd)
{
	if(CutItem == NULL){
		return TRUE;
	}
	if(TreeEditFlag == TRUE){
		//ツリーに切り取りマークがある場合
		TreeEditFlag = FALSE;
		TreeView_SetState(GetDlgItem(hWnd, WWWC_TREE), CutItem, 0, TVIS_CUT);
		CutItem = NULL;
		return TRUE;
	}

	ListView_SetItemState(GetDlgItem(hWnd, WWWC_LIST), -1, 0, LVIS_CUT);

	CallTreeItem(hWnd, CutItem, (FARPROC)TreeView_FreeItem, 1);
	CutItem = NULL;
	return TRUE;
}


/******************************************************************************

	Clipboard_Set_WF_FolderPath

	フォルダの情報を文字列のハンドルに設定

******************************************************************************/

static HANDLE Clipboard_Set_WF_FolderPath(HWND hWnd, int mode, HTREEITEM ClipItem)
{
	HANDLE hMem;
	char *buf;
	char ppath[BUFSIZE];
	char path[BUFSIZE];
	char name[BUFSIZE];
	int ErrCode;

	if(ClipItem == RootItem || ClipItem == RecyclerItem){
		return NULL;
	}
	//親フォルダのパス
	TreeView_GetPath(GetDlgItem(hWnd, WWWC_TREE), TreeView_GetParent(GetDlgItem(hWnd, WWWC_TREE), ClipItem), ppath, CuDir);
	lstrcat(ppath, "\\"DATAFILENAME);
	//フォルダのパス
	TreeView_GetPath(GetDlgItem(hWnd, WWWC_TREE), ClipItem, path, CuDir);
	//フォルダの名前
	TreeView_GetItemInfo(GetDlgItem(hWnd, WWWC_TREE), ClipItem, name);

	if((hMem = GlobalAlloc(GHND, 1 + lstrlen(ppath) + 2 + 1 + lstrlen(path) + 1 + lstrlen(name) + 3)) == NULL){
		ErrMsg(hWnd, GetLastError(), NULL);
		return NULL;
	}
	if((buf = GlobalLock(hMem)) == NULL){
		ErrCode = GetLastError();
		GlobalFree(hMem);
		ErrMsg(hWnd, ErrCode, NULL);
		return NULL;
	}
	//フォルダ情報文字列の作成
	wsprintf(buf,"%c%s\r\n%c%s\t%s\r\n",mode, ppath, FLAG_FOLDER, path, name);

	GlobalUnlock(hMem);
	return hMem;
}


/******************************************************************************

	Clipboard_Set_WF_ItemList

	アイテムの情報を文字列のハンドルに設定
	(WWWC_ClipFormat)

******************************************************************************/

HANDLE Clipboard_Set_WF_ItemList(HWND hWnd, int mode, HTREEITEM ClipItem)
{
	struct TPITEM *tpItemInfo;
	HTREEITEM hItem;
	HANDLE hMem;
	char TmpBuf[BUFSIZE];
	char SourcePath[BUFSIZE];
	char *buf, *p, *r;
	int SelectItem;
	int ErrCode;
	int cnt;

	if(ClipItem != NULL){
		if(ClipItem == RecyclerItem){
			return NULL;
		}
		return Clipboard_Set_WF_FolderPath(hWnd, mode, ClipItem);
	}

	//作成する文字列のサイズの取得
	TreeView_GetPath(GetDlgItem(hWnd, WWWC_TREE), TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)), TmpBuf, CuDir);
	wsprintf(SourcePath, "%s\\"DATAFILENAME, TmpBuf);
	cnt = lstrlen(SourcePath) + 3;			//mode + Path + '\r' + '\n'

	SelectItem = -1;
	while((SelectItem = ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), SelectItem, LVNI_SELECTED)) != -1){
		tpItemInfo = (struct TPITEM *)ListView_GetlParam(GetDlgItem(hWnd, WWWC_LIST), SelectItem);
		if(tpItemInfo != NULL){				//アイテムの場合
			//アイテム文字列のサイズ
			cnt += ItemSize(tpItemInfo);
			continue;
		}

		//フォルダ文字列のサイズ
		hItem = TreeView_GetIndexToItem(GetDlgItem(hWnd, WWWC_TREE),
			TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)), SelectItem);
		if(hItem == RecyclerItem){
			continue;
		}
		TreeView_GetPath(GetDlgItem(hWnd, WWWC_TREE), hItem, TmpBuf, CuDir);
		cnt += lstrlen(TmpBuf) + 2;			//mode + Path + '\t'
		TreeView_GetItemInfo(GetDlgItem(hWnd, WWWC_TREE), hItem, TmpBuf);
		cnt += lstrlen(TmpBuf) + 2;			//name + '\r' + '\n'
	}

	//確保
	if((hMem = GlobalAlloc(GHND, cnt + 1)) == NULL){
		ErrMsg(hWnd, GetLastError(), NULL);
		return NULL;
	}
	if((buf = GlobalLock(hMem)) == NULL){
		ErrCode = GetLastError();
		GlobalFree(hMem);
		ErrMsg(hWnd, ErrCode, NULL);
		return NULL;
	}

	p = buf;

	TreeView_GetPath(GetDlgItem(hWnd, WWWC_TREE), TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)), TmpBuf, CuDir);
	wsprintf(SourcePath, "%s\\"DATAFILENAME, TmpBuf);
	*(p++) = mode;
	for(r = SourcePath;*r != '\0';r++) *(p++) = *r;
	*(p++) = '\r', *(p++) = '\n';

	SelectItem = -1;
	while((SelectItem = ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), SelectItem, LVNI_SELECTED)) != -1){
		tpItemInfo = (struct TPITEM *)ListView_GetlParam(GetDlgItem(hWnd, WWWC_LIST), SelectItem);
		if(tpItemInfo != NULL){				//アイテムの場合
			//アイテム情報を文字列にする
			p = SetItemString(tpItemInfo, p);
			continue;
		}

		//フォルダ文字列の追加
		hItem = TreeView_GetIndexToItem(GetDlgItem(hWnd, WWWC_TREE),
			TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)), SelectItem);
		if(hItem == RecyclerItem){
			continue;
		}

		//フォルダフラグ
		*(p++) = FLAG_FOLDER;
		//パス
		TreeView_GetPath(GetDlgItem(hWnd, WWWC_TREE), hItem, TmpBuf, CuDir);
		for(r = TmpBuf;*r != '\0';r++) *(p++) = *r;
		*(p++) = '\t';
		//名前
		TreeView_GetItemInfo(GetDlgItem(hWnd, WWWC_TREE), hItem, TmpBuf);
		for(r = TmpBuf;*r != '\0';r++) *(p++) = *r;
		*(p++) = '\r', *(p++) = '\n';
	}
	*p = '\0';

	GlobalUnlock(hMem);
	return hMem;
}


/******************************************************************************

	Clipboard_Set_TEXT_FolderPath

	フォルダのパスを文字列のハンドルに設定
	(CF_TEXT)

******************************************************************************/

static HANDLE Clipboard_Set_TEXT_FolderPath(HWND hWnd, HTREEITEM hItem)
{
	HANDLE hMemText;
	char path[BUFSIZE];
	char *buf;

	//フォルダのパスを取得
	TreeView_GetPath(GetDlgItem(hWnd, WWWC_TREE), hItem, path, CuDir);

	if((hMemText = GlobalAlloc(GHND, lstrlen(path) + 1)) == NULL){
		return NULL;
	}
	if((buf = GlobalLock(hMemText)) == NULL){
		GlobalFree(hMemText);
		return NULL;
	}

	//パスをコピー
	lstrcpy(buf, path);

	GlobalUnlock(hMemText);
	return hMemText;
}


/******************************************************************************

	Clipboard_Set_TEXT_ItemInfo

	単一アイテムの情報を文字列のハンドルに設定する
	(CF_TEXT)

******************************************************************************/

static HANDLE Clipboard_Set_TEXT_ItemInfo(struct TPITEM *tpItemInfo)
{
	FARPROC Func_GetItemText;
	HANDLE hMemText;
	char buf[BUFSIZE];
	char *p;
	int ProtocolIndex;

	if(tpItemInfo == NULL){
		return NULL;
	}
	ProtocolIndex = GetProtocolIndex(tpItemInfo->CheckURL);
	if(ProtocolIndex == -1 || tpProtocol[ProtocolIndex].lib == NULL){
		//プロトコルを識別できなかった場合は、チェックするURLを設定
		if(tpItemInfo->CheckURL == NULL || tpItemInfo->CheckURL[0] == '\0'){
			return NULL;
		}
		if((hMemText = GlobalAlloc(GHND, lstrlen(tpItemInfo->CheckURL) + 1)) == NULL){
			return NULL;
		}
		if((p = GlobalLock(hMemText)) == NULL){
			GlobalFree(hMemText);
			return NULL;
		}
		lstrcpy(p, tpItemInfo->CheckURL);

		GlobalUnlock(hMemText);
		return hMemText;
	}

	//プロトコルDLLのアイテムの文字列を取得する関数を呼ぶ
	wsprintf(buf, "%sGetItemText", tpProtocol[ProtocolIndex].FuncHeader);
	Func_GetItemText = GetProcAddress((HMODULE)tpProtocol[ProtocolIndex].lib, buf);
	if(Func_GetItemText == NULL){
		return NULL;
	}
	return (HANDLE)Func_GetItemText(tpItemInfo);
}


/******************************************************************************

	Clipboard_Set_TEXT

	CF_TEXTの文字列を作成

******************************************************************************/

HANDLE Clipboard_Set_TEXT(HWND hWnd, HTREEITEM hStrItem)
{
	struct TPITEM *tpItemInfo;
	HTREEITEM hItem;
	int SelectItem;

	if(hStrItem != NULL){
		if(hStrItem == RootItem || hStrItem == RecyclerItem){
			return NULL;
		}
		//フォルダのパスを設定
		return Clipboard_Set_TEXT_FolderPath(hWnd, hStrItem);
	}

	//フォーカスを持つアイテムのインデックスを取得
	if((SelectItem = ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), -1, LVNI_FOCUSED)) == -1){
		SelectItem = ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), -1, LVNI_SELECTED);
	}
	if(SelectItem == -1){
		return NULL;
	}

	tpItemInfo = (struct TPITEM *)ListView_GetlParam(GetDlgItem(hWnd, WWWC_LIST), SelectItem);
	if(tpItemInfo == NULL){
		hItem = TreeView_GetIndexToItem(GetDlgItem(hWnd, WWWC_TREE),
			TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)), SelectItem);
		//フォルダのパスを設定
		return Clipboard_Set_TEXT_FolderPath(hWnd, hItem);

	}else{
		//アイテムの情報を設定
		return Clipboard_Set_TEXT_ItemInfo(tpItemInfo);
	}
	return NULL;
}


/******************************************************************************

	Clipboard_SetTreeData

	クリップボードにツリーアイテムを設定する

******************************************************************************/

static BOOL Clipboard_SetTreeData(HWND hWnd, int mode)
{
	HANDLE hMem, hMemText, hMemDrop;
	int ErrCode;

	if(HiTestItem == RootItem || HiTestItem == RecyclerItem){
		return FALSE;
	}

	//フォルダ情報の保存
	SaveTreeMem(hWnd, GetDlgItem(hWnd, WWWC_TREE), HiTestItem);

	if(mode == FLAG_CUT){
		//切り取りのマスクを設定
		TreeView_SetState(GetDlgItem(hWnd, WWWC_TREE), HiTestItem, TVIS_CUT, TVIS_CUT);
		CutCntFlag = TRUE;
		CutItem = HiTestItem;
	}

	//クリップボードを開く
	if(OpenClipboard(hWnd) == FALSE){
		ErrMsg(hWnd, GetLastError(), NULL);
		CutItem = NULL;
		return FALSE;
	}
	if(EmptyClipboard() == FALSE){
		ErrCode = GetLastError();
		CloseClipboard();
		ErrMsg(hWnd, ErrCode, NULL);
		CutItem = NULL;
		return FALSE;
	}

	//フォルダ情報
	hMem = Clipboard_Set_WF_ItemList(hWnd, mode, HiTestItem);
	if(hMem == NULL){
		CloseClipboard();
		CutItem = NULL;
		return FALSE;
	}
	//独自フォーマットの設定
	if(SetClipboardData(WWWC_ClipFormat, hMem) == NULL){
		ErrCode = GetLastError();
		CloseClipboard();
		ErrMsg(hWnd, ErrCode, NULL);
		CutItem = NULL;
		return FALSE;
	}

	//ドロップファイル
	if(DragDrop_CreateTreeDropFiles(hWnd, HiTestItem, Clip_DirName) == FALSE){
		return FALSE;
	}
	if((hMemDrop = DragDrop_SetDropFileMem(hWnd)) != NULL){
		SetClipboardData(CF_HDROP, hMemDrop);
	}
	FreeDropFiles();

	//フォルダのパス
	hMemText = Clipboard_Set_TEXT(hWnd, HiTestItem);
	if(hMemText != NULL){
		SetClipboardData(CF_TEXT, hMemText);
	}

	CloseClipboard();
	//ツリーでの処理を示すフラグを設定
	TreeEditFlag = TRUE;
	return TRUE;
}


/******************************************************************************

	Clipboard_SetItemData

	クリップボードにデータを設定する

******************************************************************************/

BOOL Clipboard_SetItemData(HWND hWnd, int mode)
{
	HANDLE hMem, hMemText, hMemDrop;
	int SelectItem;
	int ErrCode;

	Clipboard_DeleteCutStatus(hWnd);
	CutItem = NULL;
	TreeEditFlag = FALSE;

	if(GetFocus() == GetDlgItem(hWnd, WWWC_TREE)){
		//フォルダ
		return Clipboard_SetTreeData(hWnd, mode);
	}

	//フォルダ情報の保存
	SaveTreeMem(hWnd, GetDlgItem(hWnd, WWWC_TREE), TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)));

	SelectItem = -1;
	while((SelectItem = ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), SelectItem, LVNI_SELECTED)) != -1){
		if(mode == FLAG_CUT){
			//切り取りマスクを設定
			ListView_SetItemState(GetDlgItem(hWnd, WWWC_LIST), SelectItem, LVIS_CUT, LVIS_CUT);
		}
	}
	if(mode == FLAG_CUT){
		//切り取りフラグの設定
		CutCntFlag = TRUE;
		CutItem = TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE));
	}

	//クリップボードを開く
	if(OpenClipboard(hWnd) == FALSE){
		ErrMsg(hWnd, GetLastError(), NULL);
		CutItem = NULL;
		return FALSE;
	}
	if(EmptyClipboard() == FALSE){
		ErrCode = GetLastError();
		CloseClipboard();
		ErrMsg(hWnd, ErrCode, NULL);
		CutItem = NULL;
		return FALSE;
	}

	//アイテム情報
	hMem = Clipboard_Set_WF_ItemList(hWnd, mode, NULL);
	if(hMem == NULL){
		CloseClipboard();
		return FALSE;
	}
	if(SetClipboardData(WWWC_ClipFormat, hMem) == NULL){
		ErrCode = GetLastError();
		CloseClipboard();
		ErrMsg(hWnd, ErrCode, NULL);
		return FALSE;
	}

	//ドロップファイル
	if(DragDrop_CreateDropFiles(hWnd, Clip_DirName) == FALSE){
		return FALSE;
	}
	if((hMemDrop = DragDrop_SetDropFileMem(hWnd)) != NULL){
		SetClipboardData(CF_HDROP, hMemDrop);
	}
	FreeDropFiles();

	//アイテム情報の文字列のハンドル
	hMemText = Clipboard_Set_TEXT(hWnd, NULL);
	if(hMemText != NULL){
		SetClipboardData(CF_TEXT, hMemText);
	}

	CloseClipboard();
	return TRUE;
}


/******************************************************************************

	CopyFolder

	フォルダのコピー

******************************************************************************/

static BOOL CopyFolder(HWND hWnd, HTREEITEM hItem, char *FromPath, char *Name, int mode, int DnDFlag)
{
	char Msg[BUFSIZE * 2];
	char Path[BUFSIZE];
	char DirName[BUFSIZE];
	char CopyPath[BUFSIZE];
	char CopyName[BUFSIZE];
	int i;

	TreeView_GetPath(GetDlgItem(hWnd, WWWC_TREE), hItem, Path, CuDir);

	if(*Path == '\0' || *FromPath == '\0'){
		MessageBox(hWnd, EMSG_FOLDERCOPY_ERR, EMSG_FOLDERCOPY_TITLE, MB_ICONSTOP);
		return FALSE;
	}

	//送り側と受け側のフォルダが同じ
	if(lstrcmp(Path, FromPath) == 0){
		if(mode == FLAG_COPY){
			wsprintf(Msg, EMSG_FOLDERCOPY_CUR, Name);
			MessageBox(hWnd, Msg, EMSG_FOLDERCOPY_TITLE, MB_ICONSTOP);
		}
		return FALSE;
	}
	//受け側のフォルダは、送り側フォルダのサブフォルダ
	if(lstrcmpn(Path, FromPath, lstrlen(FromPath)) == 0 &&
		(Path[lstrlen(FromPath)] == '\\' || Path[lstrlen(FromPath)] == '/')){
		wsprintf(Msg, EMSG_FOLDERCOPY_SUB, Name);
		MessageBox(hWnd, Msg, EMSG_FOLDERCOPY_TITLE, MB_ICONSTOP);
		return FALSE;
	}

	lstrcpy(DirName, Name);
	wsprintf(CopyPath, "%s\\%s", Path, DirName);

	if(DnDFlag & DND_RECY || hItem == RecyclerItem){
		//既に存在するフォルダ名の場合は番号を付ける
		i = 1;
		lstrcpy(CopyName, DirName);
		while(GetDirSerch(CopyPath) == TRUE){
			wsprintf(CopyName, "%s_%d", DirName, i++);
			wsprintf(CopyPath, "%s\\%s", Path, CopyName);
		}
		lstrcpy(DirName, CopyName);
		lstrcpy(Name, CopyName);
	}
	if(mode == FLAG_COPY){
		//ディレクトリのコピー
		if(CopyDirTree(hWnd, FromPath, Path, DirName, Name) == -1){
			return FALSE;
		}

	}else{
		//移動先と移動元が同じ場合は処理を行わない
		if(lstrcmp(FromPath, CopyPath) == 0){
			return FALSE;
		}

		//既に存在するディレクトリの場合はコピー後削除を行う
		if(GetDirSerch(CopyPath) == TRUE){
			//ディレクトリのコピー
			if(CopyDirTree(hWnd, FromPath, Path, DirName, NULL) == -1){
				return FALSE;
			}
			DeleteDirTree(FromPath, FALSE);
			RemoveDirectory(FromPath);

		}else{
			//ディレクトリの移動
			if(MoveFile(FromPath, CopyPath) == FALSE){
				ErrMsg(hWnd, GetLastError(), EMSG_MOVEDIR_TITLE);
				return FALSE;
			}
		}
	}
	SetDirTree(GetDlgItem(hWnd, WWWC_TREE), Path, hItem);
	SetFolderToUndo(CopyPath, Name);
	return TRUE;
}


/******************************************************************************

	CopyItemInfo

	アイテム情報のコピー

******************************************************************************/

static BOOL CopyItemInfo(HWND hWnd, HTREEITEM hItem, struct TPITEM *NewItemInfo, char *SourcePath, int mode)
{
	struct TPTREE *tpTreeInfo;
	char TmpBuf[BUFSIZE];
	char ToPath[BUFSIZE];
	char *p, *r;
	char *TmpTitle;
	int cnt;
	int ret;
	int i;

	//移動先と移動元が同じ場合
	TreeView_GetPath(GetDlgItem(hWnd, WWWC_TREE), hItem, TmpBuf, CuDir);
	wsprintf(ToPath, "%s\\"DATAFILENAME, TmpBuf);
	if(lstrcmpi(ToPath, SourcePath) == 0){
		//移動の場合は処理を行わない
		if(mode == FLAG_CUT){
			return FALSE;
		}

		//アイテムリスト取得
		tpTreeInfo = (struct TPTREE *)TreeView_GetlParam(GetDlgItem(hWnd, WWWC_TREE), hItem);
		if(tpTreeInfo == NULL){
			return FALSE;
		}
		//アイテム情報を読み込む
		if(tpTreeInfo->ItemList == NULL){
			if(ReadTreeMem(hWnd, hItem) == FALSE){
				return FALSE;
			}
		}
		if(NewItemInfo->Title == NULL){
			return TRUE;
		}
		//アイテムの名前を変更する
		TmpTitle = (char *)GlobalAlloc(GPTR, lstrlen(NewItemInfo->Title) + lstrlen(STR_COPYNAME) + 1);
		wsprintf(TmpTitle, STR_COPYNAME"%s", NewItemInfo->Title);
		cnt = 1;
		while(1){
			for(i = 0;i < tpTreeInfo->ItemListCnt;i++){
				if(*(tpTreeInfo->ItemList + i) == NULL ||
					(*(tpTreeInfo->ItemList + i))->Title == NULL ||
					lstrcmp((*(tpTreeInfo->ItemList + i))->Title, TmpTitle) != 0){
					continue;
				}

				if(TmpTitle != NULL){
					GlobalFree(TmpTitle);
				}
				if(cnt++ > 100){
					return FALSE;
				}
				//同じアイテムが存在する場合は番号を付けて行く
				wsprintf(TmpBuf, STR_COPYNAME_CNT, cnt);
				TmpTitle = (char *)GlobalAlloc(GPTR, lstrlen(NewItemInfo->Title) + lstrlen(TmpBuf) + 1);
				wsprintf(TmpTitle, "%s%s", TmpBuf, NewItemInfo->Title);
				break;
			}
			if(i >= tpTreeInfo->ItemListCnt){
				break;
			}
		}
		GlobalFree(NewItemInfo->Title);
		NewItemInfo->Title = TmpTitle;
	}

	//アイテムの追加
	ret = Item_Add(hWnd, hItem, NewItemInfo);
	if(ret == -1){
		return FALSE;
	}

	SetItemToUndo(NewItemInfo);

	//コピーの場合はアイテム削除を行わない
	if(mode == FLAG_COPY || *SourcePath == '\0'){
		return TRUE;
	}

	//移動されたアイテムの削除
	if(tpDeleteItemList == NULL){
		return TRUE;
	}
	for(i = 0;i < tpDeleteItemListCnt;i++){
		if(*(tpDeleteItemList + i) == NULL){
			continue;
		}
		//タイトルの比較
		p = ((*(tpDeleteItemList + i))->Title == NULL) ? "" : (*(tpDeleteItemList + i))->Title;
		r = (NewItemInfo->Title == NULL) ? "" : NewItemInfo->Title;
		if(lstrcmp(p, r) != 0){
			continue;
		}
		//URLの比較
		p = ((*(tpDeleteItemList + i))->CheckURL == NULL) ? "" : (*(tpDeleteItemList + i))->CheckURL;
		r = (NewItemInfo->CheckURL == NULL) ? "" : NewItemInfo->CheckURL;
		if(lstrcmp(p, r) != 0){
			continue;
		}

		//アイテムの削除
		FreeItemInfo(*(tpDeleteItemList + i), TRUE);
		GlobalFree(*(tpDeleteItemList + i));
		*(tpDeleteItemList + i) = NULL;
		break;
	}
	return TRUE;
}


/******************************************************************************

	StringToItem

	アイテム情報文字列からアイテム情報を作成する

******************************************************************************/

static int StringToItem(HWND hWnd, HTREEITEM hItem, char *Data, int DnDFlag)
{
	struct TPITEM *tpItemInfo;
	char buf[MAXSIZE];
	char FromPath[BUFSIZE];
	char SourcePath[BUFSIZE];
	char Name[BUFSIZE];
	char TmpBuf[BUFSIZE];
	char ToPath[BUFSIZE];
	char *p, *r, *t;
	int ret = FLAG_COPY;
	int ListFolderCnt;
	int TreeFolderCnt;
	int i;
	BOOL SelInitFlag = FALSE;

	*SourcePath = '\0';
	tpDeleteItemList = NULL;

	//移動先のパス
	TreeView_GetPath(GetDlgItem(hWnd, WWWC_TREE), hItem, TmpBuf, CuDir);
	wsprintf(ToPath, "%s\\"DATAFILENAME, TmpBuf);

	r = buf;
	for(p = Data;*p != '\0';p++){
		if(*p != '\r'){
			if(*p != '\n'){
				*(r++) = *p;
			}
			continue;
		}
		*r = '\0';
		if(*buf == '\0'){
			r = buf;
			continue;
		}

		if(*buf == FLAG_COPY || *buf == FLAG_CUT){
			//編集フラグ + 移動元ファイル
			t = buf + 1;
			//パス
			r = SourcePath;
			for(;*t != '\t' && *t != '\0';t++, r++){
				*r = *t;
			}
			*r = '\0';

			if(*buf == FLAG_COPY || DnDFlag & DND_COPY){
				//*SourcePath = '\0';
			}else{
				//移動時に削除するアイテムリスト
				tpDeleteItemList = ReadItemList(SourcePath, &tpDeleteItemListCnt, NULL);
				ret = FLAG_CUT;
			}

			//Undoの初期化
			InitUndo(hWnd, SourcePath, hItem, ret);

		}else if(*buf == FLAG_FOLDER){
			//フォルダフラグ + 移動元ディレクトリ + フォルダ名
			t = buf + 1;

			r = FromPath;
			for(;*t != '\t' && *t != '\0';t++, r++){
				*r = *t;
			}
			*r = '\0';
			if(*t == '\t'){
				t++;
			}
			//名前
			r = Name;
			for(;*t != '\0';t++, r++){
				*r = *t;
			}
			*r = '\0';

			//コピー、切り取り
			if(CopyFolder(hWnd, hItem, FromPath, Name, ret, DnDFlag) == FALSE){
				ret = -1;
				break;
			}
			if(hItem == TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE))){
				//選択の解除
				if(SelInitFlag == FALSE){
					ListView_SetItemState(GetDlgItem(hWnd, WWWC_LIST), -1, 0, LVIS_SELECTED);
					SelInitFlag = TRUE;
				}
				//リストビューに表示されているフォルダの数
				ListFolderCnt = ListView_GetFolderCount(GetDlgItem(hWnd, WWWC_LIST));
				//ツリービューに表示されているフォルダの数
				TreeFolderCnt = TreeView_GetChildCount(GetDlgItem(hWnd, WWWC_TREE), hItem);

				if(ListFolderCnt < TreeFolderCnt){
					ListView_InsertItemEx(GetDlgItem(hWnd, WWWC_LIST), (char*)LPSTR_TEXTCALLBACK, I_IMAGECALLBACK,
						(long)NULL, ListFolderCnt);
				}

				i = TreeView_GetListIndex(GetDlgItem(hWnd, WWWC_TREE),
					TreeView_CheckName(GetDlgItem(hWnd, WWWC_TREE), hItem, Name));
				if(i != -1){
					ListView_SetItemState(GetDlgItem(hWnd, WWWC_LIST), i, LVIS_SELECTED, LVIS_SELECTED);
				}
			}

			if(UpdateItemFlag == UF_CANCEL){
				break;
			}

		}else{
			//選択の解除
			if(SelInitFlag == FALSE && (ret == FLAG_COPY || lstrcmpi(ToPath, SourcePath) != 0)){
				ListView_SetItemState(GetDlgItem(hWnd, WWWC_LIST), -1, 0, LVIS_SELECTED);
				SelInitFlag = TRUE;
			}

			//アイテム情報
			tpItemInfo = (struct TPITEM *)GlobalAlloc(GPTR, sizeof(struct TPITEM));
			if(tpItemInfo != NULL){
				tpItemInfo->iSize = sizeof(struct TPITEM);
				tpItemInfo->hItem = hItem;
				LineSetItemInfo(tpItemInfo, buf);

				if(CopyItemInfo(hWnd, hItem, tpItemInfo, SourcePath, ret) == FALSE){
					FreeItemInfo(tpItemInfo, FALSE);
					GlobalFree(tpItemInfo);
					if(UpdateItemFlag == UF_CANCEL){
						break;
					}
				}
			}
		}
		r = buf;
	}
	if(tpDeleteItemList != NULL){
		SaveItemList(hWnd, SourcePath, tpDeleteItemList, tpDeleteItemListCnt);
		FreeItemList(tpDeleteItemList, tpDeleteItemListCnt, FALSE);
		GlobalFree(tpDeleteItemList);
	}
	return ret;
}


/******************************************************************************

	Clipboard_Get_WF_String

	アイテム情報文字列を処理する

******************************************************************************/

int Clipboard_Get_WF_String(HWND hWnd, HTREEITEM hItem, char *buf, int DnDFlag)
{
	int ret;

	ListView_SetRedraw(GetDlgItem(hWnd, WWWC_LIST), LDF_NODRAW);
	SendDlgItemMessage(hWnd, WWWC_TREE, WM_SETREDRAW, (WPARAM)FALSE, 0);

	//アイテムを追加
	ret = StringToItem(hWnd, hItem, buf, DnDFlag);
	TreeView_FreeItem(hWnd, hItem, 1);

	//追加されたフォルダをリストビューに反映
	ListView_FolderRedraw(hWnd, FALSE);

	//選択の先頭アイテムにフォーカスを与える
	if(ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), -1, LVNI_SELECTED) != -1){
		ListView_SetItemState(GetDlgItem(hWnd, WWWC_LIST),
			ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), -1, LVNI_SELECTED),
			LVIS_SELECTED | LVIS_FOCUSED, LVIS_FOCUSED | LVIS_SELECTED);
		ListView_EnsureVisible(GetDlgItem(hWnd, WWWC_LIST),
			ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), -1, LVNI_SELECTED), TRUE);
	}

	ListView_SetRedraw(GetDlgItem(hWnd, WWWC_LIST), LDF_REDRAW);

	SendDlgItemMessage(hWnd, WWWC_TREE, WM_SETREDRAW, (WPARAM)TRUE, 0);
	UpdateWindow(GetDlgItem(hWnd, WWWC_TREE));
	SetProtocolMenu(hWnd);
	return ret;
}


/******************************************************************************

	Clipboard_CheckFormat

	クリップボードフォーマットのチェック

******************************************************************************/

int Clipboard_CheckFormat(void)
{
	UINT ClipFormat[CLIPFORMAT_CNT];

	ClipFormat[0] = WWWC_ClipFormat;
	ClipFormat[1] = CF_HDROP;
	ClipFormat[2] = CF_TEXT;
	return GetPriorityClipboardFormat(ClipFormat, CLIPFORMAT_CNT);
}


/******************************************************************************

	Clipboard_GetData

	クリップボードからデータを取得し処理する

******************************************************************************/

void Clipboard_GetData(HWND hWnd)
{
	HTREEITEM hItem;
	HANDLE hClip;
	char *buf;
	int ret;

	//クリップボードフォーマットのチェック
	ret = Clipboard_CheckFormat();
	if(ret <= 0){
		return;
	}
	//クリップボードを開く
	if(OpenClipboard(hWnd) == FALSE){
		return;
	}

	switch(ret)
	{
	case CF_TEXT:		//テキスト
		hClip = GetClipboardData(CF_TEXT);
		if((buf = GlobalLock(hClip)) == NULL){
			break;
		}
		//プロトコルを識別できた場合はアイテムとして追加
		if(GetProtocolIndex(buf) != -1){
			SetFocus(GetDlgItem(hWnd, WWWC_LIST));
			Item_UrlAdd(hWnd, NEWITEMNAME, buf, 0, NULL);
		}
		GlobalUnlock(hClip);
		break;

	case CF_HDROP:		//ドロップファイル
		hClip = GetClipboardData(CF_HDROP);
		DragDrop_GetDropFiles(hWnd, hClip, TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)));
		break;

	default:			//独自形式
		hItem = TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE));

		hClip = GetClipboardData(WWWC_ClipFormat);
		if((buf = GlobalLock(hClip)) == NULL){
			break;
		}
		SetFocus(GetDlgItem(hWnd, WWWC_LIST));
		WaitCursor(TRUE);

		UpdateItemFlag = UF_COPY;
		//アイテムを追加
		ret = Clipboard_Get_WF_String(hWnd, hItem, buf, 0);
		GlobalUnlock(hClip);

		if(ret == FLAG_CUT){
			EmptyClipboard();
		}

		TreeView_SetIconState(hWnd, TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)), 0);
		WaitCursor(FALSE);
		break;
	}
	CloseClipboard();
}
/* End of source */
