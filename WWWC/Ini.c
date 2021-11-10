/**************************************************************************

	WWWC

	Ini.c

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
#include "Profile.h"


/**************************************************************************
	Define
**************************************************************************/

#ifndef LVS_EX_INFOTIP
#define LVS_EX_INFOTIP          0x00000400
#endif


/**************************************************************************
	Global Variables
**************************************************************************/

//ini
char DateFormat[BUFSIZE];
char TimeFormat[BUFSIZE];

RECT WinRect;
int SEPSIZE;
int DoubleStart;
int DoubleStartMsg;
int TrayIcon;
int TrayIconMode;
int TrayIconToggle;
int TrayIconClose;
int TrayIconMin;
int TrayWinShow;
int TrayHotKeyMod;
int TrayHotKeyVk;
char TrayHotKeyVkey[BUFSIZE];
char WinTitle[BUFSIZE];
int EndRecyclerClear;
int TitleView;
char ListFontName[BUFSIZE];
int ListFontSize;
int ListFontCharSet;

char ViewTbItem[BUFSIZE];
int ViewTb;
char AniIcon[BUFSIZE];
int TbStyle;

int ViewSb;
int PartsSize1;
int PartsSize2;
int PartInfo1;
int PartInfo2;
int PartInfo3;

char RootTitle[BUFSIZE];
char RecyclerTitle[BUFSIZE];
int StartExpand;
int StartDirOpen;
char StartDirPath[BUFSIZE];
char LastDirPath[BUFSIZE];
int SucceedFromParent;
int TvWndStyle;
char TvBkColor[BUFSIZE];
char TvTextColor[BUFSIZE];
int TvIconSize;

int LvStyle;
int lvExStyle;
int LvSortFlag;
int LvAutoSort;
int LvCheckEndAutoSort;
int LvDblclkUpDir;
int LvSpaceNextFocus;
char LvColumn[BUFSIZE];
int LvColSize[100];
int LvColCnt;
int LvWndStyle;
char LvBkColor[BUFSIZE];
char LvTextBkColor[BUFSIZE];
char LvTextColor[BUFSIZE];
int LvLIconSize;
int LvSIconSize;

char Check[BUFSIZE];
int CheckIndex;
char NoCheck[BUFSIZE];
int NoCheckIndex;
char DirUP[BUFSIZE];
int DirUPIndex;
char Dir[BUFSIZE];
int DirIndex;
char DirUPchild[BUFSIZE];
int DirUPchildIndex;
char CheckChild[BUFSIZE];
int CheckChildIndex;
char Recycler[BUFSIZE];
int RecyclerIndex;
char RecyclerFull[BUFSIZE];
int RecyclerFullIndex;

char Inet[BUFSIZE];
int InetIndex;
char DirOpen[BUFSIZE];
int DirOpenIndex;

char Up[BUFSIZE];
int UpIndex;
char Error[BUFSIZE];
int ErrorIndex;
char TimeOut[BUFSIZE];
int TimeOutIndex;
char NoProtocol[BUFSIZE];
int NoProtocolIndex;
char Wait[BUFSIZE];
int WaitIndex;

int UPSnd;
char WaveFile[BUFSIZE];
int UPMsg;
int NoUpMsg;
int CheckUPItemClear;
int CheckUPItemAutoSort;
int ClearTime;
int UPMsgTop;
int UPActive;
int UPAni;
int UPMsgExpand;
int UPWinLeft;
int UPWinTop;
int UPWinRight;
int UPWinBottom;
int UPWinPosSave;
int UPWinSizeSave;
int UPWinExpandCenter;
int LvUPExStyle;
int LvUPSortFlag;
int LvUPColSize[100];
int LvUPColCnt;
char LvUPColumn[BUFSIZE];

int FindSubFolder;
int FindNoCheck;
int FindItemFlag;
int FindIconFlag;
int FindWinLeft;
int FindWinTop;
int FindWinRight;
int FindWinBottom;
int LvFindExStyle;
int LvFindColSize[100];
int LvFindColCnt;
char LvFindColumn[BUFSIZE];

int CheckMax;
int ReturnIcon;
int EndCheckMsg;

int AutoCheck;
int StartCheck;
int NoMinuteCheck;

int OpenReturnIcon;
int DnDReturnIcon;
int AllInitMsg;
int SaveDriveCheck;
int DefNoCheck;

//外部参照
extern int gCheckFlag;
extern char CuDir[];
extern struct TPLVCOLUMN *ColumnInfo;
extern struct TPLVCOLUMN *upColumnInfo;
extern struct TPLVCOLUMN *FindColumnInfo;
extern struct TP_PROTOCOL *tpProtocol;
extern int ProtocolCnt;
extern struct TP_TOOLS *ToolList;
extern int ToolListCnt;
extern struct TPCHECKTIME *tpCheckTime;
extern int tpCheckTimeCnt;


/******************************************************************************

	GetINI

	INIファイルから設定情報を読みこむ

******************************************************************************/

