/**************************************************************************

	WWWC (wwwc.dll)

	main.c

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
#include <winsock.h>
#include <commctrl.h>
#include <richedit.h>

#include "String.h"
#include "httpfilter.h"
#include "http.h"
#include "StrTbl.h"
#include "wwwcdll.h"
#include "archive.h"

#include "resource.h"


/**************************************************************************
	Define
**************************************************************************/


/**************************************************************************
	Global Variables
**************************************************************************/

char app_path[BUFSIZE];

HINSTANCE ghinst;
HANDLE hRTFLibdll = NULL;

int CheckType;
int TimeOut;

int Proxy;
char pServer[BUFSIZE];
int pPort;
int pNoCache;
int pUsePass;
char pUser[BUFSIZE];
char pPass[BUFSIZE];

char BrowserPath[BUFSIZE];
int TimeZone;
int HostNoCache;
int PassProtect;
int ArchiveGlobal;

char DateFormat[BUFSIZE];
char TimeFormat[BUFSIZE];


extern FILTER *FilterInfo;


/******************************************************************************

	DllInitialize

	初期化

******************************************************************************/

__declspec(dllexport) int CALLBACK DllInitialize(void)
{
	char *p, *r;
	char CuDir[BUFSIZE];
	char buf[BUFSIZE];
	char pbuf[PASS_BUFSIZE];

	GetModuleFileName(ghinst, CuDir, BUFSIZE - 1);
	p = r = CuDir;
	while(*p != '\0'){
		if(IS_LEAD_BYTE(p)){
			p++;
		}else if(*p == '\\' || *p == '/'){
			r = p;
		}
		p++;
	}
	*r = '\0';
	if(lstrlen(CuDir) + lstrlen("\\wwwc.ini") >= BUFSIZE){
		//パスが長すぎる場合は設定ファイルを使用しない
		*app_path = '\0';
	}else{
		wsprintf(app_path, "%s\\wwwc.ini", CuDir);
	}

	GetPrivateProfileString("FORMAT", "DateFormat", "yyyy/MM/dd", DateFormat, BUFSIZE - 1, app_path);
	GetPrivateProfileString("FORMAT", "TimeFormat", "HH:mm", TimeFormat, BUFSIZE - 1, app_path);
	TimeZone = GetPrivateProfileInt("HTTP", "TimeZone", 9, app_path);
	HostNoCache = GetPrivateProfileInt("HTTP", "HostNoCache", 0, app_path);
	//パスワードを Windows のユーザに紐付けて暗号化して保存する (0 で従来の形式)
	PassProtect = GetPrivateProfileInt("HTTP", "PassProtect", 1, app_path);
	//ページのアーカイブ (0 ですべてのアイテムでアーカイブしない)
	ArchiveGlobal = GetPrivateProfileInt("ARCHIVE", "Enable", 1, app_path);

	WritePrivateProfileString("FORMAT", "DateFormat", DateFormat, app_path);
	WritePrivateProfileString("FORMAT", "TimeFormat", TimeFormat, app_path);
	wsprintf(buf, "%ld", TimeZone);
	WritePrivateProfileString("HTTP", "TimeZone", buf, app_path);
	wsprintf(buf, "%ld", HostNoCache);
	WritePrivateProfileString("HTTP", "HostNoCache", buf, app_path);
	wsprintf(buf, "%ld", PassProtect);
	WritePrivateProfileString("HTTP", "PassProtect", buf, app_path);
	wsprintf(buf, "%ld", ArchiveGlobal);
	WritePrivateProfileString("ARCHIVE", "Enable", buf, app_path);

	CheckType = GetPrivateProfileInt("CHECK", "CheckType", 0, app_path);
	TimeOut = GetPrivateProfileInt("CHECK", "TimeOut", 60, app_path);

	Proxy = GetPrivateProfileInt("PROXY", "Proxy", 0, app_path);
	GetPrivateProfileString("PROXY", "pServer", "", pServer, BUFSIZE - 1, app_path);
	pPort = GetPrivateProfileInt("PROXY", "pPort", 0, app_path);
	pNoCache = GetPrivateProfileInt("PROXY", "pNoCache", 1, app_path);
	pUsePass = GetPrivateProfileInt("PROXY", "pUsePass", 0, app_path);
	GetPrivateProfileString("PROXY", "pUser", "", pUser, BUFSIZE - 1, app_path);
	GetPrivateProfileString("PROXY", "pPass", "", pbuf, PASS_BUFSIZE - 1, app_path);
	DecodePassword(pbuf, pPass, BUFSIZE);
	SecureZeroMemory(pbuf, sizeof(pbuf));

	GetPrivateProfileString("HTTP", "BrowserPath", "", BrowserPath, BUFSIZE - 1, app_path);

	if(FilterInfo == NULL){
		ReadFilterFile(CuDir);
	}
	Archive_Init();
	return 0;
}


