/**************************************************************************

	WWWC

	General.h

	Copyright (C) 1996-2018 by Ohno Tomoaki. All rights reserved.
		https://www.nakka.com/
		nakka@nakka.com

**************************************************************************/

#ifndef _INC_WWWC_GENERAL_H
#define _INC_WWWC_GENERAL_H

/**************************************************************************
	Include Files
**************************************************************************/

#include "commctrl.h"
#include "resource.h"

#include "StrTbl.h"


/**************************************************************************
	Define
**************************************************************************/

#define BUFSIZE						256					// バッファサイズ
#define MAXSIZE						32768				// バッファサイズ

#define APP_NAME					"WWWC"
#define APP_VERSION					"1.1.3"
#define APP_COPYRIGHT				"Copyright (C) 1996-2018 by Ohno Tomoaki. All rights reserved."
#define APP_URL						"https://www.nakka.com/"
#define APP_MAIL					"nakka@nakka.com"
#define BANNER_URL					""

#define GENERAL_INI					"general.ini"
#define USER_INI					"user.ini"

#define DATAFILENAME				"Item.dat"
#define FOLDERFILENAME				"Folder.ini"

#define SIDEBYSIDE_COMMONCONTROLS	1					//Windows XP Visual Style

#define WM_WSOCK_GETHOST			(WM_USER + 1)		//ソケットメッセージ
#define WM_WSOCK_SELECT				(WM_USER + 2)

#define WM_CHECK_RESULT				(WM_USER + 3)		//チェック処理メッセージ
#define WM_ITEMCHECK				(WM_USER + 4)
#define WM_NEXTCHECK				(WM_USER + 9)		//次のチェック
#define WM_CHECK_END				(WM_USER + 60)

#define WM_GETVERSION				(WM_USER + 5)		//データ要求メッセージ
#define WM_GETCHECKLIST				(WM_USER + 6)
#define WM_GETCHECKLISTCNT			(WM_USER + 7)
#define WM_SETITEMLIST				(WM_USER + 8)
#define WM_GETMAINWINDOW			(WM_USER + 10)
#define WM_GETMAINITEM				(WM_USER + 11)
#define WM_ITEMEXEC					(WM_USER + 12)
#define WM_ITEMINIT					(WM_USER + 13)
#define WM_GETUPWINDOW				(WM_USER + 14)
#define WM_GETFINDWINDOW			(WM_USER + 15)

#define WM_WWWC_GETINI				(WM_USER + 20)		//INIファイル処理メッセージ
#define WM_WWWC_PUTINI				(WM_USER + 21)
#define WM_WWWC_GETINIPATH			(WM_USER + 22)

#define WM_FOLDER_SAVE				(WM_USER + 31)		//フォルダ操作メッセージ
#define WM_FOLDER_LOAD				(WM_USER + 32)
#define WM_FOLDER_GETPATH			(WM_USER + 33)
#define WM_FOLDER_GETWWWCPATH		(WM_USER + 34)
#define WM_FOLDER_SELECT			(WM_USER + 35)
#define WM_FOLDER_REFRESH			(WM_USER + 36)

#define WM_DOEVENTS					(WM_USER + 50)


#define WM_ADDPROTOCOL				(WM_USER + 100)
#define WM_ADDTOOL					(WM_USER + 101)

#ifndef WM_THEMECHANGED
#define WM_THEMECHANGED				0x031A
#endif


#define Clip_DirName				"~WWWC_Clip_Files"	//クリップボード用作業ディレクトリ
#define DnD_DirName					"~WWWC_DnD_Files"	//ドラッグ＆ドロップ用作業ディレクトリ

#define IDPCANCEL					(WM_USER + 203)		//プロパティシートのキャンセルメッセージ

#define TBDEFBUTTON					"0,,1,2,5,6,,7,8,,9,10,11,,12,,13,14,,15,16,17,18,"		//デフォルトツールバー
#define TBSTYLE_FLAT				0x0800

#define LICONSIZE					32					//大きいアイコンのサイズ
#define SICONSIZE					16					//小さいアイコンのサイズ

#define TRAY_ID						100					//タスクトレイID
#define FRAME_CNT					2					//境界フレーム数
#define STEPSIZE					22					//アニメーションビットマップのサイズ
#define CLIPFORMAT_CNT				3					//クリップボードフォーマット数

#define CHECK_MAX					30					//最大チェック数
#define NEXTCHECK_INTERVAL			1					//次チェックアイテム検索タイマー
#define TIMEOUT_INTERVAL			1000				//タイムアウト用タイマー
#define CHECKANI_INTERVAL			150					//アニメーションタイマー

#define CHECKINI_CHECK				0					//チェックの初期化
#define CHECKINI_AUTOALLCHECK		1
#define CHECKINI_AUTOCHECK			2
#define CHECKINI_ALLCHECK			3
#define CHECKINI_TREECHECK			4
#define CHECKINI_DLLCHECK			5


