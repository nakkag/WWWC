/**************************************************************************

	WWWC

	Protocol.c

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

#define PROTOCOL_LISTCOL_SIZE			100


/**************************************************************************
	Global Variables
**************************************************************************/

HWND ProtocolWnd;

struct TP_PROTOCOL *tpProtocol;
int ProtocolCnt = 0;

//外部参照
extern HINSTANCE g_hinst;				//アプリケーションのインスタンスハンドル
extern HWND WWWCWnd;					//本体
extern int gCheckFlag;

extern int LvLIconSize;
extern int LvSIconSize;


/******************************************************************************

	GetProtocolIndex

	URLからプロトコル情報のインデックスを取得

******************************************************************************/

int GetProtocolIndex(char *URL)
{
	int i;
	char *p, *r, buf[BUFSIZE];

	if(URL == NULL){
		return -1;
	}
	for(i = 0;i < ProtocolCnt;i++){
		p = tpProtocol[i].scheme;
		r = buf;
		while(1){
			if(*p == '\t' || *p == '\0'){
				*r = '\0';

				if(lstrcmpni(URL, buf, lstrlen(buf)) == 0){
					return i;
				}

				for(;*p == '\t' && *p != '\0';p++);
				if(*p == '\0'){
					break;
				}

				r = buf;
			}
			*(r++) = *(p++);
		}
	}
	return -1;
}


/******************************************************************************

	FreeProtocol

	プロトコル情報を解放

******************************************************************************/

void FreeProtocol(void)
{
	int i;

	//プロトコル情報を解放
	for(i = 0;i < ProtocolCnt;i++){
		if(tpProtocol[i].lib != NULL){
			FreeLibrary(tpProtocol[i].lib);
		}
		if(tpProtocol[i].tpMenu != NULL){
			GlobalFree(tpProtocol[i].tpMenu);
			tpProtocol[i].tpMenu = NULL;
			tpProtocol[i].tpMenuCnt = 0;
		}
	}
	if(tpProtocol != NULL){
		GlobalFree(tpProtocol);
		tpProtocol = NULL;
	}
	ProtocolCnt = 0;
}


/******************************************************************************

	SetProtocolInfo

	プロトコル情報の設定

******************************************************************************/

void SetProtocolInfo(void)
{
	struct TPPROTOCOLINFO tpProtocolInfo;
	FARPROC Func_GetInfo;
	char buf[BUFSIZE];
	int i;
	int ret;

	for(i = 0;i < ProtocolCnt;i++){
		tpProtocol[i].lib = LoadLibrary(tpProtocol[i].DLL);

		if(tpProtocol[i].lib == NULL){
			continue;
		}
		wsprintf(buf, "%sGetInfo", tpProtocol[i].FuncHeader);
		Func_GetInfo = GetProcAddress((HMODULE)tpProtocol[i].lib, buf);
		if(Func_GetInfo != NULL){
			tpProtocolInfo.iSize = sizeof(struct TPPROTOCOLINFO);
			ret = Func_GetInfo(&tpProtocolInfo);
			if(ret != -1){
				lstrcpy(tpProtocol[i].scheme, tpProtocolInfo.scheme);
				lstrcpy(tpProtocol[i].NewMenu, tpProtocolInfo.NewMenu);
				lstrcpy(tpProtocol[i].FileType, tpProtocolInfo.FileType);
				tpProtocol[i].tpMenu = tpProtocolInfo.tpMenu;
				tpProtocol[i].tpMenuCnt = tpProtocolInfo.tpMenuCnt;

			}
		}
	}
}


/******************************************************************************

	EndProtocolNotify

	プロトコル毎に終了通知を行う

******************************************************************************/

void EndProtocolNotify(void)
{
	int i;
	char buf[BUFSIZE];
	int ret;
	FARPROC Func_EndNotify;

	for(i = 0;i < ProtocolCnt;i++){
		if(tpProtocol[i].lib == NULL){
			continue;
		}
		wsprintf(buf, "%sEndNotify", tpProtocol[i].FuncHeader);
		Func_EndNotify = GetProcAddress((HMODULE)tpProtocol[i].lib, buf);
		if(Func_EndNotify != NULL){
			ret = Func_EndNotify();
		}
	}
}


/******************************************************************************

	SetProtocolImage

	プロトコル毎のアイコンを設定する

******************************************************************************/

