/**************************************************************************

	WWWC

	ListView.c

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

#define DAMMYSORT				500

#define DEFAULT_COLSIZE			100					//デフォルトのカラムサイズ

#ifndef LVS_EX_INFOTIP
#define LVS_EX_INFOTIP			0x400
#endif
#ifndef LVM_SETSELECTEDCOLUMN
#define LVM_SETSELECTEDCOLUMN	(LVM_FIRST + 140)
#endif
#ifndef ListView_SetSelectedColumn
#define ListView_SetSelectedColumn(hwnd, iCol) \
			SNDMSG((hwnd), LVM_SETSELECTEDCOLUMN, (WPARAM)iCol, 0)
#endif
#ifndef HDF_SORTDOWN
#define HDF_SORTDOWN			0x0200
#endif
#ifndef HDF_SORTUP
#define HDF_SORTUP				0x0400
#endif

/**************************************************************************
	Global Variables
**************************************************************************/

struct TPLVCOLUMN *SortColInfo;	//ソート情報
struct TPLVCOLUMN *ColumnInfo = NULL;

int DragItemIndex = -1;

//外部参照
extern HINSTANCE g_hinst;				//アプリケーションのインスタンスハンドル
extern char CuDir[];
extern HWND WWWCWnd;					//本体
extern HWND TbWnd;						//ツールバー
extern HMENU hPOPUP;					//ポップアップメニュー
extern HTREEITEM RootItem;
extern HTREEITEM RecyclerItem;
extern HTREEITEM DgdpItem;
extern struct TPLVCOLUMN *ColumnInfo;
extern BOOL WWWCDropFlag;
extern UINT WWWC_ClipFormat;

extern struct TP_PROTOCOL *tpProtocol;
extern int ProtocolCnt;

extern int LvStyle;
extern int LvSortFlag;
extern int LvAutoSort;
extern int LvDblclkUpDir;
extern int LvWndStyle;
extern char LvBkColor[];

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

extern char Up[];
extern int UpIndex;
extern char Error[];
extern int ErrorIndex;
extern char TimeOut[];
extern int TimeOutIndex;
extern char NoProtocol[];
extern int NoProtocolIndex;
extern char Wait[];
extern int WaitIndex;

extern int DnDReturnIcon;


/**************************************************************************
	Local Function Prototypes
**************************************************************************/

static int SetColmunInfo(char *Colbuf, struct TPLVCOLUMN *ColInfo);
static void SetListMenuUnCheck(HWND hWnd, int id, int st);


/******************************************************************************

	CreateListView

	リストビューの作成

******************************************************************************/

HWND CreateListView(HWND hWnd)
{
	return CreateWindowEx(WS_EX_NOPARENTNOTIFY,
		WC_LISTVIEW, (LPSTR)NULL, WS_TABSTOP | WS_VISIBLE | WS_CHILD |
		LvWndStyle | ABS(LvStyle),
		0, 0, 0, 0, hWnd, (HMENU)WWWC_LIST, g_hinst, NULL);
}


/******************************************************************************

	ListView_SetItemImage

	リストビューのイメージリストの設定

******************************************************************************/

void ListView_SetItemImage(HWND hListView, int IconSize, int LVFLag)
{
	HIMAGELIST IconList;
	HIMAGELIST TmpIconList;
	HICON TmpIcon;

	//イメージリストの作成
	IconList = ListView_GetImageList(hListView, LVFLag);
	if(IconList == NULL){
		IconList = ImageList_Create(IconSize, IconSize, ILC_COLOR16 | ILC_MASK, 0, 0);
	}else{
		ImageList_Remove(IconList, -1);
	}

	if(*LvBkColor != '\0'){
		ImageList_SetBkColor(IconList, strtol(LvBkColor, NULL, 0));
	}else{
		ImageList_SetBkColor(IconList, GetSysColor(COLOR_WINDOW));
	}

	//共通のアイコン
	ImageListIconAdd(IconList, IDI_ICON_CHECK, IconSize, Check, CheckIndex);
	ImageListIconAdd(IconList, IDI_ICON_NOCHECK, IconSize, NoCheck, NoCheckIndex);
	ImageList_SetOverlayImage(IconList, ICON_NOCHECK, 1);
	ImageListIconAdd(IconList, IDI_ICON_DIRUP, IconSize, DirUP, DirUPIndex);
	ImageListFileIconAdd(IconList, CuDir, 0, IconSize, Dir, DirIndex);
	ImageListIconAdd(IconList, IDI_ICON_UPCHILD, IconSize, DirUPchild, DirUPchildIndex);
	ImageListIconAdd(IconList, IDI_ICON_CHECKCHILD, IconSize, CheckChild, CheckChildIndex);

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

	ImageListIconAdd(IconList, IDI_ICON_RECYCLER, IconSize, Recycler, RecyclerIndex);
	ImageListIconAdd(IconList, IDI_ICON_RECYCLER_USE, IconSize, RecyclerFull, RecyclerFullIndex);

	//リストビューのアイコン
	ImageListIconAdd(IconList, IDI_ICON_UP, IconSize, Up, UpIndex);
	ImageListIconAdd(IconList, IDI_ICON_ERROR, IconSize, Error, ErrorIndex);
	ImageListIconAdd(IconList, IDI_ICON_TIMEOUT, IconSize, TimeOut, TimeOutIndex);
	ImageListIconAdd(IconList, IDI_ICON_NO, IconSize, NoProtocol, NoProtocolIndex);
	ImageListIconAdd(IconList, IDI_ICON_WAIT, IconSize, Wait, WaitIndex);

	//リストビューにイメージリストを設定
	ListView_SetImageList(hListView, IconList, LVFLag);
}


/******************************************************************************

	SetColmunInfo

	カラム情報の設定

******************************************************************************/

