/**************************************************************************

	WWWC

	main.c
	LinkFiles : wsock32.lib Comctl32.lib Winmm.lib

	Copyright (C) 1996-2018 by Ohno Tomoaki. All rights reserved.
		https://www.nakka.com/
		nakka@nakka.com

**************************************************************************/


/**************************************************************************
	Include Files
**************************************************************************/

#define _INC_OLE
#include <windows.h>
#undef	_INC_OLE
#include <winsock.h>

#include "General.h"
#include "OleDragDrop.h"
#include "Profile.h"


/**************************************************************************
	Define
**************************************************************************/

#define WWWC_WNDCLASS				"WWWCWindowClass"
#define MUTEX						"WWWC_Mutex_"

#define WM_TRAY_NOTIFY				(WM_APP + 100)		//�^�X�N�g���C
#define ID_MENUITEM_ALLINITICON_NOMSG	(WM_APP + 101)
#define HKEY_ID						0x1FFF

#define ICC_BAR_CLASSES				0x00000004 // toolbar, statusbar, trackbar, tooltips

#ifndef TVM_SETBKCOLOR
#define TVM_SETBKCOLOR				(TV_FIRST + 29)
#endif
#ifndef TVM_SETTEXTCOLOR
#define TVM_SETTEXTCOLOR			(TV_FIRST + 30)
#endif


/**************************************************************************
	Global Variables
**************************************************************************/

HINSTANCE g_hinst;					/* �A�v���P�[�V�����̃C���X�^���X�n���h�� */
HANDLE hAccel;
HANDLE hFindAccel;

char CuDir[BUFSIZE];
char TempDir[BUFSIZE];
char DefDirPath[BUFSIZE];
char WorkDir[BUFSIZE];
char CurrentUser[BUFSIZE];
char GeneralUser[BUFSIZE];
char CmdDirPath[BUFSIZE];
char CmdUser[BUFSIZE];

BOOL save_flag;
int gCheckFlag = 0;
UINT WWWC_ClipFormat;
BOOL WWWCDropFlag;

HWND WWWCWnd = NULL;	//�{�̃E�B���h�E
HWND FocusWnd = NULL;	//�t�H�[�J�X�����E�B���h�E
HWND AniWnd = NULL;
HWND UpWnd = NULL;
HWND FindWnd = NULL;
HMENU hPOPUP;
HTREEITEM HiTestItem;
HTREEITEM DgdpItem;
HTREEITEM PropItem;
HICON TrayIcon_Main;
HICON TrayIcon_Chaeck;
HICON TrayIcon_Up;
HICON TrayIcon_Main_Win;
HICON TrayIcon_Chaeck_Win;
HICON StCheckIcon;
HFONT ListFont = NULL;

BOOL UpIconFlag;
BOOL CmdCheckEnd;
BOOL CmdNoUpCheckEnd;

static BOOL WindowFlag;
static BOOL AccelFlag;
static BOOL NoActiveFlag;

//�O���Q��
extern HWND TbWnd;
extern struct TPITEM **CheckItemList;
extern struct TPITEM **UpItemList;
extern int UpItemListCnt;

extern struct TPLVCOLUMN *SortColInfo;
extern struct TPLVCOLUMN *ColumnInfo;
extern struct TPLVCOLUMN *upColumnInfo;
extern struct TPLVCOLUMN *FindColumnInfo;
extern struct TP_PROTOCOL *tpProtocol;
extern int ProtocolCnt;
extern struct TP_TOOLS *ToolList;
extern int ToolListCnt;
extern HACCEL hToolAccel;
extern struct TPCHECKTIME *tpCheckTime;
extern int tpCheckTimeCnt;

extern HTREEITEM RootItem;
extern HTREEITEM RecyclerItem;

extern BOOL ErrCheckFlag;
extern int UpdateItemFlag;
extern char *ToolTipString;

//ini
extern RECT WinRect;
extern int SEPSIZE;
extern int DoubleStart;
extern int DoubleStartMsg;
extern int TrayIcon;
extern int TrayIconMode;
extern int TrayIconToggle;
extern int TrayIconClose;
extern int TrayIconMin;
extern int TrayWinShow;
extern int TrayHotKeyMod;
extern int TrayHotKeyVk;
extern char WinTitle[];
extern int EndRecyclerClear;
extern int TitleView;
extern char ListFontName[];
extern int ListFontSize;
extern int ListFontCharSet;

extern int ViewTb;
extern int ViewSb;

extern int StartExpand;
extern int StartDirOpen;
extern char StartDirPath[];
extern char LastDirPath[];
extern char TvBkColor[];
extern char TvTextColor[];

extern int LvStyle;
extern int lvExStyle;
extern int LvSortFlag;
extern int LvAutoSort;
extern int LvSpaceNextFocus;
extern char LvColumn[];
extern int LvColSize[];
extern int LvColCnt;
extern char LvBkColor[];
extern char LvTextBkColor[];
extern char LvTextColor[];
extern int LvLIconSize;
extern int LvSIconSize;

extern int CheckMax;
extern int EndCheckMsg;

extern int AutoCheck;
extern int StartCheck;

extern int AllInitMsg;


/**************************************************************************
	Local Function Prototypes
**************************************************************************/

static void SetOption(HWND hWnd);
static void ShowHelp(HWND hWnd);
static void WindowRefresh(HWND hWnd);
static void FolderRefresh(HWND hWnd);
static void SaveIniFile(HWND hWnd);
static void RefreshIniFile(HWND hWnd);
static LRESULT ListViewHeaderNotifyProc(HWND hWnd, LPARAM lParam, struct TPLVCOLUMN *ColInfo);
static LRESULT NotifyProc(HWND hWnd, LPARAM lParam);
static BOOL CreateControls(HWND hWnd);
static void SetControls(HWND hWnd);
static BOOL InitWindow(HWND hWnd, BOOL StartFlag);
static BOOL EndWindow(HWND hWnd, int Flag, int EndFlag);
static LONG APIENTRY MainProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static void DelLastYen(char *p);
static void GetAppPath(HINSTANCE hinst);
static void CommandLineFunc(HWND hWnd, char *StrCmdLine);
static void StartCommandLine(char *StrCmdLine);
static BOOL InitApplication(HANDLE hInstance);
static HWND InitInstance(HANDLE hInstance, int nCmdShow);
static BOOL initWinsock(void);
static BOOL MessageFunc(HWND hWnd, MSG *msg);


/******************************************************************************

	_SetForegroundWindow

	�E�B���h�E���A�N�e�B�u�ɂ���

******************************************************************************/

BOOL _SetForegroundWindow(HWND hWnd)
{
	#define _SPI_GETFOREGROUNDLOCKTIMEOUT		  0x2000
	#define _SPI_SETFOREGROUNDLOCKTIMEOUT		  0x2001

	UINT nTimeout;
	BOOL ret;

	SystemParametersInfo(_SPI_GETFOREGROUNDLOCKTIMEOUT, 0, &nTimeout, 0);
	SystemParametersInfo(_SPI_SETFOREGROUNDLOCKTIMEOUT, 0, (PVOID)0, 0);

	ret = SetForegroundWindow(hWnd);

	SystemParametersInfo(_SPI_SETFOREGROUNDLOCKTIMEOUT, 0, (PVOID)nTimeout, 0);
	return ret;
}


/******************************************************************************

	ErrMsg

	�G���[�ԍ�����G���[���b�Z�[�W���쐬

******************************************************************************/

void ErrMsg(HWND hWnd, int ErrCode, char *Title)
{
	char *ErrStr, *p;

	p = (Title == NULL) ? "Error" : Title;

	//�G���[�R�[�h���當������擾
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, ErrCode, 0, (LPTSTR)&ErrStr, BUFSIZE - 1, NULL);

	//�G���[���b�Z�[�W��\��
	MessageBox(hWnd, ErrStr, p, MB_ICONEXCLAMATION);
	LocalFree(ErrStr);
}


/******************************************************************************

	DoEvents

	�C�x���g�̏���

******************************************************************************/

void DoEvents(void)
{
	MSG msg;

	if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) != 0){
		MessageFunc(WWWCWnd, &msg);
	}
}


/******************************************************************************

	WaitCursor

	�J�[�\���������v�ɂ���^�߂�

******************************************************************************/

void WaitCursor(BOOL WiatFlag)
{
	static HCURSOR Old_Cur;		//���̃J�[�\��
	static int Cnt;				//�J�[�\���\���J�E���g

	if(WiatFlag == TRUE){
		if(Cnt > 0){
			Cnt++;
			return;
		}
		Cnt = 1;

		//�����v�J�[�\���ɂ���
		Old_Cur = SetCursor(LoadCursor(NULL, IDC_WAIT));
	}else{
		if(Cnt > 1){
			Cnt--;
			return;
		}
		Cnt = 0;

		if(Old_Cur == NULL){
			SetCursor(LoadCursor(NULL, IDC_ARROW));
		}
		//���̃J�[�\���ɖ߂�
		SetCursor(Old_Cur);
		Old_Cur = NULL;
	}
}


/******************************************************************************

	TrayMessage

	�^�X�N�g���C�̃A�C�R���̐ݒ�

******************************************************************************/

BOOL TrayMessage(HWND hWnd, DWORD dwMessage, UINT uID, HICON hIcon, PSTR pszTip)
{
	NOTIFYICONDATA tnd;

	tnd.cbSize = sizeof(NOTIFYICONDATA);
	tnd.hWnd = hWnd;
	tnd.uID	= uID;
	tnd.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	tnd.uCallbackMessage = WM_TRAY_NOTIFY;
	tnd.hIcon = hIcon;

	if(pszTip){
		lstrcpyn(tnd.szTip, pszTip, sizeof(tnd.szTip));
	}else{
		*tnd.szTip = '\0';
	}

	return Shell_NotifyIcon(dwMessage, &tnd);
}


/******************************************************************************

	SetTrayInitIcon

	�^�X�N�g���C�̃A�C�R��������������

******************************************************************************/

void SetTrayInitIcon(HWND hWnd)
{
	char buf[BUFSIZE];

	if(gCheckFlag == 0 && TrayIcon == 1 && TrayIcon_Main != NULL){
		UpIconFlag = FALSE;
		GetWindowText(hWnd, buf, BUFSIZE - 1);
		if(TrayMessage(hWnd, NIM_MODIFY, TRAY_ID, TrayIcon_Main, buf) == FALSE){
			//�ύX�ł��Ȃ������ꍇ�͒ǉ����s��
			TrayMessage(hWnd, NIM_ADD, TRAY_ID, TrayIcon_Main, buf);
		}
	}
}


/******************************************************************************

	SetWinTitle

	�E�B���h�E�^�C�g����ݒ肷��

******************************************************************************/

void SetWinTitle(HWND hWnd)
{
	char buf[BUFSIZE];

	if(*WinTitle != '\0'){
		wsprintf(buf, APP_NAME" - %s", WinTitle);
	}else{
		lstrcpy(buf, APP_NAME);
	}
	SetWindowText(hWnd, buf);
}


/******************************************************************************

	SetOption

	�I�v�V������ʕ\��

******************************************************************************/

static void SetOption(HWND hWnd)
{
	char buf[BUFSIZE];
	HICON hIcon;

	//�I�v�V������ʕ\��
	if(ViewProperties(hWnd) == 0){
		return;
	}

	//�ݒ�ύX
	if(TrayIcon == 0){
		TrayMessage(hWnd, NIM_DELETE, TRAY_ID, NULL, NULL);
	}else{
		GetWindowText(hWnd, buf, BUFSIZE - 1);

		hIcon = (gCheckFlag == 0) ? TrayIcon_Main : TrayIcon_Chaeck;
		if(TrayMessage(hWnd, NIM_MODIFY, TRAY_ID, hIcon, buf) == FALSE){
			TrayMessage(hWnd, NIM_ADD, TRAY_ID, hIcon, buf);
		}
	}
	SetToolMenu(hWnd);
	SetEnableToolMenu(hWnd);
	SetNewItemMenu(hWnd);
	CreateToolAccelerator();

	SetFocus(GetDlgItem(hWnd, WWWC_TREE));
	SetProtocolMenu(hWnd);

	//INI�t�@�C����������
	SaveIniFile(hWnd);
}


/******************************************************************************

	ShowHelp

	�w���v�̕\��

******************************************************************************/

static void ShowHelp(HWND hWnd)
{
	char buf[BUFSIZE], *p, *r;

	GetModuleFileName(g_hinst, buf, BUFSIZE - 1);
	p = r = buf;
	while(*p != '\0'){
		if(IsDBCSLeadByte((BYTE)*p) == TRUE){
			p++;
		}else if(*p == '\\' || *p == '/'){
			r = p;
		}
		p++;
	}
	*r = '\0';

	lstrcat(buf, "\\wwwc.html");
	ExecItemFile(hWnd, buf, "", NULL, 0);
}


