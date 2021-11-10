/**************************************************************************

	WWWC

	ToolBar.c

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

#define BANDSIZE_X			22			//バンドサイズ
#define BANDSIZE_Y			22

#define BUTTON_MAX			100			//ボタンの最大数
#define TBMAXBUTTON			26			//ボタンの数
#define TBMAXBITMAP			14			//ボタンビットマップの数
#define TBICONSIZE_X		16			//ボタンビットマップのサイズ
#define TBICONSIZE_Y		16

#define ICON_VIEW_CNT		5			//標準のボタンの数
#define ICON_STD_CNT		7


/**************************************************************************
	Global Variables
**************************************************************************/

HWND TbWnd = NULL;

static TBBUTTON tbb[] = {
	{VIEW_PARENTFOLDER,	ID_KEY_UPDIR,					TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
	{0,					ID_MENUITEM_CHECK,				TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
	{1,					ID_MENUITEM_ALLCHECK,			TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
	{2,					ID_MENUITEM_FOLDERTREECHECK,	TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
	{3,					ID_MENUITEM_ALLERRORCHECK,		TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
	{4,					ID_MENUITEM_CHECKEND,			TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
	{5,					ID_MENUITEM_ALLCHECKEND,		TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
	{6,					ID_MENUITEM_INITICON,			TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
	{7,					ID_MENUITEM_ALLINITICON,		TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
	{STD_CUT,			ID_MENUITEM_CUT,				TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
	{STD_COPY,			ID_MENUITEM_COPY,				TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
	{STD_PASTE,			ID_MENUITEM_PASTE,				TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
	{STD_UNDO,			ID_MENUITEM_UNDO,				TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
	{STD_DELETE,		ID_MENUITEM_DELETE,				TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
	{STD_PROPERTIES,	ID_MENUITEM_PROP,				TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
	{VIEW_LARGEICONS,	ID_MENUITEM_VIEW_ICON,			TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
	{VIEW_SMALLICONS,	ID_MENUITEM_VIEW_SMALLICON,		TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
	{VIEW_LIST,			ID_MENUITEM_VIEW_LIST,			TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
	{VIEW_DETAILS,		ID_MENUITEM_VIEW_REPORT,		TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
	{8,					ID_MENUITEM_VIEW_REPORT_LINE,	TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
	{STD_FILENEW,		ID_MENUITEM_TBNEW,				TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
	{9,					ID_MENUITEM_OPTION,				TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
	{10,				ID_MENUITEM_VIEWUPMSG,			TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
	{11,				ID_MENUITEM_SERACH,				TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
	{12,				ID_MENUITEM_PREV_HISTORY,		TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
	{13,				ID_MENUITEM_NEXT_HISTORY,		TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
};
static TBBUTTON _tbb[sizeof(tbb) / sizeof(TBBUTTON)];

//外部参照
extern HINSTANCE g_hinst;				//アプリケーションのインスタンスハンドル
extern HWND AniWnd;
extern char ViewTbItem[];
extern int ViewTb;
extern int TbStyle;

/******************************************************************************

	ReTbButtonInfo

	ツールバー情報のリセット

******************************************************************************/

void ReTbButtonInfo(void)
{
	//ツールバーの情報をリセット
	CopyMemory(tbb, _tbb, sizeof(_tbb));
}


/******************************************************************************

	TbSetButton

	ツールバー情報の設定

******************************************************************************/

static int TbSetButton(TBBUTTON *tbtn, char *TBbuf)
{
	char buf[BUFSIZE], *p, *r;
	int i, j;

	//文字列からツールバー情報を作成
	r = buf;
	i = 0;
	for(p = TBbuf;*p != '\0';p++){
		if(*p == ','){
			*r = '\0';
			j = atoi(buf);
			if(j == -1 || lstrlen(buf) <= 0){
				//セパレータ
				(tbtn + i)->iBitmap = 0;
				(tbtn + i)->idCommand = 0;
				(tbtn + i)->fsState = 0;
				(tbtn + i)->fsStyle = TBSTYLE_SEP;
				(tbtn + i)->dwData = 0;
				(tbtn + i)->iString = 0;
			}else{
				//ツールバー情報のコピー
				*(tbtn + i) = *(tbb + j);
			}
			i++;
			if(i >= BUTTON_MAX){
				break;
			}
			r = buf;
		}else{
			*(r++) = *p;
		}
	}
	return i;
}


/******************************************************************************

	SetTbBitmap

	標準のツールバービットマップの設定

******************************************************************************/

static void SetTbBitmap(HWND hWnd, int id, int bitmapIndex)
{
	int i;

	for(i = 0;i < TBMAXBUTTON;i++){
		if((tbb + i)->idCommand == id){
			(tbb + i)->iBitmap += bitmapIndex;
			break;
		}
	}
	SendDlgItemMessage(hWnd, WWWC_TB, TB_CHANGEBITMAP,
		id, (LPARAM)MAKELPARAM(tbb[i].iBitmap, 0));
}


/******************************************************************************

	CreateTB

	ツールバーの作成

******************************************************************************/

void CreateTB(HWND hWnd)
{
	TBBUTTON setTBB[BUTTON_MAX];
	TBADDBITMAP tb;
	int stdid, Viewdid;
	int i;

	CopyMemory(_tbb, tbb, sizeof(tbb));

	//ツールバー情報の設定
	i = TbSetButton(setTBB, ViewTbItem);

	//ツールバーの作成
	TbWnd = CreateToolbarEx(hWnd, WS_CHILD | TBSTYLE_TOOLTIPS,
		WWWC_TB, TBMAXBITMAP, g_hinst, IDR_TOOLBAR, setTBB, i, 0, 0,
		TBICONSIZE_X, TBICONSIZE_Y, sizeof(TBBUTTON));
	SetWindowLong(GetDlgItem(hWnd, WWWC_TB), GWL_STYLE,
		GetWindowLong(GetDlgItem(hWnd, WWWC_TB), GWL_STYLE) | TbStyle);
	SendMessage(GetDlgItem(hWnd, WWWC_TB), TB_SETINDENT, 5, 0);
	if(ViewTb == 1){
		ShowWindow(GetDlgItem(hWnd, WWWC_TB), SW_SHOW);
	}

	AniWnd = CreateDialogParam(g_hinst, MAKEINTRESOURCE(IDD_DIALOG_ANI),
		GetDlgItem(hWnd, WWWC_TB), AniProc, 0);
	SetWindowLong(AniWnd, GWL_EXSTYLE, WS_EX_STATICEDGE);
	ShowWindow(AniWnd, SW_SHOW);
	SendDlgItemMessage(hWnd, WWWC_TB, TB_AUTOSIZE, 0, 0);

	//標準のツールバービットマップの設定
	tb.hInst = HINST_COMMCTRL;

	tb.nID = IDB_VIEW_SMALL_COLOR;
	Viewdid = SendDlgItemMessage(hWnd, WWWC_TB,
		TB_ADDBITMAP, ICON_VIEW_CNT, (LPARAM)&tb);

	SetTbBitmap(hWnd, ID_KEY_UPDIR, Viewdid);
	SetTbBitmap(hWnd, ID_MENUITEM_VIEW_ICON, Viewdid);
	SetTbBitmap(hWnd, ID_MENUITEM_VIEW_SMALLICON, Viewdid);
	SetTbBitmap(hWnd, ID_MENUITEM_VIEW_LIST, Viewdid);
	SetTbBitmap(hWnd, ID_MENUITEM_VIEW_REPORT, Viewdid);

	tb.nID = IDB_STD_SMALL_COLOR;
	stdid = SendDlgItemMessage(hWnd, WWWC_TB,
		TB_ADDBITMAP, ICON_STD_CNT, (LPARAM)&tb);

	SetTbBitmap(hWnd, ID_MENUITEM_TBNEW, stdid);
	SetTbBitmap(hWnd, ID_MENUITEM_CUT, stdid);
	SetTbBitmap(hWnd, ID_MENUITEM_COPY, stdid);
	SetTbBitmap(hWnd, ID_MENUITEM_PASTE, stdid);
	SetTbBitmap(hWnd, ID_MENUITEM_UNDO, stdid);
	SetTbBitmap(hWnd, ID_MENUITEM_DELETE, stdid);
	SetTbBitmap(hWnd, ID_MENUITEM_PROP, stdid);
}


/******************************************************************************

	ToolBar_NotifyProc

	ツールバーのカスタマイズ

******************************************************************************/

LRESULT ToolBar_NotifyProc(HWND hWnd, LPARAM lParam)
{
	TBNOTIFY *pHdr = (TBNOTIFY *)lParam;
	TBBUTTON StartBtns[BUTTON_MAX];
	TBBUTTON Btns;
	TOOLTIPTEXT *pTT;
	char tmp[BUFSIZE];
	static char buf[BUFSIZE];
	UINT dwtemp;
	int loop, count;
	int i;

	switch(pHdr->hdr.code)
	{
	case TBN_ENDADJUST:		//カスタマイズの終了
		count = SendMessage(TbWnd, TB_BUTTONCOUNT, 0, 0);
		*ViewTbItem = '\0';

		//保存用文字列の作成
		for (loop = 0;loop < count;loop++){
			SendMessage(TbWnd, TB_GETBUTTON, loop, (LPARAM)&Btns);
			if(Btns.fsStyle == TBSTYLE_SEP){
				lstrcat(ViewTbItem, ",");
			}else{
				for(i = 0;i < TBMAXBUTTON;i++){
					if(Btns.idCommand == tbb[i].idCommand){
						wsprintf(tmp, "%d,", i);
						lstrcat(ViewTbItem, tmp);
					}
				}
			}
		}
		SetWindowLong(TbWnd, GWL_STYLE, GetWindowLong(TbWnd, GWL_STYLE) | TbStyle);

		SendMessage(hWnd, WM_TB_REFRESH, 0, 0);

		InvalidateRect(TbWnd, NULL, TRUE);
		UpdateWindow(TbWnd);
		break;

	case TBN_QUERYDELETE:	//削除
		return TRUE;

	case TBN_QUERYINSERT:	//追加
		return TRUE;

	case TBN_GETBUTTONINFO:	//ボタン文字列の取得
		if(pHdr->iItem < TBMAXBUTTON){
			pHdr->tbButton = tbb[pHdr->iItem];
			dwtemp = SendMessage(pHdr->hdr.hwndFrom, TB_GETSTATE, tbb[pHdr->iItem].idCommand, 0);
			if(dwtemp != -1){
				pHdr->tbButton.fsState = (BYTE)dwtemp;
			}
			if(pHdr->pszText == NULL){
				pHdr->pszText = buf;
				pHdr->cchText = sizeof(buf);
			}
			if(LoadString(g_hinst, tbb[pHdr->iItem].idCommand, pHdr->pszText, pHdr->cchText) == 0){
				*(pHdr->pszText) = '\0';
			}
			return TRUE;
		}else{
			return FALSE;
		}

	case TBN_RESET:			//リセット
		count = SendMessage(TbWnd, TB_BUTTONCOUNT, 0, 0);
		for(loop = 0;loop < count;loop++){
			SendMessage(TbWnd, TB_DELETEBUTTON, 0, 0);
		}
		i = TbSetButton(StartBtns, TBDEFBUTTON);
		SendMessage(TbWnd, TB_ADDBUTTONS, i, (LPARAM)StartBtns);
		break;

	case TTN_NEEDTEXT:		//ツールチップ用の文字列の取得
		pTT = (TOOLTIPTEXT*)lParam;
		pTT->hinst = g_hinst;
		pTT->lpszText = MAKEINTRESOURCE(pTT->hdr.idFrom);
		break;

	}
	return 0;
}


/******************************************************************************

	ShowTbMenu

	押されたボタンの位置にメニューを表示

******************************************************************************/

void ShowTbMenu(HWND hWnd, HMENU hMenu, int cmd)
{
	RECT WinRect;
	RECT TbRect;
	POINT apos;
	TBBUTTON tbButton;
	UINT ret;
	int i, count;

	//ボタンの位置を検索
	count = SendMessage(TbWnd, TB_BUTTONCOUNT, 0, 0);
	for(i = 0; i < count; i++){
		SendMessage(TbWnd, TB_GETBUTTON, i, (LPARAM)&tbButton);
		if(tbButton.idCommand == cmd){
			break;
		}
	}
	if(i >= count){
		GetCursorPos((LPPOINT)&apos);
		//メニューを表示
		ret = (UINT)TrackPopupMenu(hMenu, TPM_TOPALIGN | TPM_RIGHTBUTTON | TPM_LEFTBUTTON | TPM_RETURNCMD,
			apos.x, apos.y, 0, hWnd, NULL);

	}else{
		//ボタンの位置を取得
		GetWindowRect(TbWnd, &WinRect);
		SendMessage(TbWnd, TB_GETITEMRECT, i, (LPARAM)&TbRect);
		TbRect.left += WinRect.left;
		TbRect.bottom += WinRect.top;

		SendMessage(TbWnd, TB_PRESSBUTTON, cmd, (LPARAM) MAKELONG(1, 0));
		//メニューを表示
		ret = (UINT)TrackPopupMenu(hMenu, TPM_TOPALIGN | TPM_RIGHTBUTTON | TPM_LEFTBUTTON | TPM_RETURNCMD,
			TbRect.left, TbRect.bottom, 0, hWnd, NULL);
		SendMessage(TbWnd, TB_PRESSBUTTON, cmd, (LPARAM) MAKELONG(0, 0));
	}
	SendMessage(hWnd, WM_COMMAND, ret, 0);
}
/* End of source */
