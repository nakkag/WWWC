/**************************************************************************

	WWWC

	FindItem.c

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

#define FIND_HISTORY				30

#define WM_SETDLGFOCUS				(WM_USER + 1)
#define WM_SETFINDINFO				(WM_USER + 2)

#define TIMER_TABVIEW				1


/**************************************************************************
	Global Variables
**************************************************************************/

struct TPLVCOLUMN *FindColumnInfo = NULL;

static struct TPITEM **FindItemList;
static int FindItemListCnt = 0;

struct TPFINDINFO {
	HWND lvWnd;
	char FindString[BUFSIZE];
	HTREEITEM hItem;
	BOOL FindSub;
	BOOL FindNoCheck;
	int ItemFlag;
	int IconFlag;
};
static struct TPFINDINFO tpFindInfo;


//�O���Q��
extern HINSTANCE g_hinst;				//�A�v���P�[�V�����̃C���X�^���X�n���h��
extern HWND WWWCWnd;					//�{��
extern HWND FindWnd;
extern HTREEITEM RootItem;
extern HTREEITEM DgdpItem;
extern struct TP_PROTOCOL *tpProtocol;
extern struct TPLVCOLUMN *SortColInfo;
extern HMENU hPOPUP;
extern BOOL WWWCDropFlag;
extern UINT WWWC_ClipFormat;
extern HFONT ListFont;

extern int LvSpaceNextFocus;

extern int FindSubFolder;
extern int FindNoCheck;
extern int FindItemFlag;
extern int FindIconFlag;
extern int FindWinLeft;
extern int FindWinTop;
extern int FindWinRight;
extern int FindWinBottom;
extern int LvFindExStyle;
extern int LvFindColSize[];
extern int LvFindColCnt;
extern char LvFindColumn[];


/**************************************************************************
	Local Function Prototypes
**************************************************************************/

static BOOL AddFindItem(struct TPITEM *NewItemInfo);
static void FreeFindItemList(void);
static void CALLBACK FindItem(HWND hWnd, HTREEITEM hItem, long Param);
static void FindStart(HWND hDlg);
static LPARAM TabCtrl_GetlParam(HWND hTabCtrl, int iItem);
static void TabCtrl_SetlParam(HWND hTabCtrl, int iItem, LPARAM lParam);
static void InitializeFindWindow(HWND hDlg);
static void SizeFindWindow(HWND hDlg);
static void CloseFindWindow(HWND hDlg);
static BOOL CALLBACK FindNameProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK FindItemProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static LRESULT Tab_NotifyProc(HWND hWnd, LPARAM lParam, WPARAM wParam);
static LRESULT FindListViewHeaderNotifyProc(HWND hWnd, LPARAM lParam, struct TPLVCOLUMN *ColInfo);
static BOOL ListView_FindGetDispItem(HWND hWnd, LV_ITEM *lvItem);


/******************************************************************************

	AddFindItem

	�������ʃ��X�g�ɃA�C�e����ǉ�

******************************************************************************/

static BOOL AddFindItem(struct TPITEM *NewItemInfo)
{
	struct TPITEM **tpItemInfo;

	tpItemInfo = (struct TPITEM **)GlobalAlloc(GPTR,
		sizeof(struct TPITEM *) * (FindItemListCnt + 1));
	if(tpItemInfo == NULL){
		return FALSE;
	}

	if(FindItemList != NULL){
		CopyMemory(tpItemInfo, FindItemList, sizeof(struct TPITEM *) * FindItemListCnt);
	}
	//�R�s�[�̍쐬
	*(tpItemInfo + FindItemListCnt) = Item_Copy(NewItemInfo);

	if(FindItemList != NULL){
		GlobalFree(FindItemList);
	}
	FindItemList = tpItemInfo;
	FindItemListCnt++;
	return TRUE;
}


/******************************************************************************

	FreeFindItemList

	�������ʃ��X�g�̉��

******************************************************************************/

static void FreeFindItemList(void)
{
	if(FindItemList == NULL){
		return;
	}
	//UP�A�C�e���̃����������
	FreeItemList(FindItemList, FindItemListCnt, FALSE);
	GlobalFree(FindItemList);
	FindItemList = NULL;
	FindItemListCnt = 0;
}


/******************************************************************************

	FindItem

	�t�H���_���̃A�C�e��������

******************************************************************************/

