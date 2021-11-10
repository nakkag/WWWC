/**************************************************************************

	WWWC (wwwc.dll)

	main.c

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
#include <winsock.h>
#include <commctrl.h>
#include <richedit.h>

#include "String.h"
#include "httpfilter.h"
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

char DateFormat[BUFSIZE];
char TimeFormat[BUFSIZE];

char AppName[30][BUFSIZE];
int AppCnt;

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
	int i;

	GetModuleFileName(ghinst, CuDir, BUFSIZE - 1);
	p = r = CuDir;
	while(*p != '\0'){
		if(IsDBCSLeadByte((BYTE)*p) == TRUE){
			p++;
		}else if(*p == '\\' || *p == '/'){
			r = p;
		}
		p++;
	}
	*r = '\0';
	wsprintf(app_path, "%s\\wwwc.ini", CuDir);

	GetPrivateProfileString("FORMAT", "DateFormat", "yyyy/MM/dd", DateFormat, BUFSIZE - 1, app_path);
	GetPrivateProfileString("FORMAT", "TimeFormat", "HH:mm", TimeFormat, BUFSIZE - 1, app_path);
	TimeZone = GetPrivateProfileInt("HTTP", "TimeZone", 9, app_path);
	HostNoCache = GetPrivateProfileInt("HTTP", "HostNoCache", 0, app_path);

	WritePrivateProfileString("FORMAT", "DateFormat", DateFormat, app_path);
	WritePrivateProfileString("FORMAT", "TimeFormat", TimeFormat, app_path);
	wsprintf(buf, "%ld", TimeZone);
	WritePrivateProfileString("HTTP", "TimeZone", buf, app_path);
	wsprintf(buf, "%ld", HostNoCache);
	WritePrivateProfileString("HTTP", "HostNoCache", buf, app_path);

	CheckType = GetPrivateProfileInt("CHECK", "CheckType", 0, app_path);
	TimeOut = GetPrivateProfileInt("CHECK", "TimeOut", 60, app_path);

	Proxy = GetPrivateProfileInt("PROXY", "Proxy", 0, app_path);
	GetPrivateProfileString("PROXY", "pServer", "", pServer, BUFSIZE - 1, app_path);
	pPort = GetPrivateProfileInt("PROXY", "pPort", 0, app_path);
	pNoCache = GetPrivateProfileInt("PROXY", "pNoCache", 1, app_path);
	pUsePass = GetPrivateProfileInt("PROXY", "pUsePass", 0, app_path);
	GetPrivateProfileString("PROXY", "pUser", "", pUser, BUFSIZE - 1, app_path);
	GetPrivateProfileString("PROXY", "pPass", "", buf, BUFSIZE - 1, app_path);
	dPass(buf,pPass);

	GetPrivateProfileString("HTTP", "BrowserPath", "", BrowserPath, BUFSIZE - 1, app_path);

	AppCnt = GetPrivateProfileInt("DDE", "AppCnt", -1, app_path);
	if(AppCnt < 0){
		AppCnt = 4;
		lstrcpy(AppName[0], "IEXPLORE");
		lstrcpy(AppName[1], "Mozilla");
		lstrcpy(AppName[2], "Netscape6");
		lstrcpy(AppName[3], "NETSCAPE");
	}else{
		if(AppCnt > 30) AppCnt = 30;
		for(i = 0; i < AppCnt; i++){
			wsprintf(buf, "AppName_%d", i);
			GetPrivateProfileString("DDE", buf, "", AppName[i], BUFSIZE - 1, app_path);
		}
	}

	if(FilterInfo == NULL){
		ReadFilterFile(CuDir);
	}
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
	char *ErrStr, *p;

	p = (Title == NULL) ? "Error" : Title;

	//エラーコードから文字列を取得
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, ErrCode, 0, (LPTSTR)&ErrStr, BUFSIZE - 1, NULL);

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