void SetProtocolImage(HWND hWnd)
{
	HIMAGELIST IconList_NORMAL, IconList_SMALL, TmpIconList;
	HICON hLIcon, hSIcon, TmpIcon;
	int i;
	char IconPath[BUFSIZE];

	//イメージリストの取得
	IconList_NORMAL = ListView_GetImageList(GetDlgItem(hWnd, WWWC_LIST), LVSIL_NORMAL);
	IconList_SMALL = ListView_GetImageList(GetDlgItem(hWnd, WWWC_LIST), LVSIL_SMALL);

	for(i = 0;i < ProtocolCnt;i++){
		//アイコンの取得
		lstrcpy(IconPath, tpProtocol[i].IconFile);
		if(*IconPath == '\0'){
			lstrcpy(IconPath, tpProtocol[i].DLL);
		}
		ExtractIconEx(IconPath, tpProtocol[i].IconIndex, &hLIcon, &TmpIcon, 1);
		if(LvLIconSize < LICONSIZE){
			DestroyIcon(hLIcon);
			hLIcon = TmpIcon;
		}else{
			DestroyIcon(TmpIcon);
		}
		if(hLIcon == NULL){
			hLIcon = (HICON)LoadImage(g_hinst, MAKEINTRESOURCE(IDI_ICON_NO), IMAGE_ICON, LvLIconSize, LvLIconSize, 0);
		}
		ExtractIconEx(IconPath, tpProtocol[i].IconIndex, &TmpIcon, &hSIcon, 1);
		if(LvSIconSize >= LICONSIZE){
			DestroyIcon(hSIcon);
			hSIcon = TmpIcon;
		}else{
			DestroyIcon(TmpIcon);
		}
		if(hSIcon == NULL){
			hSIcon = (HICON)LoadImage(g_hinst, MAKEINTRESOURCE(IDI_ICON_NO), IMAGE_ICON, LvSIconSize, LvSIconSize, 0);
		}
		//プロトコルのアイコンの位置を設定
		tpProtocol[i].Icon = ImageList_GetImageCount(IconList_NORMAL);
		//標準アイコン
		ImageList_AddIcon(IconList_NORMAL, hLIcon);
		ImageList_AddIcon(IconList_SMALL, hSIcon);

		//チェック待ちアイコン
		TmpIconList = ImageList_Merge(IconList_NORMAL, tpProtocol[i].Icon, IconList_NORMAL, ICON_WAIT, 0, 0);
		TmpIcon = ImageList_GetIcon(TmpIconList, 0, ILD_NORMAL);
		ImageList_AddIcon(IconList_NORMAL, TmpIcon);
		DestroyIcon(TmpIcon);
		ImageList_Destroy((void *)TmpIconList);

		TmpIconList = ImageList_Merge(IconList_SMALL, tpProtocol[i].Icon, IconList_SMALL, ICON_WAIT, 0, 0);
		TmpIcon = ImageList_GetIcon(TmpIconList, 0, ILD_NORMAL);
		ImageList_AddIcon(IconList_SMALL, TmpIcon);
		DestroyIcon(TmpIcon);
		ImageList_Destroy((void *)TmpIconList);

		//チェック中アイコン
		TmpIconList = ImageList_Merge(IconList_NORMAL, tpProtocol[i].Icon, IconList_NORMAL, ICON_CHECK, 0, 0);
		TmpIcon = ImageList_GetIcon(TmpIconList, 0, ILD_NORMAL);
		ImageList_AddIcon(IconList_NORMAL, TmpIcon);
		DestroyIcon(TmpIcon);
		ImageList_Destroy((void *)TmpIconList);

		TmpIconList = ImageList_Merge(IconList_SMALL, tpProtocol[i].Icon, IconList_SMALL, ICON_CHECK, 0, 0);
		TmpIcon = ImageList_GetIcon(TmpIconList, 0, ILD_NORMAL);
		ImageList_AddIcon(IconList_SMALL, TmpIcon);
		DestroyIcon(TmpIcon);
		ImageList_Destroy((void *)TmpIconList);

		//アイコンの破棄
		DestroyIcon(hLIcon);
		DestroyIcon(hSIcon);

		//UPアイコンの取得
		if(tpProtocol[i].UpIconIndex == -1){
			ImageList_AddIcon(IconList_NORMAL, ImageList_GetIcon(IconList_NORMAL, ICON_UP, ILD_NORMAL));
			ImageList_AddIcon(IconList_SMALL, ImageList_GetIcon(IconList_SMALL, ICON_UP, ILD_NORMAL));

		}else{
			lstrcpy(IconPath, tpProtocol[i].UpIconFile);
			if(*IconPath == '\0'){
				lstrcpy(IconPath, tpProtocol[i].DLL);
			}
			ExtractIconEx(IconPath, tpProtocol[i].UpIconIndex, &hLIcon, &TmpIcon, 1);
			if(LvLIconSize < LICONSIZE){
				DestroyIcon(hLIcon);
				hLIcon = TmpIcon;
			}else{
				DestroyIcon(TmpIcon);
			}
			if(hLIcon == NULL){
				hLIcon = (HICON)ImageList_GetIcon(IconList_NORMAL, ICON_UP, ILD_NORMAL);
			}
			ExtractIconEx(IconPath, tpProtocol[i].UpIconIndex, &TmpIcon, &hSIcon, 1);
			if(LvSIconSize >= LICONSIZE){
				DestroyIcon(hSIcon);
				hSIcon = TmpIcon;
			}else{
				DestroyIcon(TmpIcon);
			}
			if(hSIcon == NULL){
				hSIcon = (HICON)ImageList_GetIcon(IconList_SMALL, ICON_UP, ILD_NORMAL);
			}

			ImageList_AddIcon(IconList_NORMAL, hLIcon);
			ImageList_AddIcon(IconList_SMALL, hSIcon);
		}
	}
}


/******************************************************************************

	SetProtocolListIcon

	プロトコル設定画面のアイコンを設定する

******************************************************************************/

