/**************************************************************************

	WWWC

	Tool.c

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

#define TOOL_LISTCOL_SIZE1			200
#define TOOL_LISTCOL_SIZE2			100

#define ID_MENU_PUROTOCOL			43000
#define ID_MENU_COMMANDLINE			44000

#define DLLSELECT					45000

#define WM_HIDEOPTION				(WM_USER + 1)

#define CHECKOPTION(item, j, num)	CheckDlgButton(hDlg, item, (j & num) ? 1 : 0)
#define VISIBLEOPTION(item, j, num)	ShowWindow(GetDlgItem(hDlg, item), (j & num) ? SW_HIDE : SW_SHOW)
#define CHECKITEMCNT(item, num)		((IsDlgButtonChecked(hDlg, item) == 1) ? num : 0)


/**************************************************************************
	Global Variables
**************************************************************************/

HWND ToolWnd;
HACCEL hToolAccel;

struct TP_TOOLS *ToolList;
int ToolListCnt = 0;

//外部参照
extern HINSTANCE g_hinst;				//アプリケーションのインスタンスハンドル
extern HWND WWWCWnd;					//本体
extern struct TP_PROTOCOL *tpProtocol;
extern int ProtocolCnt;

extern int gCheckFlag;


/******************************************************************************

	FreeTool

	ツール情報を解放する

******************************************************************************/

void FreeTool(void)
{
	int i;

	//ツール情報を解放
	for(i = 0;i < ToolListCnt;i++){
		if(ToolList[i].lib != NULL){
			FreeLibrary(ToolList[i].lib);
		}
	}
	if(ToolList != NULL){
		GlobalFree(ToolList);
		ToolList = NULL;
	}
	ToolListCnt = 0;
	if(hToolAccel != NULL){
		DestroyAcceleratorTable(hToolAccel);
		hToolAccel = NULL;
	}
}


/******************************************************************************

	CreateToolAccelerator

	ツールのショートカットキーを作成

******************************************************************************/

void CreateToolAccelerator(void)
{
	ACCEL *ac;
	int cnt = 0;
	int i;

	if(hToolAccel != NULL){
		DestroyAcceleratorTable(hToolAccel);
		hToolAccel = NULL;
	}

	ac = (ACCEL *)LocalAlloc(LPTR, sizeof(ACCEL) * ToolListCnt);
	if(ac == NULL) return;

	for(i = 0;i < ToolListCnt;i++){
		if((ToolList[i].Action & TOOL_EXEC_WINDOWMENU) == 0 || (ToolList[i].Ctrl == 0 && ToolList[i].Key == 0)){
			continue;
		}
		ac[cnt].fVirt = ToolList[i].Ctrl | FVIRTKEY;
		ac[cnt].key = ToolList[i].Key;
		ac[cnt].cmd = ID_ACCEL_TOOL_ACTION + i;
		cnt++;
	}
	if(cnt != 0){
		hToolAccel = CreateAcceleratorTable(ac, cnt);
	}
	LocalFree(ac);
}


/******************************************************************************

	SetToolMenu

	ウィンドウメニューのツールを設定する

******************************************************************************/

void SetToolMenu(HWND hWnd)
{
	HMENU ToolMenu;
	char buf[BUFSIZE];
	char *p;
	int MenuCnt;
	int i;
	UINT scan_code;
	int ext_flag = 0;

	ToolMenu = GetSubMenu(GetMenu(hWnd), MENU_HWND_TOOL);

	MenuCnt = GetMenuItemCount(ToolMenu);
	for(i = 0;i < MenuCnt;i++){
		DeleteMenu(ToolMenu, 0, MF_BYPOSITION);
	}

	MenuCnt = 0;
	//ツールのリストからメニューを作成
	for(i = 0;i < ToolListCnt;i++){
		if((ToolList[i].Action & TOOL_EXEC_WINDOWMENU) != 0 && ToolList[i].title[0] != '\0'){
			if(lstrcmp(ToolList[i].title, "-") == 0){
				// メニュー名が "-" だった場合はセパレータにする
				AppendMenu(ToolMenu, MF_SEPARATOR, 0, NULL);
			}else{
				p = iStrCpy(buf, ToolList[i].title);
				if(ToolList[i].Key != 0 && (scan_code = MapVirtualKey(ToolList[i].Key, 0)) > 0){
					*(p++) = '\t';
					if(ToolList[i].Ctrl & 8){
						p = iStrCpy(p, "Ctrl+");
					}
					if(ToolList[i].Ctrl & 4){
						p = iStrCpy(p, "Shift+");
					}
					if(ToolList[i].Ctrl & 16){
						p = iStrCpy(p, "Alt+");
					}
					if(ToolList[i].Key == VK_APPS ||
						ToolList[i].Key == VK_PRIOR ||
						ToolList[i].Key == VK_NEXT ||
						ToolList[i].Key == VK_END ||
						ToolList[i].Key == VK_HOME ||
						ToolList[i].Key == VK_LEFT ||
						ToolList[i].Key == VK_UP ||
						ToolList[i].Key == VK_RIGHT ||
						ToolList[i].Key == VK_DOWN ||
						ToolList[i].Key == VK_INSERT ||
						ToolList[i].Key == VK_DELETE ||
						ToolList[i].Key == VK_NUMLOCK) ext_flag = 1 << 24;
					GetKeyNameText((scan_code << 16) | ext_flag, p, BUFSIZE - (p - buf) - 1);
				}
				AppendMenu(ToolMenu, MF_STRING, ID_WMENU_TOOL_ACTION + i, buf);
			}
			MenuCnt++;
		}
	}

	if(MenuCnt == 0){
		AppendMenu(ToolMenu, MF_STRING | MF_DISABLED | MF_GRAYED, 0, NOMENUITEM);
	}
}


/******************************************************************************

	DllToolExec

	DLLのツールを実行する

******************************************************************************/

int DllToolExec(HWND hWnd, int i, struct TPITEM **ToolItemList, int ToolItemListCnt, int type, int CheckType)
{
	int ret;

	if(ToolList[i].lib == NULL){
		ToolList[i].lib = LoadLibrary(ToolList[i].FileName);
		if(ToolList[i].lib == NULL){
			ErrMsg(hWnd, GetLastError(), NULL);
			return 0;
		}
	}

	//ツールの関数名からアドレスを取得
	if(ToolList[i].Func_Tool == NULL){
		ToolList[i].Func_Tool = GetProcAddress((HMODULE)ToolList[i].lib, ToolList[i].func);
	}
	if(ToolList[i].Func_Tool == NULL){
		ErrMsg(hWnd, GetLastError(), NULL);
	}else{
		//DLL内のツールを実行
		ret = ToolList[i].Func_Tool(hWnd, ToolItemList, ToolItemListCnt, type, CheckType);
	}
	if(WWWCWnd == NULL || IsWindow(WWWCWnd) == 0){
		//プロセスの強制終了
		exit(0);
	}
	return ret;
}


/******************************************************************************

	DllToolProp

	DLLのツールのプロパティを表示

******************************************************************************/

static int DllToolProp(HWND hWnd, char *File, char *Func)
{
	HINSTANCE hLib;
	FARPROC Func_Tool;
	int ret = 0;

	hLib = LoadLibrary(File);
	if(hLib == NULL){
		return 0;
	}

	//ツールの関数名からアドレスを取得
	Func_Tool = GetProcAddress((HMODULE)hLib, Func);
	if(Func_Tool != NULL){
		//DLL内のツールを実行
		ret = Func_Tool(hWnd, NULL, -1, TOOL_EXEC_PORP, 0);
	}
	FreeLibrary(hLib);
	if(WWWCWnd == NULL || IsWindow(WWWCWnd) == 0){
		//プロセスの強制終了
		exit(0);
	}
	return ret;
}


/******************************************************************************

	ExecTool

	ツールを実行

******************************************************************************/

