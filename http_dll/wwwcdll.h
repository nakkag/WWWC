/**************************************************************************

	WWWC (dll)

	wwwcdll.h
	Ver 1.1.0

	Copyright (C) 1996-2018 by Ohno Tomoaki. All rights reserved.
		https://www.nakka.com/
		nakka@nakka.com

**************************************************************************/

#ifndef _INC_WWWCDLL_H
#define _INC_WWWCDLL_H

/**************************************************************************
	Define
**************************************************************************/

/* ウィンドウ内のコントロール */
#define WWWC_TB						102					/* ToolBar */
#define WWWC_SB						103					/* StatusBar */
#define WWWC_LIST					1002				/* ListView */
#define WWWC_TREE					1114				/* TreeView */

/* ソケットメッセージ */
#define WM_WSOCK_GETHOST			(WM_USER + 1)		/* ホスト情報を取得するメッセージ */
#define WM_WSOCK_SELECT				(WM_USER + 2)		/* 非同期selectのメッセージ */

/* チェックメッセージ */
#define WM_CHECK_RESULT				(WM_USER + 3)		/* アイテムのチェック結果を知らせるメッセージ */
#define WM_ITEMCHECK				(WM_USER + 4)		/* アイテムのチェック開始要求 */
#define WM_NEXTCHECK				(WM_USER + 9)		/* 次のチェック */
#define WM_CHECK_END				(WM_USER + 60)		/* アイテムのチェック終了を知らせるメッセージ */

/* データ要求メッセージ */
#define WM_GETVERSION				(WM_USER + 5)		/* WWWCのバージョンを取得 */
#define WM_GETCHECKLIST				(WM_USER + 6)		/* チェック中のアイテムリストを取得 */
#define WM_GETCHECKLISTCNT			(WM_USER + 7)		/* チェック中のアイテムリストの数を取得 */
#define WM_SETITEMLIST				(WM_USER + 8)		/* アイテムの追加 */
#define WM_GETMAINWINDOW			(WM_USER + 10)		/* メインウィンドウのハンドルを取得 */
#define WM_GETMAINITEM				(WM_USER + 11)		/* アイテムの実体を取得 */
#define WM_ITEMEXEC					(WM_USER + 12)		/* アイテムを実行 */
#define WM_ITEMINIT					(WM_USER + 13)		/* アイテムを初期化 */
#define WM_GETUPWINDOW				(WM_USER + 14)		/* 更新メッセージのハンドルを取得 */
#define WM_GETFINDWINDOW			(WM_USER + 15)		/* 検索ウィンドウのハンドルを取得 */

/* INIファイル処理メッセージ */
#define WM_WWWC_GETINI				(WM_USER + 20)		/* INIファイルから設定を読み直す */
#define WM_WWWC_PUTINI				(WM_USER + 21)		/* INIファイルに設定を書き込む */
#define WM_WWWC_GETINIPATH			(WM_USER + 22)		/* INIファイルの保存先のパスを取得 */

/* フォルダ操作メッセージ */
#define WM_FOLDER_SAVE				(WM_USER + 31)		/* 開いているフォルダの内容を保存 */
#define WM_FOLDER_LOAD				(WM_USER + 32)		/* 開いているフォルダの内容を読み直す */
#define WM_FOLDER_GETPATH			(WM_USER + 33)		/* フォルダのパスを取得 */
#define WM_FOLDER_GETWWWCPATH		(WM_USER + 34)		/* フォルダのWWWC内でのパスを取得 */
#define WM_FOLDER_SELECT			(WM_USER + 35)		/* フォルダを選択してそのパスを取得 */
#define WM_FOLDER_REFRESH			(WM_USER + 36)

#define WM_DOEVENTS					(WM_USER + 50)		/* 本体にイベントを処理させる */


/* アイテムの状態 */
#define ST_DEFAULT					0					/* 通常 */
#define ST_UP						1					/* UP */
#define ST_ERROR					2					/* エラー */
#define ST_TIMEOUT					4					/* タイムアウト */

/* アイコンの状態 */
#define ICON_ST_NOCHECK				1					/* チェック待機 */
#define ICON_ST_CHECK				2					/* チェック中 */

/* 戻り値 */
#define CHECK_ERROR					-1					/* エラー */
#define CHECK_SUCCEED				0					/* 処理完了 */
#define CHECK_NO					1					/* 待機 */
#define CHECK_END					2					/* チェック終了 */

/* ドロップファイルの戻り値 */
#define DROPFILE_NONE				0					/* 未処理 */
#define DROPFILE_NEWITEM			1					/* アイテム作成 */