#define AUTOTIME_NON				0					//自動チェックタイプ
#define AUTOTIME_MIN				1
#define AUTOTIME_HOU				2
#define AUTOTIME_DAY				3
#define AUTOTIME_WEEK				4
#define AUTOTIME_MON				5


#define ID_MENU_NEWITEM				51000				//動的メニューID
#define ID_MENU_ACTION_OPEM			52000
#define ID_MENU_ACTION_RECY			52001
#define ID_MENU_ACTION				52002
#define ID_MENU_TOOL_ACTION			54000
#define ID_WMENU_TOOL_ACTION		55000
#define ID_ACCEL_TOOL_ACTION		56000


#define TIMER_SEP					1					//タイマーID
#define TIMER_CHECKTIMEOUT			2
#define TIMER_CHECK					3
#define TIMER_AUTOCHECK				4
#define TIMER_SBTEXT				5
#define TIMER_ANI					6
#define TIMER_TREEEXPAND			7
#define TIMER_NEXTFOCUS				8


//#define WWWC_TREE					100					//コントロールID
//#define WWWC_LIST					101
#define WWWC_TB						102
#define WWWC_SB						103


//General Icon
#define ICON_CHECK					0					//アイコンインデックス
#define ICON_NOCHECK				1
#define ICON_DIRUP					2
#define ICON_DIR_CLOSE				3
#define ICON_DIRUPCHILD				4
#define ICON_CHECKCHILD				5
#define ICON_DIR_CLOSE_CH			6
#define ICON_DIR_CLOSE_UP			7
#define ICON_DIR_CLOSE_UPCHILD		8
#define ICON_DIR_CLOSE_CHECKCHILD	9
#define ICON_DIR_RECYCLER			10
#define ICON_DIR_RECYCLER_USE		11

//TreeView Icon
#define ICON_INET					12
#define ICON_DIR_OPEN				13
#define ICON_DIR_OPEN_CH			14
#define ICON_DIR_OPEN_UP			15
#define ICON_DIR_OPEN_UPCHILD		16
#define ICON_DIR_OPEN_CHECKCHILD	17

//ListView Icon
#define ICON_UP						12
#define ICON_ERR					13
#define ICON_TIMEOUT				14
#define ICON_NOICON					15
#define ICON_WAIT					16

//Overlay Mask Icon
#define ICON_ST_NOCHECK				1


#define ST_DEFAULT					0					//アイテムの状態
#define ST_UP						1
#define ST_ERROR					2
#define ST_TIMEOUT					4

#define ST_NOCHECK					1					//アイテムのチェック状態
#define ST_CHECK					2


//update flag
#define UF_NOMSG					0					//上書きメッセージ
#define UF_COPY						1
#define UF_ALLCOPY					2
#define UF_NOCOPY					3
#define UF_ALLNOCOPY				4
#define UF_CANCEL					-1

//ListView draw flag
#define LDF_NODRAW					0					//リストビュー描画フラグ
#define LDF_REDRAW					1

//TreeView item status
#define TREEICON_UP					1
#define TREEICON_UPCHILD			2
#define TREEICON_CH					4
#define TREEICON_CHECKCHILD			8

#define WM_UP_INIT					(WM_USER + 100)
#define WM_UP_FREE					(WM_USER + 101)
#define WM_UP_CLOSE					(WM_USER + 102)
#define WM_UP_WININI				(WM_USER + 103)

#define WM_LV_EVENT					(WM_USER + 300)		//リストビューイベント
#define WM_LV_INITICON				(WM_USER + 302)

#define WM_TV_EVENT					(WM_USER + 310)		//ツリービューイベント

#define WM_TB_REFRESH				(WM_USER + 320)		//ツールバー更新

#define WM_DRAGMSG					(WM_USER + 500)		//ドラッグ＆ドロップ
#define WM_DATAOBJECT_GETDATA		(WM_USER + 501)		//ドロップデータ要求イベント


#define CHECK_ERROR					-1					//チェック結果
#define CHECK_SUCCEED				0
#define CHECK_NO					1
#define CHECK_END					2


#define MENU_HWND_FILE				0					//メニューインデックス
#define MENU_HWND_EDIT				1
#define MENU_HWND_VIEW				2
#define MENU_HWND_CHECK				3
#define MENU_HWND_TOOL				4

#define MENU_POP_NEW				0
#define MENU_POP_ITEM				1
#define MENU_POP_FOLDER				2
#define MENU_POP_DGDP				3
#define MENU_POP_UPITEM				4
#define MENU_POP_TASKTRAY			5
#define MENU_POP_UPMSG				6


#define FLAG_COPY					1					//クリップボード操作フラグ
#define FLAG_CUT					2
#define FLAG_FOLDER					3

