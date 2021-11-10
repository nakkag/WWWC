/**************************************************************************

	WWWC (wwwc.dll)

	httpprop.c

	Copyright (C) 1996-2008 by Nakashima Tomoaki. All rights reserved.
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


/**************************************************************************
	Define
**************************************************************************/

#define WM_LV_EVENT					(WM_USER + 300)		//リストビューイベント

#define MAXPROPITEM					100


/**************************************************************************
	Global Variables
**************************************************************************/

struct TPITEM *gItemInfo;
int PropRet;

struct ITEMPROP
{
	HWND hWnd;
	struct TPITEM *tpItemInfo;
};
struct ITEMPROP ItemProp[MAXPROPITEM];


extern char app_path[];
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

extern char AppName[30][BUFSIZE];
extern int AppCnt;


/**************************************************************************
	Local Function Prototypes
**************************************************************************/

static LRESULT OptionNotifyProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static void AddItemInfo(HWND hWnd, struct TPITEM *tpItemInfo);
static struct TPITEM *GetItemInfo(HWND hWnd);
static void DeleteItemInfo(struct TPITEM *tpItemInfo);
static void DrawScrollControl(LPDRAWITEMSTRUCT lpDrawItem, UINT i);
static BOOL CALLBACK PropertyProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static void ReqTypeEnable(HWND hDlg);
static BOOL CALLBACK PropertyCheckProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK PropertyOptionProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK ProtocolPropertyCheckProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);


/******************************************************************************

	OptionNotifyProc

	ダイアログの通知を処理する

******************************************************************************/

static LRESULT OptionNotifyProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PSHNOTIFY *pshn = (PSHNOTIFY FAR *) lParam;
	NMHDR *lpnmhdr = (NMHDR FAR *)&pshn->hdr;

	switch(lpnmhdr->code)
	{
	case PSN_APPLY:
		SendMessage(hDlg, WM_COMMAND, IDOK, 0);
		PropRet = 0;
		break;

	case PSN_QUERYCANCEL:
		SendMessage(hDlg, WM_COMMAND, IDPCANCEL, 0);
		break;

	case PSN_RESET:
		PropRet = -1;
		break;

	default:
		return PSNRET_NOERROR;
	}
	return PSNRET_NOERROR;
}


/******************************************************************************

	AddItemInfo

	アイテム情報リストにアイテム情報を登録

******************************************************************************/

static void AddItemInfo(HWND hWnd, struct TPITEM *tpItemInfo)
{
	int i;

	for(i = 0; i < MAXPROPITEM; i++){
		if(ItemProp[i].hWnd == NULL){
			ItemProp[i].hWnd = hWnd;
			ItemProp[i].tpItemInfo = tpItemInfo;
			break;
		}
	}
}


/******************************************************************************

	GetItemInfo

	アイテム情報リストからアイテム情報を検索

******************************************************************************/

static struct TPITEM *GetItemInfo(HWND hWnd)
{
	int i;

	for(i = 0; i < MAXPROPITEM; i++){
		if(ItemProp[i].hWnd == hWnd){
			return ItemProp[i].tpItemInfo;
		}
	}
	return NULL;
}


/******************************************************************************

	DeleteItemInfo

	アイテム情報リストからアイテム情報を削除

******************************************************************************/

static void DeleteItemInfo(struct TPITEM *tpItemInfo)
{
	int i;

	for(i = 0; i < MAXPROPITEM; i++){
		if(ItemProp[i].tpItemInfo == tpItemInfo){
			ItemProp[i].hWnd = NULL;
			ItemProp[i].tpItemInfo = NULL;
			break;
		}
	}
}


/******************************************************************************

	DrawScrollControl

	スクロールバーのボタンの描画

******************************************************************************/

static void DrawScrollControl(LPDRAWITEMSTRUCT lpDrawItem, UINT i)
{
	#define FOCUSRECT_SIZE		3

	if(lpDrawItem->itemState & ODS_DISABLED){
		//使用不能
		i |= DFCS_INACTIVE;
	}
	if(lpDrawItem->itemState & ODS_SELECTED){
		//選択
		i |= DFCS_PUSHED;
	}

	//フレームコントロールの描画
	DrawFrameControl(lpDrawItem->hDC, &(lpDrawItem->rcItem), DFC_SCROLL, i);

	//フォーカス
	if(lpDrawItem->itemState & ODS_FOCUS){
		lpDrawItem->rcItem.left += FOCUSRECT_SIZE;
		lpDrawItem->rcItem.top += FOCUSRECT_SIZE;
		lpDrawItem->rcItem.right -= FOCUSRECT_SIZE;
		lpDrawItem->rcItem.bottom -= FOCUSRECT_SIZE;
		DrawFocusRect(lpDrawItem->hDC, &(lpDrawItem->rcItem));
	}
}


/******************************************************************************

	PropertyProc

	アイテムのプロパティ設定ダイアログ

******************************************************************************/