void ExecTool(HWND hWnd, int id, BOOL MenuFlag)
{
	struct TPTREE *tpTreeInfo = NULL;
	struct TPITEM **ToolItemList = NULL;
	struct TPITEM **TmpItemList;
	HTREEITEM hItem = NULL;
	int i, j;

	if(gCheckFlag == 1 && (ToolList[id].Action & TOOL_EXEC_NOTCHECK) != 0){
		return;
	}

	if(GetDlgItem(hWnd, WWWC_TREE) != NULL){
		hItem = TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE));
	}
	if(hItem != NULL){
		tpTreeInfo = (struct TPTREE *)TreeView_GetlParam(GetDlgItem(hWnd, WWWC_TREE), hItem);
	}
	if(tpTreeInfo == NULL && MenuFlag == TRUE){
		return;
	}
	if(tpTreeInfo != NULL){
		tpTreeInfo->MemFlag++;
	}

	if((ToolList[id].Action & TOOL_EXEC_SAVEFOLDER) != 0){
		//フォルダの内容を保存
		SendMessage(hWnd, WM_FOLDER_SAVE, 0, 0);
	}

	if(MenuFlag == TRUE){
		//ウィンドウメニュー
		i = tpTreeInfo->ItemListCnt;
		ToolItemList = Item_ProtocolSelect(tpTreeInfo->ItemList, &i, ToolList[id].Protocol);
		if(ToolItemList != NULL){
			if(str_match("*.dll", ToolList[id].FileName) == TRUE){
				//DLLの実行
				DllToolExec(hWnd, id, ToolItemList, i, TOOL_EXEC_WINDOWMENU, 0);
			}else{
				//ツールに設定されたファイルを実行
				ExecItemFile(hWnd, ToolList[id].FileName, ToolList[id].CommandLine, NULL,
					ToolList[id].Action & TOOL_EXEC_SYNC);
			}
			GlobalFree(ToolItemList);
		}
	}else{
		TmpItemList = ListView_SelectItemToMem(GetDlgItem(hWnd, WWWC_LIST), &i);
		if(TmpItemList != NULL){
			ToolItemList = Item_ProtocolSelect(TmpItemList, &i, ToolList[id].Protocol);
			GlobalFree(TmpItemList);
		}
		if(ToolItemList != NULL){
			//アイテムメニュー
			if(str_match("*.dll", ToolList[id].FileName) == TRUE){
				//DLLの実行
				DllToolExec(hWnd, id, ToolItemList, i, TOOL_EXEC_ITEMMENU, 0);
			}else{
				for(j = 0; j < i; j++){
					if((*(ToolItemList + j)) == NULL){
						continue;
					}
					//ツールに設定されたファイルを実行
					ExecItemFile(hWnd, ToolList[id].FileName, ToolList[id].CommandLine, (*(ToolItemList + j)),
						ToolList[id].Action & TOOL_EXEC_SYNC);
				}
			}
			GlobalFree(ToolItemList);
		}
	}
	if(tpTreeInfo != NULL){
		tpTreeInfo->MemFlag--;
	}

	if((ToolList[id].Action & TOOL_EXEC_SAVEFOLDER) != 0){
		//フォルダの内容を読み直す
		SendMessage(hWnd, WM_FOLDER_LOAD, 0, 0);
	}

	//アイコンの初期化
	if(ToolList[id].Action & TOOL_EXEC_INITITEM){
		if(MenuFlag == TRUE){
			i = tpTreeInfo->ItemListCnt;
			ToolItemList = Item_ProtocolSelect(tpTreeInfo->ItemList, &i, ToolList[id].Protocol);
		}else{
			TmpItemList = ListView_SelectItemToMem(GetDlgItem(hWnd, WWWC_LIST), &i);
			if(TmpItemList != NULL){
				ToolItemList = Item_ProtocolSelect(TmpItemList, &i, ToolList[id].Protocol);
				GlobalFree(TmpItemList);
			}
		}
		if(ToolItemList != NULL){
			Item_ItemListIni(hWnd, ToolItemList, i);
			GlobalFree(ToolItemList);

			InvalidateRect(GetDlgItem(hWnd, WWWC_LIST), NULL, FALSE);
			UpdateWindow(GetDlgItem(hWnd, WWWC_LIST));
			TreeView_SetIconState(hWnd, TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)), 0);
		}
	}
	//フォルダの内容の保存
	if(tpTreeInfo != NULL){
		if(hItem == TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE))){
			ListView_RefreshItem(GetDlgItem(hWnd, WWWC_LIST));
		}
		TreeView_FreeItem(hWnd, hItem, 1);
	}
}


/******************************************************************************

	SubItemExecTool

	アイテムリストからツールの実行

******************************************************************************/

void SubItemExecTool(HWND hWnd, int id, struct TPITEM **ToolItemList, int cnt, int Action, int CheckType)
{
	int j;

	if(gCheckFlag == 1 && (ToolList[id].Action & TOOL_EXEC_NOTCHECK) != 0){
		return;
	}
	if((ToolList[id].Action & TOOL_EXEC_SAVEFOLDER) != 0){
		//フォルダの内容を保存
		SendMessage(WWWCWnd, WM_FOLDER_SAVE, 0, 0);
	}

	if(str_match("*.dll", ToolList[id].FileName) == TRUE){
		//DLLの実行
		DllToolExec(hWnd, id, ToolItemList, cnt, Action, CheckType);
	}else{
		if(Action == TOOL_EXEC_ITEMMENU){
			for(j = 0; j < cnt; j++){
				if((*(ToolItemList + j)) == NULL){
					continue;
				}
				//ツールに設定されたファイルを実行
				ExecItemFile(hWnd, ToolList[id].FileName, ToolList[id].CommandLine, (*(ToolItemList + j)),
					ToolList[id].Action & TOOL_EXEC_SYNC);
			}
		}else{
			ExecItemFile(hWnd, ToolList[id].FileName, ToolList[id].CommandLine, NULL, ToolList[id].Action & TOOL_EXEC_SYNC);
		}
	}

	if((ToolList[id].Action & TOOL_EXEC_SAVEFOLDER) != 0){
		SendMessage(WWWCWnd, WM_FOLDER_LOAD, 0, 0);
	}
}


/******************************************************************************

	SetDLLToolListProc

	DLL内のツールを取得してリスト表示するウィンドウプロシージャ

******************************************************************************/

