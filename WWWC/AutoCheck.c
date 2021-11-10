/**************************************************************************

	WWWC

	AutoCheck.c

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

#define HOU							60		//�ꎞ��

#define UD_DAY						MAKELONG((short)31, (short)1)
#define UD_HOU						MAKELONG((short)23, (short)0)
#define UD_MIN						MAKELONG((short)59, (short)0)
#define UD_EVERYHOU					MAKELONG((short)1024, (short)1)
#define UD_EVERYMIN					MAKELONG((short)1024, (short)1)

#define LISTCOL_SIZE				300


/**************************************************************************
	Global Variables
**************************************************************************/

struct TPCHECKTIME *tpCheckTime;
int tpCheckTimeCnt = 0;

//�O���Q��
extern HINSTANCE g_hinst;				//�A�v���P�[�V�����̃C���X�^���X�n���h��
extern HWND WWWCWnd;					//�{��
extern HTREEITEM PropItem;
extern int AutoCheck;
extern int StartCheck;
extern int NoMinuteCheck;


/**************************************************************************
	Local Function Prototypes
**************************************************************************/

static int CheckTime(struct TPCHECKTIME *tpTimeInfo);
static int GetAutoCheckString(struct TPCHECKTIME *tpTimeInfo, char *ret);
static void EnableWeekContorol(HWND hDlg, BOOL eFlag);
static void EnableCheckContorol(HWND hDlg, BOOL eFlag);
static void EnableAutoTimeList(HWND hDlg, BOOL eFlag);
static void EnableAutoTimeContorol(HWND hDlg);
static BOOL SetAutoCheckInfo(HWND hDlg);
static BOOL CALLBACK SetAutoCheckProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL InitializeAutoCheckListWindow(HWND hDlg);
static void SetAutoCheckList(HWND hDlg);
static void TreeCopyAutoCheckInfo(HWND hDlg);


/******************************************************************************

	CheckTime

	�����`�F�b�N���s�����`�F�b�N���s��

******************************************************************************/

static int CheckTime(struct TPCHECKTIME *tpTimeInfo)
{
	SYSTEMTIME SysTime;

	if(tpTimeInfo->flag == 1){
		return 0;
	}

	//���[�J���^�C���擾
	GetLocalTime(&SysTime);

	switch(tpTimeInfo->type)
	{
	//����
	case AUTOTIME_MIN:
		//���J�E���^
		tpTimeInfo->mCnt++;
		if(tpTimeInfo->mCnt >= tpTimeInfo->m){
			tpTimeInfo->mCnt = 0;
			return 1;
		}
		break;

	//���Ԗ�
	case AUTOTIME_HOU:
		//���J�E���^
		tpTimeInfo->mCnt++;
		if(tpTimeInfo->mCnt >= HOU){
			tpTimeInfo->mCnt = 0;
			//���J�E���^
			tpTimeInfo->hCnt++;
			if(tpTimeInfo->hCnt >= tpTimeInfo->h){
				tpTimeInfo->hCnt = 0;
				return 1;
			}
		}
		break;

	//���Ԏw��
	case AUTOTIME_DAY:
		if((tpTimeInfo->h == SysTime.wHour) && (tpTimeInfo->m == SysTime.wMinute)){
			return 1;
		}
		break;

	//�j���w��
	case AUTOTIME_WEEK:
		if((tpTimeInfo->week == SysTime.wDayOfWeek) &&
			(tpTimeInfo->h == SysTime.wHour) &&
			(tpTimeInfo->m == SysTime.wMinute)){
			return 1;
		}
		break;

	//���t�w��
	case AUTOTIME_MON:
		if((tpTimeInfo->day == SysTime.wDay) &&
			(tpTimeInfo->h == SysTime.wHour) &&
			(tpTimeInfo->m == SysTime.wMinute)){
			return 1;
		}
		break;

	}
	return 0;

}


/******************************************************************************

	CheckAutoTime

	�����`�F�b�N�̏����`�F�b�N

******************************************************************************/

BOOL CheckAutoTime(HWND hWnd)
{
	int i;

	//�����`�F�b�N�ݒ���`�F�b�N����
	for(i = 0;i < tpCheckTimeCnt;i++){
		if((tpCheckTime + i) == NULL || (tpCheckTime + i)->type == AUTOTIME_NON){
			continue;
		}
		if(NoMinuteCheck == 1 && (tpCheckTime + i)->type == AUTOTIME_MIN){
			continue;
		}
		if(CheckTime(tpCheckTime + i) == 1){
			return TRUE;
		}
	}
	return FALSE;
}


/******************************************************************************

	TreeCheckAutoTime

	�t�H���_���̎����`�F�b�N�̏����`�F�b�N

******************************************************************************/

void CALLBACK TreeCheckAutoTime(HWND hWnd, HTREEITEM hItem, long Param)
{
	struct TPTREE *tpTreeInfo;
	int TimeSize;
	int i;

	if(TreeView_IsRecyclerItem(GetDlgItem(hWnd, WWWC_TREE), hItem) == TRUE){
		//���ݔ������ݔ����̃t�H���_�̏ꍇ
		return;
	}

	if((tpTreeInfo = (struct TPTREE *)TreeView_GetlParam(GetDlgItem(hWnd, WWWC_TREE), hItem)) == NULL){
		return;
	}
	if(tpTreeInfo->CheckSt == 1 || tpTreeInfo->AutoCheckSt == 1){
		return;
	}

	TimeSize = tpTreeInfo->tpCheckTimeCnt;

	//�����`�F�b�N�ݒ���`�F�b�N����
	for(i = 0;i < TimeSize;i++){
		if((tpTreeInfo->tpCheckTime + i) == NULL ||
			(tpTreeInfo->tpCheckTime + i)->type == AUTOTIME_NON){
			continue;
		}
		if(CheckTime(tpTreeInfo->tpCheckTime + i) == 1){
			//�`�F�b�N�J�n
			CheckIniProc(hWnd, hItem, CHECKINI_AUTOCHECK, CHECKTYPE_AUTO | CHECKTYPE_ITEM);
		}
	}
}