/******************************************************************************

	WindowRefresh

	�E�B���h�E�̓��e���ĕ`�悷��

******************************************************************************/

static void WindowRefresh(HWND hWnd)
{
	WaitCursor(TRUE);

	SetDirTree(GetDlgItem(hWnd, WWWC_TREE), CuDir, RootItem);
	CallTreeItem(hWnd, RootItem, (FARPROC)FindTreeDir, 0);

	ListView_SetRedraw(GetDlgItem(hWnd, WWWC_LIST), LDF_NODRAW);
	ListView_DeleteAllItems(GetDlgItem(hWnd, WWWC_LIST));

	TreeView_FreeItem(hWnd, TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)), 1);
	ReadTreeMem(hWnd, TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)));

	ListView_ShowItem(hWnd, TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)));
	ListView_SetItemState(GetDlgItem(hWnd, WWWC_LIST), 0, LVIS_FOCUSED, LVIS_FOCUSED);

	CallTreeItem(hWnd, RootItem, (FARPROC)TreeView_SetIconState, 0);
	CallTreeItem(hWnd, RootItem, (FARPROC)TreeView_FreeItem, 0);

	SetProtocolMenu(hWnd);
	SetSbText(hWnd);

	InvalidateRect(hWnd, NULL, FALSE);
	UpdateWindow(hWnd);

	ListView_SetRedraw(GetDlgItem(hWnd, WWWC_LIST), LDF_REDRAW);
	WaitCursor(FALSE);
}


/******************************************************************************

	FolderRefresh

	�t�H���_�̓��e��ǂݒ���

******************************************************************************/

static void FolderRefresh(HWND hWnd)
{
	HTREEITEM hItem;

	hItem = TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE));

	WaitCursor(TRUE);
	ListView_SetRedraw(GetDlgItem(hWnd, WWWC_LIST), LDF_NODRAW);
	ListView_DeleteAllItems(GetDlgItem(hWnd, WWWC_LIST));

	TreeView_FreeItem(hWnd, hItem, 2);
	ReadTreeMem(hWnd, hItem);

	ListView_ShowItem(hWnd, hItem);
	ListView_SetItemState(GetDlgItem(hWnd, WWWC_LIST), 0, LVIS_FOCUSED, LVIS_FOCUSED);
	SetProtocolMenu(hWnd);
	SetSbText(hWnd);

	InvalidateRect(hWnd, NULL, FALSE);
	UpdateWindow(hWnd);

	ListView_SetRedraw(GetDlgItem(hWnd, WWWC_LIST), LDF_REDRAW);
	WaitCursor(FALSE);
}


/******************************************************************************

	SaveIniFile

	ini�t�@�C���̕ۑ�

******************************************************************************/

static void SaveIniFile(HWND hWnd)
{
	long wl;
	int ret;

	ListView_SetRedraw(GetDlgItem(hWnd, WWWC_LIST), LDF_NODRAW);

	//���X�g�r���[�̏ڍו\�����̃J�����̕����擾
	wl = GetWindowLong(GetDlgItem(hWnd, WWWC_LIST), GWL_STYLE);
	wl ^= ABS(LvStyle);
	SetWindowLong(GetDlgItem(hWnd, WWWC_LIST), GWL_STYLE, wl | LVS_REPORT);
	for(ret = 0;ret < LvColCnt;ret++){
		LvColSize[ret] = ListView_GetColumnWidth(GetDlgItem(hWnd, WWWC_LIST), ret);
	}
	//ini�t�@�C���̏�������
	PutINI();
	PutAutoTime();
	PutProtocolInfo();
	PutToolList();

	SetWindowLong(GetDlgItem(hWnd, WWWC_LIST), GWL_STYLE, wl | ABS(LvStyle));
	ListView_SetRedraw(GetDlgItem(hWnd, WWWC_LIST), LDF_REDRAW);
}


/******************************************************************************

	RefreshIniFile

	Ini�t�@�C����ǂݍ��ݒ����ăE�B���h�E�ɔ��f������

******************************************************************************/

static void RefreshIniFile(HWND hWnd)
{
	if(gCheckFlag != 0){
		CallTreeItem(hWnd, RootItem, (FARPROC)TreeCancelItem, 0);
	}
	WaitCursor(TRUE);

	EndProtocolNotify();
	FreeProtocol();

	GetINI();
	GetProtocolInfo();

	EndWindow(hWnd, 1, 1);

	DestroyWindow(GetDlgItem(hWnd, WWWC_LIST));
	DestroyWindow(GetDlgItem(hWnd, WWWC_TREE));
	DestroyWindow(AniWnd);
	DestroyWindow(GetDlgItem(hWnd, WWWC_TB));
	DestroyWindow(GetDlgItem(hWnd, WWWC_SB));

	ReTbButtonInfo();

	GetINI();
	GetProtocolInfo();

	InitWindow(hWnd, FALSE);

	WaitCursor(FALSE);
}


/******************************************************************************

	ListViewHeaderNotifyProc

	���X�g�r���[�w�b�_���b�Z�[�W

******************************************************************************/

static LRESULT ListViewHeaderNotifyProc(HWND hWnd, LPARAM lParam, struct TPLVCOLUMN *ColInfo)
{
	HD_NOTIFY *phd = (HD_NOTIFY *)lParam;
	struct TPTREE *tpTreeInfo;

	switch(phd->hdr.code)
	{
	case HDN_ITEMCLICK:
		WaitCursor(TRUE);

		ListView_SortClear(GetDlgItem(hWnd, WWWC_LIST), LvSortFlag);

		//�\�[�g�̐ݒ�
		LvSortFlag = (ABS(LvSortFlag) == (phd->iItem + 1)) ? (LvSortFlag * -1) : (phd->iItem + 1);
		SortColInfo = ColInfo;
		//�\�[�g
		ListView_SortItems(GetDlgItem(hWnd, WWWC_LIST), CompareFunc, LvSortFlag);
		ListView_SortSelect(GetDlgItem(hWnd, WWWC_LIST), LvSortFlag);

		//���X�g�r���[�A�C�e���̕��т��������ɔ��f����
		tpTreeInfo = (struct TPTREE *)TreeView_GetlParam(GetDlgItem(hWnd, WWWC_TREE),
			TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)));
		if(tpTreeInfo != NULL){
			GlobalFree(tpTreeInfo->ItemList);
			tpTreeInfo->ItemListCnt = 0;
			tpTreeInfo->ItemList = ListView_SetListToMem(hWnd, &tpTreeInfo->ItemListCnt);
		}
		WaitCursor(FALSE);
		break;
	}
	return FALSE;
}


/******************************************************************************

	NotifyProc

	�R���g���[���̃C�x���g�̏���

******************************************************************************/

static LRESULT NotifyProc(HWND hWnd, LPARAM lParam)
{
	NMHDR *CForm = (NMHDR *)lParam;

	//���X�g�r���[
	if(CForm->hwndFrom == GetDlgItem(hWnd, WWWC_LIST)){
		return ListView_NotifyProc(hWnd, lParam);
	}

	//�c���[�r���[
	if(CForm->hwndFrom == GetDlgItem(hWnd, WWWC_TREE)){
		return TreeView_NotifyProc(hWnd, lParam);
	}

	//���X�g�r���[�w�b�_
	if(CForm->hwndFrom == GetWindow(GetDlgItem(hWnd, WWWC_LIST), GW_CHILD)){
		return ListViewHeaderNotifyProc(hWnd, lParam, ColumnInfo);
	}

	//�X�e�[�^�X�o�[
	if(CForm->code == TTN_NEEDTEXT && CForm->idFrom < 3){
		return StatusBar_NotifyProc(hWnd, lParam);
	}

	//�c�[���o�[
	if(CForm->hwndFrom == TbWnd || CForm->code == TTN_NEEDTEXT){
		return ToolBar_NotifyProc(hWnd, lParam);
	}
	return FALSE;
}


/******************************************************************************

	CreateControls

	���C���E�B���h�E�ɃR���g���[�����쐬����

******************************************************************************/

static BOOL CreateControls(HWND hWnd)
{
	//TreeView
	if(CreateTreeView(hWnd) == NULL){
		return FALSE;
	}
	TreeView_Initialize(hWnd);
	if(*TvBkColor != '\0'){
		SendDlgItemMessage(hWnd, WWWC_TREE, TVM_SETBKCOLOR, 0, strtol(TvBkColor, NULL, 0));
	}
	if(*TvTextColor != '\0'){
		SendDlgItemMessage(hWnd, WWWC_TREE, TVM_SETTEXTCOLOR, 0, strtol(TvTextColor, NULL, 0));
	}
	if(ListFont != NULL){
		SendMessage(GetDlgItem(hWnd, WWWC_TREE), WM_SETFONT, (WPARAM)ListFont, MAKELPARAM(TRUE, 0));
	}

	//ListView
	if(CreateListView(hWnd) == NULL){
		return FALSE;
	}
	SendDlgItemMessage(hWnd, WWWC_LIST, LVM_SETEXTENDEDLISTVIEWSTYLE, 0,
		lvExStyle | SendDlgItemMessage(hWnd, WWWC_LIST, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0));
	ListView_SetItemImage(GetDlgItem(hWnd, WWWC_LIST), LvLIconSize, LVSIL_NORMAL);
	ListView_SetItemImage(GetDlgItem(hWnd, WWWC_LIST), LvSIconSize, LVSIL_SMALL);
	SetProtocolImage(hWnd);
	LvColCnt = ListView_AddColumn(GetDlgItem(hWnd, WWWC_LIST), LvColumn, LvColSize, ColumnInfo);
	if(*LvBkColor != '\0'){
		ListView_SetBkColor(GetDlgItem(hWnd, WWWC_LIST), strtol(LvBkColor, NULL, 0));
	}
	if(*LvTextBkColor != '\0'){
		ListView_SetTextBkColor(GetDlgItem(hWnd, WWWC_LIST), strtol(LvTextBkColor, NULL, 0));
	}
	if(*LvTextColor != '\0'){
		ListView_SetTextColor(GetDlgItem(hWnd, WWWC_LIST), strtol(LvTextColor, NULL, 0));
	}
	if(ListFont != NULL){
		SendMessage(GetDlgItem(hWnd, WWWC_LIST), WM_SETFONT, (WPARAM)ListFont, MAKELPARAM(TRUE, 0));
	}

	//ToolBar
	CreateTB(hWnd);

	//StatusBar
	CreateStatusBar(hWnd);
	return TRUE;
}


/******************************************************************************

	SetControls

	�R���g���[���̈ʒu�A�T�C�Y��ݒ肷��

******************************************************************************/

static void SetControls(HWND hWnd)
{
	RECT WinRec;
	RECT TBRec;
	long TBSize = 0;
	RECT SBRec;
	long SBSize = 0;

	GetClientRect(hWnd, (LPRECT)&WinRec);

	//ToolBar�̃T�C�Y�̎擾
	if(IsWindowVisible(TbWnd) != 0){
		GetWindowRect(TbWnd, (LPRECT)&TBRec);
		TBSize = (TBRec.bottom - TBRec.top);
	}
	//StatusBar�̃T�C�Y�̎擾
	if(IsWindowVisible(GetDlgItem(hWnd, WWWC_SB)) != 0){
		GetWindowRect(GetDlgItem(hWnd, WWWC_SB), (LPRECT)&SBRec);
		SBSize = (SBRec.bottom - SBRec.top);
	}

	//�A�j���[�V�����p�_�C�A���O�̈ړ�
	MoveWindow(AniWnd, WinRec.right - STEPSIZE - 2, 0, STEPSIZE + 2, STEPSIZE + 2, TRUE);
	//TreeView�̈ʒu�A�T�C�Y�̐ݒ�
	MoveWindow(GetDlgItem(hWnd, WWWC_TREE), 0, TBSize, SEPSIZE, WinRec.bottom - SBSize - TBSize, TRUE);
	UpdateWindow(GetDlgItem(hWnd, WWWC_TREE));
	//ListView�̈ʒu�A�T�C�Y�̐ݒ�
	MoveWindow(GetDlgItem(hWnd, WWWC_LIST), SEPSIZE + (FRAME_CNT * 2), TBSize,
		WinRec.right - SEPSIZE - (FRAME_CNT * 2), WinRec.bottom - SBSize - TBSize, TRUE);
	UpdateWindow(GetDlgItem(hWnd, WWWC_LIST));


	//�X�e�[�^�X�o�[�̐ݒ�
	SBSetParts(hWnd);

	if(AniWnd != NULL){
		InvalidateRect(AniWnd, NULL, FALSE);
		UpdateWindow(AniWnd);
	}
}


/******************************************************************************

	InitWindow

	�v���O�����̏�����

******************************************************************************/