static BOOL CALLBACK SetDLLToolListProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	struct TP_TOOLS *tpSetTool;
	struct TP_GETTOOL GetTool;
	HINSTANCE hLib;
	FARPROC Func_GetToolList;
	int cnt = 0;
	int ret, i;

	switch (uMsg)
	{
	case WM_INITDIALOG:
		SetWindowLong(hDlg, GWL_USERDATA, (LONG)lParam);

		tpSetTool = (struct TP_TOOLS *)lParam;
		SetWindowText(GetDlgItem(hDlg, IDC_STATIC_FILENAME), tpSetTool->FileName);

		hLib = LoadLibrary(tpSetTool->FileName);
		if(hLib == NULL){
			ErrMsg(hDlg, GetLastError(), NULL);
			EndDialog(hDlg, FALSE);
			break;
		}

		//ツール取得関数のアドレスを取得
		Func_GetToolList = GetProcAddress((HMODULE)hLib, "GetToolList");
		if(Func_GetToolList == NULL){
			ErrMsg(hDlg, GetLastError(), NULL);
			FreeLibrary(hLib);
			EndDialog(hDlg, FALSE);
			break;
		}

		//ツール情報を取得
		cnt = 0;
		while(1){
			ZeroMemory(&GetTool, sizeof(struct TP_GETTOOL));
			GetTool.iSize = sizeof(struct TP_GETTOOL);
			ret = Func_GetToolList(cnt++, &GetTool);
			if(ret <= -1){
				break;
			}
			SendDlgItemMessage(hDlg, IDC_LIST_DLLENTRY, LB_ADDSTRING, 0, (LPARAM)GetTool.title);
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
			//リストの選択項目を取得
			i = SendDlgItemMessage(hDlg, IDC_LIST_DLLENTRY, LB_GETCURSEL, 0, 0);
			if(i == -1){
				MessageBox(hDlg, EMSG_SELECT_TOOL, EMSG_SELECT_TITLE_TOOL, MB_OK | MB_ICONEXCLAMATION);
				break;
			}

			tpSetTool = (struct TP_TOOLS *)GetWindowLong(hDlg, GWL_USERDATA);

			hLib = LoadLibrary(tpSetTool->FileName);
			if(hLib == NULL){
				ErrMsg(hDlg, GetLastError(), NULL);
				EndDialog(hDlg, FALSE);
				break;
			}

			//ツール取得関数のアドレスを取得
			Func_GetToolList = GetProcAddress((HMODULE)hLib, "GetToolList");
			if(Func_GetToolList == NULL){
				ErrMsg(hDlg, GetLastError(), NULL);
				FreeLibrary(hLib);
				EndDialog(hDlg, FALSE);
				break;
			}

			ZeroMemory(&GetTool, sizeof(struct TP_GETTOOL));
			GetTool.iSize = sizeof(struct TP_GETTOOL);
			GetTool.MenuIndex = -1;
			ret = Func_GetToolList(i, &GetTool);

			CopyMemory(tpSetTool, &GetTool, sizeof(struct TP_GETTOOL));

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

	EnableProtocol

	プロトコル設定の有効無効切り替え

******************************************************************************/

static void EnableProtocol(HWND hDlg)
{
	int i;

	i = IsDlgButtonChecked(hDlg, IDC_CHECK_WINDOWMENU) |
		IsDlgButtonChecked(hDlg, IDC_CHECK_ITEMMENU) |
		IsDlgButtonChecked(hDlg, IDC_CHECK_CHECKEND);
	EnableWindow(GetDlgItem(hDlg, IDC_EDIT_PROTOCOL), i);
	EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_PUROTOCOL), i);
	EnableWindow(GetDlgItem(hDlg, IDC_CHECK_INITICON), i);
}


/******************************************************************************

	ToolInfoToDialog

	ツールの情報をダイログに表示する

******************************************************************************/

static void ToolInfoToDialog(HWND hDlg, struct TP_TOOLS *tpSetTool, char *funcStr)
{
	char buf[BUFSIZE];
	int i;

	SendDlgItemMessage(hDlg, IDC_EDIT_TITLE, WM_SETTEXT, 0, (LPARAM)tpSetTool->title);
	SendDlgItemMessage(hDlg, IDC_EDIT_FILENAME, WM_SETTEXT, 0, (LPARAM)tpSetTool->FileName);

	/* ホットキーの設定 */
	i = 0;
	if(tpSetTool->Ctrl & 4){
		i |= HOTKEYF_SHIFT;
	}
	if(tpSetTool->Ctrl & 8){
		i |= HOTKEYF_CONTROL;
	}
	if(tpSetTool->Ctrl & 16){
		i |= HOTKEYF_ALT;
	}
	if(tpSetTool->Key != 0){
		SendDlgItemMessage(hDlg, IDC_HOTKEY,
			HKM_SETHOTKEY, (WPARAM)MAKEWORD(tpSetTool->Key, i), 0);
	}

	CHECKOPTION(IDC_CHECK_ITEMMENU, tpSetTool->Action, TOOL_EXEC_ITEMMENU);
	CHECKOPTION(IDC_CHECK_WINDOWMENU, tpSetTool->Action, TOOL_EXEC_WINDOWMENU);
	CHECKOPTION(IDC_CHECK_START, tpSetTool->Action, TOOL_EXEC_START);
	CHECKOPTION(IDC_CHECK_END, tpSetTool->Action, TOOL_EXEC_END);
	CHECKOPTION(IDC_CHECK_CHECHSTART, tpSetTool->Action, TOOL_EXEC_CHECKSTART);
	CHECKOPTION(IDC_CHECK_CHECKEND, tpSetTool->Action, TOOL_EXEC_CHECKEND);

	if(tpSetTool->Action & TOOL_EXEC_CHECKENDUP){
		CheckDlgButton(hDlg, IDC_CHECK_CHECKENDUP, 1);
	}else if(tpSetTool->Action & TOOL_EXEC_CHECKENDNOUP){
		CheckDlgButton(hDlg, IDC_CHECK_CHECKENDNOUP, 1);
	}else{
		CheckDlgButton(hDlg, IDC_CHECK_CHECKENDEXEC, 1);
	}

	CHECKOPTION(IDC_CHECK_SYNC, tpSetTool->Action, TOOL_EXEC_SYNC);
	CHECKOPTION(IDC_CHECK_INITICON, tpSetTool->Action, TOOL_EXEC_INITITEM);
	CHECKOPTION(IDC_CHECK_MENUDEFAULT, tpSetTool->Action, TOOL_EXEC_MENUDEFAULT);
	CHECKOPTION(IDC_CHECK_NOTCHECK, tpSetTool->Action, TOOL_EXEC_NOTCHECK);
	CHECKOPTION(IDC_CHECK_SAVEFOLDER, tpSetTool->Action, TOOL_EXEC_SAVEFOLDER);

	SendDlgItemMessage(hDlg, IDC_EDIT_PROTOCOL, WM_SETTEXT, 0, (LPARAM)tpSetTool->Protocol);
	wsprintf(buf, "%d", tpSetTool->MenuIndex);
	SendDlgItemMessage(hDlg, IDC_EDIT_MENUINDEX, WM_SETTEXT, 0, (LPARAM)buf);

	if(str_match("*.dll", tpSetTool->FileName) == TRUE){
		SetWindowText(GetDlgItem(hDlg, IDC_STATIC_COMMANDLINE), TOOL_TITLE_DLLFUNC);
		EnableWindow(GetDlgItem(hDlg, IDC_EDIT_COMMANDLINE), FALSE);
		SendDlgItemMessage(hDlg, IDC_EDIT_COMMANDLINE, WM_SETTEXT, 0, (LPARAM)tpSetTool->func);
	}else{
		SetWindowText(GetDlgItem(hDlg, IDC_STATIC_COMMANDLINE), TOOL_TITLE_COMMANDLINE);
		EnableWindow(GetDlgItem(hDlg, IDC_EDIT_COMMANDLINE), TRUE);
		SendDlgItemMessage(hDlg, IDC_EDIT_COMMANDLINE, WM_SETTEXT, 0, (LPARAM)tpSetTool->CommandLine);
	}
	lstrcpy(funcStr, tpSetTool->func);
}


/******************************************************************************

	VisibleToolOption

	使用できないオプションを非表示にする

******************************************************************************/

