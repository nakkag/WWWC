/**************************************************************************

	WWWC

	Option.c

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
#ifdef OP_XP_STYLE
#include <uxtheme.h>
#include <vssym32.h>
#endif	//OP_XP_STYLE

#include "General.h"
#include "Profile.h"


/**************************************************************************
	Define
**************************************************************************/

#define PROP_CNT_FOLDER			2
#define PROP_CNT_OPTION			4

#define UD_CHECKMAX				MAKELONG((short)CHECK_MAX, (short)1)


/**************************************************************************
	Global Variables
**************************************************************************/

static int PropRet;
#ifdef OP_XP_STYLE
static HMODULE hModThemes;
#endif	//OP_XP_STYLE

//外部参照
extern HINSTANCE g_hinst;
extern char DefDirPath[];
extern char CurrentUser[];
extern char GeneralUser[];
extern HWND WWWCWnd;
extern char CuDir[];
extern int gCheckFlag;
extern struct TP_PROTOCOL *tpProtocol;
extern int ProtocolCnt;
extern HTREEITEM RootItem;
extern HTREEITEM PropItem;


extern int TrayIcon;
extern int TrayWinShow;
extern char Inet[];
extern int InetIndex;
extern int CheckMax;
extern int UPSnd;
extern char WaveFile[];
extern int UPMsg;


/**************************************************************************
	Local Function Prototypes
**************************************************************************/

#ifdef OP_XP_STYLE
static void init_themes(void);
static void themes_free(void);
#endif	//OP_XP_STYLE
static void AllocGetText(HWND hEdit, char **buf);
static BOOL CALLBACK OptionItemProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK OptionFolderProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK OptionCheckProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);


/******************************************************************************

	init_themes

	XP用スタイルの初期化

******************************************************************************/

#ifdef OP_XP_STYLE
static void init_themes(void)
{
	hModThemes = LoadLibrary(TEXT("uxtheme.dll"));
}
#endif	//OP_XP_STYLE


/******************************************************************************

	themes_free

	XP用スタイルの解放

******************************************************************************/

#ifdef OP_XP_STYLE
static void themes_free(void)
{
	FreeLibrary(hModThemes);
	hModThemes = NULL;
}
#endif	//OP_XP_STYLE


/******************************************************************************

	open_theme

	XP用スタイルを開く

******************************************************************************/

#ifdef OP_XP_STYLE
long open_theme(const HWND hWnd, const WCHAR *class_name)
{
	static FARPROC _OpenThemeData = NULL;

	if(hModThemes == NULL){
		return 0;
	}
	if(_OpenThemeData == NULL){
		_OpenThemeData = GetProcAddress(hModThemes, "OpenThemeData");
	}
	if(_OpenThemeData == NULL){
		return 0;
	}
	return (long)_OpenThemeData(hWnd, class_name);
}
#endif	//OP_XP_STYLE


/******************************************************************************

	close_theme

	XP用スタイルを閉じる

******************************************************************************/

#ifdef OP_XP_STYLE
void close_theme(long hTheme)
{
	static FARPROC _CloseThemeData = NULL;

	if(hModThemes == NULL){
		return;
	}
	if(_CloseThemeData == NULL){
		_CloseThemeData = GetProcAddress(hModThemes, "CloseThemeData");
	}
	if(_CloseThemeData == NULL){
		return;
	}
	_CloseThemeData((HTHEME)hTheme);
}
#endif	//OP_XP_STYLE


/******************************************************************************

	close_theme

	XP用スタイルでスクロールバーのボタンの描画

******************************************************************************/