static BOOL InitWindow(HWND hWnd, BOOL StartFlag)
{
	HTREEITEM hItem;
	char buf[BUFSIZE], *p;
	UINT cf[CLIPFORMAT_CNT];
	int i;
	BOOL ReloadFlag = FALSE;

	WaitCursor(TRUE);

	//�h���b�O�^�[�Q�b�g�ɓo�^
	cf[0] = WWWC_ClipFormat;
	cf[1] = CF_HDROP;
	cf[2] = CF_TEXT;
	OLE_IDropTarget_RegisterDragDrop(hWnd, WM_DRAGMSG, cf, CLIPFORMAT_CNT);

	//�E�B���h�E�̃^�C�g����ݒ�
	SetWinTitle(hWnd);

	//�t�H���g�쐬
	if(*ListFontName != '\0'){
		if(ListFont != NULL){
			DeleteObject(ListFont);
		}
		ListFont = CreateListFont(ListFontName, ListFontSize, ListFontCharSet);
	}

	//�E�B���h�E���̃R���g���[�����쐬
	if(CreateControls(hWnd) == FALSE){
		MessageBox(hWnd, EMSG_CREATECONTROL, APP_NAME, MB_OK | MB_ICONERROR);
		return FALSE;
	}
	UpWnd = CreateDialogParam(g_hinst, MAKEINTRESOURCE(IDD_DIALOG_UP), NULL, UpMessageProc, 0);
	if(UpWnd == NULL){
		MessageBox(hWnd, EMSG_CREATEUPMSG, APP_NAME, MB_OK | MB_ICONERROR);
		return FALSE;
	}
	//�E�B���h�E�쐬�t���O
	WindowFlag = TRUE;

	//�A�C�R�������[�h
	TrayIcon_Main = LoadImage(g_hinst, MAKEINTRESOURCE(IDI_ICON_MAIN), IMAGE_ICON, SICONSIZE, SICONSIZE, 0);
	TrayIcon_Chaeck = LoadImage(g_hinst, MAKEINTRESOURCE(IDI_ICON_MAIN_CHECK), IMAGE_ICON, SICONSIZE, SICONSIZE, 0);
	TrayIcon_Up = LoadImage(g_hinst, MAKEINTRESOURCE(IDI_ICON_UP), IMAGE_ICON, SICONSIZE, SICONSIZE, 0);
	TrayIcon_Main_Win = LoadIcon(g_hinst, MAKEINTRESOURCE(IDI_ICON_MAIN));
	TrayIcon_Chaeck_Win = LoadIcon(g_hinst, MAKEINTRESOURCE(IDI_ICON_MAIN_CHECK));

	StCheckIcon = (HICON)LoadImage(g_hinst, MAKEINTRESOURCE(IDI_ICON_CHECK), IMAGE_ICON, SICONSIZE, SICONSIZE, 0);

	//�^�X�N�g���C�ɃA�C�R����ǉ�
	GetWindowText(hWnd, buf, BUFSIZE - 1);
	if(TrayIcon == 1){
		TrayMessage(hWnd, NIM_ADD, TRAY_ID, TrayIcon_Main, buf);
	}

	//�c�[���̏�����
	GetToolList();
	SetToolMenu(hWnd);
	CreateToolAccelerator();

	//�v���g�R���̐ݒ�
	SetProtocolInfo();
	SetNewItemMenu(hWnd);
	SetProtocolMenu(hWnd);
	//���j���[�ɐV�K�쐬���j���[���֘A�t����
	ModifyMenu(GetSubMenu(GetMenu(hWnd), MENU_HWND_FILE), ID_MENUITEM_POP_NEWITEM, MF_POPUP,
		(UINT)GetSubMenu(hPOPUP, MENU_POP_NEW), MENU_STR_NEWITEM);
	ModifyMenu(GetSubMenu(hPOPUP, MENU_POP_FOLDER), ID_MENUITEM_POP_NEWITEM, MF_POPUP,
		(UINT)GetSubMenu(hPOPUP, MENU_POP_NEW), MENU_STR_NEWITEM);

	//�����`�F�b�N�p�^�C�}�[�̐ݒ�
	GetAutoTime();
	SetTimer(hWnd, TIMER_AUTOCHECK, 60000, NULL);

	//�E�B���h�E���̃R���g���[����������
	SetControls(hWnd);
	ListView_StyleChange(hWnd, LvStyle);
	if(ViewTb == 1){
		CheckMenuItem(GetSubMenu(GetMenu(hWnd), MENU_HWND_VIEW), ID_MENUITEM_VIEW_TB, MF_CHECKED);
	}
	if(ViewSb == 1){
		CheckMenuItem(GetSubMenu(GetMenu(hWnd), MENU_HWND_VIEW), ID_MENUITEM_VIEW_SB, MF_CHECKED);
	}
	if(LvAutoSort == 1){
		CheckMenuItem(GetSubMenu(GetMenu(hWnd), MENU_HWND_VIEW), ID_MENUITEM_AUTOSORT, MF_CHECKED);
		CheckMenuItem(GetSubMenu(hPOPUP, MENU_POP_FOLDER), ID_MENUITEM_AUTOSORT, MF_CHECKED);
	}

	//TreeView�̏�����Ԑݒ�
	TreeView_Expand(GetDlgItem(hWnd, WWWC_TREE), RootItem, TVE_EXPAND);
	if(StartExpand != 0){
		CallTreeItem(hWnd, RootItem, (FARPROC)TreeView_AllExpand, StartExpand);
	}
	switch(StartDirOpen)
	{
	case 1:
		//�N�����\������t�H���_
		p = StartDirPath;
		break;

	case 2:
		//�O��I�����ɊJ���Ă����t�H���_
		p = LastDirPath;
		break;

	default:
		p = NULL;
		TreeView_SelectItem(GetDlgItem(hWnd, WWWC_TREE), RootItem);
		TreeView_EnsureVisible(GetDlgItem(hWnd, WWWC_TREE), RootItem);
		break;
	}
	if(p != NULL){
		hItem = TreeView_FindItemPath(GetDlgItem(hWnd, WWWC_TREE), RootItem, p);
		if(hItem != NULL){
			TreeView_SelectItem(GetDlgItem(hWnd, WWWC_TREE), hItem);
			TreeView_EnsureVisible(GetDlgItem(hWnd, WWWC_TREE), hItem);
		}
	}
	CallTreeItem(hWnd, RootItem, (FARPROC)TreeView_FreeItem, 0);

	//�R���g���[���̃t�H�[�J�X��ݒ�
	FocusWnd = GetDlgItem(hWnd, WWWC_TREE);
	SetFocus(FocusWnd);

	//�E�B���h�E���N���b�v�{�[�h�r���[�A�`�F�[���ɒǉ�
	Clipboard_SetChain(hWnd);

	//�z�b�g�L�[�̓o�^
	if(TrayHotKeyMod != 0 && TrayHotKeyVk != 0 &&
		RegisterHotKey(hWnd, HKEY_ID, TrayHotKeyMod, TrayHotKeyVk) == FALSE){
		ErrMsg(hWnd, GetLastError(), NULL);
	}

	WaitCursor(FALSE);

	//�N�����ł͂Ȃ��ꍇ�͊֐��𔲂���
	if(StartFlag == FALSE){
		return TRUE;
	}

	//�N�����Ɏ��s����c�[��
	for(i = 0;i < ToolListCnt;i++){
		if((ToolList[i].Action & TOOL_EXEC_START) == 0){
			continue;
		}
		if(str_match("*.dll", ToolList[i].FileName) == TRUE){
			DllToolExec(hWnd, i, NULL, -1, TOOL_EXEC_START, 0);
		}else{
			ExecItemFile(hWnd, ToolList[i].FileName, ToolList[i].CommandLine, NULL, ToolList[i].Action & TOOL_EXEC_SYNC);
		}
		if((ToolList[i].Action & TOOL_EXEC_SAVEFOLDER) != 0){
			ReloadFlag = TRUE;
		}
	}
	if(ReloadFlag == TRUE){
		//�t�H���_�̓��e��ǂݒ���
		SendMessage(hWnd, WM_FOLDER_LOAD, 0, 0);
	}

	//�N���������`�F�b�N
	if(StartCheck == 1){
		SendMessage(hWnd, WM_COMMAND, ID_MENUITEM_ALLCHECK, 0);
	}
	return TRUE;
}


/******************************************************************************

	EndWindow

	�v���O�������I������

******************************************************************************/

static BOOL EndWindow(HWND hWnd, int Flag, int EndFlag)
{
	HIMAGELIST hImgList;
	char RootText[BUFSIZE];
	char CurPath[BUFSIZE];
	int i;
	BOOL ReloadFlag = FALSE;

	//�E�B���h�E�T�C�Y�̋����ۑ�
	SendMessage(hWnd, WM_EXITSIZEMOVE, 0, 0);

	//�`�F�b�N���̃A�C�e�������݂���ꍇ�̓L�����Z�����s��
	if(gCheckFlag != 0){
		if(EndCheckMsg == 1 && EndFlag == 0){
			if(MessageBox(hWnd, QMSG_END_CHECKCANCEL, QMSG_END_TITLE, MB_ICONEXCLAMATION | MB_YESNO) == IDNO){
				return FALSE;
			}
		}
		CallTreeItem(hWnd, RootItem, (FARPROC)TreeCancelItem, 0);
	}

	//���݊J���Ă���t�H���_��ۑ�
	if(SaveTreeMem(hWnd, GetDlgItem(hWnd, WWWC_TREE), TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE))) == FALSE){
		return FALSE;
	}

	//�I�����Ɏ��s����c�[�������s
	for(i = 0;i < ToolListCnt;i++){
		if((ToolList[i].Action & TOOL_EXEC_END) == 0){
			continue;
		}
		if(str_match("*.dll", ToolList[i].FileName) == TRUE){
			DllToolExec(hWnd, i, NULL, -1, TOOL_EXEC_END, 0);
		}else{
			ExecItemFile(hWnd, ToolList[i].FileName, ToolList[i].CommandLine, NULL,
				ToolList[i].Action & TOOL_EXEC_SYNC);
		}
		if((ToolList[i].Action & TOOL_EXEC_SAVEFOLDER) != 0){
			ReloadFlag = TRUE;
		}
	}
	if(ReloadFlag == TRUE){
		//�t�H���_�̓��e��ǂݒ���
		SendMessage(hWnd, WM_FOLDER_LOAD, 0, 0);
	}

	WindowFlag = FALSE;
	WaitCursor(TRUE);

	EndProtocolNotify();

	if(StartDirOpen == 2){
		//����N�����ɊJ���t�H���_�̎擾
		TreeView_GetItemInfo(GetDlgItem(hWnd, WWWC_TREE), RootItem, RootText);
		wsprintf(CurPath, "\\\\%s", RootText);
		TreeView_GetPath(GetDlgItem(hWnd, WWWC_TREE),
			TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)), LastDirPath, CurPath);
	}

	//�I�����ɃS�~������ɂ���ݒ�̏ꍇ�̓S�~������ɂ���
	if(EndRecyclerClear == 1){
		ClearRecycler(hWnd, TRUE);
	}

	//�N���b�v�{�[�h�̊Ď��`�F�[������O��
	Clipboard_DeleteChain(hWnd);
	DeleteTmpDropFiles(Clip_DirName);

	//OLE�h���b�v�^�[�Q�b�g������
	OLE_IDropTarget_RevokeDragDrop(hWnd);
	DeleteTmpDropFiles(DnD_DirName);

	//�����E�B���h�E��j��
	if(FindWnd != NULL){
		SendMessage(FindWnd, WM_CLOSE, 0, 0);
	}
	//�X�V���b�Z�[�W�E�B���h�E��j��
	SendMessage(UpWnd, WM_UP_CLOSE, 0, 0);
	//�N�[���o�[���̃A�C�R���\���E�B�h�E�����
	SendMessage(AniWnd, WM_CLOSE, 0, 0);

	if (save_flag == FALSE) {
		// ini�t�@�C���ɐݒ���e����������
		SaveIniFile(hWnd);
	}

	//�c���[�������
	CallTreeItem(hWnd, RootItem, (FARPROC)TreeView_FreeTreeItem, 0);
	//�J�����������
	GlobalFree(ColumnInfo);
	ColumnInfo = NULL;
	GlobalFree(upColumnInfo);
	upColumnInfo = NULL;
	GlobalFree(FindColumnInfo);
	FindColumnInfo = NULL;

	if(UpItemList != NULL){
		GlobalFree(UpItemList);
		UpItemList = NULL;
		UpItemListCnt = 0;
	}

	//�����`�F�b�N�ݒ�����
	if(tpCheckTime != NULL){
		GlobalFree(tpCheckTime);
		tpCheckTime = NULL;
		tpCheckTimeCnt = 0;
	}

	//Undo�o�b�t�@�̉��
	FreeUndo();

	//�c�[���`�b�v�p�o�b�t�@�̉��
	if(ToolTipString != NULL){
		GlobalFree(ToolTipString);
		ToolTipString = NULL;
	}

	//�^�X�N�g���C�̃A�C�R��������
	if(TrayIcon == 1){
		TrayMessage(hWnd, NIM_DELETE, TRAY_ID, NULL, NULL);
	}
	UnregisterHotKey(hWnd, HKEY_ID);

	//�c���[�r���[�ƃ��X�g�r���[�Ɋ֘A�t����ꂽ�C���[�W���X�g��j��
	hImgList = TreeView_SetImageList(GetDlgItem(hWnd, WWWC_TREE), NULL, TVSIL_NORMAL);
	ImageList_Destroy((void *)hImgList);
	hImgList = ListView_SetImageList(GetDlgItem(hWnd, WWWC_LIST), NULL, LVSIL_NORMAL);
	ImageList_Destroy((void *)hImgList);
	hImgList = ListView_SetImageList(GetDlgItem(hWnd, WWWC_LIST), NULL, LVSIL_SMALL);
	ImageList_Destroy((void *)hImgList);

	//�A�C�R�������
	DestroyIcon(TrayIcon_Main);
	DestroyIcon(TrayIcon_Chaeck);
	DestroyIcon(TrayIcon_Up);
	DestroyIcon(TrayIcon_Main_Win);
	DestroyIcon(TrayIcon_Chaeck_Win);
	DestroyIcon(StCheckIcon);

	if(Flag == 0){
		//���C���E�B���h�E��j��
		DestroyWindow(AniWnd);
		DestroyWindow(TbWnd);
		DestroyWindow(GetDlgItem(hWnd, WWWC_SB));
		DestroyWindow(GetDlgItem(hWnd, WWWC_TREE));
		DestroyWindow(GetDlgItem(hWnd, WWWC_LIST));
		DestroyWindow(hWnd);
		WWWCWnd = NULL;

		if(ListFont != NULL){
			DeleteObject(ListFont);
		}
	}
	WaitCursor(FALSE);
	return TRUE;
}