static void SetProtocolListIcon(HWND hListView)
{
	HIMAGELIST IconList;
	int PCnt, i;
	char buf[BUFSIZE];
	char dll[BUFSIZE];
	char IconFile[BUFSIZE];
	int IconIndex;
	char IconPath[BUFSIZE];
	HICON hLIcon, hSIcon;

	IconList = ListView_GetImageList(hListView, LVSIL_SMALL);

	ImageList_Remove(IconList, -1);

	PCnt = ListView_GetItemCount(hListView);

	for(i = 0;i < PCnt;i++){
		*dll = '\0';
		*IconFile = '\0';
		*buf = '\0';
		ListView_GetItemText(hListView, i, 1, dll, BUFSIZE - 1);
		ListView_GetItemText(hListView, i, 3, IconFile, BUFSIZE - 1);
		ListView_GetItemText(hListView, i, 4, buf, BUFSIZE - 1);
		IconIndex = atoi(buf);

		lstrcpy(IconPath, IconFile);
		if(*IconPath == '\0'){
			lstrcpy(IconPath, dll);
		}
		ExtractIconEx(IconPath, IconIndex, &hLIcon, &hSIcon, 1);
		if(hSIcon == NULL){
			hSIcon = (HICON)LoadImage(g_hinst, MAKEINTRESOURCE(IDI_ICON_NO), IMAGE_ICON, SICONSIZE, SICONSIZE, 0);
		}

		ImageList_AddIcon(IconList, hSIcon);
		DestroyIcon(hSIcon);
		DestroyIcon(hLIcon);

		ListView_SetItemIcon(hListView, i, i);

		ListView_RedrawItems(hListView, i, i);
	}
	UpdateWindow(hListView);
}


/******************************************************************************

	ListView_AddProtocol

	プロトコル設定画面にプロトコルを追加する

******************************************************************************/

static void ListView_AddProtocol(HWND hListView, struct TP_PROTOCOL *tpProtocolInfo)
{
	LV_ITEM lvi;
	char buf[BUFSIZE];
	int i;

	i = ListView_GetItemCount(hListView);

	lvi.mask = LVIF_TEXT;

	lvi.iItem = i;
	lvi.iSubItem = 0;
	lvi.pszText = tpProtocolInfo->title;
	lvi.cchTextMax = BUFSIZE - 1;
	lvi.iImage = 0;
	lvi.lParam = 0;

	ListView_InsertItem(hListView, &lvi);

	ListView_SetItemText(hListView, i, 1, tpProtocolInfo->DLL);
	ListView_SetItemText(hListView, i, 2, tpProtocolInfo->FuncHeader);
	ListView_SetItemText(hListView, i, 3, tpProtocolInfo->IconFile);
	wsprintf(buf, "%ld", tpProtocolInfo->IconIndex);
	ListView_SetItemText(hListView, i, 4, buf);
	ListView_SetItemText(hListView, i, 5, tpProtocolInfo->UpIconFile);
	wsprintf(buf, "%ld", tpProtocolInfo->UpIconIndex);
	ListView_SetItemText(hListView, i, 6, buf);
}


/******************************************************************************

	SetProtocolListProc

	DLL内のプロトコル一覧を表示するウィンドウプロシージャ

******************************************************************************/

static BOOL CALLBACK SetProtocolListProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	struct TP_PROTOCOL tpProtocolInfo;
	struct TPPROTOCOLSET tpProtocolSet;
	FARPROC Func_GetProtocolList;
	HINSTANCE hLib;
	HWND ParamLvWnd;
	char buf[BUFSIZE];
	int ret, i;
	int cnt = 0;

	switch (uMsg)
	{
	case WM_INITDIALOG:
		if(FileSelect(hDlg, "", PROTOCOL_FILEFILTER, NULL, buf, NULL, 1) == -1){
			EndDialog(hDlg, FALSE);
			break;
		}
		SetWindowText(GetDlgItem(hDlg, IDC_STATIC_FILENAME), buf);

		hLib = LoadLibrary(buf);
		if(hLib == NULL){
			ErrMsg(hDlg, GetLastError(), NULL);
			EndDialog(hDlg, FALSE);
			break;
		}

		//プロトコル情報取得関数のアドレスを取得
		Func_GetProtocolList = GetProcAddress((HMODULE)hLib, "GetProtocolList");
		if(Func_GetProtocolList == NULL){
			ErrMsg(hDlg, GetLastError(), NULL);
			FreeLibrary(hLib);
			EndDialog(hDlg, FALSE);
			break;
		}

		//プロトコル情報を取得
		while(1){
			ZeroMemory(&tpProtocolSet, sizeof(struct TPPROTOCOLSET));
			tpProtocolSet.iSize = sizeof(struct TPPROTOCOLSET);
			ret = Func_GetProtocolList(cnt++, &tpProtocolSet);
			if(ret <= -1){
				break;
			}
			SendDlgItemMessage(hDlg, IDC_LIST_DLLENTRY, LB_ADDSTRING, 0, (LPARAM)tpProtocolSet.Title);
		}
		FreeLibrary(hLib);
		break;

	case WM_CLOSE:
		EndDialog(hDlg, FALSE);
		break;

	case WM_COMMAND:
		if(HIWORD(wParam) == LBN_DBLCLK){
			//リストで項目がダブルクリックされた場合
			if((HWND)lParam == GetDlgItem(hDlg, IDC_LIST_DLLENTRY)){
				SendMessage(hDlg, WM_COMMAND, IDOK, 0);
			}
			break;
		}
		switch(wParam)
		{
		case IDOK:
			if(SendDlgItemMessage(hDlg, IDC_LIST_DLLENTRY, LB_GETSELCOUNT, 0, 0) <= 0){
				MessageBox(hDlg, EMSG_SELECT_PROTOCOL, EMSG_SELECT_TITLE_PROTOCOL, MB_OK | MB_ICONEXCLAMATION);
				break;
			}

			GetWindowText(GetDlgItem(hDlg, IDC_STATIC_FILENAME), tpProtocolInfo.DLL, BUFSIZE - 1);
			hLib = LoadLibrary(tpProtocolInfo.DLL);
			if(hLib == NULL){
				ErrMsg(hDlg, GetLastError(), NULL);
				EndDialog(hDlg, FALSE);
				break;
			}

			//プロトコル情報取得関数のアドレスを取得
			Func_GetProtocolList = GetProcAddress((HMODULE)hLib, "GetProtocolList");
			if(Func_GetProtocolList == NULL){
				ErrMsg(hDlg, GetLastError(), NULL);
				FreeLibrary(hLib);
				EndDialog(hDlg, FALSE);
				break;
			}

			//親ウィンドウ(プロパティシート内)のListViewのハンドルを取得
			ParamLvWnd = GetDlgItem(PropSheet_GetCurrentPageHwnd(GetParent(hDlg)), IDC_LIST_PROTOCOL);

			for(i = 0;i < SendDlgItemMessage(hDlg, IDC_LIST_DLLENTRY, LB_GETCOUNT, 0, 0);i++){
				if(SendDlgItemMessage(hDlg, IDC_LIST_DLLENTRY, LB_GETSEL, i, 0) == 0){
					continue;
				}
				ZeroMemory(&tpProtocolSet, sizeof(struct TPPROTOCOLSET));
				tpProtocolSet.iSize = sizeof(struct TPPROTOCOLSET);
				tpProtocolSet.UpIconIndex = -1;

				//プロトコル情報を取得
				ret = Func_GetProtocolList(i, &tpProtocolSet);
				if(ret <= -1){
					break;
				}
				lstrcpy(tpProtocolInfo.title, tpProtocolSet.Title);
				lstrcpy(tpProtocolInfo.FuncHeader, tpProtocolSet.FuncHeader);
				lstrcpy(tpProtocolInfo.IconFile, tpProtocolSet.IconFile);
				tpProtocolInfo.IconIndex = tpProtocolSet.IconIndex;
				lstrcpy(tpProtocolInfo.UpIconFile, tpProtocolSet.UpIconFile);
				tpProtocolInfo.UpIconIndex = tpProtocolSet.UpIconIndex;

				//プロトコル情報をリストビューに追加
				ListView_AddProtocol(ParamLvWnd, &tpProtocolInfo);
			}
			FreeLibrary(hLib);
			EndDialog(hDlg, TRUE);
			break;

		case IDCANCEL:
			EndDialog(hDlg, FALSE);
			break;
		}
		break;

	default:
		return(FALSE);
	}
	return(TRUE);
}