/******************************************************************************

	CopyAutoTime

	�����`�F�b�N���̃R�s�[

******************************************************************************/

void CALLBACK CopyAutoTime(HWND hWnd, HTREEITEM hItem, long Param)
{
	struct TPTREE *tpTreeInfo;
	struct TPTREE *tpTreeInfoTop;

	if((tpTreeInfo = (struct TPTREE *)TreeView_GetlParam(GetDlgItem(hWnd, WWWC_TREE), hItem)) == NULL){
		return;
	}
	tpTreeInfoTop = (struct TPTREE *)Param;

	tpTreeInfo->CheckSt = tpTreeInfoTop->CheckSt;
	if(tpTreeInfo->CheckSt == 0){
		TreeView_SetState(GetDlgItem(hWnd, WWWC_TREE), hItem, 0, TVIS_OVERLAYMASK);
		if(TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)) == TreeView_GetParent(GetDlgItem(hWnd, WWWC_TREE), hItem)){
			ListView_SetItemState(GetDlgItem(hWnd, WWWC_LIST),
				TreeView_GetListIndex(GetDlgItem(hWnd, WWWC_TREE), hItem), 0, LVIS_OVERLAYMASK);
		}
	}else{
		TreeView_SetState(GetDlgItem(hWnd, WWWC_TREE), hItem, INDEXTOOVERLAYMASK(ICON_ST_NOCHECK),
			TVIS_OVERLAYMASK);
		if(TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)) == TreeView_GetParent(GetDlgItem(hWnd, WWWC_TREE), hItem)){
			ListView_SetItemState(GetDlgItem(hWnd, WWWC_LIST),
				TreeView_GetListIndex(GetDlgItem(hWnd, WWWC_TREE), hItem), INDEXTOOVERLAYMASK(ICON_ST_NOCHECK), LVIS_OVERLAYMASK);
		}
	}

	tpTreeInfo->AutoCheckSt = tpTreeInfoTop->AutoCheckSt;

	if(tpTreeInfoTop->tpCheckTime == NULL){
		if(tpTreeInfo->tpCheckTime != NULL){
			GlobalFree(tpTreeInfo->tpCheckTime);
			tpTreeInfo->tpCheckTime = NULL;
			tpTreeInfo->tpCheckTimeCnt = 0;
		}
		return;
	}
	if(tpTreeInfo->tpCheckTime != NULL){
		GlobalFree(tpTreeInfo->tpCheckTime);
		tpTreeInfo->tpCheckTimeCnt = 0;
	}
	tpTreeInfo->tpCheckTime = (struct TPCHECKTIME *)GlobalAlloc(GPTR, sizeof(struct TPCHECKTIME) * tpTreeInfoTop->tpCheckTimeCnt);

	if(tpTreeInfo->tpCheckTime == NULL){
		ErrMsg(hWnd, GetLastError(), NULL);
		return;
	}
	tpTreeInfo->tpCheckTimeCnt = tpTreeInfoTop->tpCheckTimeCnt;

	CopyMemory(tpTreeInfo->tpCheckTime,
		tpTreeInfoTop->tpCheckTime, sizeof(struct TPCHECKTIME) * tpTreeInfoTop->tpCheckTimeCnt);
}


/******************************************************************************

	GetAutoCheckString

	�����`�F�b�N��񂩂�\����������쐬

******************************************************************************/

static int GetAutoCheckString(struct TPCHECKTIME *tpTimeInfo, char *ret)
{
	char strWeek[][STRWEEK_STRLEN] = {
		STRWEEK_SUN,
		STRWEEK_MON,
		STRWEEK_TUE,
		STRWEEK_WED,
		STRWEEK_THU,
		STRWEEK_FRI,
		STRWEEK_SAT,
	};

	switch(tpTimeInfo->type)
	{
	case AUTOTIME_MIN:
		wsprintf(ret, LISTSTR_MIN, tpTimeInfo->m);
		return 1;

	case AUTOTIME_HOU:
		wsprintf(ret, LISTSTR_NOU, tpTimeInfo->h);
		return 1;

	case AUTOTIME_DAY:
		wsprintf(ret, LISTSTR_DAY, tpTimeInfo->h, tpTimeInfo->m);
		return 1;
		break;

	case AUTOTIME_WEEK:
		wsprintf(ret, LISTSTR_WEEK, strWeek[tpTimeInfo->week], tpTimeInfo->h, tpTimeInfo->m);
		return 1;

	case AUTOTIME_MON:
		wsprintf(ret, LISTSTR_MON, tpTimeInfo->day, tpTimeInfo->h, tpTimeInfo->m);
		return 1;
	}
	return 0;
}


/******************************************************************************

	EnableWeekContorol

	�T�I���R���g���[���̐ݒ�

******************************************************************************/

static void EnableWeekContorol(HWND hDlg, BOOL eFlag)
{
	EnableWindow(GetDlgItem(hDlg, IDC_RADIO6), eFlag);
	EnableWindow(GetDlgItem(hDlg, IDC_RADIO7), eFlag);
	EnableWindow(GetDlgItem(hDlg, IDC_RADIO8), eFlag);
	EnableWindow(GetDlgItem(hDlg, IDC_RADIO9), eFlag);
	EnableWindow(GetDlgItem(hDlg, IDC_RADIO10), eFlag);
	EnableWindow(GetDlgItem(hDlg, IDC_RADIO11), eFlag);
	EnableWindow(GetDlgItem(hDlg, IDC_RADIO12), eFlag);
}


/******************************************************************************

	EnableCheckContorol

	�`�F�b�N���@�I���R���g���[���̐ݒ�

******************************************************************************/