/******************************************************************************

	MainProc

	���C���E�B���h�E�̃v���V�[�W��

******************************************************************************/

static LONG APIENTRY MainProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static BOOL SepMoveFlag;
	int ret, i;

	switch (uMsg)
	{
	case WM_CREATE:
		save_flag = TRUE;
		WWWCWnd = hWnd;
		{
			HWND tWnd = NULL;

			//�^�C�g���E�B���h�E�\��
			if(TitleView == 1){
				tWnd = CreateTitleWindow(hWnd);
			}
			//�E�B���h�E�̏�����
			SetWindowPos(hWnd, HWND_TOP, WinRect.left, WinRect.top, WinRect.right, WinRect.bottom,
				SWP_HIDEWINDOW);
			if(InitWindow(hWnd, TRUE) == FALSE){
				DestroyWindow(hWnd);
				WWWCWnd = NULL;
				break;
			}
			CloseTitleWindow(tWnd);
		}
		save_flag = FALSE;
		break;

	//�N���b�v�{�[�h�r���[�A�`�F�[���̕ύX
	case WM_CHANGECBCHAIN:
		Clipboard_ChangeChain(hWnd, uMsg, wParam, lParam);
		break;

	//�N���b�v�{�[�h�̓��e�ύX
	case WM_DRAWCLIPBOARD:
		Clipboard_Draw(hWnd, uMsg, wParam, lParam);
		break;

	case WM_COPYDATA:
		CommandLineFunc(hWnd, ((PCOPYDATASTRUCT)lParam)->lpData);
		break;

	case WM_ACTIVATE:
		if(LOWORD(wParam) == WA_INACTIVE && IsIconic(hWnd) == 0 && GetFocus() != NULL && GetFocus() != AniWnd){
			FocusWnd = GetFocus();
		}
		break;

	case WM_SETFOCUS:
		if(WindowFlag == FALSE){
			break;
		}
		SetFocus(FocusWnd);
		SetTrayInitIcon(hWnd);
		SetProtocolMenu(hWnd);
		break;

	//���E�̈ړ�
	case WM_LBUTTONDOWN:
		if(GetForegroundWindow() != hWnd){
			break;
		}
		if(SepMoveFlag == FALSE){
			if(FrameInitialize(hWnd) == FALSE){
				break;
			}
			SepMoveFlag = TRUE;
			FrameDraw(hWnd);
			SetTimer(hWnd, TIMER_SEP, 1, NULL);
		}
		break;

	case WM_MOUSEMOVE:
		if(SepMoveFlag == TRUE){
			FrameDraw(hWnd);
		}
		break;

	case WM_LBUTTONUP:
		if(SepMoveFlag == TRUE){
			SepMoveFlag = FALSE;

			KillTimer(hWnd, TIMER_SEP);

			if((ret = FrameDrawEnd(hWnd)) == -1){
				break;
			}
			SEPSIZE = ret;
			SetControls(hWnd);
		}
		break;

	case WM_SIZE:
		if(wParam == SIZE_MINIMIZED){
			if(TrayIcon == 1 && TrayIconMin == 1){
				ShowWindow(hWnd, SW_HIDE);
				return 0;
			}
		}

		SendMessage(TbWnd, WM_SIZE, wParam, lParam);
		SendDlgItemMessage(hWnd, WWWC_SB, WM_SIZE, wParam, lParam);
		SetControls(hWnd);
		break;

	//�E�B���h�E�T�C�Y�Ҕ�
	case WM_EXITSIZEMOVE:
		if(IsWindowVisible(hWnd) != 0 && IsIconic(hWnd) == 0 && IsZoomed(hWnd) == 0){
			GetWindowRect(hWnd, (LPRECT)&WinRect);
			WinRect.right -= WinRect.left;
			WinRect.bottom -= WinRect.top;
		}
		break;

	case WM_ENDSESSION:
		if (EndWindow(hWnd, 0, 1) == TRUE) {
			save_flag = TRUE;
		}
		return 0;

	case WM_CLOSE:
		if (EndWindow(hWnd, 0, 0) == TRUE) {
			save_flag = TRUE;
		}
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_TIMER:
		switch(wParam)
		{
		//���E�̈ړ�
		case TIMER_SEP:
			if(hWnd != GetForegroundWindow() || GetAsyncKeyState(VK_ESCAPE) < 0 ||
				GetAsyncKeyState(VK_RBUTTON) < 0)
			{
				KillTimer(hWnd, wParam);
				FrameFree();
				SepMoveFlag = FALSE;
				InvalidateRect(hWnd, NULL, FALSE);
				UpdateWindow(hWnd);
			}
			break;

		//�h���b�O���̃c���[�r���[�A�C�e���̓W�J
		case TIMER_TREEEXPAND:
			KillTimer(hWnd, wParam);
			if(TreeView_GetHiTestItem(GetDlgItem(hWnd, WWWC_TREE)) != NULL){
				TreeView_Expand(GetDlgItem(hWnd, WWWC_TREE),
					TreeView_GetHiTestItem(GetDlgItem(hWnd, WWWC_TREE)), TVE_EXPAND);
			}
			break;

		//�`�F�b�N���̃^�C���A�E�g
		case TIMER_CHECKTIMEOUT:
			TimeoutItem(hWnd);
			break;

		//�`�F�b�N
		case TIMER_CHECK:
			CheckProc(hWnd);
			break;

		//�����`�F�b�N
		case TIMER_AUTOCHECK:
			if(AutoCheck == 0){
				if(CheckAutoTime(hWnd) == TRUE){
					ErrCheckFlag = FALSE;

					CheckIniProc(hWnd, NULL, CHECKINI_AUTOALLCHECK, CHECKTYPE_AUTO | CHECKTYPE_ALL);

					CheckProc(hWnd);
					InvalidateRect(GetDlgItem(hWnd, WWWC_LIST), NULL, FALSE);
					UpdateWindow(GetDlgItem(hWnd, WWWC_LIST));
				}
				CallTreeItem(hWnd, RootItem, (FARPROC)TreeCheckAutoTime, 0);
			}
			break;

		//�X�e�[�^�X�o�[�̐ݒ�
		case TIMER_SBTEXT:
			KillTimer(hWnd, wParam);
			SetSbText(hWnd);
			SetProtocolMenu(hWnd);
			break;

		//���̃A�C�e���Ƀt�H�[�J�X���ړ�����
		case TIMER_NEXTFOCUS:
			KillTimer(hWnd, wParam);
			ListView_SetItemState(GetDlgItem(hWnd, WWWC_LIST),
				ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), -1, LVNI_FOCUSED) + 1,
				LVIS_FOCUSED, LVIS_FOCUSED);
			ListView_EnsureVisible(GetDlgItem(hWnd, WWWC_LIST),
				ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), -1, LVNI_FOCUSED), TRUE);
			break;
		}
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		//====== �L�[ =========
		case ID_KEY_RETURN:
			if(GetFocus() != GetDlgItem(hWnd, WWWC_LIST)){
				break;
			}
			//�v���g�R�����̃��j���[��ݒ�
			SetProtocolMenu(hWnd);
			i = GetMenuDefaultItem(GetSubMenu(hPOPUP, MENU_POP_ITEM), 0, 0);
			if(i == -1){
				break;
			}
			SendMessage(hWnd, WM_COMMAND, i, 0);
			break;

		case ID_KEY_TAB:
			keybd_event(VK_TAB,0,0,0);
			keybd_event(VK_TAB,0,KEYEVENTF_KEYUP,0);
			break;

		case ID_KEY_UPDIR:
			if(TreeView_GetParent(GetDlgItem(hWnd, WWWC_TREE),
				TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE))) != NULL){

				TreeView_SelectItem(GetDlgItem(hWnd, WWWC_TREE),
					TreeView_GetParent(GetDlgItem(hWnd, WWWC_TREE),
					TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE))));
			}
			break;

		case ID_KEY_RBUTTON:
			SetProtocolMenu(hWnd);
			if(GetFocus() == GetDlgItem(hWnd, WWWC_LIST)){
				if(ListView_GetSelectedCount(GetDlgItem(hWnd, WWWC_LIST)) <= 0){
					ShowMenu(hWnd, hPOPUP, MENU_POP_FOLDER);
					break;
				}
				ShowMenu(hWnd, hPOPUP, MENU_POP_ITEM);
			}else{
				HiTestItem = TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE));
				SetFolderEnableMenu(hWnd, HiTestItem);
				ShowMenu(hWnd, hPOPUP, MENU_POP_ITEM);
			}
			break;

		//====== �t�@�C�� =========
		case ID_MENUITEM_TBNEW:
			ShowTbMenu(hWnd, GetSubMenu(hPOPUP, MENU_POP_NEW), ID_MENUITEM_TBNEW);
			break;

		case ID_MENU_ACTION_OPEM:
			Item_Open(hWnd, -1);
			break;

		case ID_MENUITEM_NEWFOLDER:
			TreeView_NewFolderItem(hWnd);
			break;

		case ID_MENUITEM_DELETE:
			HiTestItem = TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE));
		case ID_MENUITEM_DELETE_POP:
			DeleteItem(hWnd);
			break;

		case ID_KEY_EDITNAME:
		case ID_MENUITEM_EDITNAME:
			HiTestItem = TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE));
		case ID_MENUITEM_EDITNAME_POP:
			if(GetFocus() == GetDlgItem(hWnd, WWWC_TREE)){
				TreeView_EditLabel(GetDlgItem(hWnd, WWWC_TREE), HiTestItem);
			}else if(GetFocus() == GetDlgItem(hWnd, WWWC_LIST)){
				ListView_SetItemState(GetDlgItem(hWnd, WWWC_LIST),
					ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), -1, LVNI_FOCUSED),
					LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
				ListView_EditLabel(GetDlgItem(hWnd, WWWC_LIST),
					ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), -1, LVNI_FOCUSED));
			}
			break;

		case ID_MENUITEM_PROP:
			HiTestItem = TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE));
		case ID_MENUITEM_PROP_POP:
			Item_Property(hWnd);
			break;

		case ID_MENUITEM_CLOSE:
			SendMessage(hWnd, WM_CLOSE, 0, 0);
			break;

		//====== �ҏW =========
		case ID_MENUITEM_UNDO:
			ExecUndo(hWnd);
			break;

		case ID_MENUITEM_COPY:
			HiTestItem = TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE));
		case ID_MENUITEM_COPY_POP:
			Clipboard_SetItemData(hWnd, FLAG_COPY);
			break;

		case ID_MENUITEM_CUT:
			HiTestItem = TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE));
		case ID_MENUITEM_CUT_POP:
			Clipboard_SetItemData(hWnd, FLAG_CUT);
			break;

		case ID_MENUITEM_PASTE:
		case ID_KEY_PASTE:
			Clipboard_GetData(hWnd);
			SetSbText(hWnd);
			SetProtocolMenu(hWnd);
			break;

		case ID_MENUITEM_SERACH:
			HiTestItem = TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE));
		case ID_MENUITEM_SERACH_POP:
			if(FindWnd != NULL){
				_SetForegroundWindow(FindWnd);
				break;
			}
			if(GetFocus() == GetDlgItem(hWnd, WWWC_LIST) && ListView_GetSelectedCount(GetDlgItem(hWnd, WWWC_LIST)) != 0){
				HiTestItem = TreeView_GetIndexToItem(GetDlgItem(hWnd, WWWC_TREE), TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)),
					ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), -1, LVNI_FOCUSED));
			}
			if(HiTestItem == NULL){
				HiTestItem = TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE));
			}
			FindWnd = CreateDialogParam(g_hinst, MAKEINTRESOURCE(IDD_DIALOG_FIND), NULL, FindProc, (LPARAM)HiTestItem);
			break;

		//====== �A�C�e���̑I�� =========
		case ID_MENUITEM_ALLSELECT:
			SetFocus(GetDlgItem(hWnd, WWWC_LIST));
			ListView_SetItemState(GetDlgItem(hWnd, WWWC_LIST), -1, LVIS_SELECTED, LVIS_SELECTED);
			break;

		case ID_MENUITEM_SWITCHSELECT:
			SetFocus(GetDlgItem(hWnd, WWWC_LIST));
			ListView_SwitchSelectItem(GetDlgItem(hWnd, WWWC_LIST));
			break;

		case ID_MENUITEM_UPSELECT:
			SetFocus(GetDlgItem(hWnd, WWWC_LIST));
			ListView_UpSelectItem(GetDlgItem(hWnd, WWWC_LIST));
			break;

		//====== �\�� =========
		case ID_MENUITEM_VIEW_TB:
			if(ViewTb == 1){
				ViewTb = 0;
				ShowWindow(TbWnd, SW_HIDE);
				CheckMenuItem(GetSubMenu(GetMenu(hWnd), MENU_HWND_VIEW), LOWORD(wParam), MF_UNCHECKED);
			}else{
				ViewTb = 1;
				ShowWindow(TbWnd, SW_SHOW);
				CheckMenuItem(GetSubMenu(GetMenu(hWnd), MENU_HWND_VIEW), LOWORD(wParam), MF_CHECKED);
			}
			SetControls(hWnd);
			InvalidateRect(GetDlgItem(hWnd, WWWC_TREE), NULL, FALSE);
			UpdateWindow(GetDlgItem(hWnd, WWWC_TREE));
			InvalidateRect(GetDlgItem(hWnd, WWWC_LIST), NULL, FALSE);
			UpdateWindow(GetDlgItem(hWnd, WWWC_LIST));
			break;

		case ID_MENUITEM_VIEW_SB:
			if(ViewSb == 1){
				ViewSb = 0;
				ShowWindow(GetDlgItem(hWnd, WWWC_SB), SW_HIDE);
				CheckMenuItem(GetSubMenu(GetMenu(hWnd), MENU_HWND_VIEW), LOWORD(wParam), MF_UNCHECKED);
			}else{
				ViewSb = 1;
				ShowWindow(GetDlgItem(hWnd, WWWC_SB), SW_SHOW);
				CheckMenuItem(GetSubMenu(GetMenu(hWnd), MENU_HWND_VIEW), LOWORD(wParam), MF_CHECKED);
			}
			SetControls(hWnd);
			break;

		case ID_MENUITEM_VIEWUPMSG:
			ShowWindow(UpWnd, SW_SHOW);
			_SetForegroundWindow(UpWnd);
			break;

		case ID_MENUITEM_OPTION:
			SetOption(hWnd);
			break;

		//====== ListView Style =========
		case ID_MENUITEM_VIEW_ICON:
			ListView_SetStyle(hWnd, LVS_ICON);
			break;

		case ID_MENUITEM_VIEW_SMALLICON:
			ListView_SetStyle(hWnd, LVS_SMALLICON);
			break;

		case ID_MENUITEM_VIEW_LIST:
			ListView_SetStyle(hWnd, LVS_LIST);
			break;

		case ID_MENUITEM_VIEW_REPORT:
			ListView_SetStyle(hWnd, LVS_REPORT);
			break;

		case ID_MENUITEM_VIEW_REPORT_LINE:
			ListView_SetStyle(hWnd, LVS_REPORT * -1);
			break;

		//====== �\�[�g =========
		case ID_MENUITEM_SORT_ICON:
			ListView_MenuSort(hWnd, 100);
			break;

		case ID_MENUITEM_SORT_NAME:
			ListView_MenuSort(hWnd, 0);
			break;

		case ID_MENUITEM_SORT_SIZE:
			ListView_MenuSort(hWnd, 1);
			break;

		case ID_MENUITEM_SORT_DATE:
			ListView_MenuSort(hWnd, 2);
			break;

		case ID_MENUITEM_SORT_URL:
			ListView_MenuSort(hWnd, 3);
			break;

		//�����\�[�g
		case ID_MENUITEM_AUTOSORT:
			if(LvAutoSort == 1){
				LvAutoSort = 0;
				CheckMenuItem(GetSubMenu(GetMenu(hWnd), MENU_HWND_VIEW), LOWORD(wParam), MF_UNCHECKED);
				CheckMenuItem(GetSubMenu(hPOPUP, MENU_POP_FOLDER), LOWORD(wParam), MF_UNCHECKED);
				ListView_SortClear(GetDlgItem(hWnd, WWWC_LIST), LvSortFlag);
				InvalidateRect(hWnd, NULL, FALSE);
				UpdateWindow(hWnd);
			}else{
				struct TPTREE *tpTreeInfo;

				LvAutoSort = 1;
				CheckMenuItem(GetSubMenu(GetMenu(hWnd), MENU_HWND_VIEW), LOWORD(wParam), MF_CHECKED);
				CheckMenuItem(GetSubMenu(hPOPUP, MENU_POP_FOLDER), LOWORD(wParam), MF_CHECKED);

				WaitCursor(TRUE);
				SortColInfo = ColumnInfo;
				ListView_SortItems(GetDlgItem(hWnd, WWWC_LIST), CompareFunc, LvSortFlag);
				ListView_SortSelect(GetDlgItem(hWnd, WWWC_LIST), LvSortFlag);

				tpTreeInfo = (struct TPTREE *)TreeView_GetlParam(GetDlgItem(hWnd, WWWC_TREE),
					TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)));
				if(tpTreeInfo != NULL){
					GlobalFree(tpTreeInfo->ItemList);
					tpTreeInfo->ItemListCnt = 0;
					tpTreeInfo->ItemList = ListView_SetListToMem(hWnd, &tpTreeInfo->ItemListCnt);
				}
				WaitCursor(FALSE);
			}
			break;

		//====== �ړ� =========
		case ID_MENUITEM_PREV_HISTORY:
			TreeView_PrevHistory(hWnd);
			break;

		case ID_MENUITEM_NEXT_HISTORY:
			TreeView_NextHistory(hWnd);
			break;

		//====== �ŐV�\�� =========
		case ID_MENUITEM_REFRESH:
			WindowRefresh(hWnd);
			break;

		case ID_MENUITEM4_TB_CUSTOM:
			SendMessage(TbWnd, TB_CUSTOMIZE, 0, 0);
			break;

		//====== �`�F�b�N�̊J�n =========
		case ID_MENUITEM_CHECK:
			HiTestItem = TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE));
		case ID_MENUITEM_CHECK_POP:
			i = 0;
			if(ErrCheckFlag == TRUE && gCheckFlag != 0){
				i = ErrCheckFlag;
			}
			ErrCheckFlag = FALSE;

			if(GetFocus() == GetDlgItem(hWnd, WWWC_LIST)){
				HiTestItem = TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE));
			}
			CheckIniProc(hWnd, HiTestItem, CHECKINI_CHECK, CHECKTYPE_ITEM);

			ErrCheckFlag = i;

			CheckProc(hWnd);
			InvalidateRect(GetDlgItem(hWnd, WWWC_LIST), NULL, FALSE);
			UpdateWindow(GetDlgItem(hWnd, WWWC_LIST));
			break;

		case ID_MENUITEM_ALLCHECK:
			ErrCheckFlag = FALSE;

			CheckIniProc(hWnd, NULL, CHECKINI_ALLCHECK, CHECKTYPE_ALL);
			CheckProc(hWnd);
			InvalidateRect(GetDlgItem(hWnd, WWWC_LIST), NULL, FALSE);
			UpdateWindow(GetDlgItem(hWnd, WWWC_LIST));
			break;

		case ID_MENUITEM_FOLDERTREECHECK:
			HiTestItem = TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE));
		case ID_MENUITEM_FOLDERTREECHECK_POP:
			i = 0;
			if(ErrCheckFlag == TRUE && gCheckFlag != 0){
				i = ErrCheckFlag;
			}
			ErrCheckFlag = FALSE;

			if(GetFocus() == GetDlgItem(hWnd, WWWC_LIST)){
				HiTestItem = TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE));
			}
			CheckIniProc(hWnd, HiTestItem, CHECKINI_TREECHECK, CHECKTYPE_TREE);

			ErrCheckFlag = i;

			CheckProc(hWnd);
			InvalidateRect(GetDlgItem(hWnd, WWWC_LIST), NULL, FALSE);
			UpdateWindow(GetDlgItem(hWnd, WWWC_LIST));
			break;

		case ID_MENUITEM_ALLERRORCHECK:
			ErrCheckFlag = TRUE;

			CheckIniProc(hWnd, NULL, CHECKINI_ALLCHECK, CHECKTYPE_ERROR);
			CheckProc(hWnd);
			InvalidateRect(GetDlgItem(hWnd, WWWC_LIST), NULL, FALSE);
			UpdateWindow(GetDlgItem(hWnd, WWWC_LIST));
			break;

		//====== �`�F�b�N�̒��~ =========
		case ID_MENUITEM_CHECKEND:
			HiTestItem = TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE));
		case ID_MENUITEM_CHECKEND_POP:
			if(gCheckFlag == 0){
				break;
			}
			WaitCursor(TRUE);

			if(GetFocus() == GetDlgItem(hWnd, WWWC_LIST)){
				ListCancelItem(hWnd);
			}else{
				TreeCancelItem(hWnd, HiTestItem, 0);
			}

			WaitCursor(FALSE);

			InvalidateRect(GetDlgItem(hWnd, WWWC_LIST), NULL, FALSE);
			UpdateWindow(GetDlgItem(hWnd, WWWC_LIST));

			if(FindCheckNoCheckItem(hWnd, RootItem) == 0){
				WaitCursor(TRUE);
				CheckEndProc(hWnd);
				WaitCursor(FALSE);
			}else{
				SetTimer(hWnd, TIMER_CHECK, NEXTCHECK_INTERVAL, NULL);
			}
			break;

		case ID_MENUITEM_ALLCHECKEND:
			if(gCheckFlag == 0){
				break;
			}
			WaitCursor(TRUE);

			CallTreeItem(hWnd, RootItem, (FARPROC)TreeCancelItem, 0);

			WaitCursor(FALSE);

			InvalidateRect(GetDlgItem(hWnd, WWWC_LIST), NULL, FALSE);
			UpdateWindow(GetDlgItem(hWnd, WWWC_LIST));

			WaitCursor(TRUE);
			CheckEndProc(hWnd);
			WaitCursor(FALSE);
			break;

		//====== �A�C�R���̏����� =========
		case ID_MENUITEM_INITICON:
			HiTestItem = TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE));
		case ID_MENUITEM_INITICON_POP:

			WaitCursor(TRUE);

			if(GetFocus() == GetDlgItem(hWnd, WWWC_LIST)){
				ListItemIni(hWnd);
			}else{
				TreeItemIni(hWnd, HiTestItem, 0);
			}

			WaitCursor(FALSE);

			InvalidateRect(GetDlgItem(hWnd, WWWC_LIST), NULL, FALSE);
			UpdateWindow(GetDlgItem(hWnd, WWWC_LIST));
			SetSbText(hWnd);
			break;

		case ID_MENUITEM_ALLINITICON:
			if(gCheckFlag != 0 || (AllInitMsg == 1 &&
				MessageBox(hWnd, QMSG_ALLINITICON, QMSG_ALLINITICON_TITLE, MB_ICONEXCLAMATION | MB_YESNO) == IDNO)){
				break;
			}
		case ID_MENUITEM_ALLINITICON_NOMSG:
			if(gCheckFlag != 0){
				break;
			}
			WaitCursor(TRUE);

			CallTreeItem(hWnd, RootItem, (FARPROC)TreeItemIni, 0);

			WaitCursor(FALSE);

			InvalidateRect(GetDlgItem(hWnd, WWWC_LIST), NULL, FALSE);
			UpdateWindow(GetDlgItem(hWnd, WWWC_LIST));
			SetSbText(hWnd);
			break;

		//====== �A�C�R���ύX =========
		case ID_MENUITEM_ICON_UP:
			Item_ChangeState(hWnd, ST_UP);
			break;

		case ID_MENUITEM_ICON_ERROR:
			Item_ChangeState(hWnd, ST_ERROR);
			break;

		case ID_MENUITEM_ICON_TIMEOUT:
			Item_ChangeState(hWnd, ST_TIMEOUT);
			break;

		case ID_MENUITEM_CHECKSWITCH:
			HiTestItem = TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE));
		case ID_MENUITEM_CHECKSWITCH_POP:
			Item_SwitchCheckState(hWnd, HiTestItem);
			break;

		//====== �o�[�W������� =========
		case ID_MENUITEM_HELP:
			ShowHelp(hWnd);
			break;

		case ID_MENUITEM_ABOUT:
			MessageBox(hWnd,
				APP_NAME" Ver "APP_VERSION"\n"APP_COPYRIGHT"\n\nWEB SITE: "APP_URL"\nE-MAIL: "APP_MAIL,
				ABOUTTITLE, MB_OK | MB_ICONINFORMATION);
			break;

		//====== �|�b�v�A�b�v���j���[ =========
		case ID_MENU_ACTION_RECY:
			ClearRecycler(hWnd, FALSE);
			break;

		case ID_MENUITEM_REWINDOW:
			ShowWindow(hWnd, SW_SHOW);
			//�A�C�R��������Ă���ꍇ�͌��̃T�C�Y�ɖ߂�
			if(IsIconic(hWnd) != 0){
				ShowWindow(hWnd, SW_RESTORE);
			}
			_SetForegroundWindow(hWnd);
			SetFocus(FocusWnd);
			break;

		case ID_MENUITEM_AUTOCHECK:
			AutoCheck = (AutoCheck == 0) ? 1 : 0;
			break;

		case ID_MENUITEM_END:
			PostMessage(hWnd, WM_CLOSE, 0, 0);
			break;

		default:
			if(LOWORD(wParam) >= ID_MENU_NEWITEM && LOWORD(wParam) < (ID_MENU_NEWITEM + 1000)){
				//�V�K�쐬�̃��j���[
				Item_Create(hWnd, LOWORD(wParam) - ID_MENU_NEWITEM);

			}else if(LOWORD(wParam) >= ID_MENU_ACTION && LOWORD(wParam) < (ID_MENU_ACTION + 1000)){
				//�A�C�e�����̃A�N�V�������j���[
				Item_Open(hWnd, LOWORD(wParam) - ID_MENU_ACTION);

			}else if(LOWORD(wParam) >= ID_MENU_TOOL_ACTION && LOWORD(wParam) < (ID_MENU_TOOL_ACTION + 1000)){
				//�c�[���i�A�C�e�����j���[�j
				ExecTool(hWnd, LOWORD(wParam) - ID_MENU_TOOL_ACTION, FALSE);

			}else if(LOWORD(wParam) >= ID_WMENU_TOOL_ACTION && LOWORD(wParam) < (ID_WMENU_TOOL_ACTION + 1000)){
				//�c�[���i�E�B���h�E���j���[�j
				ExecTool(hWnd, LOWORD(wParam) - ID_WMENU_TOOL_ACTION, TRUE);

			}else if(LOWORD(wParam) >= ID_ACCEL_TOOL_ACTION && LOWORD(wParam) < (ID_ACCEL_TOOL_ACTION + 1000)){
				//�c�[�� (�V���[�g�J�b�g�L�[)
				ExecTool(hWnd, LOWORD(wParam) - ID_ACCEL_TOOL_ACTION, TRUE);
			}
			break;
		}
		break;

	case WM_SYSCOMMAND:
		if(wParam == SC_MINIMIZE && GetFocus() != NULL){
			FocusWnd = GetFocus();
		}
		if(wParam == SC_CLOSE && TrayIcon == 1 && TrayIconClose == 1){
			ShowWindow(hWnd, SW_HIDE);
			return 0;
		}
		return (DefWindowProc(hWnd, uMsg, wParam, lParam));

	//�^�X�N�g���C���b�Z�[�W
	case WM_TRAY_NOTIFY:
		switch(lParam)
		{
		case WM_LBUTTONDBLCLK:
			if(TrayIconMode != 0){
				if(TrayIconToggle == 1 && IsIconic(hWnd) == 0 && IsWindowVisible(hWnd) != 0){
					ShowWindow(hWnd, SW_MINIMIZE);
				}else{
					SendMessage(hWnd, WM_COMMAND, ID_MENUITEM_REWINDOW, 0);
				}
			}
			break;

		case WM_LBUTTONDOWN:
			if(TrayIconMode == 0){
				if(TrayIconToggle == 1 && IsIconic(hWnd) == 0 && IsWindowVisible(hWnd) != 0){
					ShowWindow(hWnd, SW_MINIMIZE);
				}else{
					SendMessage(hWnd, WM_COMMAND, ID_MENUITEM_REWINDOW, 0);
				}
			}
			break;

		case WM_RBUTTONUP:
			SetMenuDefaultItem(GetSubMenu(hPOPUP, MENU_POP_TASKTRAY),
				ID_MENUITEM_REWINDOW, 0);
			EnableMenuItem(GetSubMenu(hPOPUP, MENU_POP_TASKTRAY),
				ID_MENUITEM_ALLCHECKEND, !(gCheckFlag));
			CheckMenuItem(GetSubMenu(hPOPUP, MENU_POP_TASKTRAY), ID_MENUITEM_AUTOCHECK,
				((AutoCheck == 1) ? MF_UNCHECKED : MF_CHECKED));
			ShowMenu(hWnd, hPOPUP, MENU_POP_TASKTRAY);
			break;
		}
		break;

	//�z�b�g�L�[���b�Z�[�W
	case WM_HOTKEY:
		SendMessage(hWnd, WM_COMMAND, ID_MENUITEM_REWINDOW, 0);
		break;

	//�R���g���[���ʒm���b�Z�[�W
	case WM_NOTIFY:
		if(WindowFlag == FALSE){
			break;
		}
		return NotifyProc(hWnd, lParam);

	//���X�g�r���[�̃C�x���g
	case WM_LV_EVENT:
		switch(wParam)
		{
		case LVN_ITEMCHANGED:
			//�X�e�[�^�X�o�[�̐ݒ�
			SetTimer(hWnd, TIMER_SBTEXT, 100, NULL);
			break;

		case LVN_BEGINLABELEDIT:
			//���ݔ��͖��O�ύX�s��
			if(ListView_IsRecyclerItem(hWnd) == TRUE){
				return TRUE;
			}
			AccelFlag = TRUE;
			ListView_SetItemState(GetDlgItem(hWnd, WWWC_LIST), -1, 0, LVIS_SELECTED);
			break;

		case LVN_ENDLABELEDIT:
			AccelFlag = FALSE;
			ListView_SetItemTitle(hWnd, (LV_DISPINFO *)lParam);
			break;

		case LVN_GETDISPINFO:
			return ListView_GetDispItem(hWnd, &(((LV_DISPINFO *)lParam)->item));

		case LVN_BEGINDRAG:
		case LVN_BEGINRDRAG:
			ListView_StartDragItem(hWnd);
			break;

		case LVN_KEYDOWN:
			if(((LV_KEYDOWN *)lParam)->wVKey == VK_BACK){
				//BackSpace�L�[�������ꂽ�ꍇ�́A���̊K�w�Ɉړ�
				SendMessage(hWnd, WM_COMMAND, ID_KEY_UPDIR, 0);
				break;
			}
			if(LvSpaceNextFocus == 1 && ((LV_KEYDOWN *)lParam)->wVKey == VK_SPACE){
				SetTimer(hWnd, TIMER_NEXTFOCUS, 1, NULL);
			}
			break;

		case NM_SETFOCUS:
			//�v���g�R�����̃��j���[��ݒ�
			SetProtocolMenu(hWnd);
			break;

		case NM_CLICK:
		case NM_DBLCLK:
			ListView_ItemClick(hWnd, (NMHDR *)lParam);
			break;

		case NM_RCLICK:
			//�v���g�R�����̃��j���[��ݒ�
			SendMessage(hWnd, WM_COMMAND, ID_KEY_RBUTTON, 0);
			break;
		}
		break;

	//���X�g�r���[�A�C�e���̏�����
	case WM_LV_INITICON:
		{
			struct TPITEM *tpItemInfo;
			tpItemInfo = (struct TPITEM *)ListView_GetlParam(GetDlgItem(hWnd, WWWC_LIST), wParam);
			if(tpItemInfo == NULL){
				break;
			}
			Item_Initialize(hWnd, tpItemInfo, FALSE);
			ListView_RedrawItems(GetDlgItem(hWnd, WWWC_LIST), wParam, wParam);
			UpdateWindow(GetDlgItem(hWnd, WWWC_LIST));
			ListView_RefreshItem(GetDlgItem(hWnd, WWWC_LIST));

			TreeView_SetIconState(hWnd, TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)), 0);
			SetSbText(hWnd);
		}
		break;

	//�c���[�r���[�̃C�x���g
	case WM_TV_EVENT:
		switch(wParam)
		{
		case TVN_SELCHANGING:
			return TreeView_SelItemChanging(hWnd,
				((NM_TREEVIEW *)lParam)->itemNew.hItem,
				((NM_TREEVIEW *)lParam)->itemOld.hItem
				);

		case TVN_SELCHANGED:
			return TreeView_SelItemChanged(hWnd,
				((NM_TREEVIEW *)lParam)->itemNew.hItem,
				((NM_TREEVIEW *)lParam)->itemOld.hItem
				);

		case TVN_BEGINLABELEDIT:
			//���[�g�A�C�e���̏ꍇ�̓��x���ҏW���s��Ȃ�
			if(((TV_DISPINFO *)lParam)->item.hItem == RootItem ||
				((TV_DISPINFO *)lParam)->item.hItem == RecyclerItem){
				return TRUE;
			}
			AccelFlag = TRUE;
			return FALSE;

		case TVN_ENDLABELEDIT:
			AccelFlag = FALSE;
			if(((TV_DISPINFO *)lParam)->item.pszText == NULL ||
				((TV_DISPINFO *)lParam)->item.pszText[0] == '\0'){
				break;
			}
			//�c���[�A�C�e���̃^�C�g����ݒ肷��
			TreeView_SetName(hWnd, ((TV_DISPINFO *)lParam)->item.hItem,
				((TV_DISPINFO *)lParam)->item.pszText);
			ListView_RefreshFolder(hWnd);
			break;

		case TVN_BEGINDRAG:
		case TVN_BEGINRDRAG:
			TreeView_StartDragItem(hWnd, ((NM_TREEVIEW *)lParam)->itemNew.hItem);
			break;

		case NM_SETFOCUS:
			//�v���g�R�����̃��j���[��ݒ�
			SetProtocolMenu(hWnd);
			break;

		case NM_RCLICK:
			SetProtocolMenu(hWnd);
			HiTestItem = (HTREEITEM)SendDlgItemMessage(hWnd, WWWC_TREE,
				TVM_GETNEXTITEM, TVGN_DROPHILITE, 0);
			if(HiTestItem == NULL){
				HiTestItem = TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE));
			}
			TreeView_SelectDropTarget(GetDlgItem(hWnd, WWWC_TREE), HiTestItem);
			SetFolderEnableMenu(hWnd, HiTestItem);
			ShowMenu(hWnd, hPOPUP, MENU_POP_ITEM);
			TreeView_SelectDropTarget(GetDlgItem(hWnd, WWWC_TREE), NULL);
			break;
		}
		break;

	//�c�[���o�[�̕ύX�ɂ�郁�j���[�̍Đݒ�
	case WM_TB_REFRESH:
		SetProtocolMenu(hWnd);
		ListView_StyleChange(hWnd, LvStyle);
		break;

	//�h���b�O�C�x���g
	case WM_DRAGMSG:
		return DragDrop_NotifyProc(hWnd, wParam, lParam);

	//�h���b�v�f�[�^�v��
	case WM_DATAOBJECT_GETDATA:
		if(wParam == WWWC_ClipFormat){
			*((HGLOBAL *)lParam) = Clipboard_Set_WF_ItemList(hWnd, FLAG_CUT, DgdpItem);
			WWWCDropFlag = TRUE;
			break;
		}

		WWWCDropFlag = FALSE;
		switch(wParam)
		{
		case CF_TEXT:
			*((HGLOBAL *)lParam) = Clipboard_Set_TEXT(hWnd, DgdpItem);
			break;

		case CF_HDROP:
			*((HGLOBAL *)lParam) = DragDrop_SetDropFileMem(hWnd);
			break;
		}
		break;

	//�z�X�g���擾���b�Z�[�W (WSAAsyncGetHostByName)
	case WM_WSOCK_GETHOST:
		if(gCheckFlag == 0){
			break;
		}
		GethostMsg(hWnd, wParam, lParam);
		break;

	//Select���b�Z�[�W (WSAAsyncSelect)
	case WM_WSOCK_SELECT:
		if(gCheckFlag == 0){
			break;
		}
		SelectMsg(hWnd, wParam, lParam);
		break;

	//DLL����̃A�C�e���̃`�F�b�N�I���ʒm
	case WM_CHECK_END:
		if (gCheckFlag == 0) {
			break;
		}
		if (ResultCheckStatus(hWnd, ((struct TPITEM *)lParam), wParam) == FALSE) {
			//�`�F�b�N�A�C�e�����X�g�̏�����
			InitCheckItemList(hWnd, ((struct TPITEM *)lParam));
		}
		break;

	//DLL����̃A�C�e���̃`�F�b�N���ʒʒm
	case WM_CHECK_RESULT:
		if(gCheckFlag == 0){
			break;
		}
		CheckEndItem(hWnd, wParam, lParam);
		break;

	//���̃`�F�b�N
	case WM_NEXTCHECK:
		if(gCheckFlag == 0){
			break;
		}
		InitCheckItemList(hWnd, (struct TPITEM *)lParam);
		break;

	//�A�C�e���̃`�F�b�N�v��
	case WM_ITEMCHECK:
		ItemCheckIni(hWnd, (struct TPITEM *)lParam, TRUE);
		CheckIniProc(hWnd, ((struct TPITEM *)lParam)->hItem, CHECKINI_DLLCHECK, CHECKTYPE_ITEM);
		CheckProc(hWnd);
		ret = ListView_GetMemToIndex(GetDlgItem(hWnd, WWWC_LIST), (struct TPITEM *)lParam);
		if(ret != -1){
			ListView_RedrawItems(GetDlgItem(hWnd, WWWC_LIST), ret, ret);
			UpdateWindow(GetDlgItem(hWnd, WWWC_LIST));
		}
		break;

	//�A�C�e�����X�g�̗v��
	case WM_GETCHECKLIST:
		return (long)CheckItemList;

	//�A�C�e�����X�g���̗v��
	case WM_GETCHECKLISTCNT:
		return CheckMax;

	//�A�C�e�����Z�b�g���郁�b�Z�[�W
	case WM_SETITEMLIST:
		if(Item_ListAdd(hWnd, (struct TPITEM **)lParam, wParam) == TRUE){
			FocusWnd = GetDlgItem(hWnd, WWWC_LIST);
			SetFocus(FocusWnd);
		}
		break;

	//�A�C�e���̎��̂�v��
	case WM_GETMAINITEM:
		return (long)FindMainItem(hWnd, (struct TPITEM *)lParam);

	//�A�C�e�������s
	case WM_ITEMEXEC:
		return Item_DefaultOpen(hWnd, (struct TPITEM *)lParam);

	//�A�C�e����������
	case WM_ITEMINIT:
		{
			struct TPITEM *tpItemInfo;

			//�A�C�e���̌���
			tpItemInfo = FindMainItem(hWnd, (struct TPITEM *)lParam);
			if(tpItemInfo == NULL){
				break;
			}
			//�A�C�e���̏�����
			Item_Initialize(hWnd, tpItemInfo, FALSE);
			TreeView_SetIconState(hWnd, tpItemInfo->hItem, 0);
			TreeView_FreeItem(hWnd, tpItemInfo->hItem, 1);

			if(tpItemInfo->hItem == TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE))){
				//���X�g�r���[�̍X�V
				i = ListView_GetMemToIndex(GetDlgItem(hWnd, WWWC_LIST), tpItemInfo);
				ListView_RedrawItems(GetDlgItem(hWnd, WWWC_LIST), i, i);
				UpdateWindow(GetDlgItem(hWnd, WWWC_LIST));
				ListView_RefreshItem(GetDlgItem(hWnd, WWWC_LIST));
			}
		}
		break;

	//WWWC�̃o�[�W�����v��
	case WM_GETVERSION:
		if(lParam != 0){
			lstrcpy((char *)lParam, APP_VERSION);
		}
		break;

	//���C���E�B���h�E�̃n���h�����擾
	case WM_GETMAINWINDOW:
		if(lParam != 0){
			*(HWND *)lParam = hWnd;
		}
		break;

	//�X�V���b�Z�[�W�̃n���h�����擾
	case WM_GETUPWINDOW:
		if(lParam != 0){
			*(HWND *)lParam = UpWnd;
		}
		break;

	//�����E�B���h�E�̃n���h�����擾
	case WM_GETFINDWINDOW:
		if(lParam != 0){
			*(HWND *)lParam = FindWnd;
		}
		break;

	//�ݒ�̓ǂݒ���
	case WM_WWWC_GETINI:
		RefreshIniFile(hWnd);
		break;

	//�ݒ�̏�������
	case WM_WWWC_PUTINI:
		SaveIniFile(hWnd);
		break;

	//INI�t�@�C���̕ۑ���̃p�X���擾
	case WM_WWWC_GETINIPATH:
		if(lParam != 0){
			lstrcpy((char *)lParam, CuDir);
		}
		break;

	//�t�H���_�̓��e��ۑ�
	case WM_FOLDER_SAVE:
		TreeView_FreeItem(hWnd, TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)), 1);
		break;

	//�t�H���_�̓��e��ǂݍ���
	case WM_FOLDER_LOAD:
		FolderRefresh(hWnd);
		break;

	//�t�H���_�̃p�X���擾
	case WM_FOLDER_GETPATH:
		if(lParam != 0){
			TreeView_GetPath(GetDlgItem(hWnd, WWWC_TREE),
				((wParam == 0) ? TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)) : (HTREEITEM)wParam),
				(char *)lParam, CuDir);
		}
		break;

	//�t�H���_�̃p�X���擾
	case WM_FOLDER_GETWWWCPATH:
		if(lParam != 0){
			char buf[BUFSIZE];
			char tmp[BUFSIZE];

			TreeView_GetItemInfo(GetDlgItem(hWnd, WWWC_TREE), RootItem, buf);
			wsprintf(tmp, "\\\\%s", buf);
			TreeView_GetPath(GetDlgItem(hWnd, WWWC_TREE),
				((wParam == 0) ? TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)) : (HTREEITEM)wParam),
				(char *)lParam, tmp);
		}
		break;

	//�t�H���_�̑I��
	case WM_FOLDER_SELECT:
		if(lParam != 0){
			SelectFolder((HWND)wParam, (char *)lParam);
		}
		break;

	//�t�H���_�̓���
	case WM_FOLDER_REFRESH:
		SetDirTree(GetDlgItem(hWnd, WWWC_TREE), CuDir, RootItem);
		CallTreeItem(hWnd, RootItem, (FARPROC)FindTreeDir, 0);
		break;

	//�C�x���g����
	case WM_DOEVENTS:
		DoEvents();
		break;

	default:
		return (DefWindowProc(hWnd, uMsg, wParam, lParam));
	}
	return 0;
}


