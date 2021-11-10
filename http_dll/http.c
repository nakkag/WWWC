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


/**************************************************************************
	Global Variables
**************************************************************************/

static int srcLen;

static struct TPHOSTINFO *tpHostInfo;
static int HostInfoCnt;

HWND hWndList[100];


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
static char *CreateOldData(char *buf);
static int GetMetaString(struct TPITEM *tpItemInfo);
static int Head_GetDate(struct TPITEM *tpItemInfo, int CmpOption, BOOL *DateRet);
static int Head_GetSize(struct TPITEM *tpItemInfo, int CmpOption, int SetDate, BOOL *SizeRet);
static int Get_GetSize(struct TPITEM *tpItemInfo, int CmpOption, int SetDate);
static int Get_MD5Check(struct TPITEM *tpItemInfo, int SetDate);
static int HeaderFunc(HWND hWnd, struct TPITEM *tpItemInfo);
static void SetSBText(HWND hWnd, struct TPITEM *tpItemInfo, char *msg);
static BOOL CheckHead(struct TPITEM *tpItemInfo, struct TPHTTP *tpHTTP);
static void FreeURLData(struct TPHTTP *tpHTTP);
static BOOL GetServerPort(HWND hWnd, struct TPITEM *tpItemInfo, struct TPHTTP *tpHTTP);
static BOOL CheckServer(HWND hWnd, char *RealServer);

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
		ret = NULL;
	}
	//特殊文字の変換
	ConvHTMLChar(ret);
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
		for(; *r != ' '; r++);
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
}



/******************************************************************************

	SetHttpError

	HTTPエラーを設定

******************************************************************************/