static BOOL CALLBACK PropertyProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	struct TPITEM *tpItemInfo;
	char tmp[BUFSIZE];
	char *buf;
	char *p, *r;
	int len;

	switch (uMsg)
	{
	case WM_INITDIALOG:
		AddItemInfo(GetParent(hDlg), gItemInfo);

		tpItemInfo = gItemInfo;

		/* アイテムの情報が空でない場合はアイテムの内容を表示する */
		/* タイトル */
		SendMessage(GetDlgItem(hDlg, IDC_EDIT_TITLE), WM_SETTEXT, 0,
			(LPARAM)((tpItemInfo->Title != NULL) ? tpItemInfo->Title : STR_NEWITEMNAME));

		/* チェックするURL */
		SendMessage(GetDlgItem(hDlg, IDC_EDIT_URL), WM_SETTEXT, 0,
			(LPARAM)((tpItemInfo->CheckURL != NULL) ? tpItemInfo->CheckURL : "https://"));

		/* 開くURL */
		SendMessage(GetDlgItem(hDlg, IDC_EDIT_VIEWURL), WM_SETTEXT, 0,
			(LPARAM)((tpItemInfo->ViewURL != NULL) ? tpItemInfo->ViewURL : ""));

		/* コメント */
		SendMessage(GetDlgItem(hDlg, IDC_EDIT_COMMENT), EM_LIMITTEXT, MAXSIZE - 2, 0);
		if(tpItemInfo->Comment != NULL){
			buf = (char *)GlobalAlloc(GPTR, lstrlen(tpItemInfo->Comment) + 1);
			if(buf != NULL){
				for(p = tpItemInfo->Comment, r = buf ;*p != '\0'; p++){
					if(IsDBCSLeadByte(*p) == TRUE){
						*(r++) = *(p++);
						*(r++) = *p;
					}else if(*p == '\\' && *(p + 1) == 'n'){
						*(r++) = '\r';
						*(r++) = '\n';
						p++;
					}else if(*p == '\\' && *(p + 1) == '\\'){
						*(r++) = '\\';
						p++;
					}else{
						*(r++) = *p;
					}
				}
				*r = '\0';
				SendMessage(GetDlgItem(hDlg, IDC_EDIT_COMMENT), WM_SETTEXT, 0, (LPARAM)buf);
				GlobalFree(buf);
			}
		}

		/* サイズ */
		if(tpItemInfo->Size != NULL){
			wsprintf(tmp, "%s バイト", tpItemInfo->Size);
			SendMessage(GetDlgItem(hDlg, IDC_EDIT_SIZE), WM_SETTEXT, 0, (LPARAM)tmp);
		}

		/* 更新日時 */
		SendMessage(GetDlgItem(hDlg, IDC_EDIT_UPDATE), WM_SETTEXT, 0,
			(LPARAM)((tpItemInfo->Date != NULL) ? tpItemInfo->Date : ""));

		/* チェックした日時 */
		SendMessage(GetDlgItem(hDlg, IDC_EDIT_CHECKDATE), WM_SETTEXT, 0,
			(LPARAM)((tpItemInfo->CheckDate != NULL) ? tpItemInfo->CheckDate : ""));

		/* エラー情報 */
		SendMessage(GetDlgItem(hDlg, IDC_EDIT_ERRORINFO), WM_SETTEXT, 0,
			(LPARAM)((tpItemInfo->ErrStatus != NULL) ? tpItemInfo->ErrStatus : ""));
		break;

	case WM_DRAWITEM:
		if((UINT)wParam != IDC_BUTTON_CHANGE){
			return FALSE;
		}
		//ボタンの描画
		DrawScrollControl((LPDRAWITEMSTRUCT)lParam, DFCS_SCROLLDOWN);
		break;

	case WM_NOTIFY:
		return OptionNotifyProc(hDlg, uMsg, wParam, lParam);

	case WM_COMMAND:
		switch(wParam)
		{
		case IDOK:
			tpItemInfo = GetItemInfo(GetParent(hDlg));

			/* タイトルを設定する */
			if(tpItemInfo->Title != NULL){
				GlobalFree(tpItemInfo->Title);
			}
			len = SendMessage(GetDlgItem(hDlg, IDC_EDIT_TITLE), WM_GETTEXTLENGTH, 0, 0) + 1;
			tpItemInfo->Title = GlobalAlloc(GPTR, len + 1);
			if(tpItemInfo->Title != NULL){
				SendMessage(GetDlgItem(hDlg, IDC_EDIT_TITLE), WM_GETTEXT, len, (LPARAM)tpItemInfo->Title);
			}

			/* URLを設定する */
			if(tpItemInfo->CheckURL != NULL){
				len = SendMessage(GetDlgItem(hDlg, IDC_EDIT_URL), WM_GETTEXTLENGTH, 0, 0) + 1;
				buf = GlobalAlloc(GPTR, len + 1);
				if(buf != NULL){
					SendMessage(GetDlgItem(hDlg, IDC_EDIT_URL), WM_GETTEXT, len, (LPARAM)buf);
					if(lstrcmp(tpItemInfo->CheckURL, buf) != 0){
						if(tpItemInfo->Size != NULL){
							GlobalFree(tpItemInfo->Size);
							tpItemInfo->Size = NULL;
						}
						if(tpItemInfo->Date != NULL){
							GlobalFree(tpItemInfo->Date);
							tpItemInfo->Date = NULL;
						}
						if(tpItemInfo->OldDate != NULL){
							GlobalFree(tpItemInfo->OldDate);
							tpItemInfo->OldDate = NULL;
						}
						if(tpItemInfo->OldSize != NULL){
							GlobalFree(tpItemInfo->OldSize);
							tpItemInfo->OldSize = NULL;
						}
						if(tpItemInfo->DLLData1 != NULL){
							GlobalFree(tpItemInfo->DLLData1);
							tpItemInfo->DLLData1 = NULL;
						}
						if(tpItemInfo->DLLData2 != NULL){
							GlobalFree(tpItemInfo->DLLData2);
							tpItemInfo->DLLData2 = NULL;
						}
					}
					GlobalFree(buf);
				}
				GlobalFree(tpItemInfo->CheckURL);
			}
			len = SendMessage(GetDlgItem(hDlg, IDC_EDIT_URL), WM_GETTEXTLENGTH, 0, 0) + 1;
			tpItemInfo->CheckURL = GlobalAlloc(GPTR, len + 1);
			if(tpItemInfo->CheckURL != NULL){
				SendMessage(GetDlgItem(hDlg,IDC_EDIT_URL), WM_GETTEXT, len, (LPARAM)tpItemInfo->CheckURL);
			}

			/* 開くURLを設定する */
			if(tpItemInfo->ViewURL != NULL){
				GlobalFree(tpItemInfo->ViewURL);
			}
			len = SendMessage(GetDlgItem(hDlg, IDC_EDIT_VIEWURL), WM_GETTEXTLENGTH, 0, 0) + 1;
			tpItemInfo->ViewURL = GlobalAlloc(GPTR, len + 1);
			if(tpItemInfo->ViewURL != NULL){
				SendMessage(GetDlgItem(hDlg, IDC_EDIT_VIEWURL), WM_GETTEXT, len, (LPARAM)tpItemInfo->ViewURL);
			}

			/* コメントを設定する */
			if(tpItemInfo->Comment != NULL){
				GlobalFree(tpItemInfo->Comment);
			}
			len = SendMessage(GetDlgItem(hDlg, IDC_EDIT_COMMENT), WM_GETTEXTLENGTH, 0, 0) + 1;
			tpItemInfo->Comment = (char *)GlobalAlloc(GPTR, len + 1);
			if(tpItemInfo->Comment != NULL){
				SendMessage(GetDlgItem(hDlg, IDC_EDIT_COMMENT), WM_GETTEXT, len, (LPARAM)tpItemInfo->Comment);
			}
			break;

		case IDC_BUTTON_GETURL:
			DialogBoxParam(ghinst, MAKEINTRESOURCE(IDD_DIALOG_URLLIST), hDlg, URLListroc, (LPARAM)hDlg);
			break;

		case IDC_BUTTON_CHANGE:		//URLの入れ替え
			/* チェックするURLの取得 */
			len = SendMessage(GetDlgItem(hDlg, IDC_EDIT_URL), WM_GETTEXTLENGTH, 0, 0) + 1;
			p = GlobalAlloc(GPTR, len + 1);
			if(p != NULL){
				SendMessage(GetDlgItem(hDlg,IDC_EDIT_URL), WM_GETTEXT, len, (LPARAM)p);
				SendMessage(GetDlgItem(hDlg, IDC_EDIT_VIEWURL), WM_SETTEXT, 0, (LPARAM)p);
				GlobalFree(p);
			}
			break;
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

static void ReqTypeEnable(HWND hDlg)
{
	if(CheckType == 1 ||
		(IsDlgButtonChecked(hDlg, IDC_CHECK_SIZE) != 0 && IsDlgButtonChecked(hDlg, IDC_CHECK_NOTAGSIZE) != 0) ||
		IsDlgButtonChecked(hDlg, IDC_CHECK_MD5) != 0 ||
		IsDlgButtonChecked(hDlg, IDC_CHECK_META) != 0){
		EnableWindow(GetDlgItem(hDlg, IDC_RADIO_REQTYPE_AUTO), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_RADIO_REQTYPE_GET), FALSE);
	}else{
		EnableWindow(GetDlgItem(hDlg, IDC_RADIO_REQTYPE_AUTO), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_RADIO_REQTYPE_GET), TRUE);
	}
}


