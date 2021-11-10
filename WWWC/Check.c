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

//�O���Q��
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

	�z�X�g���n���h������A�C�e��������

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

	�\�P�b�g����A�C�e��������

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

	�`�F�b�N�A�C�e����񂩂�A�C�e��������

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

	���X�g�̃`�F�b�N���̎擾

******************************************************************************/

static int ListCheckCount(HWND hWnd)
{
	struct TPTREE *tpTreeInfo;
	struct TPITEM *tpItemInfo;
	HTREEITEM hItem;
	int cnt = 0;
	int i;

	//�c���[�����擾
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

	�`�F�b�N����A�C�e�����̎擾

******************************************************************************/

static int TreeCheckCount(HWND hWnd, HTREEITEM hItem, int CheckFlag)
{
	struct TPTREE *tpTreeInfo;
	int cnt = 0;
	int i;

	//�c���[�����擾
	if((tpTreeInfo = (struct TPTREE *)TreeView_GetlParam(GetDlgItem(hWnd, WWWC_TREE), hItem)) == NULL ||
		(CheckFlag != CHECKINI_TREECHECK && TreeView_IsRecyclerItem(GetDlgItem(hWnd, WWWC_TREE), hItem) == TRUE) ||	//���ݔ�
		(CheckFlag != CHECKINI_CHECK && tpTreeInfo->CheckSt == 1)){
		return 0;
	}

	//�A�C�e�����X�g�̓ǂݍ���
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

	�A�C�e���̃`�F�b�N�̏�����

******************************************************************************/

void ItemCheckIni(HWND hWnd, struct TPITEM *tpItemInfo, BOOL no_err)
{
	char buf[BUFSIZE];
	int ProtocolIndex;
	int ret;

	if(tpItemInfo == NULL || tpItemInfo->IconStatus != ST_DEFAULT){
		return;
	}

	//�G���[�A�C�e���̂݃`�F�b�N�̏ꍇ
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
	//�A�C�e���̏�����
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

	���X�g�̃A�C�e���̏�����

******************************************************************************/

static void ListCheckIni(HWND hWnd, BOOL TreeFlag)
{
	struct TPTREE *tpTreeInfo;
	struct TPITEM *tpItemInfo;
	HTREEITEM hItem;
	int i;

	//�c���[�����擾
	tpTreeInfo = (struct TPTREE *)TreeView_GetlParam(GetDlgItem(hWnd, WWWC_TREE),
		TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)));
	if(tpTreeInfo == NULL || tpTreeInfo->ItemList == NULL){
		return;
	}

	//�t�H���_�Ƀ`�F�b�N���t���O���Z�b�g
	tpTreeInfo->CheckFlag = 1;

	//���X�g�r���[�̑I�����ڂ̃`�F�b�N
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

	�t�H���_�̃`�F�b�N�̏�����

******************************************************************************/

void FolderCheckIni(HWND hWnd, HTREEITEM hItem)
{
	struct TPTREE *tpTreeInfo;
	int i;

	//�c���[�����擾
	tpTreeInfo = (struct TPTREE *)TreeView_GetlParam(GetDlgItem(hWnd, WWWC_TREE), hItem);
	if(tpTreeInfo == NULL ||
		tpTreeInfo->CheckFlag != 1 ||
		tpTreeInfo->ItemList == NULL){
		return;
	}

	//�t�H���_���̑S�ẴA�C�e�����`�F�b�N
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

	�c���[�̃A�C�e���̏�����

******************************************************************************/

static void CALLBACK TreeCheckIni(HWND hWnd, HTREEITEM hItem, long Param)
{
	struct TPTREE *tpTreeInfo;

	//�c���[�����擾
	if((tpTreeInfo = (struct TPTREE *)TreeView_GetlParam(GetDlgItem(hWnd, WWWC_TREE), hItem)) == NULL ||
		(Param != CHECKINI_TREECHECK && TreeView_IsRecyclerItem(GetDlgItem(hWnd, WWWC_TREE), hItem) == TRUE) ||	//���ݔ�
		(Param != CHECKINI_CHECK && tpTreeInfo->CheckSt == 1) ||				//�`�F�b�N���Ȃ��ݒ�̃t�H���_
		(Param == CHECKINI_AUTOALLCHECK && tpTreeInfo->AutoCheckSt == 0)){		//�����`�F�b�N�ł��f�t�H���g�̃`�F�b�N�^�C�����g�p����
		return;
	}

	//�t�H���_�Ƀ`�F�b�N���t���O���Z�b�g
	tpTreeInfo->CheckFlag = 1;

	//�A�C�e�������݂��Ȃ��ꍇ�͔�����
	if(tpTreeInfo->ItemList == NULL){
		return;
	}

	//�t�H���_���̑S�ẴA�C�e�����`�F�b�N
	FolderCheckIni(hWnd, hItem);
}


/******************************************************************************

	CheckIniProc

	�`�F�b�N�̏�����

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

	//�`�F�b�N�\�ȃA�C�e�������擾
	switch(CheckFlag)
	{
	case CHECKINI_CHECK:
		//�P�̃`�F�b�N
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
		//�����`�F�b�N
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
		//�`�F�b�N����A�C�e������
		if(hItem == NULL){
			TreeView_FreeItem(hWnd, hItem, 1);
		}
		return FALSE;
	}

	gCheckType |= CheckType;

	if(gCheckFlag != 1){
		//�t�H���_�̓��e��ۑ�
		SendMessage(hWnd, WM_FOLDER_SAVE, 0, 0);

		//�`�F�b�N�J�n���Ɏ��s����c�[�������s����
		for(i = 0;i < ToolListCnt;i++){
			if((ToolList[i].Action & TOOL_EXEC_CHECKSTART) != 0){
				if(str_match("*.dll", ToolList[i].FileName) == TRUE){
					//�c�[���� -1 ��Ԃ����ꍇ�̓`�F�b�N���s��Ȃ�
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
			//�t�H���_�̓��e��ǂݒ���
			SendMessage(hWnd, WM_FOLDER_LOAD, 0, 0);
		}
	}

	//�`�F�b�N�̃^�C�v�ɂ�菉�����̕��@��ς���
	switch(CheckFlag)
	{
	case CHECKINI_CHECK:
		//�P�̃`�F�b�N
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
		//�����`�F�b�N
		TreeCheckIni(hWnd, hItem, CheckFlag);
		break;

	case CHECKINI_AUTOALLCHECK:
	case CHECKINI_ALLCHECK:
		//���ׂẴA�C�e�����`�F�b�N
		CallTreeItem(hWnd, RootItem, (FARPROC)TreeCheckIni, CheckFlag);
		break;

	case CHECKINI_TREECHECK:
		//�K�w�`�F�b�N
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

	//�`�F�b�N�A�C�e�����X�g�̊m��
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
	//�X�V�A�C�e�����X�g�̊m��
	if(UpItemList == NULL){
		UpItemList = (struct TPITEM **)GlobalAlloc(GPTR, sizeof(struct TPITEM *) * 0);
		if(UpItemList == NULL){
			ErrMsg(hWnd, GetLastError(), NULL);
			return FALSE;
		}
		UpItemListCnt = 0;
	}

	//�E�B���h�E�^�C�g���̐ݒ�
	GetDateTime(fDay, fTime);
	wsprintf(WinTitle, TITLE_TIMEFORMAT, fDay, fTime);
	SetWinTitle(hWnd);

	//�^�X�N�g���C�̐ݒ�
	GetWindowText(hWnd, buf, BUFSIZE - 1);
	if(TrayIcon == 1 && TrayMessage(hWnd, NIM_MODIFY, TRAY_ID, TrayIcon_Chaeck, buf) == FALSE){
		TrayMessage(hWnd, NIM_ADD, TRAY_ID, TrayIcon_Chaeck, buf);
	}
	SetClassLong(hWnd, GCL_HICON, (long)TrayIcon_Chaeck_Win);

	//���Ƀ`�F�b�N���̏ꍇ
	if(gCheckFlag == 1){
		SetTimer(hWnd, TIMER_CHECK, NEXTCHECK_INTERVAL, NULL);		//�A�C�e���`�F�b�N�p�^�C�}�[
		return TRUE;
	}

	//�`�F�b�N�J�n���X�V�A�C�e�����������ݒ�
	if(ClearTime == 0){
		switch(CheckUPItemClear)
		{
		//������
		case 0:
			SendMessage(UpWnd, WM_UP_FREE, 0, 0);
			break;

		//�E�B���h�E���\����Ԃ̂Ƃ�
		case 1:
			if(IsWindowVisible(UpWnd) == 0){
				SendMessage(UpWnd, WM_UP_FREE, 0, 0);
			}
			break;

		//��ɂ��Ȃ�
		case 2:
			break;
		}
	}

	//�`�F�b�N���t���O�̃Z�b�g
	gCheckFlag = 1;

	//���j���[�̏�Ԃ�ݒ�
	SetEnableMenu(hWnd);
	SetEnableToolMenu(hWnd);
	SetEnableToolItemMenu(GetSubMenu(hPOPUP, MENU_POP_ITEM));

	//�X�e�[�^�X�o�[�̐ݒ�
	SendDlgItemMessage(hWnd, WWWC_SB, SB_SETTEXT, (WPARAM)0 | 0, (LPARAM)"�`�F�b�N���J�n");
	//�X�e�[�^�X�o�[�ɃA�C�R����ݒ�
	SendDlgItemMessage(hWnd, WWWC_SB, WM_USER + 15, (WPARAM)0 | 0, (LPARAM)StCheckIcon);

	//�`�F�b�N�p�^�C�}�[�̊J�n
	SetTimer(AniWnd, TIMER_ANI, CHECKANI_INTERVAL, NULL);		//�A�j���[�V�����p�^�C�}�[
	SetTimer(hWnd, TIMER_CHECK, NEXTCHECK_INTERVAL, NULL);		//�A�C�e���`�F�b�N�p�^�C�}�[
	SetTimer(hWnd, TIMER_CHECKTIMEOUT, TIMEOUT_INTERVAL, NULL);	//�^�C���A�E�g�p�^�C�}�[
	return TRUE;
}


/******************************************************************************

	FindCheckTree

	�`�F�b�N���̃A�C�e������������

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

	//�`�F�b�N���̃A�C�e���̌���
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

	�`�F�b�N�����`�F�b�N�ҋ@���̃A�C�e������������

******************************************************************************/

static void CALLBACK FindCheckNoCheckTree(HWND hWnd, HTREEITEM hItem, long Param)
{
	struct TPTREE *tpTreeInfo;
	int *ret = (int *)Param;

	//���Ƀ`�F�b�N�����`�F�b�N�ҋ@���̃A�C�e�������t�����Ă���ꍇ
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

	�`�F�b�N�J�n���Ԃ̎擾

******************************************************************************/

static void SetCheckDateTime(struct TPITEM *tpItemInfo)
{
	char fDay[BUFSIZE];
	char fTime[BUFSIZE];

	//���݂̓������擾����
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

	�A�C�e���̃`�F�b�N�̊J�n

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

	//�v���g�R��DLL�̃`�F�b�N�J�n�̊֐����Ă�
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
	//�G���[
	case CHECK_ERROR:
		tpItemInfo->Status = (tpItemInfo->Status & ST_UP) ? ST_UP | ST_ERROR : ST_ERROR;
		tpItemInfo->IconStatus = ST_DEFAULT;
		TreeView_SetIconState(hWnd, tpItemInfo->hItem, 0);
		NotifyItemCheckEnd(hWnd, tpItemInfo);
		return -1;

	//�`�F�b�N�J�n
	case CHECK_SUCCEED:
		tpItemInfo->IconStatus = ST_CHECK;
		*(CheckItemList + CheckListIndex) = tpItemInfo;
		break;

	//�`�F�b�N�҂�
	case CHECK_NO:
		tpItemInfo->IconStatus = ST_NOCHECK;
		break;

	//�`�F�b�N�I��
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

	�t�H���_���Ƃ̃`�F�b�N�̊J�n

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

	//�A�C�e�����X�g�̓ǂݍ���
	if(tpTreeInfo->ItemList == NULL){
		WaitCursor(TRUE);
		ReadTreeMem(hWnd, hItem);
		WaitCursor(FALSE);
	}
	if(tpTreeInfo->ItemList == NULL){
		tpTreeInfo->CheckFlag = 0;
		return;
	}

	//�S�ẴA�C�e���ɑ΂��ă`�F�b�N�J�n�������s��
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
			//�`�F�b�N�J�n
			if(CheckStartItem(hWnd, (*(tpTreeInfo->ItemList + i)), ProtocolIndex, Param) == -1){
				continue;
			}
			CheckFlag = TRUE;
			if(*(CheckItemList + Param) != NULL){
				break;
			}
		}
	}
	//�`�F�b�N�\�A�C�e�������݂��Ȃ������ꍇ�̓t�H���_�̃`�F�b�N���I������
	if(CheckFlag == FALSE){
		tpTreeInfo->CheckFlag = 0;
		TreeView_FreeItem(hWnd, hItem, 1);
		SetEnableMenu(hWnd);
	}
}