static void CALLBACK FindItem(HWND hWnd, HTREEITEM hItem, long Param)
{
	struct TPTREE *tpTreeInfo;
	struct TPFINDINFO *FindInfo = (struct TPFINDINFO *)Param;
	int i;
	BOOL AddFlag;

	if((tpTreeInfo = (struct TPTREE *)TreeView_GetlParam(GetDlgItem(hWnd, WWWC_TREE), hItem)) == NULL){
		return;
	}
	if(FindInfo->FindNoCheck == TRUE && tpTreeInfo->CheckSt == 1){
		return;
	}
	if(tpTreeInfo->ItemList == NULL){
		ReadTreeMem(hWnd, hItem);
	}
	if(tpTreeInfo->ItemList == NULL){
		return;
	}

	//�A�C�e���̌���
	for(i = 0;i < tpTreeInfo->ItemListCnt;i++){
		if((*(tpTreeInfo->ItemList + i)) == NULL){
			continue;
		}
		//����
		AddFlag = FALSE;
		if((FindInfo->ItemFlag & 1) &&
			str_match(FindInfo->FindString, (*(tpTreeInfo->ItemList + i))->Title) == TRUE){
			AddFlag = TRUE;
		}
		if(AddFlag == FALSE && (FindInfo->ItemFlag & 2) &&
			str_match(FindInfo->FindString, (*(tpTreeInfo->ItemList + i))->CheckURL) == TRUE){
			AddFlag = TRUE;
		}
		if(AddFlag == FALSE && (FindInfo->ItemFlag & 4) &&
			str_match(FindInfo->FindString, (*(tpTreeInfo->ItemList + i))->ViewURL) == TRUE){
			AddFlag = TRUE;
		}
		if(AddFlag == FALSE && (FindInfo->ItemFlag & 8) &&
			str_match(FindInfo->FindString, (*(tpTreeInfo->ItemList + i))->Date) == TRUE){
			AddFlag = TRUE;
		}
		if(AddFlag == FALSE && (FindInfo->ItemFlag & 16) &&
			str_match(FindInfo->FindString, (*(tpTreeInfo->ItemList + i))->Comment) == TRUE){
			AddFlag = TRUE;
		}

		//�A�C�R��
		if(AddFlag == TRUE){
			AddFlag = FALSE;
			if((*(tpTreeInfo->ItemList + i))->Status & ST_ERROR){
				if(FindInfo->IconFlag & 4){
					AddFlag = TRUE;
				}
			}else if((*(tpTreeInfo->ItemList + i))->Status & ST_TIMEOUT){
				if(FindInfo->IconFlag & 8){
					AddFlag = TRUE;
				}
			}else if((*(tpTreeInfo->ItemList + i))->Status & ST_UP){
				if(FindInfo->IconFlag & 2){
					AddFlag = TRUE;
				}
			}else if(FindInfo->IconFlag & 1){
				AddFlag = TRUE;
			}

			if((*(tpTreeInfo->ItemList + i))->CheckSt == 1 && !(FindInfo->IconFlag & 16)){
				AddFlag = FALSE;
			}
		}

		if(AddFlag == TRUE){
			//�������ʃ��X�g�ɃA�C�e����ǉ�
			AddFindItem(*(tpTreeInfo->ItemList + i));
		}
	}
	TreeView_FreeItem(hWnd, hItem, 0);
}


/******************************************************************************

	FindStart

	�������J�n

******************************************************************************/

static void FindStart(HWND hDlg)
{
	char buf[BUFSIZE];
	int i;

	WaitCursor(TRUE);
	SendMessage(GetDlgItem(hDlg, WWWC_LIST), WM_SETREDRAW, (WPARAM)FALSE, 0);

	ListView_DeleteAllItems(GetDlgItem(hDlg, WWWC_LIST));
	FreeFindItemList();
	tpFindInfo.lvWnd = GetDlgItem(hDlg, WWWC_LIST);

	SendMessage((HWND)TabCtrl_GetlParam(GetDlgItem(hDlg, IDC_TAB), 0), WM_SETFINDINFO, 0, (LPARAM)&tpFindInfo);
	SendMessage((HWND)TabCtrl_GetlParam(GetDlgItem(hDlg, IDC_TAB), 1), WM_SETFINDINFO, 0, (LPARAM)&tpFindInfo);

	if(tpFindInfo.FindSub == TRUE){
		//�ċA���Č���
		CallTreeItem(WWWCWnd, tpFindInfo.hItem, (FARPROC)FindItem, (long)&tpFindInfo);
	}else{
		//�w��̃t�H���_�̂݌���
		FindItem(WWWCWnd, tpFindInfo.hItem, (long)&tpFindInfo);
	}

	if(FindItemList != NULL){
		ListView_SetItemCount(GetDlgItem(hDlg, WWWC_LIST), FindItemListCnt);

		//���X�g�r���[�ɃA�C�e����ǉ��i�R�[���o�b�N�A�C�e���j
		for(i = 0;i < FindItemListCnt;i++){
			if((*(FindItemList + i)) == NULL){
				continue;
			}
			ListView_InsertItemEx(GetDlgItem(hDlg, WWWC_LIST),
				(char*)LPSTR_TEXTCALLBACK, I_IMAGECALLBACK,
				(long)*(FindItemList + i), i);
			ListView_SetItemState(GetDlgItem(hDlg, WWWC_LIST), i,
				(((*(FindItemList + i))->CheckSt == 1) ? INDEXTOOVERLAYMASK(ICON_ST_NOCHECK) : 0), LVIS_OVERLAYMASK);
		}
	}
	SendMessage(GetDlgItem(hDlg, WWWC_LIST), WM_SETREDRAW, (WPARAM)TRUE, 0);
	UpdateWindow(GetDlgItem(hDlg, WWWC_LIST));
	WaitCursor(FALSE);

	if(FindItemListCnt > 0){
		SetFocus(GetDlgItem(hDlg, WWWC_LIST));
		ListView_SetItemState(GetDlgItem(hDlg, WWWC_LIST), 0, LVIS_FOCUSED, LVIS_FOCUSED);
	}

	wsprintf(buf, "���� - %d ��", FindItemListCnt);
	SetWindowText(hDlg, buf);
}


