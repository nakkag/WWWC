/**************************************************************************

	WWWC (wwwc.dll)

	httptools.c

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
#include <commctrl.h>
#include <DDEML.H>

#include "httptools.h"
#include "http.h"
#include "StrTbl.h"
#include "wwwcdll.h"
#include "resource.h"


/**************************************************************************
	Define
**************************************************************************/


/**************************************************************************
	Global Variables
**************************************************************************/

//外部参照
extern char app_path[];
extern HINSTANCE ghinst;

extern char BrowserPath[];

extern char AppName[30][BUFSIZE];
extern int AppCnt;


/**************************************************************************
	Local Function Prototypes
**************************************************************************/

//DDE
static HDDEDATA CALLBACK DDECallback(WORD wType, WORD wFmt, HCONV hConv, HSZ hsz1, HSZ hsz2, HDDEDATA hData, DWORD lData1, DWORD lData2);
static void GetDDEStr(char *buf, char *URL, char *Title, char *Frame);
static int GetActiveURLInfo(HWND hWnd, char *browser, char *Title, char *URL);
static int GetURLInfo(HWND hWnd, UINT sMsg, char *browser);
static int SetListItem(HWND hListView, char *buf, int Img, long lp, int iItem);
static LRESULT DialogLvNotifyProc(HWND hWnd, LPARAM lParam, HWND hListView);


/******************************************************************************

	DDECallback

	DDEコールバック

******************************************************************************/

static HDDEDATA CALLBACK DDECallback(WORD wType, WORD wFmt, HCONV hConv, HSZ hsz1,
									 HSZ hsz2, HDDEDATA hData, DWORD lData1, DWORD lData2)
{
	return (HDDEDATA)0;
}


/******************************************************************************

	GetDDEStr

	DDEにて取得した文字列を分解する

******************************************************************************/

static void GetDDEStr(char *buf, char *URL, char *Title, char *Frame)
{
	char *p, *r;

	p = buf;

	if(*p == '\"'){
		p++;
	}
	for(r = URL; *p != '\"' && *p != '\0'; p++){
		if(IsDBCSLeadByte((BYTE)*p) == TRUE){
			*(r++) = *(p++);
		}else if(*p == '\\'){
			p++;
		}
		*(r++) = *p;
	}
	*r = '\0';

	for(; (*p == '\"' || *p == ',') && *p != '\0'; p++);

	for(r = Title; *p != '\"' && *p != '\0'; p++){
		if(IsDBCSLeadByte((BYTE)*p) == TRUE){
			*(r++) = *(p++);
		}else if(*p == '\\'){
			p++;
		}
		*(r++) = *p;
	}
	*r = '\0';

	for(; (*p == '\"' || *p == ',') && *p != '\0'; p++);

	for(r = Frame; *p != '\"' && *p != '\0'; p++){
		if(IsDBCSLeadByte((BYTE)*p) == TRUE){
			*(r++) = *(p++);
		}else if(*p == '\\'){
			p++;
		}
		*(r++) = *p;
	}
	*r = '\0';
}


/******************************************************************************

	AddAppNameProc

	アプリケーション名の追加ダイアログ

******************************************************************************/