void GetINI(void)
{
	RECT DeskTopRect;
	char app_path[BUFSIZE];
	char buf[BUFSIZE];
	char tmp[BUFSIZE];
	char *p, *r;
	int i;

	wsprintf(app_path, "%s\\%s", CuDir, USER_INI);
	profile_initialize(app_path, TRUE);

	profile_get_string("GENERAL", "DateFormat", "yyyy/MM/dd", DateFormat, BUFSIZE - 1, app_path);
	profile_get_string("GENERAL", "TimeFormat", "HH:mm", TimeFormat, BUFSIZE - 1, app_path);

	GetWindowRect(GetDesktopWindow(), &DeskTopRect);

	WinRect.left = profile_get_int("WINDOW", "left", 0, app_path);
	WinRect.top = profile_get_int("WINDOW", "top", 0, app_path);
	WinRect.right = profile_get_int("WINDOW", "right", 500, app_path);
	WinRect.bottom = profile_get_int("WINDOW", "bottom", 300, app_path);
	SEPSIZE = profile_get_int("WINDOW", "SEPSIZE", 150, app_path);
	if(SEPSIZE < 1 || (WinRect.right > 0 && SEPSIZE > WinRect.right)){
		SEPSIZE = 150;
	}
	DoubleStart = profile_get_int("WINDOW", "DoubleStart", 1, app_path);
	DoubleStartMsg = profile_get_int("WINDOW", "DoubleStartMsg", 0, app_path);
	TrayIcon = profile_get_int("WINDOW", "TrayIcon", 0, app_path);
	TrayIconMode = profile_get_int("WINDOW", "TrayIconMode", 0, app_path);
	TrayIconToggle = profile_get_int("WINDOW", "TrayIconToggle", 0, app_path);
	TrayIconClose = profile_get_int("WINDOW", "TrayIconClose", 0, app_path);
	TrayIconMin = profile_get_int("WINDOW", "TrayIconMin", 1, app_path);
	TrayWinShow = profile_get_int("WINDOW", "TrayWinShow", 0, app_path);
	TrayHotKeyMod = profile_get_int("WINDOW", "TrayHotKeyMod", 0, app_path);
	profile_get_string("WINDOW", "TrayHotKeyVkey", "", TrayHotKeyVkey, BUFSIZE - 1, app_path);
	TrayHotKeyVk = *TrayHotKeyVkey;

	profile_get_string("WINDOW", "WinTitle", "", WinTitle, BUFSIZE - 1, app_path);
	EndRecyclerClear = profile_get_int("WINDOW", "EndRecyclerClear", 0, app_path);
	TitleView = profile_get_int("WINDOW", "TitleView", 1, app_path);
	profile_get_string("WINDOW", "ListFontName", "", ListFontName, BUFSIZE - 1, app_path);
	ListFontSize = profile_get_int("WINDOW", "ListFontSize", 9, app_path);
	ListFontCharSet = profile_get_int("WINDOW", "ListFontCharSet", 1, app_path);

	profile_get_string("TOOLBAR", "ViewTbItem", TBDEFBUTTON, ViewTbItem, BUFSIZE - 1, app_path);
	ViewTb = profile_get_int("TOOLBAR", "ViewTb", 1, app_path);
	profile_get_string("TOOLBAR", "AniIcon", "", AniIcon, BUFSIZE - 1, app_path);
	TbStyle = profile_get_int("TOOLBAR", "TbStyle", TBSTYLE_FLAT | TBSTYLE_ALTDRAG | CCS_ADJUSTABLE, app_path);

	ViewSb = profile_get_int("STATUSBAR", "ViewSb", 1, app_path);
	PartsSize1 = profile_get_int("STATUSBAR", "PartsSize1", 150, app_path);
	PartsSize2 = profile_get_int("STATUSBAR", "PartsSize2", 200, app_path);
	PartInfo1 = profile_get_int("STATUSBAR", "PartInfo1", 1, app_path);
	PartInfo2 = profile_get_int("STATUSBAR", "PartInfo2", 2, app_path);
	PartInfo3 = profile_get_int("STATUSBAR", "PartInfo3", 3, app_path);

	profile_get_string("TREEVIEW", "RootTitle", "Internet", RootTitle, BUFSIZE - 1, app_path);
	profile_get_string("TREEVIEW", "RecyclerTitle", "ごみ箱", RecyclerTitle, BUFSIZE - 1, app_path);
	StartExpand = profile_get_int("TREEVIEW", "StartExpand", 0, app_path);
	StartDirOpen = profile_get_int("TREEVIEW", "StartDirOpen", 0, app_path);
	profile_get_string("TREEVIEW", "StartDirPath", "", StartDirPath, BUFSIZE - 1, app_path);
	profile_get_string("TREEVIEW", "LastDirPath", "", LastDirPath, BUFSIZE - 1, app_path);
	SucceedFromParent = profile_get_int("TREEVIEW", "SucceedFromParent", 1, app_path);
	TvWndStyle = profile_get_int("TREEVIEW", "TvWndStyle", TVS_HASLINES | TVS_SHOWSELALWAYS | TVS_HASBUTTONS | TVS_EDITLABELS, app_path);
	profile_get_string("TREEVIEW", "TvBkColor", "", TvBkColor, BUFSIZE - 1, app_path);
	profile_get_string("TREEVIEW", "TvTextColor", "", TvTextColor, BUFSIZE - 1, app_path);
	TvIconSize = profile_get_int("TREEVIEW", "TvIconSize", SICONSIZE, app_path);

	LvStyle = profile_get_int("LISTVIEW", "LvStyle", 1, app_path);
	LvSortFlag = profile_get_int("LISTVIEW", "LvSortFlag", 1, app_path);
	LvAutoSort = profile_get_int("LISTVIEW", "LvAutoSort", 0, app_path);
	LvCheckEndAutoSort = profile_get_int("LISTVIEW", "LvCheckEndAutoSort", 1, app_path);
	LvDblclkUpDir = profile_get_int("LISTVIEW", "LvDblclkUpDir", 0, app_path);
	LvSpaceNextFocus = profile_get_int("LISTVIEW", "LvSpaceNextFocus", 0, app_path);
	LvWndStyle = profile_get_int("LISTVIEW", "LvWndStyle", LVS_AUTOARRANGE | LVS_SHOWSELALWAYS | LVS_EDITLABELS, app_path);
	lvExStyle = profile_get_int("LISTVIEW", "lvExStyle", LVS_EX_INFOTIP, app_path);
	profile_get_string("LISTVIEW", "LvBkColor", "", LvBkColor, BUFSIZE - 1, app_path);
	profile_get_string("LISTVIEW", "LvTextBkColor", "", LvTextBkColor, BUFSIZE - 1, app_path);
	profile_get_string("LISTVIEW", "LvTextColor", "", LvTextColor, BUFSIZE - 1, app_path);
	LvLIconSize = profile_get_int("LISTVIEW", "LvLIconSize", LICONSIZE, app_path);
	LvSIconSize = profile_get_int("LISTVIEW", "LvSIconSize", SICONSIZE, app_path);
	profile_get_string("LISTVIEW", "LvColumn", "2,3,1,10,", LvColumn, BUFSIZE - 1, app_path);
	ZeroMemory(LvColSize, sizeof(LvColSize));

	*LvColSize = profile_get_int("LISTVIEW", "LvColSize-0", 100, app_path);
	LvColCnt = 1;
	i = 1;
	r = tmp;
	for(p = LvColumn;*p != '\0';p++){
		if(*p == ','){
			LvColCnt++;
			*r = '\0';
			wsprintf(buf, "LvColSize-%d", atoi(tmp));
			*(LvColSize + i) = profile_get_int("LISTVIEW", buf, 100, app_path);

			i++;
			r = tmp;
		}else{
			*(r++) = *p;
		}
	}

	if(ColumnInfo != NULL){
		GlobalFree(ColumnInfo);
	}
	ColumnInfo = (struct TPLVCOLUMN *)GlobalAlloc(GPTR, sizeof(struct TPLVCOLUMN) * LvColCnt);
	if(ColumnInfo == NULL){
		abort();
	}

	profile_get_string("ICON", "Check", "", Check, BUFSIZE - 1, app_path);
	CheckIndex = profile_get_int("ICON", "CheckIndex", 0, app_path);
	profile_get_string("ICON", "NoCheck", "", NoCheck, BUFSIZE - 1, app_path);
	NoCheckIndex = profile_get_int("ICON", "NoCheckIndex", 0, app_path);
	profile_get_string("ICON", "DirUP", "", DirUP, BUFSIZE - 1, app_path);
	DirUPIndex = profile_get_int("ICON", "DirUPIndex", 0, app_path);
	profile_get_string("ICON", "Dir", "", Dir, BUFSIZE - 1, app_path);
	DirIndex = profile_get_int("ICON", "DirIndex", 0, app_path);
	profile_get_string("ICON", "DirUPchild", "", DirUPchild, BUFSIZE - 1, app_path);
	DirUPchildIndex = profile_get_int("ICON", "DirUPchildIndex", 0, app_path);
	profile_get_string("ICON", "CheckChild", "", CheckChild, BUFSIZE - 1, app_path);
	CheckChildIndex = profile_get_int("ICON", "CheckChildIndex", 0, app_path);
	profile_get_string("ICON", "Recycler", "", Recycler, BUFSIZE - 1, app_path);
	RecyclerIndex = profile_get_int("ICON", "RecyclerIndex", 0, app_path);
	profile_get_string("ICON", "RecyclerFull", "", RecyclerFull, BUFSIZE - 1, app_path);
	RecyclerFullIndex = profile_get_int("ICON", "RecyclerFullIndex", 0, app_path);

	profile_get_string("ICON", "Inet", "", Inet, BUFSIZE - 1, app_path);
	InetIndex = profile_get_int("ICON", "InetIndex", 0, app_path);
	profile_get_string("ICON", "DirOpen", "", DirOpen, BUFSIZE - 1, app_path);
	DirOpenIndex = profile_get_int("ICON", "DirOpenIndex", 0, app_path);

	profile_get_string("ICON", "Up", "", Up, BUFSIZE - 1, app_path);
	UpIndex = profile_get_int("ICON", "UpIndex", 0, app_path);
	profile_get_string("ICON", "Error", "", Error, BUFSIZE - 1, app_path);
	ErrorIndex = profile_get_int("ICON", "ErrorIndex", 0, app_path);
	profile_get_string("ICON", "TimeOut", "", TimeOut, BUFSIZE - 1, app_path);
	TimeOutIndex = profile_get_int("ICON", "TimeOutIndex", 0, app_path);
	profile_get_string("ICON", "NoProtocol", "", NoProtocol, BUFSIZE - 1, app_path);
	NoProtocolIndex = profile_get_int("ICON", "NoProtocolIndex", 0, app_path);
	profile_get_string("ICON", "Wait", "", Wait, BUFSIZE - 1, app_path);
	WaitIndex = profile_get_int("ICON", "WaitIndex", 0, app_path);

	UPSnd = profile_get_int("UPMSG", "UPSnd", 1, app_path);
	profile_get_string("UPMSG", "WaveFile", "", WaveFile, BUFSIZE - 1, app_path);
	UPMsg = profile_get_int("UPMSG", "UPMsg", 1, app_path);
	NoUpMsg = profile_get_int("UPMSG", "NoUpMsg", 0, app_path);
	CheckUPItemClear = profile_get_int("UPMSG", "CheckUPItemClear", 1, app_path);
	CheckUPItemAutoSort = profile_get_int("UPMSG", "CheckUPItemAutoSort", 0, app_path);
	ClearTime = profile_get_int("UPMSG", "ClearTime", 1, app_path);
	UPMsgTop = profile_get_int("UPMSG", "UPMsgTop", 0, app_path);
	UPActive = profile_get_int("UPMSG", "UPActive", 0, app_path);
	UPAni = profile_get_int("UPMSG", "UPAni", 1, app_path);
	UPMsgExpand = profile_get_int("UPMSG", "UPMsgExpand", 0, app_path);
	UPWinLeft = profile_get_int("UPMSG", "UPWinLeft", 0, app_path);
	UPWinTop = profile_get_int("UPMSG", "UPWinTop", 0, app_path);
	UPWinRight = profile_get_int("UPMSG", "UPWinRight", 0, app_path);
	UPWinBottom = profile_get_int("UPMSG", "UPWinBottom", 0, app_path);
	UPWinPosSave = profile_get_int("UPMSG", "UPWinPosSave", 0, app_path);
	UPWinSizeSave = profile_get_int("UPMSG", "UPWinSizeSave", 1, app_path);
	UPWinExpandCenter = profile_get_int("UPMSG", "UPWinExpandCenter", 1, app_path);
	LvUPExStyle = profile_get_int("UPMSG", "LvUPExStyle", 0x20 + 0x40 + 0x400, app_path);
	LvUPSortFlag = profile_get_int("UPMSG", "LvUPSortFlag", 1, app_path);
	profile_get_string("UPMSG", "LvUPColumn", "2,3,4,1,11,", LvUPColumn, BUFSIZE - 1, app_path);
	ZeroMemory(LvUPColSize, sizeof(LvUPColSize));

	*LvUPColSize = profile_get_int("UPMSG", "LvUPColSize-0", 100, app_path);
	LvUPColCnt = 1;
	i = 1;
	r = tmp;
	for(p = LvUPColumn;*p != '\0';p++){
		if(*p == ','){
			LvUPColCnt++;
			*r = '\0';
			wsprintf(buf, "LvUPColSize-%d", atoi(tmp));
			*(LvUPColSize + i) = profile_get_int("UPMSG", buf, 100, app_path);

			i++;
			r = tmp;
		}else{
			*(r++) = *p;
		}
	}

	if(upColumnInfo != NULL){
		GlobalFree(upColumnInfo);
	}
	upColumnInfo = (struct TPLVCOLUMN *)GlobalAlloc(GPTR, sizeof(struct TPLVCOLUMN) * LvUPColCnt);
	if(upColumnInfo == NULL){
		abort();
	}

	FindSubFolder = profile_get_int("FIND_ITEM", "FindSubFolder", 1, app_path);
	FindNoCheck = profile_get_int("FIND_ITEM", "FindNoCheck", 0, app_path);
	FindItemFlag = profile_get_int("FIND_ITEM", "FindItemFlag", 31, app_path);
	FindIconFlag = profile_get_int("FIND_ITEM", "FindIconFlag", 31, app_path);
	FindWinLeft = profile_get_int("FIND_ITEM", "FindWinLeft", 0, app_path);
	FindWinTop = profile_get_int("FIND_ITEM", "FindWinTop", 0, app_path);
	FindWinRight = profile_get_int("FIND_ITEM", "FindWinRight", 0, app_path);
	FindWinBottom = profile_get_int("FIND_ITEM", "FindWinBottom", 0, app_path);
	LvFindExStyle = profile_get_int("FIND_ITEM", "LvFindExStyle", 0x20 + 0x40 + 0x400, app_path);
	profile_get_string("FIND_ITEM", "LvFindColumn", "11,2,3,1,7,10,", LvFindColumn, BUFSIZE - 1, app_path);
	ZeroMemory(LvFindColSize, sizeof(LvFindColSize));

	*LvFindColSize = profile_get_int("FIND_ITEM", "LvFindColSize-0", 100, app_path);
	LvFindColCnt = 1;
	i = 1;
	r = tmp;
	for(p = LvFindColumn;*p != '\0';p++){
		if(*p == ','){
			LvFindColCnt++;
			*r = '\0';
			wsprintf(buf, "LvFindColSize-%d", atoi(tmp));
			*(LvFindColSize + i) = profile_get_int("FIND_ITEM", buf, 100, app_path);

			i++;
			r = tmp;
		}else{
			*(r++) = *p;
		}
	}

	if(FindColumnInfo != NULL){
		GlobalFree(FindColumnInfo);
	}
	FindColumnInfo = (struct TPLVCOLUMN *)GlobalAlloc(GPTR, sizeof(struct TPLVCOLUMN) * LvFindColCnt);
	if(FindColumnInfo == NULL){
		abort();
	}

	CheckMax = profile_get_int("CHECK", "CheckMax", 10, app_path);
	if(CheckMax > CHECK_MAX){
		CheckMax = CHECK_MAX;
	}
	if(CheckMax < 1){
		CheckMax = 1;
	}
	ReturnIcon = profile_get_int("CHECK", "ReturnIcon", 0, app_path);
	EndCheckMsg = profile_get_int("CHECK", "EndCheckMsg", 1, app_path);

	AutoCheck = profile_get_int("CHECK", "AutoCheck", 0, app_path);
	StartCheck = profile_get_int("CHECK", "StartCheck", 0, app_path);
	NoMinuteCheck = profile_get_int("CHECK", "NoMinuteCheck", 1, app_path);

	OpenReturnIcon = profile_get_int("ITEM", "OpenReturnIcon", 1, app_path);
	DnDReturnIcon = profile_get_int("ITEM", "DnDReturnIcon", 1, app_path);
	AllInitMsg = profile_get_int("ITEM", "AllInitMsg", 1, app_path);
	SaveDriveCheck = profile_get_int("ITEM", "SaveDriveCheck", 1, app_path);
	DefNoCheck = profile_get_int("ITEM", "DefNoCheck", 0, app_path);

	profile_free();
}