#define DND_MOVE					0					//ドラッグ＆ドロップ操作フラグ
#define DND_COPY					1
#define DND_RECY					2

#define UNDO_ITEM					1					//Undo種別
#define UNDO_ITEM_COPY				2
#define UNDO_ITEM_DELETE			3
#define UNDO_LV_TITLE				4
#define UNDO_TV_TITLE				5


#define TOOL_EXEC_PORP				0					//プロパティ
#define TOOL_EXEC_ITEMMENU			1					//ツール実行フラグ
#define TOOL_EXEC_WINDOWMENU		2
#define TOOL_EXEC_START				4
#define TOOL_EXEC_END				8
#define TOOL_EXEC_CHECKSTART		16
#define TOOL_EXEC_CHECKEND			32
#define TOOL_EXEC_CHECKENDUP		64
#define TOOL_EXEC_CHECKENDNOUP		128
#define TOOL_EXEC_SYNC				256
#define TOOL_EXEC_INITITEM			512
#define TOOL_EXEC_MENUDEFAULT		1024
#define TOOL_EXEC_NOTCHECK			2048
#define TOOL_EXEC_SAVEFOLDER		4096

#define TOOL_HIDE_ITEMMENU			1					//ツール非表示フラグ
#define TOOL_HIDE_WINDOWMENU		2
#define TOOL_HIDE_START				4
#define TOOL_HIDE_END				8
#define TOOL_HIDE_CHECKSTART		16
#define TOOL_HIDE_CHECKEND			32
#define TOOL_HIDE_CHECKENDUP		64
#define TOOL_HIDE_CHECKENDNOUP		128
#define TOOL_HIDE_SYNC				256
#define TOOL_HIDE_INITITEM			512
#define TOOL_HIDE_MENUDEFAULT		1024
#define TOOL_HIDE_NOTCHECK			2048
#define TOOL_HIDE_SAVEFOLDER		4096
#define TOOL_HIDE_HOTKEY			8192
#define TOOL_HIDE_MENUINDEX			16384
#define TOOL_HIDE_PROTOCOL			32768


#define CHECKTYPE_ITEM				1
#define CHECKTYPE_ALL				2
#define CHECKTYPE_TREE				4
#define CHECKTYPE_ERROR				8
#define CHECKTYPE_AUTO				16


#define ABS(n)						((n < 0) ? (n * -1) : n)	//絶対値


/**************************************************************************
	Struct
**************************************************************************/

//自動チェック
struct TPCHECKTIME{
	int type;											//自動チェックの種別
	int flag;											//チェックする／しない フラグ

	int day;											//自動チェック開始日時
	int week;
	int h;
	int m;

	int mCnt;											//分毎
	int hCnt;											//時間毎
};

//アイテム情報
struct TPITEM{
	long iSize;											//構造体のサイズ

	HTREEITEM hItem;									//このアイテムが入ってるフォルダ

	char *Title;										//タイトル
	char *CheckURL;										//チェックを行うURL
	char *Size;											//サイズ
	char *Date;											//更新日時
	int Status;											//アイテムの状態 (ST_)

	char *CheckDate;									//チェックした日時
	char *OldSize;										//旧更新日
	char *OldDate;										//旧サイズ

	char *ViewURL;										//表示するURL
	char *Option1;										//オプション 1
	char *Option2;										//オプション 2
	char *Comment;										//コメント

	int CheckSt;										//チェックフラグ

	char *ErrStatus;									//エラー状態
	char *DLLData1;										//DLL用データ
	char *DLLData2;										//DLL用データ

	//以下保存されない情報
	int IconStatus;										//アイコンの状態 (ICON_ST_)

	int Soc1;											//ソケット1
	int Soc2;											//ソケット2
	HANDLE hGetHost1;									//ホスト情報のハンドル1
	HANDLE hGetHost2;									//ホスト情報のハンドル1
	long Param1;										//DLL用long値1
	long Param2;										//DLL用long値2
	long Param3;										//DLL用long値3
	long Param4;										//DLL用long値4
	int user1;											//DLL用int値1
	int user2;											//DLL用int値2

	BOOL RefreshFlag;
//	BOOL DeleteFlag;
};

//フォルダ情報
struct TPTREE{
	int CheckFlag;										//チェックフラグ
	int MemFlag;										//解放防止カウンタ
	int Icon;											//状態アイコン
	int Expand;

	int CheckSt;										//チェックする／しない
	int AutoCheckSt;									//デフォルトのチェックを使用する／しない
	int CheckMax;										//フォルダ毎チェック数

	char *Comment;										//コメント

	struct TPCHECKTIME *tpCheckTime;					//自動チェック設定
	int tpCheckTimeCnt;
	struct TPITEM **ItemList;							//アイテムリスト
	int ItemListCnt;
};