/******************************************************************************

	DelLastYen

	�f�B���N�g���p�X�̍Ō�� \ ����������

******************************************************************************/

static void DelLastYen(char *p)
{
	while(*p != '\0'){
		if(IsDBCSLeadByte((BYTE)*p) == TRUE){
			p++;
		}else if((*p == '\\' || *p == '/') && *(p + 1) == '\0'){
			*p = '\0';
			break;
		}
		p++;
	}
}


/******************************************************************************

	GetAppPath

	���[�U�f�B���N�g���̍쐬

******************************************************************************/

static void GetAppPath(HINSTANCE hinst)
{
	char DefItem[BUFSIZE];
	char gIniFile[BUFSIZE];
	char *p, *r;
	int i;

	//�A�v���P�[�V�����̃p�X���擾
	GetModuleFileName(hinst, DefDirPath, BUFSIZE - 1);
	r = p = DefDirPath;
	while(*p != '\0'){
		if(IsDBCSLeadByte((BYTE)*p) == TRUE){
			p++;
		}else if(*p == '\\' || *p == '/'){
			r = p;
		}
		p++;
	}
	*r = '\0';

	//�f�t�H���g�A�C�e��
	wsprintf(DefItem, "%s\\DefItem", DefDirPath);

	//���ʂ̐ݒ���g�p����ꍇ�̃��[�U���̎擾
	wsprintf(gIniFile, "%s\\%s", DefDirPath, GENERAL_INI);
	profile_initialize(gIniFile, TRUE);
	profile_get_string("GENERAL", "User", "", GeneralUser, BUFSIZE - 1, gIniFile);
	//��ƃf�B���N�g�����擾
	profile_get_string("GENERAL", "WorkDir", "", WorkDir, BUFSIZE - 1, gIniFile);
	if(*WorkDir != '\0'){
		DelLastYen(WorkDir);
		mkDirStr(WorkDir);
		if(GetDirSerch(WorkDir) == TRUE){
			lstrcpy(DefDirPath, WorkDir);
		}
	}
	profile_write_string("GENERAL", "WorkDir", WorkDir, gIniFile);
	profile_flush(gIniFile);
	profile_free();

	if(*CmdDirPath != '\0'){
		//�R�}���h���C���Ŏw�肳�ꂽ�f�B���N�g��
		lstrcpy(DefDirPath, CmdDirPath);
	}

	//�ꎞ�f�B���N�g���̃p�X���擾
	if(GetTempPath(BUFSIZE - 1, TempDir) == 0){
		//���s�����ꍇ�̓A�v���P�[�V�����̃p�X�̉��ɐݒ�
		wsprintf(TempDir, "%s\\Temp", DefDirPath);
	}else{
		DelLastYen(TempDir);
	}

	if(*CmdUser != '\0'){
		//�R�}���h���C���Ŏw�肳�ꂽ���[�U��
		lstrcpy(CurrentUser, CmdUser);
	}else{
		//���݂̃��O�C�����[�U���̎擾
		i = BUFSIZE - 1;
		if(GetUserName(CurrentUser, &i) == FALSE) {
			lstrcpy(CurrentUser, DEFAULTUSER);
		}
	}

	//�t�@�C�����Ɏg���Ȃ������� '_' �ɕϊ�
	FileNameConv(GeneralUser, '_');
	FileNameConv(CurrentUser, '_');

	wsprintf(CuDir, "%s\\%s", DefDirPath, (*CmdUser != '\0' || *GeneralUser == '\0') ? CurrentUser : GeneralUser);

	if(GetDirSerch(CuDir) == FALSE){
		//���[�U�f�B���N�g���̍쐬
		CreateDirectory(CuDir, NULL);
		CopyDirTree(NULL, DefItem, DefDirPath, (*CmdUser != '\0' || *GeneralUser == '\0') ? CurrentUser : GeneralUser, NULL);
	}
	SetEnvironmentVariable("USER_DIR", CuDir);
}