#ifdef OP_XP_STYLE
BOOL draw_theme_scroll(LPDRAWITEMSTRUCT lpDrawItem, UINT i, long hTheme)
{
	static FARPROC _DrawThemeBackground = NULL;
	DWORD state = 0;

	if(hModThemes == NULL){
		return FALSE;
	}
	if(_DrawThemeBackground == NULL){
		_DrawThemeBackground = GetProcAddress(hModThemes, "DrawThemeBackground");
	}
	if(_DrawThemeBackground == NULL){
		return FALSE;
	}
	switch(i)
	{
	case DFCS_SCROLLUP:
		if(lpDrawItem->itemState & ODS_DISABLED){
			state = ABS_UPDISABLED;
		}else if(lpDrawItem->itemState & ODS_SELECTED){
			state = ABS_UPPRESSED;
		}else if(lpDrawItem->itemState & ODS_FOCUS){
			state = ABS_UPHOT;
		}else{
			state = ABS_UPNORMAL;
		}
		break;

	case DFCS_SCROLLDOWN:
		if(lpDrawItem->itemState & ODS_DISABLED){
			state = ABS_DOWNDISABLED;
		}else if(lpDrawItem->itemState & ODS_SELECTED){
			state = ABS_DOWNPRESSED;
		}else if(lpDrawItem->itemState & ODS_FOCUS){
			state = ABS_DOWNHOT;
		}else{
			state = ABS_DOWNNORMAL;
		}
		break;

	case DFCS_SCROLLRIGHT:
		if(lpDrawItem->itemState & ODS_DISABLED){
			state = ABS_RIGHTDISABLED;
		}else if(lpDrawItem->itemState & ODS_SELECTED){
			state = ABS_RIGHTPRESSED;
		}else if(lpDrawItem->itemState & ODS_FOCUS){
			state = ABS_RIGHTHOT;
		}else{
			state = ABS_RIGHTNORMAL;
		}
		break;
	}
	_DrawThemeBackground((HTHEME)hTheme, lpDrawItem->hDC, SBP_ARROWBTN, state, &(lpDrawItem->rcItem), NULL);
	return TRUE;
}
#endif	//OP_XP_STYLE


/******************************************************************************

	DrawScrollControl

	スクロールバーのボタンの描画

******************************************************************************/

void DrawScrollControl(LPDRAWITEMSTRUCT lpDrawItem, UINT i)
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

	AllocGetText

	EDITに設定されているサイズ分のメモリを確保してEDITの内容を設定する

******************************************************************************/

static void AllocGetText(HWND hEdit, char **buf)
{
	int len;

	if(*buf != NULL){
		GlobalFree(*buf);
	}
	len = SendMessage(hEdit, WM_GETTEXTLENGTH, 0, 0) + 1;
	*buf = (char *)GlobalAlloc(LMEM_FIXED, sizeof(char) * (len + 1));
	if(*buf != NULL){
		**buf = '\0';
		SendMessage(hEdit, WM_GETTEXT, len, (LPARAM)*buf);
	}
}


/******************************************************************************

	OptionItemProc

	プロトコルが登録されていないアイテムのプロパティ

******************************************************************************/