/******************************************************************************

	PutINI

	INIファイルへ設定情報を書き出す

******************************************************************************/

void PutINI(void)
{
	char app_path[BUFSIZE];
	char buf[BUFSIZE];
	char *p, *r;
	char tmp[BUFSIZE];
	int i;

	wsprintf(app_path, "%s\\%s", CuDir, USER_INI);
	profile_initialize(app_path, TRUE);

	profile_write_string("GENERAL", "DateFormat", DateFormat, app_path);
	profile_write_string("GENERAL", "TimeFormat", TimeFormat, app_path);

	profile_write_int("WINDOW", "left", WinRect.left, app_path);
	profile_write_int("WINDOW", "top", WinRect.top, app_path);
	profile_write_int("WINDOW", "right", WinRect.right, app_path);
	profile_write_int("WINDOW", "bottom", WinRect.bottom, app_path);
	profile_write_int("WINDOW", "SEPSIZE", SEPSIZE, app_path);
	profile_write_int("WINDOW", "DoubleStart", DoubleStart, app_path);
	profile_write_int("WINDOW", "DoubleStartMsg", DoubleStartMsg, app_path);
	profile_write_int("WINDOW", "TrayIcon", TrayIcon, app_path);
	profile_write_int("WINDOW", "TrayIconMode", TrayIconMode, app_path);
	profile_write_int("WINDOW", "TrayIconToggle", TrayIconToggle, app_path);
	profile_write_int("WINDOW", "TrayIconClose", TrayIconClose, app_path);
	profile_write_int("WINDOW", "TrayIconMin", TrayIconMin, app_path);
	profile_write_int("WINDOW", "TrayWinShow", TrayWinShow, app_path);
	profile_write_int("WINDOW", "TrayHotKeyMod", TrayHotKeyMod, app_path);
	profile_write_string("WINDOW", "TrayHotKeyVkey", TrayHotKeyVkey, app_path);
	profile_write_string("WINDOW", "WinTitle", WinTitle, app_path);
	profile_write_int("WINDOW", "EndRecyclerClear", EndRecyclerClear, app_path);
	profile_write_int("WINDOW", "TitleView", TitleView, app_path);
	profile_write_string("WINDOW", "ListFontName", ListFontName, app_path);
	profile_write_int("WINDOW", "ListFontSize", ListFontSize, app_path);
	profile_write_int("WINDOW", "ListFontCharSet", ListFontCharSet, app_path);

	profile_write_string("TOOLBAR", "ViewTbItem", ViewTbItem, app_path);
	profile_write_int("TOOLBAR", "ViewTb", ViewTb, app_path);
	profile_write_string("TOOLBAR", "AniIcon", AniIcon, app_path);
	profile_write_int("TOOLBAR", "TbStyle", TbStyle, app_path);

	profile_write_int("STATUSBAR", "ViewSb", ViewSb, app_path);
	profile_write_int("STATUSBAR", "PartsSize1", PartsSize1, app_path);
	profile_write_int("STATUSBAR", "PartsSize2", PartsSize2, app_path);
	profile_write_int("STATUSBAR", "PartInfo1", PartInfo1, app_path);
	profile_write_int("STATUSBAR", "PartInfo2", PartInfo2, app_path);
	profile_write_int("STATUSBAR", "PartInfo3", PartInfo3, app_path);

	profile_write_string("TREEVIEW", "RootTitle", RootTitle, app_path);
	profile_write_string("TREEVIEW", "RecyclerTitle", RecyclerTitle, app_path);
	profile_write_int("TREEVIEW", "StartExpand", StartExpand, app_path);
	profile_write_int("TREEVIEW", "StartDirOpen", StartDirOpen, app_path);
	profile_write_string("TREEVIEW", "StartDirPath", StartDirPath, app_path);
	profile_write_string("TREEVIEW", "LastDirPath", LastDirPath, app_path);
	profile_write_int("TREEVIEW", "SucceedFromParent", SucceedFromParent, app_path);
	profile_write_int("TREEVIEW", "TvWndStyle", TvWndStyle, app_path);
	profile_write_string("TREEVIEW", "TvBkColor", TvBkColor, app_path);
	profile_write_string("TREEVIEW", "TvTextColor", TvTextColor, app_path);
	profile_write_int("TREEVIEW", "TvIconSize", TvIconSize, app_path);

	profile_write_int("LISTVIEW", "LvStyle", LvStyle, app_path);
	profile_write_int("LISTVIEW", "LvSortFlag", LvSortFlag, app_path);
	profile_write_int("LISTVIEW", "LvAutoSort", LvAutoSort, app_path);
	profile_write_int("LISTVIEW", "LvCheckEndAutoSort", LvCheckEndAutoSort, app_path);
	profile_write_int("LISTVIEW", "LvDblclkUpDir", LvDblclkUpDir, app_path);
	profile_write_int("LISTVIEW", "LvSpaceNextFocus", LvSpaceNextFocus, app_path);
	profile_write_int("LISTVIEW", "LvWndStyle", LvWndStyle, app_path);
	profile_write_int("LISTVIEW", "lvExStyle", lvExStyle, app_path);
	profile_write_string("LISTVIEW", "LvBkColor", LvBkColor, app_path);
	profile_write_string("LISTVIEW", "LvTextBkColor", LvTextBkColor, app_path);
	profile_write_string("LISTVIEW", "LvTextColor", LvTextColor, app_path);
	profile_write_int("LISTVIEW", "LvLIconSize", LvLIconSize, app_path);
	profile_write_int("LISTVIEW", "LvSIconSize", LvSIconSize, app_path);
	profile_write_string("LISTVIEW", "LvColumn", LvColumn, app_path);

	profile_write_int("LISTVIEW", "LvColSize-0", *LvColSize, app_path);
	i = 1;
	r = tmp;
	for(p = LvColumn;*p != '\0';p++){
		if(*p == ','){
			*r = '\0';
			wsprintf(buf, "LvColSize-%d", atoi(tmp));
			profile_write_int("LISTVIEW", buf, LvColSize[i], app_path);

			i++;
			r = tmp;
		}else{
			*(r++) = *p;
		}
	}

	profile_write_string("ICON", "Check", Check, app_path);
	profile_write_int("ICON", "CheckIndex", CheckIndex, app_path);
	profile_write_string("ICON", "NoCheck", NoCheck, app_path);
	profile_write_int("ICON", "NoCheckIndex", NoCheckIndex, app_path);
	profile_write_string("ICON", "DirUP", DirUP, app_path);
	profile_write_int("ICON", "DirUPIndex", DirUPIndex, app_path);
	profile_write_string("ICON", "Dir", Dir, app_path);
	profile_write_int("ICON", "DirIndex", DirIndex, app_path);
	profile_write_string("ICON", "DirUPchild", DirUPchild, app_path);
	profile_write_int("ICON", "DirUPchildIndex", DirUPchildIndex, app_path);
	profile_write_string("ICON", "CheckChild", CheckChild, app_path);
	profile_write_int("ICON", "CheckChildIndex", CheckChildIndex, app_path);
	profile_write_string("ICON", "Recycler", Recycler, app_path);
	profile_write_int("ICON", "RecyclerIndex", RecyclerIndex, app_path);
	profile_write_string("ICON", "RecyclerFull", RecyclerFull, app_path);
	profile_write_int("ICON", "RecyclerFullIndex", RecyclerFullIndex, app_path);

	profile_write_string("ICON", "Inet", Inet, app_path);
	profile_write_int("ICON", "InetIndex", InetIndex, app_path);
	profile_write_string("ICON", "DirOpen", DirOpen, app_path);
	profile_write_int("ICON", "DirOpenIndex", DirOpenIndex, app_path);

	profile_write_string("ICON", "Up", Up, app_path);
	profile_write_int("ICON", "UpIndex", UpIndex, app_path);
	profile_write_string("ICON", "Error", Error, app_path);
	profile_write_int("ICON", "ErrorIndex", ErrorIndex, app_path);
	profile_write_string("ICON", "TimeOut", TimeOut, app_path);
	profile_write_int("ICON", "TimeOutIndex", TimeOutIndex, app_path);
	profile_write_string("ICON", "NoProtocol", NoProtocol, app_path);
	profile_write_int("ICON", "NoProtocolIndex", NoProtocolIndex, app_path);
	profile_write_string("ICON", "Wait", Wait, app_path);
	profile_write_int("ICON", "WaitIndex", WaitIndex, app_path);

	profile_write_int("UPMSG", "UPSnd", UPSnd, app_path);
	profile_write_string("UPMSG", "WaveFile", WaveFile, app_path);
	profile_write_int("UPMSG", "UPMsg", UPMsg, app_path);
	profile_write_int("UPMSG", "NoUpMsg", NoUpMsg, app_path);
	profile_write_int("UPMSG", "CheckUPItemClear", CheckUPItemClear, app_path);
	profile_write_int("UPMSG", "CheckUPItemAutoSort", CheckUPItemAutoSort, app_path);
	profile_write_int("UPMSG", "ClearTime", ClearTime, app_path);
	profile_write_int("UPMSG", "UPMsgTop", UPMsgTop, app_path);
	profile_write_int("UPMSG", "UPActive", UPActive, app_path);
	profile_write_int("UPMSG", "UPAni", UPAni, app_path);
	profile_write_int("UPMSG", "UPMsgExpand", UPMsgExpand, app_path);
	profile_write_int("UPMSG", "UPWinLeft", UPWinLeft, app_path);
	profile_write_int("UPMSG", "UPWinTop", UPWinTop, app_path);
	profile_write_int("UPMSG", "UPWinRight", UPWinRight, app_path);
	profile_write_int("UPMSG", "UPWinBottom", UPWinBottom, app_path);
	profile_write_int("UPMSG", "UPWinPosSave", UPWinPosSave, app_path);
	profile_write_int("UPMSG", "UPWinSizeSave", UPWinSizeSave, app_path);
	profile_write_int("UPMSG", "UPWinExpandCenter", UPWinExpandCenter, app_path);
	profile_write_int("UPMSG", "LvUPExStyle", LvUPExStyle, app_path);
	profile_write_int("UPMSG", "LvUPSortFlag", LvUPSortFlag, app_path);
	profile_write_string("UPMSG", "LvUPColumn", LvUPColumn, app_path);

	profile_write_int("UPMSG", "LvUPColSize-0", *LvUPColSize, app_path);
	i = 1;
	r = tmp;
	for(p = LvUPColumn;*p != '\0';p++){
		if(*p == ','){
			*r = '\0';
			wsprintf(buf, "LvUPColSize-%d", atoi(tmp));
			profile_write_int("UPMSG", buf, LvUPColSize[i], app_path);

			i++;
			r = tmp;
		}else{
			*(r++) = *p;
		}
	}

	profile_write_int("FIND_ITEM", "FindSubFolder", FindSubFolder, app_path);
	profile_write_int("FIND_ITEM", "FindNoCheck", FindNoCheck, app_path);
	profile_write_int("FIND_ITEM", "FindItemFlag", FindItemFlag, app_path);
	profile_write_int("FIND_ITEM", "FindIconFlag", FindIconFlag, app_path);
	profile_write_int("FIND_ITEM", "FindWinLeft", FindWinLeft, app_path);
	profile_write_int("FIND_ITEM", "FindWinTop", FindWinTop, app_path);
	profile_write_int("FIND_ITEM", "FindWinRight", FindWinRight, app_path);
	profile_write_int("FIND_ITEM", "FindWinBottom", FindWinBottom, app_path);
	profile_write_int("FIND_ITEM", "LvFindExStyle", LvFindExStyle, app_path);
	profile_write_string("FIND_ITEM", "LvFindColumn", LvFindColumn, app_path);

	profile_write_int("FIND_ITEM", "LvFindColSize-0", *LvFindColSize, app_path);
	i = 1;
	r = tmp;
	for(p = LvFindColumn;*p != '\0';p++){
		if(*p == ','){
			*r = '\0';
			wsprintf(buf, "LvFindColSize-%d", atoi(tmp));
			profile_write_int("FIND_ITEM", buf, LvFindColSize[i], app_path);

			i++;
			r = tmp;
		}else{
			*(r++) = *p;
		}
	}

	profile_write_int("CHECK", "CheckMax", CheckMax, app_path);
	profile_write_int("CHECK", "ReturnIcon", ReturnIcon, app_path);
	profile_write_int("CHECK", "EndCheckMsg", EndCheckMsg, app_path);

	profile_write_int("CHECK", "AutoCheck", AutoCheck, app_path);
	profile_write_int("CHECK", "StartCheck", StartCheck, app_path);
//	profile_write_int("CHECK", "NoMinuteCheck", NoMinuteCheck, app_path);

	profile_write_int("ITEM", "OpenReturnIcon", OpenReturnIcon, app_path);
	profile_write_int("ITEM", "DnDReturnIcon", DnDReturnIcon, app_path);
	profile_write_int("ITEM", "AllInitMsg", AllInitMsg, app_path);
	profile_write_int("ITEM", "SaveDriveCheck", SaveDriveCheck, app_path);
	profile_write_int("ITEM", "DefNoCheck", DefNoCheck, app_path);

	profile_flush(app_path);
	profile_free();
}