/******************************************************************************

	PropertyCheckProc

	アイテムのチェック設定ダイアログ

******************************************************************************/

static BOOL CALLBACK PropertyCheckProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	struct TPITEM *tpItemInfo;
	char buf[BUFSIZE];
	char type[BUFSIZE];
	char name[BUFSIZE];
	char content[BUFSIZE];
	int ReqType;
	BOOL EnableFlag;

	switch (uMsg)
	{
	case WM_INITDIALOG:
		tpItemInfo = GetItemInfo(GetParent(hDlg));

		SendMessage(GetDlgItem(hDlg, IDC_EDIT_META_TYPE), EM_LIMITTEXT, BUFSIZE - 2, 0);
		SendMessage(GetDlgItem(hDlg, IDC_EDIT_META_TYPENAME), EM_LIMITTEXT, BUFSIZE - 2, 0);
		SendMessage(GetDlgItem(hDlg, IDC_EDIT_META_CONTENT), EM_LIMITTEXT, BUFSIZE - 2, 0);

		switch(GetOptionInt(tpItemInfo->Option1, OP1_REQTYPE))
		{
		case 1:
			SetWindowText(GetDlgItem(hDlg, IDC_RADIO_REQTYPE_AUTO), STR_REQTYPE_AUTO_GET);
		case 0:
			CheckDlgButton(hDlg, IDC_RADIO_REQTYPE_AUTO, 1);
			break;

		case 2:
			CheckDlgButton(hDlg, IDC_RADIO_REQTYPE_GET, 1);
			break;
		}

		CheckDlgButton(hDlg, IDC_CHECK_DATE, !GetOptionInt(tpItemInfo->Option1, OP1_NODATE));
		CheckDlgButton(hDlg, IDC_CHECK_SIZE, !GetOptionInt(tpItemInfo->Option1, OP1_NOSIZE));
		CheckDlgButton(hDlg, IDC_CHECK_NOTAGSIZE, GetOptionInt(tpItemInfo->Option1, OP1_NOTAGSIZE));
		CheckDlgButton(hDlg, IDC_CHECK_MD5, GetOptionInt(tpItemInfo->Option1, OP1_MD5));

		CheckDlgButton(hDlg, IDC_CHECK_META, GetOptionInt(tpItemInfo->Option1, OP1_META));

		if(GetOptionString(tpItemInfo->Option1, buf, OP1_TYPE) == TRUE){
			SendMessage(GetDlgItem(hDlg, IDC_EDIT_META_TYPE), WM_SETTEXT, 0, (LPARAM)buf);
		}
		if(GetOptionString(tpItemInfo->Option1, buf, OP1_NAME) == TRUE){
			SendMessage(GetDlgItem(hDlg, IDC_EDIT_META_TYPENAME), WM_SETTEXT, 0, (LPARAM)buf);
		}
		if(GetOptionString(tpItemInfo->Option1, buf, OP1_CONTENT) == TRUE){
			SendMessage(GetDlgItem(hDlg, IDC_EDIT_META_CONTENT), WM_SETTEXT, 0, (LPARAM)buf);
		}

		if(IsDlgButtonChecked(hDlg, IDC_CHECK_SIZE) == 0){
			EnableWindow(GetDlgItem(hDlg, IDC_CHECK_NOTAGSIZE), FALSE);
		}
		if(IsDlgButtonChecked(hDlg, IDC_CHECK_META) == 0){
			EnableWindow(GetDlgItem(hDlg, IDC_EDIT_META_TYPE), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_EDIT_META_TYPENAME), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_EDIT_META_CONTENT), FALSE);
		}else{
			EnableWindow(GetDlgItem(hDlg, IDC_CHECK_DATE), FALSE);
		}
		ReqTypeEnable(hDlg);
		break;

	case WM_NOTIFY:
		return OptionNotifyProc(hDlg, uMsg, wParam, lParam);

	case WM_COMMAND:
		switch(wParam)
		{
		case IDC_CHECK_SIZE:
			if(IsDlgButtonChecked(hDlg, IDC_CHECK_SIZE) == 0){
				EnableWindow(GetDlgItem(hDlg, IDC_CHECK_NOTAGSIZE), FALSE);
			}else{
				EnableWindow(GetDlgItem(hDlg, IDC_CHECK_NOTAGSIZE), TRUE);
			}
			ReqTypeEnable(hDlg);
			break;

		case IDC_CHECK_META:
			EnableFlag = (IsDlgButtonChecked(hDlg, IDC_CHECK_META) == 0) ? FALSE : TRUE;

			EnableWindow(GetDlgItem(hDlg, IDC_CHECK_DATE), !EnableFlag);

			EnableWindow(GetDlgItem(hDlg, IDC_EDIT_META_TYPE), EnableFlag);
			EnableWindow(GetDlgItem(hDlg, IDC_EDIT_META_TYPENAME), EnableFlag);
			EnableWindow(GetDlgItem(hDlg, IDC_EDIT_META_CONTENT), EnableFlag);

		case IDC_CHECK_NOTAGSIZE:
		case IDC_CHECK_MD5:
			ReqTypeEnable(hDlg);
			break;

		case IDOK:
			tpItemInfo = GetItemInfo(GetParent(hDlg));

			if(IsDlgButtonChecked(hDlg, IDC_CHECK_DATE) == 0 &&
				IsDlgButtonChecked(hDlg, IDC_CHECK_SIZE) == 0 &&
				IsDlgButtonChecked(hDlg, IDC_CHECK_MD5) == 0 &&
				IsDlgButtonChecked(hDlg, IDC_CHECK_META) == 0){
				CheckDlgButton(hDlg, IDC_CHECK_DATE, 1);
				CheckDlgButton(hDlg, IDC_CHECK_SIZE, 1);
			}

			ReqType = GetOptionInt(tpItemInfo->Option1, OP1_REQTYPE);
			ReqType = (IsDlgButtonChecked(hDlg, IDC_RADIO_REQTYPE_GET) == 1)
				? 2 : ((ReqType == 2) ? 0 : ReqType);

			SendMessage(GetDlgItem(hDlg, IDC_EDIT_META_TYPE), WM_GETTEXT, BUFSIZE - 1, (LPARAM)type);
			SendMessage(GetDlgItem(hDlg, IDC_EDIT_META_TYPENAME), WM_GETTEXT, BUFSIZE - 1, (LPARAM)name);
			SendMessage(GetDlgItem(hDlg, IDC_EDIT_META_CONTENT), WM_GETTEXT, BUFSIZE - 1, (LPARAM)content);

			if(tpItemInfo->Option1 != NULL){
				GlobalFree(tpItemInfo->Option1);
			}
			tpItemInfo->Option1 = (char *)GlobalAlloc(GPTR,
				30 + lstrlen(type) + lstrlen(name) + lstrlen(content));
			if(tpItemInfo->Option1 != NULL){
				wsprintf(tpItemInfo->Option1, "%d;;%d;;%d;;%d;;%d;;%s;;%s;;%s;;%d",
					ReqType,
					!IsDlgButtonChecked(hDlg, IDC_CHECK_DATE),
					!IsDlgButtonChecked(hDlg, IDC_CHECK_SIZE),
					IsDlgButtonChecked(hDlg, IDC_CHECK_NOTAGSIZE),
					IsDlgButtonChecked(hDlg, IDC_CHECK_META),
					type, name, content,
					IsDlgButtonChecked(hDlg, IDC_CHECK_MD5));
			}
			break;
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}