static BOOL CALLBACK OptionItemProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	struct TPITEM *tpItemInfo;
	HICON hIcon;
	char *tmp;

	switch (uMsg)
	{
	case WM_INITDIALOG:
		SetWindowLong(hDlg, GWL_USERDATA, lParam);
		tpItemInfo = (struct TPITEM *)lParam;
		if(tpItemInfo == NULL){
			EndDialog(hDlg, FALSE);
			break;
		}

		if(tpItemInfo->Title != NULL && *tpItemInfo->Title != '\0'){
			tmp = (char *)GlobalAlloc(GPTR, lstrlen(tpItemInfo->Title) + lstrlen("のプロパティ") + 1);
			if(tmp != NULL){
				wsprintf(tmp, "%sのプロパティ", tpItemInfo->Title);
			}
			SetWindowText(hDlg, tmp);
			GlobalFree(tmp);
		}else{
			SetWindowText(hDlg, "プロパティ");
		}

		//アイコンを設定
		hIcon = ImageList_GetIcon(ListView_GetImageList(GetDlgItem(WWWCWnd, WWWC_LIST), LVSIL_NORMAL), ICON_NOICON, 0);
		SendDlgItemMessage(hDlg, IDC_STATIC_ICON, STM_SETICON, (WPARAM)hIcon, 0);

		SendDlgItemMessage(hDlg, IDC_EDIT_TITLE, WM_SETTEXT, 0, (LPARAM)tpItemInfo->Title);
		SendDlgItemMessage(hDlg, IDC_EDIT_CHECKURL, WM_SETTEXT, 0, (LPARAM)tpItemInfo->CheckURL);
		SendDlgItemMessage(hDlg, IDC_EDIT_VIEWURL, WM_SETTEXT, 0, (LPARAM)tpItemInfo->ViewURL);

		SendMessage(GetDlgItem(hDlg, IDC_EDIT_SIZE), WM_SETTEXT, 0,
			(LPARAM)((tpItemInfo->Size != NULL) ? tpItemInfo->Size : ""));
		SendMessage(GetDlgItem(hDlg, IDC_EDIT_UPDATE), WM_SETTEXT, 0,
			(LPARAM)((tpItemInfo->Date != NULL) ? tpItemInfo->Date : ""));

		//コメントを表示
		if(tpItemInfo->Comment != NULL){
			SendDlgItemMessage(hDlg, IDC_EDIT_COMMENT, WM_SETTEXT, 0, (LPARAM)tpItemInfo->Comment);
		}
		break;

	case WM_CLOSE:
		hIcon = (HICON)SendDlgItemMessage(hDlg, IDC_STATIC_ICON, STM_GETICON, 0, 0);
		DestroyIcon(hIcon);
		EndDialog(hDlg, FALSE);
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDCANCEL:
			SendMessage(hDlg, WM_CLOSE, 0, 0);
			break;

		case IDOK:
			tpItemInfo = (struct TPITEM *)GetWindowLong(hDlg, GWL_USERDATA);
			if(tpItemInfo == NULL){
				EndDialog(hDlg, FALSE);
				break;
			}

			//タイトル
			AllocGetText(GetDlgItem(hDlg, IDC_EDIT_TITLE), &tpItemInfo->Title);

			//チェックするURL
			AllocGetText(GetDlgItem(hDlg, IDC_EDIT_CHECKURL), &tpItemInfo->CheckURL);

			//表示するURL
			AllocGetText(GetDlgItem(hDlg, IDC_EDIT_VIEWURL), &tpItemInfo->ViewURL);

			//コメント
			AllocGetText(GetDlgItem(hDlg, IDC_EDIT_COMMENT), &tpItemInfo->Comment);

			//アイコンの破棄
			hIcon = (HICON)SendDlgItemMessage(hDlg, IDC_STATIC_ICON, STM_GETICON, 0, 0);
			DestroyIcon(hIcon);

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

	ShowItemProp

	アイテムのプロパティ表示

******************************************************************************/

BOOL ShowItemProp(HWND hWnd, struct TPITEM *tpItemInfo)
{
	FARPROC Func_Property = NULL;
	char buf[BUFSIZE];
	int ProtocolIndex;
	int ret;

	ProtocolIndex = GetProtocolIndex(tpItemInfo->CheckURL);
	if(ProtocolIndex != -1 && tpProtocol[ProtocolIndex].lib != NULL){
		wsprintf(buf, "%sProperty", tpProtocol[ProtocolIndex].FuncHeader);
		Func_Property = GetProcAddress((HMODULE)tpProtocol[ProtocolIndex].lib, buf);
	}

	//アイテムのプロパティを表示
	if(Func_Property == NULL){
		return DialogBoxParam(g_hinst,
			MAKEINTRESOURCE(IDD_DIALOG_ITEM), hWnd, OptionItemProc, (LPARAM)tpItemInfo);
	}
	ret = Func_Property(hWnd, tpItemInfo);
	return ((ret == -1) ? FALSE : TRUE);
}


/******************************************************************************

	FindCheckTree

	チェック中のアイテムを検索する

******************************************************************************/

static void CALLBACK FindPropTree(HWND hWnd, HTREEITEM hItem, long Param)
{
	struct TPTREE *tpTreeInfo;
	int *ret = (int *)Param;

	if(*ret == 1 ||
		(tpTreeInfo = (struct TPTREE *)TreeView_GetlParam(GetDlgItem(hWnd, WWWC_TREE), hItem)) == NULL ||
		tpTreeInfo->ItemList == NULL){
		return;
	}

	if(tpTreeInfo->MemFlag > 0){
		*ret = 1;
	}
}
int FindPropItem(HWND hWnd, HTREEITEM hItem)
{
	int ret = 0;

	CallTreeItem(hWnd, hItem, (FARPROC)FindPropTree, (long)&ret);
	return ret;
}


/******************************************************************************

	OptionNotifyProc

	プロパティシートのイベントの通知

******************************************************************************/

LRESULT OptionNotifyProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PSHNOTIFY *pshn = (PSHNOTIFY FAR *) lParam;
	NMHDR *lpnmhdr = (NMHDR FAR *)&pshn->hdr;

	switch(lpnmhdr->code)
	{
	case PSN_APPLY:				//OK
		SendMessage(hDlg, WM_COMMAND, IDOK, 0);
		break;

	case PSN_QUERYCANCEL:		//キャンセル
		SendMessage(hDlg, WM_COMMAND, IDPCANCEL, 0);
		break;

	default:
		return PSNRET_NOERROR;
	}
	return PSNRET_NOERROR;
}


