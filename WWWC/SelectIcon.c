/**************************************************************************

	WWWC

	SelectIcon.c

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

//�O���Q��
extern HINSTANCE g_hinst;				//�A�v���P�[�V�����̃C���X�^���X�n���h��

typedef struct _ICON_INFO {
	char *path;
	int index;
} ICON_INFO;


/**************************************************************************
	Local Function Prototypes
**************************************************************************/


/******************************************************************************

	SetListIcon

	���X�g�r���[�ɃA�C�R���ꗗ��ݒ�

******************************************************************************/

static int SetListIcon(HWND hListView, char *path, int index)
{
	HIMAGELIST IconList;
	HICON hIcon = NULL;
	HICON hsIcon = NULL;
	char buf[BUFSIZE];
	int icon_cnt;
	int ret;
	int i, j;

	SendMessage(hListView, WM_SETREDRAW, (WPARAM)FALSE, 0);
	//���X�g�r���[�̃N���A
	ListView_DeleteAllItems(hListView);

	if(path == NULL){
		SendMessage(hListView, WM_SETREDRAW, (WPARAM)TRUE, 0);
		UpdateWindow(hListView);
		return 0;
	}
	//�A�C�R�����擾
	icon_cnt = ExtractIconEx(path, -1, NULL, NULL, 1);
	if(icon_cnt <= 0){
		SendMessage(hListView, WM_SETREDRAW, (WPARAM)TRUE, 0);
		UpdateWindow(hListView);
		return 0;
	}

	//�C���[�W���X�g�̍쐬�A�ݒ�
	IconList = ListView_GetImageList(hListView, LVSIL_NORMAL);
	if(IconList == NULL){
		IconList = ImageList_Create(32, 32, ILC_COLOR16 | ILC_MASK, 0, 0);
		ImageList_SetBkColor(IconList, GetSysColor(COLOR_WINDOW));
		ListView_SetImageList(hListView, IconList, LVSIL_NORMAL);
	}else{
		ImageList_Remove(IconList, -1);
	}

	for(i = 0; i < icon_cnt; i++){
		//�A�C�R���̒��o
		ExtractIconEx(path, i, &hIcon, &hsIcon, 1);

		if(hIcon != NULL){
			//�C���[�W���X�g�ɒǉ�
			ret = ImageList_AddIcon(IconList, hIcon);
			DestroyIcon(hIcon);

			//���X�g�r���[�ɃA�C�e����ǉ�
			wsprintf(buf, "%d", i);
			j = ListView_InsertItemEx(hListView, buf, ret, ret, -1);
			if(index == ret){
				ListView_SetItemState(hListView, j, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
				ListView_EnsureVisible(hListView, j, TRUE);
			}
		}
		if(hsIcon != NULL){
			DestroyIcon(hsIcon);
		}
	}
	SendMessage(hListView, WM_SETREDRAW, (WPARAM)TRUE, 0);
	UpdateWindow(hListView);
	return icon_cnt;
}


/******************************************************************************

	SelectIconProc

	�A�C�R���I���E�B���h�E�v���V�[�W��

******************************************************************************/

static BOOL CALLBACK SelectIconProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static ICON_INFO *icon_info;
	char buf[BUFSIZE];
	int i;

	switch (uMsg)
	{
	case WM_INITDIALOG:
		icon_info = (ICON_INFO *)lParam;
		SendDlgItemMessage(hDlg, IDC_EDIT_FILE, WM_SETTEXT, 0, (LPARAM)icon_info->path);
		SetListIcon(GetDlgItem(hDlg, IDC_LIST_ICON), icon_info->path, icon_info->index);
		SendMessage(hDlg, WM_LV_EVENT, LVN_ITEMCHANGED, 0);
		SendDlgItemMessage(hDlg, IDC_EDIT_FILE, EM_SETMODIFY, FALSE, 0);
		break;

	case WM_CLOSE:
		ImageList_Destroy((void *)ListView_GetImageList(GetDlgItem(hDlg, IDC_LIST_ICON), LVSIL_NORMAL));
		EndDialog(hDlg, FALSE);
		break;

	case WM_NOTIFY:
		DialogLvNotifyProc(hDlg, lParam, GetDlgItem(hDlg, IDC_LIST_ICON));
		break;

	case WM_LV_EVENT:
		if(wParam == LVN_ITEMCHANGED){
			EnableWindow(GetDlgItem(hDlg, IDOK),
				(ListView_GetNextItem(GetDlgItem(hDlg, IDC_LIST_ICON), -1, LVNI_SELECTED) == -1) ? FALSE : TRUE);
		}
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_EDIT_FILE:
			if(HIWORD(wParam) == EN_KILLFOCUS && SendDlgItemMessage(hDlg, IDC_EDIT_FILE, EM_GETMODIFY, 0, 0) == TRUE){
				//�ύX���A�C�R���ꗗ���擾���Ȃ���
				SendDlgItemMessage(hDlg, IDC_EDIT_FILE, WM_GETTEXT, BUFSIZE - 1, (LPARAM)buf);
				SetListIcon(GetDlgItem(hDlg, IDC_LIST_ICON), buf, -1);
				SendMessage(hDlg, WM_LV_EVENT, LVN_ITEMCHANGED, 0);
				SendDlgItemMessage(hDlg, IDC_EDIT_FILE, EM_SETMODIFY, FALSE, 0);
			}
			break;

		case IDC_BUTTON_BROWS:
			//�A�C�R���t�@�C���I��
			if(FileSelect(hDlg, "", FILEFILTER_ICON, NULL, buf, NULL, 1) == -1){
				break;
			}
			SendDlgItemMessage(hDlg, IDC_EDIT_FILE, WM_SETTEXT, 0, (LPARAM)buf);
			SetListIcon(GetDlgItem(hDlg, IDC_LIST_ICON), buf, -1);
			SendMessage(hDlg, WM_LV_EVENT, LVN_ITEMCHANGED, 0);
			break;

		case IDCANCEL:
			SendMessage(hDlg, WM_CLOSE, 0, 0);
			break;

		case IDOK:
		case IDC_BUTTON_EDIT:
			if((i = ListView_GetNextItem(GetDlgItem(hDlg, IDC_LIST_ICON), -1, LVNI_SELECTED)) == -1){
				break;
			}
			SendDlgItemMessage(hDlg, IDC_EDIT_FILE, WM_GETTEXT, BUFSIZE - 1, (LPARAM)icon_info->path);
			icon_info->index = ListView_GetlParam(GetDlgItem(hDlg, IDC_LIST_ICON), i);

			ImageList_Destroy((void *)ListView_GetImageList(GetDlgItem(hDlg, IDC_LIST_ICON), LVSIL_NORMAL));
			EndDialog(hDlg, TRUE);
			break;
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}


/******************************************************************************

	SelectIcon

	�A�C�R���I���E�B���h�E�̕\��

******************************************************************************/

int SelectIcon(HWND hWnd, char *path, int index)
{
	ICON_INFO icon_info;

	icon_info.path = path;
	icon_info.index = index;

	if(DialogBoxParam(g_hinst, MAKEINTRESOURCE(IDD_DIALOG_SELECTICON), hWnd, SelectIconProc, (LPARAM)&icon_info) == FALSE){
		return -1;
	}
	return icon_info.index;
}
/* End of source */