static void SetHttpError(struct TPITEM *tpItemInfo, char *def, char *func)
{
	DWORD err;
	char *msg = NULL;
	char *buf;

	err = GetLastError();
	if (err == NO_ERROR) {
		buf = (char *)HeapAlloc(GetProcessHeap(), 0, lstrlen(def) + 2 + lstrlen(func) + 1);
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
		wsprintf(buf, "%s (%u): %s", msg, err, func);
		SetErrorString(tpItemInfo, buf, FALSE);
		HeapFree(GetProcessHeap(), 0, buf);
		LocalFree(msg);
	} else {
		buf = (char *)HeapAlloc(GetProcessHeap(), 0, lstrlen(def) + 2 + 10 + 3 + lstrlen(func) + 1);
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
	headcontent = (char *)HeapAlloc(GetProcessHeap(), 0, len + 2);
	if(headcontent == NULL){
		return ST_DEFAULT;
	}
	*DateRet = GetHeadContent(tpHTTP->buf, "Last-Modified:", headcontent);
	if(*DateRet == TRUE){
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
			if(DateConv(headcontent, chtime) == 0){
				lstrcpy(headcontent, chtime);
			}
			/* アイテムに今回のチェック内容をセットする */
			if(tpItemInfo->Date != NULL){
				GlobalFree(tpItemInfo->Date);
			}
			tpItemInfo->Date = (char *)GlobalAlloc(GMEM_FIXED, sizeof(char) * lstrlen(headcontent) + ((CmpMsg == ST_UP) ? 2 : 1));
			if(tpItemInfo->Date != NULL){
				lstrcpy(tpItemInfo->Date, headcontent);
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
	headcontent = (char *)HeapAlloc(GetProcessHeap(), 0, len + 2);
	if(headcontent == NULL){
		return ST_DEFAULT;
	}
	*SizeRet = GetHeadContent(tpHTTP->buf, "Content-Length:", headcontent);
	if(*SizeRet == TRUE){
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
	for(r = tmp; *p != ' ' && *p != '\0'; p++, r++){
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
		if(Status == 301 || Status == 302){
			break;
		}
		//チェックの終了をWWWCに通知
		SendMessage(hWnd, WM_CHECK_RESULT, ST_DEFAULT, (LPARAM)tpItemInfo);
		return CHECK_END;

	case 3:
		if(Status == 301 || Status == 302){
			break;
		}
		//タイトル取得
		if(Status == 200){
			if((titlebuf = GetTitle(tpHTTP->buf + HeaderSize(tpHTTP->buf))) != NULL){
				if(tpItemInfo->Title != NULL){
					GlobalFree(tpItemInfo->Title);
				}
				tpItemInfo->Title = GlobalAlloc(GPTR, lstrlen(titlebuf) + 1);
				lstrcpy(tpItemInfo->Title, titlebuf);
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
		if(tpItemInfo->Param4 > 10){
			tpItemInfo->Param4 = 0;
			SetErrorString(tpItemInfo, STR_ERROR_MOVE, FALSE);
			SendMessage(hWnd, WM_CHECK_RESULT, ST_ERROR, (LPARAM)tpItemInfo);
			break;
		}
		len = GetHeadContentSize(tpHTTP->buf, "Location:");
		headcontent = (char *)GlobalAlloc(GPTR, len + 1);
		if(headcontent != NULL && GetHeadContent(tpHTTP->buf, "Location:", headcontent) == TRUE){
			tpItemInfo->Param4++;
			if(tpItemInfo->Param2 != 0){
				GlobalFree((HGLOBAL)tpItemInfo->Param2);
			}
			tpItemInfo->Param2 = (long)GlobalAlloc(GPTR, lstrlen(headcontent) + 1);
			if(tpItemInfo->Param2 != 0) lstrcpy((char*)tpItemInfo->Param2, headcontent);

			//チェックするURLを変更
			if(tpItemInfo->CheckURL != NULL){
				GlobalFree((HGLOBAL)tpItemInfo->CheckURL);
			}
			tpItemInfo->CheckURL = (char *)GlobalAlloc(GPTR, lstrlen(headcontent) + 1);
			if(tpItemInfo->CheckURL != NULL) lstrcpy(tpItemInfo->CheckURL, headcontent);
			GlobalFree(headcontent);

			//リクエストの初期化
			return CHECK_NO;
		}else{
			if(headcontent != NULL) GlobalFree(headcontent);
			SetErrorString(tpItemInfo, tpHTTP->buf, TRUE);
			SendMessage(hWnd, WM_CHECK_RESULT, ST_ERROR, (LPARAM)tpItemInfo);
		}
		break;

	case 302:	/* 移動の場合 */
	case 307:
		if(tpItemInfo->Param4 > 10){
			tpItemInfo->Param4 = 0;
			SetErrorString(tpItemInfo, STR_ERROR_MOVE, FALSE);
			SendMessage(hWnd, WM_CHECK_RESULT, ST_ERROR, (LPARAM)tpItemInfo);
			break;
		}
		len = GetHeadContentSize(tpHTTP->buf, "Location:");
		headcontent = (char *)GlobalAlloc(GPTR, len + 1);
		if(headcontent != NULL && GetHeadContent(tpHTTP->buf, "Location:", headcontent) == TRUE){
			tpItemInfo->Param4++;
			if(tpItemInfo->Param2 != 0){
				GlobalFree((HGLOBAL)tpItemInfo->Param2);
			}
			tpItemInfo->Param2 = (long)CreateMoveURL(tpHTTP->Path, headcontent);
			GlobalFree(headcontent);

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

static BOOL CheckHead(struct TPITEM *tpItemInfo, struct TPHTTP *tpHTTP)
{
	char *p;

	//すべて受信する必要がある場合
	if(tpItemInfo->Param3 >= 2 || tpHTTP->FilterFlag == TRUE ||
		GetOptionInt(tpItemInfo->Option1, OP1_META) == 1 ||
		GetOptionInt(tpItemInfo->Option1, OP1_MD5) == 1 ||
		GetOptionInt(tpItemInfo->Option1, OP1_NOTAGSIZE) == 1){
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
	if(tpItemInfo->Param3 == 1){
		return TRUE;
	}

	//ヘッダにサイズが設定されているかチェック
	if(tpItemInfo->Param3 == 0 && GetHeadContentSize(tpHTTP->buf, "Content-Length:") <= 0){
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
	if(tpItemInfo->Param1 != 0){
		if((HGLOBAL)((struct TPHTTP *)tpItemInfo->Param1)->buf != NULL){
			GlobalFree((HGLOBAL)((struct TPHTTP *)tpItemInfo->Param1)->buf);
		}
		FreeURLData((struct TPHTTP *)tpItemInfo->Param1);
		GlobalFree((HGLOBAL)tpItemInfo->Param1);
		tpItemInfo->Param1 = 0;
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
	switch(tpItemInfo->Param3){
	case 1:
	case 2:
		//ヘッダ、ソース表示
		vWnd = CreateDialogParam(ghinst, MAKEINTRESOURCE(IDD_DIALOG_VIEW), NULL, ViewProc, (long)tpItemInfo);
		ShowWindow(vWnd, SW_SHOW);
		break;

	case 3:
		//タイトル取得
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
		break;
	}
	if (tpHTTP->hThread != NULL) {
		TerminateThread(tpHTTP->hThread, 0);
		tpHTTP->hThread = NULL;
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
	struct TPHTTP *tpHTTP;

	if(tpItemInfo->user1 == -1){
		return CHECK_SUCCEED;
	}
	tpItemInfo->user1++;
	if(tpItemInfo->user1 == TimeOut){
		if(tpItemInfo->Param3 == 1 || tpItemInfo->Param3 == 2){
			tpItemInfo->user1 = -1;
			MessageBox(hWnd, STR_ERROR_TIMEOUT, tpItemInfo->CheckURL, MB_ICONERROR);
		}
		tpHTTP = (struct TPHTTP *)tpItemInfo->Param1;
		if (tpHTTP != NULL && tpHTTP->hThread != NULL) {
			TerminateThread(tpHTTP->hThread, 0);
			tpHTTP->hThread = NULL;
		}
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

	HTTP_Thread

	HTTP処理スレッド

******************************************************************************/

static unsigned int CALLBACK HTTP_Thread(struct TPITEM *tpItemInfo)
{
	struct TPHTTP *tpHTTP;
	HINTERNET hSession, hConnect, hRequest;
	URL_COMPONENTS urlComponents;
	WCHAR szHostName[256], szUrlPath[2048];
	WCHAR *wp;
	DWORD dwSize;
	WCHAR *header;
	WCHAR url[2048];
	WCHAR *sendHeader;
	WCHAR *s;
	char BaseStr1[BUFSIZE];
	char BaseStr2[BUFSIZE];
	char user[BUFSIZE];
	char pass[BUFSIZE];
	char referer[BUFSIZE];
	char buf[BUFSIZE];
	int ProxyFlag;

	SetLastError(NO_ERROR);

	tpHTTP = (struct TPHTTP *)tpItemInfo->Param1;
	if (tpHTTP == NULL) {
		HTTP_FreeItem(tpItemInfo);
		return -1;
	}
	ProxyFlag = GetOptionInt(tpItemInfo->Option2, OP2_NOPROXY);
	if ((ProxyFlag == 0 && Proxy == 1) || ProxyFlag == 2) {
		WCHAR wproxy[2048];
		char proxy[2048];
		wsprintf(proxy, "http://%s:%d", tpHTTP->SvName, tpHTTP->Port);
		MultiByteToWideChar(CP_ACP, 0, proxy, -1, wproxy, 2048);
		hSession = WinHttpOpen(USER_AGENT, WINHTTP_ACCESS_TYPE_NAMED_PROXY, wproxy, WINHTTP_NO_PROXY_BYPASS, 0);
	} else {
		hSession = WinHttpOpen(USER_AGENT, WINHTTP_ACCESS_TYPE_NO_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
	}
	if (hSession == NULL) {
		SetHttpError(tpItemInfo, STR_ERROR_CONNECT, "WinHttpOpen");
		SendMessage(tpHTTP->hWnd, WM_CHECK_END, CHECK_ERROR, (LPARAM)tpItemInfo);
		SendMessage(tpHTTP->hWnd, WM_CHECK_RESULT, ST_ERROR, (LPARAM)tpItemInfo);
		tpHTTP->hThread = NULL;
		HTTP_FreeItem(tpItemInfo);
		return -1;
	}

	ZeroMemory(&urlComponents, sizeof(URL_COMPONENTS));
	urlComponents.dwStructSize = sizeof(URL_COMPONENTS);
	urlComponents.lpszHostName = szHostName;
	urlComponents.dwHostNameLength = sizeof(szHostName) / sizeof(WCHAR);
	urlComponents.lpszUrlPath = szUrlPath;
	urlComponents.dwUrlPathLength = sizeof(szUrlPath) / sizeof(WCHAR);

	MultiByteToWideChar(CP_ACP, 0, (char *)tpItemInfo->Param2, -1, url, 2048);
	if (!WinHttpCrackUrl(url, wcslen(url), 0, &urlComponents)) {
		SetHttpError(tpItemInfo, STR_ERROR_CONNECT, "WinHttpCrackUrl");
		WinHttpCloseHandle(hSession);
		SendMessage(tpHTTP->hWnd, WM_CHECK_END, CHECK_ERROR, (LPARAM)tpItemInfo);
		SendMessage(tpHTTP->hWnd, WM_CHECK_RESULT, ST_ERROR, (LPARAM)tpItemInfo);
		tpHTTP->hThread = NULL;
		HTTP_FreeItem(tpItemInfo);
		return -1;
	}
	for (wp = szUrlPath; *wp != L'\0'; wp++) {
		if (*wp == L'#') {
			*wp = L'\0';
			break;
		}
	}

	hConnect = WinHttpConnect(hSession, szHostName, urlComponents.nPort, 0);
	if (hConnect == NULL) {
		SetHttpError(tpItemInfo, STR_ERROR_CONNECT, "WinHttpConnect");
		WinHttpCloseHandle(hSession);
		SendMessage(tpHTTP->hWnd, WM_CHECK_END, CHECK_ERROR, (LPARAM)tpItemInfo);
		SendMessage(tpHTTP->hWnd, WM_CHECK_RESULT, ST_ERROR, (LPARAM)tpItemInfo);
		tpHTTP->hThread = NULL;
		HTTP_FreeItem(tpItemInfo);
		return -1;
	}

	hRequest = WinHttpOpenRequest(hConnect,
		(tpItemInfo->user2 == REQUEST_HEAD) ? L"HEAD" : L"GET",
		szUrlPath, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
		(INTERNET_SCHEME_HTTPS == urlComponents.nScheme) ? WINHTTP_FLAG_SECURE : 0);
	if (hRequest == NULL) {
		SetHttpError(tpItemInfo, STR_ERROR_SEND, "WinHttpOpenRequest");
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
		SendMessage(tpHTTP->hWnd, WM_CHECK_END, CHECK_ERROR, (LPARAM)tpItemInfo);
		SendMessage(tpHTTP->hWnd, WM_CHECK_RESULT, ST_ERROR, (LPARAM)tpItemInfo);
		tpHTTP->hThread = NULL;
		HTTP_FreeItem(tpItemInfo);
		return -1;
	}
	DWORD dwFlags =
		SECURITY_FLAG_IGNORE_UNKNOWN_CA |
		SECURITY_FLAG_IGNORE_CERT_WRONG_USAGE |
		SECURITY_FLAG_IGNORE_CERT_CN_INVALID |
		SECURITY_FLAG_IGNORE_CERT_DATE_INVALID;
	WinHttpSetOption(hRequest, WINHTTP_OPTION_SECURITY_FLAGS, &dwFlags, sizeof(dwFlags));

	dwSize = BUFSIZE;
	if (Proxy == 1 && pUsePass == 1) {
		dwSize += (lstrlen(pUser) + 1 + lstrlen(pPass)) * 13 / 10;
	}
	if (GetOptionString(tpItemInfo->Option2, referer, OP2_REFERER) == TRUE) {
		dwSize += lstrlen(referer);
	}
	if (tpHTTP->user[0] != '\0') {
		dwSize += (lstrlen(tpHTTP->user) + 1 + lstrlen(tpHTTP->pass)) * 13 / 10;
	}
	if (GetOptionInt(tpItemInfo->Option2, OP2_USEPASS) == 1) {
		if (GetOptionString(tpItemInfo->Option2, user, OP2_USER) == FALSE) {
			*user = '\0';
		}
		if (GetOptionString(tpItemInfo->Option2, BaseStr2, OP2_PASS) == FALSE) {
			*pass = '\0';
		} else {
			dPass(BaseStr2, pass);
		}
		dwSize += (lstrlen(user) + 1 + lstrlen(pass)) * 13 / 10;
	}
	sendHeader = (WCHAR *)HeapAlloc(GetProcessHeap(), 0, dwSize * sizeof(WCHAR));
	s = iStrCpyW(sendHeader, "Accept: */*");
	//If-Modified-Since
	if (tpItemInfo->user2 == REQUEST_GET && tpItemInfo->Param3 == 0 &&
		tpItemInfo->DLLData1 != NULL && *tpItemInfo->DLLData1 != '\0' &&
		GetOptionInt(tpItemInfo->Option1, OP1_NODATE) == 0 &&
		GetOptionInt(tpItemInfo->Option1, OP1_META) == 0 &&
		GetOptionInt(tpItemInfo->Option1, OP1_MD5) == 0 &&
		GetOptionInt(tpItemInfo->Option1, OP1_NOTAGSIZE) == 0) {
		//GETで更新日でチェックする場合は強制的にIMSを発行
		s = iStrCpyW(s, "\r\nIf-Modified-Since: ");
		s = iStrCpyW(s, tpItemInfo->DLLData1);
	}
	//キャッシュ制御
	if (Proxy == 1 && pNoCache == 1) {
		s = iStrCpyW(s, "\r\nPragma: no-cache\r\nCache-Control: no-cache");
	}
	//プロキシ認証
	if (Proxy == 1 && pUsePass == 1) {
		wsprintf(BaseStr1, "%s:%s", pUser, pPass);
		eBase(BaseStr1, BaseStr2);

		s = iStrCpyW(s, "\r\nProxy-Authorization: Basic ");
		s = iStrCpyW(s, BaseStr2);
	}
	//Referer
	if (GetOptionString(tpItemInfo->Option2, referer, OP2_REFERER) == TRUE) {
		s = iStrCpyW(s, "\r\nReferer: ");
		s = iStrCpyW(s, referer);
	}
	//ページ認証
	*BaseStr2 = '\0';
	if (tpHTTP->user[0] != '\0') {
		lstrcpy(user, tpHTTP->user);

		if (tpHTTP->pass[0] != '\0') {
			lstrcpy(pass, tpHTTP->pass);
		}
		wsprintf(BaseStr1, "%s:%s", user, pass);
		eBase(BaseStr1, BaseStr2);
	}
	if (GetOptionInt(tpItemInfo->Option2, OP2_USEPASS) == 1) {
		if (GetOptionString(tpItemInfo->Option2, user, OP2_USER) == FALSE) {
			*user = '\0';
		}
		if (GetOptionString(tpItemInfo->Option2, BaseStr2, OP2_PASS) == FALSE) {
			*pass = '\0';
		} else {
			dPass(BaseStr2, pass);
		}
		wsprintf(BaseStr1, "%s:%s", user, pass);
		eBase(BaseStr1, BaseStr2);
	}
	if (*BaseStr2 != '\0') {
		s = iStrCpyW(s, "\r\nAuthorization: Basic ");
		s = iStrCpyW(s, BaseStr2);
	}

	if (!WinHttpSendRequest(hRequest, sendHeader, -1, WINHTTP_NO_REQUEST_DATA, 0, WINHTTP_IGNORE_REQUEST_TOTAL_LENGTH, 0)) {
		SetHttpError(tpItemInfo, STR_ERROR_SEND, "WinHttpSendRequest");
		HeapFree(GetProcessHeap(), 0, sendHeader);
		WinHttpCloseHandle(hRequest);
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
		SendMessage(tpHTTP->hWnd, WM_CHECK_END, CHECK_ERROR, (LPARAM)tpItemInfo);
		SendMessage(tpHTTP->hWnd, WM_CHECK_RESULT, ST_ERROR, (LPARAM)tpItemInfo);
		tpHTTP->hThread = NULL;
		HTTP_FreeItem(tpItemInfo);
		return -1;
	}
	HeapFree(GetProcessHeap(), 0, sendHeader);
	if (WinHttpReceiveResponse(hRequest, NULL) == FALSE) {
		SetHttpError(tpItemInfo, STR_ERROR_RECV, "WinHttpReceiveResponse");
		WinHttpCloseHandle(hRequest);
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
		SendMessage(tpHTTP->hWnd, WM_CHECK_END, CHECK_ERROR, (LPARAM)tpItemInfo);
		SendMessage(tpHTTP->hWnd, WM_CHECK_RESULT, ST_ERROR, (LPARAM)tpItemInfo);
		tpHTTP->hThread = NULL;
		HTTP_FreeItem(tpItemInfo);
		return -1;
	}
	dwSize = 0;
	if (WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_RAW_HEADERS_CRLF, WINHTTP_HEADER_NAME_BY_INDEX, NULL, &dwSize, WINHTTP_NO_HEADER_INDEX) == FALSE) {
		if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
			SetHttpError(tpItemInfo, STR_ERROR_RECV, "WinHttpQueryHeaders(Size)");
			WinHttpCloseHandle(hRequest);
			WinHttpCloseHandle(hConnect);
			WinHttpCloseHandle(hSession);
			SendMessage(tpHTTP->hWnd, WM_CHECK_END, CHECK_ERROR, (LPARAM)tpItemInfo);
			SendMessage(tpHTTP->hWnd, WM_CHECK_RESULT, ST_ERROR, (LPARAM)tpItemInfo);
			tpHTTP->hThread = NULL;
			HTTP_FreeItem(tpItemInfo);
			return -1;
		}
	}
	if (dwSize <= 12) {
		SetHttpError(tpItemInfo, STR_ERROR_RECV, "WinHttpQueryHeaders(dwSize)");
		WinHttpCloseHandle(hRequest);
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
		SendMessage(tpHTTP->hWnd, WM_CHECK_END, CHECK_ERROR, (LPARAM)tpItemInfo);
		SendMessage(tpHTTP->hWnd, WM_CHECK_RESULT, ST_ERROR, (LPARAM)tpItemInfo);
		tpHTTP->hThread = NULL;
		HTTP_FreeItem(tpItemInfo);
		return -1;
	}
	header = (WCHAR *)HeapAlloc(GetProcessHeap(), 0, dwSize);
	if (WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_RAW_HEADERS_CRLF, WINHTTP_HEADER_NAME_BY_INDEX, header, &dwSize, WINHTTP_NO_HEADER_INDEX) == FALSE) {
		SetHttpError(tpItemInfo, STR_ERROR_RECV, "WinHttpQueryHeaders");
		HeapFree(GetProcessHeap(), 0, header);
		WinHttpCloseHandle(hRequest);
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
		SendMessage(tpHTTP->hWnd, WM_CHECK_END, CHECK_ERROR, (LPARAM)tpItemInfo);
		SendMessage(tpHTTP->hWnd, WM_CHECK_RESULT, ST_ERROR, (LPARAM)tpItemInfo);
		tpHTTP->hThread = NULL;
		HTTP_FreeItem(tpItemInfo);
		return -1;
	}
	int clen = WideCharToMultiByte(CP_ACP, 0, header, -1, NULL, 0, NULL, NULL);
	char *cbuf = (char *)GlobalAlloc(GMEM_FIXED, (clen + 1) * sizeof(char));
	WideCharToMultiByte(CP_ACP, 0, header, -1, cbuf, clen, NULL, NULL);
	HeapFree(GetProcessHeap(), 0, header);
	if (tpHTTP->buf != NULL) GlobalFree((HGLOBAL)tpHTTP->buf);
	tpHTTP->Size = clen;
	tpHTTP->buf = cbuf;
	if (tpHTTP->HeadCheckFlag == FALSE && CheckHead(tpItemInfo, tpHTTP) == TRUE) {
		WinHttpCloseHandle(hRequest);
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
		SendMessage(tpHTTP->hWnd, WM_CHECK_END, HeaderFunc(tpHTTP->hWnd, tpItemInfo), (LPARAM)tpItemInfo);
		SetSBText(tpHTTP->hWnd, tpItemInfo, STR_STATUS_CHECKEND);
		tpHTTP->hThread = NULL;
		HTTP_FreeItem(tpItemInfo);
		return 0;
	}
	while (1) {
		DWORD dwSize = 0;
		if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) {
			SetHttpError(tpItemInfo, STR_ERROR_RECV, "WinHttpQueryDataAvailable");
			WinHttpCloseHandle(hRequest);
			WinHttpCloseHandle(hConnect);
			WinHttpCloseHandle(hSession);
			SendMessage(tpHTTP->hWnd, WM_CHECK_END, CHECK_ERROR, (LPARAM)tpItemInfo);
			SendMessage(tpHTTP->hWnd, WM_CHECK_RESULT, ST_ERROR, (LPARAM)tpItemInfo);
			tpHTTP->hThread = NULL;
			return -1;
		}
		if (dwSize == 0) {
			break;
		}

		BYTE *pszBuffer = (LPBYTE)HeapAlloc(GetProcessHeap(), 0, dwSize + 1);
		if (pszBuffer == NULL) {
			break;
		}
		if (!WinHttpReadData(hRequest, pszBuffer, dwSize, NULL)) {
			SetHttpError(tpItemInfo, STR_ERROR_RECV, "WinHttpReadData");
			HeapFree(GetProcessHeap(), 0, pszBuffer);
			WinHttpCloseHandle(hRequest);
			WinHttpCloseHandle(hConnect);
			WinHttpCloseHandle(hSession);
			SendMessage(tpHTTP->hWnd, WM_CHECK_END, CHECK_ERROR, (LPARAM)tpItemInfo);
			SendMessage(tpHTTP->hWnd, WM_CHECK_RESULT, ST_ERROR, (LPARAM)tpItemInfo);
			tpHTTP->hThread = NULL;
			HTTP_FreeItem(tpItemInfo);
			return -1;
		}
		pszBuffer[dwSize] = '\0';
		tpHTTP->Size += dwSize;

		wsprintf(buf, STR_STATUS_RECV, tpHTTP->Size);
		SetSBText(tpHTTP->hWnd, tpItemInfo, buf);

		char *wkbuf = (char *)GlobalAlloc(GMEM_FIXED, tpHTTP->Size + 1);
		if (wkbuf != NULL) {
			char *p = iStrCpy(wkbuf, tpHTTP->buf);
			lstrcpy(p, pszBuffer);
			if (tpHTTP->buf != NULL) GlobalFree((HGLOBAL)tpHTTP->buf);
			tpHTTP->buf = wkbuf;
			if (tpHTTP->HeadCheckFlag == FALSE && CheckHead(tpItemInfo, tpHTTP) == TRUE) {
				HeapFree(GetProcessHeap(), 0, pszBuffer);
				break;
			}
		}
		HeapFree(GetProcessHeap(), 0, pszBuffer);
	}
	WinHttpCloseHandle(hRequest);
	WinHttpCloseHandle(hConnect);
	WinHttpCloseHandle(hSession);

	SendMessage(tpHTTP->hWnd, WM_CHECK_END, HeaderFunc(tpHTTP->hWnd, tpItemInfo), (LPARAM)tpItemInfo);
	SetSBText(tpHTTP->hWnd, tpItemInfo, STR_STATUS_CHECKEND);
	tpHTTP->hThread = NULL;
	HTTP_FreeItem(tpItemInfo);
	return 0;
}


/******************************************************************************

	Start_HTTP_Thread

	HTTPスレッドの開始

******************************************************************************/

static BOOL Start_HTTP_Thread(const HWND hWnd, struct TPITEM *tpItemInfo)
{
	struct TPHTTP *tpHTTP;
	int threadId;

	tpHTTP = (struct TPHTTP *)tpItemInfo->Param1;
	if (tpHTTP == NULL) {
		return FALSE;
	}
	tpHTTP->hWnd = hWnd;
	tpHTTP->hThread = CreateThread(NULL, 0, HTTP_Thread, (void *)tpItemInfo, 0, (unsigned *)&threadId);
	return TRUE;
}


/******************************************************************************

	HTTP_Start

	チェックの開始

******************************************************************************/

__declspec(dllexport) int CALLBACK HTTP_Start(HWND hWnd, struct TPITEM *tpItemInfo)
{
	struct TPHTTP *tpHTTP;

	tpHTTP = (struct TPHTTP *)GlobalAlloc(GPTR, sizeof(struct TPHTTP));
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
		FreeURLData(tpHTTP);
		GlobalFree(tpHTTP);
		tpItemInfo->Param1 = 0;
		return CHECK_NO;
	}

	SetSBText(hWnd, tpItemInfo, STR_STATUS_CONNECT);

	if (Start_HTTP_Thread(hWnd, tpItemInfo) == FALSE) {
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

	if(iPath != NULL){
		p = iStrCpy(buf, fPath);
		p = iStrCpy(p, "\\");
		p = iStrCpy(p, iPath);
	}else{
		lstrcpy(buf, fPath);
	}

	URL = NULL;

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
			if(IsDBCSLeadByte((BYTE)*p) == TRUE){
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
	int i;

	for(i = 0; i < 100; i++){
		if(hWndList[i] != NULL){
			SendMessage(hWndList[i], WM_CLOSE, 0, 0);
		}
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