/******************************************************************************

	DialogLvNotifyProc

	オプション画面のリストビューメッセージ

******************************************************************************/

LRESULT DialogLvNotifyProc(HWND hWnd, LPARAM lParam, HWND hListView)
{
	NMHDR *CForm = (NMHDR *)lParam;
	LV_KEYDOWN *LKey = (LV_KEYDOWN *)lParam;
	LV_DISPINFO *plv = (LV_DISPINFO *)lParam;

	if(CForm->hwndFrom != hListView){
		return 0;
	}

	switch(plv->hdr.code)
	{
	case LVN_ITEMCHANGED:		//アイテムの選択状態の変更
		return SendMessage(hWnd, WM_LV_EVENT, plv->hdr.code, lParam);
	}

	switch(CForm->code)
	{
	case NM_DBLCLK:				//ダブルクリック
		SendMessage(hWnd, WM_COMMAND, IDC_BUTTON_EDIT, 0);
		return 1;
	}

	switch(LKey->hdr.code)
	{
	case LVN_KEYDOWN:			//キーダウン
		if(LKey->wVKey == VK_DELETE){
			SendMessage(hWnd, WM_COMMAND, IDC_BUTTON_DELETE, 0);
			return 1;
		}
	}
	return 0;
}


/******************************************************************************

	OptionFolderProc

	フォルダプロパティのプロシージャ

******************************************************************************/