static void VisibleToolOption(HWND hDlg, int HideOption)
{
	VISIBLEOPTION(IDC_CHECK_WINDOWMENU, HideOption, TOOL_HIDE_WINDOWMENU);
	VISIBLEOPTION(IDC_HOTKEY, HideOption, TOOL_HIDE_HOTKEY);
	VISIBLEOPTION(IDC_STATIC_HOTKEY, HideOption, TOOL_HIDE_HOTKEY);

	VISIBLEOPTION(IDC_CHECK_ITEMMENU, HideOption, TOOL_HIDE_ITEMMENU);
	VISIBLEOPTION(IDC_EDIT_MENUINDEX, HideOption, TOOL_HIDE_MENUINDEX);
	VISIBLEOPTION(IDC_STATIC_MENUINDEX, HideOption, TOOL_HIDE_MENUINDEX);
	VISIBLEOPTION(IDC_SPIN_MENUINDEX, HideOption, TOOL_HIDE_MENUINDEX);
	VISIBLEOPTION(IDC_CHECK_MENUDEFAULT, HideOption, TOOL_HIDE_MENUDEFAULT);

	VISIBLEOPTION(IDC_EDIT_PROTOCOL, HideOption, TOOL_HIDE_PROTOCOL);
	VISIBLEOPTION(IDC_STATIC_PROTOCOL, HideOption, TOOL_HIDE_PROTOCOL);
	VISIBLEOPTION(IDC_BUTTON_PUROTOCOL, HideOption, TOOL_HIDE_PROTOCOL);

	VISIBLEOPTION(IDC_CHECK_INITICON, HideOption, TOOL_HIDE_INITITEM);

	VISIBLEOPTION(IDC_CHECK_START, HideOption, TOOL_HIDE_START);
	VISIBLEOPTION(IDC_CHECK_END, HideOption, TOOL_HIDE_END);
	VISIBLEOPTION(IDC_CHECK_CHECHSTART, HideOption, TOOL_HIDE_CHECKSTART);
	VISIBLEOPTION(IDC_CHECK_CHECKEND, HideOption, TOOL_HIDE_CHECKEND);
	VISIBLEOPTION(IDC_CHECK_CHECKENDEXEC, HideOption, TOOL_HIDE_CHECKEND);
	VISIBLEOPTION(IDC_CHECK_CHECKENDUP, HideOption, TOOL_HIDE_CHECKENDUP);
	VISIBLEOPTION(IDC_CHECK_CHECKENDNOUP, HideOption, TOOL_HIDE_CHECKENDNOUP);

	VISIBLEOPTION(IDC_CHECK_NOTCHECK, HideOption, TOOL_HIDE_NOTCHECK);
	VISIBLEOPTION(IDC_CHECK_SYNC, HideOption, TOOL_HIDE_SYNC);
	VISIBLEOPTION(IDC_CHECK_SAVEFOLDER, HideOption, TOOL_HIDE_SAVEFOLDER);
}


/******************************************************************************

	SetToolExeProc

	ツールの編集を行うウィンドウプロシージャ

******************************************************************************/

static BOOL CALLBACK SetToolExeProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	struct TP_TOOLS *tpSetTool;
	HMENU hMenu;		//ポップアップメニューのハンドル
	RECT ButtonRect;	//ボタンの位置
	HINSTANCE hLib;
	FARPROC Func_GetToolHideOption;
	char buf[BUFSIZE], tmp[BUFSIZE];
	char *p, *r;
	static char funcStr[BUFSIZE];
	int i;
#ifdef OP_XP_STYLE
	static long hThemeP, hThemeC;