static int SetColmunInfo(char *Colbuf, struct TPLVCOLUMN *ColInfo)
{
	struct TPLVCOLUMN *TmpColumnInfo;
	char buf[BUFSIZE], *p, *r;
	int Cnt = 0;

	for(p = Colbuf;*p != '\0';p++){
		if(*p == ','){
			Cnt++;
		}
	}
	Cnt++;

	TmpColumnInfo = ColInfo;

	lstrcpy(TmpColumnInfo->Title, COLUMN_NAME);
	TmpColumnInfo->fmt = LVCFMT_LEFT;
	TmpColumnInfo->p = sizeof(long) + sizeof(HTREEITEM);
	TmpColumnInfo++;

	r = buf;
	for(p = Colbuf;*p != '\0';p++){
		if(*p != ','){
			*(r++) = *p;

		}else{
			*r = '\0';

			TmpColumnInfo->p = sizeof(long) + sizeof(HTREEITEM);
			switch(atoi(buf))
			{
			case 0:
				lstrcpy(TmpColumnInfo->Title, COLUMN_NAME);
				TmpColumnInfo->fmt = LVCFMT_LEFT;
//				TmpColumnInfo->p = sizeof(long) + sizeof(HTREEITEM);
				break;

			case 1:
				lstrcpy(TmpColumnInfo->Title, COLUMN_CHECKURL);
				TmpColumnInfo->fmt = LVCFMT_LEFT;
				TmpColumnInfo->p += sizeof(char *);
				break;

			case 2:
				lstrcpy(TmpColumnInfo->Title, COLUMN_SIZE);
				TmpColumnInfo->fmt = LVCFMT_RIGHT;
				TmpColumnInfo->p += sizeof(char *) * 2;
				break;

			case 3:
				lstrcpy(TmpColumnInfo->Title, COLUMN_DATE);
				TmpColumnInfo->fmt = LVCFMT_LEFT;
				TmpColumnInfo->p += sizeof(char *) * 3;
				break;

			case 4:
				lstrcpy(TmpColumnInfo->Title, COLUMN_CHECKDATE);
				TmpColumnInfo->fmt = LVCFMT_LEFT;
				TmpColumnInfo->p += sizeof(char *) * 4 + sizeof(int);
				break;

			case 5:
				lstrcpy(TmpColumnInfo->Title, COLUMN_OLDSIZE);
				TmpColumnInfo->fmt = LVCFMT_RIGHT;
				TmpColumnInfo->p += sizeof(char *) * 5 + sizeof(int);
				break;

			case 6:
				lstrcpy(TmpColumnInfo->Title, COLUMN_OLDDATE);
				TmpColumnInfo->fmt = LVCFMT_LEFT;
				TmpColumnInfo->p += sizeof(char *) * 6 + sizeof(int);
				break;

			case 7:
				lstrcpy(TmpColumnInfo->Title, COLUMN_VIEWURL);
				TmpColumnInfo->fmt = LVCFMT_LEFT;
				TmpColumnInfo->p += sizeof(char *) * 7 + sizeof(int);
				break;

			case 8:
				lstrcpy(TmpColumnInfo->Title, COLUMN_OPTION1);
				TmpColumnInfo->fmt = LVCFMT_LEFT;
				TmpColumnInfo->p += sizeof(char *) * 8 + sizeof(int);
				break;

			case 9:
				lstrcpy(TmpColumnInfo->Title, COLUMN_OPTION2);
				TmpColumnInfo->fmt = LVCFMT_LEFT;
				TmpColumnInfo->p += sizeof(char *) * 9 + sizeof(int);
				break;

			case 10:
				lstrcpy(TmpColumnInfo->Title, COLUMN_COMMENT);
				TmpColumnInfo->fmt = LVCFMT_LEFT;
				TmpColumnInfo->p += sizeof(char *) * 10 + sizeof(int);
				break;

			case 11:
				lstrcpy(TmpColumnInfo->Title, COLUMN_PATH);
				TmpColumnInfo->fmt = LVCFMT_LEFT;
				TmpColumnInfo->p = 0;
				break;

			case 12:
				lstrcpy(TmpColumnInfo->Title, COLUMN_ERROR);
				TmpColumnInfo->fmt = LVCFMT_LEFT;
				TmpColumnInfo->p += sizeof(char *) * 11 + sizeof(int) * 2;
				break;

			case 13:
				lstrcpy(TmpColumnInfo->Title, COLUMN_DLLDATA1);
				TmpColumnInfo->fmt = LVCFMT_LEFT;
				TmpColumnInfo->p += sizeof(char *) * 12 + sizeof(int) * 2;
				break;

			case 14:
				lstrcpy(TmpColumnInfo->Title, COLUMN_DLLDATA2);
				TmpColumnInfo->fmt = LVCFMT_LEFT;
				TmpColumnInfo->p += sizeof(char *) * 13 + sizeof(int) * 2;
				break;

			default:
				r = buf;
				Cnt--;
				continue;
			}
			TmpColumnInfo++;
			r = buf;
		}
	}
	return Cnt;
}


/******************************************************************************

	ListView_AddColumn

	カラムの追加

******************************************************************************/

int ListView_AddColumn(HWND hListView, char *chLvColumn, int *lColSize, struct TPLVCOLUMN *ColInfo)
{
	LV_COLUMN lvc = { 0 };
	int i, cnt;

	if(ColInfo == NULL){
		return 0;
	}

	while(ListView_DeleteColumn(hListView, 0) == TRUE);

	cnt = SetColmunInfo(chLvColumn, ColInfo);
	if(cnt == 0){
		lstrcpy(ColInfo->Title, COLUMN_NAME);
		ColInfo->fmt = LVCFMT_LEFT;
		ColInfo->p = sizeof(HTREEITEM);
		cnt = 1;
	}

	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;

	for(i = 0;i < cnt;i++){
		lvc.fmt = ColInfo[i].fmt;
		if(lColSize[i] == 0){
			lColSize[i] = DEFAULT_COLSIZE;
		}
		lvc.cx = lColSize[i];
		lvc.pszText = ColInfo[i].Title;
		lvc.iSubItem = i;
		ListView_InsertColumn(hListView, i, &lvc);
	}
	return cnt;
}


/******************************************************************************

	CompareFunc

	ソートフラグに従って文字列の比較を行う

******************************************************************************/

int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	char buf1[BUFSIZE], buf2[BUFSIZE];
	char tmp1[BUFSIZE], tmp2[BUFSIZE];
	int ret, sfg, ghed;

	if(lParamSort == DAMMYSORT){
		return 0;
	}

	sfg = (lParamSort < 0) ? 1 : 0;	//昇順／降順
	ghed = ABS(lParamSort) - 1;		//ソートを行うヘッダ

	*buf1 = '\0';
	*buf2 = '\0';

	if(lParam1 == 0 || lParam2 == 0){
		return 0;
	}

	if(ghed == 100){
		//アイコン順にソート
		if(((struct TPITEM *)lParam1)->Status == ST_DEFAULT){
			lstrcpy(buf1, "9");
		}else{
			wsprintf(buf1, "%d", ((struct TPITEM *)lParam1)->Status);
		}

		if(((struct TPITEM *)lParam2)->Status == ST_DEFAULT){
			lstrcpy(buf2, "9");
		}else{
			wsprintf(buf2, "%d", ((struct TPITEM *)lParam2)->Status);
		}

	}else if((SortColInfo + ghed)->p == 0){
		//アイテムが格納されているフォルダのパス
		if(IsTreeItem(WWWCWnd, ((struct TPITEM *)lParam1)->hItem) == FALSE){
			*buf1 = '\0';
		}else{
			TreeView_GetItemInfo(GetDlgItem(WWWCWnd, WWWC_TREE), RootItem, tmp1);
			wsprintf(tmp2, "\\\\%s", tmp1);
			TreeView_GetPath(GetDlgItem(WWWCWnd, WWWC_TREE),
				((struct TPITEM *)lParam1)->hItem, buf1, tmp2);
		}
		if(IsTreeItem(WWWCWnd, ((struct TPITEM *)lParam2)->hItem) == FALSE){
			*buf2 = '\0';
		}else{
			TreeView_GetItemInfo(GetDlgItem(WWWCWnd, WWWC_TREE), RootItem, tmp1);
			wsprintf(tmp2, "\\\\%s", tmp1);
			TreeView_GetPath(GetDlgItem(WWWCWnd, WWWC_TREE),
				((struct TPITEM *)lParam2)->hItem, buf2, tmp2);
		}

	}else{
		//比較を行う文字列
		if((char *)*((long *)((long)((struct TPITEM *)lParam1) + (SortColInfo + ghed)->p)) != NULL){
			lstrcpyn(buf1,
				(char *)*((long *)((long)((struct TPITEM *)lParam1) + (SortColInfo + ghed)->p)),
				BUFSIZE - 1);
		}
		if((char *)*((long *)((long)((struct TPITEM *)lParam2) + (SortColInfo + ghed)->p)) != NULL){
			lstrcpyn(buf2,
				(char *)*((long *)((long)((struct TPITEM *)lParam2) + (SortColInfo + ghed)->p)),
				BUFSIZE - 1);
		}

		//数値の場合
		if((SortColInfo + ghed)->fmt == LVCFMT_RIGHT){
			GetNumString(buf1, tmp1);
			GetNumString(buf2, tmp2);

			wsprintf(buf1, "%20ld", atol(tmp1));
			wsprintf(buf2, "%20ld", atol(tmp2));
		}
	}

	//小文字に変換して比較を行う
	CharLower(buf1);
	CharLower(buf2);
	ret = lstrcmp(buf1, buf2);

	if(ret < 0 && sfg == 1){
		return 1;
	}
	if(ret > 0 && sfg == 0){
		return 1;
	}

	return -1;
}