/******************************************************************************

	CheckProc

	�`�F�b�N����

******************************************************************************/

int CheckProc(HWND hWnd)
{
	int j, ret;
	int ListIndex;

	if(CheckItemList == NULL){
		return 0;
	}
	//�`�F�b�N���X�g�̋󂫂�����
	for(j = 0;j < CheckMax;j++){
		if(*(CheckItemList + j) == NULL){
			break;
		}
	}
	if(j >= CheckMax){
		KillTimer(hWnd, TIMER_CHECK);
		return 0;
	}

	//�`�F�b�N����A�C�e�����������ă`�F�b�N���J�n
	CallTreeItem(hWnd, RootItem, (FARPROC)CheckStartFolder, (long)j);
	if(CheckItemList == NULL){
		return 0;
	}

	if(*(CheckItemList + j) == NULL){		//�`�F�b�N���ׂ��A�C�e�������������ꍇ
		KillTimer(hWnd, TIMER_CHECK);

		//���݃`�F�b�N���̃A�C�e��������
		for(j = 0;j < CheckMax;j++){
			if(*(CheckItemList + j) != NULL){
				break;
			}
		}
		if(j < CheckMax){
			return 0;
		}

		//�`�F�b�N�\�ȃt�H���_������
		ret = FindCheckNoCheckItem(hWnd, RootItem);
		if(ret != 0){
			return 0;
		}

		//�`�F�b�N�I��
		InvalidateRect(GetDlgItem(hWnd, WWWC_LIST), NULL, FALSE);
		UpdateWindow(GetDlgItem(hWnd, WWWC_LIST));

		WaitCursor(TRUE);
		CheckEndProc(hWnd);
		WaitCursor(FALSE);

	}else{
		//���ݑI�����Ă���t�H���_�̃A�C�e���̏ꍇ�̓A�C�e���̕\���X�V���s��
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

	�`�F�b�N���ʂɂ���Ԃ�ύX

******************************************************************************/

BOOL ResultCheckStatus(HWND hWnd, struct TPITEM *tpItemInfo, int ret)
{
	switch(ret)
	{
	case CHECK_ERROR:
		//�G���[
		tpItemInfo->Status = (tpItemInfo->Status & ST_UP) ? ST_UP | ST_ERROR : ST_ERROR;
		tpItemInfo->IconStatus = ST_DEFAULT;
		NotifyItemCheckEnd(hWnd, tpItemInfo);
		break;

	case CHECK_SUCCEED:
		//����E�ҋ@
		return TRUE;

	case CHECK_NO:
		//�`�F�b�N�҂�
		tpItemInfo->IconStatus = ST_NOCHECK;
		break;

	case CHECK_END:
		//�`�F�b�N�I��
	default:
		tpItemInfo->IconStatus = ST_DEFAULT;
		NotifyItemCheckEnd(hWnd, tpItemInfo);
		break;
	}
	return FALSE;
}


