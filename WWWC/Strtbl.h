/**************************************************************************

	WWWC

	StrTbl.h

	Copyright (C) 1996-2003 by Nakashima Tomoaki. All rights reserved.
		http://www.nakka.com/
		nakka@nakka.com

**************************************************************************/

#ifndef _INC_STRTBL_H
#define _INC_STRTBL_H

/**************************************************************************
	Define
**************************************************************************/

//General
#define NEWITEMNAME						"新しいアイテム"
#define FOLDERNAME						"フォルダ"

#define STR_COPYNAME					"コピー ～ "
#define STR_COPYNAME_CNT				"コピー (%d) ～ "

#define EMSG_PROP_TITLE					"プロパティ"
#define EMSG_PROP						"設定できるプロパティはありません。"

#define QMSG_DELETE_TITLE				"削除の確認"
#define QMSG_DELETE						"選択されている項目を削除してもよろしいですか？"

//main.c
#define CLIPBOARDFORMAT					"WWWC ItemData"
#define DEFAULTUSER						"DEFAULT"

#define MENU_STR_NEWITEM				"新規作成(&N)"

#define ABOUTTITLE						"バージョン情報"

#define IMSG_DOUBLESTART				"既に起動しています"

#define QMSG_ALLINITICON_TITLE			"アイコンの初期化の確認"
#define QMSG_ALLINITICON				"すべてのアイコンを初期化してもよろしいですか？"
#define QMSG_END_TITLE					"終了"
#define QMSG_END_CHECKCANCEL			"チェック中のアイテムが存在しますが、終了してもよろしいですか？"

#define EMSG_OLEINIT					"OLEの初期化に失敗しました。"
#define EMSG_WINSOCKINIT				"WinSockの初期化に失敗しました。"
#define EMSG_ATEXIT						"atexitに失敗しました。"
#define EMSG_REGWINDOWCLASS				"WindowClassの登録に失敗しました。"
#define EMSG_CREATEWINDOW				"Windowの作成に失敗しました。"
#define EMSG_CREATECONTROL				"コントロールの作成に失敗しました。"
#define EMSG_CREATEUPMSG				"更新メッセージの作成に失敗しました。"

//AutoCheck.c
#define LISTCOL_TITLE					"時間"

#define STRWEEK_STRLEN					3
#define STRWEEK_SUN						"日"
#define STRWEEK_MON						"月"
#define STRWEEK_TUE						"火"
#define STRWEEK_WED						"水"
#define STRWEEK_THU						"木"
#define STRWEEK_FRI						"金"
#define STRWEEK_SAT						"土"

#define LISTSTR_MIN						"%d分毎 にチェック"
#define LISTSTR_NOU						"%d時間毎 にチェック"
#define LISTSTR_DAY						"%02d時 %02d分 にチェック"
#define LISTSTR_WEEK					"%s曜日 %02d時 %02d分 にチェック"
#define LISTSTR_MON						"%d日 %02d時 %02d分 にチェック"

#define TIMEOPTIONSTR_HOU				"時"
#define TIMEOPTIONSTR_MIN				"分"
#define TIMEOPTIONSTR_EVERYHOU			"時間毎"
#define TIMEOPTIONSTR_EVERYMIN			"分毎"

#define TIMEOPTIONERRMSG_TITLE			"自動チェック時間"
#define TIMEOPTIONERRMSG_DAY			"\"日\" が入力されていません。"
#define TIMEOPTIONERRMSG_HOU			"\"時\" が入力されていません。"
#define TIMEOPTIONERRMSG_MIN			"\"分\" が入力されていません。"

//ClipBoard.c
#define EMSG_FOLDERCOPY_TITLE			"フォルダのコピーのエラー"
#define EMSG_FOLDERCOPY_CUR				"%s をコピーできません。送り側と受け側のフォルダが同じです。"
#define EMSG_FOLDERCOPY_SUB				"%s をコピーできません。受け側のフォルダは、送り側フォルダのサブフォルダです。"
#define EMSG_FOLDERCOPY_ERR				"フォルダのコピーに失敗しました。"

#define EMSG_MOVEDIR_TITLE				"フォルダの移動のエラー"

//DragDrop.c
#define EMSG_ITEMREAD_TITLE				"アイテムの読み込み"
#define EMSG_ITEMREAD					"アイテム情報ファイルが開けませんでした。"

//File.c
#define FILEDATE_FORMAT					"%s %s"

#define STR_NOTITLE						"(no-title)"

#define ICON_FILE_FILTER				"*.*", "すべてのファイル (*.*)\0*.*\0\0"

#define EMSG_SAVE_TITLE					"保存エラー"
#define EMSG_EXEC_TITLE					"実行エラー"

#define EMSG_DISKFULL					"空きディスク容量が足らないためアイテム情報を保存できません。"

//Item.c
#define STR_UPDATEMSG					"フォルダ '%s' には既に '%s' アイテムが存在します。"
#define STR_UPDATEINFO					"サイズ: %s\n更新日時: %s"

