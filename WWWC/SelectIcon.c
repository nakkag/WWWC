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

//外部参照
extern HINSTANCE g_hinst;				//アプリケーションのインスタンスハンドル

typedef struct _ICON_INFO {
	char *path;
	int index;
} ICON_INFO;


/**************************************************************************
	Local Function Prototypes
**************************************************************************/


/******************************************************************************

	SetListIcon

	リストビューにアイコン一覧を設定

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
	//リストビューのクリア
	ListView_DeleteAllItems(hListView);

	if(path == NULL){
		SendMessage(hListView, WM_SETREDRAW, (WPARAM)TRUE, 0);
		UpdateWindow(hListView);
		return 0;
	}
	//アイコン数取得
	icon_cnt = ExtractIconEx(path, -1, NULL, NULL, 1);
	if(icon_cnt <= 0){
		SendMessage(hListView, WM_SETREDRAW, (WPARAM)TRUE, 0);
		UpdateWindow(hListView);
		return 0;
	}

	//イメージリストの作成、設定
	IconList = ListView_GetImageList(hListView, LVSIL_NORMAL);
	if(IconList == NULL){
		IconList = ImageList_Create(32, 32, ILC_COLOR16 | ILC_MASK, 0, 0);
		ImageList_SetBkColor(IconList, GetSysColor(COLOR_WINDOW));
		ListView_SetImageList(hListView, IconList, LVSIL_NORMAL);
	}else{
		ImageList_Remove(IconList, -1);
	}

	for(i = 0; i < icon_cnt; i++){
		//アイコンの抽出
		ExtractIconEx(path, i, &hIcon, &hsIcon, 1);

		if(hIcon != NULL){
			//イメージリストに追加
			ret = ImageList_AddIcon(IconList, hIcon);
			DestroyIcon(hIcon);

			//リストビューにアイテムを追加
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

	アイコン選択ウィンドウプロシージャ

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
				//変更時アイコン一覧を取得しなおす
				SendDlgItemMessage(hDlg, IDC_EDIT_FILE, WM_GETTEXT, BUFSIZE - 1, (LPARAM)buf);
				SetListIcon(GetDlgItem(hDlg, IDC_LIST_ICON), buf, -1);
				SendMessage(hDlg, WM_LV_EVENT, LVN_ITEMCHANGED, 0);
				SendDlgItemMessage(hDlg, IDC_EDIT_FILE, EM_SETMODIFY, FALSE, 0);
			}
			break;

		case IDC_BUTTON_BROWS:
			//アイコンファイル選択
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

	アイコン選択ウィンドウの表示

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