//プロトコルメニュー
struct TPPROTOCOLMENU{
	char name[BUFSIZE];
	char Action[BUFSIZE];
	BOOL Default;
	int Flag;
};

//プロトコル設定取得用
struct TPPROTOCOLSET{
	long iSize;							/* 構造体のサイズ */

	char Title[BUFSIZE];
	char FuncHeader[BUFSIZE];
	char IconFile[BUFSIZE];
	int IconIndex;
	char UpIconFile[BUFSIZE];
	int UpIconIndex;
};

//プロトコル情報取得用
struct TPPROTOCOLINFO{
	long iSize;							/* 構造体のサイズ */

	char scheme[BUFSIZE];
	char NewMenu[BUFSIZE];
	char FileType[BUFSIZE];
	struct TPPROTOCOLMENU *tpMenu;
	int tpMenuCnt;
};

//プロトコル情報
struct TP_PROTOCOL{
	char scheme[BUFSIZE];
	char NewMenu[BUFSIZE];
	char FileType[BUFSIZE];
	struct TPPROTOCOLMENU *tpMenu;
	int tpMenuCnt;

	char title[BUFSIZE];
	char DLL[BUFSIZE];
	char FuncHeader[BUFSIZE];
	char IconFile[BUFSIZE];
	int IconIndex;
	char UpIconFile[BUFSIZE];
	int UpIconIndex;
	int Icon;

	HANDLE lib;
	FARPROC Func_Initialize;
	FARPROC Func_Start;
	FARPROC Func_Gethost;
	FARPROC Func_Select;
	FARPROC Func_Timer;
	FARPROC Func_Cancel;
	FARPROC Func_ItemCheckEnd;
	FARPROC Func_CreateDropFile;
	FARPROC Func_DropFile;
	FARPROC Func_ExecItem;
	FARPROC Func_InitItem;
	FARPROC Func_FreeItem;
};

//リストビューカラム
struct TPLVCOLUMN{
	char Title[BUFSIZE];
	int fmt;
	long p;
};

//ツール取得用
struct TP_GETTOOL{
	long iSize;				/* 構造体のサイズ */

	char title[BUFSIZE];
	char func[BUFSIZE];
	char Protocol[BUFSIZE];
	int MenuIndex;
	int Action;
};

//ツール
struct TP_TOOLS{
	long iSize;				/* 構造体のサイズ */

	char title[BUFSIZE];
	char func[BUFSIZE];
	char Protocol[BUFSIZE];
	int MenuIndex;
	int Action;

	int Ctrl;
	int Key;

	char FileName[BUFSIZE];
	char CommandLine[BUFSIZE];

	HANDLE lib;
	FARPROC Func_Tool;
};


/**************************************************************************
	Function Prototypes
**************************************************************************/
//--------------------------
//	String
//--------------------------
char *AllocCopy(char *buf);
BOOL Trim(char *buf);
char *iStrCpy(char *ret, char *buf);
int lstrcmpn(char *buf1, char *buf2, int len);
int lstrcmpni(char *buf1, char *buf2, int len);
BOOL str_match(const TCHAR *ptn, const TCHAR *str);
BOOL strlistcmp(char *buf, char *Format, char Sep);
BOOL FileNameCheck(char *buf);
void FileNameConv(char *buf, char NewChar);
void EscToCode(char *buf);
char *CodeToEsc(char *buf);
void GetNumString(char *buf, char *ret);
void GetDateTime(char *fDay, char *fTime);
char *GetIndexToString(struct TPITEM *tpItemInfo, int Index);

//--------------------------
//	File
//--------------------------
long GetFileSerchSize(char *FileName);
BOOL GetFileMakeDay(char *FileName, char *buf);
BOOL GetDirSerch(char *fPath);
BOOL GetPathToFilename(char *fPath, char *ret);
int CopyDirTree(HWND hWnd, char *Path, char *NewPath, char *name, char *ret);
BOOL DeleteDirTree(char *Path, BOOL AllFlag);
void SetDirTree(HWND hTreeView, char *Path, HTREEITEM hItem);
void CALLBACK FindTreeDir(HWND hWnd, HTREEITEM hItem, long Param);
void SaveDirTree(HWND hTreeView, char *Path, HTREEITEM hItem);
void mkDirStr(char *buf);
char *LineSetItemInfo(struct TPITEM *tpItemList, char *t);
struct TPITEM **ReadItemList(char *FileName, int *cnt, HTREEITEM hItem);
BOOL ReadTreeMem(HWND hWnd, HTREEITEM hItem);
int ItemSize(struct TPITEM *tpItemInfo);
char *SetItemString(struct TPITEM *tpItemInfo, char *p);
int SaveItemList(HWND hWnd, char *FileName, struct TPITEM **tpItemList, int ItemCnt);
BOOL SaveTreeMem(HWND hWnd, HWND hTreeView, HTREEITEM hItem);
void InitGetDiskFreeSpaceEx();
void FreeGetDiskFreeSpaceEx();
int FileSelect(HWND hDlg, char *oFile, char *oFilter, char *oTitle, char *ret, char *def, int Index);
int ExecItemFile(HWND hWnd, char *FileName, char *CommandLine, struct TPITEM *tpItemInfo, int SyncFlag);
HDROP CreateDropFileMem(char **FileName, int cnt);