/******************************************************************************

	DllMain

	メイン

******************************************************************************/

int WINAPI DllMain(HINSTANCE hInst, DWORD fdwReason, PVOID pvReserved)
{
    switch(fdwReason)
    {
    case DLL_PROCESS_ATTACH:
		ghinst = hInst;

		InitCommonControls();
		DllInitialize();

		if(hRTFLibdll == NULL){
			hRTFLibdll = LoadLibrary("RICHED32.DLL");
		}
        break;

    case DLL_PROCESS_DETACH:
		if(hRTFLibdll != NULL){
			FreeLibrary(hRTFLibdll);
			hRTFLibdll = NULL;
		}
		FreeFilter();
        break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }
	return TRUE;
}



/******************************************************************************

	GetProtocolList

	プロトコル設定

******************************************************************************/

__declspec(dllexport) int CALLBACK GetProtocolList(int cnt, struct TPPROTOCOLSET *tpProtocolSet)
{
	int ret = 0;

	switch(cnt)
	{
	case 0:
		lstrcpy(tpProtocolSet->Title, "HTTP");
		lstrcpy(tpProtocolSet->FuncHeader, "HTTP_");
		lstrcpy(tpProtocolSet->IconFile, "");
		tpProtocolSet->IconIndex = 0;
		lstrcpy(tpProtocolSet->UpIconFile, "");
		tpProtocolSet->UpIconIndex = -1;
		break;

	case 1:
		lstrcpy(tpProtocolSet->Title, "FILE");
		lstrcpy(tpProtocolSet->FuncHeader, "FILE_");
		lstrcpy(tpProtocolSet->IconFile, "");
		tpProtocolSet->IconIndex = 1;
		lstrcpy(tpProtocolSet->UpIconFile, "");
		tpProtocolSet->UpIconIndex = -1;
		break;

	default:
		ret = -1;
		break;
	}
	return ret;
}


/******************************************************************************

	GetToolList

	ツール設定

******************************************************************************/

__declspec(dllexport) int CALLBACK GetToolList(int cnt, struct TP_TOOLS *tpToolInfo)
{
	int ret = 0;

	switch(cnt)
	{
	case 0:
		lstrcpy(tpToolInfo->title, STR_TOOL_TITLE_BROWSINFO);
		tpToolInfo->Action = TOOL_EXEC_WINDOWMENU;
		lstrcpy(tpToolInfo->func, "GetBrowserInfo");
		break;

	case 1:
		lstrcpy(tpToolInfo->title, STR_TOOL_TITLE_BROWSINFOLIST);
		tpToolInfo->Action = TOOL_EXEC_WINDOWMENU;
		lstrcpy(tpToolInfo->func, "GetBrowserInfoList");
		break;

	case 2:
		lstrcpy(tpToolInfo->title, STR_TOOL_TITLE_OPENURL);
		tpToolInfo->Action = TOOL_EXEC_ITEMMENU | TOOL_EXEC_INITITEM;
		tpToolInfo->MenuIndex = 2;
		lstrcpy(tpToolInfo->Protocol, "HTTP");
		lstrcpy(tpToolInfo->func, "DDE_OpenURL");
		break;

	case 3:
		lstrcpy(tpToolInfo->title, STR_TOOL_TITLE_FILTERRELOAD);
		tpToolInfo->Action = TOOL_EXEC_CHECKSTART | TOOL_EXEC_WINDOWMENU;
		lstrcpy(tpToolInfo->func, "FilterReload");
		break;

	default:
		ret = -1;
		break;
	}
	return ret;
}


/******************************************************************************

	GetToolHideOption

	ツール設定で非表示にするオプションの取得 (戻り値 TOOL_HIDE_ の組み合わせ)

******************************************************************************/