static void EnableCheckContorol(HWND hDlg, BOOL eFlag)
{
	if(NoMinuteCheck == 1 && PropItem == NULL){
		EnableWindow(GetDlgItem(hDlg, IDC_RADIO1), FALSE);
	}else{
		EnableWindow(GetDlgItem(hDlg, IDC_RADIO1), eFlag);
	}
	EnableWindow(GetDlgItem(hDlg, IDC_RADIO2), eFlag);
	EnableWindow(GetDlgItem(hDlg, IDC_RADIO3), eFlag);
	EnableWindow(GetDlgItem(hDlg, IDC_RADIO4), eFlag);
	EnableWindow(GetDlgItem(hDlg, IDC_RADIO5), eFlag);
}


/******************************************************************************

	EnableAutoTimeList

	�����`�F�b�N���X�g�̐ݒ�

******************************************************************************/

static void EnableAutoTimeList(HWND hDlg, BOOL eFlag)
{
	EnableWindow(GetDlgItem(hDlg, IDC_LIST_AUTOTIME), eFlag);
	EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_ADD), eFlag);
	EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_EDIT), eFlag);
	EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_DELETE), eFlag);
}


/******************************************************************************

	EnableAutoTimeContorol

	�����`�F�b�N�ݒ��ʂ̐ݒ�

******************************************************************************/

static void EnableAutoTimeContorol(HWND hDlg)
{
	SetWindowText(GetDlgItem(hDlg, IDC_STATIC_HOU), TIMEOPTIONSTR_HOU);
	SetWindowText(GetDlgItem(hDlg, IDC_STATIC_MIN), TIMEOPTIONSTR_MIN);
	SendDlgItemMessage(hDlg, IDC_SPIN_HOU, UDM_SETRANGE, 0, (LPARAM)UD_HOU);
	SendDlgItemMessage(hDlg, IDC_SPIN_MIN, UDM_SETRANGE, 0, (LPARAM)UD_MIN);

	if(IsDlgButtonChecked(hDlg, IDC_CHECK_NO) == 1){
		EnableCheckContorol(hDlg, FALSE);

		EnableWindow(GetDlgItem(hDlg, IDC_EDIT_DAY), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_STATIC_DAY), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_EDIT_HOU), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_STATIC_HOU), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_EDIT_MIN), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_STATIC_MIN), FALSE);
		EnableWeekContorol(hDlg, FALSE);
		return;
	}
	EnableCheckContorol(hDlg, TRUE);

	EnableWindow(GetDlgItem(hDlg, IDC_EDIT_DAY), FALSE);
	EnableWindow(GetDlgItem(hDlg, IDC_STATIC_DAY), FALSE);
	EnableWindow(GetDlgItem(hDlg, IDC_EDIT_HOU), TRUE);
	EnableWindow(GetDlgItem(hDlg, IDC_STATIC_HOU), TRUE);
	EnableWindow(GetDlgItem(hDlg, IDC_EDIT_MIN), TRUE);
	EnableWindow(GetDlgItem(hDlg, IDC_STATIC_MIN), TRUE);

	EnableWeekContorol(hDlg, FALSE);

	if(IsDlgButtonChecked(hDlg, IDC_RADIO1) == 1){
		EnableWindow(GetDlgItem(hDlg, IDC_EDIT_HOU), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_STATIC_HOU), FALSE);

		SetWindowText(GetDlgItem(hDlg, IDC_STATIC_MIN), TIMEOPTIONSTR_EVERYMIN);
		SendDlgItemMessage(hDlg, IDC_SPIN_MIN, UDM_SETRANGE, 0, (LPARAM)UD_EVERYMIN);

	}else if(IsDlgButtonChecked(hDlg, IDC_RADIO2) == 1){
		EnableWindow(GetDlgItem(hDlg, IDC_EDIT_MIN), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_STATIC_MIN), FALSE);

		SetWindowText(GetDlgItem(hDlg, IDC_STATIC_HOU), TIMEOPTIONSTR_EVERYHOU);
		SendDlgItemMessage(hDlg, IDC_SPIN_HOU, UDM_SETRANGE, 0, (LPARAM)UD_EVERYHOU);

	}else if(IsDlgButtonChecked(hDlg, IDC_RADIO3) == 1){

	}else if(IsDlgButtonChecked(hDlg, IDC_RADIO4) == 1){
		EnableWeekContorol(hDlg, TRUE);

	}else if(IsDlgButtonChecked(hDlg, IDC_RADIO5) == 1){
		EnableWindow(GetDlgItem(hDlg, IDC_EDIT_DAY), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_STATIC_DAY), TRUE);
	}
}


/******************************************************************************

	SetAutoCheckInfo

	�����`�F�b�N���ڂ̐ݒ�̔��f

******************************************************************************/