static BOOL CALLBACK OptionFolderProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	struct TPTREE *tpTreeInfo;
	HTREEITEM cItem;
	HICON hIcon;
	char buf[BUFSIZE];
	char *tmp;
	char FolderPath[BUFSIZE];
	int ItemCnt, i;
	int TmpCnt;
	int TreeCnt;

	switch (uMsg)
	{
	case WM_INITDIALOG:
		//アイコンを設定
		i = TreeView_GetItemInfo(GetDlgItem(WWWCWnd, WWWC_TREE), PropItem, buf);
		if(PropItem != RootItem){
			hIcon = ImageList_GetIcon(ListView_GetImageList(GetDlgItem(WWWCWnd, WWWC_LIST), LVSIL_NORMAL), i, 0);
		}else{
			if(*Inet == '\0'){
				hIcon = (HICON)LoadImage(g_hinst, MAKEINTRESOURCE(IDI_ICON_INET), IMAGE_ICON, LICONSIZE, LICONSIZE, 0);
			}else{
				hIcon = ExtractIcon(g_hinst, Inet, InetIndex);
			}
		}
		SendDlgItemMessage(hDlg, IDC_STATIC_ICON, STM_SETICON, (WPARAM)hIcon, 0);

		//フォルダ名を表示
		SendDlgItemMessage(hDlg, IDC_EDIT_TITLE, WM_SETTEXT, 0, (LPARAM)buf);

		//場所を表示
		TreeView_GetPath(GetDlgItem(WWWCWnd, WWWC_TREE), PropItem, FolderPath, CuDir);
		SendDlgItemMessage(hDlg, IDC_EDIT_LOCATE, WM_SETTEXT, 0, (LPARAM)FolderPath);

		//アイテム数を取得
		tpTreeInfo = (struct TPTREE *)TreeView_GetlParam(GetDlgItem(WWWCWnd, WWWC_TREE), PropItem);
		if(tpTreeInfo == NULL){
			break;
		}

		WaitCursor(TRUE);
		if(tpTreeInfo->ItemList == NULL){
			if(ReadTreeMem(WWWCWnd, PropItem) == FALSE){
				WaitCursor(FALSE);
				break;
			}
		}
		WaitCursor(FALSE);

		TmpCnt = tpTreeInfo->ItemListCnt;
		ItemCnt = 0;
		for(i = 0;i < TmpCnt;i++){
			if((*(tpTreeInfo->ItemList + i)) != NULL){
				ItemCnt++;
			}
		}

		//フォルダ数を取得
		TreeCnt = 0;
		cItem = TreeView_GetChild(GetDlgItem(WWWCWnd, WWWC_TREE), PropItem);
		while(cItem != NULL){
			TreeCnt++;
			cItem = TreeView_GetNextSibling(GetDlgItem(WWWCWnd, WWWC_TREE), cItem);
		}

		//アイテム数、フォルダ数を表示
		wsprintf(buf, STR_ITEMCNT, ItemCnt, TreeCnt);
		SendDlgItemMessage(hDlg, IDC_EDIT_INFO, WM_SETTEXT, 0, (LPARAM)buf);

		//作成日時を表示
		if(GetFileMakeDay(FolderPath, buf) == TRUE){
			SendDlgItemMessage(hDlg, IDC_EDIT_MAKEDAY, WM_SETTEXT, 0, (LPARAM)buf);
		}else{
			SendDlgItemMessage(hDlg, IDC_EDIT_MAKEDAY, WM_SETTEXT, 0, (LPARAM)STR_NOMAKEDATE);
		}

		//コメントを表示
		SendDlgItemMessage(hDlg, IDC_EDIT_COMMENT, EM_LIMITTEXT, MAXSIZE - 2, 0);
		if(tpTreeInfo->Comment != NULL){
			tmp = (char *)GlobalAlloc(GPTR, lstrlen(tpTreeInfo->Comment) + 1);
			if(tmp != NULL){
				lstrcpy(tmp, tpTreeInfo->Comment);
				EscToCode(tmp);
				SendDlgItemMessage(hDlg, IDC_EDIT_COMMENT, WM_SETTEXT, 0, (LPARAM)tmp);
				GlobalFree(tmp);
			}
		}

		TreeView_SetIconState(WWWCWnd, PropItem, 0);
		CallTreeItem(WWWCWnd, PropItem, (FARPROC)TreeView_FreeItem, 0);
		break;

	case WM_NOTIFY:
		return OptionNotifyProc(hDlg, uMsg, wParam, lParam);

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDPCANCEL:
			hIcon = (HICON)SendDlgItemMessage(hDlg, IDC_STATIC_ICON, STM_GETICON, 0, 0);
			DestroyIcon(hIcon);
			break;

		case IDOK:
			tpTreeInfo = (struct TPTREE *)TreeView_GetlParam(GetDlgItem(WWWCWnd, WWWC_TREE), PropItem);
			if(tpTreeInfo == NULL){
				hIcon = (HICON)SendDlgItemMessage(hDlg, IDC_STATIC_ICON, STM_GETICON, 0, 0);
				DestroyIcon(hIcon);
				break;
			}
			i = SendDlgItemMessage(hDlg, IDC_EDIT_COMMENT, WM_GETTEXTLENGTH, 0, 0) + 1;
			tmp = (char *)GlobalAlloc(GPTR, i + 1);
			if(tmp == NULL){
				abort();
			}
			SendDlgItemMessage(hDlg, IDC_EDIT_COMMENT, WM_GETTEXT, i, (LPARAM)tmp);
			if(tpTreeInfo->Comment != NULL){
				GlobalFree(tpTreeInfo->Comment);
			}
			tpTreeInfo->Comment = CodeToEsc(tmp);
			GlobalFree(tmp);

			//アイコンの破棄
			hIcon = (HICON)SendDlgItemMessage(hDlg, IDC_STATIC_ICON, STM_GETICON, 0, 0);
			DestroyIcon(hIcon);
			break;
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}


/******************************************************************************

	ViewFolderProperties

	フォルダプロパティ表示

******************************************************************************/