/******************************************************************************

	CommandLineFunc

	�R�}���h���C���̉��

******************************************************************************/

static void CommandLineFunc(HWND hWnd, char *StrCmdLine)
{
	HTREEITEM hItem = NULL;
	char *p, *r;
	char *buf;
	BOOL CancelFlag = FALSE;

	if(StrCmdLine == NULL || *StrCmdLine == '\0'){
		return;
	}

	buf = (char *)LocalAlloc(LMEM_FIXED, lstrlen(StrCmdLine) + 1);
	if(buf == NULL) return;
	*buf = '\0';

	p = StrCmdLine;
	while(*p != '\0'){
		for(; *p == ' '; p++);
		if(*p == '\0') break;

		if(*p != '/'){
			if(*p == '"'){
				p++;
				for(r = p; *r != '\0' && *r != '"'; r++);
			}else{
				for(r = p; *r != '\0' && *r != ' '; r++);
			}
			lstrcpyn(buf, p, r - p + 1);
			p = r;
			if(*p != '\0') p++;

			if(CancelFlag == TRUE){
				continue;
			}
			if(str_match("*.dll", buf) == TRUE){
				SelectDll(hWnd, buf);
				continue;
			}
			if(hItem == NULL){
				//�ǉ�����t�H���_�̑I��
				hItem = SelectFolderItem(hWnd, TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)));
				UpdateItemFlag = UF_COPY;
				if(hItem == TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE))){
					ListView_SetItemState(GetDlgItem(hWnd, WWWC_LIST), -1, 0, LVIS_SELECTED);
				}
			}
			if(hItem == NULL){
				CancelFlag = TRUE;
				continue;
			}
			//�A�C�e���̒ǉ�
			if(GetProtocolIndex(buf) != -1){
				Item_UrlAdd(hWnd, NEWITEMNAME, buf, 1, hItem);
			}else{
				if(GetDirSerch(buf) == TRUE){
					DragDrop_GetDropTree(hWnd, buf, hItem);
				}else{
					DragDrop_GetDropItemFiles(hWnd, buf, hItem);
				}
				ListView_RefreshFolder(hWnd);
				if(hItem == TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE))){
					//�ǉ��A�C�e���̃t�H�[�J�X�ݒ�
					if(ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), -1, LVNI_SELECTED) != -1){
						ListView_SetItemState(GetDlgItem(hWnd, WWWC_LIST),
							ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), -1, LVNI_SELECTED),
							LVIS_SELECTED | LVIS_FOCUSED, LVIS_FOCUSED | LVIS_SELECTED);
						ListView_EnsureVisible(GetDlgItem(hWnd, WWWC_LIST),
							ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), -1, LVNI_SELECTED), TRUE);
					}
				}
			}
			//�R�s�[���L�����Z������Ă���ꍇ
			if(UpdateItemFlag == UF_CANCEL){
				CancelFlag = TRUE;
			}
		}else{
			for(p++; *p != '\0' && *p != ' '; p++){
				switch(*p)
				{
				case 'c':
				case 'C':
					//�S�ẴA�C�e�����`�F�b�N
					SendMessage(hWnd, WM_COMMAND, ID_MENUITEM_ALLCHECK, 0);
					break;

				case 'i':
				case 'I':
					//�S�ẴA�C�e����������
					SendMessage(hWnd, WM_COMMAND, ID_MENUITEM_ALLINITICON_NOMSG, 0);
					break;

				case 's':
				case 'S':
					//�S�Ẵ`�F�b�N�𒆎~
					SendMessage(hWnd, WM_COMMAND, ID_MENUITEM_ALLCHECKEND, 0);
					break;

				case 'e':
				case 'E':
					//����`�F�b�N��I��
					CmdCheckEnd = TRUE;
					break;

				case 'n':
				case 'N':
					//�X�V������Ƃ��͏���`�F�b�N�I���𖳌�
					CmdNoUpCheckEnd = TRUE;
					break;

				case 'd':
				case 'D':
				case 'u':
				case 'U':
					//��ƃf�B���N�g���A���[�U���̎w��
					p++;
					if(*p != ':') break;
					p++;
					if(*p == '"'){
						p++;
						for(; *p != '\0' && *p != '"'; p++);
						if(*p != '\0') p++;
					}else{
						for(; *p != '\0' && *p != ' '; p++);
					}
					break;
				}
				if(*p == '\0') break;
			}
		}
	}
	LocalFree(buf);
}