static BOOL CALLBACK AddAppNameProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	char *buf;

	switch (uMsg)
	{
	case WM_INITDIALOG:
		if((char *)lParam == NULL){
			EndDialog(hDlg, FALSE);
			break;
		}
		SetWindowLong(hDlg, GWL_USERDATA, lParam);
		SendMessage(GetDlgItem(hDlg, IDC_EDIT_APPNAME), WM_SETTEXT, 0, lParam);
		break;

	case WM_CLOSE:
		EndDialog(hDlg, FALSE);
		break;

	case WM_COMMAND:
		switch(wParam)
		{
		case IDOK:
			buf = (char *)GetWindowLong(hDlg, GWL_USERDATA);
			if(buf == NULL){
				EndDialog(hDlg, FALSE);
				break;
			}
			SendMessage(GetDlgItem(hDlg, IDC_EDIT_APPNAME), WM_GETTEXT, BUFSIZE - 1, (LPARAM)buf);
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

	SetDDEAppProc

	DDEのアプリケーション名を登録するダイアログ

******************************************************************************/

static BOOL CALLBACK SetDDEAppProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	char buf[BUFSIZE];
	int i;

	switch (uMsg)
	{
	case WM_INITDIALOG:
		for(i = 0; i < AppCnt; i++){
			if(*(AppName[i]) != '\0'){
				SendMessage(GetDlgItem(hDlg, IDC_LIST_APP), LB_ADDSTRING, 0, (LPARAM)AppName[i]);
			}
		}
		break;

	case WM_CLOSE:
		EndDialog(hDlg, FALSE);
		break;

	case WM_COMMAND:
		switch(wParam)
		{
		case IDC_BUTTON_ADD:
			if(SendMessage(GetDlgItem(hDlg, IDC_LIST_APP), LB_GETCOUNT, 0, 0) >= 30){
				break;
			}
			*buf = '\0';
			if(DialogBoxParam(ghinst, MAKEINTRESOURCE(IDD_DIALOG_ADDAPPNAME), hDlg, AddAppNameProc, (LPARAM)buf) == FALSE){
				break;
			}
			if(*buf == '\0'){
				break;
			}
			i = SendMessage(GetDlgItem(hDlg, IDC_LIST_APP), LB_ADDSTRING, 0, (LPARAM)buf);
			SendMessage(GetDlgItem(hDlg, IDC_LIST_APP), LB_SETCURSEL, i, 0);
			break;

		case IDC_BUTTON_EDIT:
			i = SendMessage(GetDlgItem(hDlg, IDC_LIST_APP), LB_GETCURSEL, 0, 0);
			if(i < 0){
				break;
			}
			SendMessage(GetDlgItem(hDlg, IDC_LIST_APP), LB_GETTEXT, i, (LPARAM)buf);
			if(DialogBoxParam(ghinst, MAKEINTRESOURCE(IDD_DIALOG_ADDAPPNAME), hDlg, AddAppNameProc, (LPARAM)buf) == FALSE){
				break;
			}
			if(*buf == '\0'){
				break;
			}
			SendMessage(GetDlgItem(hDlg, IDC_LIST_APP), LB_DELETESTRING, i, 0);
			SendMessage(GetDlgItem(hDlg, IDC_LIST_APP), LB_INSERTSTRING, i, (LPARAM)buf);
			SendMessage(GetDlgItem(hDlg, IDC_LIST_APP), LB_SETCURSEL, i, 0);
			break;

		case IDC_BUTTON_DELETE:
			i = SendMessage(GetDlgItem(hDlg, IDC_LIST_APP), LB_GETCURSEL, 0, 0);
			if(i < 0 || MessageBox(hDlg, STR_Q_MSG_DEL, STR_Q_TITLE_DEL, MB_ICONQUESTION | MB_YESNO) == IDNO){
				break;
			}
			SendMessage(GetDlgItem(hDlg, IDC_LIST_APP), LB_DELETESTRING, i, 0);
			SendMessage(GetDlgItem(hDlg, IDC_LIST_APP), LB_SETCURSEL, i, 0);
			break;

		case IDC_BUTTON_UP:
			i = SendMessage(GetDlgItem(hDlg, IDC_LIST_APP), LB_GETCURSEL, 0, 0);
			if(i <= 0){
				break;
			}
			SendMessage(GetDlgItem(hDlg, IDC_LIST_APP), LB_GETTEXT, i, (LPARAM)buf);
			SendMessage(GetDlgItem(hDlg, IDC_LIST_APP), LB_DELETESTRING, i, 0);
			i--;
			SendMessage(GetDlgItem(hDlg, IDC_LIST_APP), LB_INSERTSTRING, i, (LPARAM)buf);
			SendMessage(GetDlgItem(hDlg, IDC_LIST_APP), LB_SETCURSEL, i, 0);
			break;

		case IDC_BUTTON_DOWN:
			i = SendMessage(GetDlgItem(hDlg, IDC_LIST_APP), LB_GETCURSEL, 0, 0);
			if(i < 0 || i >= SendMessage(GetDlgItem(hDlg, IDC_LIST_APP), LB_GETCOUNT, 0, 0) - 1){
				break;
			}
			SendMessage(GetDlgItem(hDlg, IDC_LIST_APP), LB_GETTEXT, i, (LPARAM)buf);
			SendMessage(GetDlgItem(hDlg, IDC_LIST_APP), LB_DELETESTRING, i, 0);
			i++;
			SendMessage(GetDlgItem(hDlg, IDC_LIST_APP), LB_INSERTSTRING, i, (LPARAM)buf);
			SendMessage(GetDlgItem(hDlg, IDC_LIST_APP), LB_SETCURSEL, i, 0);
			break;

		case IDOK:
			AppCnt = SendMessage(GetDlgItem(hDlg, IDC_LIST_APP), LB_GETCOUNT, 0, 0);
			if(AppCnt > 30) AppCnt = 30;

			WritePrivateProfileString("DDE", NULL, NULL, app_path);

			wsprintf(buf, "%ld",AppCnt);
			WritePrivateProfileString("DDE", "AppCnt", buf, app_path);

			for(i = 0; i < AppCnt; i++){
				SendMessage(GetDlgItem(hDlg, IDC_LIST_APP), LB_GETTEXT, i, (LPARAM)AppName[i]);
				wsprintf(buf, "AppName_%d", i);
				WritePrivateProfileString("DDE", buf, AppName[i], app_path);
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

	GetActiveURLInfo

	最後にアクティブだったブラウザが表示していたURLとページタイトルを取得

******************************************************************************/

static int GetActiveURLInfo(HWND hWnd, char *browser, char *Title, char *URL)
{
	HSZ hszService;
	HSZ hszTopic;
	HCONV hConv;
	HSZ hszParam;
	HDDEDATA hDDEData;
	char *gbuf;
	char tmp[MAXSIZE];
	DWORD m_dwDDEID;

	m_dwDDEID = 0L;
	if(DdeInitialize(&m_dwDDEID, (PFNCALLBACK)MakeProcInstance((FARPROC)DDECallback, AfxGetInstanceHandle()),
		CBF_SKIP_ALLNOTIFICATIONS | APPCMD_CLIENTONLY, 0L) != DMLERR_NO_ERROR){
		MessageBox(hWnd, STR_ERR_MSG_DDEEINIT, STR_ERR_TITLE_DDEEINIT, MB_ICONWARNING | MB_OK);
		return 0;
	}

	hszService = DdeCreateStringHandle(m_dwDDEID, browser, 0);
	hszTopic = DdeCreateStringHandle(m_dwDDEID, "WWW_GetWindowInfo", 0);
	hszParam = DdeCreateStringHandle(m_dwDDEID, "0xFFFFFFFF", 0);

	//DDE接続
	hConv = DdeConnect(m_dwDDEID, hszService, hszTopic, NULL);
	if(hConv == 0){
		DdeFreeStringHandle(m_dwDDEID, hszParam);
		DdeFreeStringHandle(m_dwDDEID, hszTopic);
		DdeFreeStringHandle(m_dwDDEID, hszService);
		DdeUninitialize(m_dwDDEID);
		return 0;
	}

	//トランザクションの開始
	hDDEData = DdeClientTransaction(NULL, 0, hConv, hszParam, CF_TEXT, XTYP_REQUEST, 10000L, NULL);

	//データの取得
	gbuf = (char *)GlobalAlloc(GPTR, MAXSIZE);
	DdeGetData(hDDEData, (LPBYTE)gbuf, MAXSIZE - 1, 0);
	GetDDEStr((char *)gbuf, URL, Title, tmp);

	//解放
	GlobalFree(gbuf);

	DdeFreeDataHandle(hDDEData);
	DdeFreeStringHandle(m_dwDDEID, hszParam);
	DdeFreeStringHandle(m_dwDDEID, hszTopic);
	DdeFreeStringHandle(m_dwDDEID, hszService);

	//DDE切断
	DdeDisconnect(hConv);
	DdeUninitialize(m_dwDDEID);
	return 1;
}


/******************************************************************************

	GetBrowserInfo

	最後にアクティブだったブラウザの情報を取得

******************************************************************************/

__declspec(dllexport) int CALLBACK GetBrowserInfo(HWND hWnd, struct TPITEM **ToolItemList,
												  int ToolItemListCnt, int type, int CheckType)
{
	struct TPITEM **ItemList;
	char Title[MAXSIZE], URL[MAXSIZE];
	int i;

	//プロパティ
	if(type == TOOL_EXEC_PORP){
		DialogBoxParam(ghinst, MAKEINTRESOURCE(IDD_DIALOG_SETDDEAPP), hWnd, SetDDEAppProc, 0);
		return 1;
	}

	//ウィンドウメニュー以外からの呼び出しの場合は処理を行わない
	if(type != TOOL_EXEC_WINDOWMENU){
		return 0;
	}

	//最後にアクティブだったブラウザのURLとタイトルを取得
	//IEを検索後Netscapeを検索
	for(i = 0; i < AppCnt; i++){
		if(GetActiveURLInfo(hWnd, AppName[i], Title, URL) != 0){
			break;
		}
	}
	if(i >= AppCnt){
		MessageBox(hWnd, STR_ERR_MSG_DDE, STR_ERR_TITLE_DDE, MB_ICONWARNING | MB_OK);
		return 0;
	}

	//アイテムリストを確保
	ItemList = (struct TPITEM **)GlobalAlloc(GPTR, sizeof(struct TPITEM *));
	if(ItemList == NULL){
		return 0;
	}
	//アイテム情報を確保
	*ItemList = (struct TPITEM *)GlobalAlloc(GPTR, sizeof(struct TPITEM));
	if(*ItemList == NULL){
		GlobalFree(ItemList);
		return 0;
	}

	//タイトルを設定
	(*(ItemList))->Title = (char *)GlobalAlloc(GPTR, sizeof(char) * lstrlen(Title) + 1);
	if((*(ItemList))->Title == NULL){
		GlobalFree(*ItemList);
		GlobalFree(ItemList);
		return 0;
	}
	lstrcpy((*(ItemList))->Title, Title);

	//URLを設定
	(*(ItemList))->CheckURL = (char *)GlobalAlloc(GPTR, sizeof(char) * lstrlen(URL) + 1);
	if((*(ItemList))->CheckURL == NULL){
		GlobalFree((*(ItemList))->Title);
		GlobalFree(*ItemList);
		GlobalFree(ItemList);
		return 0;
	}
	lstrcpy((*(ItemList))->CheckURL, URL);

	//本体に追加するようにメッセージを送信
	SendMessage(hWnd, WM_SETITEMLIST, 1, (LPARAM)ItemList);

	GlobalFree(ItemList);
	return 1;
}


/******************************************************************************

	GetURLInfo

	全てのブラウザの情報を取得

******************************************************************************/

static int GetURLInfo(HWND hWnd, UINT sMsg, char *browser)
{
	HSZ hszService;
	HSZ hszTopic;
	HCONV hConv;
	HSZ hszParam;
	HDDEDATA hDDEData;
	DWORD dwActivateID[500];
	char tmp[BUFSIZE];
	char *gbuf;
	int i;
	DWORD m_dwDDEID;

	m_dwDDEID = 0L;
	if(DdeInitialize(&m_dwDDEID, (PFNCALLBACK)MakeProcInstance((FARPROC)DDECallback, AfxGetInstanceHandle()),
		CBF_SKIP_ALLNOTIFICATIONS | APPCMD_CLIENTONLY, 0L) != DMLERR_NO_ERROR){
		MessageBox(hWnd, STR_ERR_MSG_DDEEINIT, STR_ERR_TITLE_DDEEINIT, MB_ICONWARNING | MB_OK);
		return 0;
	}

	//ブラウザを列挙する
	hszService = DdeCreateStringHandle(m_dwDDEID, browser, 0);
	hszTopic = DdeCreateStringHandle(m_dwDDEID, "WWW_ListWindows", 0);
	hszParam = DdeCreateStringHandle(m_dwDDEID, "0xFFFFFFFF", 0);

	//DDE接続
	hConv = DdeConnect(m_dwDDEID, hszService, hszTopic, NULL);
	if(hConv == 0){
		DdeFreeStringHandle(m_dwDDEID, hszParam);
		DdeFreeStringHandle(m_dwDDEID, hszTopic);
		DdeFreeStringHandle(m_dwDDEID, hszService);
		DdeUninitialize(m_dwDDEID);
		return 0;
	}

	//トランザクションの開始
	hDDEData = DdeClientTransaction(NULL, 0, hConv, hszParam, CF_TEXT, XTYP_REQUEST, 10000L, NULL);

	//ブラウザのIDを取得
	memset(dwActivateID, 0, sizeof(dwActivateID));
	DdeGetData(hDDEData, (LPBYTE)dwActivateID, sizeof(dwActivateID), 0);

	//解放
	DdeFreeDataHandle(hDDEData);
	DdeFreeStringHandle(m_dwDDEID, hszParam);
	DdeFreeStringHandle(m_dwDDEID, hszTopic);

	//DDE切断
	DdeDisconnect(hConv);


	//ブラウザ情報を取得する
	hszTopic = DdeCreateStringHandle(m_dwDDEID, "WWW_GetWindowInfo", 0);

	//DDE接続
	hConv = DdeConnect(m_dwDDEID, hszService, hszTopic, NULL);
	if(hConv == 0){
		DdeFreeStringHandle(m_dwDDEID, hszTopic);
		DdeFreeStringHandle(m_dwDDEID, hszService);
		DdeUninitialize(m_dwDDEID);
		return 0;
	}
	i = 0;

	gbuf = (char *)GlobalAlloc(GPTR, MAXSIZE);

	while(dwActivateID[i] != 0){
		wsprintf(tmp,"%d",dwActivateID[i]);
		hszParam = DdeCreateStringHandle(m_dwDDEID, tmp, 0);

		//トランザクションの開始
		hDDEData = DdeClientTransaction(NULL, 0, hConv, hszParam, CF_TEXT, XTYP_REQUEST, 10000L, NULL);

		//ブラウザ情報の取得
		DdeGetData(hDDEData, (LPBYTE)gbuf, MAXSIZE - 1, 0);

		//ウィンドウに取得した情報を送信
		SendMessage(hWnd, sMsg, 0, (LPARAM)gbuf);

		//解放
		DdeFreeDataHandle(hDDEData);
		DdeFreeStringHandle(m_dwDDEID, hszParam);
		i++;
	}

	//解放
	GlobalFree(gbuf);

	DdeFreeStringHandle(m_dwDDEID, hszTopic);
	DdeFreeStringHandle(m_dwDDEID, hszService);

	//DDE切断
	DdeDisconnect(hConv);
	DdeUninitialize(m_dwDDEID);
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
	LV_COLUMN lvc;
	HWND pWnd;
	char Title[MAXSIZE];
	char Frame[MAXSIZE];
	char URL[MAXSIZE];
	int SelectItem;
	int i, ret;

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
		lvc.cx = 100;
		lvc.pszText = ST_LV_COL_FLAME;
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

		//ブラウザ情報取得
		ret = 0;
		for(i = 0; i < AppCnt; i++){
			ret |= GetURLInfo(hDlg, WM_GETBROUSERINFO, AppName[i]);
		}
		if(ret == 0 || ListView_GetItemCount(GetDlgItem(hDlg, IDC_LIST)) == 0){
			MessageBox(hDlg, STR_ERR_MSG_DDE, STR_ERR_TITLE_DDE, MB_ICONWARNING | MB_OK);
			EndDialog(hDlg, FALSE);
		}

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

	case WM_GETBROUSERINFO:
		//ブラウザ情報文字列を分解
		GetDDEStr((char *)lParam, URL, Title, Frame);
		if(*Title == '\0' && *URL == '\0'){
			break;
		}

		//リストビューに追加する
		i = SetListItem(GetDlgItem(hDlg, IDC_LIST), Title, 0, 0, -1);
		ListView_SetItemText(GetDlgItem(hDlg, IDC_LIST), i, 1, Frame);
		ListView_SetItemText(GetDlgItem(hDlg, IDC_LIST), i, 2, URL);
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
		DialogBoxParam(ghinst, MAKEINTRESOURCE(IDD_DIALOG_SETDDEAPP), hWnd, SetDDEAppProc, 0);
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

	DDE_SetActiveBrowser

	最後にアクティブだったブラウザをアクティブにする

******************************************************************************/

DWORD DDE_SetActiveBrowser(HWND hWnd, char *browser)
{
	HSZ hszService;
	HSZ hszTopic;
	HSZ hszParam;
	HCONV hConv;
	HDDEDATA hDDEData;
	DWORD dwActivateID = 0;
	DWORD m_dwDDEID;

	m_dwDDEID = 0L;
	if(DdeInitialize(&m_dwDDEID, (PFNCALLBACK)MakeProcInstance((FARPROC)DDECallback, AfxGetInstanceHandle()),
		CBF_SKIP_ALLNOTIFICATIONS | APPCMD_CLIENTONLY, 0L) != DMLERR_NO_ERROR){
		MessageBox(hWnd, STR_ERR_MSG_DDEEINIT, STR_ERR_TITLE_DDEEINIT, MB_ICONWARNING | MB_OK);
		return 0;
	}

	hszService = DdeCreateStringHandle(m_dwDDEID, browser,0);
	hszTopic = DdeCreateStringHandle(m_dwDDEID, "WWW_Activate", 0);
	//DDE Connect
	hConv = DdeConnect(m_dwDDEID, hszService, hszTopic, NULL);
	DdeFreeStringHandle(m_dwDDEID, hszTopic);
	DdeFreeStringHandle(m_dwDDEID, hszService);
	if(hConv == 0){
		DdeUninitialize(m_dwDDEID);
		return 0;
	}

	// 0xFFFFFFFF は最後にアクティブだったブラウザを指す。
	// ブラウザのIDを指定するとそのブラウザをアクティブにする。
	hszParam = DdeCreateStringHandle(m_dwDDEID, "0xFFFFFFFF,0x0", 0);
	//Client Transaction
	hDDEData = DdeClientTransaction(NULL, 0, hConv, hszParam, CF_TEXT, XTYP_REQUEST, 10000L, NULL);
	DdeFreeStringHandle(m_dwDDEID, hszParam);
	if(hDDEData == 0){
		DdeDisconnect(hConv);
		DdeUninitialize(m_dwDDEID);
		return 0;
	}

	//Get Data
	//アクティブにしたブラウザのIDを取得
	DdeGetData(hDDEData, (LPBYTE)&dwActivateID, sizeof(DWORD), 0);

    DdeFreeDataHandle(hDDEData);
	DdeDisconnect(hConv);
	DdeUninitialize(m_dwDDEID);
	return dwActivateID;
}


/******************************************************************************

	DDE_SetBrowseURL

	最後にアクティブだったブラウザにURLを送る

******************************************************************************/

BOOL DDE_SetBrowseURL(HWND hWnd, char *browser, char *URL)
{
	HSZ hszService;
	HSZ hszTopic;
	HSZ hszParam;
	HCONV hConv;
	HDDEDATA hDDEData;
	char sParam[BUFSIZE];
	DWORD m_dwDDEID;

	m_dwDDEID = 0L;
	if(DdeInitialize(&m_dwDDEID, (PFNCALLBACK)MakeProcInstance((FARPROC)DDECallback, AfxGetInstanceHandle()),
		CBF_SKIP_ALLNOTIFICATIONS | APPCMD_CLIENTONLY, 0L) != DMLERR_NO_ERROR){
		MessageBox(hWnd, STR_ERR_MSG_DDEEINIT, STR_ERR_TITLE_DDEEINIT, MB_ICONWARNING | MB_OK);
		return FALSE;
	}

	hszService = DdeCreateStringHandle(m_dwDDEID, browser, 0);
	hszTopic = DdeCreateStringHandle(m_dwDDEID, "WWW_OpenURL", 0);
	//DDE Connect
	hConv = DdeConnect(m_dwDDEID, hszService, hszTopic, NULL);
	DdeFreeStringHandle(m_dwDDEID, hszService);
	DdeFreeStringHandle(m_dwDDEID, hszTopic);
	if(hConv == 0){
		DdeUninitialize(m_dwDDEID);
		return FALSE;
	}

	// 0xFFFFFFFF は最後にアクティブだったブラウザを指す。
	//ブラウザのIDを指定するとそのブラウザにURLを送る。
	wsprintf(sParam, "\"%s\",,0xFFFFFFFF,0x0,,,", URL);
	hszParam = DdeCreateStringHandle(m_dwDDEID, (LPCSTR)sParam, 0);
	//Client Transaction
	hDDEData = DdeClientTransaction(NULL, 0, hConv, hszParam, CF_TEXT, XTYP_REQUEST, 10000L, NULL);
	DdeFreeStringHandle(m_dwDDEID, hszParam);

    DdeFreeDataHandle(hDDEData);
	DdeDisconnect(hConv);
	DdeUninitialize(m_dwDDEID);

	DDE_SetActiveBrowser(hWnd, browser);
	return TRUE;
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
		case IDC_BUTTON_SETDDE:
			DialogBoxParam(ghinst, MAKEINTRESOURCE(IDD_DIALOG_SETDDEAPP), hDlg, SetDDEAppProc, 0);
			break;

		case IDC_BUTTON_BROWS:
			if(FileSelect(hDlg, "*.exe", STR_FILTER_EXESELECT, STR_TITLE_FILESELECT, buf, NULL, 1) == -1){
				break;
			}
			SendMessage(GetDlgItem(hDlg, IDC_EDIT_PATH), WM_SETTEXT, 0, (LPARAM)buf);
			break;

		case IDOK:
			SendMessage(GetDlgItem(hDlg, IDC_EDIT_PATH), WM_GETTEXT,
				SendMessage(GetDlgItem(hDlg, IDC_EDIT_PATH), WM_GETTEXTLENGTH, 0, 0) + 1, (LPARAM)BrowserPath);
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

	ブラウザにURLを送る

******************************************************************************/

__declspec(dllexport) int CALLBACK DDE_OpenURL(HWND hWnd, struct TPITEM **ToolItemList,
													  int ToolItemListCnt, int type, int CheckType)
{
	char *url;
	int rc = -1;
	int i, j;

	switch(type)
	{
	case TOOL_EXEC_ITEMMENU:
		for(i = 0; i < ToolItemListCnt; i++){
			url = ((*(ToolItemList + i))->ViewURL != NULL && *(*(ToolItemList + i))->ViewURL != '\0') ?
				(*(ToolItemList + i))->ViewURL : (*(ToolItemList + i))->CheckURL;

			for(j = 0; j < AppCnt; j++){
				if(DDE_SetBrowseURL(hWnd, AppName[j], url) == TRUE){
					break;
				}
			}
			if(j >= AppCnt){
				if(*BrowserPath == '\0'){
					rc = ExecItem(hWnd, url, NULL);
				}else{
					rc = ExecItem(hWnd, BrowserPath, url);
				}
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