/******************************************************************************

	TabCtrl_GetlParam

	�^�u�R���g���[����lParam���擾

******************************************************************************/

static LPARAM TabCtrl_GetlParam(HWND hTabCtrl, int iItem)
{
	TC_ITEM tcItem;

	tcItem.mask = TCIF_PARAM;
	TabCtrl_GetItem(hTabCtrl, iItem, &tcItem);

	return tcItem.lParam;
}


/******************************************************************************

	TabCtrl_SetlParam

	�^�u�R���g���[����lParam��ݒ�

******************************************************************************/

static void TabCtrl_SetlParam(HWND hTabCtrl, int iItem, LPARAM lParam)
{
	TC_ITEM tcItem;

	tcItem.mask = TCIF_PARAM;
	tcItem.lParam = lParam;
	TabCtrl_SetItem(hTabCtrl, iItem, &tcItem);
}


/******************************************************************************

	InitializeFindWindow

	�����E�B���h�E�̏�����

******************************************************************************/

static void InitializeFindWindow(HWND hDlg)
{
	RECT WindowRect;
	TC_ITEM tcItem;
	NMHDR CForm;
	HICON hIcon;
	HICON hIconS;
	HWND tDlg;

	//�^�u�̐ݒ�
	tcItem.mask = TCIF_TEXT | TCIF_PARAM;
	tcItem.pszText = "����������Əꏊ";
	tcItem.cchTextMax = BUFSIZE - 1;
	tcItem.lParam = 0;
	TabCtrl_InsertItem(GetDlgItem(hDlg, IDC_TAB), 0, &tcItem);

	tcItem.pszText = "��������";
	tcItem.cchTextMax = BUFSIZE - 1;
	tcItem.lParam = 0;
	TabCtrl_InsertItem(GetDlgItem(hDlg, IDC_TAB), 1, &tcItem);

	//�^�u���ɕ\������_�C�A���O�̍쐬
	tDlg = CreateDialogParam(g_hinst, MAKEINTRESOURCE(IDD_DIALOG_FINDNAME), hDlg, FindNameProc, (LPARAM)&tpFindInfo);
	TabCtrl_SetlParam(GetDlgItem(hDlg, IDC_TAB), 0, (LPARAM)tDlg);

	tDlg = CreateDialogParam(g_hinst, MAKEINTRESOURCE(IDD_DIALOG_FINDITEM), hDlg, FindItemProc, (LPARAM)&tpFindInfo);
	TabCtrl_SetlParam(GetDlgItem(hDlg, IDC_TAB), 1, (LPARAM)tDlg);

	//�����^�u�̕\��
	CForm.hwndFrom = GetDlgItem(hDlg, IDC_TAB);
	CForm.code = TCN_SELCHANGE;
	Tab_NotifyProc(hDlg, (LPARAM)&CForm, 0);

	SetTimer(hDlg, TIMER_TABVIEW, 1, NULL);

	FindItemList = NULL;
	FindItemListCnt = 0;

	hIcon = ImageList_GetIcon(
		ListView_GetImageList(GetDlgItem(WWWCWnd, WWWC_LIST), LVSIL_NORMAL),
		ICON_CHECK, 0);
	hIconS = ImageList_GetIcon(
		ListView_GetImageList(GetDlgItem(WWWCWnd, WWWC_LIST), LVSIL_SMALL),
		ICON_CHECK, 0);

	SendMessage(hDlg, WM_SETICON, (WPARAM)TRUE, (LPARAM)hIcon);
	SendMessage(hDlg, WM_SETICON, (WPARAM)FALSE, (LPARAM)hIconS);

	//���X�g�r���[��������
	SetWindowLong(GetDlgItem(hDlg, WWWC_LIST), GWL_STYLE,
		GetWindowLong(GetDlgItem(hDlg, WWWC_LIST), GWL_STYLE) | LVS_SHOWSELALWAYS);
	//���X�g�r���[�̊g���X�^�C����ݒ�
	SendDlgItemMessage(hDlg, WWWC_LIST, LVM_SETEXTENDEDLISTVIEWSTYLE, 0,
		LvFindExStyle | SendDlgItemMessage(hDlg, WWWC_LIST, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0));
	//���X�g�r���[�̃t�H���g�ݒ�
	if(ListFont != NULL){
		SendMessage(GetDlgItem(hDlg, WWWC_LIST), WM_SETFONT, (WPARAM)ListFont, MAKELPARAM(TRUE, 0));
	}

	LvFindColCnt = ListView_AddColumn(GetDlgItem(hDlg, WWWC_LIST), LvFindColumn, LvFindColSize, FindColumnInfo);

	ListView_SetImageList(GetDlgItem(hDlg, WWWC_LIST),
		ListView_GetImageList(GetDlgItem(WWWCWnd, WWWC_LIST), LVSIL_NORMAL), LVSIL_NORMAL);
	ListView_SetImageList(GetDlgItem(hDlg, WWWC_LIST),
		ListView_GetImageList(GetDlgItem(WWWCWnd, WWWC_LIST), LVSIL_SMALL), LVSIL_SMALL);

	//���C���E�B���h�E�̃T�C�Y�ݒ�
	if(FindWinRight == 0 || FindWinBottom == 0){
		GetWindowRect(hDlg, (LPRECT)&WindowRect);
		FindWinRight = WindowRect.right - WindowRect.left;
		FindWinBottom = WindowRect.bottom - WindowRect.top;
	}
	MoveWindow(hDlg, FindWinLeft, FindWinTop, FindWinRight, FindWinBottom, TRUE);
	SizeFindWindow(hDlg);
	ShowWindow(hDlg, SW_SHOW);
}


