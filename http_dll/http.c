/**************************************************************************

	WWWC (wwwc.dll)

	http.c

	Copyright (C) 1996-2018 by Ohno Tomoaki. All rights reserved.
		https://www.nakka.com/
		nakka@nakka.com

**************************************************************************/


/**************************************************************************
	Include Files
**************************************************************************/

#define _INC_OLE
#include <windows.h>
#undef  _INC_OLE
#include <winsock.h>
#include <commctrl.h>
#include <richedit.h>
#ifdef THGHN
#include <process.h>
#endif
#include <time.h>
#include <winhttp.h>

#include "String.h"
#include "httptools.h"
#include "httpfilter.h"
#include "http.h"
#include "StrTbl.h"
#include "wwwcdll.h"
#include "resource.h"
#include "global.h"
#include "md5.h"


/**************************************************************************
	Define
**************************************************************************/

#define ESC					0x1B		/* エスケープコード */

#define TH_EXIT				(WM_USER + 1200)

#define REQUEST_HEAD		0
#define REQUEST_GET			1

#define HTTP_MENU_CNT		6

#define USER_AGENT			L"WWWC/1.13"

#define WM_HTTP_DONE		(WM_USER + 1300)		/* スレッドの終了通知 */
#define WM_HTTP_PROGRESS	(WM_USER + 1301)		/* 受信状況の通知 */
#define NOTIFY_WINDOW_CLASS	"WWWC_HTTP_Notify"
#define THREAD_END_WAIT		5000					/* 終了時にスレッドを待つ時間 (ミリ秒) */

#ifndef HWND_MESSAGE
#define HWND_MESSAGE		((HWND)-3)
#endif


/**************************************************************************
	Global Variables
**************************************************************************/

static int srcLen;

static struct TPHOSTINFO *tpHostInfo;
static int HostInfoCnt;

HWND hWndList[100];

static HWND hNotifyWnd;							/* スレッドからの通知を受け取るウィンドウ */
static CRITICAL_SECTION ListCs;					/* 実行中のリストの排他 */
static BOOL ListCsInit;
static struct TPHTTP *ActiveList;				/* 実行中のリスト */
static LONG ThreadCnt;							/* 実行中のスレッド数 */


//外部参照
extern HINSTANCE ghinst;

extern int CheckType;
extern int TimeOut;

extern int Proxy;
extern char pServer[];
extern int pPort;
extern int pNoCache;
extern int pUsePass;
extern char pUser[];
extern char pPass[];

extern char BrowserPath[];
extern int TimeZone;
extern int HostNoCache;

extern char DateFormat[];
extern char TimeFormat[];


/**************************************************************************
	Local Function Prototypes
**************************************************************************/

//View
static DWORD CALLBACK EditStreamProc(DWORD dwCookie, LPBYTE pbBuf, LONG cb, LONG *pcb);
static BOOL CALLBACK ViewProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

//Title
static char *GetTitle(char *buf);

//Check
static void SetErrorString(struct TPITEM *tpItemInfo, char *buf, BOOL HeadFlag);
static void SetHttpError(struct TPITEM *tpItemInfo, char *def, char *func, DWORD err);
static char *CreateOldData(char *buf);
static int GetMetaString(struct TPITEM *tpItemInfo);
static int Head_GetDate(struct TPITEM *tpItemInfo, int CmpOption, BOOL *DateRet);
static int Head_GetSize(struct TPITEM *tpItemInfo, int CmpOption, int SetDate, BOOL *SizeRet);
static int Get_GetSize(struct TPITEM *tpItemInfo, int CmpOption, int SetDate);
static int Get_MD5Check(struct TPITEM *tpItemInfo, int SetDate);
static int HeaderFunc(HWND hWnd, struct TPITEM *tpItemInfo);
static void SetSBText(HWND hWnd, struct TPITEM *tpItemInfo, char *msg);
static BOOL CheckHead(struct TPHTTP *tpHTTP);
static void FreeURLData(struct TPHTTP *tpHTTP);
static BOOL GetServerPort(HWND hWnd, struct TPITEM *tpItemInfo, struct TPHTTP *tpHTTP);
static BOOL CheckServer(HWND hWnd, char *RealServer);
static void ReleaseHTTP(struct TPHTTP *tpHTTP);
static void CloseRequestHandles(struct TPHTTP *tpHTTP);
static void CancelHTTP(struct TPHTTP *tpHTTP);
static BOOL IsSameOrigin(char *URL1, char *URL2);
static BOOL HasItemAuth(struct TPITEM *tpItemInfo);

//Drag & Dropb
static BOOL WriteInternetShortcut(char *URL, char *path);

//Option
static LRESULT OptionNotifyProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK PropertyProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK PropertyCheckProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK PropertyOptionProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

static BOOL CALLBACK ProtocolPropertyCheckProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);


/******************************************************************************

	EditStreamProc

	リッチエディットへの文字列の設定

******************************************************************************/

static DWORD CALLBACK EditStreamProc(DWORD dwCookie, LPBYTE pbBuf, LONG cb, LONG *pcb)
{
	int len;

	lstrcpyn((char *)pbBuf, (char *)dwCookie + srcLen, cb);
	*pcb = len = lstrlen((char *)pbBuf);
	srcLen += len;
    return FALSE;
}


/******************************************************************************

	ViewProc

	アイテムのヘッダー・ソース表示ダイアログ

******************************************************************************/

static BOOL CALLBACK ViewProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	struct TPITEM *tpItemInfo;
	struct TPHTTP *tpHTTP;
	EDITSTREAM eds;
	RECT DesktopRect, WindowRect;
	CHARRANGE seltext, settext;
	HWND fWnd;
	char *buf = NULL;
	char *ret;
	char *filter_title;
	long wl;
	int i;

	switch (uMsg)
	{
	case WM_INITDIALOG:
		CreateWindowEx(WS_EX_CLIENTEDGE, "RICHEDIT",
			(LPSTR)NULL, WS_TABSTOP | WS_CHILD | WS_VSCROLL | WS_HSCROLL |
			ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_READONLY | ES_SAVESEL |
			ES_NOHIDESEL | ES_DISABLENOSCROLL,
			0, 0, 0, 0, hDlg, (HMENU)IDC_EDIT_VIEW, ghinst, NULL);
		ShowWindow(GetDlgItem(hDlg, IDC_EDIT_VIEW), SW_SHOW);

		GetWindowRect(GetDesktopWindow(), (LPRECT)&DesktopRect);
		GetWindowRect(hDlg, (LPRECT)&WindowRect);
		SetWindowPos(hDlg, HWND_TOP, (DesktopRect.right / 2) - ((WindowRect.right - WindowRect.left) / 2),
			(DesktopRect.bottom / 2) - ((WindowRect.bottom - WindowRect.top) / 2), 0, 0, SWP_NOSIZE);

		tpItemInfo = (struct TPITEM *)lParam;
		tpHTTP = (struct TPHTTP *)tpItemInfo->Param1;
		if(tpHTTP == NULL || tpHTTP->buf == NULL){
			DestroyWindow(hDlg);
			break;
		}

		SendDlgItemMessage(hDlg, IDC_EDIT_VIEW, EM_SETTEXTMODE, TM_RICHTEXT, 0);
		if(tpItemInfo->Param3 == 1){
			buf = (char *)GlobalAlloc(GMEM_FIXED, lstrlen(STR_TITLE_HEADVIEW) + lstrlen(tpItemInfo->CheckURL) + 1);
			if(buf != NULL){
				wsprintf(buf, STR_TITLE_HEADVIEW, tpItemInfo->CheckURL);
			}
			*(tpHTTP->buf + HeaderSize(tpHTTP->buf)) = '\0';
			SendMessage(GetDlgItem(hDlg, IDC_EDIT_VIEW), WM_SETTEXT, 0, (LPARAM)tpHTTP->buf);

		}else if(tpItemInfo->Param3 == 2){
			SetCursor(LoadCursor(NULL, IDC_WAIT));
			DeleteSizeInfo(tpHTTP->buf, tpHTTP->Size);

			filter_title = "";
			if(tpHTTP->FilterFlag == TRUE){
				//Shiftが押されている場合はフィルタを処理
				tpHTTP->FilterFlag = FilterCheck(tpItemInfo->CheckURL, tpHTTP->buf + HeaderSize(tpHTTP->buf), tpHTTP->Size);
				if(tpHTTP->FilterFlag == TRUE){
					filter_title = STR_TITLE_FILTER;
				}else if(FilterMatch(tpItemInfo->CheckURL) == TRUE){
					filter_title = STR_TITLE_FILTERERR;
				}
			}

			buf = SrcConv(tpHTTP->buf + HeaderSize(tpHTTP->buf), tpHTTP->Size);
			if(buf != NULL){
				i = GetHtml2RtfSize(buf);
				ret = (char *)GlobalAlloc(GMEM_FIXED, i + 1);
				if(ret != NULL){
					Html2Rtf(buf, ret);
					SendDlgItemMessage(hDlg, IDC_EDIT_VIEW, EM_LIMITTEXT, i + 1, 0);
					srcLen = 0;
					eds.dwCookie = (DWORD)ret;
					eds.dwError = 0;
					eds.pfnCallback = EditStreamProc;
					SendMessage(GetDlgItem(hDlg, IDC_EDIT_VIEW), EM_STREAMIN, SF_RTF, (LPARAM)&eds);
					GlobalFree(ret);
				}
				GlobalFree(buf);
			}
			SetCursor(LoadCursor(NULL, IDC_ARROW));

			buf = (char *)GlobalAlloc(GMEM_FIXED, lstrlen(STR_TITLE_SOURCEVIEW) + lstrlen(filter_title) + lstrlen(tpItemInfo->CheckURL) + 1);
			if(buf != NULL){
				wsprintf(buf, STR_TITLE_SOURCEVIEW, filter_title, tpItemInfo->CheckURL);
			}
		}
		if(buf != NULL){
			SetWindowText(hDlg, buf);
			GlobalFree(buf);
		}
		for(i = 0; i < 100; i++){
			if(hWndList[i] == NULL){
				hWndList[i] = hDlg;
				break;
			}
		}
		break;

	case WM_SIZE:
		GetWindowRect(hDlg, (LPRECT)&WindowRect);
		SetWindowPos(GetDlgItem(hDlg, IDC_EDIT_VIEW), 0, 0, 0,
			(WindowRect.right - WindowRect.left) - (GetSystemMetrics(SM_CXFRAME) * 2),
			(WindowRect.bottom - WindowRect.top) - GetSystemMetrics(SM_CYSIZE) - (GetSystemMetrics(SM_CXFRAME) * 2) - 30,
			SWP_NOZORDER);
		SetWindowPos(GetDlgItem(hDlg, IDOK), 0,
			(WindowRect.right - WindowRect.left) - (GetSystemMetrics(SM_CXFRAME) * 2) - 100,
			(WindowRect.bottom - WindowRect.top) - GetSystemMetrics(SM_CYSIZE) - (GetSystemMetrics(SM_CXFRAME) * 2) - 25,
			0, 0, SWP_NOZORDER | SWP_NOSIZE);
		SetWindowPos(GetDlgItem(hDlg, ID_BUTTON_COPY), 0,
			(WindowRect.right - WindowRect.left) - (GetSystemMetrics(SM_CXFRAME) * 2) - 200,
			(WindowRect.bottom - WindowRect.top) - GetSystemMetrics(SM_CYSIZE) - (GetSystemMetrics(SM_CXFRAME) * 2) - 25,
			0, 0, SWP_NOZORDER | SWP_NOSIZE);

		InvalidateRect(GetDlgItem(hDlg, IDOK), NULL, FALSE);
		UpdateWindow(GetDlgItem(hDlg, IDOK));
		InvalidateRect(GetDlgItem(hDlg, ID_BUTTON_COPY), NULL, FALSE);
		UpdateWindow(GetDlgItem(hDlg, ID_BUTTON_COPY));
		break;

	case WM_CLOSE:
		EndDialog(hDlg, 0);
		for(i = 0; i < 100; i++){
			if(hWndList[i] == hDlg){
				hWndList[i] = NULL;
				break;
			}
		}
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
		case ID_KEY_ESC:
			SendMessage(hDlg, WM_CLOSE, 0, 0);
			break;

		case ID_MENUITEM_COPY:
		case ID_BUTTON_COPY:
			SendMessage(GetDlgItem(hDlg, IDC_EDIT_VIEW), EM_EXGETSEL, 0, (LPARAM)&seltext);
			if(seltext.cpMin == seltext.cpMax){
				settext.cpMin = 0;
				settext.cpMax = SendMessage(GetDlgItem(hDlg, IDC_EDIT_VIEW), WM_GETTEXTLENGTH, 0, 0);
				SendMessage(GetDlgItem(hDlg, IDC_EDIT_VIEW), EM_EXSETSEL, 0, (LPARAM)&settext);
				SendMessage(GetDlgItem(hDlg, IDC_EDIT_VIEW), WM_COPY, 0, 0);
				SendMessage(GetDlgItem(hDlg, IDC_EDIT_VIEW), EM_EXSETSEL, 0, (LPARAM)&seltext);
			}else{
				SendMessage(GetDlgItem(hDlg, IDC_EDIT_VIEW), WM_COPY, 0, 0);
			}
			break;

		case ID_KEY_RETURN:
			SendMessage(hDlg, WM_COMMAND, GetDlgCtrlID(GetFocus()), 0);
			break;

		case ID_KEY_TAB:
			fWnd = GetWindow(GetFocus(), GW_HWNDNEXT);
			if(fWnd == NULL){
				fWnd = GetWindow(GetFocus(), GW_HWNDFIRST);
				SetFocus(fWnd);
				break;
			}
			wl = GetWindowLong(fWnd, GWL_STYLE);
			while((wl & WS_TABSTOP) == 0 || (wl & WS_VISIBLE) == 0){
				fWnd = GetWindow(fWnd, GW_HWNDNEXT);
				if(fWnd == NULL){
					fWnd = GetWindow(GetFocus(), GW_HWNDFIRST);
					break;
				}
				wl = GetWindowLong(fWnd, GWL_STYLE);
			}
			SetFocus(fWnd);
			break;

		case ID_MENUITEM_ALLSELECT:
			if(GetFocus() != GetDlgItem(hDlg, IDC_EDIT_VIEW)){
				break;
			}
			seltext.cpMin = 0;
			seltext.cpMax = SendMessage(GetDlgItem(hDlg, IDC_EDIT_VIEW), WM_GETTEXTLENGTH, 0, 0);
			SendMessage(GetDlgItem(hDlg, IDC_EDIT_VIEW), EM_EXSETSEL, 0, (LPARAM)&seltext);
			break;
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}