#endif	//OP_XP_STYLE

	switch (uMsg)
	{
	case WM_INITDIALOG:
#ifdef OP_XP_STYLE
		//XP
		hThemeP = open_theme(GetDlgItem(hDlg, IDC_BUTTON_PUROTOCOL), L"SCROLLBAR");
		hThemeC = open_theme(GetDlgItem(hDlg, IDC_BUTTON_COMMANDLINE), L"SCROLLBAR");
#endif	//OP_XP_STYLE
		//スピンコントロールの設定
		SendDlgItemMessage(hDlg, IDC_SPIN_MENUINDEX,
			UDM_SETRANGE, 0, (LPARAM)MAKELONG((short)100, (short)-1));

		SetWindowLong(hDlg, GWL_USERDATA, (LONG)lParam);
		ToolInfoToDialog(hDlg, (struct TP_TOOLS *)lParam, funcStr);

		SendMessage(hDlg, WM_COMMAND, IDC_CHECK_WINDOWMENU, 0);
		SendMessage(hDlg, WM_COMMAND, IDC_CHECK_ITEMMENU, 0);
		SendMessage(hDlg, WM_COMMAND, IDC_CHECK_CHECKEND, 0);

		if(str_match("*.dll", ((struct TP_TOOLS *)lParam)->FileName) == TRUE){
			SendMessage(hDlg, WM_HIDEOPTION, 0, lParam);
		}
		break;

	case WM_DRAWITEM:
		//描画するフレームコントロールスタイルを設定
		switch((UINT)wParam)
		{
		case IDC_BUTTON_PUROTOCOL:
		case IDC_BUTTON_COMMANDLINE:
			i = DFCS_SCROLLRIGHT;
			break;

		default:
			return FALSE;
		}
		//ボタンの描画
#ifdef OP_XP_STYLE
		if(hThemeP != 0 && hThemeC != 0){
			draw_theme_scroll((LPDRAWITEMSTRUCT)lParam, i, (i == IDC_BUTTON_PUROTOCOL) ? hThemeP : hThemeC);
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
		if(hThemeP != 0 && hThemeC != 0){
			close_theme(hThemeP);
			close_theme(hThemeC);
		}
		hThemeP = open_theme(GetDlgItem(hDlg, IDC_BUTTON_PUROTOCOL), L"SCROLLBAR");
		hThemeC = open_theme(GetDlgItem(hDlg, IDC_BUTTON_COMMANDLINE), L"SCROLLBAR");
		break;
#endif	//OP_XP_STYLE

	case WM_CLOSE:
#ifdef OP_XP_STYLE
		if(hThemeP != 0 && hThemeC != 0){
			close_theme(hThemeP);
			close_theme(hThemeC);
		}
#endif	//OP_XP_STYLE
		EndDialog(hDlg, FALSE);
		break;

	case WM_HIDEOPTION:
		hLib = LoadLibrary(((struct TP_TOOLS *)lParam)->FileName);
		if(hLib == NULL){
			break;
		}
		//ツール取得関数のアドレスを取得
		Func_GetToolHideOption = GetProcAddress((HMODULE)hLib, "GetToolHideOption");
		if(Func_GetToolHideOption == NULL){
			FreeLibrary(hLib);
			break;
		}
		i = Func_GetToolHideOption(((struct TP_TOOLS *)lParam)->func);
		FreeLibrary(hLib);
		VisibleToolOption(hDlg, i);
		break;

	case WM_COMMAND:
		switch(wParam)
		{
		case IDC_CHECK_WINDOWMENU:
			EnableWindow(GetDlgItem(hDlg, IDC_HOTKEY), IsDlgButtonChecked(hDlg, IDC_CHECK_WINDOWMENU));
			EnableProtocol(hDlg);
			break;

		case IDC_CHECK_ITEMMENU:
			i = IsDlgButtonChecked(hDlg, IDC_CHECK_ITEMMENU);
			EnableWindow(GetDlgItem(hDlg, IDC_EDIT_MENUINDEX), i);
			EnableWindow(GetDlgItem(hDlg, IDC_CHECK_MENUDEFAULT), i);
			EnableProtocol(hDlg);
			break;

		case IDC_CHECK_CHECKEND:
			i = IsDlgButtonChecked(hDlg, IDC_CHECK_CHECKEND);
			EnableWindow(GetDlgItem(hDlg, IDC_CHECK_CHECKENDUP), i);
			EnableWindow(GetDlgItem(hDlg, IDC_CHECK_CHECKENDNOUP), i);
			EnableWindow(GetDlgItem(hDlg, IDC_CHECK_CHECKENDEXEC), i);
			EnableProtocol(hDlg);
			break;

		//ファイル選択
		case IDC_BUTTON_FILESELECT:
			SetFocus(GetDlgItem(hDlg, IDC_EDIT_FILENAME));
			if(FileSelect(hDlg, "", TOOL_FILE_FILTER, NULL, buf, NULL, 1) == -1){
				break;
			}
			if(str_match("*.dll", buf) == FALSE){
				SetWindowText(GetDlgItem(hDlg, IDC_STATIC_COMMANDLINE), TOOL_TITLE_COMMANDLINE);
				EnableWindow(GetDlgItem(hDlg, IDC_EDIT_COMMANDLINE), TRUE);
				SendDlgItemMessage(hDlg, IDC_EDIT_FILENAME, WM_SETTEXT, 0, (LPARAM)buf);

				*tmp = '\0';
				SendDlgItemMessage(hDlg, IDC_EDIT_TITLE, WM_GETTEXT, BUFSIZE - 1, (LPARAM)tmp);
				if(*tmp == '\0'){
					//タイトルの設定
					r = "";
					for(p = buf; *p != '\0'; p++){
						if(IsDBCSLeadByte((BYTE)*p) == TRUE){
							p++;
							continue;
						}
						if(*p == '\\' || *p == '/') r = p + 1;
					}
					SendDlgItemMessage(hDlg, IDC_EDIT_TITLE, WM_SETTEXT, 0, (LPARAM)r);
				}

				VisibleToolOption(hDlg, 0);
				break;
			}
			lParam = (LPARAM)buf;

		//DLL内の関数選択
		case DLLSELECT:
			tpSetTool = (struct TP_TOOLS *)GlobalAlloc(GPTR, sizeof(struct TP_TOOLS));
			if(tpSetTool == NULL){
				ErrMsg(hDlg, GetLastError(), NULL);
				break;
			}
			tpSetTool->iSize = sizeof(struct TP_TOOLS);
			lstrcpy(tpSetTool->FileName, (char *)lParam);
			tpSetTool->MenuIndex = -1;
			if(DialogBoxParam(g_hinst, MAKEINTRESOURCE(IDD_DIALOG_TOOL_DLLLIST),
				hDlg, SetDLLToolListProc, (LPARAM)tpSetTool) == TRUE){
				ToolInfoToDialog(hDlg, tpSetTool, funcStr);

				SendMessage(hDlg, WM_COMMAND, IDC_CHECK_WINDOWMENU, 0);
				SendMessage(hDlg, WM_COMMAND, IDC_CHECK_ITEMMENU, 0);
				SendMessage(hDlg, WM_COMMAND, IDC_CHECK_CHECKEND, 0);

				SendMessage(hDlg, WM_HIDEOPTION, 0, (LPARAM)tpSetTool);
			}
			GlobalFree(tpSetTool);
			break;

		//コマンドラインもしくは関数 選択ボタン
		case IDC_BUTTON_COMMANDLINE:
			*buf = '\0';
			SendDlgItemMessage(hDlg, IDC_EDIT_FILENAME, WM_GETTEXT, BUFSIZE - 1, (LPARAM)buf);
			if(str_match("*.dll", buf) == TRUE){
				SendMessage(hDlg, WM_COMMAND, DLLSELECT, (LPARAM)buf);
				break;
			}
			hMenu = CreatePopupMenu();

			AppendMenu(hMenu, MF_STRING, ID_MENU_COMMANDLINE + 0, COMMANDLINE_u);
			AppendMenu(hMenu, MF_STRING, ID_MENU_COMMANDLINE + 1, COMMANDLINE_c);
			AppendMenu(hMenu, MF_STRING, ID_MENU_COMMANDLINE + 2, COMMANDLINE_v);
			AppendMenu(hMenu, MF_STRING, ID_MENU_COMMANDLINE + 3, COMMANDLINE_t);
			AppendMenu(hMenu, MF_STRING, ID_MENU_COMMANDLINE + 4, COMMANDLINE_s);
			AppendMenu(hMenu, MF_STRING, ID_MENU_COMMANDLINE + 5, COMMANDLINE_d);
			AppendMenu(hMenu, MF_STRING, ID_MENU_COMMANDLINE + 6, COMMANDLINE_k);
			AppendMenu(hMenu, MF_STRING, ID_MENU_COMMANDLINE + 7, COMMANDLINE_m);
			AppendMenu(hMenu, MF_STRING, ID_MENU_COMMANDLINE + 8, COMMANDLINE_f);
			AppendMenu(hMenu, MF_STRING, ID_MENU_COMMANDLINE + 9, COMMANDLINE_F);
			AppendMenu(hMenu, MF_STRING, ID_MENU_COMMANDLINE + 10, COMMANDLINE_p);
			AppendMenu(hMenu, MF_STRING, ID_MENU_COMMANDLINE + 11, COMMANDLINE_P);

			GetWindowRect(GetDlgItem(hDlg, IDC_BUTTON_COMMANDLINE), (LPRECT)&ButtonRect);
			TrackPopupMenu(hMenu, TPM_TOPALIGN, ButtonRect.right, ButtonRect.top, 0, hDlg, NULL);
			DestroyMenu(hMenu);
			break;

		//プロトコル選択メニュー表示
		case IDC_BUTTON_PUROTOCOL:
			hMenu = CreatePopupMenu();

			for(i = 0;i < ProtocolCnt;i++){
				wsprintf(buf, "&%d. %s", i + 1, tpProtocol[i].title);
				AppendMenu(hMenu, MF_STRING, ID_MENU_PUROTOCOL + i, buf);
			}
			AppendMenu(hMenu, MF_STRING, ID_MENU_PUROTOCOL + i, TOOL_PROTOCOL_ALL);

			GetWindowRect(GetDlgItem(hDlg, IDC_BUTTON_PUROTOCOL), (LPRECT)&ButtonRect);
			TrackPopupMenu(hMenu, TPM_TOPALIGN, ButtonRect.right, ButtonRect.top, 0, hDlg, NULL);
			DestroyMenu(hMenu);
			break;

		case IDOK:
			SendDlgItemMessage(hDlg, IDC_EDIT_TITLE, WM_GETTEXT, BUFSIZE - 1, (LPARAM)buf);
			if(*buf == '\0'){
				MessageBox(hDlg, EMSG_NOINPUT, EMSG_NOINPUT_TITLE, MB_OK | MB_ICONEXCLAMATION);
				break;
			}
			tpSetTool = (struct TP_TOOLS *)GetWindowLong(hDlg, GWL_USERDATA);

			lstrcpy(tpSetTool->title, buf);
			*tpSetTool->FileName = '\0';
			SendDlgItemMessage(hDlg, IDC_EDIT_FILENAME, WM_GETTEXT, BUFSIZE - 1, (LPARAM)tpSetTool->FileName);

			/* ホットキーの取得 */
			i = SendDlgItemMessage(hDlg,IDC_HOTKEY, HKM_GETHOTKEY, 0, 0);
			tpSetTool->Key = LOBYTE(i);
			i = HIBYTE(i);
			tpSetTool->Ctrl = ((i & HOTKEYF_SHIFT) ? 4 : 0) |
				((i & HOTKEYF_CONTROL) ? 8 : 0) |
				((i & HOTKEYF_ALT) ? 16 : 0);

			tpSetTool->Action = 0;
			tpSetTool->Action += CHECKITEMCNT(IDC_CHECK_ITEMMENU, TOOL_EXEC_ITEMMENU);
			tpSetTool->Action += CHECKITEMCNT(IDC_CHECK_WINDOWMENU, TOOL_EXEC_WINDOWMENU);
			tpSetTool->Action += CHECKITEMCNT(IDC_CHECK_START, TOOL_EXEC_START);
			tpSetTool->Action += CHECKITEMCNT(IDC_CHECK_END, TOOL_EXEC_END);
			tpSetTool->Action += CHECKITEMCNT(IDC_CHECK_CHECHSTART, TOOL_EXEC_CHECKSTART);
			tpSetTool->Action += CHECKITEMCNT(IDC_CHECK_CHECKEND, TOOL_EXEC_CHECKEND);
			tpSetTool->Action += CHECKITEMCNT(IDC_CHECK_CHECKENDUP, TOOL_EXEC_CHECKENDUP);
			tpSetTool->Action += CHECKITEMCNT(IDC_CHECK_CHECKENDNOUP, TOOL_EXEC_CHECKENDNOUP);
			tpSetTool->Action += CHECKITEMCNT(IDC_CHECK_SYNC, TOOL_EXEC_SYNC);
			tpSetTool->Action += CHECKITEMCNT(IDC_CHECK_INITICON, TOOL_EXEC_INITITEM);
			tpSetTool->Action += CHECKITEMCNT(IDC_CHECK_MENUDEFAULT, TOOL_EXEC_MENUDEFAULT);
			tpSetTool->Action += CHECKITEMCNT(IDC_CHECK_NOTCHECK, TOOL_EXEC_NOTCHECK);
			tpSetTool->Action += CHECKITEMCNT(IDC_CHECK_SAVEFOLDER, TOOL_EXEC_SAVEFOLDER);

			*tpSetTool->Protocol = '\0';
			SendDlgItemMessage(hDlg, IDC_EDIT_PROTOCOL, WM_GETTEXT, BUFSIZE - 1, (LPARAM)tpSetTool->Protocol);

			SendDlgItemMessage(hDlg, IDC_EDIT_MENUINDEX, WM_GETTEXT, BUFSIZE - 1, (LPARAM)buf);
			if(*buf == '\0'){
				lstrcpy(buf, "-1");
			}
			tpSetTool->MenuIndex = atoi(buf);

			*tpSetTool->CommandLine = '\0';
			*tpSetTool->func = '\0';
			if(str_match("*.dll", tpSetTool->FileName) == TRUE){
				SendDlgItemMessage(hDlg, IDC_EDIT_COMMANDLINE, WM_GETTEXT, BUFSIZE - 1, (LPARAM)tpSetTool->func);
			}else{
				SendDlgItemMessage(hDlg, IDC_EDIT_COMMANDLINE, WM_GETTEXT, BUFSIZE - 1, (LPARAM)tpSetTool->CommandLine);
			}
#ifdef OP_XP_STYLE
			if(hThemeP != 0 && hThemeC != 0){
				close_theme(hThemeP);
				close_theme(hThemeC);
			}
#endif	//OP_XP_STYLE
			EndDialog(hDlg, TRUE);
			break;

		case IDCANCEL:
#ifdef OP_XP_STYLE
			if(hThemeP != 0 && hThemeC != 0){
				close_theme(hThemeP);
				close_theme(hThemeC);
			}
#endif	//OP_XP_STYLE
			EndDialog(hDlg, FALSE);
			break;

		default:
			if(LOWORD(wParam) >= ID_MENU_PUROTOCOL && LOWORD(wParam) < ID_MENU_PUROTOCOL + 100){
				//プロトコル選択メニュー
				i = LOWORD(wParam) - ID_MENU_PUROTOCOL;
				if(i >= ProtocolCnt){
					SendDlgItemMessage(hDlg, IDC_EDIT_PROTOCOL, WM_SETTEXT, 0, (LPARAM)"*");
					SetFocus(GetDlgItem(hDlg, IDC_EDIT_PROTOCOL));
					break;
				}

				*buf = '\0';
				SendDlgItemMessage(hDlg, IDC_EDIT_PROTOCOL, WM_GETTEXT, BUFSIZE - 1, (LPARAM)buf);
				if(lstrcmp(buf, "*") == 0){
					*buf = '\0';
				}

				if(*buf != '\0' && strlistcmp(tpProtocol[i].title, buf, ',') == TRUE){
					break;
				}
				if(*buf == '\0'){
					SendDlgItemMessage(hDlg, IDC_EDIT_PROTOCOL,
						WM_SETTEXT, 0, (LPARAM)tpProtocol[i].title);
					SetFocus(GetDlgItem(hDlg, IDC_EDIT_PROTOCOL));
					break;
				}
				if(*(buf + lstrlen(buf) - 1) != ','){
					lstrcat(buf, ",");
				}

				lstrcat(buf, tpProtocol[i].title);
				SendDlgItemMessage(hDlg, IDC_EDIT_PROTOCOL, WM_SETTEXT, 0, (LPARAM)buf);
				SetFocus(GetDlgItem(hDlg, IDC_EDIT_PROTOCOL));

			}else if(LOWORD(wParam) >= ID_MENU_COMMANDLINE && LOWORD(wParam) < ID_MENU_COMMANDLINE + 100){
				//コマンドライン選択メニュー
				*buf = '\0';
				SendDlgItemMessage(hDlg, IDC_EDIT_COMMANDLINE, WM_GETTEXT, BUFSIZE - 1, (LPARAM)buf);
				switch(LOWORD(wParam) - ID_MENU_COMMANDLINE)
				{
				case 0: p = "%u"; break;
				case 1: p = "%c"; break;
				case 2: p = "%v"; break;
				case 3: p = "%t"; break;
				case 4: p = "%s"; break;
				case 5: p = "%d"; break;
				case 6: p = "%k"; break;
				case 7: p = "%m"; break;
				case 8: p = "%f"; break;
				case 9: p = "%F"; break;
				case 10: p = "%p"; break;
				case 11: p = "%P"; break;
				default: return TRUE;
				}
				lstrcat(buf, p);
				SendDlgItemMessage(hDlg, IDC_EDIT_COMMANDLINE, WM_SETTEXT, 0, (LPARAM)buf);
				SetFocus(GetDlgItem(hDlg, IDC_EDIT_COMMANDLINE));
			}
			break;
		}
		break;

	default:
		return(FALSE);
	}
	return(TRUE);
}


