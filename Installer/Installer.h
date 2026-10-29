/*
 * WWWC Installer
 *
 * Installer.h
 *
 * Copyright (C) 1996-2026 by Ohno Tomoaki. All rights reserved.
 *		https://www.nakka.com/
 *		nakka@nakka.com
 */

#ifndef _INC_WWWC_INSTALLER_H
#define _INC_WWWC_INSTALLER_H

/* Define */
#define BUF_SIZE						256

#define APP_NAME						TEXT("WWWC")
#define APP_EXE							TEXT("WWWC.exe")
#define APP_PUBLISHER					TEXT("Ohno Tomoaki")
#define APP_URL							TEXT("https://nakka.com/wwwc/")
// 設定とアイテムの保存先 (%LOCALAPPDATA% の配下、インストール先の wwwc_app.ini で指定)
#define APP_DATA_FOLDER					TEXT("WWWC")

// WWWC本体の起動確認用 (ウィンドウのタイトルは設定で変わるためクラス名のみで判定する)
#define MAIN_WND_CLASS					TEXT("WWWCWindowClass")

// インストール先の既定フォルダ名
#define DEFAULT_FOLDER					TEXT("WWWC")
// インストール先に配置するアンインストーラ
#define UNINSTALL_EXE					TEXT("uninstall.exe")
// インストールしたファイルの一覧
#define UNINSTALL_LOG					TEXT("uninstall.dat")
// 使用中で上書きできないファイルを退避するときの拡張子
#define OLD_FILE_EXT					TEXT(".old")

// アプリの一覧への登録先
#define UNINSTALL_KEY					TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall")
#define UNINSTALL_SUBKEY				TEXT("WWWC")

// コマンドラインオプション
#define CMD_UNINSTALL					TEXT("/uninstall")
#define CMD_UNINSTALL_RUN				TEXT("/uninstallrun")
#define CMD_DELETE_DATA					TEXT("/deletedata")
#define CMD_PID							TEXT("/pid:")

/* Struct */
// アプリの一覧の登録場所
typedef struct _REG_TARGET {
	HKEY root;									// HKEY_LOCAL_MACHINE / HKEY_CURRENT_USER
	REGSAM view;								// KEY_WOW64_32KEY / KEY_WOW64_64KEY
	TCHAR subkey[BUF_SIZE];						// Uninstall配下のキー名
} REG_TARGET;

// 可変長の文字列バッファ
typedef struct _STR_BUF {
	TCHAR *buf;
	DWORD len;									// 格納済みの文字数
	DWORD size;									// 確保済みの文字数
} STR_BUF;

#endif
/* End of source */
