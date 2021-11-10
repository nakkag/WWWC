/**************************************************************************

	WWWC (wwwc.dll)

	file.c

	Copyright (C) 1996-2003 by Nakashima Tomoaki. All rights reserved.
		http://www.nakka.com/
		nakka@nakka.com

**************************************************************************/


/**************************************************************************
	Include Files
**************************************************************************/

#define _INC_OLE
#include <windows.h>
#undef  _INC_OLE

#include "String.h"
#include "http.h"
#include "StrTbl.h"
#include "wwwcdll.h"

#include "resource.h"


/**************************************************************************
	Define
**************************************************************************/

#define BUFSIZE					256

#define NOCMP_SIZE				1
#define NOCMP_DATE				2


/**************************************************************************
	Global Variables
**************************************************************************/

extern HINSTANCE ghinst;

extern char DateFormat[];
extern char TimeFormat[];

int LresultCmp(char *Oldbuf, char *Newbuf);
int ExecItem(HWND hWnd, char *buf, char *cmdline);


/******************************************************************************

	FILE_InitItem

	�A�C�e���̏�����

******************************************************************************/

__declspec(dllexport) int CALLBACK FILE_InitItem(HWND hWnd, struct TPITEM *tpItemInfo)
{
	/* �X�V�}�[�N(*)���������� */
	if(tpItemInfo->Size != NULL && tpItemInfo->Size[0] != '\0' && tpItemInfo->Size[lstrlen(tpItemInfo->Size) - 1] == '*'){
		tpItemInfo->Size[lstrlen(tpItemInfo->Size) - 1] = '\0';
	}
	if(tpItemInfo->Date != NULL && tpItemInfo->Date[0] != '\0' && tpItemInfo->Date[lstrlen(tpItemInfo->Date) - 1] == '*'){
		tpItemInfo->Date[lstrlen(tpItemInfo->Date) - 1] = '\0';
	}
	return 0;
}


/******************************************************************************

	FILE_Initialize

	�`�F�b�N�J�n�̏�����

******************************************************************************/

__declspec(dllexport) int CALLBACK FILE_Initialize(HWND hWnd, struct TPITEM *tpItemInfo)
{
	return 0;
}


/******************************************************************************

	FILE_Cancel

	�`�F�b�N�̃L�����Z��

******************************************************************************/

__declspec(dllexport) int CALLBACK FILE_Cancel(HWND hWnd, struct TPITEM *tpItemInfo)
{
	return 0;
}


/******************************************************************************

	FILE_Timer

	�^�C�}�[ (1�b�Ԋu)

******************************************************************************/

__declspec(dllexport) int CALLBACK FILE_Timer(HWND hWnd, struct TPITEM *tpItemInfo)
{
	return CHECK_SUCCEED;
}


/******************************************************************************

	FILE_Start

	�`�F�b�N�̊J�n

******************************************************************************/