/******************************************************************************

	SizeFindWindow

	�����E�B���h�E���R���g���[���̃T�C�Y�ݒ�

******************************************************************************/

static void SizeFindWindow(HWND hDlg)
{
	RECT WindowRect, tabRect, lvRect;
	HWND tDlg;

	GetClientRect(hDlg, (LPRECT)&WindowRect);
	GetWindowRect(GetDlgItem(hDlg, IDC_TAB), (LPRECT)&tabRect);
	GetWindowRect(GetDlgItem(hDlg, WWWC_LIST), (LPRECT)&lvRect);

	//�{�^��
	MoveWindow(GetDlgItem(hDlg, IDOK),
		WindowRect.right - 120, 30, 108, 23, TRUE);
	MoveWindow(GetDlgItem(hDlg, IDCANCEL),
		WindowRect.right - 120, 60, 108, 23, TRUE);
	MoveWindow(GetDlgItem(hDlg, ID_BUTTON_SAVEOPTION),
		WindowRect.right - 120, 100, 108, 23, TRUE);

	//�^�u
	MoveWindow(GetDlgItem(hDlg, IDC_TAB), 8, 8,
		WindowRect.right - 140, tabRect.bottom - tabRect.top, FALSE);

	//���X�g�r���[
	MoveWindow(GetDlgItem(hDlg, WWWC_LIST), 0, tabRect.bottom - tabRect.top + 20,
		WindowRect.right, WindowRect.bottom - (tabRect.bottom - tabRect.top + 20), TRUE);

	//�^�u���̃_�C�A���O
	tDlg = (HWND)TabCtrl_GetlParam(GetDlgItem(hDlg, IDC_TAB), TabCtrl_GetCurSel(GetDlgItem(hDlg, IDC_TAB)));
	GetClientRect(GetDlgItem(hDlg, IDC_TAB), &tabRect);
	TabCtrl_AdjustRect(GetDlgItem(hDlg, IDC_TAB), FALSE, &tabRect);
	MoveWindow(tDlg, tabRect.left + 8, tabRect.top + 8, tabRect.right - tabRect.left, tabRect.bottom - tabRect.top, FALSE);

	InvalidateRect(hDlg, NULL, TRUE);
	UpdateWindow(hDlg);
}


/******************************************************************************

	CloseFindWindow

	�����E�B���h�E�̏I������

******************************************************************************/

static void CloseFindWindow(HWND hDlg)
{
	HICON hIcon;
	HICON hIconS;
	int i;

	SendMessage((HWND)TabCtrl_GetlParam(GetDlgItem(hDlg, IDC_TAB), 0), WM_CLOSE, 0, 0);
	SendMessage((HWND)TabCtrl_GetlParam(GetDlgItem(hDlg, IDC_TAB), 1), WM_CLOSE, 0, 0);

	hIcon = (HICON)SendMessage(hDlg, WM_GETICON, TRUE, 0);
	hIconS = (HICON)SendMessage(hDlg, WM_GETICON, FALSE, 0);
	DestroyIcon(hIcon);
	DestroyIcon(hIconS);

	ListView_DeleteAllItems(GetDlgItem(hDlg, WWWC_LIST));

	//���X�g�r���[�̃J�����̃T�C�Y���擾
	for(i = 0;i < LvFindColCnt;i++){
		LvFindColSize[i] = ListView_GetColumnWidth(GetDlgItem(hDlg, WWWC_LIST), i);
	}

	FreeFindItemList();
	DestroyWindow(hDlg);
	FindWnd = NULL;
}


/******************************************************************************

	FindNameProc

	����������̐ݒ�^�u

******************************************************************************/