/******************************************************************************

	ListView_SetRedraw

	リストビューの描画切り替え

******************************************************************************/

void ListView_SetRedraw(HWND hListView, int DrawFlag)
{
	switch(DrawFlag)
	{
	case LDF_NODRAW:
		SendMessage(hListView, WM_SETREDRAW, (WPARAM)FALSE, 0);
		break;

	case LDF_REDRAW:
		SendMessage(hListView, WM_SETREDRAW, (WPARAM)TRUE, 0);
		UpdateWindow(hListView);
		break;
	}
}


/******************************************************************************

	ListView_InsertItemEx

	指定位置にアイテムを挿入

******************************************************************************/

int ListView_InsertItemEx(HWND hListView, char *buf, int Img, long lp, int iItem)
{
	LV_ITEM lvi = { 0 };

	lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;

	if(iItem == -1){
		iItem = ListView_GetItemCount(hListView);
	}
	lvi.iItem = iItem;
	lvi.iSubItem = 0;
	lvi.pszText = buf;
	lvi.cchTextMax = BUFSIZE;
	lvi.iImage = Img;
	lvi.lParam = lp;

	// リストビューにアイテムを追加する
	return ListView_InsertItem(hListView, &lvi);
}


/******************************************************************************

	ListView_SetItemTitle

	アイテムのタイトルを設定する

******************************************************************************/

void ListView_SetItemTitle(HWND hWnd, LV_DISPINFO *plv)
{
	struct TPITEM *tpItemInfo;
	HTREEITEM hItem;
	int i;

	if(plv->item.pszText == NULL || plv->item.pszText[0] == '\0'){
		return;
	}

	tpItemInfo = (struct TPITEM *)ListView_GetlParam(GetDlgItem(hWnd, WWWC_LIST), plv->item.iItem);
	if(tpItemInfo != NULL){
		//Undoバッファにセット
		SetLVTitleToUndo(TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)), tpItemInfo->Title, plv->item.pszText, tpItemInfo->CheckURL);
		//リストビューのアイテムのタイトルを設定する
		GlobalFree(tpItemInfo->Title);
		tpItemInfo->Title = (char *)GlobalAlloc(GMEM_FIXED, lstrlen(plv->item.pszText) + 1);
		if(tpItemInfo->Title == NULL){
			ErrMsg(hWnd, GetLastError(), NULL);
			return;
		}
		lstrcpy(tpItemInfo->Title, plv->item.pszText);
		//アイテムの再設定
		ListView_SetItemText(GetDlgItem(hWnd, WWWC_LIST), plv->item.iItem, 0, LPSTR_TEXTCALLBACK);
	}else{
		//対応するツリービューアイテムのハンドルの取得
		hItem = TreeView_GetIndexToItem(GetDlgItem(hWnd, WWWC_TREE),
			TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)), plv->item.iItem);
		if(hItem == NULL){
			return;
		}
		//ツリービューアイテムのタイトルを変更する
		TreeView_SetName(hWnd, hItem, plv->item.pszText);

		ListView_RefreshFolder(hWnd);

		//名前の変更されたアイテムを選択する
		if((i = TreeView_GetListIndex(GetDlgItem(hWnd, WWWC_TREE), hItem)) != -1){
			ListView_SetItemState(GetDlgItem(hWnd, WWWC_LIST),
				i, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
			ListView_EnsureVisible(GetDlgItem(hWnd, WWWC_LIST), i, TRUE);
		}
	}
}


/******************************************************************************

	ListView_SetItemIcon

	リストビューのアイコンを設定

******************************************************************************/

void ListView_SetItemIcon(HWND hListView, int i, int Icon)
{
	LV_ITEM lvi = { 0 };

	lvi.mask = LVIF_IMAGE;
	lvi.iItem = i;
	lvi.iSubItem = 0;
	lvi.iImage = Icon;

	ListView_SetItem(hListView, &lvi);
}


/******************************************************************************

	ListView_MoveItem

	リストビューのアイテムを移動

******************************************************************************/

