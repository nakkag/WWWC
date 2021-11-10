/**************************************************************************

	WWWC (wwwc.dll)

	http.h

	Copyright (C) 1996-2018 by Ohno Tomoaki. All rights reserved.
		https://www.nakka.com/
		nakka@nakka.com

**************************************************************************/

#ifndef _INC_HTTP_H
#define _INC_HTTP_H

/**************************************************************************
	Define
**************************************************************************/

#define WM_GETBROUSERINFO				(WM_USER + 1)

#define IDC_EDIT_VIEW					100

#define IDPCANCEL						(WM_USER + 203)

#define ID_KEY_RETURN					40100
#define ID_KEY_TAB						40009
#define ID_KEY_ESC						40101
#define ID_MENUITEM_COPY				40056
#define ID_MENUITEM_ALLSELECT			40057

#define BUFSIZE							256
#define MAXSIZE							32768
#define RECVSIZE						32768				/* 受信サイズ */

#define OP1_REQTYPE						0
#define OP1_NODATE						1
#define OP1_NOSIZE						2
#define OP1_NOTAGSIZE					3
#define OP1_META						4
#define OP1_TYPE						5
#define OP1_NAME						6
#define OP1_CONTENT						7
#define OP1_MD5							8

#define OP2_NOPROXY						0
#define OP2_SETPROXY					1
#define OP2_PROXY						2
#define OP2_PORT						3
#define OP2_USEPASS						4
#define OP2_USER						5
#define OP2_PASS						6
#define OP2_REFERER						7

#define LVM_FIRST						0x1000      // ListView messages
#define LVM_SETEXTENDEDLISTVIEWSTYLE	(LVM_FIRST + 54)
#define LVM_GETEXTENDEDLISTVIEWSTYLE	(LVM_FIRST + 55)
#define LVS_EX_TRACKSELECT				0x00000008
#define LVS_EX_FULLROWSELECT			0x00000020 // applies to report mode only
#define LVS_EX_ONECLICKACTIVATE			0x00000040


/**************************************************************************
	Struct
**************************************************************************/

struct TPHTTP{
	HWND hWnd;
	HANDLE hThread;

	char *SvName;						/* サーバ名 */
	char *Path;							/* パス */
	int Port;							/* ポート番号 */

	char *hSvName;						/* HOSTサーバ名 */
	int hPort;							/* HOSTポート番号 */

	char *buf;
	long Size;
	BOOL HeadCheckFlag;
	BOOL FilterFlag;
	int Status;

	char *user;
	char *pass;
};

struct TPGETHOSTBYNAME{
	HWND hWnd;
	UINT wMsg;
	char *name;
	HANDLE hth;
	BOOL EventFlag;
};

struct TPHOSTINFO {
	char *HostName;
	UINT hash;
	unsigned long addr;
};


/**************************************************************************
	Function Prototypes
**************************************************************************/

void ErrMsg(HWND hWnd, int ErrCode, char *Title);
int ExecItem(HWND hWnd, char *buf, char* cmdline);
int FileSelect(HWND hDlg,char *oFile,char *oFilter,char *oTitle,char *ret,char *def,int Index);

#endif
/* End of source */