static BOOL SetAutoCheckInfo(HWND hDlg)
{
	struct TPCHECKTIME *tpCheckTimeItem;
	char buf[BUFSIZE];
	long wndParam;
	HWND pWnd;

	//�� �̎擾
	if(IsWindowEnabled(GetDlgItem(hDlg, IDC_EDIT_DAY)) != 0){
		SendDlgItemMessage(hDlg, IDC_EDIT_DAY, WM_GETTEXT, BUFSIZE - 1, (LPARAM)buf);
		if(*buf == '\0'){
			MessageBox(hDlg, TIMEOPTIONERRMSG_DAY, TIMEOPTIONERRMSG_TITLE, MB_OK | MB_ICONEXCLAMATION);
			return FALSE;
		}
	}

	//�� �̎擾
	if(IsWindowEnabled(GetDlgItem(hDlg, IDC_EDIT_HOU)) != 0){
		SendDlgItemMessage(hDlg, IDC_EDIT_HOU, WM_GETTEXT, BUFSIZE - 1, (LPARAM)buf);
		if(*buf == '\0'){
			MessageBox(hDlg, TIMEOPTIONERRMSG_HOU, TIMEOPTIONERRMSG_TITLE, MB_OK | MB_ICONEXCLAMATION);
			return FALSE;
		}
	}

	//�� �̎擾
	if(IsWindowEnabled(GetDlgItem(hDlg, IDC_EDIT_MIN)) != 0){
		SendDlgItemMessage(hDlg, IDC_EDIT_MIN, WM_GETTEXT, BUFSIZE - 1, (LPARAM)buf);
		if(*buf == '\0'){
			MessageBox(hDlg, TIMEOPTIONERRMSG_MIN, TIMEOPTIONERRMSG_TITLE, MB_OK | MB_ICONEXCLAMATION);
			return FALSE;
		}
	}

	//�����`�F�b�N���̎擾
	wndParam = GetWindowLong(hDlg, GWL_USERDATA);
	if(wndParam == 0){
		//�V�K�ǉ��̏ꍇ�͊m��
		tpCheckTimeItem = (struct TPCHECKTIME *)GlobalAlloc(GPTR, sizeof(struct TPCHECKTIME));
		if(tpCheckTimeItem == NULL){
			ErrMsg(hDlg, GetLastError(), NULL);
			return FALSE;
		}
	}else{
		tpCheckTimeItem = (struct TPCHECKTIME *)wndParam;
	}
	tpCheckTimeItem->flag = IsDlgButtonChecked(hDlg, IDC_CHECK_NO);

	if(IsDlgButtonChecked(hDlg, IDC_RADIO1) == 1){
		//����
		tpCheckTimeItem->type = AUTOTIME_MIN;
		SendDlgItemMessage(hDlg, IDC_EDIT_MIN, WM_GETTEXT, BUFSIZE - 1, (LPARAM)buf);
		tpCheckTimeItem->m = atoi(buf);

		tpCheckTimeItem->day = 0;
		tpCheckTimeItem->h = 0;
		tpCheckTimeItem->week = 0;

	}else if(IsDlgButtonChecked(hDlg, IDC_RADIO2) == 1){
		//���Ԗ�
		tpCheckTimeItem->type = AUTOTIME_HOU;
		SendDlgItemMessage(hDlg, IDC_EDIT_HOU, WM_GETTEXT, BUFSIZE - 1, (LPARAM)buf);
		tpCheckTimeItem->h = atoi(buf);

		tpCheckTimeItem->day = 0;
		tpCheckTimeItem->m = 0;
		tpCheckTimeItem->week = 0;

	}else if(IsDlgButtonChecked(hDlg, IDC_RADIO3) == 1){
		//���Ԏw��
		tpCheckTimeItem->type = AUTOTIME_DAY;
		SendDlgItemMessage(hDlg, IDC_EDIT_HOU, WM_GETTEXT, BUFSIZE - 1, (LPARAM)buf);
		tpCheckTimeItem->h = atoi(buf);
		SendDlgItemMessage(hDlg, IDC_EDIT_MIN, WM_GETTEXT, BUFSIZE - 1, (LPARAM)buf);
		tpCheckTimeItem->m = atoi(buf);

		tpCheckTimeItem->day = 0;
		tpCheckTimeItem->week = 0;

	}else if(IsDlgButtonChecked(hDlg, IDC_RADIO4) == 1){
		//�j���w��
		tpCheckTimeItem->type = AUTOTIME_WEEK;

		if(IsDlgButtonChecked(hDlg, IDC_RADIO6) == 1){
			tpCheckTimeItem->week = 0;		//��
		}else if(IsDlgButtonChecked(hDlg, IDC_RADIO7) == 1){
			tpCheckTimeItem->week = 1;		//��
		}else if(IsDlgButtonChecked(hDlg, IDC_RADIO8) == 1){
			tpCheckTimeItem->week = 2;		//��
		}else if(IsDlgButtonChecked(hDlg, IDC_RADIO9) == 1){
			tpCheckTimeItem->week = 3;		//��
		}else if(IsDlgButtonChecked(hDlg, IDC_RADIO10) == 1){
			tpCheckTimeItem->week = 4;		//��
		}else if(IsDlgButtonChecked(hDlg, IDC_RADIO11) == 1){
			tpCheckTimeItem->week = 5;		//��
		}else if(IsDlgButtonChecked(hDlg, IDC_RADIO12) == 1){
			tpCheckTimeItem->week = 6;		//�y
		}
		SendDlgItemMessage(hDlg, IDC_EDIT_HOU, WM_GETTEXT, BUFSIZE - 1, (LPARAM)buf);
		tpCheckTimeItem->h = atoi(buf);
		SendDlgItemMessage(hDlg, IDC_EDIT_MIN, WM_GETTEXT, BUFSIZE - 1, (LPARAM)buf);
		tpCheckTimeItem->m = atoi(buf);

		tpCheckTimeItem->day = 0;

	}else if(IsDlgButtonChecked(hDlg, IDC_RADIO5) == 1){
		//���t�w��
		tpCheckTimeItem->type = AUTOTIME_MON;
		SendDlgItemMessage(hDlg, IDC_EDIT_DAY, WM_GETTEXT, BUFSIZE - 1, (LPARAM)buf);
		tpCheckTimeItem->day = atoi(buf);
		SendDlgItemMessage(hDlg, IDC_EDIT_HOU, WM_GETTEXT, BUFSIZE - 1, (LPARAM)buf);
		tpCheckTimeItem->h = atoi(buf);
		SendDlgItemMessage(hDlg, IDC_EDIT_MIN, WM_GETTEXT, BUFSIZE - 1, (LPARAM)buf);
		tpCheckTimeItem->m = atoi(buf);

		tpCheckTimeItem->week = 0;
	}

	tpCheckTimeItem->mCnt = 0;
	tpCheckTimeItem->hCnt = 0;

	//�e�E�B���h�E�̃��X�g�r���[�ɒǉ�
	pWnd = PropSheet_GetCurrentPageHwnd(GetParent(hDlg));
	if(GetAutoCheckString(tpCheckTimeItem, buf) == 1){
		if(wndParam == 0){
			ListView_InsertItemEx(GetDlgItem(pWnd, IDC_LIST_AUTOTIME), buf, tpCheckTimeItem->flag,
				(long)tpCheckTimeItem, -1);
		}else{
			ListView_SetItemText(GetDlgItem(pWnd, IDC_LIST_AUTOTIME),
				ListView_GetNextItem(GetDlgItem(pWnd, IDC_LIST_AUTOTIME), -1, LVNI_SELECTED), 0, buf);

			ListView_SetItemIcon(GetDlgItem(pWnd, IDC_LIST_AUTOTIME),
				ListView_GetNextItem(GetDlgItem(pWnd, IDC_LIST_AUTOTIME), -1, LVNI_SELECTED), tpCheckTimeItem->flag);
		}
	}
	return TRUE;
}