/******************************************************************************

	InitCheckItemList

	�A�C�e���`�F�b�N�I�����̃`�F�b�N�A�C�e�����X�g�̏�����

******************************************************************************/

void InitCheckItemList(HWND hWnd, struct TPITEM *tpItemInfo)
{
	int ListIndex;
	int i;

	//�c���[�r���[�ƃ��X�g�r���[�̕\���X�V
	TreeView_SetIconState(hWnd, tpItemInfo->hItem, 0);
	if(tpItemInfo->hItem == TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE))){
		ListIndex = ListView_GetMemToIndex(GetDlgItem(hWnd, WWWC_LIST), tpItemInfo);
		ListView_RedrawItems(GetDlgItem(hWnd, WWWC_LIST), ListIndex, ListIndex);
		UpdateWindow(GetDlgItem(hWnd, WWWC_LIST));
	}

	//�`�F�b�N�A�C�e�����X�g�̏�����
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

	//���̃`�F�b�N�A�C�e��
	SetTimer(hWnd, TIMER_CHECK, NEXTCHECK_INTERVAL, NULL);
}


/******************************************************************************

	GethostMsg

	�z�X�g���擾�C�x���g

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

	//�v���g�R��DLL�̃z�X�g���擾�֐����Ă�
	ProtocolIndex = GetProtocolIndex(tpItemInfo->CheckURL);
	if(ProtocolIndex == -1){
		tpItemInfo->Status = (tpItemInfo->Status & ST_UP) ? ST_UP | ST_ERROR : ST_ERROR;
		tpItemInfo->IconStatus = ST_DEFAULT;
		//�`�F�b�N�A�C�e�����X�g�̏�����
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
	//�`�F�b�N�A�C�e�����X�g�̏�����
	InitCheckItemList(hWnd, tpItemInfo);
	return 0;
}


/******************************************************************************

	SelectMsg

	Select�C�x���g

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

	//�v���g�R��DLL��Select�����֐����Ă�
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
	//�`�F�b�N�A�C�e�����X�g�̏�����
	InitCheckItemList(hWnd, tpItemInfo);
	return 0;
}


/******************************************************************************

	TimeoutItem

	�^�C���A�E�g�p�^�C�}�[����

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

	�A�C�e���̃L�����Z��

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

	//�v���g�R��DLL�̃`�F�b�N�̃L�����Z���֐����ďo��
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

	//�\������Ă���A�C�e���̏ꍇ�͕\���̍X�V���s��
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

	���X�g�A�C�e���̃L�����Z��

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

	//�I������Ă���A�C�e���̃L�����Z�����s��
	i = -1;
	while((i = ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), i, LVNI_SELECTED)) != -1){
		tpItemInfo = (struct TPITEM *)ListView_GetlParam(GetDlgItem(hWnd, WWWC_LIST), i);
		if(tpItemInfo != NULL){
			CancelItem(hWnd, tpItemInfo);
			tpItemInfo->IconStatus = ST_DEFAULT;

		}else{
			//�t�H���_�̏ꍇ�͍ċA����
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

	//�`�F�b�N���̃A�C�e������������
	for(i = 0;i < tpTreeInfo->ItemListCnt;i++){
		if((*(tpTreeInfo->ItemList + i)) == NULL){
			continue;
		}
		if((*(tpTreeInfo->ItemList + i))->IconStatus != ST_DEFAULT){
			break;
		}
	}
	if(i >= tpTreeInfo->ItemListCnt){
		//�`�F�b�N���̃A�C�e���������ꍇ
		tpTreeInfo->CheckFlag = 0;
		TreeView_SetIconState(hWnd, hItem, 0);
	}
}


/******************************************************************************

	TreeCancelItem

	�K�w�I�ɃA�C�e���̃L�����Z��

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

	//�A�C�e�����X�g�̒��̃A�C�e�����ׂĂɃL�����Z�����s��
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

	�A�C�e���̃`�F�b�N���ʒʒm

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
	//�X�V����
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

	�A�C�e���̃`�F�b�N�I���ʒm

******************************************************************************/