/******************************************************************************

	GetProtocolInfo

	INIファイルからプロトコル情報を読みこむ

******************************************************************************/

void GetProtocolInfo(void)
{
	char app_path[BUFSIZE], buf[BUFSIZE];
	int i;

	wsprintf(app_path, "%s\\%s", CuDir, USER_INI);
	profile_initialize(app_path, TRUE);

	ProtocolCnt = profile_get_int("PROTOCOL", "ProtocolCnt", 0, app_path);

	if(tpProtocol != NULL){
		if(gCheckFlag == 0){
			EndProtocolNotify();
		}
		FreeProtocol();
	}
	if(ProtocolCnt == 0){
		ProtocolCnt = 1;
		tpProtocol = (struct TP_PROTOCOL *)GlobalAlloc(GPTR, sizeof(struct TP_PROTOCOL) * ProtocolCnt);
		if(tpProtocol == NULL){
			abort();
		}
		lstrcpy(tpProtocol[0].title, "HTTP");
		lstrcpy(tpProtocol[0].DLL, "wwwc.dll");
		lstrcpy(tpProtocol[0].FuncHeader, "HTTP_");
		lstrcpy(tpProtocol[0].IconFile, "");
		tpProtocol[0].IconIndex = 0;
		tpProtocol[0].UpIconIndex = -1;
		tpProtocol[0].Icon = 0;
		tpProtocol[0].lib = LoadLibrary(tpProtocol[0].DLL);
	}else{
		tpProtocol = (struct TP_PROTOCOL *)GlobalAlloc(GPTR, sizeof(struct TP_PROTOCOL) * ProtocolCnt);
		if(tpProtocol == NULL){
			abort();
		}
		for(i = 0;i < ProtocolCnt;i++){
			wsprintf(buf, "title-%d", i);
			profile_get_string("PROTOCOL", buf, "", tpProtocol[i].title, BUFSIZE - 1, app_path);
			wsprintf(buf, "DLL-%d", i);
			profile_get_string("PROTOCOL", buf, "", tpProtocol[i].DLL, BUFSIZE - 1, app_path);
			wsprintf(buf, "FuncHeader-%d", i);
			profile_get_string("PROTOCOL", buf, "", tpProtocol[i].FuncHeader, BUFSIZE - 1, app_path);
			wsprintf(buf, "IconFile-%d", i);
			profile_get_string("PROTOCOL", buf, "", tpProtocol[i].IconFile, BUFSIZE - 1, app_path);
			wsprintf(buf, "IconIndex-%d", i);
			tpProtocol[i].IconIndex = profile_get_int("PROTOCOL", buf, 0, app_path);
			wsprintf(buf, "UpIconFile-%d", i);
			profile_get_string("PROTOCOL", buf, "", tpProtocol[i].UpIconFile, BUFSIZE - 1, app_path);
			wsprintf(buf, "UpIconIndex-%d", i);
			tpProtocol[i].UpIconIndex = profile_get_int("PROTOCOL", buf, -1, app_path);

			tpProtocol[i].Icon = 0;
		}
	}
	profile_free();
}