/******************************************************************************

	PropertyOptionProc

	アイテムのオプション設定ダイアログ

******************************************************************************/

static BOOL CALLBACK PropertyOptionProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	struct TPITEM *tpItemInfo;
	char buf[BUFSIZE];
	char proxy[BUFSIZE];
	char port[BUFSIZE];
	char user[BUFSIZE];
	char pass[BUFSIZE];
	char referer[BUFSIZE];
	BOOL EnableFlag;

	switch (uMsg)
	{
	case WM_INITDIALOG:
		tpItemInfo = GetItemInfo(GetParent(hDlg));

		/* アイテムの情報が空でない場合はアイテムの内容を表示する */
		SendMessage(GetDlgItem(hDlg, IDC_EDIT_PROXY), EM_LIMITTEXT, BUFSIZE - 2, 0);
		SendMessage(GetDlgItem(hDlg, IDC_EDIT_PROXY_PORT), EM_LIMITTEXT, BUFSIZE - 2, 0);
		SendMessage(GetDlgItem(hDlg, IDC_EDIT_REFERER), EM_LIMITTEXT, BUFSIZE - 2, 0);

		CheckDlgButton(hDlg, IDC_CHECK_NOPROXY, GetOptionInt(tpItemInfo->Option2, OP2_NOPROXY));
		CheckDlgButton(hDlg, IDC_CHECK_SETPROXY, GetOptionInt(tpItemInfo->Option2, OP2_SETPROXY));

		if(GetOptionString(tpItemInfo->Option2, buf, OP2_PROXY) == TRUE){
			SendMessage(GetDlgItem(hDlg, IDC_EDIT_PROXY), WM_SETTEXT, 0, (LPARAM)buf);
		}

		if(GetOptionString(tpItemInfo->Option2, buf, OP2_PORT) == TRUE){
			SendMessage(GetDlgItem(hDlg, IDC_EDIT_PROXY_PORT), WM_SETTEXT, 0, (LPARAM)buf);
		}

		CheckDlgButton(hDlg, IDC_CHECK_USEPASS, GetOptionInt(tpItemInfo->Option2, OP2_USEPASS));

		if(GetOptionString(tpItemInfo->Option2, buf, OP2_USER) == TRUE){
			SendMessage(GetDlgItem(hDlg, IDC_EDIT_USER), WM_SETTEXT, 0, (LPARAM)buf);
		}

		if(GetOptionString(tpItemInfo->Option2, buf, OP2_PASS) == TRUE){
			dPass(buf, pass);
			SendMessage(GetDlgItem(hDlg, IDC_EDIT_PASS), WM_SETTEXT, 0, (LPARAM)pass);
		}

		if(GetOptionString(tpItemInfo->Option2, buf, OP2_REFERER) == TRUE){
			SendMessage(GetDlgItem(hDlg, IDC_EDIT_REFERER), WM_SETTEXT, 0, (LPARAM)buf);
		}

		if(IsDlgButtonChecked(hDlg, IDC_CHECK_NOPROXY) != 0){
			EnableWindow(GetDlgItem(hDlg, IDC_CHECK_SETPROXY), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_EDIT_PROXY), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_EDIT_PROXY_PORT), FALSE);
		}
		if(IsDlgButtonChecked(hDlg, IDC_CHECK_SETPROXY) == 0){
			EnableWindow(GetDlgItem(hDlg, IDC_EDIT_PROXY), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_EDIT_PROXY_PORT), FALSE);
		}
		if(IsDlgButtonChecked(hDlg, IDC_CHECK_USEPASS) == 0){
			EnableWindow(GetDlgItem(hDlg, IDC_EDIT_USER), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_EDIT_PASS), FALSE);
		}
		break;

	case WM_NOTIFY:
		return OptionNotifyProc(hDlg, uMsg, wParam, lParam);

	case WM_COMMAND:
		switch(wParam)
		{
		case IDC_CHECK_NOPROXY:
			if(IsDlgButtonChecked(hDlg, IDC_CHECK_NOPROXY) != 0){
				EnableWindow(GetDlgItem(hDlg, IDC_CHECK_SETPROXY), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_EDIT_PROXY), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_EDIT_PROXY_PORT), FALSE);
			}else{
				EnableWindow(GetDlgItem(hDlg, IDC_CHECK_SETPROXY), TRUE);
				if(IsDlgButtonChecked(hDlg, IDC_CHECK_SETPROXY) != 0){
					EnableWindow(GetDlgItem(hDlg, IDC_EDIT_PROXY), TRUE);
					EnableWindow(GetDlgItem(hDlg, IDC_EDIT_PROXY_PORT), TRUE);
				}
			}
			break;

		case IDC_CHECK_SETPROXY:
			EnableFlag = (IsDlgButtonChecked(hDlg, IDC_CHECK_SETPROXY) == 0) ? FALSE : TRUE;

			EnableWindow(GetDlgItem(hDlg, IDC_EDIT_PROXY), EnableFlag);
			EnableWindow(GetDlgItem(hDlg, IDC_EDIT_PROXY_PORT), EnableFlag);
			break;

		case IDC_CHECK_USEPASS:
			EnableFlag = (IsDlgButtonChecked(hDlg, IDC_CHECK_USEPASS) == 0) ? FALSE : TRUE;

			EnableWindow(GetDlgItem(hDlg, IDC_EDIT_USER), EnableFlag);
			EnableWindow(GetDlgItem(hDlg, IDC_EDIT_PASS), EnableFlag);
			break;

		case IDOK:
			tpItemInfo = GetItemInfo(GetParent(hDlg));

			SendMessage(GetDlgItem(hDlg, IDC_EDIT_PROXY), WM_GETTEXT, BUFSIZE - 1, (LPARAM)proxy);
			SendMessage(GetDlgItem(hDlg, IDC_EDIT_PROXY_PORT), WM_GETTEXT, BUFSIZE - 1, (LPARAM)port);
			SendMessage(GetDlgItem(hDlg, IDC_EDIT_USER), WM_GETTEXT, BUFSIZE - 1, (LPARAM)user);
			SendMessage(GetDlgItem(hDlg, IDC_EDIT_PASS), WM_GETTEXT, BUFSIZE - 1, (LPARAM)buf);
			ePass(buf, pass);
			SendMessage(GetDlgItem(hDlg, IDC_EDIT_REFERER), WM_GETTEXT, BUFSIZE - 1, (LPARAM)referer);

			if(tpItemInfo->Option2 != NULL){
				GlobalFree(tpItemInfo->Option2);
			}
			tpItemInfo->Option2 = (char *)GlobalAlloc(GPTR,
				18 + lstrlen(proxy) + lstrlen(port) + lstrlen(user) + lstrlen(pass) + lstrlen(referer));
			if(tpItemInfo->Option2 != NULL){
				wsprintf(tpItemInfo->Option2, "%d;;%d;;%s;;%s;;%d;;%s;;%s;;%s",
					IsDlgButtonChecked(hDlg, IDC_CHECK_NOPROXY),
					IsDlgButtonChecked(hDlg, IDC_CHECK_SETPROXY),
					proxy, port,
					IsDlgButtonChecked(hDlg, IDC_CHECK_USEPASS),
					user, pass, referer);
			}
			break;
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}


