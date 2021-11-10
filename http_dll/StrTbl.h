/**************************************************************************

	WWWC (wwwc.dll)

	StrTbl.h

	Copyright (C) 1996-2003 by Nakashima Tomoaki. All rights reserved.
		http://www.nakka.com/
		nakka@nakka.com

**************************************************************************/

#ifndef _INC_DLL_STRTBL_H
#define _INC_DLL_STRTBL_H

/**************************************************************************
	Define
**************************************************************************/

//general
#define STR_NEWITEMNAME					"新しいアイテム"

#define STR_Q_TITLE_DEL					"削除"
#define STR_Q_MSG_DEL					"削除してもよろしいですか？"

#define STR_TITLE_FILESELECT			"ファイルの選択"

#define STR_ERROR_MEMORY				"メモリの確保に失敗しました。"

//main.c
#define STR_TOOL_TITLE_BROWSINFO		"ブラウザ情報取得(&B)"
#define STR_TOOL_TITLE_BROWSINFOLIST	"すべてのブラウザ情報から選択(&A)..."
#define STR_TOOL_TITLE_OPENURL			"既存のブラウザに開く(&D)"
#define STR_TOOL_TITLE_FILTERRELOAD		"フィルタ再読み込み(&F)"

#define STR_ERR_TITLE_OPEN				"開く"

#define STR_FILTER_EXESELECT			"実行ファイル (*.exe)\0*.exe\0\0"

//http.c
#define STR_TITLE_HEADVIEW				"ヘッダー表示 - [%s]"

#define STR_TITLE_SOURCEVIEW			"ソース表示%s - [%s]"
#define STR_TITLE_FILTER				" (フィルタ)"
#define STR_TITLE_FILTERERR				" (フィルタ エラー)"

#define STR_ERR_TITLE_URLOPEN			"URLを開く"
#define STR_ERR_MSG_URLOPEN				"URLが設定されていません。"

#define STR_ERROR_URL					"URLを展開できませんでした。"
#define STR_ERROR_GETHOST_TH			"gethostbyname用スレッドの作成に失敗しました。"
#define STR_ERROR_GETHOST				"IPアドレスの取得に失敗しました。"
#define STR_ERROR_SOCKET				"soketの作成に失敗しました。"
#define STR_ERROR_SELECT				"WSAAsyncSelectに失敗しました。"
#define STR_ERROR_CONNECT				"接続に失敗しました。"
#define STR_ERROR_SEND					"送信に失敗しました。"
#define STR_ERROR_RECV					"受信に失敗しました。"
#define STR_ERROR_HEADER				"ヘッダを解析できませんでした。"
#define STR_ERROR_MOVE					"転送エラー"
#define STR_ERROR_TIMEOUT				"タイムアウトしました。"

#define STR_STATUS_GETHOST				"ホスト情報を取得しています..."
#define STR_STATUS_CONNECT				"接続中..."
#define STR_STATUS_RESPONSE				"サーバからの応答を待っています..."
#define STR_STATUS_RECV					"%ld バイト受信"
#define STR_STATUS_CHECKEND				"チェック終了"

#define STR_GETINFO_HTTP_NEWMENU		"HTTPアイテムの追加(&H)..."
#define STR_GETINFO_HTTP_OPEN			"開く(&O)"
#define STR_GETINFO_HTTP_CHECKOPEN		"チェックするURLで開く(&U)"
#define STR_GETINFO_HTTP_HEADER			"ヘッダー表示(&H)"
#define STR_GETINFO_HTTP_SOURCE			"ソース表示(&S)"
#define STR_GETINFO_HTTP_GETTITLE		"タイトル取得(&L)"

//httpprop.c
#define STR_REQTYPE_AUTO_GET			"自動(&A) (現在 GET)"

#define STR_TITLE_PROP					"%sのプロパティ"
#define STR_TITLE_ADDITEM				"HTTPアイテムの追加"

#define STR_TITLE_SETHTTP				"HTTPプロトコルの設定"

//httptools.c
#define STR_ERR_TITLE_DDEEINIT			"DDE初期化"
#define STR_ERR_MSG_DDEEINIT			"DDEの初期化に失敗しました。"

#define STR_ERR_TITLE_DDE				"ブラウザ情報の取得"
#define STR_ERR_MSG_DDE					"ブラウザが起動されていないか、ブラウザ情報の取得に失敗しました。"

#define ST_LV_COL_TITLE					"タイトル"
#define ST_LV_COL_FLAME					"フレーム"
#define ST_LV_COL_URL					"URL"

//file.c
#define STR_ERR_TITLE_FILEOPEN			"ファイルを開く"
#define STR_ERR_MSG_FILEOPEN			"ファイルが設定されていません。"

#define STR_FILTER_FILESELECT			"すべてのファイル (*.*)\0*.*\0\0"

#define STR_GETINFO_FILE_NEWMENU		"ファイルの追加(&I)..."
#define STR_GETINFO_FILE_OPEN			"開く(&O)"

#endif
/* End of source */