/******************************************************************************

	PutProtocolInfo

	INIファイルへプロトコル情報を書き出す

******************************************************************************/

void PutProtocolInfo(void)
{
	char app_path[BUFSIZE], buf[BUFSIZE], ret[BUFSIZE];
	int i;

	wsprintf(app_path, "%s\\%s", CuDir, USER_INI);
	profile_initialize(app_path, TRUE);

	profile_write_string("PROTOCOL", NULL, NULL, app_path);

	wsprintf(ret, "%ld", ProtocolCnt);
	profile_write_string("PROTOCOL", "ProtocolCnt", ret, app_path);

	if(ProtocolCnt != 0){
		for(i = 0;i < ProtocolCnt;i++){
			wsprintf(buf, "title-%d", i);
			profile_write_string("PROTOCOL", buf, tpProtocol[i].title, app_path);
			wsprintf(buf, "DLL-%d", i);
			profile_write_string("PROTOCOL", buf, tpProtocol[i].DLL, app_path);
			wsprintf(buf, "FuncHeader-%d", i);
			profile_write_string("PROTOCOL", buf, tpProtocol[i].FuncHeader, app_path);
			wsprintf(buf, "IconFile-%d", i);
			profile_write_string("PROTOCOL", buf, tpProtocol[i].IconFile, app_path);
			wsprintf(buf, "IconIndex-%d", i);
			profile_write_int("PROTOCOL", buf, tpProtocol[i].IconIndex, app_path);
			wsprintf(buf, "UpIconFile-%d", i);
			profile_write_string("PROTOCOL", buf, tpProtocol[i].UpIconFile, app_path);
			wsprintf(buf, "UpIconIndex-%d", i);
			profile_write_int("PROTOCOL", buf, tpProtocol[i].UpIconIndex, app_path);
		}
	}
	profile_flush(app_path);
	profile_free();
}