//--------------------------
//	Ini
//--------------------------
void GetINI(void);
void PutINI(void);
void GetProtocolInfo(void);
void PutProtocolInfo(void);
void GetAutoTime(void);
void PutAutoTime(void);
void GetToolList(void);
void PutToolList(void);
void GetDirInfo(HWND hTreeView, char *path, HTREEITEM hItem);
void PutDirInfo(HWND hTreeView, char *path, HTREEITEM hItem);

//--------------------------
//	Item
//--------------------------
void FreeItemInfo(struct TPITEM *TmpItemInfo, BOOL ProtocolFreeFlag);
void FreeItemList(struct TPITEM **tpItemInfo, int ItemCnt, BOOL ProtocolFreeFlag);
struct TPITEM *GetMainItem(HWND hWnd, struct TPTREE *tpTreeInfo, struct TPITEM *SelItemInfo);
struct TPITEM *FindMainItem(HWND hWnd, struct TPITEM *SelItemInfo);
int Item_Open(HWND hWnd, int ID);
BOOL Item_DefaultOpen(HWND hWnd, struct TPITEM *tpItemInfo);
void Item_ContentCopy(struct TPITEM *NewItemInfo, struct TPITEM *TmpItemInfo);
struct TPITEM *Item_Copy(struct TPITEM *TmpItemInfo);
void Item_CopyMainContent(HWND hWnd, struct TPITEM **tpItemList, int cnt);
void Item_CopyMainRefreshContent(HWND hWnd, struct TPITEM **tpItemList, int cnt);
BOOL Item_Select(HWND hWnd, HTREEITEM hItem);
struct TPITEM **Item_ProtocolSelect(struct TPITEM **tpItemList, int *cnt, char *protocol);
int Item_Add(HWND hWnd, HTREEITEM hItem, struct TPITEM *NewItemInfo);
BOOL Item_UrlAdd(HWND hWnd, char *Title, char *buf, int NameFlag, HTREEITEM hItem);
BOOL Item_ListAdd(HWND hWnd, struct TPITEM **NewItemInfo, int cnt);
BOOL Item_Delete(struct TPTREE *tpTreeInfo, struct TPITEM *tpItemInfo);
BOOL Item_Create(HWND hWnd, int ProtocolIndex);
BOOL Item_Property(HWND hWnd);
int Item_UpCount(HWND hWnd, HTREEITEM hItem);
int Item_Initialize(HWND hWnd, struct TPITEM *tpItemInfo, BOOL InitIconFlag);
void ListItemIni(HWND hWnd);
void Item_ItemListIni(HWND hWnd, struct TPITEM **tpItemList, int cnt);
void Item_MainItemIni(HWND hMainWnd, HWND hWnd, struct TPITEM **tpItemList, int cnt);
void CALLBACK TreeItemIni(HWND hWnd, HTREEITEM hItem, long Param);
BOOL Item_ChangeState(HWND hWnd, int St);
BOOL Item_SwitchCheckState(HWND hWnd, HTREEITEM hItem);
BOOL DeleteItem(HWND hWnd);
BOOL CheckRecycler(HWND hWnd);
BOOL ClearRecycler(HWND hWnd, BOOL Flag);

//--------------------------
//	ImageList
//--------------------------
int ImageListIconAdd(HIMAGELIST IconList, int Index, int IconSize, char *buf, int iIndex);
int ImageListFileIconAdd(HIMAGELIST IconList, char *FileName, UINT uFlags, int IconSize,char *buf, int iIndex);