__declspec(dllexport) int CALLBACK FILE_Start(HWND hWnd, struct TPITEM *tpItemInfo)
{
	WIN32_FIND_DATA FindData;
	HANDLE hFindFile;
	int CmpMsg = ST_DEFAULT;
	char buf[BUFSIZE];
	char *FileName;
	FILETIME lFileTime;
	SYSTEMTIME fSysTime;
	char *p;
	char fDay[BUFSIZE];
	char fTime[BUFSIZE];

	for(FileName = tpItemInfo->CheckURL + lstrlen("file:"); *FileName == '/' && *FileName != '\0'; FileName++);

	if((hFindFile = FindFirstFile(FileName, &FindData)) != INVALID_HANDLE_VALUE){
		FindClose(hFindFile);

		//�T�C�Y�̔�r
		wsprintf(buf, "%ld", (long)FindData.nFileSizeLow);
		/* �O��`�F�b�N���̂��̂Ɣ�r���� */
		if(GetOptionInt(tpItemInfo->Option1, 1) == 0 && LresultCmp(tpItemInfo->Size, buf) != 0){
			CmpMsg = ST_UP;
		}
		/* �A�C�e���ɍ���̃`�F�b�N���e���Z�b�g���� */
		if(tpItemInfo->Size != NULL){
			GlobalFree(tpItemInfo->Size);
		}
		tpItemInfo->Size = (char *)GlobalAlloc(GPTR, sizeof(char) * lstrlen(buf) + 1);
		lstrcpy(tpItemInfo->Size, buf);

		//�X�V���̔�r
		FileTimeToLocalFileTime(&(FindData.ftLastWriteTime), &lFileTime);
		FileTimeToSystemTime(&lFileTime, &fSysTime);

		if(*DateFormat == '\0'){
			p = NULL;
		}else{
			p = DateFormat;
		}
		if(GetDateFormat(0, 0, &fSysTime, p, fDay, BUFSIZE - 1) == 0){
			SendMessage(hWnd, WM_CHECK_RESULT, ST_ERROR, (LPARAM)tpItemInfo);
			return CHECK_END;
		}
		if(*TimeFormat == '\0'){
			p = NULL;
		}else{
			p = TimeFormat;
		}
		if(GetTimeFormat(0, 0, &fSysTime, p, fTime, BUFSIZE - 1) == 0){
			SendMessage(hWnd, WM_CHECK_RESULT, ST_ERROR, (LPARAM)tpItemInfo);
			return CHECK_END;
		}
		wsprintf(buf, "%s %s", fDay, fTime);

		/* �O��`�F�b�N���̂��̂Ɣ�r���� */
		if(GetOptionInt(tpItemInfo->Option1, 0) == 0 && LresultCmp(tpItemInfo->Date, buf) != 0){
			CmpMsg = ST_UP;
		}
		/* �A�C�e���ɍ���̃`�F�b�N���e���Z�b�g���� */
		if(tpItemInfo->Date != NULL){
			GlobalFree(tpItemInfo->Date);
		}
		tpItemInfo->Date = (char *)GlobalAlloc(GPTR, sizeof(char) * lstrlen(buf) + 1);
		lstrcpy(tpItemInfo->Date,buf);
	}else{
		CmpMsg = ST_ERROR;
	}
	SendMessage(hWnd, WM_CHECK_RESULT, CmpMsg, (LPARAM)tpItemInfo);
	return CHECK_END;
}


/******************************************************************************

	FILE_GetItemText

	�N���b�v�{�[�h�p�A�C�e���̃e�L�X�g�̐ݒ�

******************************************************************************/

__declspec(dllexport) HANDLE CALLBACK FILE_GetItemText(struct TPITEM *tpItemInfo)
{
	HANDLE hMemText;
	char *buf;
	char *p;

	for(buf = tpItemInfo->CheckURL + lstrlen("file:"); *buf == '/' && *buf != '\0'; buf++);

	if((hMemText = GlobalAlloc(GHND, lstrlen(buf) + 1)) == NULL){
		return NULL;
	}
	if((p = GlobalLock(hMemText)) == NULL){
		GlobalFree(hMemText);
		return NULL;
	}
	lstrcpy(p,buf);
	GlobalUnlock(hMemText);
	return hMemText;
}


/******************************************************************************

	FILE_CreateDropItem

	�A�C�e���̃h���b�v�t�@�C���̐ݒ�

******************************************************************************/

__declspec(dllexport) BOOL CALLBACK FILE_CreateDropItem(struct TPITEM *tpItemInfo, char *fPath, char *iPath, char *ret)
{
	char *buf;

	for(buf = tpItemInfo->CheckURL + lstrlen("file:"); *buf == '/' && *buf != '\0'; buf++);
	lstrcpy(ret, buf);
	return TRUE;
}


/******************************************************************************

	FILE_ExecItem

	�A�C�e���̎��s

******************************************************************************/

__declspec(dllexport) int CALLBACK FILE_ExecItem(HWND hWnd, char *Action, struct TPITEM *tpItemInfo)
{
	char *buf = NULL;
	int rc = -1;

	if(lstrcmp(Action,"open") == 0){
		if(tpItemInfo->CheckURL != NULL){
			for(buf = tpItemInfo->CheckURL + lstrlen("file:"); *buf == '/' && *buf != '\0'; buf++);
		}
		if(buf != NULL && *buf != '\0'){
			rc = ExecItem(hWnd, buf, NULL);
		}else{
			MessageBox(hWnd, STR_ERR_MSG_FILEOPEN, STR_ERR_TITLE_FILEOPEN, MB_ICONEXCLAMATION);
			return -1;
		}
	}
	// 1 ��Ԃ��ƃA�C�R��������������B
	return rc;
}