/******************************************************************************

	HTTP_Property

	アイテムのプロパティ表示

******************************************************************************/

__declspec(dllexport) int CALLBACK HTTP_Property(HWND hWnd, struct TPITEM *tpItemInfo)
{
#define sizeof_PROPSHEETHEADER		40	//古いコモンコントロール対策
	PROPSHEETPAGE psp;
	PROPSHEETHEADER psh;
	HPROPSHEETPAGE hpsp[3];
	char *title;
	int i;

	for(i = 0; i < MAXPROPITEM; i++){
		if(ItemProp[i].hWnd == NULL){
			break;
		}
	}
	if(i >= MAXPROPITEM){
		return -1;
	}

	if(tpItemInfo->Title != NULL){
		title = (char *)GlobalAlloc(GPTR, lstrlen(tpItemInfo->Title) + lstrlen(STR_TITLE_PROP) + 1);
		if(title != NULL){
			wsprintf(title, STR_TITLE_PROP, tpItemInfo->Title);
		}
	}else{
		title = (char *)GlobalAlloc(GPTR, lstrlen(STR_TITLE_ADDITEM) + 1);
		if(title != NULL){
			lstrcpy(title, STR_TITLE_ADDITEM);
		}
	}

	psp.dwSize = sizeof(PROPSHEETPAGE);
	psp.dwFlags = PSP_DEFAULT;
	psp.hInstance = ghinst;

	psp.pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_HTTPPROP);
	psp.pfnDlgProc = PropertyProc;
	hpsp[0] = CreatePropertySheetPage(&psp);

	psp.pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_HTTPPROP_CHECK);
	psp.pfnDlgProc = PropertyCheckProc;
	hpsp[1] = CreatePropertySheetPage(&psp);

	psp.pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_HTTPPROP_OPTION);
	psp.pfnDlgProc = PropertyOptionProc;
	hpsp[2] = CreatePropertySheetPage(&psp);

	ZeroMemory(&psh, sizeof(PROPSHEETHEADER));
	psh.dwSize = sizeof_PROPSHEETHEADER;
	psh.dwFlags = PSH_NOAPPLYNOW;
	psh.hInstance = ghinst;
	psh.hwndParent = hWnd;
	psh.pszCaption = title;
	psh.nPages = sizeof(hpsp) / sizeof(HPROPSHEETPAGE);
	psh.phpage = hpsp;
	psh.nStartPage = 0;

	gItemInfo = tpItemInfo;
	PropRet = -1;

	PropertySheet(&psh);

	DeleteItemInfo(tpItemInfo);
	if(title != NULL){
		GlobalFree(title);
	}
	return PropRet;
}