static BOOL CALLBACK FindNameProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HWND hCombo;
	struct TPFINDINFO *FindInfo;
	RECT WindowRect;
	static char FindStrList[FIND_HISTORY][BUFSIZE];
	char buf[BUFSIZE];
	char tmp[BUFSIZE];
	int Pos;
	int Size;
	int i;

	switch (uMsg)
	{
	case WM_INITDIALOG:
		FindInfo = (struct TPFINDINFO *)lParam;
		for(i = 0; i < FIND_HISTORY; i++){
			if(*(FindStrList[i]) == '\0'){
				continue;
			}
			SendMessage(GetDlgItem(hDlg, IDC_COMBO_FINDSTRING), CB_ADDSTRING, 0, (LPARAM)FindStrList[i]);
		}

		TreeView_GetItemInfo(GetDlgItem(WWWCWnd, WWWC_TREE), RootItem, buf);
		wsprintf(tmp, "\\\\%s", buf);
		TreeView_GetPath(GetDlgItem(WWWCWnd, WWWC_TREE),
			FindInfo->hItem, buf, tmp);
		SendMessage(GetDlgItem(hDlg, IDC_EDIT_FINDLOCATE), WM_SETTEXT, 0, (LPARAM)buf);
		CheckDlgButton(hDlg, IDC_CHECK_FINDSUBFOLDER, FindInfo->FindSub);
		CheckDlgButton(hDlg, IDC_CHECK_FINDNOCHECK, FindInfo->FindNoCheck);
		break;

	case WM_SIZE:
		GetWindowRect(GetDlgItem(hDlg, IDC_EDIT_FINDLOCATE), (LPRECT)&WindowRect);
		Pos = WindowRect.top;
		Size = WindowRect.bottom - WindowRect.top;

		GetWindowRect(hDlg, (LPRECT)&WindowRect);
		Pos -= WindowRect.top;

		GetClientRect(hDlg, (LPRECT)&WindowRect);

		//���͈�
		SetWindowPos(GetDlgItem(hDlg, IDC_COMBO_FINDSTRING), 0,
			0, 0, WindowRect.right - 20, Size, SWP_NOMOVE | SWP_NOZORDER);
		SetWindowPos(GetDlgItem(hDlg, IDC_EDIT_FINDLOCATE), 0,
			0, 0, WindowRect.right - 20 - 60, Size, SWP_NOMOVE | SWP_NOZORDER);

		//�Q�ƃ{�^��
		SetWindowPos(GetDlgItem(hDlg, IDC_BUTTON_SELECT), 0,
			WindowRect.right - 10 - 60, Pos - 1, 60, Size + 2, SWP_NOZORDER);
		break;

	case WM_CLOSE:
		for(i = 0; i < FIND_HISTORY; i++){
			SendMessage(GetDlgItem(hDlg, IDC_COMBO_FINDSTRING), CB_GETLBTEXT, i, (LPARAM)FindStrList[i]);
		}
		DestroyWindow(hDlg);
		break;

	case WM_SETDLGFOCUS:
		SetFocus(GetDlgItem(hDlg, IDC_COMBO_FINDSTRING));
		break;

	case WM_SETFINDINFO:
		FindInfo = (struct TPFINDINFO *)lParam;
		//�R���{�{�b�N�X�̐ݒ�
		hCombo = GetDlgItem(hDlg, IDC_COMBO_FINDSTRING);
		if(SendMessage(hCombo, CB_GETDROPPEDSTATE, 0, 0) == TRUE){
			if((i = SendMessage(hCombo, CB_GETCURSEL, 0, 0)) > -1){
				SendMessage(hCombo, CB_SETCURSEL, i , 0);
			}
			SendMessage(hCombo, CB_SHOWDROPDOWN, FALSE, 0);
		}
		SendMessage(hCombo, WM_GETTEXT, BUFSIZE - 3, (LPARAM)buf);
		if(*buf != '\0'){
			if((i = SendMessage(hCombo, CB_FINDSTRING, -1, (LPARAM)buf)) > -1){
				SendMessage(hCombo, CB_DELETESTRING, i, 0);
			}
			SendMessage(hCombo, CB_INSERTSTRING, 0, (LPARAM)buf);
			SendMessage(hCombo, CB_SETCURSEL, 0, 0);
		}
		//����������
		wsprintf(FindInfo->FindString, "*%s*", buf);

		//�p�X
		SendMessage(GetDlgItem(hDlg, IDC_EDIT_FINDLOCATE), WM_GETTEXT, BUFSIZE - 1, (LPARAM)buf);
		FindInfo->hItem = TreeView_FindItemPath(GetDlgItem(WWWCWnd, WWWC_TREE), RootItem, buf);
		if(FindInfo->hItem == NULL){
			FindInfo->hItem = RootItem;
		}
		FindInfo->FindSub = IsDlgButtonChecked(hDlg, IDC_CHECK_FINDSUBFOLDER);
		FindInfo->FindNoCheck = IsDlgButtonChecked(hDlg, IDC_CHECK_FINDNOCHECK);
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_BUTTON_SELECT:
			//�p�X�̑I��
			SendMessage(GetDlgItem(hDlg, IDC_EDIT_FINDLOCATE), WM_GETTEXT, BUFSIZE - 1, (LPARAM)buf);
			if(SelectFolder(hDlg, buf) == TRUE){
				SendMessage(GetDlgItem(hDlg, IDC_EDIT_FINDLOCATE), WM_SETTEXT, 0, (LPARAM)buf);
			}
			break;
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}


/******************************************************************************

	FindItemProc

	�������ڂ̐ݒ�^�u

******************************************************************************/