/* ツール実行フラグ */
#define TOOL_EXEC_PORP				0					/* プロパティ */
#define TOOL_EXEC_ITEMMENU			1					/* アイテムメニュー */
#define TOOL_EXEC_WINDOWMENU		2					/* ウィンドウメニュー */
#define TOOL_EXEC_START				4					/* 起動時 */
#define TOOL_EXEC_END				8					/* 終了時 */
#define TOOL_EXEC_CHECKSTART		16					/* チェック開始時 */
#define TOOL_EXEC_CHECKEND			32					/* チェック終了時 */
#define TOOL_EXEC_CHECKENDUP		64					/* チェック終了時で更新ありのとき */
#define TOOL_EXEC_CHECKENDNOUP		128					/* チェック終了時で更新なしのとき */
#define TOOL_EXEC_SYNC				256					/* ツールが終了するまで待機 (DLLの場合は無効) */
#define TOOL_EXEC_INITITEM			512					/* 実行後にアイテムを初期化 */
#define TOOL_EXEC_MENUDEFAULT		1024				/* デフォルト項目 */
#define TOOL_EXEC_NOTCHECK			2048				/* チェック中は実行しない */
#define TOOL_EXEC_SAVEFOLDER		4096				/* 実行前にフォルダの内容を保存して実行後に読み直す */

/* ツール非表示フラグ */
#define TOOL_HIDE_ITEMMENU			1
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
#define TOOL_HIDE_HOTKEY			8192				/* ホットキー */
#define TOOL_HIDE_MENUINDEX			16384				/* メニューIndex */
#define TOOL_HIDE_PROTOCOL			32768				/* プロトコル選択 */

/* チェックの種類 */
#define CHECKTYPE_ITEM				1					/* アイテムをチェック */
#define CHECKTYPE_ALL				2					/* 全てのアイテムをチェック */
#define CHECKTYPE_TREE				4					/* 階層チェック */
#define CHECKTYPE_ERROR				8					/* エラーアイテムをチェック */
#define CHECKTYPE_AUTO				16					/* 自動チェック */


/**************************************************************************
	Struct
**************************************************************************/

/* アイテム情報 */
struct TPITEM{
	long iSize;						/* 構造体のサイズ */

	long hItem;						/* アイテムが格納されているフォルダのハンドル (HTREEITEM) */

	char *Title;					/* タイトル */
	char *CheckURL;					/* チェックを行うURL */
	char *Size;						/* サイズ */
	char *Date;						/* 更新日時 */
	int Status;						/* アイテムの状態 (ST_) */

	char *CheckDate;				/* チェックした日時 */
	char *OldSize;					/* 旧更新日 */
	char *OldDate;					/* 旧サイズ */

	char *ViewURL;					/* 表示するURL */
	char *Option1;					/* オプション 1 */
	char *Option2;					/* オプション 2 */
	char *Comment;					/* コメント */

	int CheckSt;					/* チェックフラグ */

	char *ErrStatus;				/* エラー状態 */
	char *DLLData1;					/* DLL用データ 1 */
	char *DLLData2;					/* DLL用データ 2 */

/* 以下保存されない情報 */
	int IconStatus;					/* アイコンの状態 (ICON_ST_) */

	int Soc1;						/* ソケット1 */
	int Soc2;						/* ソケット2 */
	HANDLE hGetHost1;				/* ホスト情報のハンドル1 */
	HANDLE hGetHost2;				/* ホスト情報のハンドル1 */
	long Param1;					/* DLL用long値1 */
	long Param2;					/* DLL用long値2 */
	long Param3;					/* DLL用long値3 */
	long Param4;					/* DLL用long値4 */
	int user1;						/* DLL用int値1 */
	int user2;						/* DLL用int値2 */

	BOOL RefreshFlag;				/* 再描画フラグ */
};


/* プロトコル設定 */
struct TPPROTOCOLSET{
	long iSize;						/* 構造体のサイズ */

	char Title[256];				/* タイトル */
	char FuncHeader[256];			/* 関数名の先頭に付ける文字 */
	char IconFile[256];				/* アイコンファイル名 */
	int IconIndex;					/* アイコンインデックス */
	char UpIconFile[256];			/* UPアイコンファイル名 */
	int UpIconIndex;				/* UPアイコンインデックス */
};

/* プロトコルメニュー */
struct TPPROTOCOLMENU{
	char Name[256];					/* メニュー名 */
	char Action[256];				/* 実行時のアクション */
	BOOL Default;					/* デフォルトフラグ */
	int Flag;						/* 動作フラグ */
};

/* プロトコル情報 */
struct TPPROTOCOLINFO{
	long iSize;						/* 構造体のサイズ */

	char Scheme[256];				/* 対応するスキーマ (例: "http://\thttps://") */
	char NewMenu[256];				/* 新規作成メニュー名 */
	char FileType[256];				/* D&Dで処理可能なファイルの拡張子 (例: "txt\tdoc\tdat") */
	struct TPPROTOCOLMENU *tpMenu;	/* アイテムメニューに追加する項目 */
	int tpMenuCnt;					/* アイテムメニューに追加する項目の数 */
};


/* ツール設定 */
struct TP_TOOLS{
	long iSize;						/* 構造体のサイズ */

	char title[256];				/* タイトル */
	char func[256];					/* 関数名 */
	char Protocol[256];				/* 対応するプロトコル (例: http,ftp) */
	int MenuIndex;					/* メニュー位置 */
	int Action;						/* 実行フラグ */
};

#endif
/* End of source */