int ViewFolderProperties(HWND hWnd, HTREEITEM hItem)
{
#define sizeof_PROPSHEETHEADER		40	//古いコモンコントロール対策
	PROPSHEETPAGE psp;
	PROPSHEETHEADER psh;
	HPROPSHEETPAGE hpsp[PROP_CNT_FOLDER];
	char buf[BUFSIZE];
	char name[BUFSIZE];

	TreeView_GetItemInfo(GetDlgItem(hWnd, WWWC_TREE), hItem, buf);
	wsprintf(name, STR_PROPTITLE, buf);

	psp.dwSize = sizeof(PROPSHEETPAGE);
	psp.dwFlags = PSP_DEFAULT;
	psp.hInstance = g_hinst;

	PropItem = hItem;

	psp.pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_FOLDER);
	psp.pfnDlgProc = OptionFolderProc;
	hpsp[0] = CreatePropertySheetPage(&psp);

	psp.pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_AUTOTIME);
	psp.pfnDlgProc = AutoCheckListProc;
	hpsp[1] = CreatePropertySheetPage(&psp);

	ZeroMemory(&psh, sizeof(PROPSHEETHEADER));
	psh.dwSize = sizeof_PROPSHEETHEADER;
	psh.dwFlags = PSH_NOAPPLYNOW;
	psh.hInstance = g_hinst;
	psh.hwndParent = hWnd;
	psh.pszCaption = (LPSTR)name;
	psh.nPages = sizeof(hpsp) / sizeof(HPROPSHEETPAGE);
	psh.phpage = hpsp;
	psh.nStartPage = 0;

	PropertySheet(&psh);

	TreeView_GetPath(GetDlgItem(hWnd, WWWC_TREE), hItem, buf, CuDir);
	PutDirInfo(GetDlgItem(hWnd, WWWC_TREE), buf, hItem);

	TreeView_UpdateList(hWnd, hItem);
	return 0;
}


/******************************************************************************

	OptionCheckProc

	チェックオプションのプロシージャ

******************************************************************************/

static BOOL CALLBACK OptionCheckProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	char buf[BUFSIZE];

	switch (uMsg)
	{
	case WM_INITDIALOG:
		//アイコンの設定
		SendDlgItemMessage(hDlg, IDC_STATIC_ICON, STM_SETICON, (WPARAM)LoadIcon(NULL, IDI_APPLICATION), 0);
		//スピンコントロールの設定
		SendDlgItemMessage(hDlg, IDC_SPIN_CHECKMAX, UDM_SETRANGE, 0, (LPARAM)MAKELONG((short)CHECK_MAX, (short)1));

		if(*GeneralUser != '\0'){
			CheckDlgButton(hDlg, IDC_CHECK_GENERALUSER, 1);
		}

		wsprintf(buf, "%d", CheckMax);
		SendDlgItemMessage(hDlg, IDC_EDIT_CHECKITEMCNT, WM_SETTEXT, 0, (LPARAM)buf);

		EnableWindow(GetDlgItem(hDlg, IDC_EDIT_CHECKITEMCNT), !gCheckFlag);

		CheckDlgButton(hDlg, IDC_CHECK_TRAY, TrayIcon);
		CheckDlgButton(hDlg, IDC_CHECK_TRAYWINSHOW, TrayWinShow);
		EnableWindow(GetDlgItem(hDlg, IDC_CHECK_TRAYWINSHOW), TrayIcon);

		CheckDlgButton(hDlg, IDC_CHECK_UPMSG, UPMsg);
		CheckDlgButton(hDlg, IDC_CHECK_UPSND, UPSnd);
		SendDlgItemMessage(hDlg, IDC_EDIT_WAVEFILE, WM_SETTEXT, 0, (LPARAM)WaveFile);

		EnableWindow(GetDlgItem(hDlg, IDC_EDIT_WAVEFILE), UPSnd);
		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_BROWS), UPSnd);
		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_PLAY), UPSnd);
		break;

	case WM_NOTIFY:
		return OptionNotifyProc(hDlg, uMsg, wParam, lParam);

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_CHECK_TRAY:
			EnableWindow(GetDlgItem(hDlg, IDC_CHECK_TRAYWINSHOW), IsDlgButtonChecked(hDlg, IDC_CHECK_TRAY));
			break;

		case IDC_CHECK_UPSND:
			EnableWindow(GetDlgItem(hDlg, IDC_EDIT_WAVEFILE), IsDlgButtonChecked(hDlg, IDC_CHECK_UPSND));
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_BROWS), IsDlgButtonChecked(hDlg, IDC_CHECK_UPSND));
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_PLAY), IsDlgButtonChecked(hDlg, IDC_CHECK_UPSND));
			break;

		case IDC_BUTTON_BROWS:
			SetFocus(GetDlgItem(hDlg, IDC_EDIT_WAVEFILE));
			if(FileSelect(hDlg, "", FILEFILTER_WAV, NULL, buf, NULL, 1) == -1){
				break;
			}
			SendDlgItemMessage(hDlg, IDC_EDIT_WAVEFILE, WM_SETTEXT, 0, (LPARAM)buf);
			break;

		case IDC_BUTTON_PLAY:
			SendDlgItemMessage(hDlg, IDC_EDIT_WAVEFILE, WM_GETTEXT, BUFSIZE - 1, (LPARAM)buf);
			if(*buf == '\0' || sndPlaySound(buf, SND_ASYNC | SND_NODEFAULT) == FALSE){
				MessageBeep(MB_ICONASTERISK);
			}
			break;

		case IDOK:
			if(IsDlgButtonChecked(hDlg, IDC_CHECK_GENERALUSER) == 1){
				if(*GeneralUser == '\0'){
					lstrcpy(GeneralUser, CurrentUser);
				}
			}else{
				*GeneralUser = '\0';
			}
			//共通で使用する場合のユーザ名を設定
			//空の場合はユーザ毎の設定
			wsprintf(buf, "%s\\%s", DefDirPath, GENERAL_INI);
			profile_initialize(buf, TRUE);
			profile_write_string("GENERAL", "User", GeneralUser, buf);
			profile_flush(buf);
			profile_free();

			if(gCheckFlag == 0){
				SendDlgItemMessage(hDlg, IDC_EDIT_CHECKITEMCNT, WM_GETTEXT, BUFSIZE - 1, (LPARAM)buf);
				CheckMax = atoi(buf);
				if(CheckMax < 1){
					CheckMax = 1;
				}
				if(CheckMax > CHECK_MAX){
					CheckMax = CHECK_MAX;
				}
			}
			TrayIcon = IsDlgButtonChecked(hDlg, IDC_CHECK_TRAY);
			TrayWinShow = IsDlgButtonChecked(hDlg, IDC_CHECK_TRAYWINSHOW);

			UPMsg = IsDlgButtonChecked(hDlg, IDC_CHECK_UPMSG);
			UPSnd = IsDlgButtonChecked(hDlg, IDC_CHECK_UPSND);
			SendDlgItemMessage(hDlg, IDC_EDIT_WAVEFILE, WM_GETTEXT, BUFSIZE - 1, (LPARAM)WaveFile);
			PropRet = 1;
			break;
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}