void ListView_MoveItem(HWND hListView, int SelectItem, int Move, int ColSize)
{
#define MOVEITEM_MAX_COLSIZE	30
	LV_ITEM lvItem = { 0 };
	char buf[MOVEITEM_MAX_COLSIZE][BUFSIZE];
	int i = 0;

	for(i = 0;i < ColSize;i++){
		*(buf[i]) = '\0';
	}

	lvItem.mask = LVIF_IMAGE;
	lvItem.iItem = SelectItem;
	lvItem.iSubItem = 0;
	lvItem.iImage = 0;
	if(ListView_GetItem(hListView, &lvItem) == FALSE){
		return;
	}
	for(i = 0;i < ColSize;i++){
		ListView_GetItemText(hListView, SelectItem, i, buf[i], BUFSIZE - 1);
	}
	ListView_DeleteItem(hListView, SelectItem);

	SelectItem = SelectItem + Move;

	ListView_InsertItemEx(hListView, buf[0], lvItem.iImage, 0, SelectItem);
	for(i = 1;i < ColSize;i++){
		ListView_SetItemText(hListView, SelectItem, i, buf[i]);
	}
	ListView_SetItemState(hListView, SelectItem,
		LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	ListView_EnsureVisible(hListView, SelectItem, TRUE);
}


/******************************************************************************

	ListView_GetFolderCount

	リストビューに表示されているフォルダの数を取得

******************************************************************************/

int ListView_GetFolderCount(HWND hListView)
{
	struct TPITEM *tpItemInfo;
	int cnt;
	int ret = 0;

	//リストビューに表示されているフォルダの数
	cnt = ListView_GetItemCount(hListView);
	for(; ret < cnt; ret++){
		tpItemInfo = (struct TPITEM *)ListView_GetlParam(hListView, ret);
		if(tpItemInfo != NULL){
			break;
		}
	}
	return ret;
}


/******************************************************************************

	ListView_GetMemToIndex

	アイテム情報からアイテムのインデックスを取得

******************************************************************************/

int ListView_GetMemToIndex(HWND hListView, struct TPITEM *tpItemInfo)
{
	int cnt;
	int i;

	cnt = ListView_GetItemCount(hListView);
	for(i = 0; i < cnt; i++){
		if(tpItemInfo == (struct TPITEM *)ListView_GetlParam(hListView, i)){
			return i;
		}
	}
	return -1;
}


/******************************************************************************

	ListView_GetlParam

	アイテムのLPARAMを取得

******************************************************************************/

long ListView_GetlParam(HWND hListView, int i)
{
	LV_ITEM lvi = { 0 };
	long j = 0;

	lvi.mask = LVIF_PARAM;
	lvi.iItem = i;
	lvi.iSubItem = 0;
	lvi.lParam = j;
	ListView_GetItem(hListView, &lvi);
	return lvi.lParam;
}


/******************************************************************************

	ListView_SetlParam

	アイテムのLPARAMを設定

******************************************************************************/

void ListView_SetlParam(HWND hListView, int i, long lParam)
{
	LV_ITEM lvi = { 0 };

	lvi.mask = LVIF_PARAM;
	lvi.iItem = i;
	lvi.iSubItem = 0;
	lvi.lParam = lParam;
	ListView_SetItem(hListView, &lvi);
}


/******************************************************************************

	ListView_SetListToMem

	リストビューのアイテムからアイテム情報を作成

******************************************************************************/

struct TPITEM **ListView_SetListToMem(HWND hWnd, int *RetItemCnt)
{
	struct TPITEM **RetItemList;
	struct TPITEM *tpItemInfo;
	int ItemCnt, i, j;

	ItemCnt = ListView_GetItemCount(GetDlgItem(hWnd, WWWC_LIST));
	for(i = 0;i < ItemCnt;i++){
		tpItemInfo = (struct TPITEM *)ListView_GetlParam(GetDlgItem(hWnd, WWWC_LIST), i);
		if(tpItemInfo != NULL){
			break;
		}
	}
	RetItemList = (struct TPITEM **)GlobalAlloc(GPTR, sizeof(struct TPITEM *) * (ItemCnt - i));
	if(RetItemList == NULL){
		abort();
	}
	*RetItemCnt = ItemCnt - i;

	j = 0;
	for(;i < ItemCnt;i++){
		tpItemInfo = (struct TPITEM *)ListView_GetlParam(GetDlgItem(hWnd, WWWC_LIST), i);
		if(tpItemInfo == NULL){
			continue;
		}
		*(RetItemList + j) = tpItemInfo;
		j++;
	}
	return RetItemList;
}


/******************************************************************************

	ListView_SelectItemToMem

	リストビューの選択アイテムからアイテム情報を作成

******************************************************************************/

struct TPITEM **ListView_SelectItemToMem(HWND hListView, int *cnt)
{
	struct TPITEM **RetItemList;
	struct TPITEM *tpItemInfo;
	int i;

	//選択アイテム数の取得
	i = -1;
	*cnt = 0;
	while((i = ListView_GetNextItem(hListView, i, LVNI_SELECTED)) != -1){
		tpItemInfo = (struct TPITEM *)ListView_GetlParam(hListView, i);
		if(tpItemInfo == NULL){
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
	i = -1;
	*cnt = 0;
	while((i = ListView_GetNextItem(hListView, i, LVNI_SELECTED)) != -1){
		tpItemInfo = (struct TPITEM *)ListView_GetlParam(hListView, i);
		if(tpItemInfo == NULL){
			continue;
		}
		*(RetItemList + *cnt) = tpItemInfo;
		(*cnt)++;
	}
	return RetItemList;
}


/******************************************************************************

	ListView_GetHiTestItem

	マウスの下のアイテムのインデックスを取得

******************************************************************************/

int ListView_GetHiTestItem(HWND hListView)
{
	LV_HITTESTINFO lvht = { 0 };
	POINT apos;
	RECT LvRect;

	GetCursorPos((LPPOINT)&apos);
	GetWindowRect(hListView, (LPRECT)&LvRect);
	apos.x = apos.x - LvRect.left;
	apos.y = apos.y - LvRect.top;

	lvht.pt = apos;
	lvht.flags = LVHT_ONITEMICON | LVHT_ONITEMLABEL | LVHT_ONITEMSTATEICON;
	lvht.iItem = 0;

	return ListView_HitTest(hListView, &lvht);
}


/******************************************************************************

	ListView_SwitchSelectItem

	アイテムの選択の切り替え

******************************************************************************/

void ListView_SwitchSelectItem(HWND hListView)
{
	int cnt, i;

	cnt = ListView_GetItemCount(hListView);

	for(i = 0; i < cnt; i++){
		if(ListView_GetItemState(hListView, i, LVIS_SELECTED) == LVIS_SELECTED){
			ListView_SetItemState(hListView, i, 0, LVIS_SELECTED);
		}else{
			ListView_SetItemState(hListView, i, LVIS_SELECTED, LVIS_SELECTED);
		}
	}
}


/******************************************************************************

	ListView_UpSelectItem

	UPアイコンの選択

******************************************************************************/

void ListView_UpSelectItem(HWND hListView)
{
	struct TPITEM *tpItemInfo;
	int cnt, i;

	cnt = ListView_GetItemCount(hListView);
	for(i = 0; i < cnt; i++){
		tpItemInfo = (struct TPITEM *)ListView_GetlParam(hListView, i);
		if(tpItemInfo != NULL && tpItemInfo->Status == ST_UP){
			ListView_SetItemState(hListView, i, LVIS_SELECTED, LVIS_SELECTED);
		}else{
			ListView_SetItemState(hListView, i, 0, LVIS_SELECTED);
		}
	}
	ListView_SetItemState(hListView,
		ListView_GetNextItem(hListView, -1, LVIS_SELECTED), LVIS_FOCUSED, LVIS_FOCUSED);
	ListView_EnsureVisible(hListView,
		ListView_GetNextItem(hListView, -1, LVIS_SELECTED), TRUE);
}


/******************************************************************************

	ListView_IsRecyclerItem

	ごみ箱が選択されていないかチェックする

******************************************************************************/

BOOL ListView_IsRecyclerItem(HWND hWnd)
{
	int SelectItem;

	SelectItem = -1;
	while((SelectItem = ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), SelectItem, LVNI_SELECTED)) != -1){
		if(TreeView_GetIndexToItem(GetDlgItem(hWnd, WWWC_TREE),
			TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)), SelectItem) == RecyclerItem){
			return TRUE;
		}
	}
	return FALSE;
}


/******************************************************************************

	SetListMenuUnCheck

	リストビュースタイルメニューのチェックを解除する

******************************************************************************/

static void SetListMenuUnCheck(HWND hWnd, int id, int st)
{
	if(st == 1){
		CheckMenuRadioItem(GetSubMenu(GetMenu(hWnd), MENU_HWND_VIEW), ID_MENUITEM_VIEW_ICON,
			ID_MENUITEM_VIEW_REPORT_LINE, id, MF_BYCOMMAND);

		CheckMenuRadioItem(GetSubMenu(hPOPUP, MENU_POP_FOLDER), ID_MENUITEM_VIEW_ICON,
			ID_MENUITEM_VIEW_REPORT_LINE, id, MF_BYCOMMAND);
	}
	SendMessage(TbWnd, TB_CHECKBUTTON, id, (LPARAM) MAKELONG(st, 0));
	SendMessage(TbWnd, TB_PRESSBUTTON, id, (LPARAM) MAKELONG(st, 0));
}