static void NotifyItemCheckEnd(HWND hWnd, struct TPITEM *tpItemInfo)
{
	char buf[BUFSIZE];
	int ProtocolIndex;

	//�`�F�b�N�I���ʒm
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

	�S�`�F�b�N�̏I��

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

	//���j���[�̏�Ԃ̐ݒ�
	SetEnableMenu(hWnd);
	SetEnableToolMenu(hWnd);
	SetEnableToolItemMenu(GetSubMenu(hPOPUP, MENU_POP_ITEM));

	//�X�e�[�^�X�o�[�̃A�C�R��������
	SendDlgItemMessage(hWnd, WWWC_SB, WM_USER + 15, (WPARAM)0 | 0, (LPARAM)NULL);
	SetSbText(hWnd);

	//���X�g�r���[���\�[�g
	if(LvAutoSort == 1 && LvCheckEndAutoSort == 1){
		tpTreeInfo = (struct TPTREE *)TreeView_GetlParam(GetDlgItem(hWnd, WWWC_TREE),
			TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)));
		if(tpTreeInfo != NULL){
			//�A�C�e�����\�[�g
			WaitCursor(TRUE);
			SortColInfo = ColumnInfo;
			ListView_SortItems(GetDlgItem(hWnd, WWWC_LIST), CompareFunc, LvSortFlag);

			GlobalFree(tpTreeInfo->ItemList);
			tpTreeInfo->ItemListCnt = 0;
			tpTreeInfo->ItemList = ListView_SetListToMem(hWnd, &tpTreeInfo->ItemListCnt);
			WaitCursor(FALSE);
		}
	}

	//�`�F�b�N�I�����Ɏ��s����c�[��
	for(i = 0;i < ToolListCnt;i++){
		if((ToolList[i].Action & TOOL_EXEC_CHECKEND) != 0 &&
			((UPcnt != 0 && (ToolList[i].Action & TOOL_EXEC_CHECKENDUP) != 0) ||
			(UPcnt == 0 && (ToolList[i].Action & TOOL_EXEC_CHECKENDNOUP) != 0) ||
			((ToolList[i].Action & TOOL_EXEC_CHECKENDUP) == 0 &&
			(ToolList[i].Action & TOOL_EXEC_CHECKENDNOUP) == 0))){

			//�w��v���g�R���̃A�C�e���𒊏o
			j = UpItemListCnt;
			ToolItemList = Item_ProtocolSelect(UpItemList, &j, ToolList[i].Protocol);
			if(ToolItemList != NULL){
				//�c�[���̎��s
				SubItemExecTool(hWnd, i, ToolItemList, j, TOOL_EXEC_CHECKEND, gCheckType);
				if(ToolList[i].Action & TOOL_EXEC_INITITEM){
					//�A�C�R���̏�����
					Item_MainItemIni(hWnd, hWnd, ToolItemList, j);
					InitFlag = TRUE;
				}
				GlobalFree(ToolItemList);
			}
		}
	}
	gCheckType = 0;

	//�c�[�����s��ɃA�C�R���̏��������s���ݒ�̏���
	WaitCursor(TRUE);
	//�A�C�e�����̉��
	CallTreeItem(hWnd, RootItem, (FARPROC)TreeView_FreeItem, 1);

	//�\���̍X�V
	if(InitFlag == TRUE){
		InvalidateRect(GetDlgItem(hWnd, WWWC_LIST), NULL, FALSE);
		UpdateWindow(GetDlgItem(hWnd, WWWC_LIST));
	}
	WaitCursor(FALSE);

	//�X�V�A�C�e������̏ꍇ
	if(NoUpMsg == 1 || UPcnt > 0){
		//�T�E���h�̍Đ�
		if(UPSnd == 1){
			if(*WaveFile == '\0' || sndPlaySound(WaveFile, SND_ASYNC | SND_NODEFAULT) == FALSE){
				MessageBeep(MB_ICONASTERISK);
			}
		}
		//�X�V���b�Z�[�W�E�B���h�E�̏�����
		if(CmdCheckEnd == FALSE || CmdNoUpCheckEnd == TRUE){
			SendMessage(UpWnd, WM_UP_INIT, 0, 0);
		}
	}

	//�A�C�R���̐ݒ�
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