/******************************************************************************

	ListToToolInfo

	リストビューのアイテムよりツールの情報を設定する

******************************************************************************/

static void ListToToolInfo(HWND hListView, int SelectItem, struct TP_TOOLS *tpSetTool)
{
	char buf[BUFSIZE];

	ZeroMemory(tpSetTool, sizeof(struct TP_TOOLS));
	tpSetTool->iSize = sizeof(struct TP_TOOLS);
	ListView_GetItemText(hListView, SelectItem, 0, tpSetTool->title, BUFSIZE - 1);
	ListView_GetItemText(hListView, SelectItem, 1, tpSetTool->FileName, BUFSIZE - 1);
	*buf = '\0';
	ListView_GetItemText(hListView, SelectItem, 2, buf, BUFSIZE - 1);
	tpSetTool->Action = atoi(buf);
	ListView_GetItemText(hListView, SelectItem, 3, tpSetTool->Protocol, BUFSIZE - 1);
	*buf = '\0';
	ListView_GetItemText(hListView, SelectItem, 4, buf, BUFSIZE - 1);
	tpSetTool->MenuIndex = atoi(buf);
	*buf = '\0';
	ListView_GetItemText(hListView, SelectItem, 6, buf, BUFSIZE - 1);
	tpSetTool->Ctrl = atoi(buf);
	*buf = '\0';
	ListView_GetItemText(hListView, SelectItem, 7, buf, BUFSIZE - 1);
	tpSetTool->Key = *buf;
	ListView_GetItemText(hListView, SelectItem, 8, tpSetTool->CommandLine, BUFSIZE - 1);
	ListView_GetItemText(hListView, SelectItem, 9, tpSetTool->func, BUFSIZE - 1);
}


/******************************************************************************

	ToolInfoToList

	ツールの情報をリストビューのアイテムに設定する

******************************************************************************/

static void ToolInfoToList(HWND hListView, int SelectItem, struct TP_TOOLS *tpSetTool)
{
	char buf[BUFSIZE], *p;
	UINT scan_code;
	int ext_flag = 0;

	ListView_SetItemText(hListView, SelectItem, 0, tpSetTool->title);
	ListView_SetItemText(hListView, SelectItem, 1, tpSetTool->FileName);
	wsprintf(buf, "%d", tpSetTool->Action);
	ListView_SetItemText(hListView, SelectItem, 2, buf);
	ListView_SetItemText(hListView, SelectItem, 3, tpSetTool->Protocol);
	wsprintf(buf, "%d", tpSetTool->MenuIndex);
	ListView_SetItemText(hListView, SelectItem, 4, buf);

	p = buf;
	*p = '\0';
	if(tpSetTool->Key != 0 && (scan_code = MapVirtualKey(tpSetTool->Key, 0)) > 0){
		if(tpSetTool->Ctrl & 8){
			p = iStrCpy(p, "Ctrl+");
		}
		if(tpSetTool->Ctrl & 4){
			p = iStrCpy(p, "Shift+");
		}
		if(tpSetTool->Ctrl & 16){
			p = iStrCpy(p, "Alt+");
		}
		if(tpSetTool->Key == VK_APPS ||
			tpSetTool->Key == VK_PRIOR ||
			tpSetTool->Key == VK_NEXT ||
			tpSetTool->Key == VK_END ||
			tpSetTool->Key == VK_HOME ||
			tpSetTool->Key == VK_LEFT ||
			tpSetTool->Key == VK_UP ||
			tpSetTool->Key == VK_RIGHT ||
			tpSetTool->Key == VK_DOWN ||
			tpSetTool->Key == VK_INSERT ||
			tpSetTool->Key == VK_DELETE ||
			tpSetTool->Key == VK_NUMLOCK) ext_flag = 1 << 24;
		GetKeyNameText((scan_code << 16) | ext_flag, p, BUFSIZE - (p - buf) - 1);
	}
	ListView_SetItemText(hListView, SelectItem, 5, buf);

	wsprintf(buf, "%d", tpSetTool->Ctrl);
	ListView_SetItemText(hListView, SelectItem, 6, buf);
	wsprintf(buf, "%c", tpSetTool->Key);
	ListView_SetItemText(hListView, SelectItem, 7, buf);
	ListView_SetItemText(hListView, SelectItem, 8, tpSetTool->CommandLine);
	ListView_SetItemText(hListView, SelectItem, 9, tpSetTool->func);
}