__declspec(dllexport) int CALLBACK GetToolHideOption(char *FuncName)
{
	return 0;
}


/******************************************************************************

	ErrMsg

	エラーメッセージの表示

******************************************************************************/

void ErrMsg(HWND hWnd, int ErrCode, char *Title)
{
	char *ErrStr = NULL, *p;
	char buf[BUFSIZE];

	p = (Title == NULL) ? "Error" : Title;

	//エラーコードから文字列を取得
	if(FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, ErrCode, 0, (LPTSTR)&ErrStr, BUFSIZE - 1, NULL) == 0 || ErrStr == NULL){
		wsprintf(buf, "Error (%d)", ErrCode);
		MessageBox(hWnd, buf, p, MB_ICONEXCLAMATION);
		return;
	}

	//エラーメッセージを表示
	MessageBox(hWnd, ErrStr, p, MB_ICONEXCLAMATION);
	LocalFree(ErrStr);
}


/******************************************************************************

	ExecItem

	ファイル,URL の実行

******************************************************************************/

int ExecItem(HWND hWnd, char *buf, char *cmdline)
{
	int ret;

	ret = (int)ShellExecute((HWND)NULL, (LPCTSTR)NULL, (LPCTSTR)buf, cmdline, NULL, SW_SHOWNORMAL);
	if(ret <= 32){			//エラーの場合は、エラーの文字列を取得してメッセージボックスを表示する
		ret = GetLastError();
		ErrMsg(hWnd, ret, STR_ERR_TITLE_OPEN);
		return -1;
	}
	return 1;
}


/******************************************************************************

	ConfirmOpenURL

	http, https 以外のURLを開く場合は確認を行う

******************************************************************************/

BOOL ConfirmOpenURL(HWND hWnd, char *URL)
{
	char *msg;
	char *p;
	BOOL ret;

	if(URL == NULL || *URL == '\0'){
		return FALSE;
	}
	if(IsWebURL(URL) == TRUE){
		return TRUE;
	}
	//引用符を含むURLは引数を分割されるため開かない
	for(p = URL; *p != '\0'; p++){
		if(*p == '\"'){
			MessageBox(hWnd, STR_ERR_MSG_URLQUOTE, STR_ERR_TITLE_URLOPEN, MB_ICONEXCLAMATION);
			return FALSE;
		}
	}
	msg = (char *)GlobalAlloc(GMEM_FIXED, lstrlen(STR_Q_MSG_OPENURL) + lstrlen(URL) + 1);
	if(msg == NULL){
		return FALSE;
	}
	wsprintf(msg, STR_Q_MSG_OPENURL, URL);
	ret = (MessageBox(hWnd, msg, STR_ERR_TITLE_URLOPEN, MB_ICONQUESTION | MB_YESNO) == IDYES) ? TRUE : FALSE;
	GlobalFree(msg);
	return ret;
}


/******************************************************************************

	FileSelect

	ファイルの選択

******************************************************************************/

int FileSelect(HWND hDlg,char *oFile,char *oFilter,char *oTitle,char *ret,char *def,int Index)
{
	OPENFILENAME of;
	char tmp[BUFSIZE];

	lstrcpy(tmp, oFile);

	of.lStructSize = sizeof(of);
	of.hInstance = NULL;
	of.hwndOwner = hDlg;
	of.lpstrFilter = oFilter;
	of.lpstrTitle = oTitle;
	of.lpstrCustomFilter = NULL;
	of.nMaxCustFilter = 40;
	of.nFilterIndex = Index;
	of.lpstrFile = tmp;
	of.nMaxFile = BUFSIZE - 1;
	of.lpstrFileTitle = NULL;
	of.nMaxFileTitle = 0;
	of.lpstrInitialDir = NULL;
	of.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	of.nFileOffset = 0;
	of.nFileExtension = 0;
	of.lpstrDefExt = def;
	of.lCustData = 0;
	of.lpfnHook = NULL;
	of.lpTemplateName = NULL;

	if(GetOpenFileName((LPOPENFILENAME)&of) == TRUE){
		lstrcpy(ret,of.lpstrFile);
		return of.nFilterIndex;
	}
	return -1;
}
/* End of source */
