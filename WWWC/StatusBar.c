/**************************************************************************

	WWWC

	StatusBar.c

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

#define PART_CNT				3		//�p�[�c��
#define PART_MAXSIZE			-1		//�p�[�c�̃T�C�Y

#define SBT_TOOLTIPS			0x0800


/**************************************************************************
	Global Variables
**************************************************************************/

char *ToolTipString = NULL;


//�O���Q��
extern HINSTANCE g_hinst;
extern int gCheckFlag;		//�`�F�b�N�t���O
extern int ViewSb;			//StatusBar�\���I�v�V����
extern int PartsSize1;
extern int PartsSize2;
extern int PartInfo1;
extern int PartInfo2;
extern int PartInfo3;


/******************************************************************************

	CreateStatusBar

	StatusBar�̍쐬

******************************************************************************/

HWND CreateStatusBar(HWND hWnd)
{
	HWND sbWnd;

	sbWnd = CreateStatusWindow(WS_CHILD | SBT_TOOLTIPS, "", hWnd, WWWC_SB);

	if(sbWnd == NULL){
		return NULL;
	}
	//StatusBar��\������ݒ�̏ꍇ
	if(ViewSb == 1){
		ShowWindow(sbWnd, SW_SHOW);
	}
	return sbWnd;
}


/******************************************************************************

	SBSetParts

	�p�[�c�̐ݒ�

******************************************************************************/

void SBSetParts(HWND hWnd)
{
	int Width[PART_CNT];
	RECT WindowRect;

	//�E�B���h�E�T�C�Y����p�[�c�̃T�C�Y���v�Z
	GetWindowRect(hWnd, &WindowRect);
	*(Width + 2) = PART_MAXSIZE;
	*(Width + 1) = WindowRect.right - WindowRect.left - PartsSize2;
	*(Width + 0) = WindowRect.right - WindowRect.left - (PartsSize1 + PartsSize2);

	SendDlgItemMessage(hWnd, WWWC_SB, SB_SETPARTS,
		(WPARAM)sizeof(Width) / sizeof(int), (LPARAM)((LPINT)Width));
}


/******************************************************************************

	SetSbText

	�e�L�X�g�̐ݒ�

******************************************************************************/

void SetSbText(HWND hWnd)
{
	struct TPITEM *tpItemInfo;
	int SelectItem;
	int  Cnt, UpCnt;
	char buf[BUFSIZE];

	SendDlgItemMessage(hWnd, WWWC_SB, SB_SETTEXT, (WPARAM)1 | 0, (LPARAM)"");
	SendDlgItemMessage(hWnd, WWWC_SB, SB_SETTEXT, (WPARAM)2 | 0, (LPARAM)"");

	SelectItem = ListView_GetSelectedCount(GetDlgItem(hWnd, WWWC_LIST));

	//ListView�A�C�e���������I������Ă���ꍇ
	if(gCheckFlag == 0 && SelectItem > 1){
		wsprintf(buf, ITEMSELECTMSG, SelectItem);
		SendDlgItemMessage(hWnd, WWWC_SB, SB_SETTEXT, (WPARAM)0 | 0, (LPARAM)buf);
		return;
	}
	//ListView�A�C�e�����I������Ă��Ȃ��ꍇ
	if(gCheckFlag == 0 && SelectItem == 0){
		Cnt = ListView_GetItemCount(GetDlgItem(hWnd, WWWC_LIST));
		UpCnt = Item_UpCount(hWnd, TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)));
		if(UpCnt <= 0){
			wsprintf(buf, ITEMMSG, Cnt);
		}else{
			wsprintf(buf, ITEMMSG_UP, Cnt, UpCnt);
		}
		SendDlgItemMessage(hWnd, WWWC_SB, SB_SETTEXT, (WPARAM)0 | 0, (LPARAM)buf);
		return;
	}
	if(gCheckFlag == 0){
		SendDlgItemMessage(hWnd, WWWC_SB, SB_SETTEXT, (WPARAM)0 | 0, (LPARAM)"");
	}

	if((SelectItem = ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), -1, LVNI_FOCUSED)) == -1){
		return;
	}
	tpItemInfo = (struct TPITEM *)ListView_GetlParam(GetDlgItem(hWnd, WWWC_LIST), SelectItem);
	//�t�H���_���I������Ă���ꍇ
	if(tpItemInfo == NULL){
		if(gCheckFlag == 0){
			SendDlgItemMessage(hWnd, WWWC_SB, SB_SETTEXT,
				(WPARAM)0 | 0, (LPARAM)FOLDERNAME);
		}
		return;
	}

	//�A�C�e���̏���ݒ�
	if(gCheckFlag == 0){
		SendDlgItemMessage(hWnd, WWWC_SB, SB_SETTEXT,
			(WPARAM)0 | 0, (LPARAM)GetIndexToString(tpItemInfo, PartInfo1));
	}
	SendDlgItemMessage(hWnd, WWWC_SB, SB_SETTEXT,
		(WPARAM)1 | 0, (LPARAM)GetIndexToString(tpItemInfo, PartInfo2));
	SendDlgItemMessage(hWnd, WWWC_SB, SB_SETTEXT,
		(WPARAM)2 | 0, (LPARAM)GetIndexToString(tpItemInfo, PartInfo3));
}


/******************************************************************************

	StatusBar_NotifyProc

	�X�e�[�^�X�o�[�̃c�[���`�b�v�̐ݒ�

******************************************************************************/

LRESULT StatusBar_NotifyProc(HWND hWnd, LPARAM lParam)
{
	NMHDR *CForm = (NMHDR *)lParam;
	TOOLTIPTEXT *pTT = (TOOLTIPTEXT *)lParam;
	int len;

	if(ToolTipString != NULL){
		GlobalFree(ToolTipString);
		ToolTipString = NULL;
	}

	pTT->hinst = NULL;
	len = SendDlgItemMessage(hWnd, WWWC_SB, SB_GETTEXTLENGTH, (WPARAM)CForm->idFrom, 0);
	if(len <= 0){
		pTT->lpszText = NULL;
		return 0;
	}
	ToolTipString = (char *)GlobalAlloc(GPTR, len + 1);
	if(ToolTipString == NULL){
		pTT->lpszText = NULL;
		return 0;
	}
	SendDlgItemMessage(hWnd, WWWC_SB, SB_GETTEXT, (WPARAM)CForm->idFrom, (LPARAM)ToolTipString);
	pTT->lpszText = ToolTipString;
	return 1;
}
/* End of source */