/******************************************************************************

	StartCommandLine

	���s���p�R�}���h���C���̉��

******************************************************************************/

static void StartCommandLine(char *StrCmdLine)
{
	char *p, *r;

	p = StrCmdLine;
	while(*p != '\0'){
		for(; *p == ' '; p++);
		if(*p == '\0') break;

		if(*p != '/'){
			if(*p == '"'){
				p++;
				for(p++; *p != '\0' && *p != '"'; p++);
			}else{
				for(; *p != '\0' && *p != ' '; p++);
			}
			if(*p != '\0') p++;
			continue;
		}

		for(p++; *p != '\0' && *p != ' '; p++){
			switch(*p)
			{
			case 'a':
			case 'A':
				//���ɋN�����Ă���E�B���h�E���A�N�e�B�u���Ȃ�
				NoActiveFlag = TRUE;
				break;

			case 'd':
			case 'D':
				//��ƃf�B���N�g���̎w��
				p++;
				if(*p != ':') break;
				p++;
				if(*p == '"'){
					p++;
					for(r = p; *r != '\0' && *r != '"'; r++);
				}else{
					for(r = p; *r != '\0' && *r != ' '; r++);
				}
				lstrcpyn(CmdDirPath, p, r - p + 1);
				p = r;
				if(*p == '\"') p++;
				break;

			case 'u':
			case 'U':
				//���[�U���̎w��
				p++;
				if(*p != ':') break;
				p++;
				if(*p == '"'){
					p++;
					for(r = p; *r != '\0' && *r != '"'; r++);
				}else{
					for(r = p; *r != '\0' && *r != ' '; r++);
				}
				lstrcpyn(CmdUser, p, r - p + 1);
				p = r;
				if(*p == '\"') p++;
				break;
			}
			if(*p == '\0') break;
		}
	}
}