/******************************************************************************

	GetTitle

	HTMLのタイトルを取得

******************************************************************************/

static char *GetTitle(char *buf)
{
#define TITLE_TAG	"title"
	char *SjisBuf, *ret;
	int len;
	BOOL rc;

	//JIS を SJIS に変換
	SjisBuf = SrcConv(buf, lstrlen(buf) + 1);

	//タイトルタグのサイズを取得
	len = GetTagContentSize(SjisBuf, TITLE_TAG);
	if(len == 0){
		GlobalFree(SjisBuf);
		return NULL;
	}

	//タイトルを取得
	ret = (char *)GlobalAlloc(GMEM_FIXED, len + 1);
	if(ret == NULL){
		GlobalFree(SjisBuf);
		return NULL;
	}
	rc = GetTagContent(SjisBuf, TITLE_TAG, ret);
	GlobalFree(SjisBuf);

	if(rc == FALSE){
		GlobalFree(ret);
		return NULL;
	}
	//特殊文字の変換
	ConvHTMLChar(ret);
	DelCtrlChar(ret);
	return ret;
}



/******************************************************************************

	SetErrorString

	エラー文字列の設定

******************************************************************************/

static void SetErrorString(struct TPITEM *tpItemInfo, char *buf, BOOL HeadFlag)
{
	char *p, *r, *t;

	if(tpItemInfo->ErrStatus != NULL){
		GlobalFree(tpItemInfo->ErrStatus);
	}

	r = buf;
	if(HeadFlag == TRUE){
		for(; *r != ' ' && *r != '\0'; r++);
		r = (*r == ' ') ? r + 1 : buf;
	}

	for(p = r; *p != '\0' && *p != '\r' && *p != '\n'; p++);

	tpItemInfo->ErrStatus = (char *)GlobalAlloc(GMEM_FIXED, p - r + 2);
	if(tpItemInfo->ErrStatus == NULL) return;

	t = tpItemInfo->ErrStatus;
	while(p > r){
		*(t++) = *(r++);
	}
	*t = '\0';
	DelCtrlChar(tpItemInfo->ErrStatus);
}



/******************************************************************************

	SetHttpError

	HTTPエラーを設定

******************************************************************************/

static void SetHttpError(struct TPITEM *tpItemInfo, char *def, char *func, DWORD err)
{
	char *msg = NULL;
	char *buf;

	if (err == NO_ERROR) {
		buf = (char *)HeapAlloc(GetProcessHeap(), 0, lstrlen(def) + 2 + lstrlen(func) + 1);
		if (buf == NULL) {
			if (msg != NULL) LocalFree(msg);
			return;
		}
		wsprintf(buf, "%s :%s", def, func);
		SetErrorString(tpItemInfo, buf, FALSE);
		HeapFree(GetProcessHeap(), 0, buf);
		return;
	}
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS,
		GetModuleHandle("winhttp.dll"), err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&msg, 0, NULL);
	if (msg != NULL) {
		char *p;
		for (p = msg; *p != '\0'; p++) {
			if (*p == '\r' || *p == '\n') {
				*p = '\0';
				break;
			}
		}
		buf = (char *)HeapAlloc(GetProcessHeap(), 0, lstrlen(msg) + 2 + 10 + 3 + lstrlen(func) + 1);
		if (buf == NULL) {
			if (msg != NULL) LocalFree(msg);
			return;
		}
		wsprintf(buf, "%s (%u): %s", msg, err, func);
		SetErrorString(tpItemInfo, buf, FALSE);
		HeapFree(GetProcessHeap(), 0, buf);
		LocalFree(msg);
	} else {
		buf = (char *)HeapAlloc(GetProcessHeap(), 0, lstrlen(def) + 2 + 10 + 3 + lstrlen(func) + 1);
		if (buf == NULL) {
			if (msg != NULL) LocalFree(msg);
			return;
		}
		wsprintf(buf, "%s (%u): %s", def, err, func);
		SetErrorString(tpItemInfo, buf, FALSE);
		HeapFree(GetProcessHeap(), 0, buf);
	}
}


/******************************************************************************

	CreateOldData

	旧データの作成

******************************************************************************/

static char *CreateOldData(char *buf)
{
	char *ret;
	int len;

	if(buf == NULL || *buf == '\0'){
		return NULL;
	}
	len = lstrlen(buf);
	ret = (char *)GlobalAlloc(GMEM_FIXED, sizeof(char) * (len + 1));
	if(ret != NULL){
		lstrcpy(ret, buf);
		if(*(ret + len - 1) == '*'){
			*(ret + len - 1) = '\0';
		}
	}
	return ret;
}


/******************************************************************************

	GetMetaString

	METAタグの取得

******************************************************************************/

static int GetMetaString(struct TPITEM *tpItemInfo)
{
#define DEF_META_TYPE		"name"
#define DEF_META_NAME		"wwwc"
#define DEF_META_CONTENT	"content"

#define NO_META				"no meta"

	struct TPHTTP *tpHTTP;
	char type[BUFSIZE];
	char name[BUFSIZE];
	char content[BUFSIZE];
	char *MetaContent;
	char *p;
	int CmpMsg = ST_DEFAULT;

	tpHTTP = (struct TPHTTP *)tpItemInfo->Param1;
	if(tpHTTP == NULL){
		return CmpMsg;
	}

	if(GetOptionString(tpItemInfo->Option1, type, OP1_TYPE) == FALSE){
		lstrcpy(type, DEF_META_TYPE);
	}
	if(GetOptionString(tpItemInfo->Option1, name, OP1_NAME) == FALSE){
		lstrcpy(name, DEF_META_NAME);
	}
	if(GetOptionString(tpItemInfo->Option1, content, OP1_CONTENT) == FALSE){
		lstrcpy(content, DEF_META_CONTENT);
	}

	//SJISに変換
	p = SrcConv(tpHTTP->buf + HeaderSize(tpHTTP->buf), tpHTTP->Size);

	if((MetaContent = GetMETATag(p, type, name, content)) != NULL){
		//特殊文字の変換
		ConvHTMLChar(MetaContent);
		DelCtrlChar(MetaContent);

		if(LresultCmp(tpItemInfo->Date, MetaContent) != 0){
			CmpMsg = ST_UP;
		}
		if(CmpMsg == ST_UP || tpItemInfo->Date == NULL || *tpItemInfo->Date == '\0'){
			//旧更新日の設定
			if(tpItemInfo->OldDate != NULL){
				GlobalFree(tpItemInfo->OldDate);
			}
			tpItemInfo->OldDate = CreateOldData(tpItemInfo->Date);

			// アイテムに今回のチェック内容をセットする
			if(tpItemInfo->Date != NULL){
				GlobalFree(tpItemInfo->Date);
			}
			tpItemInfo->Date = (char *)GlobalAlloc(GPTR, lstrlen(MetaContent) + 1);
			if(tpItemInfo->Date != NULL){
				lstrcpy(tpItemInfo->Date, MetaContent);
			}
		}
		GlobalFree(MetaContent);

	}else{
		//METAタグを取得できなかったメッセージを設定
		if(tpItemInfo->Date != NULL){
			GlobalFree(tpItemInfo->Date);
		}
		tpItemInfo->Date = (char *)GlobalAlloc(GPTR, lstrlen(NO_META) + 1);
		if(tpItemInfo->Date != NULL){
			lstrcpy(tpItemInfo->Date, NO_META);
		}
	}
	GlobalFree(p);
	return CmpMsg;
}


/******************************************************************************

	Head_GetDate

	ヘッダから更新日時を取得

******************************************************************************/

static int Head_GetDate(struct TPITEM *tpItemInfo, int CmpOption, BOOL *DateRet)
{
	struct TPHTTP *tpHTTP;
	char chtime[BUFSIZE];
	char *headcontent;
	int CmpMsg = ST_DEFAULT;
	int len;

	tpHTTP = (struct TPHTTP *)tpItemInfo->Param1;
	if(tpHTTP == NULL){
		return ST_DEFAULT;
	}

	len = GetHeadContentSize(tpHTTP->buf, "Last-Modified:");
	if(len > BUFSIZE - 2){
		*DateRet = FALSE;
		return ST_DEFAULT;
	}
	headcontent = (char *)HeapAlloc(GetProcessHeap(), 0, len + 2);
	if(headcontent == NULL){
		return ST_DEFAULT;
	}
	*DateRet = GetHeadContent(tpHTTP->buf, "Last-Modified:", headcontent);
	if(*DateRet == TRUE){
		DelCtrlChar(headcontent);
		if(CmpOption == 1 && tpItemInfo->DLLData1 != NULL && *tpItemInfo->DLLData1 != '\0' &&
			lstrcmp(tpItemInfo->DLLData1, headcontent) != 0){
			CmpMsg = ST_UP;
		}
		if(CmpMsg == ST_UP || CmpOption == 0 || tpItemInfo->DLLData1 == NULL || *tpItemInfo->DLLData1 == '\0'){
			//DLL用データに日付を格納
			if(tpItemInfo->DLLData1 != NULL){
				GlobalFree(tpItemInfo->DLLData1);
			}
			tpItemInfo->DLLData1 = (char *)GlobalAlloc(GMEM_FIXED, sizeof(char) * lstrlen(headcontent) + 1);
			if(tpItemInfo->DLLData1 != NULL) lstrcpy(tpItemInfo->DLLData1, headcontent);
		}

		if(CmpMsg == ST_UP || CmpOption == 0 || tpItemInfo->Date == NULL || *tpItemInfo->Date == '\0'){
			//旧更新日の設定
			if(tpItemInfo->OldDate != NULL){
				GlobalFree(tpItemInfo->OldDate);
			}
			tpItemInfo->OldDate = CreateOldData(tpItemInfo->Date);

			//日付形式の変換
			if(DateConv(headcontent, chtime) != 0){
				lstrcpy(chtime, headcontent);
			}
			/* アイテムに今回のチェック内容をセットする */
			if(tpItemInfo->Date != NULL){
				GlobalFree(tpItemInfo->Date);
			}
			tpItemInfo->Date = (char *)GlobalAlloc(GMEM_FIXED, sizeof(char) * lstrlen(chtime) + ((CmpMsg == ST_UP) ? 2 : 1));
			if(tpItemInfo->Date != NULL){
				lstrcpy(tpItemInfo->Date, chtime);
				if(CmpMsg == ST_UP){
					//日付の変更を示す記号の付加
					lstrcat(tpItemInfo->Date, "*");
				}
			}
		}
	}
	HeapFree(GetProcessHeap(), 0, headcontent);
	return CmpMsg;
}


/******************************************************************************

	Head_GetSize

	ヘッダからサイズを取得

******************************************************************************/

static int Head_GetSize(struct TPITEM *tpItemInfo, int CmpOption, int SetDate, BOOL *SizeRet)
{
	struct TPHTTP *tpHTTP;
	char buf[BUFSIZE];
	char *headcontent;
	int CmpMsg = ST_DEFAULT;
	int len;

	tpHTTP = (struct TPHTTP *)tpItemInfo->Param1;
	if(tpHTTP == NULL){
		return ST_DEFAULT;
	}

	/* サイズを取得する */
	len = GetHeadContentSize(tpHTTP->buf, "Content-Length:");
	if(len > BUFSIZE - 2){
		*SizeRet = FALSE;
		return ST_DEFAULT;
	}
	headcontent = (char *)HeapAlloc(GetProcessHeap(), 0, len + 2);
	if(headcontent == NULL){
		return ST_DEFAULT;
	}
	*SizeRet = GetHeadContent(tpHTTP->buf, "Content-Length:", headcontent);
	if(*SizeRet == TRUE){
		DelCtrlChar(headcontent);
		/* 前回チェック時のものと比較する */
		if(CmpOption == 1 && LresultCmp(tpItemInfo->Size, headcontent) != 0){
			CmpMsg = ST_UP;

			if(SetDate == 0 && CreateDateTime(buf) == 0){
				//旧更新日の設定
				if(tpItemInfo->OldDate != NULL){
					GlobalFree(tpItemInfo->OldDate);
				}
				tpItemInfo->OldDate = CreateOldData(tpItemInfo->Date);

				/* アイテムに現在の時刻を設定する */
				if(tpItemInfo->Date != NULL){
					GlobalFree(tpItemInfo->Date);
				}
				tpItemInfo->Date = (char *)GlobalAlloc(GMEM_FIXED, sizeof(char) * lstrlen(buf) + 1);
				if(tpItemInfo->Date != NULL) lstrcpy(tpItemInfo->Date, buf);
			}
		}
		if(CmpMsg == ST_UP || CmpOption == 0 || tpItemInfo->Size == NULL || *tpItemInfo->Size == '\0'){
			//旧サイズの設定
			if(tpItemInfo->OldSize != NULL){
				GlobalFree(tpItemInfo->OldSize);
			}
			tpItemInfo->OldSize = CreateOldData(tpItemInfo->Size);

			/* アイテムに今回のチェック内容をセットする */
			if(tpItemInfo->Size != NULL){
				GlobalFree(tpItemInfo->Size);
			}
			tpItemInfo->Size = (char *)GlobalAlloc(GMEM_FIXED, sizeof(char) * lstrlen(headcontent) + 1);
			if(tpItemInfo->Size != NULL) lstrcpy(tpItemInfo->Size, headcontent);
		}
	}
	HeapFree(GetProcessHeap(), 0, headcontent);
	return CmpMsg;
}


/******************************************************************************

	Get_GetSize

	HTMLのサイズを取得

******************************************************************************/