#define QMSG_FOLDERDELETE				"フォルダ '%s' とフォルダ内のすべてのアイテムを削除してもよろしいですか？"
#define QMSG_ITEMDELETE					"'%s' を削除してもよろしいですか？"
#define QMSG_MANYITEMDELETE				"これらの %d 個のアイテムを削除してもよろしいですか？"

#define QMSG_CLEARRECYCLER_TITLE		"ごみ箱を空にする"
#define QMSG_CLEARRECYCLER				"ごみ箱を空にしてもよろしいですか？"

#define EMSG_FOLDERDELETE_TITLE			"フォルダの削除"
#define EMSG_FOLDERCHECK				"チェック中のフォルダは削除できません。"
#define EMSG_FOLDERPROP					"処理中のアイテムが存在するため削除できませんでした。"
#define EMSG_FOLDERNODELETE				"削除できませんでした。"

#define EMSG_ITEMDELETE_TITLE			"アイテムの削除"
#define EMSG_ITEMCHECK					"チェック中のアイテムは削除できません。"
#define EMSG_ITEMNODELETE				"削除できませんでした。"

#define EMSG_CLEARRECYCLER_TITLE		"ごみ箱を空にする"
#define EMSG_CLEARRECYCLER				"チェック中のアイテムが存在するためごみ箱を空にできませんでした。"
#define EMSG_PROPRECYCLER				"処理中のアイテムが存在するためごみ箱を空にできませんでした。"

#define EMSG_CHECKUPDATE_TITLE			"上書き"
#define EMSG_CHECKUPDATE				"チェック中のアイテムのため上書きできませんでした。"

#define EMSG_NOPROPITEM					"アイテムが見つかりませんでした。"

//ListView.c
#define COLUMN_NAME						"名前"
#define COLUMN_CHECKURL					"チェックするURL"
#define COLUMN_SIZE						"サイズ"
#define COLUMN_DATE						"更新日時"
#define COLUMN_CHECKDATE				"チェック日時"
#define COLUMN_OLDSIZE					"旧サイズ"
#define COLUMN_OLDDATE					"旧更新日時"
#define COLUMN_VIEWURL					"表示するURL"
#define COLUMN_OPTION1					"オプション1"
#define COLUMN_OPTION2					"オプション2"
#define COLUMN_COMMENT					"コメント"
#define COLUMN_ERROR					"エラー情報"
#define COLUMN_PATH						"パス"
#define COLUMN_DLLDATA1					"プロトコル情報1"
#define COLUMN_DLLDATA2					"プロトコル情報2"

//Menu.c
#define MENU_STR_OPEN					"開く(&O)"
#define MENU_STR_CLEARRECY				"ごみ箱を空にする(&B)"
#define MENU_STR_FOLDERTREECHECK		"階層チェック(&K)"
#define MENU_STR_SERACH					"検索(&F)..."

#define MENU_STR_UNDO_TITLE				"元に戻す"
#define MENU_STR_UNDO_KEY				"(&U)\tCtrl+Z"

#define MENU_STR_UNDO_NON				MENU_STR_UNDO_TITLE""MENU_STR_UNDO_KEY
#define MENU_STR_UNDO_MOVE				MENU_STR_UNDO_TITLE" - 移動"MENU_STR_UNDO_KEY
#define MENU_STR_UNDO_COPY				MENU_STR_UNDO_TITLE" - コピー"MENU_STR_UNDO_KEY
#define MENU_STR_UNDO_DELETE			MENU_STR_UNDO_TITLE" - 削除"MENU_STR_UNDO_KEY
#define MENU_STR_UNDO_NAME				MENU_STR_UNDO_TITLE" - 名前の変更"MENU_STR_UNDO_KEY

//Option.c
#define STR_ITEMCNT						"アイテム数: %d、フォルダ数: %d"
#define STR_NOMAKEDATE					"(不明)"
#define STR_PROPTITLE					"%sのプロパティ"
#define STR_OPTIONTITLE					"オプション"

#define FILEFILTER_WAV					"サウンド (*.wav)\0*.wav\0すべてのファイル (*.*)\0*.*\0\0"

//Protocol.c
#define PROTOCOL_FILEFILTER				"DLL (*.dll)\0*.dll\0すべてのファイル (*.*)\0*.*\0\0"

#define PROTOCOL_LISTCOL_TITLE			"タイトル"
#define PROTOCOL_LISTCOL_DLL			"DLL"
#define PROTOCOL_LISTCOL_FUNCHEAD		"ヘッダ"
#define PROTOCOL_LISTCOL_ICONFILE		"アイコンファイル"
#define PROTOCOL_LISTCOL_ICONINDEX		"アイコンインデックス"
#define PROTOCOL_LISTCOL_UPICONFILE		"UPアイコンファイル"
#define PROTOCOL_LISTCOL_UPICONINDEX	"UPアイコンインデックス"