/******************************************************************************

	ProtocolPropertyCheckProc

	プロトコルのプロパティ設定ダイアログ

******************************************************************************/

static BOOL CALLBACK ProtocolPropertyCheckProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	char buf[BUFSIZE];
	BOOL EnableFlag;

	switch (uMsg)
	{
	case WM_INITDIALOG:
		wsprintf(buf, "%ld", TimeOut);
		SendMessage(GetDlgItem(hDlg, IDC_EDIT_TIMEOUIT), WM_SETTEXT, 0, (LPARAM)buf);

		CheckDlgButton(hDlg, IDC_CHECK_REQGET, CheckType);

		CheckDlgButton(hDlg, IDC_CHECK_PROXY, Proxy);
		EnableWindow(GetDlgItem(hDlg, IDC_EDIT_PSERVER), Proxy);
		EnableWindow(GetDlgItem(hDlg, IDC_EDIT_PPORT), Proxy);
		EnableWindow(GetDlgItem(hDlg, IDC_CHECK_NOCACHE), Proxy);
		EnableWindow(GetDlgItem(hDlg, IDC_CHECK_USEPASS), Proxy);
		EnableWindow(GetDlgItem(hDlg, IDC_EDIT_USER), Proxy);
		EnableWindow(GetDlgItem(hDlg, IDC_EDIT_PASS), Proxy);

		SendMessage(GetDlgItem(hDlg, IDC_EDIT_PSERVER), WM_SETTEXT, 0, (LPARAM)pServer);
		wsprintf(buf,"%ld", pPort);
		SendMessage(GetDlgItem(hDlg, IDC_EDIT_PPORT), WM_SETTEXT, 0, (LPARAM)buf);

		CheckDlgButton(hDlg, IDC_CHECK_NOCACHE, pNoCache);

		CheckDlgButton(hDlg, IDC_CHECK_USEPASS, pUsePass);
		SendMessage(GetDlgItem(hDlg, IDC_EDIT_USER), WM_SETTEXT, 0, (LPARAM)pUser);
		SendMessage(GetDlgItem(hDlg, IDC_EDIT_PASS), WM_SETTEXT, 0, (LPARAM)pPass);

		if(IsDlgButtonChecked(hDlg, IDC_CHECK_USEPASS) == 0){
			EnableWindow(GetDlgItem(hDlg, IDC_EDIT_USER), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_EDIT_PASS), FALSE);
		}
		break;

	case WM_NOTIFY:
		return OptionNotifyProc(hDlg, uMsg, wParam, lParam);

	case WM_COMMAND:
		switch(wParam)
		{
		case IDC_CHECK_PROXY:
			if(IsDlgButtonChecked(hDlg, IDC_CHECK_PROXY) == 1){
				EnableWindow(GetDlgItem(hDlg, IDC_EDIT_PSERVER), TRUE);
				EnableWindow(GetDlgItem(hDlg, IDC_EDIT_PPORT), TRUE);

				EnableWindow(GetDlgItem(hDlg, IDC_CHECK_NOCACHE), TRUE);

				EnableWindow(GetDlgItem(hDlg, IDC_CHECK_USEPASS),TRUE);
				if(IsDlgButtonChecked(hDlg, IDC_CHECK_USEPASS) == 1){
					EnableWindow(GetDlgItem(hDlg, IDC_EDIT_USER), TRUE);
					EnableWindow(GetDlgItem(hDlg, IDC_EDIT_PASS), TRUE);
				}
			}else{
				EnableWindow(GetDlgItem(hDlg, IDC_EDIT_PSERVER), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_EDIT_PPORT), FALSE);

				EnableWindow(GetDlgItem(hDlg, IDC_CHECK_NOCACHE), FALSE);

				EnableWindow(GetDlgItem(hDlg, IDC_CHECK_USEPASS), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_EDIT_USER), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_EDIT_PASS), FALSE);
			}
			break;

		case IDC_CHECK_USEPASS:
			EnableFlag = (IsDlgButtonChecked(hDlg, IDC_CHECK_USEPASS) == 1) ? TRUE : FALSE;

			EnableWindow(GetDlgItem(hDlg, IDC_EDIT_USER), EnableFlag);
			EnableWindow(GetDlgItem(hDlg, IDC_EDIT_PASS), EnableFlag);
			break;

		case IDOK:
			SendMessage(GetDlgItem(hDlg, IDC_EDIT_TIMEOUIT), WM_GETTEXT, BUFSIZE - 1, (LPARAM)buf);
			TimeOut = atoi(buf);
			CheckType = IsDlgButtonChecked(hDlg, IDC_CHECK_REQGET);

			Proxy = IsDlgButtonChecked(hDlg, IDC_CHECK_PROXY);
			SendMessage(GetDlgItem(hDlg, IDC_EDIT_PSERVER), WM_GETTEXT, BUFSIZE - 1, (LPARAM)pServer);
			SendMessage(GetDlgItem(hDlg, IDC_EDIT_PPORT), WM_GETTEXT, BUFSIZE - 1, (LPARAM)buf);
			pPort = atoi(buf);

			pNoCache = IsDlgButtonChecked(hDlg, IDC_CHECK_NOCACHE);

			pUsePass = IsDlgButtonChecked(hDlg, IDC_CHECK_USEPASS);
			SendMessage(GetDlgItem(hDlg, IDC_EDIT_USER), WM_GETTEXT, BUFSIZE - 1, (LPARAM)pUser);
			SendMessage(GetDlgItem(hDlg, IDC_EDIT_PASS), WM_GETTEXT, BUFSIZE - 1, (LPARAM)pPass);

			wsprintf(buf, "%ld", TimeOut);
			WritePrivateProfileString("CHECK", "TimeOut", buf, app_path);
			wsprintf(buf, "%ld", CheckType);
			WritePrivateProfileString("CHECK", "CheckType", buf, app_path);

			wsprintf(buf, "%ld", Proxy);
			WritePrivateProfileString("PROXY", "Proxy", buf, app_path);
			WritePrivateProfileString("PROXY", "pServer", pServer, app_path);
			wsprintf(buf, "%ld",pPort);
			WritePrivateProfileString("PROXY", "pPort", buf, app_path);

			wsprintf(buf, "%ld", pNoCache);
			WritePrivateProfileString("PROXY", "pNoCache", buf, app_path);

			wsprintf(buf, "%ld", pUsePass);
			WritePrivateProfileString("PROXY", "pUsePass", buf, app_path);
			WritePrivateProfileString("PROXY", "pUser", pUser, app_path);
			ePass(pPass, buf);
			WritePrivateProfileString("PROXY", "pPass", buf, app_path);
			break;
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
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
		ShowWindow(GetDlgItem(hDlg, IDOK), SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDCANCEL), SW_HIDE);
		break;

	case WM_NOTIFY:
		return OptionNotifyProc(hDlg, uMsg, wParam, lParam);

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
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}