static int Get_GetSize(struct TPITEM *tpItemInfo, int CmpOption, int SetDate)
{
	struct TPHTTP *tpHTTP;
	char buf[BUFSIZE];
	char tmp[BUFSIZE];
	char *p;
	int CmpMsg = ST_DEFAULT;
	int head_size;

	tpHTTP = (struct TPHTTP *)tpItemInfo->Param1;
	if(tpHTTP == NULL){
		return ST_DEFAULT;
	}

	head_size = HeaderSize(tpHTTP->buf);
	if(GetOptionInt(tpItemInfo->Option1, OP1_NOTAGSIZE) == 1){
		/* 受信したものからヘッダとタグを除去したサイズを計算する */
		p = SrcConv(tpHTTP->buf + head_size, tpHTTP->Size);
		tpHTTP->Size = DelTagSize(p);
		wsprintf(tmp, "%ld", tpHTTP->Size);
		GlobalFree(p);
	}else if(tpHTTP->FilterFlag == TRUE){
		tpHTTP->Size = lstrlen(tpHTTP->buf + head_size);
		wsprintf(tmp, "%ld", tpHTTP->Size);
	}else{
		/* 受信したものからヘッダを除去したサイズを計算する */
		wsprintf(tmp, "%ld", tpHTTP->Size - head_size);
	}

	/* 前回チェック時のものと比較する */
	if(CmpOption == 1 && LresultCmp(tpItemInfo->Size, tmp) != 0){
		CmpMsg = ST_UP;

		if(SetDate == 0 && CreateDateTime(buf) == 0){
			//旧更新日の設定
			if(tpItemInfo->OldDate != NULL){
				GlobalFree(tpItemInfo->OldDate);
			}
			tpItemInfo->OldDate = CreateOldData(tpItemInfo->Date);

			/* アイテムに現在の時刻を設定する */
			if(tpItemInfo->Date != NULL){
				GlobalFree(tpItemInfo->Date);
			}
			tpItemInfo->Date = (char *)GlobalAlloc(GMEM_FIXED, sizeof(char) * lstrlen(buf) + 1);
			if(tpItemInfo->Date != NULL) lstrcpy(tpItemInfo->Date, buf);
		}
	}
	if(CmpMsg == ST_UP || CmpOption == 0 || tpItemInfo->Size == NULL || *tpItemInfo->Size == '\0'){
		//旧サイズの設定
		if(tpItemInfo->OldSize != NULL){
			GlobalFree(tpItemInfo->OldSize);
		}
		tpItemInfo->OldSize = CreateOldData(tpItemInfo->Size);

		/* アイテムに今回のチェック内容をセットする */
		if(tpItemInfo->Size != NULL){
			GlobalFree(tpItemInfo->Size);
		}
		tpItemInfo->Size = (char *)GlobalAlloc(GMEM_FIXED, sizeof(char) * lstrlen(tmp) + 1);
		if(tpItemInfo->Size != NULL) lstrcpy(tpItemInfo->Size, tmp);
	}
	return CmpMsg;
}


/******************************************************************************

	Get_MD5Check

	MD5のダイジェスト値を取得してチェック

******************************************************************************/

static int Get_MD5Check(struct TPITEM *tpItemInfo, int SetDate)
{
	struct TPHTTP *tpHTTP;
	char buf[BUFSIZE];
	int CmpMsg = ST_DEFAULT;
    MD5_CTX context;
    unsigned char digest[16];
	char tmp[34];
	int i, len;

	tpHTTP = (struct TPHTTP *)tpItemInfo->Param1;
	if(tpHTTP == NULL){
		return ST_DEFAULT;
	}

	//ダイジェスト値の生成
    MD5Init(&context);
    MD5Update(&context, tpHTTP->buf + HeaderSize(tpHTTP->buf), lstrlen(tpHTTP->buf + HeaderSize(tpHTTP->buf)));
    MD5Final(digest, &context);

	for(i = 0, len = 0; i < 16; i++, len += 2){
		wsprintf(tmp + len, "%02x", digest[i]);
	}
	*(tmp + len) = '\0';

	/* 前回チェック時のものと比較する */
	if(LresultCmp(tpItemInfo->DLLData2, tmp) != 0){
		CmpMsg = ST_UP;

		if(SetDate == 0 && CreateDateTime(buf) == 0){
			//旧更新日の設定
			if(tpItemInfo->OldDate != NULL){
				GlobalFree(tpItemInfo->OldDate);
			}
			tpItemInfo->OldDate = CreateOldData(tpItemInfo->Date);

			/* アイテムに現在の時刻を設定する */
			if(tpItemInfo->Date != NULL){
				GlobalFree(tpItemInfo->Date);
			}
			tpItemInfo->Date = (char *)GlobalAlloc(GMEM_FIXED, sizeof(char) * lstrlen(buf) + 1);
			if(tpItemInfo->Date != NULL) lstrcpy(tpItemInfo->Date, buf);
		}
	}
	if(CmpMsg == ST_UP || tpItemInfo->DLLData2 == NULL || *tpItemInfo->DLLData2 == '\0'){
		/* アイテムに今回のチェック内容をセットする */
		if(tpItemInfo->DLLData2 != NULL){
			GlobalFree(tpItemInfo->DLLData2);
		}
		tpItemInfo->DLLData2 = (char *)GlobalAlloc(GMEM_FIXED, sizeof(char) * lstrlen(tmp) + 1);
		if(tpItemInfo->DLLData2 != NULL) lstrcpy(tpItemInfo->DLLData2, tmp);
	}
	return CmpMsg;
}


/******************************************************************************

	HeaderFunc

	ヘッダから更新情報を取得して処理を行う

******************************************************************************/

static int HeaderFunc(HWND hWnd, struct TPITEM *tpItemInfo)
{
	struct TPHTTP *tpHTTP;
	char tmp[BUFSIZE];
	char *titlebuf;
	char *headcontent;
	char *p, *r;
	int Status;
	int SizeRet = 0, DateRet = 0;
	int CmpMsg = ST_DEFAULT;
	int DateCheck;
	int SizeCheck;
	int SetDate = 0;
	int len;

	if(tpItemInfo->ErrStatus != NULL){
		GlobalFree(tpItemInfo->ErrStatus);
		tpItemInfo->ErrStatus = NULL;
	}
	tpHTTP = (struct TPHTTP *)tpItemInfo->Param1;
	if(tpHTTP == NULL){
		return CHECK_ERROR;
	}

	//HTTPのステータスを取得
	for(p = tpHTTP->buf; *p != ' ' && *p != '\0'; p++);
	if(*p == '\0'){
		SetErrorString(tpItemInfo, STR_ERROR_HEADER, FALSE);
		return CHECK_ERROR;
	}
	p++;
	for(r = tmp; *p != ' ' && *p != '\r' && *p != '\n' && *p != '\0' && r - tmp < BUFSIZE - 1; p++, r++){
		*r = *p;
	}
	*r = '\0';
	if(*p == '\0'){
		SetErrorString(tpItemInfo, STR_ERROR_HEADER, FALSE);
		return CHECK_ERROR;
	}
	Status = atoi(tmp);

	//ツール
	switch(tpItemInfo->Param3){
	case 1:
	case 2:
		//ヘッダ、ソース表示
		if(Status == 301 || Status == 302 || Status == 303 || Status == 307 || Status == 308){
			break;
		}
		//チェックの終了をWWWCに通知
		SendMessage(hWnd, WM_CHECK_RESULT, ST_DEFAULT, (LPARAM)tpItemInfo);
		return CHECK_END;

	case 3:
		if(Status == 301 || Status == 302 || Status == 303 || Status == 307 || Status == 308){
			break;
		}
		//タイトル取得
		if(Status == 200){
			if((titlebuf = GetTitle(tpHTTP->buf + HeaderSize(tpHTTP->buf))) != NULL){
				if(tpItemInfo->Title != NULL){
					GlobalFree(tpItemInfo->Title);
				}
				tpItemInfo->Title = GlobalAlloc(GPTR, lstrlen(titlebuf) + 1);
				if(tpItemInfo->Title != NULL){
					lstrcpy(tpItemInfo->Title, titlebuf);
				}
				GlobalFree(titlebuf);
				tpItemInfo->RefreshFlag = TRUE;
			}
		}
		SendMessage(hWnd, WM_CHECK_RESULT, ST_DEFAULT, (LPARAM)tpItemInfo);
		return CHECK_END;
	}

	if(Status == 304){
		//更新が無かった場合
		SendMessage(hWnd, WM_CHECK_RESULT, ST_DEFAULT, (LPARAM)tpItemInfo);
		return CHECK_END;
	}

	//チェック設定
	DateCheck = !GetOptionInt(tpItemInfo->Option1, OP1_NODATE);
	SizeCheck = !GetOptionInt(tpItemInfo->Option1, OP1_NOSIZE);

	switch(Status)
	{
	case 200:	/* 成功の場合 */
		if(tpItemInfo->user2 == REQUEST_HEAD){
			//HEAD
			//更新日時を取得
			CmpMsg |= Head_GetDate(tpItemInfo, DateCheck, &DateRet);
			//サイズを取得
			CmpMsg |= Head_GetSize(tpItemInfo, SizeCheck, DateRet, &SizeRet);
			if(SizeRet == 0 && DateRet == 0){
				//GETリクエスト送信のためのチェック待ち状態
				if(tpItemInfo->Option1 == NULL || *tpItemInfo->Option1 == '\0'){
					if(tpItemInfo->Option1 != NULL) GlobalFree(tpItemInfo->Option1);
					tpItemInfo->Option1 = (char *)GlobalAlloc(GPTR, 2);
					lstrcpy(tpItemInfo->Option1, "1");
				}else{
					*tpItemInfo->Option1 = '1';
				}
				return CHECK_NO;
			}else{
				//チェックの終了をWWWCに通知
				SendMessage(hWnd, WM_CHECK_RESULT, CmpMsg, (LPARAM)tpItemInfo);
			}
			break;
		}

		//GET
		if(GetOptionInt(tpItemInfo->Option1, OP1_META) == 1){
			//METAタグを取得
			CmpMsg |= GetMetaString(tpItemInfo);
			SetDate = 1;
		}else{
			//ヘッダから更新日時を取得
			CmpMsg |= Head_GetDate(tpItemInfo, DateCheck, &SetDate);
		}
		if(GetHeadContentSize(tpHTTP->buf, "Content-Length:") > 0 &&
			GetOptionInt(tpItemInfo->Option1, OP1_NOTAGSIZE) == 0 && tpHTTP->FilterFlag == FALSE){
			if(tpItemInfo->Option1 != NULL && *tpItemInfo->Option1 != '\0' && *tpItemInfo->Option1 != '2'){
				*tpItemInfo->Option1 = '0';
			}
			//ヘッダからサイズを取得
			CmpMsg |= Head_GetSize(tpItemInfo, SizeCheck, SetDate, &SizeRet);
		}
		if(SizeRet == 0){
			//バイト情報の除去
			tpHTTP->Size = DeleteSizeInfo(tpHTTP->buf, tpHTTP->Size);
			//フィルタ処理
			if(tpHTTP->FilterFlag == TRUE &&
				FilterCheck(tpItemInfo->CheckURL, tpHTTP->buf + HeaderSize(tpHTTP->buf), tpHTTP->Size) == FALSE){
				tpHTTP->FilterFlag = FALSE;
				if(tpItemInfo->Date != NULL){
					GlobalFree(tpItemInfo->Date);
				}
				tpItemInfo->Date = (char *)GlobalAlloc(GMEM_FIXED, lstrlen("filter error") + 1);
				if(tpItemInfo->Date != NULL){
					lstrcpy(tpItemInfo->Date, "filter error");
				}
				SetDate = 1;
			}
			//HTMLのサイズを取得
			CmpMsg |= Get_GetSize(tpItemInfo, SizeCheck, SetDate);
		}
		if(GetOptionInt(tpItemInfo->Option1, OP1_MD5) == 1){
			//MD5のダイジェスト値でチェック
			CmpMsg |= Get_MD5Check(tpItemInfo, SetDate);
		}
		//チェックの終了をWWWCに通知
		SendMessage(hWnd, WM_CHECK_RESULT, CmpMsg, (LPARAM)tpItemInfo);
		break;

	case 301:	/* 移動の場合 */
	case 302:
	case 303:
	case 307:
	case 308:
		if(tpItemInfo->Param4 > 10){
			tpItemInfo->Param4 = 0;
			SetErrorString(tpItemInfo, STR_ERROR_MOVE, FALSE);
			SendMessage(hWnd, WM_CHECK_RESULT, ST_ERROR, (LPARAM)tpItemInfo);
			break;
		}
		len = GetHeadContentSize(tpHTTP->buf, "Location:");
		headcontent = (char *)GlobalAlloc(GPTR, len + 1);
		if(headcontent != NULL && GetHeadContent(tpHTTP->buf, "Location:", headcontent) == TRUE){
			DelCtrlChar(headcontent);
			//現在のURLを基準に移動先のURLを作成 (http, https 以外は NULL)
			p = CreateMoveURL((tpItemInfo->Param2 != 0) ? (char *)tpItemInfo->Param2 : tpItemInfo->CheckURL, headcontent);
			GlobalFree(headcontent);
			if(p == NULL){
				SetErrorString(tpItemInfo, tpHTTP->buf, TRUE);
				SendMessage(hWnd, WM_CHECK_RESULT, ST_ERROR, (LPARAM)tpItemInfo);
				break;
			}
			tpItemInfo->Param4++;

			//恒久的な移動の場合はチェックするURLを変更する
			//認証情報を使用するアイテムは別の接続先に認証情報を送らないよう同じ接続先の場合のみ変更する
			if((Status == 301 || Status == 308) &&
				(HasItemAuth(tpItemInfo) == FALSE || IsSameOrigin(p, tpItemInfo->CheckURL) == TRUE)){
				r = (char *)GlobalAlloc(GPTR, lstrlen(p) + 1);
				if(r != NULL){
					lstrcpy(r, p);
					if(tpItemInfo->CheckURL != NULL){
						GlobalFree((HGLOBAL)tpItemInfo->CheckURL);
					}
					tpItemInfo->CheckURL = r;
				}
			}
			if(tpItemInfo->Param2 != 0){
				GlobalFree((HGLOBAL)tpItemInfo->Param2);
			}
			tpItemInfo->Param2 = (long)p;

			//リクエストの初期化
			return CHECK_NO;
		}else{
			if(headcontent != NULL) GlobalFree(headcontent);
			SetErrorString(tpItemInfo, tpHTTP->buf, TRUE);
			SendMessage(hWnd, WM_CHECK_RESULT, ST_ERROR, (LPARAM)tpItemInfo);
		}
		break;

	case 408:	/* サーバタイムアウトの場合 */
		SetErrorString(tpItemInfo, tpHTTP->buf, TRUE);
		SendMessage(hWnd, WM_CHECK_RESULT, ST_TIMEOUT, (LPARAM)tpItemInfo);
		break;

	default:	/* エラーの場合 */
		SetErrorString(tpItemInfo, tpHTTP->buf, TRUE);
		SendMessage(hWnd, WM_CHECK_RESULT, ST_ERROR, (LPARAM)tpItemInfo);
		break;
	}
	return CHECK_END;
}