#define EMSG_SELECT_TITLE_PROTOCOL		"プロトコル選択"
#define EMSG_SELECT_PROTOCOL			"追加するプロトコルを選択してください。"

//SelectDll.c
#define EMSG_DLL_TITLE					"DLL"
#define EMSG_DLL						"DLLからプロトコルとツールの情報が見つかりませんでした。"
#define EMSG_SELECT						"追加するプロトコル及びツールを選択してください。"

//SelectIcon.c
#define FILEFILTER_ICON					"アイコン ファイル\0*.ico;*.exe;*.dll;*.icl\0" \
											"プログラム\0*.exe\0ライブラリ\0*.dll;*.icl\0" \
											"アイコン\0*.ico\0すべてのファイル\0*.*\0\0"

//StatusBar.c
#define ITEMMSG							"%d 個のアイテム"
#define ITEMMSG_UP						"%d 個のアイテム (UPアイテム %d 個)"
#define ITEMSELECTMSG					"%d 個のアイテムを選択"

//Tool.c
#define NOMENUITEM						"(なし)"
#define TOOL_FILE_FILTER				"ツール (*.exe;*.dll)\0*.exe;*.dll\0すべてのファイル (*.*)\0*.*\0\0"

#define TOOL_LISTCOL_TITLE				"タイトル"
#define TOOL_LISTCOL_FILENAME			"ファイル名"
#define TOOL_LISTCOL_EXEOPTION			"実行オプション"
#define TOOL_LISTCOL_EXEPROTOCOL		"実行プロトコル"
#define TOOL_LISTCOL_MENUINDEX			"メニュー位置"
#define TOOL_LISTCOL_SKEY				"ショートカットキー"
#define TOOL_LISTCOL_CTRL				"キー 1"
#define TOOL_LISTCOL_KEY				"キー 2"
#define TOOL_LISTCOL_COMMANDLINE		"コマンドライン"
#define TOOL_LISTCOL_DLLFUNC			"関数(DLL)"

#define EMSG_NOINPUT_TITLE				"ツール設定"
#define EMSG_NOINPUT					"\"タイトル\" が入力されていません。"

#define EMSG_SELECT_TITLE_TOOL			"ツール選択"
#define EMSG_SELECT_TOOL				"追加するツールを選択してください。"

#define TOOL_TITLE_COMMANDLINE			"コマンドライン(&C):"
#define TOOL_TITLE_DLLFUNC				"実行する関数名:"

#define TOOL_PROTOCOL_ALL				"&0. すべて (*)"

#define COMMANDLINE_u					"&1. %u - 表示するURL, 未設定時チェックするURL"
#define COMMANDLINE_c					"&2. %c - チェックするURL"
#define COMMANDLINE_v					"&3. %v - 表示するURL"
#define COMMANDLINE_t					"&4. %t - アイテムの名前"
#define COMMANDLINE_s					"&5. %s - アイテムのサイズ"
#define COMMANDLINE_d					"&6. %d - アイテムの更新日時"
#define COMMANDLINE_k					"&7. %k - アイテムのチェック日時"
#define COMMANDLINE_m					"&8. %m - アイテムに設定されているコメント"
#define COMMANDLINE_f					"&9. %f - 開いているフォルダの実パス"
#define COMMANDLINE_F					"&10. %F - 開いているフォルダの実パス (短い形式)"
#define COMMANDLINE_p					"&11. %p - ユーザディレクトリのパス"
#define COMMANDLINE_P					"&12. %P - ユーザディレクトリのパス (短い形式)"

//TreeView.c
#define NEWFOLDER						"新しいフォルダ"

#define EMSG_CHANGEFILENAME_TITLE		"名前の変更"
#define EMSG_CHANGEFILENAME				"ファイル名に次の文字は使えません。\n\t\\ / : , ; * ? \" < > |"

#define EMSG_DIRMOVE_TITLE				"フォルダの移動"
#define EMSG_DIRMOVE					"アイテム情報ファイルが開けませんでした。"

//undo.c
#define QMSG_ITEMDELETE_UNDO			"削除されるアイテムがありますがよろしいですか？"

//UpMessage.c
#define STR_UPMSG						"%d 件更新された可能性があるアイテムがあります。\n\n%s %s"
#define STR_NOTUPMSG					"%d 件更新された可能性があるアイテムがあります。\n※ 最後のチェックでは更新はありませんでした。\n%s %s"
#define STR_ZEROMSG						"更新されたアイテムはありません。\n\n%s %s"
#define STR_NOUPMSG						"更新されたアイテムはありません。"

#define BTN_UPINFO_ON					"詳細(&D) >>"
#define BTN_UPINFO_OFF					"詳細(&D) <<"

#define EMSG_MAINSHOW_TITLE				"表示"
#define EMSG_NOMAINITEM					"本体からアイテムが見つかりませんでした。"

#endif
/* End of source */
