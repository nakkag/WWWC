/**************************************************************************

	WWWC (wwwc.dll)

	httptools.c

	Copyright (C) 1996-2026 by Ohno Tomoaki. All rights reserved.
		http://www.nakka.com/
		nakka@nakka.com

**************************************************************************/


/**************************************************************************
	Include Files
**************************************************************************/

#define _INC_OLE
#include <windows.h>
#undef  _INC_OLE
#include <commctrl.h>

#include "String.h"
#include "httptools.h"
#include "http.h"
#include "StrTbl.h"
#include "wwwcdll.h"
#include "resource.h"
#include "browser.h"


/**************************************************************************
	Global Variables
**************************************************************************/

//外部参照
extern char app_path[];
extern HINSTANCE ghinst;

extern char BrowserPath[];


/**************************************************************************
	Local Function Prototypes
**************************************************************************/

static int SetListItem(HWND hListView, char *buf, int Img, long lp, int iItem);
static LRESULT DialogLvNotifyProc(HWND hWnd, LPARAM lParam, HWND hListView);
static int GetPages(HWND hWnd, BROWSER_PAGE **pages, BOOL First);


/******************************************************************************

	GetPages

	ブラウザで表示しているページを取得する (見つからない場合はメッセージを表示)

******************************************************************************/

static int GetPages(HWND hWnd, BROWSER_PAGE **pages, BOOL First)
{
	HCURSOR hCursor;
	int cnt;

	hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
	cnt = Browser_GetPages(pages, First);
	SetCursor(hCursor);
	if(cnt <= 0){
		MessageBox(hWnd, STR_ERR_MSG_BROWSER, STR_ERR_TITLE_BROWSER, MB_ICONWARNING | MB_OK);
	}
	return cnt;
}


/******************************************************************************

	GetBrowserInfo

	最後にアクティブだったブラウザで表示しているページからアイテムを作成する

******************************************************************************/

__declspec(dllexport) int CALLBACK GetBrowserInfo(HWND hWnd, struct TPITEM **ToolItemList,
												  int ToolItemListCnt, int type, int CheckType)
{
	struct TPITEM **ItemList;
	BROWSER_PAGE *pages;
	char Title[TITLE_MAX];
	int cnt;

	//プロパティ
	if(type == TOOL_EXEC_PORP){
		MessageBox(hWnd, STR_MSG_BROWSER_INFO, STR_ERR_TITLE_BROWSER, MB_ICONINFORMATION | MB_OK);
		return 1;
	}

	//ウィンドウメニュー以外からの呼び出しの場合は処理を行わない
	if(type != TOOL_EXEC_WINDOWMENU){
		return 0;
	}

	//最後にアクティブだったブラウザのURLとタイトルを取得
	if((cnt = GetPages(hWnd, &pages, TRUE)) <= 0){
		return 0;
	}
	lstrcpyn(Title, pages[0].Title, TITLE_MAX);

	//アイテムリストを確保
	ItemList = (struct TPITEM **)GlobalAlloc(GPTR, sizeof(struct TPITEM *));
	if(ItemList == NULL){
		Browser_FreePages(pages, cnt);
		return 0;
	}
	//アイテム情報を確保
	*ItemList = (struct TPITEM *)GlobalAlloc(GPTR, sizeof(struct TPITEM));
	if(*ItemList == NULL){
		GlobalFree(ItemList);
		Browser_FreePages(pages, cnt);
		return 0;
	}

	//タイトルを設定
	(*(ItemList))->Title = (char *)GlobalAlloc(GPTR, sizeof(char) * lstrlen(Title) + 1);
	if((*(ItemList))->Title == NULL){
		GlobalFree(*ItemList);
		GlobalFree(ItemList);
		Browser_FreePages(pages, cnt);
		return 0;
	}
	lstrcpy((*(ItemList))->Title, Title);

	//URLを設定
	(*(ItemList))->CheckURL = (char *)GlobalAlloc(GPTR, sizeof(char) * lstrlen(pages[0].Url) + 1);
	if((*(ItemList))->CheckURL == NULL){
		GlobalFree((*(ItemList))->Title);
		GlobalFree(*ItemList);
		GlobalFree(ItemList);
		Browser_FreePages(pages, cnt);
		return 0;
	}
	lstrcpy((*(ItemList))->CheckURL, pages[0].Url);
	Browser_FreePages(pages, cnt);

	//本体に追加するようにメッセージを送信
	SendMessage(hWnd, WM_SETITEMLIST, 1, (LPARAM)ItemList);

	GlobalFree(ItemList);
	return 1;
}


/******************************************************************************

	SetListItem

	ListView にアイテムを追加する

******************************************************************************/