/******************************************************************************

	SetSBText

	WWWCのステータスバーにテキストを表示する

******************************************************************************/

static void SetSBText(HWND hWnd, struct TPITEM *tpItemInfo, char *msg)
{
	char *buf, *p;

	//ステータスバーにテキストを送る。
	if (tpItemInfo->Param2 != 0) {
		buf = (char *)HeapAlloc(GetProcessHeap(), 0, lstrlen(msg) + lstrlen((char *)tpItemInfo->Param2) + 4);
		if (buf == NULL) {
			return;
		}
		p = iStrCpy(buf, msg);
		*(p++) = ' ';
		*(p++) = '(';
		p = iStrCpy(p, (char *)tpItemInfo->Param2);
		*(p++) = ')';
		*(p++) = '\0';
		SendMessage(GetDlgItem(hWnd, WWWC_SB), SB_SETTEXT, (WPARAM)0 | 0, (LPARAM)buf);
		HeapFree(GetProcessHeap(), 0, buf);
	} else {
		SendMessage(GetDlgItem(hWnd, WWWC_SB), SB_SETTEXT, (WPARAM)0 | 0, (LPARAM)msg);
	}
}


/******************************************************************************

	CheckHead

	受信済みデータに必要な情報を取得しているかチェック

******************************************************************************/

static BOOL CheckHead(struct TPHTTP *tpHTTP)
{
	char *p;

	//すべて受信する必要がある場合
	if(tpHTTP->NeedBody == TRUE){
		tpHTTP->HeadCheckFlag = TRUE;
		return FALSE;
	}

	//ヘッダをすべて受信しているかチェック
	p = tpHTTP->buf;
	while(1){
		p = StrNextContent(p);
		if(*p == '\0'){
			return FALSE;
		}
		if(*p == '\r' || *p == '\n'){
			break;
		}
	}

	//ヘッダ表示の場合は受信を打ち切る
	if(tpHTTP->Mode == 1){
		return TRUE;
	}

	//ヘッダにサイズが設定されているかチェック
	if(tpHTTP->Mode == 0 && GetHeadContentSize(tpHTTP->buf, "Content-Length:") <= 0){
		tpHTTP->HeadCheckFlag = TRUE;
		return FALSE;
	}
	return TRUE;
}


/******************************************************************************

	HTTP_FreeItem

	アイテム情報の解放

******************************************************************************/

__declspec(dllexport) int CALLBACK HTTP_FreeItem(struct TPITEM *tpItemInfo)
{
	struct TPHTTP *tpHTTP;

	if(tpItemInfo->Param1 != 0){
		//受信中の場合は中止してアイテムから切り離す (スレッドが終了した時点で解放される)
		tpHTTP = (struct TPHTTP *)tpItemInfo->Param1;
		tpItemInfo->Param1 = 0;
		CancelHTTP(tpHTTP);
		ReleaseHTTP(tpHTTP);
	}
	if(tpItemInfo->Param2 != 0){
		GlobalFree((HGLOBAL)tpItemInfo->Param2);
		tpItemInfo->Param2 = 0;
	}
	tpItemInfo->Param3 = 0;
	return 0;
}


/******************************************************************************

	HTTP_InitItem

	アイテムの初期化

******************************************************************************/

__declspec(dllexport) int CALLBACK HTTP_InitItem(HWND hWnd, struct TPITEM *tpItemInfo)
{
	if(tpItemInfo->ErrStatus != NULL){
		GlobalFree(tpItemInfo->ErrStatus);
		tpItemInfo->ErrStatus = NULL;
	}
	if((tpItemInfo->Status & ST_UP) == 0){
		/* 更新マーク(*)を除去する */
		if(tpItemInfo->Size != NULL && *tpItemInfo->Size != '\0' &&
			*(tpItemInfo->Size + lstrlen(tpItemInfo->Size) - 1) == '*'){
			*(tpItemInfo->Size + lstrlen(tpItemInfo->Size) - 1) = '\0';
		}
		if(tpItemInfo->Date != NULL && *tpItemInfo->Date != '\0' &&
			*(tpItemInfo->Date + lstrlen(tpItemInfo->Date) - 1) == '*'){
			*(tpItemInfo->Date + lstrlen(tpItemInfo->Date) - 1) = '\0';
		}
		if(tpItemInfo->DLLData2 != NULL && *tpItemInfo->DLLData2 != '\0' &&
			*(tpItemInfo->DLLData2 + lstrlen(tpItemInfo->DLLData2) - 1) == '*'){
			*(tpItemInfo->DLLData2 + lstrlen(tpItemInfo->DLLData2) - 1) = '\0';
		}
	}
	return 0;
}


/******************************************************************************

	HTTP_Initialize

	チェック開始の初期化

******************************************************************************/

__declspec(dllexport) int CALLBACK HTTP_Initialize(HWND hWnd, struct TPITEM *tpItemInfo)
{
	//前回のチェックで残った移動先のURLを解放
	if(tpItemInfo->Param1 == 0 && tpItemInfo->Param2 != 0){
		GlobalFree((HGLOBAL)tpItemInfo->Param2);
		tpItemInfo->Param2 = 0;
	}
	tpItemInfo->user1 = 0;
	tpItemInfo->user2 = 0;
	tpItemInfo->Param4 = 0;
	return 0;
}


/******************************************************************************

	HTTP_Cancel

	チェックのキャンセル

******************************************************************************/

__declspec(dllexport) int CALLBACK HTTP_Cancel(HWND hWnd, struct TPITEM *tpItemInfo)
{
	struct TPHTTP *tpHTTP;
	HWND vWnd;
	char *titlebuf;

	tpHTTP = (struct TPHTTP *)tpItemInfo->Param1;
	if (tpHTTP == NULL) {
		return 0;
	}
	//受信を中止してからバッファを参照する (中止後はスレッドはバッファを変更しない)
	EnterCriticalSection(&tpHTTP->cs);
	tpHTTP->CancelFlag = TRUE;
	CloseRequestHandles(tpHTTP);
	LeaveCriticalSection(&tpHTTP->cs);

	switch(tpItemInfo->Param3){
	case 1:
	case 2:
		//ヘッダ、ソース表示
		vWnd = CreateDialogParam(ghinst, MAKEINTRESOURCE(IDD_DIALOG_VIEW), NULL, ViewProc, (long)tpItemInfo);
		ShowWindow(vWnd, SW_SHOW);
		break;

	case 3:
		//タイトル取得
		if(tpHTTP->buf != NULL && (titlebuf = GetTitle(tpHTTP->buf + HeaderSize(tpHTTP->buf))) != NULL){
			if(tpItemInfo->Title != NULL){
				GlobalFree(tpItemInfo->Title);
			}
			tpItemInfo->Title = GlobalAlloc(GPTR, lstrlen(titlebuf) + 1);
			if(tpItemInfo->Title != NULL){
				lstrcpy(tpItemInfo->Title, titlebuf);
			}
			GlobalFree(titlebuf);
			tpItemInfo->RefreshFlag = TRUE;
		}
		break;
	}
	HTTP_FreeItem(tpItemInfo);
	return 0;
}


/******************************************************************************

	HTTP_Timer

	タイマー (1秒間隔)

******************************************************************************/

__declspec(dllexport) int CALLBACK HTTP_Timer(HWND hWnd, struct TPITEM *tpItemInfo)
{
	if(tpItemInfo->user1 == -1){
		return CHECK_SUCCEED;
	}
	tpItemInfo->user1++;
	if(tpItemInfo->user1 == TimeOut){
		if(tpItemInfo->Param3 == 1 || tpItemInfo->Param3 == 2){
			tpItemInfo->user1 = -1;
			MessageBox(hWnd, STR_ERROR_TIMEOUT, tpItemInfo->CheckURL, MB_ICONERROR);
		}
		//受信を中止 (スレッドは強制終了せず、WinHTTPのハンドルを閉じて終了させる)
		HTTP_FreeItem(tpItemInfo);
		SendMessage(hWnd, WM_CHECK_RESULT, ST_TIMEOUT, (LPARAM)tpItemInfo);
		return CHECK_END;
	}
	return CHECK_SUCCEED;
}


/******************************************************************************

	FreeURLData

	URL情報の解放

******************************************************************************/

static void FreeURLData(struct TPHTTP *tpHTTP)
{
	if(tpHTTP->SvName != NULL) GlobalFree((HGLOBAL)tpHTTP->SvName);
	if(tpHTTP->Path != NULL) GlobalFree((HGLOBAL)tpHTTP->Path);
	if(tpHTTP->hSvName != NULL) GlobalFree((HGLOBAL)tpHTTP->hSvName);
	if(tpHTTP->user != NULL) GlobalFree((HGLOBAL)tpHTTP->user);
	if(tpHTTP->pass != NULL) GlobalFree((HGLOBAL)tpHTTP->pass);
}


/******************************************************************************

	GetServerPort

	接続するサーバとポートを取得する

******************************************************************************/

static BOOL GetServerPort(HWND hWnd, struct TPITEM *tpItemInfo, struct TPHTTP *tpHTTP)
{
	char ItemProxy[BUFSIZE];
	char ItemPort[BUFSIZE];
	char *px = "";
	int pt;
	int ProxyFlag;

	ProxyFlag = GetOptionInt(tpItemInfo->Option2, OP2_NOPROXY);
	if(ProxyFlag == 0 && GetOptionInt(tpItemInfo->Option2, OP2_SETPROXY) == 1){
		//アイテムに設定されたプロキシを使用
		ProxyFlag = 2;
		GetOptionString(tpItemInfo->Option2, ItemProxy, OP2_PROXY);
		GetOptionString(tpItemInfo->Option2, ItemPort, OP2_PORT);

		px = ItemProxy;
		pt = atoi(ItemPort);

	}else if(ProxyFlag == 0 && Proxy == 1){
		px = pServer;
		pt = pPort;
	}

	if((ProxyFlag == 0 && Proxy == 1) || ProxyFlag == 2){
		//Proxyのサーバ名とポート番号を取得
		tpHTTP->SvName = (char *)GlobalAlloc(GPTR, lstrlen(px) + 1);
		if(tpHTTP->SvName == NULL){
			if(tpItemInfo->Param3 == 1 || tpItemInfo->Param3 == 2){
				tpItemInfo->user1 = -1;
				MessageBox(hWnd, STR_ERROR_MEMORY, tpItemInfo->CheckURL, MB_ICONERROR);
			}
			SetErrorString(tpItemInfo, STR_ERROR_MEMORY, FALSE);
			return FALSE;
		}
		tpHTTP->Port = GetURL((char *)px, tpHTTP->SvName, NULL, pt, NULL, NULL);
		if(tpHTTP->Port == -1){
			lstrcpy(tpHTTP->SvName, px);
			tpHTTP->Port = pt;
		}

		//Proxyに渡すURLを設定
		tpHTTP->Path = (char *)GlobalAlloc(GPTR, lstrlen((char *)tpItemInfo->Param2) + 1);
		if(tpHTTP->Path == NULL){
			if(tpItemInfo->Param3 == 1 || tpItemInfo->Param3 == 2){
				tpItemInfo->user1 = -1;
				MessageBox(hWnd, STR_ERROR_MEMORY, tpItemInfo->CheckURL, MB_ICONERROR);
			}
			SetErrorString(tpItemInfo, STR_ERROR_MEMORY, FALSE);
			return FALSE;
		}
		lstrcpy(tpHTTP->Path, (char *)tpItemInfo->Param2);

		//取得するURLからサーバ名とポート番号を取得
		tpHTTP->hSvName = (char *)GlobalAlloc(GPTR, lstrlen((char *)tpItemInfo->Param2) + 1);
		if(tpHTTP->hSvName == NULL){
			if(tpItemInfo->Param3 == 1 || tpItemInfo->Param3 == 2){
				tpItemInfo->user1 = -1;
				MessageBox(hWnd, STR_ERROR_MEMORY, tpItemInfo->CheckURL, MB_ICONERROR);
			}
			SetErrorString(tpItemInfo, STR_ERROR_MEMORY, FALSE);
			return FALSE;
		}
		tpHTTP->user = (char *)GlobalAlloc(GPTR, lstrlen((char *)tpItemInfo->Param2) + 1);
		if(tpHTTP->user == NULL){
			if(tpItemInfo->Param3 == 1 || tpItemInfo->Param3 == 2){
				tpItemInfo->user1 = -1;
				MessageBox(hWnd, STR_ERROR_MEMORY, tpItemInfo->CheckURL, MB_ICONERROR);
			}
			SetErrorString(tpItemInfo, STR_ERROR_MEMORY, FALSE);
			return FALSE;
		}
		tpHTTP->pass = (char *)GlobalAlloc(GPTR, lstrlen((char *)tpItemInfo->Param2) + 1);
		if(tpHTTP->pass == NULL){
			if(tpItemInfo->Param3 == 1 || tpItemInfo->Param3 == 2){
				tpItemInfo->user1 = -1;
				MessageBox(hWnd, STR_ERROR_MEMORY, tpItemInfo->CheckURL, MB_ICONERROR);
			}
			SetErrorString(tpItemInfo, STR_ERROR_MEMORY, FALSE);
			return FALSE;
		}
		tpHTTP->hPort = GetURL((char *)tpItemInfo->Param2, tpHTTP->hSvName, NULL, 80, tpHTTP->user, tpHTTP->pass);
		if(tpHTTP->Port == -1){
			if(tpItemInfo->Param3 == 1 || tpItemInfo->Param3 == 2){
				tpItemInfo->user1 = -1;
				MessageBox(hWnd, STR_ERROR_URL, tpItemInfo->CheckURL, MB_ICONERROR);
			}
			SetErrorString(tpItemInfo, STR_ERROR_URL, FALSE);
			return FALSE;
		}
		return TRUE;
	}

	/* URLからサーバ名とパスを取得する */
	tpHTTP->SvName = (char *)GlobalAlloc(GPTR, lstrlen((char *)tpItemInfo->Param2) + 1);
	if(tpHTTP->SvName == NULL){
		if(tpItemInfo->Param3 == 1 || tpItemInfo->Param3 == 2){
			tpItemInfo->user1 = -1;
			MessageBox(hWnd, STR_ERROR_MEMORY, tpItemInfo->CheckURL, MB_ICONERROR);
		}
		SetErrorString(tpItemInfo, STR_ERROR_MEMORY, FALSE);
		return FALSE;
	}
	tpHTTP->Path = (char *)GlobalAlloc(GPTR, lstrlen((char *)tpItemInfo->Param2) + 1);
	if(tpHTTP->Path == NULL){
		if(tpItemInfo->Param3 == 1 || tpItemInfo->Param3 == 2){
			tpItemInfo->user1 = -1;
			MessageBox(hWnd, STR_ERROR_MEMORY, tpItemInfo->CheckURL, MB_ICONERROR);
		}
		SetErrorString(tpItemInfo, STR_ERROR_MEMORY, FALSE);
		return FALSE;
	}
	tpHTTP->user = (char *)GlobalAlloc(GPTR, lstrlen((char *)tpItemInfo->Param2) + 1);
	if(tpHTTP->user == NULL){
		if(tpItemInfo->Param3 == 1 || tpItemInfo->Param3 == 2){
			tpItemInfo->user1 = -1;
			MessageBox(hWnd, STR_ERROR_MEMORY, tpItemInfo->CheckURL, MB_ICONERROR);
		}
		SetErrorString(tpItemInfo, STR_ERROR_MEMORY, FALSE);
		return FALSE;
	}
	tpHTTP->pass = (char *)GlobalAlloc(GPTR, lstrlen((char *)tpItemInfo->Param2) + 1);
	if(tpHTTP->pass == NULL){
		if(tpItemInfo->Param3 == 1 || tpItemInfo->Param3 == 2){
			tpItemInfo->user1 = -1;
			MessageBox(hWnd, STR_ERROR_MEMORY, tpItemInfo->CheckURL, MB_ICONERROR);
		}
		SetErrorString(tpItemInfo, STR_ERROR_MEMORY, FALSE);
		return FALSE;
	}
	tpHTTP->Port = GetURL((char *)tpItemInfo->Param2, tpHTTP->SvName, tpHTTP->Path, 80, tpHTTP->user, tpHTTP->pass);
	if(tpHTTP->Port == -1){
		if(tpItemInfo->Param3 == 1 || tpItemInfo->Param3 == 2){
			tpItemInfo->user1 = -1;
			MessageBox(hWnd, STR_ERROR_URL, tpItemInfo->CheckURL, MB_ICONERROR);
		}
		SetErrorString(tpItemInfo, STR_ERROR_URL, FALSE);
		return FALSE;
	}
	tpHTTP->hSvName = (char *)GlobalAlloc(GPTR, lstrlen(tpHTTP->SvName) + 1);
	if(tpHTTP->hSvName == NULL){
		if(tpItemInfo->Param3 == 1 || tpItemInfo->Param3 == 2){
			tpItemInfo->user1 = -1;
			MessageBox(hWnd, STR_ERROR_MEMORY, tpItemInfo->CheckURL, MB_ICONERROR);
		}
		SetErrorString(tpItemInfo, STR_ERROR_MEMORY, FALSE);
		return FALSE;
	}
	lstrcpy(tpHTTP->hSvName, tpHTTP->SvName);
	tpHTTP->hPort = tpHTTP->Port;
	return TRUE;
}