/******************************************************************************

	SetAutoCheckProc

	�����`�F�b�N���ڂ̐ݒ���

******************************************************************************/

static BOOL CALLBACK SetAutoCheckProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	struct TPCHECKTIME *tpCheckTimeItem;
	char buf[BUFSIZE];

	switch (uMsg)
	{
	case WM_INITDIALOG:
		//�X�s���R���g���[���̐ݒ�
		SendDlgItemMessage(hDlg, IDC_SPIN_DAY, UDM_SETRANGE, 0, (LPARAM)UD_DAY);

		if(lParam == 0){
			//�V�K�ǉ�
			CheckDlgButton(hDlg, IDC_RADIO3, 1);
			CheckDlgButton(hDlg, IDC_RADIO6, 1);
			EnableAutoTimeContorol(hDlg);
			SetWindowLong(hDlg, GWL_USERDATA, 0);
			break;
		}

		//�����`�F�b�N�����E�B���h�E�ɐݒ�
		SetWindowLong(hDlg, GWL_USERDATA, lParam);

		tpCheckTimeItem = (struct TPCHECKTIME *)lParam;
		CheckDlgButton(hDlg, IDC_CHECK_NO, tpCheckTimeItem->flag);

		if(NoMinuteCheck == 1 && PropItem == NULL && tpCheckTimeItem->type - 1 == 0){
			CheckDlgButton(hDlg, IDC_RADIO2, 1);
		}else{
			CheckDlgButton(hDlg, IDC_RADIO1 + tpCheckTimeItem->type - 1, 1);
		}
		CheckDlgButton(hDlg, IDC_RADIO6 + tpCheckTimeItem->week, 1);
		wsprintf(buf, "%d", tpCheckTimeItem->day);
		SendDlgItemMessage(hDlg, IDC_EDIT_DAY, WM_SETTEXT, 0, (LPARAM)buf);
		wsprintf(buf, "%d", tpCheckTimeItem->h);
		SendDlgItemMessage(hDlg, IDC_EDIT_HOU, WM_SETTEXT, 0, (LPARAM)buf);
		wsprintf(buf, "%d", tpCheckTimeItem->m);
		SendDlgItemMessage(hDlg, IDC_EDIT_MIN, WM_SETTEXT, 0, (LPARAM)buf);

		EnableAutoTimeContorol(hDlg);
		break;

	case WM_CLOSE:
		EndDialog(hDlg, FALSE);
		break;

	case WM_COMMAND:
		switch(wParam)
		{
		case IDOK:
			if(SetAutoCheckInfo(hDlg) == TRUE){
				EndDialog(hDlg, TRUE);
			}
			break;

		case IDCANCEL:
			EndDialog(hDlg, FALSE);
			break;

		case IDC_CHECK_NO:
		case IDC_RADIO1:
		case IDC_RADIO2:
		case IDC_RADIO3:
		case IDC_RADIO4:
		case IDC_RADIO5:
			EnableAutoTimeContorol(hDlg);
			break;
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}


/******************************************************************************

	InitializeAutoCheckListWindow

	�����`�F�b�N���X�g��ʂ̏�����

******************************************************************************/

static BOOL InitializeAutoCheckListWindow(HWND hDlg)
{
	struct TPTREE *tpTreeInfo;
	struct TPCHECKTIME *tpCheckTimeItem;
	struct TPCHECKTIME *tpTmpTimeItem;
	HIMAGELIST IconList;
	LV_COLUMN lvc;
	char buf[BUFSIZE];
	int tpTmpTimeItemCnt;
	int i;

	//���X�g�r���[�̃J�����̐ݒ�
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.fmt = LVCFMT_LEFT;
	lvc.cx = LISTCOL_SIZE;
	lvc.pszText = LISTCOL_TITLE;
	lvc.iSubItem = 0;
	ListView_InsertColumn(GetDlgItem(hDlg, IDC_LIST_AUTOTIME), 0, &lvc);

	//�C���[�W���X�g�̐ݒ�
	IconList = ImageList_Create(SICONSIZE, SICONSIZE, ILC_COLOR16 | ILC_MASK, 0, 0);
	ImageList_SetBkColor(IconList, GetSysColor(COLOR_WINDOW));
	ImageListIconAdd(IconList, IDI_ICON_TIMEOUT, SICONSIZE, "", 0);
	ImageListIconAdd(IconList, IDI_ICON_NOAUTOTIME, SICONSIZE, "", 0);
	ListView_SetImageList(GetDlgItem(hDlg, IDC_LIST_AUTOTIME), IconList, LVSIL_SMALL);

	//�X�^�C���̐ݒ�
	SetWindowLong(GetDlgItem(hDlg, IDC_LIST_AUTOTIME), GWL_STYLE,
		GetWindowLong(GetDlgItem(hDlg, IDC_LIST_AUTOTIME), GWL_STYLE) | LVS_SHOWSELALWAYS);
	SendDlgItemMessage(hDlg, IDC_LIST_AUTOTIME, LVM_SETEXTENDEDLISTVIEWSTYLE, 0,
		LVS_EX_ONECLICKACTIVATE | LVS_EX_FULLROWSELECT |
		SendDlgItemMessage(hDlg, IDC_LIST_AUTOTIME, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0));

	SetWindowLong(hDlg, GWL_USERDATA, (long)PropItem);

	if(PropItem == NULL){
		//�I�v�V������ʂ���̌Ăяo��
		ShowWindow(GetDlgItem(hDlg, IDC_CHECK_FOLDER_NOCHECK), SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_CHECK_FOLDER_DEF), SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_BUTTON_TREESET), SW_HIDE);

		if(AutoCheck == 1){
			CheckDlgButton(hDlg, IDC_CHECK_NOCHECK, 1);
			EnableAutoTimeList(hDlg, FALSE);
		}
		CheckDlgButton(hDlg, IDC_CHECK_STARTCHECK, StartCheck);

		tpTmpTimeItem = tpCheckTime;
		tpTmpTimeItemCnt = tpCheckTimeCnt;
	}else{
		//�t�H���_�ݒ��ʂ���̌Ăяo��
		ShowWindow(GetDlgItem(hDlg, IDC_CHECK_NOCHECK), SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_CHECK_STARTCHECK), SW_HIDE);

		tpTreeInfo = (struct TPTREE *)TreeView_GetlParam(GetDlgItem(WWWCWnd, WWWC_TREE), (HTREEITEM)PropItem);
		if(tpTreeInfo == NULL){
			return FALSE;
		}

		if(tpTreeInfo->CheckSt == 1){
			//�`�F�b�N���Ȃ�
			CheckDlgButton(hDlg, IDC_CHECK_FOLDER_NOCHECK, 1);
			EnableAutoTimeList(hDlg, FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_CHECK_FOLDER_DEF), FALSE);
		}
		if(tpTreeInfo->AutoCheckSt == 1){
			//�f�t�H���g�̃`�F�b�N�ݒ���g�p
			CheckDlgButton(hDlg, IDC_CHECK_FOLDER_DEF, 1);
			EnableAutoTimeList(hDlg, FALSE);
		}
		tpTmpTimeItem = tpTreeInfo->tpCheckTime;
		tpTmpTimeItemCnt = tpTreeInfo->tpCheckTimeCnt;

		if(AutoCheck == 1){
			EnableAutoTimeList(hDlg, FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_CHECK_FOLDER_DEF), FALSE);
		}
	}

	if(tpTmpTimeItem == NULL){
		return TRUE;
	}

	ListView_SetRedraw(GetDlgItem(hDlg, IDC_LIST_AUTOTIME), LDF_NODRAW);
	//�����`�F�b�N���̕\��
	for(i = 0;i < tpTmpTimeItemCnt;i++){
		if((tpTmpTimeItem + i) == NULL || (tpTmpTimeItem + i)->type == AUTOTIME_NON){
			continue;
		}
		if(GetAutoCheckString(tpTmpTimeItem + i, buf) == 1){
			tpCheckTimeItem = (struct TPCHECKTIME *)GlobalAlloc(GPTR, sizeof(struct TPCHECKTIME));
			if(tpCheckTimeItem == NULL){
				ErrMsg(hDlg, GetLastError(), NULL);
				return FALSE;
			}
			CopyMemory(tpCheckTimeItem, tpTmpTimeItem + i, sizeof(struct TPCHECKTIME));
			ListView_InsertItemEx(GetDlgItem(hDlg, IDC_LIST_AUTOTIME),
				buf, tpCheckTimeItem->flag, (long)tpCheckTimeItem, -1);
		}
	}
	ListView_SetRedraw(GetDlgItem(hDlg, IDC_LIST_AUTOTIME), LDF_REDRAW);
	return TRUE;
}