/******************************************************************************

	ListView_StyleChange

	リストビューのスタイルにしたがってウィンドウのメニュー項目を変更する

******************************************************************************/

void ListView_StyleChange(HWND hWnd, int NewStyle)
{
	int st;

	SetListMenuUnCheck(hWnd, ID_MENUITEM_VIEW_ICON, 0);
	SetListMenuUnCheck(hWnd, ID_MENUITEM_VIEW_SMALLICON, 0);
	SetListMenuUnCheck(hWnd, ID_MENUITEM_VIEW_LIST, 0);
	SetListMenuUnCheck(hWnd, ID_MENUITEM_VIEW_REPORT, 0);
	SetListMenuUnCheck(hWnd, ID_MENUITEM_VIEW_REPORT_LINE, 0);

	st = SendDlgItemMessage(hWnd, WWWC_LIST, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0);
	if((st & LVS_EX_FULLROWSELECT) != 0){
		st ^= LVS_EX_FULLROWSELECT;
	}
	SendDlgItemMessage(hWnd, WWWC_LIST, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, st);

	switch(NewStyle)
	{
	case LVS_ICON:				//アイコン
		SetListMenuUnCheck(hWnd, ID_MENUITEM_VIEW_ICON, 1);
		break;

	case LVS_SMALLICON:			//小さいアイコン
		SetListMenuUnCheck(hWnd, ID_MENUITEM_VIEW_SMALLICON, 1);
		break;

	case LVS_LIST:				//リスト
		SetListMenuUnCheck(hWnd, ID_MENUITEM_VIEW_LIST, 1);
		break;

	case LVS_REPORT:			//詳細
		SetListMenuUnCheck(hWnd, ID_MENUITEM_VIEW_REPORT, 1);

		InvalidateRect(GetDlgItem(hWnd, WWWC_LIST), NULL, TRUE);
		UpdateWindow(GetDlgItem(hWnd, WWWC_LIST));
		break;

	default:					//詳細で１行選択
		SetListMenuUnCheck(hWnd, ID_MENUITEM_VIEW_REPORT_LINE, 1);
		SendDlgItemMessage(hWnd, WWWC_LIST, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT |
			SendDlgItemMessage(hWnd, WWWC_LIST, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0));

		InvalidateRect(GetDlgItem(hWnd, WWWC_LIST), NULL, TRUE);
		UpdateWindow(GetDlgItem(hWnd, WWWC_LIST));
		break;
	}
}


/******************************************************************************

	ListView_SetStyle

	リストビューのスタイルを変更する

******************************************************************************/

void ListView_SetStyle(HWND hWnd, int NewStyle)
{
	long wl;

	wl = GetWindowLong(GetDlgItem(hWnd, WWWC_LIST), GWL_STYLE);
	wl ^= ABS(LvStyle);

	LvStyle = NewStyle;
	SetWindowLong(GetDlgItem(hWnd, WWWC_LIST), GWL_STYLE, wl | ABS(LvStyle));

	ListView_StyleChange(hWnd, NewStyle);
}


/******************************************************************************

	ListView_GetDispItem

	リストビューに表示するアイテム情報の設定

******************************************************************************/

BOOL ListView_GetDispItem(HWND hWnd, LV_ITEM *hLVItem)
{
	struct TPTREE *tpTreeInfo;
	struct TPITEM *tpItemInfo;
	char buf[BUFSIZE];
	HTREEITEM hItem;
	int i;

	tpItemInfo = (struct TPITEM *)ListView_GetlParam(GetDlgItem(hWnd, WWWC_LIST), hLVItem->iItem);
	//フォルダ
	if(tpItemInfo == NULL){
		if((hItem = TreeView_GetIndexToItem(GetDlgItem(hWnd, WWWC_TREE),
			TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)), hLVItem->iItem)) == NULL){
			return FALSE;
		}

		i = TreeView_GetItemInfo(GetDlgItem(hWnd, WWWC_TREE), hItem, buf);
		//テキスト
		if(hLVItem->mask & LVIF_TEXT){
			*(hLVItem->pszText) = '\0';
			if(lstrcmp((ColumnInfo + hLVItem->iSubItem)->Title, COLUMN_NAME) == 0){
				lstrcpyn(hLVItem->pszText, buf, BUFSIZE - 1);
			}else if(lstrcmp((ColumnInfo + hLVItem->iSubItem)->Title, COLUMN_CHECKURL) == 0){
				lstrcpy(hLVItem->pszText, FOLDERNAME);
			}else if(lstrcmp((ColumnInfo + hLVItem->iSubItem)->Title, COLUMN_COMMENT) == 0){
				tpTreeInfo = (struct TPTREE *)TreeView_GetlParam(GetDlgItem(hWnd, WWWC_TREE), hItem);
				if(tpTreeInfo != NULL && tpTreeInfo->Comment != NULL){
					lstrcpyn(hLVItem->pszText, tpTreeInfo->Comment, BUFSIZE - 1);
				}
			}
		}
		//アイコン
		if(hLVItem->mask & LVIF_IMAGE){
			hLVItem->iImage = i;
		}
		return FALSE;
	}

	//アイテム
	//テキスト
	if(hLVItem->mask & LVIF_TEXT){
		if((char *)*((long *)((long)tpItemInfo + (ColumnInfo + hLVItem->iSubItem)->p)) != NULL){
			lstrcpyn(hLVItem->pszText,
				(char *)*((long *)((long)tpItemInfo + (ColumnInfo + hLVItem->iSubItem)->p)), BUFSIZE - 1);
		}else{
			*(hLVItem->pszText) = '\0';
		}
	}

	if((hLVItem->mask & LVIF_IMAGE) == 0){
		return FALSE;
	}

	//アイコン
	i = GetProtocolIndex(tpItemInfo->CheckURL);
	if(tpItemInfo->IconStatus == ST_DEFAULT || i == -1){
		if(tpItemInfo->Status & ST_ERROR){
			hLVItem->iImage = ICON_ERR;

		}else if(tpItemInfo->Status & ST_TIMEOUT){
			hLVItem->iImage = ICON_TIMEOUT;

		}else if(tpItemInfo->Status & ST_UP){
			hLVItem->iImage = (i == -1) ? ICON_UP : (tpProtocol + i)->Icon + 3;

		}else{
			hLVItem->iImage = (i == -1) ? ICON_NOICON : (tpProtocol + i)->Icon;
		}
	}else{
		switch(tpItemInfo->IconStatus)
		{
		case ST_NOCHECK:
			hLVItem->iImage = (tpProtocol + i)->Icon + 1;
			break;

		case ST_CHECK:
			hLVItem->iImage = (tpProtocol + i)->Icon + 2;
			break;
		}
	}
	return FALSE;
}


/******************************************************************************

	ListView_FolderRedraw

	リストビューに表示しているフォルダの再表示を行う

******************************************************************************/