/******************************************************************************

	SetLvToolIcon

	リストビューにアイコンを設定

******************************************************************************/

static void SetLvToolIcon(HWND hListView)
{
	HIMAGELIST IconList;
	char buf[BUFSIZE];
	int i, cnt;

	IconList = ListView_GetImageList(hListView, LVSIL_SMALL);
	if(IconList == NULL){
		IconList = ImageList_Create(16, 16, ILC_COLOR16 | ILC_MASK, 0, 0);
	}else{
		ImageList_Remove(IconList, -1);
	}
	ImageList_SetBkColor(IconList, GetSysColor(COLOR_WINDOW));

	cnt = ListView_GetItemCount(hListView);
	for(i = 0; i < cnt; i++){
		*buf = '\0';
		ListView_GetItemText(hListView, i, 1, buf, BUFSIZE - 1);
		if(*buf == '\0'){
			ImageListIconAdd(IconList, IDI_ICON_NON, 16, "", 0);
		}else{
			ImageListFileIconAdd(IconList, buf, 0, 16, "", 0);
		}
		ListView_SetItemIcon(hListView, i, i);
	}
	ListView_SetImageList(hListView, IconList, LVSIL_SMALL);
}


/******************************************************************************

	SetToolProc

	ツール情報一覧のウィンドウプロシージャ

******************************************************************************/

BOOL CALLBACK SetToolProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	struct TP_TOOLS *tpSetTool;
	LV_COLUMN lvc;
	int SelectItem;
	int i, j;
#ifdef OP_XP_STYLE
	static long hThemeUp, hThemeDown;