/******************************************************************************

	SetAutoCheckList

	�I�v�V�����܂��́A�t�H���_�Ɏ����`�F�b�N���X�g��ݒ肷��

******************************************************************************/

static void SetAutoCheckList(HWND hDlg)
{
	struct TPTREE *tpTreeInfo;
	HTREEITEM hItem;
	int i, TimeSize;

	if(GetWindowLong(hDlg, GWL_USERDATA) == 0){
		//�I�v�V������ʂ���̌Ăяo���̏ꍇ
		if(tpCheckTime != NULL){
			GlobalFree(tpCheckTime);
			tpCheckTimeCnt = 0;
		}

		TimeSize = ListView_GetItemCount(GetDlgItem(hDlg, IDC_LIST_AUTOTIME));
		if(TimeSize == 0){
			tpCheckTime = NULL;
			tpCheckTimeCnt = 0;
		}else{
			tpCheckTime = (struct TPCHECKTIME *)GlobalAlloc(GPTR, sizeof(struct TPCHECKTIME) * TimeSize);
			if(tpCheckTime == NULL){
				ErrMsg(hDlg, GetLastError(), NULL);
				return;
			}
			tpCheckTimeCnt = TimeSize;
			for(i = 0;i < TimeSize;i++){
				CopyMemory(tpCheckTime + i,
					(struct TPCHECKTIME *)ListView_GetlParam(GetDlgItem(hDlg, IDC_LIST_AUTOTIME), i), sizeof(struct TPCHECKTIME));
				GlobalFree((struct TPCHECKTIME *)ListView_GetlParam(GetDlgItem(hDlg, IDC_LIST_AUTOTIME), i));
			}
		}
		AutoCheck = IsDlgButtonChecked(hDlg, IDC_CHECK_NOCHECK);
		StartCheck = IsDlgButtonChecked(hDlg, IDC_CHECK_STARTCHECK);
		return;
	}

	//�t�H���_�ݒ��ʂ���̌Ăяo���̏ꍇ
	hItem = (HTREEITEM)GetWindowLong(hDlg, GWL_USERDATA);
	tpTreeInfo = (struct TPTREE *)TreeView_GetlParam(GetDlgItem(WWWCWnd, WWWC_TREE), hItem);
	if(tpTreeInfo == NULL){
		return;
	}

	if(tpTreeInfo->tpCheckTime != NULL){
		GlobalFree(tpTreeInfo->tpCheckTime);
		tpTreeInfo->tpCheckTimeCnt = 0;
	}

	TimeSize = ListView_GetItemCount(GetDlgItem(hDlg, IDC_LIST_AUTOTIME));
	if(TimeSize == 0){
		tpTreeInfo->tpCheckTime = NULL;
		tpTreeInfo->tpCheckTimeCnt = 0;
	}else{
		tpTreeInfo->tpCheckTime = (struct TPCHECKTIME *)GlobalAlloc(GPTR, sizeof(struct TPCHECKTIME) * TimeSize);
		if(tpTreeInfo->tpCheckTime == NULL){
			ErrMsg(hDlg, GetLastError(), NULL);
			return;
		}
		tpTreeInfo->tpCheckTimeCnt = TimeSize;
		for(i = 0;i < TimeSize;i++){
			CopyMemory(tpTreeInfo->tpCheckTime + i,
				(struct TPCHECKTIME *)ListView_GetlParam(GetDlgItem(hDlg, IDC_LIST_AUTOTIME), i),
				sizeof(struct TPCHECKTIME));
			GlobalFree((struct TPCHECKTIME *)ListView_GetlParam(GetDlgItem(hDlg, IDC_LIST_AUTOTIME), i));
		}
	}

	//�`�F�b�N����^���Ȃ�
	tpTreeInfo->CheckSt = IsDlgButtonChecked(hDlg, IDC_CHECK_FOLDER_NOCHECK);
	if(tpTreeInfo->CheckSt == 0){
		TreeView_SetState(GetDlgItem(WWWCWnd, WWWC_TREE), hItem, 0, TVIS_OVERLAYMASK);
		if(TreeView_GetSelection(GetDlgItem(WWWCWnd, WWWC_TREE)) == TreeView_GetParent(GetDlgItem(WWWCWnd, WWWC_TREE), hItem)){
			ListView_SetItemState(GetDlgItem(WWWCWnd, WWWC_LIST),
				TreeView_GetListIndex(GetDlgItem(WWWCWnd, WWWC_TREE), hItem), 0, LVIS_OVERLAYMASK);
		}

	}else{
		TreeView_SetState(GetDlgItem(WWWCWnd, WWWC_TREE), hItem, INDEXTOOVERLAYMASK(ICON_ST_NOCHECK), TVIS_OVERLAYMASK);
		if(TreeView_GetSelection(GetDlgItem(WWWCWnd, WWWC_TREE)) == TreeView_GetParent(GetDlgItem(WWWCWnd, WWWC_TREE), hItem)){
			ListView_SetItemState(GetDlgItem(WWWCWnd, WWWC_LIST),
				TreeView_GetListIndex(GetDlgItem(WWWCWnd, WWWC_TREE), hItem), INDEXTOOVERLAYMASK(ICON_ST_NOCHECK), LVIS_OVERLAYMASK);
		}
	}

	//�f�t�H���g�̃`�F�b�N�ݒ���g�p����^���Ȃ�
	tpTreeInfo->AutoCheckSt = IsDlgButtonChecked(hDlg, IDC_CHECK_FOLDER_DEF);
}