/******************************************************************************

	CheckServer

	既にチェック中のサーバかどうかチェックする

******************************************************************************/

static BOOL CheckServer(HWND hWnd, char *RealServer)
{
	struct TPITEM **CheckItemList;
	char *SvName;
	int i, j;
	int Port;

	j = SendMessage(hWnd, WM_GETCHECKLISTCNT, 0, 0);
	CheckItemList = (struct TPITEM **)SendMessage(hWnd, WM_GETCHECKLIST, 0, 0);
	if(CheckItemList == NULL){
		return FALSE;
	}
	for(i = 0; i < j; i++){
		if(*(CheckItemList + i) == NULL){
			continue;
		}
		SvName = (char *)HeapAlloc(GetProcessHeap(), 0, lstrlen((*(CheckItemList + i))->CheckURL) + 1);
		if(SvName == NULL) return FALSE;
		*SvName = '\0';
		Port = GetURL((*(CheckItemList + i))->CheckURL, SvName, NULL, 80, NULL, NULL);
		if(Port != -1 && lstrcmp(RealServer, SvName) == 0){
			HeapFree(GetProcessHeap(), 0, SvName);
			return TRUE;
		}
		HeapFree(GetProcessHeap(), 0, SvName);
	}
	return FALSE;
}


/******************************************************************************

	AllocHTTP

	チェック情報の確保 (アイテムからの参照を1つ持つ)

******************************************************************************/

static struct TPHTTP *AllocHTTP(HWND hWnd, struct TPITEM *tpItemInfo)
{
	struct TPHTTP *tpHTTP;

	tpHTTP = (struct TPHTTP *)GlobalAlloc(GPTR, sizeof(struct TPHTTP));
	if(tpHTTP == NULL){
		return NULL;
	}
	InitializeCriticalSection(&tpHTTP->cs);
	tpHTTP->RefCnt = 1;
	tpHTTP->hWnd = hWnd;
	tpHTTP->tpItemInfo = tpItemInfo;
	return tpHTTP;
}


/******************************************************************************

	FreeWideString

	ワイド文字列を消去して解放

******************************************************************************/

static void FreeWideString(WCHAR **str)
{
	if(*str != NULL){
		SecureZeroMemory(*str, lstrlenW(*str) * sizeof(WCHAR));
		HeapFree(GetProcessHeap(), 0, *str);
		*str = NULL;
	}
}


/******************************************************************************

	CloseRequestHandles

	WinHTTPのハンドルを閉じる (cs を取得して呼ぶ)

******************************************************************************/

static void CloseRequestHandles(struct TPHTTP *tpHTTP)
{
	if(tpHTTP->hRequest != NULL){
		WinHttpCloseHandle(tpHTTP->hRequest);
		tpHTTP->hRequest = NULL;
	}
	if(tpHTTP->hConnect != NULL){
		WinHttpCloseHandle(tpHTTP->hConnect);
		tpHTTP->hConnect = NULL;
	}
	if(tpHTTP->hSession != NULL){
		WinHttpCloseHandle(tpHTTP->hSession);
		tpHTTP->hSession = NULL;
	}
}


/******************************************************************************

	ReleaseHTTP

	チェック情報の参照を解放し、参照が無くなった場合は削除する

******************************************************************************/

static void ReleaseHTTP(struct TPHTTP *tpHTTP)
{
	if(InterlockedDecrement(&tpHTTP->RefCnt) != 0){
		return;
	}
	CloseRequestHandles(tpHTTP);
	if(tpHTTP->hThread != NULL){
		CloseHandle(tpHTTP->hThread);
	}
	if(tpHTTP->buf != NULL){
		GlobalFree((HGLOBAL)tpHTTP->buf);
	}
	FreeURLData(tpHTTP);
	FreeWideString(&tpHTTP->ReqURL);
	FreeWideString(&tpHTTP->ProxyName);
	FreeWideString(&tpHTTP->SendHeader);
	FreeWideString(&tpHTTP->ProxyUser);
	FreeWideString(&tpHTTP->ProxyPass);
	DeleteCriticalSection(&tpHTTP->cs);
	GlobalFree((HGLOBAL)tpHTTP);
}


/******************************************************************************

	CancelHTTP

	受信を中止してアイテムから切り離す (メインスレッドから呼ぶ)

******************************************************************************/

static void CancelHTTP(struct TPHTTP *tpHTTP)
{
	EnterCriticalSection(&tpHTTP->cs);
	tpHTTP->CancelFlag = TRUE;
	tpHTTP->tpItemInfo = NULL;
	//ハンドルを閉じると処理中のWinHTTPの関数は失敗して戻る
	CloseRequestHandles(tpHTTP);
	LeaveCriticalSection(&tpHTTP->cs);
}


/******************************************************************************

	IsSameOrigin

	スキーム、サーバ名、ポート番号が同じURLかどうかチェックする

******************************************************************************/

static BOOL IsSameOrigin(char *URL1, char *URL2)
{
	char *sv1, *sv2;
	int port1, port2;
	BOOL secure1, secure2;
	BOOL ret = FALSE;

	if(URL1 == NULL || URL2 == NULL){
		return FALSE;
	}
	secure1 = (lstrcmpni(URL1, "https://", 8) == 0) ? TRUE : FALSE;
	secure2 = (lstrcmpni(URL2, "https://", 8) == 0) ? TRUE : FALSE;
	if(secure1 != secure2){
		return FALSE;
	}
	sv1 = (char *)GlobalAlloc(GPTR, lstrlen(URL1) + 1);
	sv2 = (char *)GlobalAlloc(GPTR, lstrlen(URL2) + 1);
	if(sv1 != NULL && sv2 != NULL){
		port1 = GetURL(URL1, sv1, NULL, (secure1 == TRUE) ? 443 : 80, NULL, NULL);
		port2 = GetURL(URL2, sv2, NULL, (secure2 == TRUE) ? 443 : 80, NULL, NULL);
		ret = (port1 != -1 && port1 == port2 && lstrcmpi(sv1, sv2) == 0) ? TRUE : FALSE;
	}
	if(sv1 != NULL) GlobalFree(sv1);
	if(sv2 != NULL) GlobalFree(sv2);
	return ret;
}


/******************************************************************************

	HasItemAuth

	アイテムに認証情報が設定されているかチェックする

******************************************************************************/

static BOOL HasItemAuth(struct TPITEM *tpItemInfo)
{
	char *sv, *user;
	BOOL ret = FALSE;

	if(GetOptionInt(tpItemInfo->Option2, OP2_USEPASS) == 1){
		return TRUE;
	}
	if(tpItemInfo->CheckURL == NULL){
		return FALSE;
	}
	sv = (char *)GlobalAlloc(GPTR, lstrlen(tpItemInfo->CheckURL) + 1);
	user = (char *)GlobalAlloc(GPTR, lstrlen(tpItemInfo->CheckURL) + 1);
	if(sv != NULL && user != NULL){
		ret = (GetURL(tpItemInfo->CheckURL, sv, NULL, 80, user, NULL) != -1 && *user != '\0') ? TRUE : FALSE;
	}
	if(sv != NULL) GlobalFree(sv);
	if(user != NULL) GlobalFree(user);
	return ret;
}


/******************************************************************************

	AllocWideString

	ワイド文字列に変換して確保

******************************************************************************/

static WCHAR *AllocWideString(char *buf)
{
	WCHAR *ret;
	int len;

	len = MultiByteToWideChar(CP_ACP, 0, buf, -1, NULL, 0);
	if(len <= 0){
		return NULL;
	}
	ret = (WCHAR *)HeapAlloc(GetProcessHeap(), 0, len * sizeof(WCHAR));
	if(ret == NULL){
		return NULL;
	}
	if(MultiByteToWideChar(CP_ACP, 0, buf, -1, ret, len) == 0){
		HeapFree(GetProcessHeap(), 0, ret);
		return NULL;
	}
	return ret;
}


/******************************************************************************

	BuildRequest

	リクエストの情報を作成する (スレッドからアイテムを参照しないよう事前に作成)

******************************************************************************/