void ListView_FolderRedraw(HWND hWnd, BOOL SelFlag)
{
	HTREEITEM cItem;
	int i;

	//アイテムの表示状態を設定
	cItem = TreeView_GetChild(GetDlgItem(hWnd, WWWC_TREE), TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)));
	i = 0;
	while(cItem != NULL){
		ListView_SetItemText(GetDlgItem(hWnd, WWWC_LIST), i, 0, LPSTR_TEXTCALLBACK);
		//チェックする／しないアイコンの付加
		if(TreeView_GetState(GetDlgItem(hWnd, WWWC_TREE), cItem, TVIS_OVERLAYMASK) & INDEXTOOVERLAYMASK(ICON_ST_NOCHECK)){
			ListView_SetItemState(GetDlgItem(hWnd, WWWC_LIST), i, INDEXTOOVERLAYMASK(ICON_ST_NOCHECK), LVIS_OVERLAYMASK);
		}else{
			ListView_SetItemState(GetDlgItem(hWnd, WWWC_LIST), i, 0, LVIS_OVERLAYMASK);
		}
		if(SelFlag == TRUE){
			ListView_SetItemState(GetDlgItem(hWnd, WWWC_LIST), i, 0, LVIS_SELECTED);
		}
		i++;
		cItem = TreeView_GetNextSibling(GetDlgItem(hWnd, WWWC_TREE), cItem);
	}

	//フォルダがアイテムの前に来るようにする
	ListView_SortItems(GetDlgItem(hWnd, WWWC_LIST), CompareFunc, DAMMYSORT);
	ListView_RedrawItems(GetDlgItem(hWnd, WWWC_LIST), 0, i);
}


/******************************************************************************

	ListView_RefreshFolder

	リストビューに表示しているフォルダの再設定を行う

******************************************************************************/

void ListView_RefreshFolder(HWND hWnd)
{
	int ListFolderCnt;
	int TreeFolderCnt;
	int i;
	BOOL SelFlag = TRUE;

	//リストビューに表示されているフォルダの数
	ListFolderCnt = ListView_GetFolderCount(GetDlgItem(hWnd, WWWC_LIST));
	//ツリービューに表示されているフォルダの数
	TreeFolderCnt = TreeView_GetChildCount(GetDlgItem(hWnd, WWWC_TREE), TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)));

	if(ListFolderCnt > TreeFolderCnt){
		//リストビューに表示されている数が多い場合は削除
		for(i = TreeFolderCnt;i < ListFolderCnt;i++){
			ListView_DeleteItem(GetDlgItem(hWnd, WWWC_LIST), TreeFolderCnt);
		}
	}
	if(ListFolderCnt < TreeFolderCnt){
		//ツリービューに表示されている数が多い場合は追加
		for(i = ListFolderCnt;i < TreeFolderCnt;i++){
			ListView_InsertItemEx(GetDlgItem(hWnd, WWWC_LIST), (char*)LPSTR_TEXTCALLBACK, I_IMAGECALLBACK,
				(long)NULL, ListFolderCnt);
		}
		SelFlag = FALSE;
	}
	ListView_FolderRedraw(hWnd, SelFlag);
}


/******************************************************************************

	ListView_RefreshItem

	リストビューに表示しているアイテムの再設定を行う

******************************************************************************/

void ListView_RefreshItem(HWND hListView)
{
	struct TPITEM *tpItemInfo;
	int ItemCnt, i;

	ListView_SetRedraw(hListView, LDF_NODRAW);
	ItemCnt = ListView_GetItemCount(hListView);
	for(i = ItemCnt - 1; i >= 0; i--){
		tpItemInfo = (struct TPITEM *)ListView_GetlParam(hListView, i);
		if(tpItemInfo == NULL){
			continue;
		}
		if(tpItemInfo->RefreshFlag == TRUE){
			tpItemInfo->RefreshFlag = FALSE;
			//表示更新
			ListView_SetItemText(hListView, i, 0, LPSTR_TEXTCALLBACK);
			ListView_SetItemState(hListView, i,
				((tpItemInfo->CheckSt == 1) ? INDEXTOOVERLAYMASK(ICON_ST_NOCHECK) : 0), LVIS_OVERLAYMASK);
			ListView_RedrawItems(hListView, i, i);
		}
	}
	ListView_SetRedraw(hListView, LDF_REDRAW);
	UpdateWindow(hListView);
}


/******************************************************************************

	ListView_ShowItem

	ListViewにアイテムを表示する

******************************************************************************/

void ListView_ShowItem(HWND hWnd, HTREEITEM hItem)
{
	struct TPTREE *tpTreeInfo;
	LV_ITEM lvi;
	HWND hListView;
	HWND hTreeView;
	HTREEITEM cItem;
	int ItemCnt, i, j;

	hTreeView = GetDlgItem(hWnd, WWWC_TREE);
	hListView = GetDlgItem(hWnd, WWWC_LIST);

	tpTreeInfo = (struct TPTREE *)TreeView_GetlParam(hTreeView, hItem);
	if(tpTreeInfo == NULL){
		ListView_DeleteAllItems(hListView);
		return;
	}

	ListView_SetRedraw(hListView, LDF_NODRAW);
	ListView_DeleteAllItems(hListView);

	//ListViewにメモリを確保させる
	cItem = TreeView_GetChild(hTreeView, hItem);
	j = 0;
	while(cItem != NULL){
		j++;
		cItem = TreeView_GetNextSibling(hTreeView, cItem);
	}
	ListView_SetItemCount(hListView, j + tpTreeInfo->ItemListCnt);

	//フォルダの表示
	cItem = TreeView_GetChild(hTreeView, hItem);
	j = 0;

	//追加アイテムの初期化
	lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE;
	lvi.iSubItem = 0;
	lvi.stateMask = LVIS_OVERLAYMASK;
	lvi.pszText = LPSTR_TEXTCALLBACK;
	lvi.cchTextMax = 0;
	lvi.iImage = I_IMAGECALLBACK;
	lvi.lParam = 0;

	while(cItem != NULL){
		// リストビューにアイテムを追加する
		lvi.iItem = j + 1;
		lvi.state = ((TreeView_GetState(hTreeView, cItem, TVIS_OVERLAYMASK) & INDEXTOOVERLAYMASK(ICON_ST_NOCHECK)) != 0)
			? INDEXTOOVERLAYMASK(ICON_ST_NOCHECK) : 0;
		j = ListView_InsertItem(hListView, &lvi);
		if(j == -1){
			break;
		}
		cItem = TreeView_GetNextSibling(hTreeView, cItem);
	}

	//アイテムの表示
	if(tpTreeInfo->ItemList == NULL){
		ListView_SetRedraw(hListView, LDF_REDRAW);
		return;
	}
	ItemCnt = tpTreeInfo->ItemListCnt;

	for(i = 0;i < ItemCnt;i++){
		if((*(tpTreeInfo->ItemList + i)) == NULL){
			continue;
		}
		(*(tpTreeInfo->ItemList + i))->RefreshFlag = FALSE;
		// リストビューにアイテムを追加する
		lvi.iItem = j + 1;
		lvi.state = ((*(tpTreeInfo->ItemList + i))->CheckSt == 1)
			? INDEXTOOVERLAYMASK(ICON_ST_NOCHECK) : 0;
		lvi.lParam = (long)*(tpTreeInfo->ItemList + i);
		j = ListView_InsertItem(hListView, &lvi);
		if(j == -1){
			break;
		}
	}

	if(ABS(LvStyle) == LVS_LIST){
		//List表示時アイテムの間隔がうまく行かないので一時的に他のスタイルに変更
		ListView_SetStyle(hWnd, LVS_SMALLICON);
		ListView_SetStyle(hWnd, LVS_LIST);
	}

	if(LvAutoSort == 1){
		//アイテムをソート
		ListView_SetRedraw(hListView, LDF_NODRAW);
		SortColInfo = ColumnInfo;
		ListView_SortItems(hListView, CompareFunc, LvSortFlag);
		ListView_SortSelect(hListView, LvSortFlag);

		GlobalFree(tpTreeInfo->ItemList);
		tpTreeInfo->ItemListCnt = 0;
		tpTreeInfo->ItemList = ListView_SetListToMem(hWnd, &tpTreeInfo->ItemListCnt);
	}
	ListView_EnsureVisible(hListView, 0, TRUE);
	SendMessage(hListView, WM_HSCROLL, SB_TOP, 0);
	ListView_SetRedraw(hListView, LDF_REDRAW);
}