/******************************************************************************

	FilePropertyProc

	�A�C�e���̃v���p�e�B�ݒ�_�C�A���O

******************************************************************************/

static BOOL CALLBACK FilePropertyProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	struct TPITEM *tpItemInfo;
	char buf[BUFSIZE];
	char tmp[BUFSIZE + 10];
	char *p, *r;

	switch (uMsg)
	{
	case WM_INITDIALOG:
		/* �E�B���h�E�ɃA�C�e������Ҕ����� */
		SetWindowLong(hDlg,DWL_USER,(long)lParam);

		tpItemInfo = (struct TPITEM *)lParam;
		
		/* �A�C�e���̏�񂪋�łȂ��ꍇ�̓A�C�e���̓��e��\������ */
		if(tpItemInfo->Title != NULL){
			SendMessage(GetDlgItem(hDlg, IDC_EDIT_TITLE), WM_SETTEXT, 0, (LPARAM)tpItemInfo->Title);
		}else{
			SendMessage(GetDlgItem(hDlg, IDC_EDIT_TITLE), WM_SETTEXT, 0, (LPARAM)STR_NEWITEMNAME);
		}
		if(tpItemInfo->CheckURL != NULL){
			SendMessage(GetDlgItem(hDlg, IDC_EDIT_URL), WM_SETTEXT, 0, (LPARAM)tpItemInfo->CheckURL);
		}else{
			SendMessage(GetDlgItem(hDlg, IDC_EDIT_URL), WM_SETTEXT, 0, (LPARAM)"file:///");
		}
		if(tpItemInfo->Comment != NULL){
			SendMessage(GetDlgItem(hDlg, IDC_EDIT_COMMENT), WM_SETTEXT, 0, (LPARAM)tpItemInfo->Comment);
		}else{
			SendMessage(GetDlgItem(hDlg, IDC_EDIT_COMMENT), WM_SETTEXT, 0, (LPARAM)"");
		}

		CheckDlgButton(hDlg, IDC_CHECK_DATE, !GetOptionInt(tpItemInfo->Option1, 0));
		CheckDlgButton(hDlg, IDC_CHECK_SIZE, !GetOptionInt(tpItemInfo->Option1, 1));
		break;

	case WM_CLOSE:
		EndDialog(hDlg, FALSE);
		break;

	case WM_COMMAND:
		switch(wParam)
		{
		case IDC_BUTTON_BROWS:
			if(FileSelect(hDlg, "*.*", STR_FILTER_FILESELECT, STR_TITLE_FILESELECT, buf, NULL, 1) == -1){
				break;
			}
			wsprintf(tmp, "file:///%s", buf);
			SendMessage(GetDlgItem(hDlg, IDC_EDIT_URL), WM_SETTEXT, 0, (LPARAM)tmp);

			for(r = p = buf; *p != '\0'; p++){
				if(IsDBCSLeadByte((BYTE)*p) == TRUE){
					p++;
				}else if(*p == '\\' || *p == '/'){
					r = p;
				}
			}
			SendMessage(GetDlgItem(hDlg, IDC_EDIT_TITLE), WM_SETTEXT, 0, (LPARAM)(r + 1));
			break;

		case IDOK:
			/* �E�B���h�E����A�C�e���̏����擾���� */
			tpItemInfo = (struct TPITEM *)GetWindowLong(hDlg, DWL_USER);
			
			/* �^�C�g�� */
			if(tpItemInfo->Title != NULL){
				GlobalFree(tpItemInfo->Title);
			}
			tpItemInfo->Title = GlobalAlloc(GPTR, SendMessage(GetDlgItem(hDlg, IDC_EDIT_TITLE), WM_GETTEXTLENGTH, 0, 0) + 1);
			SendMessage(GetDlgItem(hDlg, IDC_EDIT_TITLE), WM_GETTEXT,
				SendMessage(GetDlgItem(hDlg, IDC_EDIT_TITLE), WM_GETTEXTLENGTH, 0, 0) + 1, (LPARAM)tpItemInfo->Title);

			/* URL */
			if(tpItemInfo->CheckURL != NULL){
				GlobalFree(tpItemInfo->CheckURL);
			}
			tpItemInfo->CheckURL = GlobalAlloc(GPTR, SendMessage(GetDlgItem(hDlg, IDC_EDIT_URL), WM_GETTEXTLENGTH, 0, 0) + 1);
			SendMessage(GetDlgItem(hDlg, IDC_EDIT_URL), WM_GETTEXT,
				SendMessage(GetDlgItem(hDlg, IDC_EDIT_URL), WM_GETTEXTLENGTH, 0, 0) + 1, (LPARAM)tpItemInfo->CheckURL);

			/* �R�����g */
			if(tpItemInfo->Comment != NULL){
				GlobalFree(tpItemInfo->Comment);
			}
			tpItemInfo->Comment = GlobalAlloc(GPTR, SendMessage(GetDlgItem(hDlg, IDC_EDIT_COMMENT), WM_GETTEXTLENGTH, 0, 0) + 1);
			SendMessage(GetDlgItem(hDlg, IDC_EDIT_COMMENT), WM_GETTEXT,
				SendMessage(GetDlgItem(hDlg, IDC_EDIT_COMMENT), WM_GETTEXTLENGTH, 0, 0) + 1, (LPARAM)tpItemInfo->Comment);

			/* �`�F�b�N�I�v�V���� */
			if(tpItemInfo->Option1 != NULL){
				GlobalFree(tpItemInfo->Option1);
			}
			tpItemInfo->Option1 = (char *)GlobalAlloc(GPTR, 5);
			if(tpItemInfo->Option1 != NULL){
				wsprintf(tpItemInfo->Option1, "%d;;%d",
					!IsDlgButtonChecked(hDlg, IDC_CHECK_DATE),
					!IsDlgButtonChecked(hDlg, IDC_CHECK_SIZE));
			}
			EndDialog(hDlg, TRUE);
			break;

		case IDCANCEL:
			EndDialog(hDlg, FALSE);
			break;
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}


/******************************************************************************

	FILE_Property

	�A�C�e���̃v���p�e�B

******************************************************************************/

__declspec(dllexport) int CALLBACK FILE_Property(HWND hWnd, struct TPITEM *tpItemInfo)
{
	/* �v���p�e�B�̃_�C�A���O��\������ */
	if(DialogBoxParam(ghinst, MAKEINTRESOURCE(IDD_DIALOG_FILEPROP), hWnd, FilePropertyProc, (long)tpItemInfo) == FALSE){
		return -1;
	}
	return 0;
}


/******************************************************************************

	FILE_ProtocolProperty

	�v���g�R���̃v���p�e�B

******************************************************************************/

__declspec(dllexport) int CALLBACK FILE_ProtocolProperty(HWND hWnd)
{
	return 0;
}


/******************************************************************************

	FILE_GetInfo

	�v���g�R�����

******************************************************************************/

__declspec(dllexport) int CALLBACK FILE_GetInfo(struct TPPROTOCOLINFO *tpInfo)
{
	int i = 0;

	lstrcpy(tpInfo->Scheme, "file:");
	lstrcpy(tpInfo->NewMenu, STR_GETINFO_FILE_NEWMENU);
	lstrcpy(tpInfo->FileType, "");

	tpInfo->tpMenu = (struct TPPROTOCOLMENU *)GlobalAlloc(GPTR, sizeof(struct TPPROTOCOLMENU));
	tpInfo->tpMenuCnt = 1;
	
	lstrcpy(tpInfo->tpMenu[i].Name, STR_GETINFO_FILE_OPEN);
	lstrcpy(tpInfo->tpMenu[i].Action, "open");
	tpInfo->tpMenu[i].Default = TRUE;
	tpInfo->tpMenu[i].Flag = 0;
	return 0;
}


/******************************************************************************

	FILE_EndNotify

	WWWC�̏I���̒ʒm

******************************************************************************/

__declspec(dllexport) int CALLBACK FILE_EndNotify(void)
{
	return 0;
}
/* End of source */