static BOOL BuildRequest(struct TPITEM *tpItemInfo, struct TPHTTP *tpHTTP)
{
	char BaseStr1[BUFSIZE * 2 + 2];
	char BaseStr2[BUFSIZE * 3];
	char ProxyAuth[BUFSIZE * 3];
	char user[BUFSIZE];
	char pass[BUFSIZE];
	char referer[BUFSIZE];
	char proxy[BUFSIZE * 2];
	char *sv;
	WCHAR *s;
	DWORD dwSize;
	BOOL Secure;
	BOOL AuthFlag;
	int ProxyFlag;

	//要求するURL
	tpHTTP->ReqURL = AllocWideString((char *)tpItemInfo->Param2);
	if(tpHTTP->ReqURL == NULL){
		return FALSE;
	}
	Secure = (lstrcmpni((char *)tpItemInfo->Param2, "https://", 8) == 0) ? TRUE : FALSE;

	//プロキシ
	ProxyFlag = GetOptionInt(tpItemInfo->Option2, OP2_NOPROXY);
	if((ProxyFlag == 0 && Proxy == 1) || ProxyFlag == 2){
		wsprintf(proxy, "http://%s:%d", tpHTTP->SvName, tpHTTP->Port);
		tpHTTP->ProxyName = AllocWideString(proxy);
		if(tpHTTP->ProxyName == NULL){
			return FALSE;
		}
	}

	//処理の種類
	tpHTTP->HeadRequest = (tpItemInfo->user2 == REQUEST_HEAD) ? TRUE : FALSE;
	tpHTTP->Mode = tpItemInfo->Param3;
	tpHTTP->NeedBody = (tpItemInfo->Param3 >= 2 || tpHTTP->FilterFlag == TRUE ||
		GetOptionInt(tpItemInfo->Option1, OP1_META) == 1 ||
		GetOptionInt(tpItemInfo->Option1, OP1_MD5) == 1 ||
		GetOptionInt(tpItemInfo->Option1, OP1_NOTAGSIZE) == 1) ? TRUE : FALSE;
	tpHTTP->TimeOutMs = (TimeOut > 0) ? TimeOut * 1000 : 0;

	//プロキシ認証
	*ProxyAuth = '\0';
	if(tpHTTP->ProxyName != NULL && pUsePass == 1){
		if(Secure == TRUE){
			//HTTPSではヘッダが接続先サーバに送られるためプロキシ用の資格情報として設定する
			tpHTTP->ProxyUser = AllocWideString(pUser);
			tpHTTP->ProxyPass = AllocWideString(pPass);
		}else{
			wsprintf(BaseStr1, "%s:%s", pUser, pPass);
			eBase(BaseStr1, ProxyAuth);
		}
	}
	//Referer
	if(GetOptionString(tpItemInfo->Option2, referer, OP2_REFERER) == FALSE){
		*referer = '\0';
	}

	//ページ認証
	//アイテムに設定した認証情報は、チェックするURLと同じ接続先 (スキーム、サーバ、ポート) にのみ送る
	AuthFlag = IsSameOrigin((char *)tpItemInfo->Param2, tpItemInfo->CheckURL);
	*user = '\0';
	*pass = '\0';
	*BaseStr2 = '\0';
	if(tpHTTP->user[0] != '\0'){
		//要求するURLに含まれる認証情報
		lstrcpyn(user, tpHTTP->user, BUFSIZE);
		lstrcpyn(pass, tpHTTP->pass, BUFSIZE);
	}else if(AuthFlag == TRUE && tpItemInfo->CheckURL != NULL &&
		(sv = (char *)GlobalAlloc(GPTR, lstrlen(tpItemInfo->CheckURL) + 1)) != NULL){
		//チェックするURLに含まれる認証情報
		GetURL(tpItemInfo->CheckURL, sv, NULL, 80, user, pass);
		GlobalFree(sv);
	}
	if(*user != '\0'){
		wsprintf(BaseStr1, "%s:%s", user, pass);
		eBase(BaseStr1, BaseStr2);
	}
	if(AuthFlag == TRUE && GetOptionInt(tpItemInfo->Option2, OP2_USEPASS) == 1){
		if(GetOptionString(tpItemInfo->Option2, user, OP2_USER) == FALSE){
			*user = '\0';
		}
		if(GetOptionString(tpItemInfo->Option2, BaseStr2, OP2_PASS) == FALSE){
			*pass = '\0';
		}else{
			dPass(BaseStr2, pass);
		}
		wsprintf(BaseStr1, "%s:%s", user, pass);
		eBase(BaseStr1, BaseStr2);
	}
	SecureZeroMemory(BaseStr1, sizeof(BaseStr1));
	SecureZeroMemory(pass, sizeof(pass));
	//認証情報を送る場合はリダイレクト先に送られないよう自動リダイレクトを行わない
	tpHTTP->NoRedirect = (*BaseStr2 != '\0') ? TRUE : FALSE;

	//送信ヘッダのサイズ (固定文字列分は BUFSIZE に含む)
	dwSize = BUFSIZE + lstrlen(ProxyAuth) + lstrlen(referer) + lstrlen(BaseStr2);
	if(tpItemInfo->DLLData1 != NULL){
		dwSize += lstrlen(tpItemInfo->DLLData1);
	}
	tpHTTP->SendHeader = (WCHAR *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwSize * sizeof(WCHAR));
	if(tpHTTP->SendHeader == NULL){
		SecureZeroMemory(ProxyAuth, sizeof(ProxyAuth));
		SecureZeroMemory(BaseStr2, sizeof(BaseStr2));
		return FALSE;
	}
	s = iStrCpyW(tpHTTP->SendHeader, "Accept: */*");
	//If-Modified-Since
	if(tpItemInfo->user2 == REQUEST_GET && tpItemInfo->Param3 == 0 &&
		tpItemInfo->DLLData1 != NULL && *tpItemInfo->DLLData1 != '\0' &&
		GetOptionInt(tpItemInfo->Option1, OP1_NODATE) == 0 &&
		GetOptionInt(tpItemInfo->Option1, OP1_META) == 0 &&
		GetOptionInt(tpItemInfo->Option1, OP1_MD5) == 0 &&
		GetOptionInt(tpItemInfo->Option1, OP1_NOTAGSIZE) == 0){
		//GETで更新日でチェックする場合は強制的にIMSを発行
		s = iStrCpyW(s, "\r\nIf-Modified-Since: ");
		s = iStrCpyW(s, tpItemInfo->DLLData1);
	}
	//キャッシュ制御
	if(tpHTTP->ProxyName != NULL && pNoCache == 1){
		s = iStrCpyW(s, "\r\nPragma: no-cache\r\nCache-Control: no-cache");
	}
	//プロキシ認証
	if(*ProxyAuth != '\0'){
		s = iStrCpyW(s, "\r\nProxy-Authorization: Basic ");
		s = iStrCpyW(s, ProxyAuth);
	}
	//Referer
	if(*referer != '\0'){
		s = iStrCpyW(s, "\r\nReferer: ");
		s = iStrCpyW(s, referer);
	}
	//ページ認証
	if(*BaseStr2 != '\0'){
		s = iStrCpyW(s, "\r\nAuthorization: Basic ");
		s = iStrCpyW(s, BaseStr2);
	}
	SecureZeroMemory(ProxyAuth, sizeof(ProxyAuth));
	SecureZeroMemory(BaseStr2, sizeof(BaseStr2));
	return TRUE;
}


/******************************************************************************

	SetThreadError

	スレッドで発生したエラーを記録する

******************************************************************************/

static void SetThreadError(struct TPHTTP *tpHTTP, char *def, char *func)
{
	tpHTTP->ErrCode = GetLastError();
	tpHTTP->ErrDef = def;
	tpHTTP->ErrFunc = func;
}


/******************************************************************************

	StoreHandle

	WinHTTPのハンドルを保存する (中止されている場合は閉じて FALSE を返す)

******************************************************************************/

static BOOL StoreHandle(struct TPHTTP *tpHTTP, void **field, HINTERNET hInternet)
{
	EnterCriticalSection(&tpHTTP->cs);
	if(tpHTTP->CancelFlag == TRUE){
		LeaveCriticalSection(&tpHTTP->cs);
		WinHttpCloseHandle(hInternet);
		return FALSE;
	}
	*field = hInternet;
	LeaveCriticalSection(&tpHTTP->cs);
	return TRUE;
}


/******************************************************************************

	GetRequestHandle

	WinHTTPのハンドルを取得する (中止されている場合は NULL)

******************************************************************************/

static HINTERNET GetRequestHandle(struct TPHTTP *tpHTTP, void **field)
{
	HINTERNET hInternet;

	EnterCriticalSection(&tpHTTP->cs);
	hInternet = (tpHTTP->CancelFlag == TRUE) ? NULL : *field;
	LeaveCriticalSection(&tpHTTP->cs);
	return hInternet;
}


/******************************************************************************

	PostProgress

	受信状況をメインスレッドに通知する

******************************************************************************/

static void PostProgress(struct TPHTTP *tpHTTP)
{
	DWORD tick = GetTickCount();

	if(tick - tpHTTP->ProgressTick < 200){
		return;
	}
	tpHTTP->ProgressTick = tick;
	InterlockedIncrement(&tpHTTP->RefCnt);
	if(PostMessage(hNotifyWnd, WM_HTTP_PROGRESS, (WPARAM)tpHTTP->Size, (LPARAM)tpHTTP) == FALSE){
		ReleaseHTTP(tpHTTP);
	}
}


/******************************************************************************

	HTTP_Request

	HTTPの送受信 (スレッドで実行、アイテムは参照しない)

******************************************************************************/