//--------------------------
//	TreeView
//--------------------------
void CallTreeItem(HWND hWnd, HTREEITEM hItem, FARPROC Func, long Param);
HWND CreateTreeView(HWND hWnd);
void TreeView_Initialize(HWND hWnd);
HTREEITEM TreeView_AllocItem(HWND hTreeView, HTREEITEM pItem, HTREEITEM After, char *Title, int Icon, int IconSel);
BOOL TreeView_NewFolderItem(HWND hWnd);
void CALLBACK TreeView_FreeItem(HWND hWnd, HTREEITEM hItem, long Param);
void CALLBACK TreeView_FreeTreeItem(HWND hWnd, HTREEITEM hItem, long Param);
void TreeView_DeleteTreeInfo(HWND hWnd, HTREEITEM hItem);
void CALLBACK TreeView_AllExpand(HWND hWnd, HTREEITEM hItem, long Param);
HTREEITEM TreeView_FindItemPath(HWND hTreeView, HTREEITEM hItem, char *path);
void CALLBACK FindTreeItem(HWND hWnd, HTREEITEM hItem, long Param);
BOOL IsTreeItem(HWND hWnd, HTREEITEM hItem);
HTREEITEM TreeView_CheckName(HWND hTreeView, HTREEITEM hItem, char *str);
int TreeView_GetChildCount(HWND hTreeView, HTREEITEM hItem);
BOOL TreeView_IsRecyclerItem(HWND hTreeView, HTREEITEM hItem);
void TreeView_GetPath(HWND hTreeView, HTREEITEM hItem, char *ret, char *RootString);
BOOL TreeView_SetName(HWND hWnd, HTREEITEM hItem, char *NewName);
int TreeView_GetItemInfo(HWND hTreeView, HTREEITEM hItem, char *buf);
int TreeView_GetListIndex(HWND hTreeView, HTREEITEM hItem);
HTREEITEM TreeView_GetIndexToItem(HWND hTreeView, HTREEITEM hItem, int Index);
HTREEITEM TreeView_GetHiTestItem(HWND hTreeView);
BOOL TreeView_SetlParam(HWND hTreeView, HTREEITEM hItem, long lParam);
void *TreeView_GetlParam(HWND hTreeView, HTREEITEM hItem);
BOOL TreeView_SetState(HWND hTreeView, HTREEITEM hItem, int State, int mask);
int TreeView_GetState(HWND hTreeView, HTREEITEM hItem, int mask);
BOOL TreeView_UpdateList(HWND hWnd, HTREEITEM hItem);
BOOL SetItemIcon(HWND hWnd, HTREEITEM hItem);
void CALLBACK TreeView_SetIconState(HWND hWnd, HTREEITEM hItem, long Param);
void CALLBACK TreeView_NoFileDelete(HWND hWnd, HTREEITEM hItem, long Param);
void TreeView_StartDragItem(HWND hWnd, HTREEITEM hItem);
BOOL TreeView_SelItemChanging(HWND hWnd, HTREEITEM NewItem, HTREEITEM OldItem);
BOOL TreeView_SelItemChanged(HWND hWnd, HTREEITEM NewItem, HTREEITEM OldItem);
BOOL TreeView_CheckHistory(HWND hWnd, BOOL NextFlag);
BOOL TreeView_PrevHistory(HWND hWnd);
BOOL TreeView_NextHistory(HWND hWnd);
LRESULT TreeView_NotifyProc(HWND hWnd, LPARAM lParam);

//--------------------------
//	ListView
//--------------------------
HWND CreateListView(HWND hWnd);
void ListView_SetItemImage(HWND hListView, int IconSize, int LVFLag);
int ListView_AddColumn(HWND hListView, char *chLvColumn, int *lColSize, struct TPLVCOLUMN *ColInfo);
int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
void ListView_SetRedraw(HWND hListView, int DrawFlag);
int ListView_InsertItemEx(HWND LV, char *buf, int Img, long lp, int iItem);
void ListView_SetItemTitle(HWND hWnd, LV_DISPINFO *plv);
void ListView_SetItemIcon(HWND hListView, int i, int Icon);
int ListView_GetFolderCount(HWND hListView);
void ListView_MoveItem(HWND hListView, int SelectItem, int Move, int ColSize);
int ListView_GetMemToIndex(HWND hListView, struct TPITEM *tpItemInfo);
long ListView_GetlParam(HWND hListView, int i);
void ListView_SetlParam(HWND hListView, int i, long lParam);
struct TPITEM **ListView_SetListToMem(HWND hWnd, int *RetItemCnt);
struct TPITEM **ListView_SelectItemToMem(HWND hListView, int *cnt);
int ListView_GetHiTestItem(HWND hListView);
void ListView_SwitchSelectItem(HWND hListView);
void ListView_UpSelectItem(HWND hListView);
BOOL ListView_IsRecyclerItem(HWND hWnd);
void ListView_StyleChange(HWND hWnd, int NewStyle);
void ListView_SetStyle(HWND hWnd, int NewStyle);
BOOL ListView_GetDispItem(HWND hWnd, LV_ITEM *hLVItem);
void ListView_FolderRedraw(HWND hWnd, BOOL SelFlag);
void ListView_RefreshFolder(HWND hWnd);
void ListView_RefreshItem(HWND hListView);
void ListView_ShowItem(HWND hWnd, HTREEITEM hItem);
void ListView_StartDragItem(HWND hWnd);
BOOL ListView_MouseSelectItem(HWND hListView);
void ListView_ItemClick(HWND hWnd, NMHDR *CForm);
int ListView_MenuSort(HWND hWnd, int i);
void ListView_SortClear(const HWND hListView, const int sort_flag);
void ListView_SortSelect(const HWND hListView, const int sort_flag);
LRESULT ListView_NotifyProc(HWND hWnd, LPARAM lParam);