/******************************************************************************

	ListView_StartDragItem

	選択アイテムのドラッグ＆ドロップを開始する

******************************************************************************/

void ListView_StartDragItem(HWND hWnd)
{
	struct TPITEM *tpItemInfo;
	HTREEITEM hItem;
	UINT cf[CLIPFORMAT_CNT];
	int Effect;
	int SelectItem;
	int ret;
	int cfcnt;
	BOOL NoFileFlag;

	DgdpItem = NULL;
	WWWCDropFlag = FALSE;

	//Altキーが押されている場合はファイルを扱わない
	NoFileFlag = (GetAsyncKeyState(VK_MENU) < 0) ? TRUE : FALSE;

	//フォルダ情報の保存
	SaveTreeMem(hWnd, GetDlgItem(hWnd, WWWC_TREE), TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)));

	if(NoFileFlag == FALSE){
		//ListViewの選択されているアイテムからドロップファイルを作成
		if(DragDrop_CreateDropFiles(hWnd, DnD_DirName) == FALSE){
			return;
		}
	}

	//効果を設定
	Effect = DROPEFFECT_COPY | DROPEFFECT_MOVE | DROPEFFECT_LINK;

	SelectItem = -1;
	while((SelectItem = ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), SelectItem, LVNI_SELECTED)) != -1){
		tpItemInfo = (struct TPITEM *)ListView_GetlParam(GetDlgItem(hWnd, WWWC_LIST), SelectItem);
		if(tpItemInfo == NULL){
			hItem = TreeView_GetIndexToItem(GetDlgItem(hWnd, WWWC_TREE),
				TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)), SelectItem);
			//チェック中の場合は移動の効果を外してループを抜ける
			if(FindCheckItem(hWnd, hItem) == 1){
				Effect ^= DROPEFFECT_MOVE;
				break;
			}
			//プロパティ表示中の場合は移動の効果を外してループを抜ける
			if(FindPropItem(hWnd, hItem) == 1){
				Effect ^= DROPEFFECT_MOVE;
				break;
			}
		}else{
			//チェック中の場合は移動の効果を外してループを抜ける
			if(tpItemInfo->IconStatus == ST_CHECK){
				Effect ^= DROPEFFECT_MOVE;
				break;
			}
		}
	}

	//ドラッグ＆ドロップするフォルダの先頭のインデックスを取得
	//これはフラグ的なもので同一フォルダのチェックに利用する
	DragItemIndex = ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), -1, LVNI_SELECTED);
	tpItemInfo = (struct TPITEM *)ListView_GetlParam(GetDlgItem(hWnd, WWWC_LIST), DragItemIndex);
	if(tpItemInfo != NULL){
		DragItemIndex = -1;
	}

	//クリップボードフォーマットを設定
	cf[0] = WWWC_ClipFormat;
	cf[1] = CF_TEXT;
	cf[2] = CF_HDROP;
	cfcnt = (NoFileFlag == FALSE) ? CLIPFORMAT_CNT : CLIPFORMAT_CNT - 1;

	//ドラッグ＆ドロップを開始
	ret = OLE_IDropSource_Start(hWnd, WM_DATAOBJECT_GETDATA, cf, cfcnt, Effect);
	DragItemIndex = -1;

	WaitCursor(TRUE);
	//ドロップファイル情報を解放
	if(NoFileFlag == FALSE){
		FreeDropFiles();
	}

	if(ret == DROPEFFECT_MOVE && WWWCDropFlag == TRUE){
		//削除されたフォルダ、アイテムをリストビューから削除する
		TreeView_NoFileDelete(hWnd, TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)), 0);
		ListView_RefreshFolder(hWnd);
		Item_Select(hWnd, TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)));
	}

	//WWWC以外のアプリケーションにドロップした場合はアイコンを初期化
	if(ret != -1 && ret != DROPEFFECT_NONE && WWWCDropFlag == FALSE){
		switch(DnDReturnIcon)
		{
		case 0:		//アイコンを初期化しない
			break;

		case 1:		//ドロップしたアイテムが1件の場合
			if(ListView_GetSelectedCount(GetDlgItem(hWnd, WWWC_LIST)) != 1){
				break;
			}
			if((SelectItem = ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), -1, LVNI_SELECTED)) == -1){
				break;
			}
			Item_Initialize(hWnd, (struct TPITEM *)ListView_GetlParam(GetDlgItem(hWnd, WWWC_LIST), SelectItem), FALSE);
			ListView_RedrawItems(GetDlgItem(hWnd, WWWC_LIST), SelectItem, SelectItem);
			break;

		case 2:		//アイコンを初期化する
			SelectItem = -1;
			while((SelectItem = ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), SelectItem, LVNI_SELECTED)) != -1){
				Item_Initialize(hWnd, (struct TPITEM *)ListView_GetlParam(GetDlgItem(hWnd, WWWC_LIST), SelectItem), FALSE);
				ListView_RedrawItems(GetDlgItem(hWnd, WWWC_LIST), SelectItem, SelectItem);
			}
			break;
		}
	}
	TreeView_SetIconState(hWnd, TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)), 0);
	CallTreeItem(hWnd, TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)), (FARPROC)TreeView_FreeItem, 1);

	WaitCursor(FALSE);
}


/******************************************************************************

	ListView_MouseSelectItem

	マウスの下にアイテムが選択されているかチェックする

******************************************************************************/