static BOOL CALLBACK FindItemProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	struct TPFINDINFO *FindInfo;

	switch (uMsg)
	{
	case WM_INITDIALOG:
		FindInfo = (struct TPFINDINFO *)lParam;
		CheckDlgButton(hDlg, IDC_CHECK_NAME, (FindInfo->ItemFlag & 1));
		CheckDlgButton(hDlg, IDC_CHECK_CHECKURL, (FindInfo->ItemFlag & 2));
		CheckDlgButton(hDlg, IDC_CHECK_OPENURL, (FindInfo->ItemFlag & 4));
		CheckDlgButton(hDlg, IDC_CHECK_DATE, (FindInfo->ItemFlag & 8));
		CheckDlgButton(hDlg, IDC_CHECK_COMMENT, (FindInfo->ItemFlag & 16));

		CheckDlgButton(hDlg, IDC_CHECK_ICON, (FindInfo->IconFlag & 1));
		CheckDlgButton(hDlg, IDC_CHECK_UPICON, (FindInfo->IconFlag & 2));
		CheckDlgButton(hDlg, IDC_CHECK_ERRICON, (FindInfo->IconFlag & 4));
		CheckDlgButton(hDlg, IDC_CHECK_TIMEICON, (FindInfo->IconFlag & 8));
		CheckDlgButton(hDlg, IDC_CHECK_NOCHECKICON, (FindInfo->IconFlag & 16));
		break;

	case WM_CLOSE:
		DestroyWindow(hDlg);
		break;

	case WM_SETDLGFOCUS:
		SetFocus(GetDlgItem(hDlg, IDC_CHECK_NAME));
		break;

	case WM_SETFINDINFO:
		FindInfo = (struct TPFINDINFO *)lParam;
		FindInfo->ItemFlag = 0;
		if(IsDlgButtonChecked(hDlg, IDC_CHECK_NAME) == 1){
			FindInfo->ItemFlag |= 1;
		}
		if(IsDlgButtonChecked(hDlg, IDC_CHECK_CHECKURL) == 1){
			FindInfo->ItemFlag |= 2;
		}
		if(IsDlgButtonChecked(hDlg, IDC_CHECK_OPENURL) == 1){
			FindInfo->ItemFlag |= 4;
		}
		if(IsDlgButtonChecked(hDlg, IDC_CHECK_DATE) == 1){
			FindInfo->ItemFlag |= 8;
		}
		if(IsDlgButtonChecked(hDlg, IDC_CHECK_COMMENT) == 1){
			FindInfo->ItemFlag |= 16;
		}

		FindInfo->IconFlag = 0;
		if(IsDlgButtonChecked(hDlg, IDC_CHECK_ICON) == 1){
			FindInfo->IconFlag |= 1;
		}
		if(IsDlgButtonChecked(hDlg, IDC_CHECK_UPICON) == 1){
			FindInfo->IconFlag |= 2;
		}
		if(IsDlgButtonChecked(hDlg, IDC_CHECK_ERRICON) == 1){
			FindInfo->IconFlag |= 4;
		}
		if(IsDlgButtonChecked(hDlg, IDC_CHECK_TIMEICON) == 1){
			FindInfo->IconFlag |= 8;
		}
		if(IsDlgButtonChecked(hDlg, IDC_CHECK_NOCHECKICON) == 1){
			FindInfo->IconFlag |= 16;
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}


/******************************************************************************

	Tab_NotifyProc

	�^�u�R���g���[�����b�Z�[�W

******************************************************************************/

static LRESULT Tab_NotifyProc(HWND hWnd, LPARAM lParam, WPARAM wParam)
{
	NMHDR *CForm = (NMHDR *)lParam;
	HWND hDlg;
	RECT tiRect;

	switch(CForm->code)
	{
	case TCN_SELCHANGING:
		if(TabCtrl_GetlParam(GetDlgItem(hWnd, IDC_TAB), TabCtrl_GetCurSel(GetDlgItem(hWnd, IDC_TAB))) != 0){
			ShowWindow((HWND)TabCtrl_GetlParam(GetDlgItem(hWnd, IDC_TAB), TabCtrl_GetCurSel(GetDlgItem(hWnd, IDC_TAB))), SW_HIDE);
		}
		return TRUE;

	case TCN_SELCHANGE:
		hDlg = (HWND)TabCtrl_GetlParam(GetDlgItem(hWnd, IDC_TAB), TabCtrl_GetCurSel(GetDlgItem(hWnd, IDC_TAB)));
		GetClientRect(GetDlgItem(hWnd, IDC_TAB), &tiRect);
		TabCtrl_AdjustRect(GetDlgItem(hWnd, IDC_TAB), FALSE, &tiRect);
		MoveWindow(hDlg, tiRect.left + 8, tiRect.top + 8, tiRect.right - tiRect.left, tiRect.bottom - tiRect.top, TRUE);
		ShowWindow(hDlg, SW_SHOW);
		SendMessage(hDlg, WM_SETDLGFOCUS, 0, 0);
		break;
	}

	return 0;
}


/******************************************************************************

	FindListViewHeaderNotifyProc

	���X�g�r���[�̃w�b�_���b�Z�[�W

******************************************************************************/

static LRESULT FindListViewHeaderNotifyProc(HWND hWnd, LPARAM lParam, struct TPLVCOLUMN *ColInfo)
{
	HD_NOTIFY *phd = (HD_NOTIFY *)lParam;
	static int LvFindSortFlag = 1;

	switch(phd->hdr.code)
	{
	case HDN_ITEMCLICK:			//�w�b�_���N���b�N���ꂽ�̂Ń\�[�g���s��
		WaitCursor(TRUE);

		//�\�[�g
		LvFindSortFlag = (ABS(LvFindSortFlag) == (phd->iItem + 1))
			? (LvFindSortFlag * -1) : (phd->iItem + 1);
		SortColInfo = ColInfo;
		ListView_SortItems(GetDlgItem(hWnd, WWWC_LIST), CompareFunc, LvFindSortFlag);

		//�A�C�e�����̍Đݒ�
		GlobalFree(FindItemList);
		FindItemListCnt = 0;
		FindItemList = ListView_SetListToMem(hWnd, &FindItemListCnt);

		WaitCursor(FALSE);
		break;
	}
	return FALSE;
}


/******************************************************************************

	ListView_FindGetDispItem

	���X�g�r���[�̃A�C�e���\���v��

******************************************************************************/

static BOOL ListView_FindGetDispItem(HWND hWnd, LV_ITEM *lvItem)
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
		if((FindColumnInfo + lvItem->iSubItem)->p == 0){
			//�A�C�e�����i�[����Ă���t�H���_�̃p�X
			if(IsTreeItem(WWWCWnd, tpItemInfo->hItem) == TRUE){
				TreeView_GetItemInfo(GetDlgItem(WWWCWnd, WWWC_TREE), RootItem, tmp1);
				wsprintf(tmp2, "\\\\%s", tmp1);
				TreeView_GetPath(GetDlgItem(WWWCWnd, WWWC_TREE), tpItemInfo->hItem, lvItem->pszText, tmp2);
			}

		}else if((char *)*((long *)((long)tpItemInfo + (FindColumnInfo + lvItem->iSubItem)->p)) != NULL){
			//�A�C�e�����ƃJ������񂩂�e�L�X�g��ݒ�
			lstrcpyn(lvItem->pszText,
				(char *)*((long *)((long)tpItemInfo + (FindColumnInfo + lvItem->iSubItem)->p)), BUFSIZE - 1);

		}else{
			*(lvItem->pszText) = '\0';
		}
	}

	if((lvItem->mask & LVIF_IMAGE) == 0){
		return FALSE;
	}

	//�A�C�R�����ݒ�
	i = GetProtocolIndex(tpItemInfo->CheckURL);
	if(tpItemInfo->Status & ST_ERROR){
		lvItem->iImage = ICON_ERR;

	}else if(tpItemInfo->Status & ST_TIMEOUT){
		lvItem->iImage = ICON_TIMEOUT;

	}else if(tpItemInfo->Status & ST_UP){
		lvItem->iImage = (i == -1) ? ICON_UP : (tpProtocol + i)->Icon + 3;

	}else{
		lvItem->iImage = (i == -1) ? ICON_NOICON : (tpProtocol + i)->Icon;
	}
	return FALSE;
}