//--------------------------
//	ToolBar
//--------------------------
//ToolBar
void ReTbButtonInfo(void);
void CreateTB(HWND hWnd);
LRESULT ToolBar_NotifyProc(HWND hWnd, LPARAM lParam);
void ShowTbMenu(HWND hWnd, HMENU hMenu, int cmd);

//--------------------------
//	StatusBar
//--------------------------
HWND CreateStatusBar(HWND hWnd);
void SBSetParts(HWND hWnd);
void SetSbText(HWND hWnd);
LRESULT StatusBar_NotifyProc(HWND hWnd, LPARAM lParam);

//--------------------------
//	Option
//--------------------------
#ifdef OP_XP_STYLE
long open_theme(const HWND hWnd, const WCHAR *class_name);
void close_theme(long hTheme);
BOOL draw_theme_scroll(LPDRAWITEMSTRUCT lpDrawItem, UINT i, long hTheme);
#endif	//OP_XP_STYLE
void DrawScrollControl(LPDRAWITEMSTRUCT lpDrawItem, UINT i);
BOOL ShowItemProp(HWND hWnd, struct TPITEM *tpItemInfo);
int FindPropItem(HWND hWnd, HTREEITEM hItem);
LRESULT OptionNotifyProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT DialogLvNotifyProc(HWND hWnd, LPARAM lParam, HWND hListView);
int ViewFolderProperties(HWND hWnd, HTREEITEM hItem);
int ViewProperties(HWND hWnd);

//--------------------------
//	SelectFolder
//--------------------------
BOOL SelectFolder(HWND hWnd, char *ret);
HTREEITEM SelectFolderItem(HWND hWnd, HTREEITEM hItem);

//--------------------------
//	SelectDll
//--------------------------
BOOL SelectDll(HWND hWnd, char *DLLPath);

//--------------------------
//	SelectIcon
//--------------------------
int SelectIcon(HWND hWnd, char *path, int index);

//--------------------------
//	AutoChaeck
//--------------------------
BOOL CheckAutoTime(HWND hWnd);
void CALLBACK TreeCheckAutoTime(HWND hWnd, HTREEITEM hItem, long Param);
void CALLBACK CopyAutoTime(HWND hWnd, HTREEITEM hItem, long Param);
BOOL CALLBACK AutoCheckListProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

//--------------------------
//	Protocol
//--------------------------
int GetProtocolIndex(char *URL);
void FreeProtocol(void);
void SetProtocolInfo(void);
void EndProtocolNotify(void);
void SetNewItemMenu(HWND hWnd);
void DeleteProtocolMenuItem(HMENU pMenu);
void SetProtocolItemMenu(HWND hWnd, HMENU pMenu, BOOL ItemFlag, BOOL UpWndFlag);
void SetProtocolMenu(HWND hWnd);
void SetProtocolImage(HWND hWnd);
BOOL CALLBACK SetProtocolProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

//--------------------------
//	Tool
//--------------------------
void FreeTool(void);
void CreateToolAccelerator(void);
void SetToolMenu(HWND hWnd);
int DllToolExec(HWND hWnd, int i, struct TPITEM **ToolItemList, int ToolItemListCnt, int type, int CheckType);
void ExecTool(HWND hWnd, int id, BOOL MenuFlag);
void SubItemExecTool(HWND hWnd, int id, struct TPITEM **ToolItemList, int cnt, int Action, int CheckType);
BOOL CALLBACK SetToolProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

//--------------------------
//	UpMessage
//--------------------------
BOOL AddUpItem(struct TPITEM *NewItemInfo);
BOOL SetSubItemClipboardData(HWND hWnd);
void StartDragSubItem(HWND hWnd);
BOOL InitSubIcon(HWND hDlg, int Index);
BOOL FindInitSubIcon(HWND hDlg, struct TPITEM *tpItemInfo);
BOOL MainItemSelect(HWND hWnd);
BOOL MainItemProp(HWND hWnd);
void RefreshListView(HWND hDlg);
void ExeAction(HWND hDlg, WPARAM wParam);
void SizeInitUpMessage(HWND hDlg, int Size, int Pos, int Expand);
void SetUpMessageControls(HWND hWnd);
void RefreshListView(HWND hDlg);
BOOL CALLBACK UpMessageProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

//--------------------------
//	FindItem
//--------------------------
BOOL CALLBACK FindProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

//--------------------------
//	Dialog
//--------------------------
HWND CreateTitleWindow(HWND hWnd);
void CloseTitleWindow(HWND tWnd);

BOOL CALLBACK AniProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