/******************************************************************************

	SetProtocolEditProc

	プロトコル情報設定画面のプロシージャ

******************************************************************************/

static BOOL CALLBACK SetProtocolEditProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int i;
	char buf[BUFSIZE];
	HWND pWnd;

	switch (uMsg)
	{
	case WM_INITDIALOG:
		//スピンコントロールの設定
		SendDlgItemMessage(hDlg, IDC_SPIN_ICONINDEX, UDM_SETRANGE, 0, (LPARAM)MAKELONG((short)100, (short)0));
		SendDlgItemMessage(hDlg, IDC_SPIN_UPICONINDEX, UDM_SETRANGE, 0, (LPARAM)MAKELONG((short)100, (short)-1));

		//親ウィンドウの取得
		pWnd = PropSheet_GetCurrentPageHwnd(GetParent(hDlg));

		if((i = ListView_GetNextItem(GetDlgItem(pWnd, IDC_LIST_PROTOCOL), -1, LVNI_SELECTED)) == -1){
			EndDialog(hDlg, FALSE);
		}
		*buf = '\0';
		ListView_GetItemText(GetDlgItem(pWnd, IDC_LIST_PROTOCOL), i, 0, buf, BUFSIZE - 1);
		SendDlgItemMessage(hDlg, IDC_EDIT_TITLE, WM_SETTEXT, 0, (LPARAM)buf);

		*buf = '\0';
		ListView_GetItemText(GetDlgItem(pWnd, IDC_LIST_PROTOCOL), i, 1, buf, BUFSIZE - 1);
		SendDlgItemMessage(hDlg, IDC_EDIT_DLL, WM_SETTEXT, 0, (LPARAM)buf);

		*buf = '\0';
		ListView_GetItemText(GetDlgItem(pWnd, IDC_LIST_PROTOCOL), i, 2, buf, BUFSIZE - 1);
		SendDlgItemMessage(hDlg, IDC_EDIT_HEAD, WM_SETTEXT, 0, (LPARAM)buf);

		*buf = '\0';
		ListView_GetItemText(GetDlgItem(pWnd, IDC_LIST_PROTOCOL), i, 3, buf, BUFSIZE - 1);
		SendDlgItemMessage(hDlg, IDC_EDIT_ICONFILE, WM_SETTEXT, 0, (LPARAM)buf);

		*buf = '\0';
		ListView_GetItemText(GetDlgItem(pWnd, IDC_LIST_PROTOCOL), i, 4, buf, BUFSIZE - 1);
		SendDlgItemMessage(hDlg, IDC_EDIT_ICONINDEX, WM_SETTEXT, 0, (LPARAM)buf);

		*buf = '\0';
		ListView_GetItemText(GetDlgItem(pWnd, IDC_LIST_PROTOCOL), i, 5, buf, BUFSIZE - 1);
		SendDlgItemMessage(hDlg, IDC_EDIT_UPICONFILE, WM_SETTEXT, 0, (LPARAM)buf);

		*buf = '\0';
		ListView_GetItemText(GetDlgItem(pWnd, IDC_LIST_PROTOCOL), i, 6, buf, BUFSIZE - 1);
		SendDlgItemMessage(hDlg, IDC_EDIT_UPICONINDEX, WM_SETTEXT, 0, (LPARAM)buf);
		break;

	case WM_CLOSE:
		EndDialog(hDlg, FALSE);
		break;

	case WM_COMMAND:
		switch(wParam)
		{
		case IDOK:
			pWnd = PropSheet_GetCurrentPageHwnd(GetParent(hDlg));

			if((i = ListView_GetNextItem(GetDlgItem(pWnd, IDC_LIST_PROTOCOL), -1, LVNI_SELECTED)) == -1){
				EndDialog(hDlg, FALSE);
			}

			SendDlgItemMessage(hDlg, IDC_EDIT_TITLE, WM_GETTEXT, BUFSIZE - 1, (LPARAM)buf);
			ListView_SetItemText(GetDlgItem(pWnd, IDC_LIST_PROTOCOL), i, 0, buf);

			SendDlgItemMessage(hDlg, IDC_EDIT_DLL, WM_GETTEXT, BUFSIZE - 1, (LPARAM)buf);
			ListView_SetItemText(GetDlgItem(pWnd, IDC_LIST_PROTOCOL), i, 1, buf);

			SendDlgItemMessage(hDlg, IDC_EDIT_HEAD, WM_GETTEXT, BUFSIZE - 1, (LPARAM)buf);
			ListView_SetItemText(GetDlgItem(pWnd, IDC_LIST_PROTOCOL), i, 2, buf);

			SendDlgItemMessage(hDlg, IDC_EDIT_ICONFILE, WM_GETTEXT, BUFSIZE - 1, (LPARAM)buf);
			ListView_SetItemText(GetDlgItem(pWnd, IDC_LIST_PROTOCOL), i, 3, buf);

			SendDlgItemMessage(hDlg, IDC_EDIT_ICONINDEX, WM_GETTEXT, BUFSIZE - 1, (LPARAM)buf);
			ListView_SetItemText(GetDlgItem(pWnd, IDC_LIST_PROTOCOL), i, 4, buf);

			SendDlgItemMessage(hDlg, IDC_EDIT_UPICONFILE, WM_GETTEXT, BUFSIZE - 1, (LPARAM)buf);
			ListView_SetItemText(GetDlgItem(pWnd, IDC_LIST_PROTOCOL), i, 5, buf);

			SendDlgItemMessage(hDlg, IDC_EDIT_UPICONINDEX, WM_GETTEXT, BUFSIZE - 1, (LPARAM)buf);
			ListView_SetItemText(GetDlgItem(pWnd, IDC_LIST_PROTOCOL), i, 6, buf);

			EndDialog(hDlg, TRUE);
			break;

		case IDCANCEL:
			EndDialog(hDlg, FALSE);
			break;

		//DLLファイル選択
		case IDC_BUTTON_DLLSELECT:
			SetFocus(GetDlgItem(hDlg, IDC_EDIT_DLL));
			if(FileSelect(hDlg, "", PROTOCOL_FILEFILTER, NULL, buf, NULL, 1) == -1){
				break;
			}
			SendDlgItemMessage(hDlg, IDC_EDIT_DLL, WM_SETTEXT, 0, (LPARAM)buf);
			break;

		//アイコン選択
		case IDC_BUTTON_ICONSELECT:
			//アイコンファイル名とインデックスをダイアログから取得
			*buf = '\0';
			SendDlgItemMessage(hDlg, IDC_EDIT_ICONINDEX, WM_GETTEXT, BUFSIZE - 1, (LPARAM)buf);
			i = atoi(buf);
			*buf = '\0';
			SendDlgItemMessage(hDlg, IDC_EDIT_ICONFILE, WM_GETTEXT, BUFSIZE - 1, (LPARAM)buf);
			if(*buf == '\0'){
				SendDlgItemMessage(hDlg, IDC_EDIT_DLL, WM_GETTEXT, BUFSIZE - 1, (LPARAM)buf);
			}

			SetFocus(GetDlgItem(hDlg, IDC_EDIT_ICONFILE));
			//アイコン選択ダイアログを表示
			if((i = SelectIcon(hDlg, buf, i)) == -1){
				break;
			}

			//アイコンファイル名とインデックスをダイアログに設定
			SendDlgItemMessage(hDlg, IDC_EDIT_ICONFILE, WM_SETTEXT, 0, (LPARAM)buf);
			wsprintf(buf, "%d", i);
			SendDlgItemMessage(hDlg, IDC_EDIT_ICONINDEX, WM_SETTEXT, 0, (LPARAM)buf);
			break;

		//UPアイコン選択
		case IDC_BUTTON_UPICONSELECT:
			//アイコンファイル名とインデックスをダイアログから取得
			*buf = '\0';
			SendDlgItemMessage(hDlg, IDC_EDIT_UPICONINDEX, WM_GETTEXT, BUFSIZE - 1, (LPARAM)buf);
			i = atoi(buf);
			*buf = '\0';
			SendDlgItemMessage(hDlg, IDC_EDIT_UPICONFILE, WM_GETTEXT, BUFSIZE - 1, (LPARAM)buf);
			if(*buf == '\0'){
				SendDlgItemMessage(hDlg, IDC_EDIT_DLL, WM_GETTEXT, BUFSIZE - 1, (LPARAM)buf);
			}

			SetFocus(GetDlgItem(hDlg, IDC_EDIT_UPICONFILE));
			//アイコン選択ダイアログを表示
			if((i = SelectIcon(hDlg, buf, i)) == -1){
				break;
			}

			//アイコンファイル名とインデックスをダイアログに設定
			SendDlgItemMessage(hDlg, IDC_EDIT_UPICONFILE, WM_SETTEXT, 0, (LPARAM)buf);
			wsprintf(buf, "%d", i);
			SendDlgItemMessage(hDlg, IDC_EDIT_UPICONINDEX, WM_SETTEXT, 0, (LPARAM)buf);
			break;
		}
		break;

	default:
		return(FALSE);
	}
	return(TRUE);
}