/******************************************************************************

	ViewProperties

	オプションの画面の表示

******************************************************************************/

int ViewProperties(HWND hWnd)
{
#define sizeof_PROPSHEETHEADER		40	//古いコモンコントロール対策
	PROPSHEETPAGE psp;
	PROPSHEETHEADER psh;
	HPROPSHEETPAGE hpsp[PROP_CNT_OPTION];

	psp.dwSize = sizeof(PROPSHEETPAGE);
	psp.dwFlags = PSP_DEFAULT;
	psp.hInstance = g_hinst;

	psp.pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_OPTION);
	psp.pfnDlgProc = OptionCheckProc;
	hpsp[0] = CreatePropertySheetPage(&psp);

	psp.pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_AUTOTIME);
	psp.pfnDlgProc = AutoCheckListProc;
	hpsp[1] = CreatePropertySheetPage(&psp);
	PropItem = NULL;

	psp.pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_PROTOCOL);
	psp.pfnDlgProc = SetProtocolProc;
	hpsp[2] = CreatePropertySheetPage(&psp);

	psp.pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_TOOLLIST);
	psp.pfnDlgProc = SetToolProc;
	hpsp[3] = CreatePropertySheetPage(&psp);

	ZeroMemory(&psh, sizeof(PROPSHEETHEADER));
	psh.dwSize = sizeof_PROPSHEETHEADER;
	psh.dwFlags = PSH_NOAPPLYNOW;
	psh.hInstance = g_hinst;
	psh.hwndParent = hWnd;
	psh.pszCaption = (LPSTR)STR_OPTIONTITLE;
	psh.nPages = sizeof(hpsp) / sizeof(HPROPSHEETPAGE);
	psh.phpage = hpsp;
	psh.nStartPage = 0;

#ifdef OP_XP_STYLE
	//XPスタイルの初期化
	init_themes();
#endif	//OP_XP_STYLE
	PropRet = 0;
	PropertySheet(&psh);
#ifdef OP_XP_STYLE
	themes_free();
#endif	//OP_XP_STYLE
	return PropRet;
}
/* End of source */