#endif	//OP_XP_STYLE

	switch (uMsg)
	{
	case WM_INITDIALOG:
		ToolWnd = hDlg;
#ifdef OP_XP_STYLE
		//XP
		hThemeUp = open_theme(GetDlgItem(hDlg, IDC_BUTTON_UP), L"SCROLLBAR");
		hThemeDown = open_theme(GetDlgItem(hDlg, IDC_BUTTON_DOWN), L"SCROLLBAR");
#endif	//OP_XP_STYLE
		//リストビューのカラムの設定
		i = 0;
		lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;

		lvc.fmt = LVCFMT_LEFT;
		lvc.cx = TOOL_LISTCOL_SIZE1;
		lvc.pszText = TOOL_LISTCOL_TITLE;
		lvc.iSubItem = i;
		ListView_InsertColumn(GetDlgItem(hDlg, IDC_LIST_TOOL), i, &lvc);

		i++;
		lvc.fmt = LVCFMT_LEFT;
		lvc.cx = TOOL_LISTCOL_SIZE2;
		lvc.pszText = TOOL_LISTCOL_FILENAME;
		lvc.iSubItem = i;
		ListView_InsertColumn(GetDlgItem(hDlg, IDC_LIST_TOOL), i, &lvc);

		i++;
		lvc.fmt = LVCFMT_LEFT;
		lvc.cx = TOOL_LISTCOL_SIZE2;
		lvc.pszText = TOOL_LISTCOL_EXEOPTION;
		lvc.iSubItem = i;
		ListView_InsertColumn(GetDlgItem(hDlg, IDC_LIST_TOOL), i, &lvc);

		i++;
		lvc.fmt = LVCFMT_LEFT;
		lvc.cx = TOOL_LISTCOL_SIZE2;
		lvc.pszText = TOOL_LISTCOL_EXEPROTOCOL;
		lvc.iSubItem = i;
		ListView_InsertColumn(GetDlgItem(hDlg, IDC_LIST_TOOL), i, &lvc);

		i++;
		lvc.fmt = LVCFMT_LEFT;
		lvc.cx = TOOL_LISTCOL_SIZE2;
		lvc.pszText = TOOL_LISTCOL_MENUINDEX;
		lvc.iSubItem = i;
		ListView_InsertColumn(GetDlgItem(hDlg, IDC_LIST_TOOL), i, &lvc);

		i++;
		lvc.fmt = LVCFMT_LEFT;
		lvc.cx = TOOL_LISTCOL_SIZE2;
		lvc.pszText = TOOL_LISTCOL_SKEY;
		lvc.iSubItem = i;
		ListView_InsertColumn(GetDlgItem(hDlg, IDC_LIST_TOOL), i, &lvc);

		i++;
		lvc.fmt = LVCFMT_LEFT;
		lvc.cx = 0;
		lvc.pszText = TOOL_LISTCOL_CTRL;
		lvc.iSubItem = i;
		ListView_InsertColumn(GetDlgItem(hDlg, IDC_LIST_TOOL), i, &lvc);

		i++;
		lvc.fmt = LVCFMT_LEFT;
		lvc.cx = 0;
		lvc.pszText = TOOL_LISTCOL_KEY;
		lvc.iSubItem = i;
		ListView_InsertColumn(GetDlgItem(hDlg, IDC_LIST_TOOL), i, &lvc);

		i++;
		lvc.fmt = LVCFMT_LEFT;
		lvc.cx = TOOL_LISTCOL_SIZE2;
		lvc.pszText = TOOL_LISTCOL_COMMANDLINE;
		lvc.iSubItem = i;
		ListView_InsertColumn(GetDlgItem(hDlg, IDC_LIST_TOOL), i, &lvc);

		i++;
		lvc.fmt = LVCFMT_LEFT;
		lvc.cx = TOOL_LISTCOL_SIZE2;
		lvc.pszText = TOOL_LISTCOL_DLLFUNC;
		lvc.iSubItem = i;
		ListView_InsertColumn(GetDlgItem(hDlg, IDC_LIST_TOOL), i, &lvc);

		//リストビューのスタイルの設定
		SetWindowLong(GetDlgItem(hDlg, IDC_LIST_TOOL), GWL_STYLE,
			GetWindowLong(GetDlgItem(hDlg, IDC_LIST_TOOL), GWL_STYLE) | LVS_SHOWSELALWAYS);
		SendDlgItemMessage(hDlg, IDC_LIST_TOOL, LVM_SETEXTENDEDLISTVIEWSTYLE, 0,
			LVS_EX_ONECLICKACTIVATE | LVS_EX_FULLROWSELECT |
			SendDlgItemMessage(hDlg, IDC_LIST_TOOL, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0));

		ListView_SetRedraw(GetDlgItem(hDlg, IDC_LIST_TOOL), LDF_NODRAW);
		ListView_SetItemCount(GetDlgItem(hDlg, IDC_LIST_TOOL), ToolListCnt);
		//ツール情報をリストビューに追加
		if(ToolList != NULL){
			for(i = 0;i < ToolListCnt;i++){
				if((ToolList + i) == NULL){
					continue;
				}
				j = ListView_InsertItemEx(GetDlgItem(hDlg, IDC_LIST_TOOL), "", i, 0, i);
				ToolInfoToList(GetDlgItem(hDlg, IDC_LIST_TOOL), j, ToolList + i);
			}
		}
		SetLvToolIcon(GetDlgItem(hDlg, IDC_LIST_TOOL));
		ListView_SetRedraw(GetDlgItem(hDlg, IDC_LIST_TOOL), LDF_REDRAW);
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

	case WM_ADDTOOL:
		j = ListView_InsertItemEx(GetDlgItem(hDlg, IDC_LIST_TOOL), "",
			ListView_GetItemCount(GetDlgItem(hDlg, IDC_LIST_TOOL)), 0, -1);
		ToolInfoToList(GetDlgItem(hDlg, IDC_LIST_TOOL), j, (struct TP_TOOLS *)lParam);
		SetLvToolIcon(GetDlgItem(hDlg, IDC_LIST_TOOL));
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

	case WM_NOTIFY:
		if(DialogLvNotifyProc(hDlg, lParam, GetDlgItem(hDlg, IDC_LIST_TOOL)) == FALSE){
			return OptionNotifyProc(hDlg, uMsg, wParam, lParam);
		}
		break;

	case WM_LV_EVENT:
		if(wParam == LVN_ITEMCHANGED){
			BOOL enable;

			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_EDIT), FALSE);
			if((SelectItem = ListView_GetNextItem(GetDlgItem(hDlg, IDC_LIST_TOOL), -1, LVNI_SELECTED)) != -1){
				char funcname[BUFSIZE];

				*funcname = '\0';
				ListView_GetItemText(GetDlgItem(hDlg, IDC_LIST_TOOL), SelectItem, 9, funcname, BUFSIZE - 1);
				if(*funcname != '\0'){
					EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_EDIT), TRUE);
				}
			}
			enable = (ListView_GetSelectedCount(GetDlgItem(hDlg, IDC_LIST_TOOL)) <= 0) ? FALSE : TRUE;
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_UP), enable);
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_DOWN), enable);
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_PEDIT), enable);
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_DELETE), enable);
		}
		break;

	case WM_COMMAND:
		switch(wParam)
		{
		//アイテムを上に移動
		case IDC_BUTTON_UP:
			if((SelectItem = ListView_GetNextItem(GetDlgItem(hDlg, IDC_LIST_TOOL), -1, LVNI_SELECTED)) == -1){
				break;
			}
			if(SelectItem == 0){
				break;
			}
			ListView_MoveItem(GetDlgItem(hDlg, IDC_LIST_TOOL), SelectItem, -1, 10);
			SetFocus(GetDlgItem(hDlg, IDC_BUTTON_UP));
			break;

		//アイテムを下に移動
		case IDC_BUTTON_DOWN:
			if((SelectItem = ListView_GetNextItem(GetDlgItem(hDlg, IDC_LIST_TOOL), -1, LVNI_SELECTED)) == -1){
				break;
			}
			if(SelectItem == ListView_GetItemCount(GetDlgItem(hDlg, IDC_LIST_TOOL)) - 1){
				break;
			}
			ListView_MoveItem(GetDlgItem(hDlg, IDC_LIST_TOOL), SelectItem, 1, 10);
			SetFocus(GetDlgItem(hDlg, IDC_BUTTON_DOWN));
			break;

		case IDOK:
			if(ToolList != NULL){
				FreeTool();
			}

			ToolListCnt = ListView_GetItemCount(GetDlgItem(hDlg, IDC_LIST_TOOL));

			ToolList = (struct TP_TOOLS *)GlobalAlloc(GPTR, sizeof(struct TP_TOOLS) * ToolListCnt);
			if(ToolList == NULL){
				abort();
			}
			//リストビューのアイテムをツール情報に設定する
			for(i = 0;i < ToolListCnt;i++){
				ListToToolInfo(GetDlgItem(hDlg, IDC_LIST_TOOL), i, ToolList + i);
				if(str_match("*.dll", ToolList[i].FileName) == TRUE){
					ToolList[i].lib = LoadLibrary(ToolList[i].FileName);
				}
			}
		case IDPCANCEL:
			ImageList_Destroy((void *)ListView_GetImageList(GetDlgItem(hDlg, IDC_LIST_TOOL), LVSIL_SMALL));
			ToolWnd = NULL;
			break;

		//追加
		case IDC_BUTTON_ADD:
			tpSetTool = (struct TP_TOOLS *)GlobalAlloc(GPTR, sizeof(struct TP_TOOLS));
			if(tpSetTool == NULL){
				ErrMsg(hDlg, GetLastError(), NULL);
				break;
			}
			tpSetTool->iSize = sizeof(struct TP_TOOLS);
			tpSetTool->MenuIndex = -1;

			if(DialogBoxParam(g_hinst, MAKEINTRESOURCE(IDD_DIALOG_TOOL_EXE), hDlg, SetToolExeProc, (LPARAM)tpSetTool) == TRUE){
				j = ListView_InsertItemEx(GetDlgItem(hDlg, IDC_LIST_TOOL), "",
					ListView_GetItemCount(GetDlgItem(hDlg, IDC_LIST_TOOL)), 0, -1);
				ToolInfoToList(GetDlgItem(hDlg, IDC_LIST_TOOL), j, tpSetTool);
				SetLvToolIcon(GetDlgItem(hDlg, IDC_LIST_TOOL));
			}
			GlobalFree(tpSetTool);
			break;

		//編集
		case IDC_BUTTON_PEDIT:
			if((SelectItem = ListView_GetNextItem(GetDlgItem(hDlg, IDC_LIST_TOOL), -1, LVNI_SELECTED)) == -1){
				break;
			}
			tpSetTool = (struct TP_TOOLS *)GlobalAlloc(GPTR, sizeof(struct TP_TOOLS));
			if(tpSetTool == NULL){
				ErrMsg(hDlg, GetLastError(), NULL);
				break;
			}
			ListToToolInfo(GetDlgItem(hDlg, IDC_LIST_TOOL), SelectItem, tpSetTool);
			if(DialogBoxParam(g_hinst, MAKEINTRESOURCE(IDD_DIALOG_TOOL_EXE), hDlg, SetToolExeProc, (LPARAM)tpSetTool) == TRUE){
				ToolInfoToList(GetDlgItem(hDlg, IDC_LIST_TOOL), SelectItem, tpSetTool);
				SetLvToolIcon(GetDlgItem(hDlg, IDC_LIST_TOOL));
			}
			GlobalFree(tpSetTool);
			break;

		//削除
		case IDC_BUTTON_DELETE:
			if((SelectItem = ListView_GetNextItem(GetDlgItem(hDlg, IDC_LIST_TOOL), -1, LVNI_SELECTED)) == -1){
				break;
			}
			if(MessageBox(hDlg, QMSG_DELETE, QMSG_DELETE_TITLE,
				MB_ICONQUESTION | MB_YESNO) == IDNO){
				break;
			}
			ListView_DeleteItem(GetDlgItem(hDlg, IDC_LIST_TOOL), SelectItem);
			ListView_SetItemState(GetDlgItem(hDlg, IDC_LIST_TOOL), SelectItem,
				LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
			break;

		//プロパティ
		case IDC_BUTTON_EDIT:
			if((SelectItem = ListView_GetNextItem(GetDlgItem(hDlg, IDC_LIST_TOOL), -1, LVNI_SELECTED)) != -1){
				char fname[BUFSIZE], funcname[BUFSIZE];

				*fname = '\0';
				*funcname = '\0';
				ListView_GetItemText(GetDlgItem(hDlg, IDC_LIST_TOOL), SelectItem, 1, fname, BUFSIZE - 1);
				ListView_GetItemText(GetDlgItem(hDlg, IDC_LIST_TOOL), SelectItem, 9, funcname, BUFSIZE - 1);

				if(*funcname == '\0'){
					//関数が無い場合は編集ボタンを押したことにする
					SendMessage(hDlg, WM_COMMAND, IDC_BUTTON_PEDIT, 0);
					break;
				}
				//DLL内の関数をプロパティモードで呼ぶ
				if(DllToolProp(hDlg, fname, funcname) == 0){
					MessageBox(hDlg, EMSG_PROP, EMSG_PROP_TITLE, MB_OK | MB_ICONEXCLAMATION);
				}
			}
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