/******************************************************************************

	SetProtocolProc

	プロトコル情報画面のプロシージャ

******************************************************************************/

BOOL CALLBACK SetProtocolProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LV_COLUMN lvc;
	FARPROC Func_ProtocolProperty;
	HMODULE hLib;
	HIMAGELIST IconList;
	char buf[BUFSIZE];
	char tmp1[BUFSIZE];
	char tmp2[BUFSIZE];
	int SelectItem;
	int i;
#ifdef OP_XP_STYLE
	static long hThemeUp, hThemeDown;
#endif	//OP_XP_STYLE

	switch (uMsg)
	{
	case WM_INITDIALOG:
		ProtocolWnd = hDlg;
#ifdef OP_XP_STYLE
		//XP
		hThemeUp = open_theme(GetDlgItem(hDlg, IDC_BUTTON_UP), L"SCROLLBAR");
		hThemeDown = open_theme(GetDlgItem(hDlg, IDC_BUTTON_DOWN), L"SCROLLBAR");
#endif	//OP_XP_STYLE
		//リストビューのカラムの設定
		i = 0;
		lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;

		lvc.fmt = LVCFMT_LEFT;
		lvc.cx = PROTOCOL_LISTCOL_SIZE;
		lvc.pszText = PROTOCOL_LISTCOL_TITLE;
		lvc.iSubItem = i;
		ListView_InsertColumn(GetDlgItem(hDlg, IDC_LIST_PROTOCOL), i, &lvc);

		i++;
		lvc.fmt = LVCFMT_LEFT;
		lvc.cx = PROTOCOL_LISTCOL_SIZE;
		lvc.pszText = PROTOCOL_LISTCOL_DLL;
		lvc.iSubItem = i;
		ListView_InsertColumn(GetDlgItem(hDlg, IDC_LIST_PROTOCOL), i, &lvc);

		i++;
		lvc.fmt = LVCFMT_LEFT;
		lvc.cx = PROTOCOL_LISTCOL_SIZE;
		lvc.pszText = PROTOCOL_LISTCOL_FUNCHEAD;
		lvc.iSubItem = i;
		ListView_InsertColumn(GetDlgItem(hDlg, IDC_LIST_PROTOCOL), i, &lvc);

		i++;
		lvc.fmt = LVCFMT_LEFT;
		lvc.cx = PROTOCOL_LISTCOL_SIZE;
		lvc.pszText = PROTOCOL_LISTCOL_ICONFILE;
		lvc.iSubItem = i;
		ListView_InsertColumn(GetDlgItem(hDlg, IDC_LIST_PROTOCOL), i, &lvc);

		i++;
		lvc.fmt = LVCFMT_LEFT;
		lvc.cx = PROTOCOL_LISTCOL_SIZE;
		lvc.pszText = PROTOCOL_LISTCOL_ICONINDEX;
		lvc.iSubItem = i;
		ListView_InsertColumn(GetDlgItem(hDlg, IDC_LIST_PROTOCOL), i, &lvc);

		i++;
		lvc.fmt = LVCFMT_LEFT;
		lvc.cx = PROTOCOL_LISTCOL_SIZE;
		lvc.pszText = PROTOCOL_LISTCOL_UPICONFILE;
		lvc.iSubItem = i;
		ListView_InsertColumn(GetDlgItem(hDlg, IDC_LIST_PROTOCOL), i, &lvc);

		i++;
		lvc.fmt = LVCFMT_LEFT;
		lvc.cx = PROTOCOL_LISTCOL_SIZE;
		lvc.pszText = PROTOCOL_LISTCOL_UPICONINDEX;
		lvc.iSubItem = i;
		ListView_InsertColumn(GetDlgItem(hDlg, IDC_LIST_PROTOCOL), i, &lvc);

		//リストビューのスタイルの設定
		SetWindowLong(GetDlgItem(hDlg, IDC_LIST_PROTOCOL), GWL_STYLE,
			GetWindowLong(GetDlgItem(hDlg, IDC_LIST_PROTOCOL), GWL_STYLE) | LVS_SHOWSELALWAYS);
		SendDlgItemMessage(hDlg, IDC_LIST_PROTOCOL, LVM_SETEXTENDEDLISTVIEWSTYLE, 0,
			LVS_EX_ONECLICKACTIVATE | LVS_EX_FULLROWSELECT |
			SendDlgItemMessage(hDlg, IDC_LIST_PROTOCOL, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0));

		IconList = ImageList_Create(SICONSIZE, SICONSIZE, ILC_COLOR16 | ILC_MASK, 0, 0);
		ImageList_SetBkColor(IconList, GetSysColor(COLOR_WINDOW));
		ListView_SetImageList(GetDlgItem(hDlg, IDC_LIST_PROTOCOL), IconList, LVSIL_SMALL);

		ListView_SetRedraw(GetDlgItem(hDlg, IDC_LIST_PROTOCOL), LDF_NODRAW);
		//プロトコル情報をリストビューに追加
		if(ProtocolCnt != 0){
			ListView_SetItemCount(GetDlgItem(hDlg, IDC_LIST_PROTOCOL), ProtocolCnt);
			for(i = 0;i < ProtocolCnt;i++){
				ListView_AddProtocol(GetDlgItem(hDlg, IDC_LIST_PROTOCOL), &(tpProtocol[i]));
			}
		}
		SetProtocolListIcon(GetDlgItem(hDlg, IDC_LIST_PROTOCOL));
		ListView_SetRedraw(GetDlgItem(hDlg, IDC_LIST_PROTOCOL), LDF_REDRAW);
		SendMessage(hDlg, WM_LV_EVENT, LVN_ITEMCHANGED, 0);
		break;

	case WM_DESTROY:
#ifdef OP_XP_STYLE
		if(hThemeUp != 0 && hThemeDown != 0){
			close_theme(hThemeUp);
			close_theme(hThemeDown);
		}
#endif	//OP_XP_STYLE
		break;

	case WM_DRAWITEM:
		//描画するフレームコントロールスタイルを設定
		switch((UINT)wParam)
		{
		case IDC_BUTTON_UP:
			i = DFCS_SCROLLUP;
			break;

		case IDC_BUTTON_DOWN:
			i = DFCS_SCROLLDOWN;
			break;

		default:
			return FALSE;
		}
		//ボタンの描画
#ifdef OP_XP_STYLE
		if(hThemeUp != 0 && hThemeDown != 0){
			draw_theme_scroll((LPDRAWITEMSTRUCT)lParam, i, (i == DFCS_SCROLLUP) ? hThemeUp : hThemeDown);
		}else{
			DrawScrollControl((LPDRAWITEMSTRUCT)lParam, i);
		}
#else	//OP_XP_STYLE
		DrawScrollControl((LPDRAWITEMSTRUCT)lParam, i);
#endif	//OP_XP_STYLE
		break;

#ifdef OP_XP_STYLE
	case WM_THEMECHANGED:
		//テーマの変更
		if(hThemeUp != 0 && hThemeDown != 0){
			close_theme(hThemeUp);
			close_theme(hThemeDown);
		}
		hThemeUp = open_theme(GetDlgItem(hDlg, IDC_BUTTON_UP), L"SCROLLBAR");
		hThemeDown = open_theme(GetDlgItem(hDlg, IDC_BUTTON_DOWN), L"SCROLLBAR");
		break;
#endif	//OP_XP_STYLE

	case WM_ADDPROTOCOL:
		ListView_AddProtocol(GetDlgItem(hDlg, IDC_LIST_PROTOCOL), (struct TP_PROTOCOL *)lParam);
		SetProtocolListIcon(GetDlgItem(hDlg, IDC_LIST_PROTOCOL));
		break;

	case WM_NOTIFY:
		if(DialogLvNotifyProc(hDlg, lParam, GetDlgItem(hDlg, IDC_LIST_PROTOCOL)) == FALSE){
			return OptionNotifyProc(hDlg, uMsg, wParam, lParam);
		}
		break;

	case WM_LV_EVENT:
		if(wParam == LVN_ITEMCHANGED){
			BOOL enable;

			enable = (ListView_GetSelectedCount(GetDlgItem(hDlg, IDC_LIST_PROTOCOL)) <= 0) ? FALSE : TRUE;
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_UP), enable);
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_DOWN), enable);
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_PEDIT), enable);
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_DELETE), enable);
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_EDIT), enable);
		}
		break;

	case WM_COMMAND:
		switch(wParam)
		{
		//アイテムを上に移動
		case IDC_BUTTON_UP:
			if((SelectItem = ListView_GetNextItem(GetDlgItem(hDlg, IDC_LIST_PROTOCOL), -1, LVNI_SELECTED)) == -1){
				break;
			}
			if(SelectItem == 0){
				break;
			}
			ListView_MoveItem(GetDlgItem(hDlg, IDC_LIST_PROTOCOL), SelectItem, -1, 7);
			SetFocus(GetDlgItem(hDlg, IDC_BUTTON_UP));
			break;

		//アイテムを下に移動
		case IDC_BUTTON_DOWN:
			if((SelectItem = ListView_GetNextItem(GetDlgItem(hDlg, IDC_LIST_PROTOCOL), -1, LVNI_SELECTED)) == -1){
				break;
			}
			if(SelectItem == ListView_GetItemCount(GetDlgItem(hDlg, IDC_LIST_PROTOCOL)) - 1){
				break;
			}
			ListView_MoveItem(GetDlgItem(hDlg, IDC_LIST_PROTOCOL), SelectItem, 1, 7);
			SetFocus(GetDlgItem(hDlg, IDC_BUTTON_DOWN));
			break;

		case IDOK:
			if(tpProtocol != NULL){
				if(gCheckFlag == 0){
					EndProtocolNotify();
				}
				FreeProtocol();
			}

			ProtocolCnt = ListView_GetItemCount(GetDlgItem(hDlg, IDC_LIST_PROTOCOL));

			tpProtocol = (struct TP_PROTOCOL *)GlobalAlloc(GPTR, sizeof(struct TP_PROTOCOL) * ProtocolCnt);
			if(tpProtocol == NULL){
				abort();
			}
			//リストビューのアイテムをプロトコル情報に設定する
			for(i = 0;i < ProtocolCnt;i++){
				ZeroMemory(&tpProtocol[i], sizeof(struct TP_PROTOCOL));
				ListView_GetItemText(GetDlgItem(hDlg, IDC_LIST_PROTOCOL), i, 0, tpProtocol[i].title, BUFSIZE - 1);
				ListView_GetItemText(GetDlgItem(hDlg, IDC_LIST_PROTOCOL), i, 1, tpProtocol[i].DLL, BUFSIZE - 1);
				ListView_GetItemText(GetDlgItem(hDlg, IDC_LIST_PROTOCOL), i, 2, tpProtocol[i].FuncHeader, BUFSIZE - 1);
				ListView_GetItemText(GetDlgItem(hDlg, IDC_LIST_PROTOCOL), i, 3, tpProtocol[i].IconFile, BUFSIZE - 1);
				ListView_GetItemText(GetDlgItem(hDlg, IDC_LIST_PROTOCOL), i, 4, buf, BUFSIZE - 1);
				tpProtocol[i].IconIndex = atoi(buf);
				ListView_GetItemText(GetDlgItem(hDlg, IDC_LIST_PROTOCOL), i, 5, tpProtocol[i].UpIconFile, BUFSIZE - 1);
				ListView_GetItemText(GetDlgItem(hDlg, IDC_LIST_PROTOCOL), i, 6, buf, BUFSIZE - 1);
				tpProtocol[i].UpIconIndex = atoi(buf);
				tpProtocol[i].Icon = 0;
			}

			//イメージリストの更新
			ListView_SetItemImage(GetDlgItem(WWWCWnd, WWWC_LIST), LvLIconSize, LVSIL_NORMAL);
			ListView_SetItemImage(GetDlgItem(WWWCWnd, WWWC_LIST), LvSIconSize, LVSIL_SMALL);
			SetProtocolImage(WWWCWnd);

			SetProtocolInfo();

		case IDPCANCEL:
			ImageList_Destroy((void *)ListView_GetImageList(GetDlgItem(hDlg, IDC_LIST_PROTOCOL), LVSIL_SMALL));
			ProtocolWnd = NULL;
			break;

		//追加
		case IDC_BUTTON_ADD:
			DialogBox(g_hinst, MAKEINTRESOURCE(IDD_DIALOG_DLLLIST), hDlg, SetProtocolListProc);
			SetProtocolListIcon(GetDlgItem(hDlg, IDC_LIST_PROTOCOL));
			break;

		//編集
		case IDC_BUTTON_PEDIT:
			if((SelectItem = ListView_GetNextItem(GetDlgItem(hDlg, IDC_LIST_PROTOCOL), -1, LVNI_SELECTED)) == -1){
				break;
			}
			DialogBox(g_hinst, MAKEINTRESOURCE(IDD_DIALOG_PRPTOCOLEDIT), hDlg, SetProtocolEditProc);
			SetProtocolListIcon(GetDlgItem(hDlg, IDC_LIST_PROTOCOL));
			break;

		//削除
		case IDC_BUTTON_DELETE:
			if((SelectItem = ListView_GetNextItem(GetDlgItem(hDlg, IDC_LIST_PROTOCOL), -1, LVNI_SELECTED)) == -1){
				break;
			}
			if(MessageBox(hDlg, QMSG_DELETE, QMSG_DELETE_TITLE, MB_ICONQUESTION | MB_YESNO) == IDNO){
				break;
			}
			ListView_DeleteItem(GetDlgItem(hDlg, IDC_LIST_PROTOCOL), SelectItem);
			ListView_SetItemState(GetDlgItem(hDlg, IDC_LIST_PROTOCOL), SelectItem,
				LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
			break;

		//プロパティ
		case IDC_BUTTON_EDIT:
			if((SelectItem = ListView_GetNextItem(GetDlgItem(hDlg, IDC_LIST_PROTOCOL), -1, LVNI_SELECTED)) == -1){
				break;
			}
			*tmp1 = '\0';
			*tmp2 = '\0';
			ListView_GetItemText(GetDlgItem(hDlg, IDC_LIST_PROTOCOL), SelectItem, 1, tmp1, BUFSIZE - 1);
			ListView_GetItemText(GetDlgItem(hDlg, IDC_LIST_PROTOCOL), SelectItem, 2, tmp2, BUFSIZE - 1);

			hLib = LoadLibrary(tmp1);
			if(hLib == NULL){
				ErrMsg(hDlg, GetLastError(), NULL);
				break;
			}
			wsprintf(buf, "%sProtocolProperty", tmp2);
			Func_ProtocolProperty = GetProcAddress((HMODULE)hLib, buf);
			if(Func_ProtocolProperty == NULL){
				ErrMsg(hDlg, GetLastError(), NULL);
				FreeLibrary(hLib);
				break;
			}
			if(Func_ProtocolProperty(hDlg) == 0){
				MessageBox(hDlg, EMSG_PROP, EMSG_PROP_TITLE, MB_OK | MB_ICONEXCLAMATION);
			}
			FreeLibrary(hLib);
			break;
		}
		break;

	//DLLからのメッセージを本体に転送
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
	case WM_ITEMEXEC:
	case WM_ITEMINIT:
		return SendMessage(WWWCWnd, uMsg, wParam, lParam);

	default:
		return FALSE;
	}
	return TRUE;
}
/* End of source */