/******************************************************************************

	ViewProtocolProperties

	プロトコルのプロパティ表示

******************************************************************************/

__declspec(dllexport) int CALLBACK HTTP_ProtocolProperty(HWND hWnd)
{
#define sizeof_PROPSHEETHEADER		40	//古いコモンコントロール対策
	PROPSHEETPAGE psp;
	PROPSHEETHEADER psh;
	HPROPSHEETPAGE hpsp[2];

	psp.dwSize = sizeof(PROPSHEETPAGE);
	psp.dwFlags = PSP_DEFAULT;
	psp.hInstance = ghinst;

	psp.pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_PROTOCOLPROP_CHECK);
	psp.pfnDlgProc = ProtocolPropertyCheckProc;
	hpsp[0] = CreatePropertySheetPage(&psp);

	psp.pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_SETDDEAPP);
	psp.pfnDlgProc = SetDDEAppProc;
	hpsp[1] = CreatePropertySheetPage(&psp);

	ZeroMemory(&psh, sizeof(PROPSHEETHEADER));
	psh.dwSize = sizeof_PROPSHEETHEADER;
	psh.dwFlags = PSH_NOAPPLYNOW;
	psh.hInstance = ghinst;
	psh.hwndParent = hWnd;
	psh.pszCaption = STR_TITLE_SETHTTP;
	psh.nPages = sizeof(hpsp) / sizeof(HPROPSHEETPAGE);
	psh.phpage = hpsp;
	psh.nStartPage = 0;

	PropRet = -1;
	PropertySheet(&psh);
	return 1;
}
/* End of source */