BOOL ListView_MouseSelectItem(HWND hListView)
{
	LV_HITTESTINFO lvhti;
	POINT apos;
	RECT ListViewRect;
	int hittestItem, SelectItem;

	GetCursorPos(&apos);
	GetWindowRect(hListView, &ListViewRect);
	apos.x -= ListViewRect.left;
	apos.y -= ListViewRect.top;

	lvhti.pt = apos;
	lvhti.flags = LVHT_ONITEMICON | LVHT_ONITEMLABEL | LVHT_ONITEMSTATEICON;
	lvhti.iItem = 0;

	hittestItem = ListView_HitTest(hListView, &lvhti);
	if(hittestItem == -1){
		return FALSE;
	}

	if((SelectItem = ListView_GetNextItem(hListView, -1, LVNI_SELECTED)) == -1){
		return FALSE;
	}
	if(hittestItem == SelectItem){
		return TRUE;
	}
	return FALSE;
}


/******************************************************************************

	ListView_ItemClick

	アイテムを開く処理を行う

******************************************************************************/

void ListView_ItemClick(HWND hWnd, NMHDR *CForm)
{
	int i;

	//プロトコル毎のメニューを設定
	SetProtocolMenu(hWnd);

	//アイテムが選択されていない場合は一つ上の階層へ移動
	if(LvDblclkUpDir == 1 && CForm->code == NM_DBLCLK &&
		(ListView_GetSelectedCount(GetDlgItem(hWnd, WWWC_LIST)) == 0)){
		SendMessage(hWnd, WM_COMMAND, ID_KEY_UPDIR, 0);
		return;
	}

	//マウスをアイコンの上に持っていくと選択状態になるスタイルの場合は、クリックで実行
	if(CForm->code == NM_CLICK &&
		((SendDlgItemMessage(hWnd, WWWC_LIST, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0) & LVS_EX_TRACKSELECT) == 0 ||
		ListView_GetSelectedCount(GetDlgItem(hWnd, WWWC_LIST)) != 1 ||
		ListView_MouseSelectItem(GetDlgItem(hWnd, WWWC_LIST)) == FALSE)){
		return;
	}
	//標準のスタイルの場合はダブルクリックで実行
	if(CForm->code == NM_DBLCLK &&
		(SendDlgItemMessage(hWnd, WWWC_LIST, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0) & LVS_EX_TRACKSELECT) != 0){
		return;
	}

	// [Alt]キーが押されている場合は、プロパティを表示
	if(GetAsyncKeyState(VK_MENU) < 0){
		SendMessage(hWnd, WM_COMMAND, ID_MENUITEM_PROP, 0);
		return;
	}

	//デフォルトの項目を取得
	if((i = GetMenuDefaultItem(GetSubMenu(hPOPUP, MENU_POP_ITEM), 0, 0)) == -1){
		return;
	}
	//ウィンドウに実行するためのメッセージを送信
	SendMessage(hWnd, WM_COMMAND, i, 0);
}


/******************************************************************************

	ListView_MenuSort

	メニューからのアイテムのソート

******************************************************************************/

int ListView_MenuSort(HWND hWnd, int i)
{
	struct TPTREE *tpTreeInfo;

	WaitCursor(TRUE);

	LvSortFlag = (ABS(LvSortFlag) == (i + 1)) ? (LvSortFlag * -1) : (i + 1);
	SortColInfo = ColumnInfo;
	ListView_SortItems(GetDlgItem(hWnd, WWWC_LIST), CompareFunc, LvSortFlag);
	ListView_SortSelect(GetDlgItem(hWnd, WWWC_LIST), LvSortFlag);

	tpTreeInfo = (struct TPTREE *)TreeView_GetlParam(GetDlgItem(hWnd, WWWC_TREE),
		TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)));
	if(tpTreeInfo != NULL){
		GlobalFree(tpTreeInfo->ItemList);
		tpTreeInfo->ItemListCnt = 0;
		tpTreeInfo->ItemList = ListView_SetListToMem(hWnd, &tpTreeInfo->ItemListCnt);
	}

	WaitCursor(FALSE);
	return 0;
}


/******************************************************************************

	ListView_SortClear

	ソート用選択の解除

******************************************************************************/

void ListView_SortClear(const HWND hListView, const int sort_flag)
{
	HDITEM hdi = { 0 };
	int order, colum;

	order = (sort_flag < 0) ? 1 : 0;	//昇順／降順
	colum = ABS(sort_flag) - 1;		//ソートを行うヘッダ

	ListView_SetSelectedColumn(hListView, -1);

	hdi.mask = HDI_FORMAT;
	Header_GetItem(ListView_GetHeader(hListView), colum, &hdi);
	hdi.fmt &= hdi.fmt ^ (HDF_SORTUP | HDF_SORTDOWN);
	Header_SetItem(ListView_GetHeader(hListView), colum, &hdi);
}

/******************************************************************************

	ListView_SortSelect

	ソート用選択

******************************************************************************/

void ListView_SortSelect(const HWND hListView, const int sort_flag)
{
	HDITEM hdi = { 0 };
	int order, colum;

	if(LvAutoSort != 1){
		ListView_SortClear(hListView, sort_flag);
		return;
	}
	order = (sort_flag < 0) ? 1 : 0;	//昇順／降順
	colum = ABS(sort_flag) - 1;		//ソートを行うヘッダ

	ListView_SetSelectedColumn(hListView, colum);

	hdi.mask = HDI_FORMAT;
	Header_GetItem(ListView_GetHeader(hListView), colum, &hdi);
	hdi.fmt &= hdi.fmt ^ (HDF_SORTUP | HDF_SORTDOWN);
	hdi.fmt |= (order == 0) ? HDF_SORTUP : HDF_SORTDOWN;
	Header_SetItem(ListView_GetHeader(hListView), colum, &hdi);
}


/******************************************************************************

	ListView_NotifyProc

	リストビューイベント

******************************************************************************/

LRESULT ListView_NotifyProc(HWND hWnd, LPARAM lParam)
{
	NM_LISTVIEW *lv = (NM_LISTVIEW *)lParam;
	LV_DISPINFO *plv = (LV_DISPINFO *)lParam;
	LV_KEYDOWN *LKey = (LV_KEYDOWN *)lParam;
	NMHDR *CForm = (NMHDR *)lParam;

	switch(plv->hdr.code)
	{
	case LVN_ITEMCHANGED:		//アイテムの選択状態の変更
	case LVN_BEGINLABELEDIT:	//タイトル編集開始
	case LVN_ENDLABELEDIT:		//タイトル編集終了
	case LVN_GETDISPINFO:		//表示アイテムの要求
		return SendMessage(hWnd, WM_LV_EVENT, plv->hdr.code, lParam);
	}

	switch(lv->hdr.code)
	{
	case LVN_BEGINDRAG:			//ドラッグの開始 (マウスの左ボタン)
	case LVN_BEGINRDRAG:		//ドラッグの開始 (マウスの右ボタン)
		return SendMessage(hWnd, WM_LV_EVENT, lv->hdr.code, lParam);
	}

	switch(LKey->hdr.code)
	{
	case LVN_KEYDOWN:			//キーダウン
		return SendMessage(hWnd, WM_LV_EVENT, LKey->hdr.code, lParam);
	}

	switch(CForm->code)
	{
	case NM_SETFOCUS:			//フォーカスの変更
	case NM_CLICK:				//クリック
	case NM_DBLCLK:				//ダブルクリック
	case NM_RCLICK:				//右クリック
		return SendMessage(hWnd, WM_LV_EVENT, CForm->code, lParam);
	}
	return FALSE;
}
/* End of source */