static int SetListItem(HWND hListView, char *buf,int Img,long lp,int iItem)
{
	LV_ITEM LvItem;

	LvItem.mask = LVIF_TEXT | LVIF_PARAM;

	if(iItem == -1){
		iItem = ListView_GetItemCount(hListView);
	}
	LvItem.iItem = iItem;
	LvItem.iSubItem = 0;
	LvItem.pszText = buf;
	LvItem.cchTextMax = BUFSIZE - 1;
	LvItem.iImage = Img;
	LvItem.lParam = lp;

	// リストビューにアイテムを追加する
	return ListView_InsertItem(hListView, &LvItem);
}


/******************************************************************************

	DialogLvNotifyProc

	ダイアログの通知メッセージの処理

******************************************************************************/

static LRESULT DialogLvNotifyProc(HWND hWnd, LPARAM lParam, HWND hListView)
{
	NMHDR *CForm = (NMHDR *)lParam;
	LV_KEYDOWN *LKey = (LV_KEYDOWN *)lParam;

	if(CForm->hwndFrom != hListView){
		return FALSE;
	}

	switch(LKey->hdr.code)
	{
	case LVN_KEYDOWN:			//キーダウン
		if(((LV_KEYDOWN *)lParam)->wVKey == 'A' && GetAsyncKeyState(VK_CONTROL) < 0){
			ListView_SetItemState(GetDlgItem(hWnd, IDC_LIST), -1, LVIS_SELECTED, LVIS_SELECTED);
			break;
		}
		break;
	}

	switch(CForm->code)
	{
	case NM_DBLCLK:
		SendMessage(hWnd, WM_COMMAND, IDOK, 0);
		return TRUE;
	}
	return 0;
}


/******************************************************************************

	URLListroc

	URL一覧表示ダイアログのプロシージャ

******************************************************************************/

