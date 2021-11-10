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

#define DEFAULT_COLSIZE			100					//�f�t�H���g�̃J�����T�C�Y

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

struct TPLVCOLUMN *SortColInfo;	//�\�[�g���
struct TPLVCOLUMN *ColumnInfo = NULL;

int DragItemIndex = -1;

//�O���Q��
extern HINSTANCE g_hinst;				//�A�v���P�[�V�����̃C���X�^���X�n���h��
extern char CuDir[];
extern HWND WWWCWnd;					//�{��
extern HWND TbWnd;						//�c�[���o�[
extern HMENU hPOPUP;					//�|�b�v�A�b�v���j���[
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

	���X�g�r���[�̍쐬

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

	���X�g�r���[�̃C���[�W���X�g�̐ݒ�

******************************************************************************/

void ListView_SetItemImage(HWND hListView, int IconSize, int LVFLag)
{
	HIMAGELIST IconList;
	HIMAGELIST TmpIconList;
	HICON TmpIcon;

	//�C���[�W���X�g�̍쐬
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

	//���ʂ̃A�C�R��
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

	//���X�g�r���[�̃A�C�R��
	ImageListIconAdd(IconList, IDI_ICON_UP, IconSize, Up, UpIndex);
	ImageListIconAdd(IconList, IDI_ICON_ERROR, IconSize, Error, ErrorIndex);
	ImageListIconAdd(IconList, IDI_ICON_TIMEOUT, IconSize, TimeOut, TimeOutIndex);
	ImageListIconAdd(IconList, IDI_ICON_NO, IconSize, NoProtocol, NoProtocolIndex);
	ImageListIconAdd(IconList, IDI_ICON_WAIT, IconSize, Wait, WaitIndex);

	//���X�g�r���[�ɃC���[�W���X�g��ݒ�
	ListView_SetImageList(hListView, IconList, LVFLag);
}


/******************************************************************************

	SetColmunInfo

	�J�������̐ݒ�

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

	�J�����̒ǉ�

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

	�\�[�g�t���O�ɏ]���ĕ�����̔�r���s��

******************************************************************************/

int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	char buf1[BUFSIZE], buf2[BUFSIZE];
	char tmp1[BUFSIZE], tmp2[BUFSIZE];
	int ret, sfg, ghed;

	if(lParamSort == DAMMYSORT){
		return 0;
	}

	sfg = (lParamSort < 0) ? 1 : 0;	//�����^�~��
	ghed = ABS(lParamSort) - 1;		//�\�[�g���s���w�b�_

	*buf1 = '\0';
	*buf2 = '\0';

	if(lParam1 == 0 || lParam2 == 0){
		return 0;
	}

	if(ghed == 100){
		//�A�C�R�����Ƀ\�[�g
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
		//�A�C�e�����i�[����Ă���t�H���_�̃p�X
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
		//��r���s��������
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

		//���l�̏ꍇ
		if((SortColInfo + ghed)->fmt == LVCFMT_RIGHT){
			GetNumString(buf1, tmp1);
			GetNumString(buf2, tmp2);

			wsprintf(buf1, "%20ld", atol(tmp1));
			wsprintf(buf2, "%20ld", atol(tmp2));
		}
	}

	//�������ɕϊ����Ĕ�r���s��
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

	���X�g�r���[�̕`��؂�ւ�

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

	�w��ʒu�ɃA�C�e����}��

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

	// ���X�g�r���[�ɃA�C�e����ǉ�����
	return ListView_InsertItem(hListView, &lvi);
}


/******************************************************************************

	ListView_SetItemTitle

	�A�C�e���̃^�C�g����ݒ肷��

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
		//Undo�o�b�t�@�ɃZ�b�g
		SetLVTitleToUndo(TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)), tpItemInfo->Title, plv->item.pszText, tpItemInfo->CheckURL);
		//���X�g�r���[�̃A�C�e���̃^�C�g����ݒ肷��
		GlobalFree(tpItemInfo->Title);
		tpItemInfo->Title = (char *)GlobalAlloc(GMEM_FIXED, lstrlen(plv->item.pszText) + 1);
		if(tpItemInfo->Title == NULL){
			ErrMsg(hWnd, GetLastError(), NULL);
			return;
		}
		lstrcpy(tpItemInfo->Title, plv->item.pszText);
		//�A�C�e���̍Đݒ�
		ListView_SetItemText(GetDlgItem(hWnd, WWWC_LIST), plv->item.iItem, 0, LPSTR_TEXTCALLBACK);
	}else{
		//�Ή�����c���[�r���[�A�C�e���̃n���h���̎擾
		hItem = TreeView_GetIndexToItem(GetDlgItem(hWnd, WWWC_TREE),
			TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)), plv->item.iItem);
		if(hItem == NULL){
			return;
		}
		//�c���[�r���[�A�C�e���̃^�C�g����ύX����
		TreeView_SetName(hWnd, hItem, plv->item.pszText);

		ListView_RefreshFolder(hWnd);

		//���O�̕ύX���ꂽ�A�C�e����I������
		if((i = TreeView_GetListIndex(GetDlgItem(hWnd, WWWC_TREE), hItem)) != -1){
			ListView_SetItemState(GetDlgItem(hWnd, WWWC_LIST),
				i, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
			ListView_EnsureVisible(GetDlgItem(hWnd, WWWC_LIST), i, TRUE);
		}
	}
}


/******************************************************************************

	ListView_SetItemIcon

	���X�g�r���[�̃A�C�R����ݒ�

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

	���X�g�r���[�̃A�C�e�����ړ�

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

	���X�g�r���[�ɕ\������Ă���t�H���_�̐����擾

******************************************************************************/

int ListView_GetFolderCount(HWND hListView)
{
	struct TPITEM *tpItemInfo;
	int cnt;
	int ret = 0;

	//���X�g�r���[�ɕ\������Ă���t�H���_�̐�
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

	�A�C�e����񂩂�A�C�e���̃C���f�b�N�X���擾

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

	�A�C�e����LPARAM���擾

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

	�A�C�e����LPARAM��ݒ�

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

	���X�g�r���[�̃A�C�e������A�C�e�������쐬

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

	���X�g�r���[�̑I���A�C�e������A�C�e�������쐬

******************************************************************************/

struct TPITEM **ListView_SelectItemToMem(HWND hListView, int *cnt)
{
	struct TPITEM **RetItemList;
	struct TPITEM *tpItemInfo;
	int i;

	//�I���A�C�e�����̎擾
	i = -1;
	*cnt = 0;
	while((i = ListView_GetNextItem(hListView, i, LVNI_SELECTED)) != -1){
		tpItemInfo = (struct TPITEM *)ListView_GetlParam(hListView, i);
		if(tpItemInfo == NULL){
			continue;
		}
		(*cnt)++;
	}
	//�A�C�e�����X�g�̊m��
	RetItemList = (struct TPITEM **)GlobalAlloc(GPTR, sizeof(struct TPITEM *) * *cnt);
	if(RetItemList == NULL){
		return NULL;
	}
	//�A�C�e�����X�g�ɑI���A�C�e����ݒ�
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

	�}�E�X�̉��̃A�C�e���̃C���f�b�N�X���擾

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

	�A�C�e���̑I���̐؂�ւ�

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

	UP�A�C�R���̑I��

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

	���ݔ����I������Ă��Ȃ����`�F�b�N����

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

	���X�g�r���[�X�^�C�����j���[�̃`�F�b�N����������

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

	���X�g�r���[�̃X�^�C���ɂ��������ăE�B���h�E�̃��j���[���ڂ�ύX����

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
	case LVS_ICON:				//�A�C�R��
		SetListMenuUnCheck(hWnd, ID_MENUITEM_VIEW_ICON, 1);
		break;

	case LVS_SMALLICON:			//�������A�C�R��
		SetListMenuUnCheck(hWnd, ID_MENUITEM_VIEW_SMALLICON, 1);
		break;

	case LVS_LIST:				//���X�g
		SetListMenuUnCheck(hWnd, ID_MENUITEM_VIEW_LIST, 1);
		break;

	case LVS_REPORT:			//�ڍ�
		SetListMenuUnCheck(hWnd, ID_MENUITEM_VIEW_REPORT, 1);

		InvalidateRect(GetDlgItem(hWnd, WWWC_LIST), NULL, TRUE);
		UpdateWindow(GetDlgItem(hWnd, WWWC_LIST));
		break;

	default:					//�ڍׂłP�s�I��
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

	���X�g�r���[�̃X�^�C����ύX����

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

	���X�g�r���[�ɕ\������A�C�e�����̐ݒ�

******************************************************************************/

BOOL ListView_GetDispItem(HWND hWnd, LV_ITEM *hLVItem)
{
	struct TPTREE *tpTreeInfo;
	struct TPITEM *tpItemInfo;
	char buf[BUFSIZE];
	HTREEITEM hItem;
	int i;

	tpItemInfo = (struct TPITEM *)ListView_GetlParam(GetDlgItem(hWnd, WWWC_LIST), hLVItem->iItem);
	//�t�H���_
	if(tpItemInfo == NULL){
		if((hItem = TreeView_GetIndexToItem(GetDlgItem(hWnd, WWWC_TREE),
			TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)), hLVItem->iItem)) == NULL){
			return FALSE;
		}

		i = TreeView_GetItemInfo(GetDlgItem(hWnd, WWWC_TREE), hItem, buf);
		//�e�L�X�g
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
		//�A�C�R��
		if(hLVItem->mask & LVIF_IMAGE){
			hLVItem->iImage = i;
		}
		return FALSE;
	}

	//�A�C�e��
	//�e�L�X�g
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

	//�A�C�R��
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

	���X�g�r���[�ɕ\�����Ă���t�H���_�̍ĕ\�����s��

******************************************************************************/

void ListView_FolderRedraw(HWND hWnd, BOOL SelFlag)
{
	HTREEITEM cItem;
	int i;

	//�A�C�e���̕\����Ԃ�ݒ�
	cItem = TreeView_GetChild(GetDlgItem(hWnd, WWWC_TREE), TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)));
	i = 0;
	while(cItem != NULL){
		ListView_SetItemText(GetDlgItem(hWnd, WWWC_LIST), i, 0, LPSTR_TEXTCALLBACK);
		//�`�F�b�N����^���Ȃ��A�C�R���̕t��
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

	//�t�H���_���A�C�e���̑O�ɗ���悤�ɂ���
	ListView_SortItems(GetDlgItem(hWnd, WWWC_LIST), CompareFunc, DAMMYSORT);
	ListView_RedrawItems(GetDlgItem(hWnd, WWWC_LIST), 0, i);
}


/******************************************************************************

	ListView_RefreshFolder

	���X�g�r���[�ɕ\�����Ă���t�H���_�̍Đݒ���s��

******************************************************************************/

void ListView_RefreshFolder(HWND hWnd)
{
	int ListFolderCnt;
	int TreeFolderCnt;
	int i;
	BOOL SelFlag = TRUE;

	//���X�g�r���[�ɕ\������Ă���t�H���_�̐�
	ListFolderCnt = ListView_GetFolderCount(GetDlgItem(hWnd, WWWC_LIST));
	//�c���[�r���[�ɕ\������Ă���t�H���_�̐�
	TreeFolderCnt = TreeView_GetChildCount(GetDlgItem(hWnd, WWWC_TREE), TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)));

	if(ListFolderCnt > TreeFolderCnt){
		//���X�g�r���[�ɕ\������Ă��鐔�������ꍇ�͍폜
		for(i = TreeFolderCnt;i < ListFolderCnt;i++){
			ListView_DeleteItem(GetDlgItem(hWnd, WWWC_LIST), TreeFolderCnt);
		}
	}
	if(ListFolderCnt < TreeFolderCnt){
		//�c���[�r���[�ɕ\������Ă��鐔�������ꍇ�͒ǉ�
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

	���X�g�r���[�ɕ\�����Ă���A�C�e���̍Đݒ���s��

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
			//�\���X�V
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

	ListView�ɃA�C�e����\������

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

	//ListView�Ƀ��������m�ۂ�����
	cItem = TreeView_GetChild(hTreeView, hItem);
	j = 0;
	while(cItem != NULL){
		j++;
		cItem = TreeView_GetNextSibling(hTreeView, cItem);
	}
	ListView_SetItemCount(hListView, j + tpTreeInfo->ItemListCnt);

	//�t�H���_�̕\��
	cItem = TreeView_GetChild(hTreeView, hItem);
	j = 0;

	//�ǉ��A�C�e���̏�����
	lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE;
	lvi.iSubItem = 0;
	lvi.stateMask = LVIS_OVERLAYMASK;
	lvi.pszText = LPSTR_TEXTCALLBACK;
	lvi.cchTextMax = 0;
	lvi.iImage = I_IMAGECALLBACK;
	lvi.lParam = 0;

	while(cItem != NULL){
		// ���X�g�r���[�ɃA�C�e����ǉ�����
		lvi.iItem = j + 1;
		lvi.state = ((TreeView_GetState(hTreeView, cItem, TVIS_OVERLAYMASK) & INDEXTOOVERLAYMASK(ICON_ST_NOCHECK)) != 0)
			? INDEXTOOVERLAYMASK(ICON_ST_NOCHECK) : 0;
		j = ListView_InsertItem(hListView, &lvi);
		if(j == -1){
			break;
		}
		cItem = TreeView_GetNextSibling(hTreeView, cItem);
	}

	//�A�C�e���̕\��
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
		// ���X�g�r���[�ɃA�C�e����ǉ�����
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
		//List�\�����A�C�e���̊Ԋu�����܂��s���Ȃ��̂ňꎞ�I�ɑ��̃X�^�C���ɕύX
		ListView_SetStyle(hWnd, LVS_SMALLICON);
		ListView_SetStyle(hWnd, LVS_LIST);
	}

	if(LvAutoSort == 1){
		//�A�C�e�����\�[�g
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

	�I���A�C�e���̃h���b�O���h���b�v���J�n����

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

	//Alt�L�[��������Ă���ꍇ�̓t�@�C��������Ȃ�
	NoFileFlag = (GetAsyncKeyState(VK_MENU) < 0) ? TRUE : FALSE;

	//�t�H���_���̕ۑ�
	SaveTreeMem(hWnd, GetDlgItem(hWnd, WWWC_TREE), TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)));

	if(NoFileFlag == FALSE){
		//ListView�̑I������Ă���A�C�e������h���b�v�t�@�C�����쐬
		if(DragDrop_CreateDropFiles(hWnd, DnD_DirName) == FALSE){
			return;
		}
	}

	//���ʂ�ݒ�
	Effect = DROPEFFECT_COPY | DROPEFFECT_MOVE | DROPEFFECT_LINK;

	SelectItem = -1;
	while((SelectItem = ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), SelectItem, LVNI_SELECTED)) != -1){
		tpItemInfo = (struct TPITEM *)ListView_GetlParam(GetDlgItem(hWnd, WWWC_LIST), SelectItem);
		if(tpItemInfo == NULL){
			hItem = TreeView_GetIndexToItem(GetDlgItem(hWnd, WWWC_TREE),
				TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)), SelectItem);
			//�`�F�b�N���̏ꍇ�͈ړ��̌��ʂ��O���ă��[�v�𔲂���
			if(FindCheckItem(hWnd, hItem) == 1){
				Effect ^= DROPEFFECT_MOVE;
				break;
			}
			//�v���p�e�B�\�����̏ꍇ�͈ړ��̌��ʂ��O���ă��[�v�𔲂���
			if(FindPropItem(hWnd, hItem) == 1){
				Effect ^= DROPEFFECT_MOVE;
				break;
			}
		}else{
			//�`�F�b�N���̏ꍇ�͈ړ��̌��ʂ��O���ă��[�v�𔲂���
			if(tpItemInfo->IconStatus == ST_CHECK){
				Effect ^= DROPEFFECT_MOVE;
				break;
			}
		}
	}

	//�h���b�O���h���b�v����t�H���_�̐擪�̃C���f�b�N�X���擾
	//����̓t���O�I�Ȃ��̂œ���t�H���_�̃`�F�b�N�ɗ��p����
	DragItemIndex = ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), -1, LVNI_SELECTED);
	tpItemInfo = (struct TPITEM *)ListView_GetlParam(GetDlgItem(hWnd, WWWC_LIST), DragItemIndex);
	if(tpItemInfo != NULL){
		DragItemIndex = -1;
	}

	//�N���b�v�{�[�h�t�H�[�}�b�g��ݒ�
	cf[0] = WWWC_ClipFormat;
	cf[1] = CF_TEXT;
	cf[2] = CF_HDROP;
	cfcnt = (NoFileFlag == FALSE) ? CLIPFORMAT_CNT : CLIPFORMAT_CNT - 1;

	//�h���b�O���h���b�v���J�n
	ret = OLE_IDropSource_Start(hWnd, WM_DATAOBJECT_GETDATA, cf, cfcnt, Effect);
	DragItemIndex = -1;

	WaitCursor(TRUE);
	//�h���b�v�t�@�C���������
	if(NoFileFlag == FALSE){
		FreeDropFiles();
	}

	if(ret == DROPEFFECT_MOVE && WWWCDropFlag == TRUE){
		//�폜���ꂽ�t�H���_�A�A�C�e�������X�g�r���[����폜����
		TreeView_NoFileDelete(hWnd, TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)), 0);
		ListView_RefreshFolder(hWnd);
		Item_Select(hWnd, TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)));
	}

	//WWWC�ȊO�̃A�v���P�[�V�����Ƀh���b�v�����ꍇ�̓A�C�R����������
	if(ret != -1 && ret != DROPEFFECT_NONE && WWWCDropFlag == FALSE){
		switch(DnDReturnIcon)
		{
		case 0:		//�A�C�R�������������Ȃ�
			break;

		case 1:		//�h���b�v�����A�C�e����1���̏ꍇ
			if(ListView_GetSelectedCount(GetDlgItem(hWnd, WWWC_LIST)) != 1){
				break;
			}
			if((SelectItem = ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), -1, LVNI_SELECTED)) == -1){
				break;
			}
			Item_Initialize(hWnd, (struct TPITEM *)ListView_GetlParam(GetDlgItem(hWnd, WWWC_LIST), SelectItem), FALSE);
			ListView_RedrawItems(GetDlgItem(hWnd, WWWC_LIST), SelectItem, SelectItem);
			break;

		case 2:		//�A�C�R��������������
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

	�}�E�X�̉��ɃA�C�e�����I������Ă��邩�`�F�b�N����

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

	�A�C�e�����J���������s��

******************************************************************************/

void ListView_ItemClick(HWND hWnd, NMHDR *CForm)
{
	int i;

	//�v���g�R�����̃��j���[��ݒ�
	SetProtocolMenu(hWnd);

	//�A�C�e�����I������Ă��Ȃ��ꍇ�͈��̊K�w�ֈړ�
	if(LvDblclkUpDir == 1 && CForm->code == NM_DBLCLK &&
		(ListView_GetSelectedCount(GetDlgItem(hWnd, WWWC_LIST)) == 0)){
		SendMessage(hWnd, WM_COMMAND, ID_KEY_UPDIR, 0);
		return;
	}

	//�}�E�X���A�C�R���̏�Ɏ����Ă����ƑI����ԂɂȂ�X�^�C���̏ꍇ�́A�N���b�N�Ŏ��s
	if(CForm->code == NM_CLICK &&
		((SendDlgItemMessage(hWnd, WWWC_LIST, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0) & LVS_EX_TRACKSELECT) == 0 ||
		ListView_GetSelectedCount(GetDlgItem(hWnd, WWWC_LIST)) != 1 ||
		ListView_MouseSelectItem(GetDlgItem(hWnd, WWWC_LIST)) == FALSE)){
		return;
	}
	//�W���̃X�^�C���̏ꍇ�̓_�u���N���b�N�Ŏ��s
	if(CForm->code == NM_DBLCLK &&
		(SendDlgItemMessage(hWnd, WWWC_LIST, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0) & LVS_EX_TRACKSELECT) != 0){
		return;
	}

	// [Alt]�L�[��������Ă���ꍇ�́A�v���p�e�B��\��
	if(GetAsyncKeyState(VK_MENU) < 0){
		SendMessage(hWnd, WM_COMMAND, ID_MENUITEM_PROP, 0);
		return;
	}

	//�f�t�H���g�̍��ڂ��擾
	if((i = GetMenuDefaultItem(GetSubMenu(hPOPUP, MENU_POP_ITEM), 0, 0)) == -1){
		return;
	}
	//�E�B���h�E�Ɏ��s���邽�߂̃��b�Z�[�W�𑗐M
	SendMessage(hWnd, WM_COMMAND, i, 0);
}


/******************************************************************************

	ListView_MenuSort

	���j���[����̃A�C�e���̃\�[�g

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

	�\�[�g�p�I���̉���

******************************************************************************/

void ListView_SortClear(const HWND hListView, const int sort_flag)
{
	HDITEM hdi = { 0 };
	int order, colum;

	order = (sort_flag < 0) ? 1 : 0;	//�����^�~��
	colum = ABS(sort_flag) - 1;		//�\�[�g���s���w�b�_

	ListView_SetSelectedColumn(hListView, -1);

	hdi.mask = HDI_FORMAT;
	Header_GetItem(ListView_GetHeader(hListView), colum, &hdi);
	hdi.fmt &= hdi.fmt ^ (HDF_SORTUP | HDF_SORTDOWN);
	Header_SetItem(ListView_GetHeader(hListView), colum, &hdi);
}

/******************************************************************************

	ListView_SortSelect

	�\�[�g�p�I��

******************************************************************************/

void ListView_SortSelect(const HWND hListView, const int sort_flag)
{
	HDITEM hdi = { 0 };
	int order, colum;

	if(LvAutoSort != 1){
		ListView_SortClear(hListView, sort_flag);
		return;
	}
	order = (sort_flag < 0) ? 1 : 0;	//�����^�~��
	colum = ABS(sort_flag) - 1;		//�\�[�g���s���w�b�_

	ListView_SetSelectedColumn(hListView, colum);

	hdi.mask = HDI_FORMAT;
	Header_GetItem(ListView_GetHeader(hListView), colum, &hdi);
	hdi.fmt &= hdi.fmt ^ (HDF_SORTUP | HDF_SORTDOWN);
	hdi.fmt |= (order == 0) ? HDF_SORTUP : HDF_SORTDOWN;
	Header_SetItem(ListView_GetHeader(hListView), colum, &hdi);
}


/******************************************************************************

	ListView_NotifyProc

	���X�g�r���[�C�x���g

******************************************************************************/

LRESULT ListView_NotifyProc(HWND hWnd, LPARAM lParam)
{
	NM_LISTVIEW *lv = (NM_LISTVIEW *)lParam;
	LV_DISPINFO *plv = (LV_DISPINFO *)lParam;
	LV_KEYDOWN *LKey = (LV_KEYDOWN *)lParam;
	NMHDR *CForm = (NMHDR *)lParam;

	switch(plv->hdr.code)
	{
	case LVN_ITEMCHANGED:		//�A�C�e���̑I����Ԃ̕ύX
	case LVN_BEGINLABELEDIT:	//�^�C�g���ҏW�J�n
	case LVN_ENDLABELEDIT:		//�^�C�g���ҏW�I��
	case LVN_GETDISPINFO:		//�\���A�C�e���̗v��
		return SendMessage(hWnd, WM_LV_EVENT, plv->hdr.code, lParam);
	}

	switch(lv->hdr.code)
	{
	case LVN_BEGINDRAG:			//�h���b�O�̊J�n (�}�E�X�̍��{�^��)
	case LVN_BEGINRDRAG:		//�h���b�O�̊J�n (�}�E�X�̉E�{�^��)
		return SendMessage(hWnd, WM_LV_EVENT, lv->hdr.code, lParam);
	}

	switch(LKey->hdr.code)
	{
	case LVN_KEYDOWN:			//�L�[�_�E��
		return SendMessage(hWnd, WM_LV_EVENT, LKey->hdr.code, lParam);
	}

	switch(CForm->code)
	{
	case NM_SETFOCUS:			//�t�H�[�J�X�̕ύX
	case NM_CLICK:				//�N���b�N
	case NM_DBLCLK:				//�_�u���N���b�N
	case NM_RCLICK:				//�E�N���b�N
		return SendMessage(hWnd, WM_LV_EVENT, CForm->code, lParam);
	}
	return FALSE;
}
/* End of source */