/******************************************************************************

	TreeCopyAutoCheckInfo

	���ʂ̃t�H���_�Ɏ����`�F�b�N�ݒ���R�s�[����

******************************************************************************/

static void TreeCopyAutoCheckInfo(HWND hDlg)
{
	struct TPTREE *tpTreeInfo;
	HTREEITEM hItem;
	int i, TimeSize;

	hItem = (HTREEITEM)GetWindowLong(hDlg, GWL_USERDATA);

	tpTreeInfo = (struct TPTREE *)GlobalAlloc(GPTR, sizeof(struct TPTREE));
	if(tpTreeInfo == NULL){
		ErrMsg(hDlg, GetLastError(), NULL);
		return;
	}

	TimeSize = ListView_GetItemCount(GetDlgItem(hDlg, IDC_LIST_AUTOTIME));
	if(TimeSize == 0){
		tpTreeInfo->tpCheckTime = NULL;
		tpTreeInfo->tpCheckTimeCnt = 0;
	}else{
		tpTreeInfo->tpCheckTime = (struct TPCHECKTIME *)GlobalAlloc(GPTR, sizeof(struct TPCHECKTIME) * TimeSize);
		if(tpTreeInfo->tpCheckTime == NULL){
			ErrMsg(hDlg, GetLastError(), NULL);
			GlobalFree(tpTreeInfo);
			return;
		}
		tpTreeInfo->tpCheckTimeCnt = TimeSize;

		for(i = 0;i < TimeSize;i++){
			CopyMemory(tpTreeInfo->tpCheckTime + i,
				(struct TPCHECKTIME *)ListView_GetlParam(GetDlgItem(hDlg, IDC_LIST_AUTOTIME), i),
				sizeof(struct TPCHECKTIME));
		}
	}

	tpTreeInfo->CheckSt = IsDlgButtonChecked(hDlg, IDC_CHECK_FOLDER_NOCHECK);
	tpTreeInfo->AutoCheckSt = IsDlgButtonChecked(hDlg, IDC_CHECK_FOLDER_DEF);

	//�����`�F�b�N�ݒ�̃R�s�[
	CallTreeItem(WWWCWnd, hItem, (FARPROC)CopyAutoTime, (long)tpTreeInfo);

	if(tpTreeInfo->tpCheckTime != NULL){
		GlobalFree(tpTreeInfo->tpCheckTime);
	}
	GlobalFree(tpTreeInfo);
}


/******************************************************************************

	AutoCheckListProc

	�����`�F�b�N���X�g�E�B���h�E�̃v���V�[�W��

******************************************************************************/