/******************************************************************************

	GetAutoTime

	INIファイルから自動チェック情報を読みこむ

******************************************************************************/

void GetAutoTime(void)
{
	char app_path[BUFSIZE], buf[BUFSIZE];
	int i;
	int TimeItemCnt;

	wsprintf(app_path, "%s\\%s", CuDir, USER_INI);
	profile_initialize(app_path, TRUE);

	TimeItemCnt = profile_get_int("AUTOCHECK", "Cnt", 0, app_path);

	if(TimeItemCnt == 0){
		tpCheckTime = NULL;
		tpCheckTimeCnt = 0;
	}else{
		if(tpCheckTime != NULL){
			GlobalFree(tpCheckTime);
			tpCheckTimeCnt = 0;
		}
		tpCheckTime = (struct TPCHECKTIME *)GlobalAlloc(GPTR, sizeof(struct TPCHECKTIME) * TimeItemCnt);
		if(tpCheckTime == NULL){
			abort();
		}
		tpCheckTimeCnt = TimeItemCnt;
		for(i = 0;i < TimeItemCnt;i++){
			wsprintf(buf, "type-%d", i);
			tpCheckTime[i].type = profile_get_int("AUTOCHECK", buf, 0, app_path);

			wsprintf(buf, "flag-%d", i);
			tpCheckTime[i].flag = profile_get_int("AUTOCHECK", buf, 0, app_path);
			wsprintf(buf, "day-%d", i);
			tpCheckTime[i].day = profile_get_int("AUTOCHECK", buf, 0, app_path);
			wsprintf(buf, "week-%d", i);
			tpCheckTime[i].week = profile_get_int("AUTOCHECK", buf, 0, app_path);
			wsprintf(buf, "h-%d", i);
			tpCheckTime[i].h = profile_get_int("AUTOCHECK", buf, 0, app_path);
			wsprintf(buf, "m-%d", i);
			tpCheckTime[i].m = profile_get_int("AUTOCHECK", buf, 0, app_path);
		}
	}
	profile_free();
}