/******************************************************************************

	InitApplication

	�E�B���h�E�N���X�̓o�^

******************************************************************************/

static BOOL InitApplication(HANDLE hInstance)
{
	WNDCLASS  wc;

	// �E�B���h�E�N���X��o�^
	wc.style = 0;
	wc.lpfnWndProc = (WNDPROC)MainProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON_MAIN));
	wc.hCursor = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CURSOR_SEP));
	wc.hbrBackground = (void*)COLOR_BTNSHADOW;
	wc.lpszMenuName = MAKEINTRESOURCE(IDR_MENU_MAIN);
	wc.lpszClassName = WWWC_WNDCLASS;
	return (RegisterClass(&wc));
}


/******************************************************************************

	InitInstance

	�E�B���h�E�̍쐬

******************************************************************************/

static HWND InitInstance(HANDLE hInstance, int nCmdShow)
{
	HWND hWnd;

	// �E�B���h�E���쐬
	hWnd = CreateWindow(
		WWWC_WNDCLASS,
		APP_NAME,
		WS_OVERLAPPEDWINDOW,
		WinRect.left,
		WinRect.top,
		WinRect.right,
		WinRect.bottom,
		NULL,
		NULL,
		hInstance,
		NULL
		);

	if(hWnd == NULL){
		return (NULL);
	}

	if(TrayIcon == 1 && TrayWinShow == 1){
		 return hWnd;
	}

	// �E�B���h�E��\��
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	return hWnd;
}


/******************************************************************************

	initWinsock

	WinSock�̏�����

******************************************************************************/

static BOOL initWinsock(void)
{
	WORD wVersionRequested;
	int  nErrorStatus;
	WSADATA wsaData;

	wVersionRequested = MAKEWORD(1, 1);				// �o�[�W���� 1.1 ��v��
	nErrorStatus = WSAStartup(wVersionRequested, &wsaData);

	if(nErrorStatus != 0){
		return FALSE;
	}
	return TRUE;
}


/******************************************************************************

	AppExit

	�I������

******************************************************************************/

void __cdecl AppExit(void)
{
	// OLE �̏I������
	OleUninitialize();
	// WinSock �̏I������
	WSACleanup();
	//GetDiskFreeSpaceEx �p�ɓǂݍ��񂾃��C�u�����̉��
	FreeGetDiskFreeSpaceEx();

	//�v���g�R���������
	FreeProtocol();
	//�c�[���������
	FreeTool();
}


/******************************************************************************

	MessageFunc

	���b�Z�[�W����

******************************************************************************/

static BOOL MessageFunc(HWND hWnd, MSG *msg)
{
	HWND fWnd;

	fWnd = GetForegroundWindow();
	//�c�[���p�A�N�Z�����[�^
	if(AccelFlag == FALSE && hToolAccel != NULL && fWnd == hWnd &&
		TranslateAccelerator(fWnd, hToolAccel, msg) == TRUE){
		return TRUE;
	}
	//�E�B�h�E�p�A�N�Z�����[�^
	if(AccelFlag == FALSE && TranslateAccelerator(fWnd,
		((fWnd == FindWnd) ? hFindAccel : hAccel), msg) == TRUE){
		return TRUE;
	}
	//�_�C�A���O���b�Z�[�W
	if(AccelFlag == FALSE && IsDialogMessage(fWnd, msg) != 0){
		return TRUE;
	}
	TranslateMessage(msg);
	DispatchMessage(msg);
	return TRUE;
}


/******************************************************************************

	WinMain

	���C��

******************************************************************************/

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	MSG msg;
	HWND hWnd;
	HANDLE hMutex = NULL;
	INITCOMMONCONTROLSEX iccex;
	HINSTANCE hLib;
	FARPROC _InitCommonControlsEx;

	if((hMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, MUTEX)) == NULL){
		hMutex = CreateMutex(NULL, TRUE, MUTEX);
	}else{
		CloseHandle(hMutex);
		hMutex = NULL;
	}
	g_hinst = hInstance;

	StartCommandLine(lpCmdLine);
	GetAppPath(hInstance);

	// CommonControl�̏��������s��
	InitCommonControls();
	// �V����CommonControl�̏��������s��
	hLib = LoadLibrary("comctl32.dll");
	if(hLib != NULL){
		_InitCommonControlsEx = GetProcAddress((HMODULE)hLib, "InitCommonControlsEx");
		if(_InitCommonControlsEx != NULL){
			iccex.dwSize = sizeof(INITCOMMONCONTROLSEX);
			iccex.dwICC = ICC_COOL_CLASSES | ICC_BAR_CLASSES;
			_InitCommonControlsEx(&iccex);
		}
		FreeLibrary(hLib);
	}
	// OLE�̏��������s��
	if(OleInitialize(NULL) != S_OK){
		MessageBox(NULL, EMSG_OLEINIT, APP_NAME, MB_OK | MB_ICONERROR);
		return 0;
	}
	// WinSock�̏��������s��
	if(!initWinsock()){
		MessageBox(NULL, EMSG_WINSOCKINIT, APP_NAME, MB_OK | MB_ICONERROR);
		return 0;
	}
	//GetDiskFreeSpaceEx �̃A�h���X�擾
	InitGetDiskFreeSpaceEx();

	//�ݒ��ǂݍ���
	GetINI();
	GetProtocolInfo();

	//�I������
	if(atexit(AppExit)){
		MessageBox(NULL, EMSG_ATEXIT, APP_NAME, MB_OK | MB_ICONERROR);
		return 0;
	}

	//�Q�d�N���̃`�F�b�N���s��
	if(hMutex == NULL && DoubleStart == 1){
		if(DoubleStartMsg == 1){
			//���b�Z�[�W�\���̂�
			MessageBox(NULL, IMSG_DOUBLESTART, APP_NAME, MB_OK | MB_ICONWARNING);
			return 0;
		}
		//�N�����̃E�B���h�E������
		hWnd = FindWindow(WWWC_WNDCLASS, NULL);
		if(hWnd == NULL){
			return 0;
		}
		//�\�����ăA�N�e�B�u�ɂ���
		if(NoActiveFlag == FALSE){
			ShowWindow(hWnd, SW_SHOW);
			if(IsZoomed(hWnd) == 0){
				ShowWindow(hWnd, SW_RESTORE);
			}
			_SetForegroundWindow(hWnd);
		}

		//�R�}���h���C���̑��M
		if(lpCmdLine != NULL && *lpCmdLine != TEXT('\0')){
			COPYDATASTRUCT cpdata;

			cpdata.lpData = lpCmdLine;
			cpdata.cbData = lstrlen(lpCmdLine) + 1;
			SendMessage(hWnd, WM_COPYDATA, (WPARAM)NULL, (LPARAM)&cpdata);
		}else if(StartCheck == 1){
			//�N�����`�F�b�N
			SendMessage(hWnd, WM_COMMAND, ID_MENUITEM_ALLCHECK, 0);
		}
		return 0;
	}

	//���\�[�X����|�b�v�A�b�v���j���[�����[�h
	hPOPUP = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MENU_POPUP));
	//�N���b�v�{�[�h��WWWC�f�[�^�̌`����o�^
	WWWC_ClipFormat = RegisterClipboardFormat(CLIPBOARDFORMAT);

	// �E�B���h�E�N���X�̓o�^���s��
	if(!InitApplication(hInstance)){
		DestroyMenu(hPOPUP);
		if(hMutex != NULL){
			CloseHandle(hMutex);
		}
		MessageBox(NULL, EMSG_REGWINDOWCLASS, APP_NAME, MB_OK | MB_ICONERROR);
		return 0;
	}

	// �E�B���h�E�̍쐬���s��
	if((hWnd = InitInstance(hInstance, nCmdShow)) == NULL){
		DestroyMenu(hPOPUP);
		if(hMutex != NULL){
			CloseHandle(hMutex);
		}
		MessageBox(NULL, EMSG_CREATEWINDOW, APP_NAME, MB_OK | MB_ICONERROR);
		return 0;
	}

	//���\�[�X����A�N�Z���[�^�����[�h
	hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR_WWWC));
	hFindAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR_FIND));

	CommandLineFunc(hWnd, lpCmdLine);

	// �E�B���h�E���b�Z�[�W������
	while(GetMessage(&msg, NULL, 0, 0)){
		MessageFunc(hWnd, &msg);
	}

	DestroyMenu(hPOPUP);
	if(hMutex != NULL){
		CloseHandle(hMutex);
		hMutex = NULL;
	}
	UnregisterClass(WWWC_WNDCLASS, hInstance);
	return 0;
}
/* End of source */
