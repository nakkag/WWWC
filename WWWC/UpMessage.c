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

#define DEF_RIGHT						120		//�f�t�H���g�̈ʒu
#define DEF_BOTTOM						140

#define LIST_TOP						20		//���X�g�r���[�̍���

#define BTN_LEFT						100		//�{�^���̈ʒu
#define BTN_RIGHT						88
#define BTN_BOTTOM						23
#define BTN_VIEW_TOP					10
#define BTN_WAIT_TOP					40
#define BTN_INFO_TOP					70

#define UPINFO_VIEWSPEED				20		//�ڍׂ�W�J���鑬��

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

//�O���Q��
extern HINSTANCE g_hinst;				//�A�v���P�[�V�����̃C���X�^���X�n���h��
extern HWND WWWCWnd;					//�{��
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

	�X�V�A�C�e�����ɃA�C�e������ǉ�

******************************************************************************/

BOOL AddUpItem(struct TPITEM *NewItemInfo)
{
	struct TPITEM **tpItemInfo;
	int i;

	if(UpItemList == NULL){
		UpItemListCnt = 0;
		return FALSE;
	}

	//�d���A�C�e�����̃`�F�b�N
	for(i = 0; i < UpItemListCnt; i++){
		if((*(UpItemList + i)) == NULL){
			continue;
		}
		//���ɓ����A�C�e�������݂��Ă���ꍇ�͏㏑�����s��
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
	//�R�s�[�̍쐬
	*(tpItemInfo + UpItemListCnt) = Item_Copy(NewItemInfo);

	GlobalFree(UpItemList);
	UpItemList = tpItemInfo;

	UpItemListCnt++;
	return TRUE;
}


/******************************************************************************

	CopyUPItem

	�X�V�A�C�e������\���A�C�e�����Ɉڂ�

******************************************************************************/

static BOOL CopyUPItem(void)
{
	struct TPITEM **tpItemInfo;
	int DecCnt, i, j;

	if(UpItemList == NULL){
		UpItemListCnt = 0;
		return FALSE;
	}

	//�폜�ς݃J�E���g
	DecCnt = 0;

	//�d���A�C�e�����̃`�F�b�N
	for(j = 0; j < UpItemListCnt; j++){
		if((*(UpItemList + j)) == NULL){
			continue;
		}
		for(i = 0; i < ViewUpItemListCnt; i++){
			if((*(ViewUpItemList + i)) == NULL){
				continue;
			}
			//���ɓ����A�C�e�������݂��Ă���ꍇ�͏㏑�����s��
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

	//�\���A�C�e�����̍쐬
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

	�\��UP�A�C�e���̉��

******************************************************************************/

static void FreeViewUpItemList(void)
{
	if(ViewUpItemList == NULL){
		return;
	}
	//UP�A�C�e���̃����������
	FreeItemList(ViewUpItemList, ViewUpItemListCnt, FALSE);
	GlobalFree(ViewUpItemList);
	ViewUpItemList = NULL;
	ViewUpItemListCnt = 0;
}


/******************************************************************************

	ListView_UpGetDispItem

	���X�g�r���[�̃A�C�e�����v��

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

	//�e�L�X�g�ݒ�
	if(lvItem->mask & LVIF_TEXT){
		if((upColumnInfo + lvItem->iSubItem)->p == 0){
			//�A�C�e�����i�[����Ă���t�H���_�̃p�X
			if(IsTreeItem(WWWCWnd, tpItemInfo->hItem) == TRUE){
				TreeView_GetItemInfo(GetDlgItem(WWWCWnd, WWWC_TREE), RootItem, tmp1);
				wsprintf(tmp2, "\\\\%s", tmp1);
				TreeView_GetPath(GetDlgItem(WWWCWnd, WWWC_TREE), tpItemInfo->hItem, lvItem->pszText, tmp2);
			}

		}else if((char *)*((long *)((long)tpItemInfo + (upColumnInfo + lvItem->iSubItem)->p)) != NULL){
			//�A�C�e�����ƃJ������񂩂�e�L�X�g��ݒ�
			lstrcpyn(lvItem->pszText,
				(char *)*((long *)((long)tpItemInfo + (upColumnInfo + lvItem->iSubItem)->p)), BUFSIZE - 1);

		}else{
			*(lvItem->pszText) = '\0';
		}
	}

	if((lvItem->mask & LVIF_IMAGE) == 0){
		return FALSE;
	}

	//�A�C�R�����ݒ�
	i = GetProtocolIndex(tpItemInfo->CheckURL);
	//�A�C�e���̃A�C�R��
	if(tpItemInfo->Status & ST_UP){
		lvItem->iImage = (i == -1) ? ICON_UP : (tpProtocol + i)->Icon + 3;
	}else{
		lvItem->iImage = (i == -1) ? ICON_NOICON : (tpProtocol + i)->Icon;
	}
	return FALSE;
}


/******************************************************************************

	SetSubItemClipboardData

	ListView�A�C�e�����N���b�v�{�[�h�ɃR�s�[

******************************************************************************/

BOOL SetSubItemClipboardData(HWND hWnd)
{
	HANDLE hMem, hMemText, hMemDrop;
	int ErrCode;

	//�N���b�v�{�[�h���J��
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

	//�t�H���_���
	hMem = Clipboard_Set_WF_ItemList(hWnd, FLAG_COPY, NULL);
	if(hMem == NULL){
		CloseClipboard();
		return FALSE;
	}
	//�Ǝ��t�H�[�}�b�g�̐ݒ�
	if(SetClipboardData(WWWC_ClipFormat, hMem) == NULL){
		ErrCode = GetLastError();
		CloseClipboard();
		ErrMsg(hWnd, ErrCode, NULL);
		return FALSE;
	}

	//�h���b�v�t�@�C��
	if(DragDrop_CreateDropFiles(hWnd, DnD_DirName) == FALSE){
		return FALSE;
	}
	if((hMemDrop = DragDrop_SetDropFileMem(hWnd)) != NULL){
		SetClipboardData(CF_HDROP, hMemDrop);
	}
	FreeDropFiles();

	//�t�H���_�̃p�X
	hMemText = Clipboard_Set_TEXT(hWnd, NULL);
	if(hMemText != NULL){
		SetClipboardData(CF_TEXT, hMemText);
	}
	CloseClipboard();
	return TRUE;
}


/******************************************************************************

	StartDragSubItem

	ListView�A�C�e���̃h���b�O���h���b�v

******************************************************************************/

void StartDragSubItem(HWND hWnd)
{
	UINT cf[DnD_CLIPFORMAT_CNT];
	int ret;
	int i;
	int cfcnt;
	BOOL NoFileFlag;

	DgdpItem = NULL;

	//Alt�L�[��������Ă���ꍇ�̓t�@�C��������Ȃ�
	NoFileFlag = (GetAsyncKeyState(VK_MENU) < 0) ? TRUE : FALSE;

	if(NoFileFlag == FALSE){
		if(DragDrop_CreateDropFiles(hWnd, DnD_DirName) == FALSE){
			return;
		}
	}

	WWWCDropFlag = FALSE;

	//�N���b�v�{�[�h�t�H�[�}�b�g�̐ݒ�
	cf[0] = WWWC_ClipFormat;
	cf[1] = CF_TEXT;
	cf[2] = CF_HDROP;
	cfcnt = (NoFileFlag == FALSE) ? CLIPFORMAT_CNT : CLIPFORMAT_CNT - 1;

	ret = OLE_IDropSource_Start(hWnd, WM_DATAOBJECT_GETDATA, cf, cfcnt,
		DROPEFFECT_COPY | DROPEFFECT_LINK);

	if(NoFileFlag == FALSE){
		FreeDropFiles();
	}

	//�A�C�e����WWWC�ȊO�ɃR�s�[���ꂽ�ꍇ
	if(ret != -1 && ret != DROPEFFECT_NONE && WWWCDropFlag == FALSE){
		switch(DnDReturnIcon)
		{
		case 0:			//���������Ȃ�
			break;

		case 1:			//�I�����ꌏ�̏ꍇ�̂݃A�C�R����������
			if(ListView_GetSelectedCount(GetDlgItem(hWnd, WWWC_LIST)) != 1){
				break;
			}
			if((i = ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), -1, LVNI_SELECTED)) == -1){
				break;
			}
			InitSubIcon(hWnd, i);
			SetTrayInitIcon(WWWCWnd);
			break;

		case 2:			//�A�C�R����������
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

	UP���b�Z�[�W�̃��X�g�r���[�w�b�_���b�Z�[�W

******************************************************************************/

static LRESULT UpListViewHeaderNotifyProc(HWND hWnd, LPARAM lParam, struct TPLVCOLUMN *ColInfo)
{
	HD_NOTIFY *phd = (HD_NOTIFY *)lParam;

	switch(phd->hdr.code)
	{
	case HDN_ITEMCLICK:			//�w�b�_���N���b�N���ꂽ�̂Ń\�[�g���s��
		WaitCursor(TRUE);

		//�\�[�g
		LvUPSortFlag = (ABS(LvUPSortFlag) == (phd->iItem + 1))
			? (LvUPSortFlag * -1) : (phd->iItem + 1);
		SortColInfo = ColInfo;
		ListView_SortItems(GetDlgItem(hWnd, WWWC_LIST), CompareFunc, LvUPSortFlag);

		//�A�C�e�����̍Đݒ�
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

	UP���b�Z�[�W�Ɩ{�̂̃A�C�R���̏�����

******************************************************************************/

BOOL InitSubIcon(HWND hDlg, int Index)
{
	struct TPITEM *tpItemInfo;
	struct TPITEM *SelItemInfo;

	SelItemInfo = (struct TPITEM *)ListView_GetlParam(GetDlgItem(hDlg, WWWC_LIST), Index);
	if(SelItemInfo == NULL){
		return FALSE;
	}
	//�A�C�e�������擾
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

	UP���b�Z�[�W����A�C�e�����������ăA�C�R���̏�����

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

	���b�Z�[�W�ƃ��C���E�B���h�E�̃��X�g�r���[�Ƃ̑I���A�C�e���̓���

******************************************************************************/

BOOL MainItemSelect(HWND hWnd)
{
	struct TPITEM *tpItemInfo;
	struct TPITEM *SelItemInfo;
	int i;

	//�t�H�[�J�X�����A�C�e���̎擾
	if((i = ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), -1, LVNI_FOCUSED)) == -1){
		return FALSE;
	}
	SelItemInfo = (struct TPITEM *)ListView_GetlParam(GetDlgItem(hWnd, WWWC_LIST), i);
	if(SelItemInfo == NULL || IsTreeItem(WWWCWnd, SelItemInfo->hItem) == FALSE){
		MessageBox(hWnd, EMSG_NOMAINITEM, EMSG_MAINSHOW_TITLE, MB_ICONSTOP);
		return FALSE;
	}
	//�{�̃c���[�̑I��
	TreeView_SelectItem(GetDlgItem(WWWCWnd, WWWC_TREE), SelItemInfo->hItem);
	FocusWnd = GetDlgItem(WWWCWnd, WWWC_LIST);
	//�{�̃E�B���h�E�̕\��
	SendMessage(WWWCWnd, WM_COMMAND, ID_MENUITEM_REWINDOW, 0);

	//�{�̂̃A�C�e�������擾
	tpItemInfo = FindMainItem(WWWCWnd, SelItemInfo);
	if(tpItemInfo == NULL){
		return FALSE;
	}

	//�S�ẴA�C�e���̑I����Ԃ�����
	ListView_SetItemState(GetDlgItem(WWWCWnd, WWWC_LIST), -1, 0, LVIS_SELECTED);
	//�w��A�C�e����I����Ԃɂ���
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

	�{�̂̃A�C�e���̃v���p�e�B��\��

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
	//�t�H�[�J�X�����A�C�e���̎擾
	i = ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), -1, LVNI_FOCUSED);
	if(ListView_GetItemState(GetDlgItem(hWnd, WWWC_LIST), i, LVIS_SELECTED) != LVIS_SELECTED){
		i = ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), -1, LVNI_SELECTED);
	}
	SelItemInfo = (struct TPITEM *)ListView_GetlParam(GetDlgItem(hWnd, WWWC_LIST), i);
	if(SelItemInfo == NULL || IsTreeItem(WWWCWnd, SelItemInfo->hItem) == FALSE){
		MessageBox(hWnd, EMSG_NOMAINITEM, EMSG_PROP_TITLE, MB_ICONSTOP);
		return FALSE;
	}

	//�{�̂̃A�C�e�������擾
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

	//�v���p�e�B�p�A�C�e���̍쐬
	PropItemInfo = Item_Copy(tpItemInfo);
	if(PropItemInfo == NULL){
		return FALSE;
	}

	if(PropItemInfo->Comment != NULL){
		//�R�����g�̉��s�R�[�h��ϊ�
		EscToCode(PropItemInfo->Comment);
	}

	//�v���p�e�B�\��
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
		//�R�����g�̉��s�R�[�h��ϊ�
		p = CodeToEsc(PropItemInfo->Comment);
		GlobalFree(PropItemInfo->Comment);
		PropItemInfo->Comment = p;
	}

	//Sub�E�B���h�E�̃A�C�e���ɔ��f
	i = ListView_GetMemToIndex(GetDlgItem(hWnd, WWWC_LIST), SelItemInfo);
	if(i != -1){
		FreeItemInfo(SelItemInfo, FALSE);
		Item_ContentCopy(SelItemInfo, PropItemInfo);

		ListView_SetItemText(GetDlgItem(hWnd, WWWC_LIST), i, 0, LPSTR_TEXTCALLBACK);
		ListView_RedrawItems(GetDlgItem(hWnd, WWWC_LIST), i, i);
		UpdateWindow(GetDlgItem(hWnd, WWWC_LIST));
	}

	//�{�̂̃A�C�e���ɔ��f
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

	//�{�̂̃��X�g�r���[�̍X�V
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

	UP���b�Z�[�W�̏�����

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

	//�R���g���[���̈ʒu���Z�b�g
	SetUpMessageControls(hDlg);

	//���X�g�r���[��������
	SetWindowLong(GetDlgItem(hDlg, WWWC_LIST), GWL_STYLE,
		GetWindowLong(GetDlgItem(hDlg, WWWC_LIST), GWL_STYLE) | LVS_SHOWSELALWAYS);
	//���X�g�r���[�̊g���X�^�C����ݒ�
	SendDlgItemMessage(hDlg, WWWC_LIST, LVM_SETEXTENDEDLISTVIEWSTYLE, 0,
		LvUPExStyle | SendDlgItemMessage(hDlg, WWWC_LIST, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0));
	//���X�g�r���[�̃t�H���g�ݒ�
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

	UP���b�Z�[�W�̏I������

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

	//���X�g�r���[�̃J�����̃T�C�Y���擾
	for(i = 0; i < LvUPColCnt; i++){
		LvUPColSize[i] = ListView_GetColumnWidth(GetDlgItem(hDlg, WWWC_LIST), i);
	}

	FreeViewUpItemList();
	UpWnd = NULL;

	DestroyWindow(hDlg);
}


/******************************************************************************

	ShowUpMessage

	UP���b�Z�[�W�̕\��

******************************************************************************/

static void ShowUpMessage(HWND hDlg)
{
	char buf[BUFSIZE];
	char fDay[BUFSIZE];
	char fTime[BUFSIZE];
	int i;
	int UpItemCnt;
	BOOL UpMsgFlag;

	//�X�V�A�C�e���̑��݂������t���O
	UpMsgFlag = (UpItemListCnt == 0) ? TRUE : FALSE;

	if(ClearTime == 1){		//�X�V���A�C�e�����������ݒ�
		switch(CheckUPItemClear)
		{
		//������
		case 0:
			//�O��̍X�V�A�C�e���������
			SendMessage(hDlg, WM_UP_FREE, 0, 0);
			break;

		//�E�B���h�E���\����Ԃ̂Ƃ�
		case 1:
			if(IsWindowVisible(hDlg) == 0){
				//�O��̍X�V�A�C�e���������
				SendMessage(hDlg, WM_UP_FREE, 0, 0);
			}
			break;

		//��ɂ��Ȃ�
		case 2:
			break;
		}
	}

	//�E�B���h�E���\������Ă��Ȃ��ꍇ�̓T�C�Y�̏��������s��
	if(IsWindowVisible(hDlg) == 0){
		SendMessage(hDlg, WM_UP_WININI, 0, 0);
	}

	//�X�V���b�Z�[�W�\��
	if(NoUpMsg == 1 || UPMsg == 1){
		ShowWindow(hDlg, SW_SHOWNOACTIVATE);
		if(UPMsgTop == 0){
			//��A�N�e�B�u�őO�ʂɎ����Ă���
			SetWindowPos(hDlg, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
			SetWindowPos(hDlg, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
		}
	}

	//���X�g�r���[�̃A�C�e�������ׂď���
	ListView_DeleteAllItems(GetDlgItem(hDlg, WWWC_LIST));

	//UP�A�C�e���̃��������쐬
	CopyUPItem();
	ListView_SetItemCount(GetDlgItem(hDlg, WWWC_LIST), ViewUpItemListCnt);

	//���X�g�r���[�ɃA�C�e����ǉ��i�R�[���o�b�N�A�C�e���j
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

	//�����\�[�g
	if(CheckUPItemAutoSort == 1){
		SortColInfo = upColumnInfo;
		ListView_SortItems(GetDlgItem(hDlg, WWWC_LIST), CompareFunc, LvUPSortFlag);

		//�A�C�e�����̍Đݒ�
		GlobalFree(ViewUpItemList);
		ViewUpItemListCnt = 0;
		ViewUpItemList = ListView_SetListToMem(hDlg, &ViewUpItemListCnt);
	}
	ListView_SetItemState(GetDlgItem(hDlg, WWWC_LIST), 0, LVIS_FOCUSED, LVIS_FOCUSED);

	//�X�V���b�Z�[�W��\��
	GetDateTime(fDay, fTime);
	if(UpMsgFlag == TRUE){
		//�X�V�A�C�e�������݂��Ȃ��ꍇ
		if(UpItemCnt == 0){
			//�\����0���̏ꍇ
			wsprintf(buf, STR_ZEROMSG, fDay, fTime);
		}else{
			//�\��������ꍇ
			wsprintf(buf, STR_NOTUPMSG, UpItemCnt, fDay, fTime);
		}
	}else{
		//�X�V�A�C�e�������݂���ꍇ
		wsprintf(buf, STR_UPMSG, UpItemCnt, fDay, fTime);
	}
	SetWindowText(GetDlgItem(hDlg, IDC_STATIC_MSG), buf);

	//�A�N�e�B�u�ɂ���ݒ�
	if(UPMsg == 1 && UPActive == 1){
		ShowWindow(hDlg, SW_SHOW);
		_SetForegroundWindow(hDlg);
	}
}


/******************************************************************************

	SizeInitUpMessage

	UP���b�Z�[�W�̃T�C�Y�̏�����

******************************************************************************/

static void SizeInitUpMessage(HWND hDlg, int Size, int Pos, int Expand)
{
	RECT DesktopRect, WindowRect, lvRect, StRect, BnRect, WinRec;
	int i, BnSize;

	GetWindowRect(GetDesktopWindow(), (LPRECT)&DesktopRect);	//�f�X�N�g�b�v�̃T�C�Y
	GetWindowRect(hDlg, (LPRECT)&WindowRect);				//�E�B���h�E�̃T�C�Y
	GetWindowRect(GetDlgItem(hDlg, IDC_STATIC_MSG), (LPRECT)&StRect);

	GetWindowRect(GetDlgItem(hDlg, IDC_STATIC_BANNER), (LPRECT)&BnRect);
	BnSize = (BnRect.bottom - BnRect.top < 32) ? 32 : BnRect.bottom - BnRect.top;

	if(Size == 1 && (UPWinBottom != 0 && UPWinRight != 0)){
		//�O��̃T�C�Y�ɕ\��
		SetWindowPos(hDlg, 0, 0, 0, UPWinRight, WindowRect.bottom - WindowRect.top,
			SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
	}else{
		UPWinBottom = DEF_BOTTOM;

		i = StRect.right - WindowRect.left + GetSystemMetrics(SM_CYSIZEFRAME) + DEF_RIGHT;
		SetWindowPos(hDlg, 0, 0, 0, i, WindowRect.bottom - WindowRect.top,
			SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
	}

	GetWindowRect(hDlg, (LPRECT)&WindowRect);				//�E�B���h�E�̃T�C�Y
	GetClientRect(hDlg, (LPRECT)&WinRec);
	GetWindowRect(GetDlgItem(hDlg, WWWC_LIST), (LPRECT)&lvRect);

	SetWindowPos(GetDlgItem(hDlg, WWWC_LIST), 0, 0,StRect.bottom - StRect.top + BnSize + LIST_TOP,
		WinRec.right, UPWinBottom, SWP_NOACTIVATE | SWP_NOZORDER);
	GetWindowRect(GetDlgItem(hDlg, WWWC_LIST), (LPRECT)&lvRect);

	//�E�B���h�E�̏c�T�C�Y���Z�b�g
	i = lvRect.top - WindowRect.top + GetSystemMetrics(SM_CYSIZEFRAME);
	if(Expand == 1){
		//�E�B���h�E��W�J��Ԃɂ���ꍇ�́A�{�^���̖��O��ύX
		SetWindowText(GetDlgItem(hDlg, IDC_BUTTON_INFO), BTN_UPINFO_OFF);
		//�E�B���h�E�̏c�T�C�Y�Ƀ��X�g�r���[�̃T�C�Y��������
		i = lvRect.bottom - WindowRect.top + GetSystemMetrics(SM_CYSIZEFRAME);
		ShowWindow(GetDlgItem(hDlg, WWWC_LIST), SW_SHOW);
	}else{
		SetWindowText(GetDlgItem(hDlg, IDC_BUTTON_INFO), BTN_UPINFO_ON);
		ShowWindow(GetDlgItem(hDlg, WWWC_LIST), SW_HIDE);
	}

	//�Z���^�����O�̂��߂Ɉꎞ�E�B���h�E�T�C�Y�ύX
	if(UPWinExpandCenter == 1){
		SetWindowPos(hDlg, 0, 0, 0,
			WindowRect.right - WindowRect.left,
			lvRect.bottom - WindowRect.top + GetSystemMetrics(SM_CYSIZEFRAME),
			SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
	}else{
		SetWindowPos(hDlg, 0, 0, 0, WindowRect.right - WindowRect.left, i,
			SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
	}
	GetWindowRect(hDlg, (LPRECT)&WindowRect);				//�E�B���h�E�̃T�C�Y

	if(Pos == 1 && (UPWinLeft != 0 && UPWinTop != 0)){
		//�O��̕\���ʒu�ɕ\��
		SetWindowPos(hDlg, 0, UPWinLeft, UPWinTop, WindowRect.right - WindowRect.left, i,
			SWP_NOACTIVATE | SWP_NOZORDER);
	}else{
		//�Z���^�����O���ăf�t�H���g�̃T�C�Y�ŕ\��
		SetWindowPos(hDlg, 0,
			(DesktopRect.right / 2) - ((WindowRect.right - WindowRect.left) / 2),
			(DesktopRect.bottom / 2) - ((WindowRect.bottom - WindowRect.top) / 2),
			WindowRect.right - WindowRect.left, i, SWP_NOACTIVATE | SWP_NOZORDER);
	}
}


/******************************************************************************

	SetUpMessageControls

	UP���b�Z�[�W���̃R���g���[���̃T�C�Y�E�ʒu��ύX

******************************************************************************/

static void SetUpMessageControls(HWND hDlg)
{
	RECT WinRec, WindowRect, lvRect, StRect, BnRect;
	char buf[BUFSIZE];
	int BnSize;

	GetClientRect(hDlg, (LPRECT)&WinRec);

	//�{�^���̈ʒu�ݒ�
	MoveWindow(GetDlgItem(hDlg, IDC_BUTTON_VIEW),
		WinRec.right - BTN_LEFT, BTN_VIEW_TOP, BTN_RIGHT, BTN_BOTTOM, TRUE);
	MoveWindow(GetDlgItem(hDlg, IDC_BUTTON_WAIT),
		WinRec.right - BTN_LEFT, BTN_WAIT_TOP, BTN_RIGHT, BTN_BOTTOM, TRUE);
	MoveWindow(GetDlgItem(hDlg, IDC_BUTTON_INFO),
		WinRec.right - BTN_LEFT, BTN_INFO_TOP, BTN_RIGHT, BTN_BOTTOM, TRUE);

	//�E�B���h�E�̍����ݒ�
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

	//���b�Z�[�W�̍ĕ`��
	InvalidateRect(GetDlgItem(hDlg, IDC_STATIC_MSG), NULL, FALSE);
	UpdateWindow(GetDlgItem(hDlg, IDC_STATIC_MSG));

	//�{�^���̍ĕ`��
	InvalidateRect(GetDlgItem(hDlg, IDC_BUTTON_VIEW), NULL, FALSE);
	UpdateWindow(GetDlgItem(hDlg, IDC_BUTTON_VIEW));
	InvalidateRect(GetDlgItem(hDlg, IDC_BUTTON_WAIT), NULL, FALSE);
	UpdateWindow(GetDlgItem(hDlg, IDC_BUTTON_WAIT));
	InvalidateRect(GetDlgItem(hDlg, IDC_BUTTON_INFO), NULL, FALSE);
	UpdateWindow(GetDlgItem(hDlg, IDC_BUTTON_INFO));
}


/******************************************************************************

	SizeUpMessage

	UP���b�Z�[�W�̃T�C�Y�ύX

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
		//�ڍו\���Ō��̃T�C�Y�ɖ߂����ꍇ
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
		//�ڍה�\���Ō��̃T�C�Y�ɖ߂����ꍇ
		MaxWndFlag = FALSE;
		GetWindowRect(GetDlgItem(hDlg, WWWC_LIST), (LPRECT)&lvRect);
		SetWindowPos(GetDlgItem(hDlg, WWWC_LIST), 0, 0, 0,
			lvRect.right - lvRect.left, UPWinBottom, SWP_NOZORDER | SWP_NOMOVE);

	}else if(wParam == SIZE_MAXIMIZED && MaxWndFlag == FALSE){
		//�ő剻���ꂽ�ꍇ
		MaxWndFlag = TRUE;
		SetWindowText(GetDlgItem(hDlg, IDC_BUTTON_INFO), BTN_UPINFO_OFF);
		ShowWindow(GetDlgItem(hDlg, WWWC_LIST), SW_SHOW);
	}
	//�R���g���[���̈ʒu���Z�b�g
	SetUpMessageControls(hDlg);
}


/******************************************************************************

	RefreshListView

	UP���b�Z�[�W�Ɩ{�̂̕`��̍X�V

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

	�ڍׂ̕\����Ԃ�؂�ւ���

******************************************************************************/

static void ViewUpInfo(HWND hDlg)
{
	RECT WindowRect, lvRect;
	char buf[BUFSIZE];
	int i, j;

	GetWindowRect(hDlg, (LPRECT)&WindowRect);					//�E�B���h�E�̃T�C�Y
	GetWindowRect(GetDlgItem(hDlg, WWWC_LIST), (LPRECT)&lvRect);	//���X�g�r���[�̃T�C�Y

	GetWindowText(GetDlgItem(hDlg, IDC_BUTTON_INFO), buf, BUFSIZE - 1);
	if(lstrcmp(buf, BTN_UPINFO_OFF) == 0){
		//�ڍׂ��B��
		SetWindowText(GetDlgItem(hDlg, IDC_BUTTON_INFO), BTN_UPINFO_ON);
		ShowWindow(GetDlgItem(hDlg, WWWC_LIST), SW_HIDE);

		//�E�B���h�E�̏c�T�C�Y���Z�b�g
		i = lvRect.top - WindowRect.top + GetSystemMetrics(SM_CYSIZEFRAME);

	}else{
		//�ڍׂ�\��
		//�E�B���h�E��W�J��Ԃɂ���ꍇ�́A�{�^���̖��O��ύX
		SetWindowText(GetDlgItem(hDlg, IDC_BUTTON_INFO), BTN_UPINFO_OFF);
		ShowWindow(GetDlgItem(hDlg, WWWC_LIST), SW_SHOW);

		//���X�ɕ\��
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
	//�E�B���h�E�̃T�C�Y��ݒ�
	SetWindowPos(hDlg, 0, 0, 0,WindowRect.right - WindowRect.left, i,
		SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
	SetUpMessageControls(hDlg);
}


/******************************************************************************

	ExeAction

	�v���g�R�����̏�������уc�[���̎��s

******************************************************************************/

void ExeAction(HWND hDlg, WPARAM wParam)
{
	struct TPITEM **ToolItemList = NULL;
	struct TPITEM **TmpItemList;
	int i;
	int id;

	//�v���g�R�����̏���
	if(LOWORD(wParam) >= ID_MENU_ACTION && LOWORD(wParam) < (ID_MENU_ACTION + MENU_MAX)){
		Item_Open(hDlg, LOWORD(wParam) - ID_MENU_ACTION);
		return;
	}

	id = LOWORD(wParam) - ID_MENU_TOOL_ACTION;
	if(LOWORD(wParam) < ID_MENU_TOOL_ACTION || LOWORD(wParam) >= (ID_MENU_TOOL_ACTION + MENU_MAX)){
		return;
	}

	//�w��v���g�R���̃A�C�e���𒊏o
	TmpItemList = ListView_SelectItemToMem(GetDlgItem(hDlg, WWWC_LIST), &i);
	if(TmpItemList != NULL){
		ToolItemList = Item_ProtocolSelect(TmpItemList, &i, ToolList[id].Protocol);
		GlobalFree(TmpItemList);
	}
	if(ToolItemList == NULL){
		return;
	}
	//�{�̂̃A�C�e���Ɠ���
	WaitCursor(TRUE);
	Item_CopyMainContent(WWWCWnd, ToolItemList, i);
	WaitCursor(FALSE);

	//�c�[���̎��s
	SubItemExecTool(hDlg, id, ToolItemList, i, TOOL_EXEC_ITEMMENU, 0);

	WaitCursor(TRUE);
	//�X�V�t���O�������Ă���A�C�e����{�̂ɏ����߂�
	Item_CopyMainRefreshContent(WWWCWnd, ToolItemList, i);
	if(ToolList[id].Action & TOOL_EXEC_INITITEM){
		//�A�C�R���̏�����
		Item_MainItemIni(WWWCWnd, hDlg, ToolItemList, i);
	}
	//�A�C�e�����̉��
	GlobalFree(ToolItemList);
	CallTreeItem(WWWCWnd, RootItem, (FARPROC)TreeView_FreeItem, 1);

	//�\���̍X�V
	RefreshListView(hDlg);
	WaitCursor(FALSE);
}


/******************************************************************************

	SetBannerSubClass

	�E�B���h�E�̃T�u�N���X��

******************************************************************************/

static void SetBannerSubClass(HWND hWnd)
{
	BannerWindowProcedure = (WNDPROC)SetWindowLong(hWnd, GWL_WNDPROC, (long)SubClassBannerProc);
}


/******************************************************************************

	DelBannerSubClass

	�E�B���h�E�N���X��W���̂��̂ɖ߂�

******************************************************************************/

static void DelBannerSubClass(HWND hWnd)
{
	SetWindowLong(hWnd, GWL_WNDPROC, (long)BannerWindowProcedure);
	BannerWindowProcedure = NULL;
}


/******************************************************************************

	SubClassBannerProc

	�E�B���h�E�̃T�u�N���X��

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

	UP���b�Z�[�W�̃E�B���h�E�v���V�[�W��

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

		//�T�C�Y�̏�����
		SizeFlag = TRUE;
		SizeInitUpMessage(hDlg, UPWinSizeSave, UPWinPosSave, UPMsgExpand);
		SizeFlag = FALSE;

		//�R���g���[���̈ʒu���Z�b�g
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
		//�ڍׂ��B����Ă���ꍇ�̓T�C�Y�𐧌�
		GetWindowText(GetDlgItem(hDlg, IDC_BUTTON_INFO), buf, BUFSIZE - 1);
		if(lstrcmp(buf, BTN_UPINFO_ON) == 0){
			GetWindowRect(GetDlgItem(hDlg, WWWC_LIST), (LPRECT)&lvRect);
			((LPRECT)lParam)->bottom = lvRect.top + GetSystemMetrics(SM_CYSIZEFRAME);
		}
		return FALSE;

	case WM_EXITSIZEMOVE:
		if(IsWindowVisible(hDlg) != 0 && IsIconic(hDlg) == 0 && IsZoomed(hDlg) == 0){
			//�E�B���h�E�̈ʒu��ۑ�
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
		//���̃A�C�e���Ƀt�H�[�J�X���ړ�����
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
		//���X�g�r���[�w�b�_�R���g���[��
		if(((NMHDR *)lParam)->hwndFrom == GetWindow(GetDlgItem(hDlg, WWWC_LIST), GW_CHILD)){
			return UpListViewHeaderNotifyProc(hDlg, lParam, upColumnInfo);
		}
		//���X�g�r���[
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
				//�t�H�[�J�X�̎��{�^����I��
				SendMessage(hDlg, WM_COMMAND, GetDlgCtrlID(GetFocus()), 0);
				break;
			}
			//�v���g�R�����̃��j���[��ݒ�
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
			//�A�C�e�����j���[�̕\��
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
		//�ҋ@
		case IDC_BUTTON_WAIT:
			SendMessage(hDlg, WM_CLOSE, 0, 0);
			break;

		//�Q��
		case IDC_BUTTON_VIEW:
			if(ViewUpItemListCnt != 0 && ViewUpItemList != NULL && *ViewUpItemList != NULL
				&& IsTreeItem(WWWCWnd, (*ViewUpItemList)->hItem) == TRUE){
				//��Ԗڂ̃A�C�e���̃t�H���_��I��
				TreeView_SelectItem(GetDlgItem(WWWCWnd, WWWC_TREE),
					(*ViewUpItemList)->hItem);
			}
			//�{�̃E�B���h�E�̕\��
			SendMessage(WWWCWnd, WM_COMMAND, ID_MENUITEM_REWINDOW, 0);
			ShowWindow(hDlg, SW_HIDE);
			break;

		//�ڍ�
		case IDC_BUTTON_INFO:
			//�ڍׂ̕\���̐؂�ւ�
			ViewUpInfo(hDlg);
			break;

		//�J��
		case ID_MENU_ACTION_OPEM:
			Item_Open(hDlg, -1);
			break;

		//�S�đI��
		case ID_MENUITEM_ALLSELECT:
			SetFocus(GetDlgItem(hDlg, WWWC_LIST));
			ListView_SetItemState(GetDlgItem(hDlg, WWWC_LIST), -1, LVIS_SELECTED, LVIS_SELECTED);
			break;

		//�X�V�A�C�e����I��
		case ID_MENUITEM_UPSELECT:
			SetFocus(GetDlgItem(hDlg, WWWC_LIST));
			ListView_UpSelectItem(GetDlgItem(hDlg, WWWC_LIST));
			break;

		//�\��
		case ID_MENUITEM_UPVIEW:
			//�{�̂̃A�C�R����\��
			MainItemSelect(hDlg);
			break;

		//�R�s�[
		case ID_MENUITEM_COPY:
			SetSubItemClipboardData(hDlg);
			break;

		//�S�ẴA�C�R����������
		case ID_MENUITEM_ALLINITICON:
			WaitCursor(TRUE);
			for(i = 0; i < ListView_GetItemCount(GetDlgItem(hDlg, WWWC_LIST)); i++){
				InitSubIcon(hDlg, i);
			}
			RefreshListView(hDlg);
			WaitCursor(FALSE);
			break;

		//�A�C�R���̏�����
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

		//�N���A
		case ID_MENUITEM_UPITEMCLEAR:
			SendMessage(hDlg, WM_UP_FREE, 0, 0);
			break;

		//�v���p�e�B
		case ID_MENUITEM_PROP:
			MainItemProp(hDlg);
			break;

		default:
			ExeAction(hDlg, wParam);
			break;
		}
		break;

	//���X�g�r���[�̃C�x���g
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

	//���X�g�r���[�A�C�e���̏�����
	case WM_LV_INITICON:
		InitSubIcon(hDlg, wParam);
		RefreshListView(hDlg);
		break;

	//�h���b�O���h���b�v�̃f�[�^�v��
	case WM_DATAOBJECT_GETDATA:
		//WWWC�A�C�e���t�H�[�}�b�g
		if(wParam == WWWC_ClipFormat){
			*((HGLOBAL *)lParam) = Clipboard_Set_WF_ItemList(hDlg, FLAG_COPY, NULL);
			WWWCDropFlag = TRUE;
			break;
		}

		WWWCDropFlag = FALSE;
		switch(wParam)
		{
		case CF_TEXT:	//�e�L�X�g
			*((HGLOBAL *)lParam) = Clipboard_Set_TEXT(hDlg, NULL);
			break;

		case CF_HDROP:	//�h���b�v�t�@�C��
			*((HGLOBAL *)lParam) = DragDrop_SetDropFileMem(hDlg);
			break;
		}
		break;

	//DLL����̃��b�Z�[�W��{�̂ɓ]��
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