BOOL CALLBACK AutoCheckListProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int SelectItem;
	int i, TimeSize;

	switch (uMsg)
	{
	case WM_INITDIALOG:
		InitializeAutoCheckListWindow(hDlg);
		SendMessage(hDlg, WM_LV_EVENT, LVN_ITEMCHANGED, 0);
		break;

	case WM_NOTIFY:
		if(DialogLvNotifyProc(hDlg, lParam, GetDlgItem(hDlg, IDC_LIST_AUTOTIME)) == FALSE){
			return OptionNotifyProc(hDlg, uMsg, wParam, lParam);
		}
		break;

	case WM_LV_EVENT:
		if(wParam == LVN_ITEMCHANGED && IsWindowEnabled(GetDlgItem(hDlg, IDC_LIST_AUTOTIME)) != 0){
			BOOL enable;

			enable = (ListView_GetSelectedCount(GetDlgItem(hDlg, IDC_LIST_AUTOTIME)) <= 0) ? FALSE : TRUE;
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_EDIT), enable);
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_DELETE), enable);
		}
		break;

	case WM_COMMAND:
		switch(wParam)
		{
		case IDOK:
			if(GetDlgItem(hDlg, IDC_LIST_AUTOTIME) == NULL){
				break;
			}
			SetAutoCheckList(hDlg);
			ImageList_Destroy((void *)ListView_GetImageList(GetDlgItem(hDlg, IDC_LIST_AUTOTIME), LVSIL_SMALL));
			DestroyWindow(GetDlgItem(hDlg, IDC_LIST_AUTOTIME));
			break;

		case IDPCANCEL:
			if(GetDlgItem(hDlg, IDC_LIST_AUTOTIME) == NULL){
				break;
			}
			TimeSize = ListView_GetItemCount(GetDlgItem(hDlg, IDC_LIST_AUTOTIME));
			for(i = 0;i < TimeSize;i++){
				GlobalFree((struct TPCHECKTIME *)ListView_GetlParam(GetDlgItem(hDlg, IDC_LIST_AUTOTIME), i));
			}

			ImageList_Destroy((void *)ListView_GetImageList(GetDlgItem(hDlg, IDC_LIST_AUTOTIME), LVSIL_SMALL));
			DestroyWindow(GetDlgItem(hDlg, IDC_LIST_AUTOTIME));
			break;

		//�`�F�b�N���s���^�s��Ȃ�
		case IDC_CHECK_NOCHECK:
			if(IsWindowVisible(GetDlgItem(hDlg, IDC_CHECK_NOCHECK)) == FALSE) break;
			if(IsDlgButtonChecked(hDlg, IDC_CHECK_NOCHECK) == 1){
				EnableAutoTimeList(hDlg, FALSE);
			}else{
				EnableAutoTimeList(hDlg, TRUE);
				SendMessage(hDlg, WM_LV_EVENT, LVN_ITEMCHANGED, 0);
			}
			break;

		//����`�F�b�N�̑ΏۊO�ɂ���
		case IDC_CHECK_FOLDER_NOCHECK:
			if(IsWindowVisible(GetDlgItem(hDlg, IDC_CHECK_FOLDER_NOCHECK)) == FALSE) break;
			if(IsDlgButtonChecked(hDlg, IDC_CHECK_FOLDER_NOCHECK) == 1){
				EnableAutoTimeList(hDlg, FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_CHECK_FOLDER_DEF), FALSE);
			}else{
				if(AutoCheck == 0){
					if(IsDlgButtonChecked(hDlg, IDC_CHECK_FOLDER_DEF) == 0){
						EnableAutoTimeList(hDlg, TRUE);
					}
					EnableWindow(GetDlgItem(hDlg, IDC_CHECK_FOLDER_DEF), TRUE);
				}
				SendMessage(hDlg, WM_LV_EVENT, LVN_ITEMCHANGED, 0);
			}
			break;

		//�f�t�H���g�̃`�F�b�N���g�p����^���Ȃ�
		case IDC_CHECK_FOLDER_DEF:
			if(IsWindowVisible(GetDlgItem(hDlg, IDC_CHECK_FOLDER_DEF)) == FALSE) break;
			EnableAutoTimeList(hDlg, (IsDlgButtonChecked(hDlg, IDC_CHECK_FOLDER_DEF) == 1) ? FALSE : TRUE);
			SendMessage(hDlg, WM_LV_EVENT, LVN_ITEMCHANGED, 0);
			break;

		//���̊K�w�ɔ��f (�t�H���_�ݒ��ʂ���̌Ăяo���̂�)
		case IDC_BUTTON_TREESET:
			if(IsWindowVisible(GetDlgItem(hDlg, IDC_BUTTON_TREESET)) == FALSE) break;
			if(GetDlgItem(hDlg, IDC_LIST_AUTOTIME) == NULL){
				break;
			}
			TreeCopyAutoCheckInfo(hDlg);
			break;

		//�����`�F�b�N���̒ǉ�
		case IDC_BUTTON_ADD:
			if(GetDlgItem(hDlg, IDC_LIST_AUTOTIME) == NULL){
				break;
			}
			DialogBoxParam(g_hinst, MAKEINTRESOURCE(IDD_DIALOG_SETAUTOTIME), hDlg, SetAutoCheckProc, 0);
			break;

		//�����`�F�b�N���̕ҏW
		case IDC_BUTTON_EDIT:
			if(GetDlgItem(hDlg, IDC_LIST_AUTOTIME) == NULL){
				break;
			}
			if((SelectItem = ListView_GetNextItem(GetDlgItem(hDlg, IDC_LIST_AUTOTIME), -1, LVNI_SELECTED)) == -1){
				break;
			}
			DialogBoxParam(g_hinst, MAKEINTRESOURCE(IDD_DIALOG_SETAUTOTIME), hDlg,
				SetAutoCheckProc, ListView_GetlParam(GetDlgItem(hDlg, IDC_LIST_AUTOTIME), SelectItem));
			break;

		//�����`�F�b�N���̍폜
		case IDC_BUTTON_DELETE:
			if(GetDlgItem(hDlg, IDC_LIST_AUTOTIME) == NULL){
				break;
			}
			if((SelectItem = ListView_GetNextItem(GetDlgItem(hDlg, IDC_LIST_AUTOTIME), -1, LVNI_SELECTED)) == -1){
				break;
			}
			if(MessageBox(hDlg, QMSG_DELETE, QMSG_DELETE_TITLE, MB_ICONQUESTION | MB_YESNO) == IDNO){
				break;
			}
			GlobalFree((struct TPCHECKTIME *)ListView_GetlParam(GetDlgItem(hDlg, IDC_LIST_AUTOTIME), SelectItem));
			ListView_DeleteItem(GetDlgItem(hDlg, IDC_LIST_AUTOTIME), SelectItem);
			ListView_SetItemState(GetDlgItem(hDlg, IDC_LIST_AUTOTIME), SelectItem,
				LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
			break;
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}
/* End of source */