static BOOL HTTP_Request(struct TPHTTP *tpHTTP)
{
	HINTERNET hInternet;
	URL_COMPONENTS urlComponents;
	WCHAR szHostName[256];
	WCHAR *szUrlPath;
	WCHAR *header;
	WCHAR *wp;
	DWORD dwSize;
	DWORD dwFlags;
	DWORD len;
	long bufLen, bufCap;
	BOOL Secure;

	//セッションの開始
	if(tpHTTP->ProxyName != NULL){
		hInternet = WinHttpOpen(USER_AGENT, WINHTTP_ACCESS_TYPE_NAMED_PROXY, tpHTTP->ProxyName, WINHTTP_NO_PROXY_BYPASS, 0);
	}else{
		hInternet = WinHttpOpen(USER_AGENT, WINHTTP_ACCESS_TYPE_NO_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
	}
	if(hInternet == NULL){
		SetThreadError(tpHTTP, STR_ERROR_CONNECT, "WinHttpOpen");
		return FALSE;
	}
	if(StoreHandle(tpHTTP, &tpHTTP->hSession, hInternet) == FALSE){
		return FALSE;
	}
	if(tpHTTP->TimeOutMs > 0){
		WinHttpSetTimeouts(hInternet, tpHTTP->TimeOutMs, tpHTTP->TimeOutMs, tpHTTP->TimeOutMs, tpHTTP->TimeOutMs);
	}

	//URLの解析
	len = lstrlenW(tpHTTP->ReqURL) + 1;
	szUrlPath = (WCHAR *)HeapAlloc(GetProcessHeap(), 0, len * sizeof(WCHAR));
	if(szUrlPath == NULL){
		SetThreadError(tpHTTP, STR_ERROR_MEMORY, "HeapAlloc");
		return FALSE;
	}
	ZeroMemory(&urlComponents, sizeof(URL_COMPONENTS));
	urlComponents.dwStructSize = sizeof(URL_COMPONENTS);
	urlComponents.lpszHostName = szHostName;
	urlComponents.dwHostNameLength = sizeof(szHostName) / sizeof(WCHAR);
	urlComponents.lpszUrlPath = szUrlPath;
	urlComponents.dwUrlPathLength = len;
	if(!WinHttpCrackUrl(tpHTTP->ReqURL, len - 1, 0, &urlComponents)){
		SetThreadError(tpHTTP, STR_ERROR_CONNECT, "WinHttpCrackUrl");
		HeapFree(GetProcessHeap(), 0, szUrlPath);
		return FALSE;
	}
	for(wp = szUrlPath; *wp != L'\0'; wp++){
		if(*wp == L'#'){
			*wp = L'\0';
			break;
		}
	}
	Secure = (INTERNET_SCHEME_HTTPS == urlComponents.nScheme) ? TRUE : FALSE;

	//接続
	if((hInternet = GetRequestHandle(tpHTTP, &tpHTTP->hSession)) == NULL ||
		(hInternet = WinHttpConnect(hInternet, szHostName, urlComponents.nPort, 0)) == NULL){
		SetThreadError(tpHTTP, STR_ERROR_CONNECT, "WinHttpConnect");
		HeapFree(GetProcessHeap(), 0, szUrlPath);
		return FALSE;
	}
	if(StoreHandle(tpHTTP, &tpHTTP->hConnect, hInternet) == FALSE){
		HeapFree(GetProcessHeap(), 0, szUrlPath);
		return FALSE;
	}

	//リクエストの作成
	hInternet = WinHttpOpenRequest(hInternet,
		(tpHTTP->HeadRequest == TRUE) ? L"HEAD" : L"GET",
		szUrlPath, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
		(Secure == TRUE) ? WINHTTP_FLAG_SECURE : 0);
	HeapFree(GetProcessHeap(), 0, szUrlPath);
	if(hInternet == NULL){
		SetThreadError(tpHTTP, STR_ERROR_SEND, "WinHttpOpenRequest");
		return FALSE;
	}
	if(StoreHandle(tpHTTP, &tpHTTP->hRequest, hInternet) == FALSE){
		return FALSE;
	}

	//HTTPSでのプロキシ認証
	if(Secure == TRUE && tpHTTP->ProxyUser != NULL && tpHTTP->ProxyPass != NULL){
		WinHttpSetOption(hInternet, WINHTTP_OPTION_PROXY_USERNAME, tpHTTP->ProxyUser, lstrlenW(tpHTTP->ProxyUser));
		WinHttpSetOption(hInternet, WINHTTP_OPTION_PROXY_PASSWORD, tpHTTP->ProxyPass, lstrlenW(tpHTTP->ProxyPass));
	}
	//認証情報を送る場合は自動リダイレクトを行わない (リダイレクトはチェック処理側で行う)
	if(tpHTTP->NoRedirect == TRUE){
		dwFlags = WINHTTP_OPTION_REDIRECT_POLICY_NEVER;
		WinHttpSetOption(hInternet, WINHTTP_OPTION_REDIRECT_POLICY, &dwFlags, sizeof(dwFlags));
	}

	//送信
	if(!WinHttpSendRequest(hInternet, tpHTTP->SendHeader, -1, WINHTTP_NO_REQUEST_DATA, 0, WINHTTP_IGNORE_REQUEST_TOTAL_LENGTH, 0)){
		SetThreadError(tpHTTP, STR_ERROR_SEND, "WinHttpSendRequest");
		return FALSE;
	}
	if(WinHttpReceiveResponse(hInternet, NULL) == FALSE){
		SetThreadError(tpHTTP, STR_ERROR_RECV, "WinHttpReceiveResponse");
		return FALSE;
	}

	//ヘッダの取得
	dwSize = 0;
	if(WinHttpQueryHeaders(hInternet, WINHTTP_QUERY_RAW_HEADERS_CRLF, WINHTTP_HEADER_NAME_BY_INDEX, NULL, &dwSize, WINHTTP_NO_HEADER_INDEX) == FALSE &&
		GetLastError() != ERROR_INSUFFICIENT_BUFFER){
		SetThreadError(tpHTTP, STR_ERROR_RECV, "WinHttpQueryHeaders(Size)");
		return FALSE;
	}
	if(dwSize <= 12){
		SetThreadError(tpHTTP, STR_ERROR_RECV, "WinHttpQueryHeaders(dwSize)");
		return FALSE;
	}
	header = (WCHAR *)HeapAlloc(GetProcessHeap(), 0, dwSize + sizeof(WCHAR));
	if(header == NULL || WinHttpQueryHeaders(hInternet, WINHTTP_QUERY_RAW_HEADERS_CRLF, WINHTTP_HEADER_NAME_BY_INDEX, header, &dwSize, WINHTTP_NO_HEADER_INDEX) == FALSE){
		SetThreadError(tpHTTP, STR_ERROR_RECV, "WinHttpQueryHeaders");
		if(header != NULL) HeapFree(GetProcessHeap(), 0, header);
		return FALSE;
	}
	header[dwSize / sizeof(WCHAR)] = L'\0';
	int clen = WideCharToMultiByte(CP_ACP, 0, header, -1, NULL, 0, NULL, NULL);
	char *cbuf = (clen > 0) ? (char *)GlobalAlloc(GMEM_FIXED, (clen + 1) * sizeof(char)) : NULL;
	if(cbuf == NULL || WideCharToMultiByte(CP_ACP, 0, header, -1, cbuf, clen, NULL, NULL) == 0){
		SetThreadError(tpHTTP, STR_ERROR_RECV, "WideCharToMultiByte");
		if(cbuf != NULL) GlobalFree(cbuf);
		HeapFree(GetProcessHeap(), 0, header);
		return FALSE;
	}
	HeapFree(GetProcessHeap(), 0, header);

	EnterCriticalSection(&tpHTTP->cs);
	if(tpHTTP->CancelFlag == TRUE){
		LeaveCriticalSection(&tpHTTP->cs);
		GlobalFree(cbuf);
		return FALSE;
	}
	if(tpHTTP->buf != NULL) GlobalFree((HGLOBAL)tpHTTP->buf);
	tpHTTP->Size = clen;
	tpHTTP->buf = cbuf;
	bufLen = lstrlen(cbuf);
	bufCap = clen + 1;
	if(tpHTTP->HeadCheckFlag == FALSE && CheckHead(tpHTTP) == TRUE){
		LeaveCriticalSection(&tpHTTP->cs);
		return TRUE;
	}
	LeaveCriticalSection(&tpHTTP->cs);

	//本文の受信
	while(1){
		DWORD dwRead = 0;

		dwSize = 0;
		if((hInternet = GetRequestHandle(tpHTTP, &tpHTTP->hRequest)) == NULL){
			return FALSE;
		}
		if(!WinHttpQueryDataAvailable(hInternet, &dwSize)){
			SetThreadError(tpHTTP, STR_ERROR_RECV, "WinHttpQueryDataAvailable");
			return FALSE;
		}
		if(dwSize == 0){
			break;
		}
		//最大受信サイズを超える場合は受信を打ち切る
		if(dwSize > MAX_RECVSIZE || tpHTTP->Size > MAX_RECVSIZE - (long)dwSize){
			break;
		}

		BYTE *pszBuffer = (LPBYTE)HeapAlloc(GetProcessHeap(), 0, dwSize + 1);
		if(pszBuffer == NULL){
			break;
		}
		if(!WinHttpReadData(hInternet, pszBuffer, dwSize, &dwRead)){
			SetThreadError(tpHTTP, STR_ERROR_RECV, "WinHttpReadData");
			HeapFree(GetProcessHeap(), 0, pszBuffer);
			return FALSE;
		}
		if(dwRead == 0){
			HeapFree(GetProcessHeap(), 0, pszBuffer);
			break;
		}
		pszBuffer[dwRead] = '\0';

		EnterCriticalSection(&tpHTTP->cs);
		if(tpHTTP->CancelFlag == TRUE){
			LeaveCriticalSection(&tpHTTP->cs);
			HeapFree(GetProcessHeap(), 0, pszBuffer);
			return FALSE;
		}
		tpHTTP->Size += dwRead;

		//受信データを追加 (NULを含む場合はNULまでを追加する)
		//バッファは受信バイト数 (Size) + 1 以上を確保する
		int addLen = lstrlen((char *)pszBuffer);
		if(tpHTTP->Size + 1 > bufCap){
			long newCap = bufCap * 2;
			if(newCap > MAX_RECVSIZE + 1){
				newCap = MAX_RECVSIZE + 1;
			}
			if(newCap < tpHTTP->Size + 1){
				newCap = tpHTTP->Size + 1;
			}
			char *wkbuf = (char *)GlobalAlloc(GMEM_FIXED, newCap);
			if(wkbuf == NULL){
				tpHTTP->Size -= dwRead;
				LeaveCriticalSection(&tpHTTP->cs);
				HeapFree(GetProcessHeap(), 0, pszBuffer);
				break;
			}
			CopyMemory(wkbuf, tpHTTP->buf, bufLen + 1);
			GlobalFree((HGLOBAL)tpHTTP->buf);
			tpHTTP->buf = wkbuf;
			bufCap = newCap;
		}
		CopyMemory(tpHTTP->buf + bufLen, pszBuffer, addLen + 1);
		bufLen += addLen;
		HeapFree(GetProcessHeap(), 0, pszBuffer);
		if(tpHTTP->HeadCheckFlag == FALSE && CheckHead(tpHTTP) == TRUE){
			LeaveCriticalSection(&tpHTTP->cs);
			break;
		}
		LeaveCriticalSection(&tpHTTP->cs);

		PostProgress(tpHTTP);
	}
	return TRUE;
}


/******************************************************************************

	HTTP_Thread

	HTTP処理スレッド

******************************************************************************/

static DWORD WINAPI HTTP_Thread(LPVOID lpParam)
{
	struct TPHTTP *tpHTTP = (struct TPHTTP *)lpParam;
	BOOL CancelFlag;

	SetLastError(NO_ERROR);
	if(HTTP_Request(tpHTTP) == FALSE && tpHTTP->ErrDef == NULL){
		//中止された場合
		SetThreadError(tpHTTP, STR_ERROR_RECV, "Cancel");
	}

	EnterCriticalSection(&tpHTTP->cs);
	CloseRequestHandles(tpHTTP);
	CancelFlag = tpHTTP->CancelFlag;
	LeaveCriticalSection(&tpHTTP->cs);

	//実行中のリストから削除
	EnterCriticalSection(&ListCs);
	if(tpHTTP->Prev != NULL){
		tpHTTP->Prev->Next = tpHTTP->Next;
	}else{
		ActiveList = tpHTTP->Next;
	}
	if(tpHTTP->Next != NULL){
		tpHTTP->Next->Prev = tpHTTP->Prev;
	}
	tpHTTP->Next = tpHTTP->Prev = NULL;
	LeaveCriticalSection(&ListCs);

	//結果の処理はメインスレッドで行う (スレッドの参照はメッセージに引き継ぐ)
	if(CancelFlag == TRUE || PostMessage(hNotifyWnd, WM_HTTP_DONE, 0, (LPARAM)tpHTTP) == FALSE){
		ReleaseHTTP(tpHTTP);
	}
	InterlockedDecrement(&ThreadCnt);
	return 0;
}


/******************************************************************************

	HTTP_Complete

	スレッドの処理結果をアイテムに設定する (メインスレッドで実行)

******************************************************************************/

static void HTTP_Complete(struct TPHTTP *tpHTTP)
{
	struct TPITEM *tpItemInfo = tpHTTP->tpItemInfo;
	HWND hWnd = tpHTTP->hWnd;
	int ret;

	if(tpHTTP->ErrDef != NULL){
		SetHttpError(tpItemInfo, tpHTTP->ErrDef, tpHTTP->ErrFunc, tpHTTP->ErrCode);
		SendMessage(hWnd, WM_CHECK_END, CHECK_ERROR, (LPARAM)tpItemInfo);
		SendMessage(hWnd, WM_CHECK_RESULT, ST_ERROR, (LPARAM)tpItemInfo);
		HTTP_FreeItem(tpItemInfo);
		return;
	}
	ret = HeaderFunc(hWnd, tpItemInfo);
	if(ret == CHECK_NO){
		//再チェックする場合は移動先のURL (Param2) を残す
		tpItemInfo->Param1 = 0;
		tpItemInfo->Param3 = 0;
		CancelHTTP(tpHTTP);
		ReleaseHTTP(tpHTTP);
		SendMessage(hWnd, WM_CHECK_END, ret, (LPARAM)tpItemInfo);
		return;
	}
	SendMessage(hWnd, WM_CHECK_END, ret, (LPARAM)tpItemInfo);
	SetSBText(hWnd, tpItemInfo, STR_STATUS_CHECKEND);
	HTTP_FreeItem(tpItemInfo);
}


/******************************************************************************

	NotifyWndProc

	スレッドからの通知を受け取るウィンドウプロシージャ

******************************************************************************/

static LRESULT CALLBACK NotifyWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	struct TPHTTP *tpHTTP;
	char buf[BUFSIZE];

	switch(uMsg)
	{
	case WM_HTTP_DONE:
		tpHTTP = (struct TPHTTP *)lParam;
		//中止 (アイテムから切り離し) されていない場合のみ結果を処理する
		if(tpHTTP->CancelFlag == FALSE && tpHTTP->tpItemInfo != NULL &&
			(struct TPHTTP *)tpHTTP->tpItemInfo->Param1 == tpHTTP){
			HTTP_Complete(tpHTTP);
		}
		ReleaseHTTP(tpHTTP);
		return 0;

	case WM_HTTP_PROGRESS:
		tpHTTP = (struct TPHTTP *)lParam;
		if(tpHTTP->CancelFlag == FALSE && tpHTTP->tpItemInfo != NULL){
			wsprintf(buf, STR_STATUS_RECV, (long)wParam);
			SetSBText(tpHTTP->hWnd, tpHTTP->tpItemInfo, buf);
		}
		ReleaseHTTP(tpHTTP);
		return 0;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


/******************************************************************************

	Start_HTTP_Thread

	HTTPスレッドの開始

******************************************************************************/

static BOOL Start_HTTP_Thread(struct TPHTTP *tpHTTP)
{
	WNDCLASS wc;
	DWORD threadId;

	if(ListCsInit == FALSE){
		InitializeCriticalSection(&ListCs);
		ListCsInit = TRUE;
	}

	//スレッドからの通知を受け取るウィンドウの作成 (メインスレッドで作成する)
	if(hNotifyWnd == NULL){
		ZeroMemory(&wc, sizeof(WNDCLASS));
		wc.lpfnWndProc = NotifyWndProc;
		wc.hInstance = ghinst;
		wc.lpszClassName = NOTIFY_WINDOW_CLASS;
		RegisterClass(&wc);
		hNotifyWnd = CreateWindow(NOTIFY_WINDOW_CLASS, "", 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, ghinst, NULL);
		if(hNotifyWnd == NULL){
			return FALSE;
		}
	}

	//スレッドの参照
	InterlockedIncrement(&tpHTTP->RefCnt);
	InterlockedIncrement(&ThreadCnt);

	EnterCriticalSection(&ListCs);
	tpHTTP->Prev = NULL;
	tpHTTP->Next = ActiveList;
	if(ActiveList != NULL){
		ActiveList->Prev = tpHTTP;
	}
	ActiveList = tpHTTP;
	LeaveCriticalSection(&ListCs);

	tpHTTP->hThread = CreateThread(NULL, 0, HTTP_Thread, (LPVOID)tpHTTP, 0, &threadId);
	if(tpHTTP->hThread == NULL){
		EnterCriticalSection(&ListCs);
		if(tpHTTP->Prev != NULL){
			tpHTTP->Prev->Next = tpHTTP->Next;
		}else{
			ActiveList = tpHTTP->Next;
		}
		if(tpHTTP->Next != NULL){
			tpHTTP->Next->Prev = tpHTTP->Prev;
		}
		tpHTTP->Next = tpHTTP->Prev = NULL;
		LeaveCriticalSection(&ListCs);
		InterlockedDecrement(&ThreadCnt);
		InterlockedDecrement(&tpHTTP->RefCnt);
		return FALSE;
	}
	return TRUE;
}


/******************************************************************************

	HTTP_Start

	チェックの開始

******************************************************************************/

__declspec(dllexport) int CALLBACK HTTP_Start(HWND hWnd, struct TPITEM *tpItemInfo)
{
	struct TPHTTP *tpHTTP;

	tpHTTP = AllocHTTP(hWnd, tpItemInfo);
	if(tpHTTP == NULL){
		return CHECK_ERROR;
	}
	tpItemInfo->Param1 = (long)tpHTTP;

	//タイマーの初期化
	tpItemInfo->user1 = 0;

	//チェック以外の場合に表示するURLの情報を使用する
	if(tpItemInfo->Param2 == 0){
		if(tpItemInfo->Param3 == 3 && tpItemInfo->ViewURL != NULL && *tpItemInfo->ViewURL != '\0'){
			if((tpItemInfo->Param2 = (long)GlobalAlloc(GPTR, lstrlen(tpItemInfo->ViewURL) + 1)) == 0){
				HTTP_FreeItem(tpItemInfo);
				return CHECK_ERROR;
			}
			lstrcpy((char*)tpItemInfo->Param2, tpItemInfo->ViewURL);
		}else{
			if((tpItemInfo->Param2 = (long)GlobalAlloc(GPTR, lstrlen(tpItemInfo->CheckURL) + 1)) == 0){
				HTTP_FreeItem(tpItemInfo);
				return CHECK_ERROR;
			}
			lstrcpy((char*)tpItemInfo->Param2, tpItemInfo->CheckURL);
		}
	}

	//リクエストメソッドの選択
	switch(tpItemInfo->Param3)
	{
	case 0:		//チェック
	case 1:		//ヘッダ表示
		tpItemInfo->user2 = GetOptionInt(tpItemInfo->Option1, OP1_REQTYPE);
		if(CheckType == 1 || tpItemInfo->user2 == 2 ||
			GetOptionInt(tpItemInfo->Option1, OP1_META) == 1 ||
			GetOptionInt(tpItemInfo->Option1, OP1_MD5) == 1 ||
			GetOptionInt(tpItemInfo->Option1, OP1_NOTAGSIZE) == 1){
			tpItemInfo->user2 = REQUEST_GET;
		}
		if(tpItemInfo->user2 == REQUEST_GET){
			//フィルタ
			tpHTTP->FilterFlag = FilterMatch(tpItemInfo->CheckURL);
		}
		break;

	case 2:		//ソース表示
	case 3:		//タイトル取得
		tpItemInfo->user2 = REQUEST_GET;
		tpHTTP->FilterFlag = (GetAsyncKeyState(VK_SHIFT) < 0) ? TRUE : FALSE;
		break;
	}

	//接続サーバとポート番号を取得
	tpHTTP->Port = 0;
	if(GetServerPort(hWnd, tpItemInfo, tpHTTP) == FALSE){
		HTTP_FreeItem(tpItemInfo);
		return CHECK_ERROR;
	}

	/* 既に同じサーバへチェックを行っている場合はチェックをしない */
	if(CheckServer(hWnd, tpHTTP->hSvName) == TRUE){
		tpItemInfo->Param1 = 0;
		ReleaseHTTP(tpHTTP);
		return CHECK_NO;
	}

	//リクエストの作成 (スレッドからアイテムを参照しないよう事前に作成する)
	if(BuildRequest(tpItemInfo, tpHTTP) == FALSE){
		SetErrorString(tpItemInfo, STR_ERROR_MEMORY, FALSE);
		HTTP_FreeItem(tpItemInfo);
		return CHECK_ERROR;
	}

	SetSBText(hWnd, tpItemInfo, STR_STATUS_CONNECT);

	if(Start_HTTP_Thread(tpHTTP) == FALSE){
		SetHttpError(tpItemInfo, STR_ERROR_CONNECT, "CreateThread", GetLastError());
		HTTP_FreeItem(tpItemInfo);
		return CHECK_ERROR;
	}
	return CHECK_SUCCEED;
}


/******************************************************************************

	HTTP_ItemCheckEnd

	アイテムのチェック終了通知

******************************************************************************/

__declspec(dllexport) int CALLBACK HTTP_ItemCheckEnd(HWND hWnd, struct TPITEM *tpItemInfo)
{
	HWND vWnd;

	struct TPHTTP *tpHTTP = (struct TPHTTP *)tpItemInfo->Param1;
	if (tpHTTP != NULL) {
		switch (tpItemInfo->Param3) {
		case 1:
		case 2:
			if (tpItemInfo->ErrStatus != NULL) {
				MessageBox(tpHTTP->hWnd, tpItemInfo->ErrStatus, tpItemInfo->CheckURL, MB_ICONERROR);
				break;
			}
			//ヘッダ、ソース表示
			vWnd = CreateDialogParam(ghinst, MAKEINTRESOURCE(IDD_DIALOG_VIEW), NULL, ViewProc, (long)tpItemInfo);
			ShowWindow(vWnd, SW_SHOW);
			SetFocus(GetDlgItem(vWnd, IDC_EDIT_VIEW));
			break;
		}
	}
	return 0;
}


/******************************************************************************

	HTTP_GetItemText

	クリップボード用アイテムのテキストの設定

******************************************************************************/

__declspec(dllexport) HANDLE CALLBACK HTTP_GetItemText(struct TPITEM *tpItemInfo)
{
	HANDLE hMemText;
	char *buf;
	char *URL = NULL;

	if(tpItemInfo->ViewURL != NULL && *tpItemInfo->ViewURL != '\0'){
		URL = tpItemInfo->ViewURL;
	}else if(tpItemInfo->CheckURL != NULL && *tpItemInfo->CheckURL != '\0'){
		URL = tpItemInfo->CheckURL;
	}

	if(URL == NULL){
		return NULL;
	}

	if((hMemText = GlobalAlloc(GHND, lstrlen(URL) + 1)) == NULL){
		return NULL;
	}
	if((buf = GlobalLock(hMemText)) == NULL){
		GlobalFree(hMemText);
		return NULL;
	}
	lstrcpy(buf, URL);
	GlobalUnlock(hMemText);
	return hMemText;
}


/******************************************************************************

	WriteInternetShortcut

	インターネットショートカットの作成

******************************************************************************/

static BOOL WriteInternetShortcut(char *URL, char *path)
{
#define IS_STR		"[InternetShortcut]\r\nURL="
	HANDLE hFile;
	char *buf, *p;
	DWORD ret;
	int len;

	len = lstrlen(IS_STR) + lstrlen(URL) + 2;
	p = buf = (char *)_alloca(len + 1);
	if(buf == NULL){
		return FALSE;
	}
	p = iStrCpy(p, IS_STR);
	p = iStrCpy(p, URL);
	p = iStrCpy(p, "\r\n");

	/* ファイルを開く */
	hFile = CreateFile(path, GENERIC_READ | GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == NULL || hFile == (HANDLE)-1){
		return FALSE;
	}
	/* ファイルに書き込む */
	if(WriteFile(hFile, buf, len, &ret, NULL) == FALSE){
		CloseHandle(hFile);
		return FALSE;
	}
	/* ファイルを閉じる */
	CloseHandle(hFile);
	return TRUE;
}


/******************************************************************************

	HTTP_CreateDropItem

	アイテムのドロップファイルの設定

******************************************************************************/

__declspec(dllexport) BOOL CALLBACK HTTP_CreateDropItem(struct TPITEM *tpItemInfo, char *fPath, char *iPath, char *ret)
{
	char buf[BUFSIZE];
	char ItemName[BUFSIZE];
	char *URL;
	char *p;
	BOOL rc;

	if(tpItemInfo == NULL){
		return FALSE;
	}

	//パスがバッファに収まらない場合は作成しない
	if(lstrlen(fPath) + 1 + ((iPath != NULL) ? lstrlen(iPath) : 0) >= BUFSIZE){
		return FALSE;
	}
	if(iPath != NULL){
		p = iStrCpy(buf, fPath);
		p = iStrCpy(p, "\\");
		p = iStrCpy(p, iPath);
	}else{
		lstrcpy(buf, fPath);
	}

	URL = NULL;

	if(tpItemInfo->Title == NULL || BUFSIZE - lstrlen(buf) - 6 <= 1){
		return FALSE;
	}
	lstrcpyn(ItemName, tpItemInfo->Title, BUFSIZE - lstrlen(buf) - 6);
	BadNameCheck(ItemName, '-');

	p = iStrCpy(ret, buf);
	p = iStrCpy(p, "\\");
	p = iStrCpy(p, ItemName);
	p = iStrCpy(p, ".url");

	if(tpItemInfo->ViewURL != NULL && *tpItemInfo->ViewURL != '\0'){
		URL = tpItemInfo->ViewURL;
	}else{
		URL = tpItemInfo->CheckURL;
	}

	if(URL == NULL){
		return FALSE;
	}

	rc = WriteInternetShortcut(URL, ret);
	return rc;
}


/******************************************************************************

	HTTP_DropFile

	ドロップされたファイルに対する処理

******************************************************************************/

__declspec(dllexport) int CALLBACK HTTP_DropFile(HWND hWnd, char *FileType, char *FilePath, struct TPITEM *tpItemInfo)
{
	char URL[MAXSIZE];
	char *p, *r;

	if(lstrcmpi(FileType,"url") == 0){
		GetPrivateProfileString("InternetShortcut", "URL", "", URL, MAXSIZE - 1, FilePath);

		for(r = p = FilePath; *p != '\0'; p++){
			if(IS_LEAD_BYTE(p)){
				p++;
			}else if(*p == '\\' || *p == '/'){
				r = p + 1;
			}
		}
		if(tpItemInfo->Title != NULL){
			GlobalFree(tpItemInfo->Title);
		}
		tpItemInfo->Title = GlobalAlloc(GPTR, lstrlen(r) + 1);
		if(tpItemInfo->Title != NULL){
			lstrcpy(tpItemInfo->Title, r);
		}
		if(lstrlen(tpItemInfo->Title) > 4){
			if(lstrcmpi((tpItemInfo->Title + lstrlen(tpItemInfo->Title) - 4), ".url") == 0){
				*(tpItemInfo->Title + lstrlen(tpItemInfo->Title) - 4) = '\0';
			}
		}

		if(tpItemInfo->CheckURL != NULL){
			GlobalFree(tpItemInfo->CheckURL);
		}
		tpItemInfo->CheckURL = GlobalAlloc(GPTR, lstrlen(URL) + 1);
		if(tpItemInfo->CheckURL != NULL){
			lstrcpy(tpItemInfo->CheckURL, URL);
		}
		return DROPFILE_NEWITEM;
	}
	return DROPFILE_NONE;
}


/******************************************************************************

	HTTP_ExecItem

	アイテムの実行

******************************************************************************/

__declspec(dllexport) int CALLBACK HTTP_ExecItem(HWND hWnd, char *Action, struct TPITEM *tpItemInfo)
{
	char *p = NULL;
	int rc = -1;

	if(lstrcmp(Action, "open") == 0){
		//開く
		if(tpItemInfo->ViewURL != NULL){
			p = tpItemInfo->ViewURL;
		}
		if(p == NULL || *p == '\0' && tpItemInfo->CheckURL != NULL){
			p = tpItemInfo->CheckURL;
		}
		if(p == NULL || *p == '\0'){
			MessageBox(hWnd, STR_ERR_MSG_URLOPEN, STR_ERR_TITLE_URLOPEN, MB_ICONEXCLAMATION);
			return -1;
		}
		//http, https 以外は確認してから実行する
		if(ConfirmOpenURL(hWnd, p) == FALSE){
			return -1;
		}
		rc = ExecItem(hWnd, p, NULL);

	}else if(lstrcmp(Action, "checkopen") == 0){
		//チェックするURLで開く
		if(tpItemInfo->CheckURL != NULL){
			p = tpItemInfo->CheckURL;
		}
		if(p == NULL || *p == '\0'){
			MessageBox(hWnd, STR_ERR_MSG_URLOPEN, STR_ERR_TITLE_URLOPEN, MB_ICONEXCLAMATION);
			return -1;
		}
		//http, https 以外は確認してから実行する
		if(ConfirmOpenURL(hWnd, p) == FALSE){
			return -1;
		}
		rc = ExecItem(hWnd, p, NULL);

	}else if(lstrcmp(Action, "header") == 0){
		//ヘッダ表示
		tpItemInfo->Param3 = 1;
		SendMessage(hWnd, WM_ITEMCHECK, 0, (LPARAM)tpItemInfo);

	}else if(lstrcmp(Action, "source") == 0){
		//ソース表示
		tpItemInfo->Param3 = 2;
		SendMessage(hWnd, WM_ITEMCHECK, 0, (LPARAM)tpItemInfo);

	}else if(lstrcmp(Action, "gettitle") == 0){
		//タイトル取得
		tpItemInfo->Param3 = 3;
		SendMessage(hWnd, WM_ITEMCHECK, 0, (LPARAM)tpItemInfo);
	}
	// 1 を返すとアイコンを初期化する。
	return rc;
}


/******************************************************************************

	HTTP_GetInfo

	プロトコル情報

******************************************************************************/

__declspec(dllexport) int CALLBACK HTTP_GetInfo(struct TPPROTOCOLINFO *tpInfo)
{
	int i = 0;

#ifdef THGHN
	CreateEvent(NULL, TRUE, TRUE, "GHBN_EVENT");
#endif

	lstrcpy(tpInfo->Scheme, "http://\thttps://");
	lstrcpy(tpInfo->NewMenu, STR_GETINFO_HTTP_NEWMENU);
	lstrcpy(tpInfo->FileType, "url");	//	複数ある場合は \t で区切る 例) "txt\tdoc\tdat"

	tpInfo->tpMenu = (struct TPPROTOCOLMENU *)GlobalAlloc(GPTR, sizeof(struct TPPROTOCOLMENU) * HTTP_MENU_CNT);
	tpInfo->tpMenuCnt = HTTP_MENU_CNT;

	lstrcpy(tpInfo->tpMenu[i].Name, STR_GETINFO_HTTP_OPEN);
	lstrcpy(tpInfo->tpMenu[i].Action, "open");
	tpInfo->tpMenu[i].Default = TRUE;
	tpInfo->tpMenu[i].Flag = 0;

	i++;
	lstrcpy(tpInfo->tpMenu[i].Name, STR_GETINFO_HTTP_CHECKOPEN);
	lstrcpy(tpInfo->tpMenu[i].Action, "checkopen");
	tpInfo->tpMenu[i].Default = FALSE;
	tpInfo->tpMenu[i].Flag = 0;

	i++;
	lstrcpy(tpInfo->tpMenu[i].Name, "-");
	lstrcpy(tpInfo->tpMenu[i].Action, "");
	tpInfo->tpMenu[i].Default = FALSE;
	tpInfo->tpMenu[i].Flag = 0;

	i++;
	lstrcpy(tpInfo->tpMenu[i].Name, STR_GETINFO_HTTP_HEADER);
	lstrcpy(tpInfo->tpMenu[i].Action, "header");
	tpInfo->tpMenu[i].Default = FALSE;
	tpInfo->tpMenu[i].Flag = 1;

	i++;
	lstrcpy(tpInfo->tpMenu[i].Name, STR_GETINFO_HTTP_SOURCE);
	lstrcpy(tpInfo->tpMenu[i].Action, "source");
	tpInfo->tpMenu[i].Default = FALSE;
	tpInfo->tpMenu[i].Flag = 1;

	i++;
	lstrcpy(tpInfo->tpMenu[i].Name, STR_GETINFO_HTTP_GETTITLE);
	lstrcpy(tpInfo->tpMenu[i].Action, "gettitle");
	tpInfo->tpMenu[i].Default = FALSE;
	tpInfo->tpMenu[i].Flag = 1;
	return 0;
}


/******************************************************************************

	HTTP_EndNotify

	WWWCの終了の通知

******************************************************************************/

__declspec(dllexport) int CALLBACK HTTP_EndNotify(void)
{
#ifdef THGHN
	HANDLE hEvent;
#endif
	struct TPHTTP *tpHTTP;
	int i;

	for(i = 0; i < 100; i++){
		if(hWndList[i] != NULL){
			SendMessage(hWndList[i], WM_CLOSE, 0, 0);
		}
	}

	//実行中の受信を中止してスレッドの終了を待つ
	if(ListCsInit == TRUE){
		EnterCriticalSection(&ListCs);
		for(tpHTTP = ActiveList; tpHTTP != NULL; tpHTTP = tpHTTP->Next){
			EnterCriticalSection(&tpHTTP->cs);
			tpHTTP->CancelFlag = TRUE;
			CloseRequestHandles(tpHTTP);
			LeaveCriticalSection(&tpHTTP->cs);
		}
		LeaveCriticalSection(&ListCs);
		for(i = 0; i < THREAD_END_WAIT / 50 && ThreadCnt > 0; i++){
			Sleep(50);
		}
	}
	if(hNotifyWnd != NULL){
		DestroyWindow(hNotifyWnd);
		hNotifyWnd = NULL;
		UnregisterClass(NOTIFY_WINDOW_CLASS, ghinst);
	}

#ifdef THGHN
	hEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, "GHBN_EVENT");
	if(hEvent != NULL){
		CloseHandle(hEvent);
	}
#endif

	if(tpHostInfo != NULL){
		for(i = 0; i < HostInfoCnt; i++){
			if((tpHostInfo + i)->HostName != NULL){
				LocalFree((tpHostInfo + i)->HostName);
			}
		}
		LocalFree(tpHostInfo);
		tpHostInfo = NULL;
	}

	FreeFilter();
	return 0;
}
/* End of source */