BOOL CALLBACK URLListroc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	struct TPITEM **ItemList;
	BROWSER_PAGE *pages;
	LV_COLUMN lvc;
	HWND pWnd;
	char Title[MAXSIZE];
	char URL[MAXSIZE];
	int SelectItem;
	int i, cnt;

	switch (uMsg)
	{
	case WM_INITDIALOG:
		SetWindowLong(hDlg, DWL_USER, (long)lParam);

		i = 0;
		lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		lvc.fmt = LVCFMT_LEFT;

		lvc.cx = 200;
		lvc.pszText = ST_LV_COL_TITLE;
		lvc.iSubItem = i;
		ListView_InsertColumn(GetDlgItem(hDlg, IDC_LIST), i, &lvc);

		i++;
		lvc.cx = 110;
		lvc.pszText = ST_LV_COL_BROWSER;
		lvc.iSubItem = i;
		ListView_InsertColumn(GetDlgItem(hDlg, IDC_LIST), i ,&lvc);

		i++;
		lvc.cx = 500;
		lvc.pszText = ST_LV_COL_URL;
		lvc.iSubItem = i;
		ListView_InsertColumn(GetDlgItem(hDlg, IDC_LIST), i, &lvc);

		//リストビューのスタイルの設定
		SetWindowLong(GetDlgItem(hDlg, IDC_LIST), GWL_STYLE,
			GetWindowLong(GetDlgItem(hDlg, IDC_LIST), GWL_STYLE) | LVS_SHOWSELALWAYS);
		SendMessage(GetDlgItem(hDlg, IDC_LIST), LVM_SETEXTENDEDLISTVIEWSTYLE, 0,
			LVS_EX_ONECLICKACTIVATE | LVS_EX_FULLROWSELECT |
			SendMessage(GetDlgItem(hDlg, IDC_LIST), LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0));

		//ブラウザで表示しているページを取得
		if((cnt = GetPages(hDlg, &pages, FALSE)) <= 0){
			EndDialog(hDlg, FALSE);
			break;
		}
		for(i = 0; i < cnt; i++){
			int index = SetListItem(GetDlgItem(hDlg, IDC_LIST), pages[i].Title, 0, 0, -1);
			ListView_SetItemText(GetDlgItem(hDlg, IDC_LIST), index, 1, pages[i].Browser);
			ListView_SetItemText(GetDlgItem(hDlg, IDC_LIST), index, 2, pages[i].Url);
		}
		Browser_FreePages(pages, cnt);

		CheckDlgButton(hDlg, IDC_CHECK_TITLE, 1);
		CheckDlgButton(hDlg, IDC_CHECK_CHECKURL, 1);

		GetClassName((HWND)lParam, Title, MAXSIZE - 1);
		if(lstrcmp(Title, "WWWCWindowClass") == 0){
			//本体からの呼び出し
			ShowWindow(GetDlgItem(hDlg, IDC_CHECK_TITLE), SW_HIDE);
			ShowWindow(GetDlgItem(hDlg, IDC_CHECK_CHECKURL), SW_HIDE);
		}else{
			//プロパティからの呼び出し
			SetWindowLong(GetDlgItem(hDlg, IDC_LIST), GWL_STYLE,
				GetWindowLong(GetDlgItem(hDlg, IDC_LIST), GWL_STYLE) | LVS_SINGLESEL);
		}
		ListView_SetItemState(GetDlgItem(hDlg, IDC_LIST), 0, LVIS_FOCUSED, LVIS_FOCUSED);
		break;

	case WM_NOTIFY:
		return DialogLvNotifyProc(hDlg, lParam, GetDlgItem(hDlg, IDC_LIST));

	case WM_COMMAND:
		switch(wParam)
		{
		case IDOK:
			pWnd = (HWND)GetWindowLong(hDlg, DWL_USER);
			if((SelectItem = ListView_GetNextItem(GetDlgItem(hDlg, IDC_LIST), -1, LVNI_SELECTED)) == -1){
				break;
			}

			GetClassName(pWnd, Title, BUFSIZE - 1);
			if(lstrcmp(Title, "WWWCWindowClass") == 0){
				ItemList = (struct TPITEM **)GlobalAlloc(GPTR,
					sizeof(struct TPITEM *) * ListView_GetSelectedCount(GetDlgItem(hDlg, IDC_LIST)));
				if(ItemList == NULL){
					EndDialog(hDlg, FALSE);
					break;
				}

				//アイテムリストの作成
				SelectItem = -1;
				i = 0;
				while((SelectItem = ListView_GetNextItem(GetDlgItem(hDlg, IDC_LIST), SelectItem, LVNI_SELECTED)) != -1){
					*(ItemList + i) = (struct TPITEM *)GlobalAlloc(GPTR, sizeof(struct TPITEM));
					if(*(ItemList + i) == NULL){
						EndDialog(hDlg, FALSE);
						return TRUE;
					}
					//タイトルの設定
					*Title = '\0';
					ListView_GetItemText(GetDlgItem(hDlg, IDC_LIST), SelectItem, 0, Title, MAXSIZE - 1);
					(*(ItemList + i))->Title = (char *)GlobalAlloc(GPTR, sizeof(char) * lstrlen(Title) + 1);
					if((*(ItemList + i))->Title == NULL){
						EndDialog(hDlg, FALSE);
						return TRUE;
					}
					lstrcpy((*(ItemList + i))->Title, Title);

					//チェックするURLの設定
					*URL = '\0';
					ListView_GetItemText(GetDlgItem(hDlg, IDC_LIST), SelectItem, 2, URL, MAXSIZE - 1);
					(*(ItemList + i))->CheckURL = (char *)GlobalAlloc(GPTR, sizeof(char) * lstrlen(URL) + 1);
					if((*(ItemList + i))->CheckURL == NULL){
						EndDialog(hDlg, FALSE);
						return TRUE;
					}
					lstrcpy((*(ItemList + i))->CheckURL, URL);

					if(IsDlgButtonChecked(hDlg, IDC_CHECK_VIEWURL) == 1){
						//表示するURLの設定
						(*(ItemList + i))->ViewURL = (char *)GlobalAlloc(GPTR, sizeof(char) * lstrlen(URL) + 1);
						if((*(ItemList + i))->ViewURL == NULL){
							EndDialog(hDlg, FALSE);
							return TRUE;
						}
						lstrcpy((*(ItemList + i))->ViewURL, URL);
					}
					i++;
				}
				//本体に追加するようにメッセージを送信
				SendMessage(pWnd, WM_SETITEMLIST, ListView_GetSelectedCount(GetDlgItem(hDlg, IDC_LIST)), (LPARAM)ItemList);
				GlobalFree(ItemList);

			}else{
				if(IsDlgButtonChecked(hDlg, IDC_CHECK_TITLE) == 1){
					//タイトル
					*Title = '\0';
					ListView_GetItemText(GetDlgItem(hDlg, IDC_LIST), SelectItem, 0, Title, MAXSIZE - 1);
					SendMessage(GetDlgItem(pWnd, IDC_EDIT_TITLE), WM_SETTEXT, 0, (LPARAM)Title);
				}

				*URL = '\0';
				ListView_GetItemText(GetDlgItem(hDlg,IDC_LIST), SelectItem, 2, URL, MAXSIZE - 1);

				if(IsDlgButtonChecked(hDlg, IDC_CHECK_CHECKURL) == 1){
					//チェックするURL
					SendMessage(GetDlgItem(pWnd, IDC_EDIT_URL), WM_SETTEXT, 0, (LPARAM)URL);
				}

				if(IsDlgButtonChecked(hDlg,IDC_CHECK_VIEWURL) == 1){
					//表示するURL
					SendMessage(GetDlgItem(pWnd, IDC_EDIT_VIEWURL), WM_SETTEXT, 0, (LPARAM)URL);
				}
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

	GetBrowserInfoList

	URL一覧表示ダイアログの表示

******************************************************************************/

__declspec(dllexport) int CALLBACK GetBrowserInfoList(HWND hWnd, struct TPITEM **ToolItemList,
													  int ToolItemListCnt, int type, int CheckType)
{
	//プロパティ
	if(type == TOOL_EXEC_PORP){
		MessageBox(hWnd, STR_MSG_BROWSER_INFO, STR_ERR_TITLE_BROWSER, MB_ICONINFORMATION | MB_OK);
		return 1;
	}

	//ウィンドウメニュー以外からの呼び出しの場合は処理を行わない
	if(type != TOOL_EXEC_WINDOWMENU){
		return 0;
	}

	//ダイアログの表示
	if(DialogBoxParam(ghinst, MAKEINTRESOURCE(IDD_DIALOG_URLLIST), hWnd, URLListroc, (LPARAM)hWnd) == TRUE){
		return 0;
	}
	return 1;
}


/******************************************************************************

	SetBrowserProc

	ブラウザの場所の設定

******************************************************************************/

static BOOL CALLBACK SetBrowserProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	char buf[BUFSIZE];

	switch (uMsg)
	{
	case WM_INITDIALOG:
		SendMessage(GetDlgItem(hDlg, IDC_EDIT_PATH), WM_SETTEXT, 0, (LPARAM)BrowserPath);
		break;

	case WM_CLOSE:
		EndDialog(hDlg, FALSE);
		break;

	case WM_COMMAND:
		switch(wParam)
		{
		case IDC_BUTTON_BROWS:
			if(FileSelect(hDlg, "*.exe", STR_FILTER_EXESELECT, STR_TITLE_FILESELECT, buf, NULL, 1) == -1){
				break;
			}
			SendMessage(GetDlgItem(hDlg, IDC_EDIT_PATH), WM_SETTEXT, 0, (LPARAM)buf);
			break;

		case IDOK:
			SendMessage(GetDlgItem(hDlg, IDC_EDIT_PATH), WM_GETTEXT, BUFSIZE - 1, (LPARAM)BrowserPath);
			WritePrivateProfileString("HTTP", "BrowserPath", BrowserPath, app_path);
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

	DDE_OpenURL

	ブラウザでURLを開く (以前はDDEを使用していたため関数名はそのまま)
	既定のブラウザ (または設定したブラウザ) に渡すと、起動しているブラウザで開かれる

******************************************************************************/

__declspec(dllexport) int CALLBACK DDE_OpenURL(HWND hWnd, struct TPITEM **ToolItemList,
													  int ToolItemListCnt, int type, int CheckType)
{
	char *url;
	char *p;
	int rc = -1;
	int i;

	switch(type)
	{
	case TOOL_EXEC_ITEMMENU:
		for(i = 0; i < ToolItemListCnt; i++){
			url = ((*(ToolItemList + i))->ViewURL != NULL && *(*(ToolItemList + i))->ViewURL != '\0') ?
				(*(ToolItemList + i))->ViewURL : (*(ToolItemList + i))->CheckURL;
			//http, https 以外のURLは確認してからブラウザに渡す
			if(ConfirmOpenURL(hWnd, url) == FALSE){
				continue;
			}

			if(*BrowserPath == '\0'){
				rc = ExecItem(hWnd, url, NULL);
			}else{
				//引数として渡すURLは引用符で囲む (末尾の \ は閉じ引用符をエスケープしないよう重ねる)
				char *param = (char *)GlobalAlloc(GMEM_FIXED, lstrlen(url) * 2 + 3);
				char *t;
				if(param == NULL){
					continue;
				}
				t = iStrCpy(iStrCpy(param, "\""), url);
				for(p = url + lstrlen(url); p > url && *(p - 1) == '\\'; p--){
					*(t++) = '\\';
				}
				iStrCpy(t, "\"");
				rc = ExecItem(hWnd, BrowserPath, param);
				GlobalFree(param);
			}
		}
		break;

	case TOOL_EXEC_PORP:
		DialogBoxParam(ghinst, MAKEINTRESOURCE(IDD_DIALOG_SETBROWSER), hWnd, SetBrowserProc, 0);
		rc = 1;
		break;

	default:
		return 0;
	}
	return rc;
}
/* End of source */