/******************************************************************************

	PutAutoTime

	INIファイルへ自動チェック情報を書き出す

******************************************************************************/

void PutAutoTime(void)
{
	char app_path[BUFSIZE], buf[BUFSIZE];
	int i, j;
	int TimeItemCnt;

	wsprintf(app_path, "%s\\%s", CuDir, USER_INI);
	profile_initialize(app_path, TRUE);

	profile_write_string("AUTOCHECK", NULL, NULL, app_path);

	j = 0;
	if(tpCheckTime != NULL){
		j = tpCheckTimeCnt;
	}
	TimeItemCnt = 0;
	for(i = 0;i < j;i++){
		if((tpCheckTime + i) == NULL || (tpCheckTime + i)->type == AUTOTIME_NON){
			continue;
		}
		TimeItemCnt++;
	}

	profile_write_int("AUTOCHECK", "Cnt", TimeItemCnt, app_path);

	if(TimeItemCnt != 0){
		for(i = 0;i < j;i++){
			if((tpCheckTime + i) == NULL || (tpCheckTime + i)->type == AUTOTIME_NON){
				continue;
			}
			wsprintf(buf, "type-%d", i);
			profile_write_int("AUTOCHECK", buf, tpCheckTime[i].type, app_path);
			wsprintf(buf, "flag-%d", i);
			profile_write_int("AUTOCHECK", buf, tpCheckTime[i].flag, app_path);
			wsprintf(buf, "day-%d", i);
			profile_write_int("AUTOCHECK", buf, tpCheckTime[i].day, app_path);
			wsprintf(buf, "week-%d", i);
			profile_write_int("AUTOCHECK", buf, tpCheckTime[i].week, app_path);
			wsprintf(buf, "h-%d", i);
			profile_write_int("AUTOCHECK", buf, tpCheckTime[i].h, app_path);
			wsprintf(buf, "m-%d", i);
			profile_write_int("AUTOCHECK", buf, tpCheckTime[i].m, app_path);
		}
	}
	profile_flush(app_path);
	profile_free();
}


/******************************************************************************

	GetToolList

	INIファイルからツール情報を読みこむ

******************************************************************************/

void GetToolList(void)
{
	char app_path[BUFSIZE], buf[BUFSIZE], tmp[BUFSIZE];
	int i;
	int ToolItemCnt;

	wsprintf(app_path, "%s\\%s", CuDir, USER_INI);
	profile_initialize(app_path, TRUE);

	ToolItemCnt = profile_get_int("TOOLS", "Cnt", 0, app_path);

	if(ToolItemCnt == 0){
		ToolList = NULL;
		ToolListCnt = 0;
	}else{
		if(ToolList != NULL){
			FreeTool();
		}
		ToolList = (struct TP_TOOLS *)GlobalAlloc(GPTR, sizeof(struct TP_TOOLS) * ToolItemCnt);
		if(ToolList == NULL){
			abort();
		}
		ToolListCnt = ToolItemCnt;

		for(i = 0;i < ToolItemCnt;i++){
			ToolList[i].iSize = sizeof(struct TP_TOOLS);
			wsprintf(buf, "title-%d", i);
			profile_get_string("TOOLS", buf, "", ToolList[i].title, BUFSIZE - 1, app_path);
			wsprintf(buf, "FileName-%d", i);
			profile_get_string("TOOLS", buf, "", ToolList[i].FileName, BUFSIZE - 1, app_path);
			wsprintf(buf, "Action-%d", i);
			ToolList[i].Action = profile_get_int("TOOLS", buf, 0, app_path);
			wsprintf(buf, "MenuIndex-%d", i);
			ToolList[i].MenuIndex = profile_get_int("TOOLS", buf, -1, app_path);
			wsprintf(buf, "Protocol-%d", i);
			profile_get_string("TOOLS", buf, "", ToolList[i].Protocol, BUFSIZE - 1, app_path);
			wsprintf(buf, "CommandLine-%d", i);
			profile_get_string("TOOLS", buf, "", ToolList[i].CommandLine, BUFSIZE - 1, app_path);
			wsprintf(buf, "func-%d", i);
			profile_get_string("TOOLS", buf, "", ToolList[i].func, BUFSIZE - 1, app_path);

			wsprintf(buf, "Ctrl-%d", i);
			ToolList[i].Ctrl = profile_get_int("TOOLS", buf, 0, app_path);
			wsprintf(buf, "Key-%d", i);
			profile_get_string("TOOLS", buf, "", tmp, BUFSIZE - 1, app_path);
			ToolList[i].Key = *tmp;

			if(str_match("*.dll", ToolList[i].FileName) == TRUE){
				ToolList[i].lib = LoadLibrary(ToolList[i].FileName);
			}
		}
	}
	profile_free();
}


/******************************************************************************

	PutToolList

	INIファイルへツール情報を書きこむ

******************************************************************************/

void PutToolList(void)
{
	char app_path[BUFSIZE], buf[BUFSIZE];
	char ackey[2];
	int i;

	wsprintf(app_path, "%s\\%s", CuDir, USER_INI);
	profile_initialize(app_path, TRUE);

	profile_write_string("TOOLS", NULL, NULL, app_path);

	profile_write_int("TOOLS", "Cnt", ToolListCnt, app_path);

	if(ToolListCnt != 0){
		for(i = 0;i < ToolListCnt;i++){
			wsprintf(buf, "title-%d", i);
			profile_write_string("TOOLS", buf, ToolList[i].title, app_path);
			wsprintf(buf, "FileName-%d", i);
			profile_write_string("TOOLS", buf, ToolList[i].FileName, app_path);
			wsprintf(buf, "Action-%d", i);
			profile_write_int("TOOLS", buf, ToolList[i].Action, app_path);
			wsprintf(buf, "MenuIndex-%d", i);
			profile_write_int("TOOLS", buf, ToolList[i].MenuIndex, app_path);
			wsprintf(buf, "Protocol-%d", i);
			profile_write_string("TOOLS", buf, ToolList[i].Protocol, app_path);
			wsprintf(buf, "CommandLine-%d", i);
			profile_write_string("TOOLS", buf, ToolList[i].CommandLine, app_path);
			wsprintf(buf, "func-%d", i);
			profile_write_string("TOOLS", buf, ToolList[i].func, app_path);

			wsprintf(buf, "Ctrl-%d", i);
			profile_write_int("TOOLS", buf, ToolList[i].Ctrl, app_path);

			*ackey = ToolList[i].Key;
			*(ackey + 1) = '\0';
			wsprintf(buf, "Key-%d", i);
			profile_write_string("TOOLS", buf, ackey, app_path);
		}
	}
	profile_flush(app_path);
	profile_free();
}


/******************************************************************************

	GetDirInfo

	INIファイルからフォルダ情報を読みこむ

******************************************************************************/