/******************************************************************************

	FindProc

	������ʂ̃E�B���h�E�v���V�[�W��

******************************************************************************/

BOOL CALLBACK FindProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	RECT WindowRect;
	char buf[BUFSIZE];
	int i;

	switch (uMsg)
	{
	case WM_INITDIALOG:
		*tpFindInfo.FindString = '\0';
		tpFindInfo.hItem = (HTREEITEM)lParam;
		tpFindInfo.FindSub = FindSubFolder;
		tpFindInfo.FindNoCheck = FindNoCheck;
		tpFindInfo.ItemFlag = FindItemFlag;
		tpFindInfo.IconFlag = FindIconFlag;

		InitializeFindWindow(hDlg);
		break;

	case WM_CLOSE:
		CloseFindWindow(hDlg);
		break;

	case WM_SIZE:
		SizeFindWindow(hDlg);
		break;

	case WM_EXITSIZEMOVE:
		if(IsWindowVisible(hDlg) != 0 && IsIconic(hDlg) == 0 && IsZoomed(hDlg) == 0){
			//�E�B���h�E�̈ʒu��ۑ�
			GetWindowRect(hDlg, (LPRECT)&WindowRect);
			FindWinLeft = WindowRect.left;
			FindWinTop = WindowRect.top;
			FindWinRight = WindowRect.right - WindowRect.left;
			FindWinBottom = WindowRect.bottom - WindowRect.top;
		}
		break;

	case WM_TIMER:
		switch(wParam)
		{
		case TIMER_TABVIEW:
			KillTimer(hDlg, wParam);
			SendMessage((HWND)TabCtrl_GetlParam(GetDlgItem(hDlg, IDC_TAB), 0), WM_SETDLGFOCUS, 0, 0);
			break;

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

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			FindStart(hDlg);
			break;

		case ID_KEY_ESC:
		case IDCANCEL:
			SendMessage(hDlg, WM_CLOSE, 0, 0);
			break;

		case ID_BUTTON_SAVEOPTION:
			SendMessage((HWND)TabCtrl_GetlParam(GetDlgItem(hDlg, IDC_TAB), 0),
				WM_SETFINDINFO, 0, (LPARAM)&tpFindInfo);
			SendMessage((HWND)TabCtrl_GetlParam(GetDlgItem(hDlg, IDC_TAB), 1),
				WM_SETFINDINFO, 0, (LPARAM)&tpFindInfo);
			FindSubFolder = tpFindInfo.FindSub;
			FindNoCheck = tpFindInfo.FindNoCheck;
			FindItemFlag = tpFindInfo.ItemFlag;
			FindIconFlag = tpFindInfo.IconFlag;
			break;

		case ID_KEY_RETURN:
			if(GetFocus() != GetDlgItem(hDlg, WWWC_LIST)){
				//�t�H�[�J�X�̎��{�^����I��
				if(GetDlgCtrlID(GetFocus()) == IDCANCEL ||
					GetDlgCtrlID(GetFocus()) == ID_BUTTON_SAVEOPTION){
					SendMessage(hDlg, WM_COMMAND, GetDlgCtrlID(GetFocus()), 0);
				}else if(GetDlgCtrlID(GetFocus()) == IDC_BUTTON_SELECT){
					SendMessage((HWND)TabCtrl_GetlParam(GetDlgItem(hDlg, IDC_TAB), 0),
						WM_COMMAND, IDC_BUTTON_SELECT, 0);
				}else{
					SendMessage(hDlg, WM_COMMAND, IDOK, 0);
				}
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
			DeleteProtocolMenuItem(GetSubMenu(hPOPUP, MENU_POP_UPITEM));
			if(ListView_GetSelectedCount(GetDlgItem(hDlg, WWWC_LIST)) <= 0){
				EnableMenuItem(GetSubMenu(hPOPUP, MENU_POP_UPITEM), ID_MENUITEM_UPVIEW, MF_GRAYED);
				EnableMenuItem(GetSubMenu(hPOPUP, MENU_POP_UPITEM), ID_MENUITEM_COPY, MF_GRAYED);
				EnableMenuItem(GetSubMenu(hPOPUP, MENU_POP_UPITEM), ID_MENUITEM_INITICON_POP, MF_GRAYED);
				EnableMenuItem(GetSubMenu(hPOPUP, MENU_POP_UPITEM), ID_MENUITEM_PROP, MF_GRAYED);
				ShowMenu(hDlg, hPOPUP, MENU_POP_UPITEM);
				break;
			}
			EnableMenuItem(GetSubMenu(hPOPUP, MENU_POP_UPITEM), ID_MENUITEM_UPVIEW, MF_ENABLED);
			EnableMenuItem(GetSubMenu(hPOPUP, MENU_POP_UPITEM), ID_MENUITEM_COPY, MF_ENABLED);
			EnableMenuItem(GetSubMenu(hPOPUP, MENU_POP_UPITEM), ID_MENUITEM_INITICON_POP, MF_ENABLED);
			EnableMenuItem(GetSubMenu(hPOPUP, MENU_POP_UPITEM), ID_MENUITEM_PROP, MF_ENABLED);
			//�A�C�e�����j���[�̕\��
			SetProtocolItemMenu(hDlg, GetSubMenu(hPOPUP, MENU_POP_UPITEM), TRUE, TRUE);
			SetEnableToolItemMenu(GetSubMenu(hPOPUP, MENU_POP_UPITEM));
			ShowMenu(hDlg, hPOPUP, MENU_POP_UPITEM);
			break;

		case ID_KEY_TABCHANGE:
			TabCtrl_SetCurFocus(GetDlgItem(hDlg, IDC_TAB),
				(TabCtrl_GetCurSel(GetDlgItem(hDlg, IDC_TAB)) == 0) ? 1 : 0);
			break;

		//�J��
		case ID_MENU_ACTION_OPEM:
			Item_Open(hDlg, -1);
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

		//�S�đI��
		case ID_MENUITEM_ALLSELECT:
			//�t�H�[�J�X�̎��{�^����I��
			GetClassName(GetFocus(), buf, BUFSIZE - 1);
			if(lstrcmpi(buf, "edit") == 0){
				SendMessage(GetFocus(), EM_SETSEL, 0, -1);
			}else{
				SetFocus(GetDlgItem(hDlg, WWWC_LIST));
				ListView_SetItemState(GetDlgItem(hDlg, WWWC_LIST), -1, LVIS_SELECTED, LVIS_SELECTED);
			}
			break;

		//�X�V�A�C�e����I��
		case ID_MENUITEM_UPSELECT:
			SetFocus(GetDlgItem(hDlg, WWWC_LIST));
			ListView_UpSelectItem(GetDlgItem(hDlg, WWWC_LIST));
			break;

		//�S�ẴA�C�R����������
		case ID_MENUITEM_ALLINITICON:
			WaitCursor(TRUE);
			for(i = 0;i < ListView_GetItemCount(GetDlgItem(hDlg, WWWC_LIST));i++){
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

		//�v���p�e�B
		case ID_MENUITEM_PROP:
			MainItemProp(hDlg);
			break;

		default:
			ExeAction(hDlg, wParam);
			break;
		}
		break;

	case WM_NOTIFY:
		if(((NMHDR *)lParam)->hwndFrom == GetDlgItem(hDlg, IDC_TAB)){
			return Tab_NotifyProc(hDlg, lParam, wParam);
		}
		//���X�g�r���[�w�b�_�R���g���[��
		if(((NMHDR *)lParam)->hwndFrom == GetWindow(GetDlgItem(hDlg, WWWC_LIST), GW_CHILD)){
			return FindListViewHeaderNotifyProc(hDlg, lParam, FindColumnInfo);
		}
		//���X�g�r���[
		if(((NMHDR *)lParam)->hwndFrom != GetDlgItem(hDlg, WWWC_LIST)){
			return FALSE;
		}
		return ListView_NotifyProc(hDlg, lParam);

	//���X�g�r���[�̃C�x���g
	case WM_LV_EVENT:
		switch(wParam)
		{
		case LVN_GETDISPINFO:
			return ListView_FindGetDispItem(hDlg, &(((LV_DISPINFO *)lParam)->item));

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