//--------------------------
//	Frame
//--------------------------
BOOL FrameInitialize(HWND hWnd);
void FrameFree(void);
int FrameDraw(HWND hWnd);
int FrameDrawEnd(HWND hWnd);

//--------------------------
//	Menu
//--------------------------
void ShowMenu(HWND hWnd, HMENU hMenu, int mpos);
UINT ShowMenuCommand(HWND hWnd, HMENU hMenu, int mpos);
void SetFolderEnableMenu(HWND hWnd, HTREEITEM hItem);
void SetEnableMenu(HWND hWnd);
void SetEnableToolMenu(HWND hWnd);
void SetEnableToolItemMenu(HMENU hMenu);

//--------------------------
//	ClipBoard
//--------------------------
void Clipboard_SetChain(HWND hWnd);
void Clipboard_DeleteChain(HWND hWnd);
void Clipboard_ChangeChain(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void Clipboard_Draw(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL Clipboard_DeleteCutStatus(HWND hWnd);
HANDLE Clipboard_Set_WF_ItemList(HWND hWnd, int mode, HTREEITEM ClipItem);
HANDLE Clipboard_Set_TEXT(HWND hWnd, HTREEITEM hStrItem);
BOOL Clipboard_SetItemData(HWND hWnd, int mode);
int Clipboard_Get_WF_String(HWND hWnd, HTREEITEM hItem, char *buf, int DnDFlag);
int Clipboard_CheckFormat(void);
void Clipboard_GetData(HWND hWnd);

//--------------------------
//	Drag & Drop
//--------------------------
HDROP DragDrop_SetDropFileMem(HWND hWnd);
BOOL DragDrop_CreateTreeDropFiles(HWND hWnd, HTREEITEM hItem, char *TempDirName);
BOOL DragDrop_CreateDropFiles(HWND hWnd, char *TempDirName);
void FreeDropFiles(void);
void DragDrop_GetDropItemFiles(HWND hWnd, char *buf, HTREEITEM hItem);
void DragDrop_GetDropTree(HWND hWnd, char *Path, HTREEITEM hItem);
void DeleteTmpDropFiles(char *TempDirName);
BOOL DragDrop_GetDropItem(HWND hWnd, HGLOBAL hMem, int dwEffect, HTREEITEM hItem);
void DragDrop_GetDropFiles(HWND hWnd, HDROP hDrop, HTREEITEM hItem);
long DragDrop_NotifyProc(HWND hWnd, WPARAM wParam, LPARAM lParam);

//--------------------------
//	Undo
//--------------------------
void FreeUndo(void);
int CheckUndo(void);
void InitUndo(HWND hWnd, char *path, HTREEITEM hItem, int mode);
BOOL SetFolderToUndo(char *path, char *name);
BOOL SetItemToUndo(struct TPITEM *tpItemInfo);
BOOL SetLVTitleToUndo(HTREEITEM hItem, char *Old, char *New, char *URL);
BOOL SetTVTitleToUndo(HTREEITEM hItem, char *Old);
BOOL ExecUndo(HWND hWnd);

//--------------------------
//	Check
//--------------------------
void ItemCheckIni(HWND hWnd, struct TPITEM *tpItemInfo, BOOL no_err);
void FolderCheckIni(HWND hWnd, HTREEITEM hItem);
BOOL CheckIniProc(HWND hWnd, HTREEITEM hItem, int CheckFlag, int CheckType);
int FindCheckItem(HWND hWnd, HTREEITEM hItem);
int FindCheckNoCheckItem(HWND hWnd, HTREEITEM hItem);
int CheckProc(HWND hWnd);
BOOL ResultCheckStatus(HWND hWnd, struct TPITEM *tpItemInfo, int ret);
void InitCheckItemList(HWND hWnd, struct TPITEM *tpItemInfo);
int GethostMsg(HWND hWnd, WPARAM wParam, LPARAM lParam);
int SelectMsg(HWND hWnd, WPARAM wParam, LPARAM lParam);
int CheckEndItem(HWND hWnd, WPARAM wParam, LPARAM lParam);
void TimeoutItem(HWND hWnd);
void ListCancelItem(HWND hWnd);
void CALLBACK TreeCancelItem(HWND hWnd, HTREEITEM hItem, long Param);
void CheckEndProc(HWND hWnd);

//--------------------------
//	Font
//--------------------------
HFONT CreateListFont(char *FontName, int FontSize, int CharSet);

//--------------------------
//	main
//--------------------------
BOOL _SetForegroundWindow(HWND hWnd);
void ErrMsg(HWND hWnd, int ErrCode, char *Title);
void DoEvents(void);
void WaitCursor(BOOL WiatFlag);
BOOL TrayMessage(HWND hWnd, DWORD dwMessage, UINT uID, HICON hIcon, PSTR pszTip);
void SetTrayInitIcon(HWND hWnd);
void SetWinTitle(HWND hWnd);

#endif
/* End of source */