void GetDirInfo(HWND hTreeView, char *path, HTREEITEM hItem)
{
	struct TPTREE *tpTreeInfo;
	char app_path[BUFSIZE], buf[BUFSIZE];
	char tmp[MAXSIZE];
	int i;
	int TimeItemCnt;

	tpTreeInfo = (struct TPTREE *)TreeView_GetlParam(hTreeView, hItem);
	if(tpTreeInfo == NULL){
		return;
	}

	wsprintf(app_path, "%s\\"FOLDERFILENAME, path);
	profile_initialize(app_path, TRUE);

	i = profile_get_string("GENERAL", "Comment", "", tmp, MAXSIZE - 1, app_path);
	if(tpTreeInfo->Comment != NULL){
		GlobalFree(tpTreeInfo->Comment);
		tpTreeInfo->Comment = NULL;
	}
	if(i != 0){
		tpTreeInfo->Comment = (char *)GlobalAlloc(GMEM_FIXED, i + 1);
		if(tpTreeInfo->Comment != NULL){
			lstrcpy(tpTreeInfo->Comment, tmp);
		}
	}

	tpTreeInfo->Icon = profile_get_int("GENERAL", "Icon", 0, app_path);
	tpTreeInfo->Icon &= ~(TREEICON_CH | TREEICON_CHECKCHILD);
	SetItemIcon(GetParent(hTreeView), hItem);
	tpTreeInfo->Expand = profile_get_int("GENERAL", "Expand", 0, app_path);

	tpTreeInfo->CheckSt = profile_get_int("CHECK", "CheckSt", 0, app_path);
	tpTreeInfo->CheckMax = profile_get_int("CHECK", "CheckMax", 0, app_path);

	tpTreeInfo->AutoCheckSt = profile_get_int("CHECK", "AutoCheckSt", 1, app_path);

	if(tpTreeInfo->CheckSt == 1){
		TreeView_SetState(hTreeView, hItem, INDEXTOOVERLAYMASK(ICON_ST_NOCHECK), TVIS_OVERLAYMASK);
	}

	TimeItemCnt = profile_get_int("AUTOCHECK", "Cnt", 0, app_path);

	if(TimeItemCnt == 0){
		tpTreeInfo->tpCheckTime = NULL;
		tpTreeInfo->tpCheckTimeCnt = 0;
	}else{
		if(tpTreeInfo->tpCheckTime != NULL){
			GlobalFree(tpTreeInfo->tpCheckTime);
		}
		tpTreeInfo->tpCheckTime = (struct TPCHECKTIME *)GlobalAlloc(GPTR, sizeof(struct TPCHECKTIME) * TimeItemCnt);
		if(tpTreeInfo->tpCheckTime == NULL){
			abort();
		}
		tpTreeInfo->tpCheckTimeCnt = TimeItemCnt;

		for(i = 0;i < TimeItemCnt;i++){
			wsprintf(buf, "type-%d", i);
			tpTreeInfo->tpCheckTime[i].type = profile_get_int("AUTOCHECK", buf, 0, app_path);

			wsprintf(buf, "flag-%d", i);
			tpTreeInfo->tpCheckTime[i].flag = profile_get_int("AUTOCHECK", buf, 0, app_path);
			wsprintf(buf, "day-%d", i);
			tpTreeInfo->tpCheckTime[i].day = profile_get_int("AUTOCHECK", buf, 0, app_path);
			wsprintf(buf, "week-%d", i);
			tpTreeInfo->tpCheckTime[i].week = profile_get_int("AUTOCHECK", buf, 0, app_path);
			wsprintf(buf, "h-%d", i);
			tpTreeInfo->tpCheckTime[i].h = profile_get_int("AUTOCHECK", buf, 0, app_path);
			wsprintf(buf, "m-%d", i);
			tpTreeInfo->tpCheckTime[i].m = profile_get_int("AUTOCHECK", buf, 0, app_path);
		}
	}
	profile_free();
}


/******************************************************************************

	PutDirInfo

	INIファイルへフォルダ情報を書き出す

******************************************************************************/

void PutDirInfo(HWND hTreeView, char *path, HTREEITEM hItem)
{
	struct TPTREE *tpTreeInfo;
	char app_path[BUFSIZE], buf[BUFSIZE];
	int i, j;
	int TimeItemCnt;

	tpTreeInfo = (struct TPTREE *)TreeView_GetlParam(hTreeView, hItem);
	if(tpTreeInfo == NULL){
		return;
	}

	wsprintf(app_path, "%s\\"FOLDERFILENAME, path);
	profile_initialize(app_path, TRUE);

	if(tpTreeInfo->Comment != NULL){
		profile_write_string("GENERAL", "Comment", tpTreeInfo->Comment, app_path);
	}
	profile_write_int("GENERAL", "Icon", tpTreeInfo->Icon, app_path);
	profile_write_int("GENERAL", "Expand",
		((TreeView_GetState(hTreeView, hItem, TVIS_EXPANDED) & TVIS_EXPANDED) ? 1 : 0), app_path);

	profile_write_int("CHECK", "CheckSt", tpTreeInfo->CheckSt, app_path);
	profile_write_int("CHECK", "CheckMax", tpTreeInfo->CheckMax, app_path);

	profile_write_int("CHECK", "AutoCheckSt", tpTreeInfo->AutoCheckSt, app_path);

	profile_write_string("AUTOCHECK", NULL, NULL, app_path);

	j = 0;
	if(tpTreeInfo->tpCheckTime != NULL){
		j = tpTreeInfo->tpCheckTimeCnt;
	}
	TimeItemCnt = 0;
	for(i = 0;i < j;i++){
		if((tpTreeInfo->tpCheckTime + i) == NULL || (tpTreeInfo->tpCheckTime + i)->type == AUTOTIME_NON){
			continue;
		}
		TimeItemCnt++;
	}

	profile_write_int("AUTOCHECK", "Cnt", TimeItemCnt, app_path);

	if(TimeItemCnt != 0){
		for(i = 0;i < j;i++){
			if((tpTreeInfo->tpCheckTime + i) == NULL || (tpTreeInfo->tpCheckTime + i)->type == AUTOTIME_NON){
				continue;
			}
			wsprintf(buf, "type-%d", i);
			profile_write_int("AUTOCHECK", buf, tpTreeInfo->tpCheckTime[i].type, app_path);
			wsprintf(buf, "flag-%d", i);
			profile_write_int("AUTOCHECK", buf, tpTreeInfo->tpCheckTime[i].flag, app_path);
			wsprintf(buf, "day-%d", i);
			profile_write_int("AUTOCHECK", buf, tpTreeInfo->tpCheckTime[i].day, app_path);
			wsprintf(buf, "week-%d", i);
			profile_write_int("AUTOCHECK", buf, tpTreeInfo->tpCheckTime[i].week, app_path);
			wsprintf(buf, "h-%d", i);
			profile_write_int("AUTOCHECK", buf, tpTreeInfo->tpCheckTime[i].h, app_path);
			wsprintf(buf, "m-%d", i);
			profile_write_int("AUTOCHECK", buf, tpTreeInfo->tpCheckTime[i].m, app_path);
		}
	}
	profile_flush(app_path);
	profile_free();
}
/* End of source */
