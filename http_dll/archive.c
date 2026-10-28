/**************************************************************************

	WWWC (wwwc.dll)

	archive.c

	Copyright (C) 1996-2026 by Ohno Tomoaki. All rights reserved.
		https://www.nakka.com/
		nakka@nakka.com

**************************************************************************/


/**************************************************************************
	Include Files
**************************************************************************/

#define _INC_OLE
#include <windows.h>
#undef  _INC_OLE
#include <winhttp.h>
#include <shlwapi.h>
#include <stdlib.h>

#include "String.h"
#include "http.h"
#include "StrTbl.h"
#include "wwwcdll.h"
#include "global.h"
#include "md5.h"
#include "archive.h"


/**************************************************************************
	Define
**************************************************************************/

#define ARCHIVE_DIR_EXT			".archive"				/* ユーザディレクトリ名に付加する */
#define ARCHIVE_RES_DIR			"res"
#define ARCHIVE_RES_DAT			"res.dat"
#define ARCHIVE_LATEST			"latest.dat"
#define ARCHIVE_INDEX				"index.html"
#define ARCHIVE_URLFILE			"url.txt"

#define ARCHIVE_KEYLEN			16						/* キーとリソース名のハッシュの桁数 */
#define ARCHIVE_MAX_RES			300						/* 1ページで取得するリソースの最大数 */
#define ARCHIVE_MAX_RESSIZE		(10 * 1024 * 1024)		/* リソース1つの最大サイズ */
#define ARCHIVE_MAX_TOTAL			(100 * 1024 * 1024)		/* 1ページで取得するリソースの合計の最大 */
#define ARCHIVE_MAX_REDIRECT		10
#define ARCHIVE_MAX_QUEUE			200
#define ARCHIVE_CSS_DEPTH			3						/* @import をたどる深さ */
#define ARCHIVE_MAX_DATFILE		(4 * 1024 * 1024)
#define ARCHIVE_MAX_GENFILES		1000
#define ARCHIVE_DIR_MARGIN		40						/* ディレクトリ名に付加するファイル名の分 */
#define ARCHIVE_END_WAIT			5000
#define ARCHIVE_FINISH_WAIT		30000					/* 終了時に取得待ちのアーカイブを処理する時間 */

#define WWWC_WNDCLASS			"WWWCWindowClass"

typedef struct _ARCHIVE_MEMO {
	char *url;
	int type;
	char *file;
	struct _ARCHIVE_MEMO *next;
} ARCHIVE_MEMO;

typedef struct _ARCHIVE_ENTRY {
	char *url;
	char *etag;
	char *lastmod;
	char *file;
	struct _ARCHIVE_ENTRY *next;
} ARCHIVE_ENTRY;

typedef struct _ARCHIVE_JOB {
	struct _ARCHIVE_JOB *next;
	char *Url;
	char *Title;
	char Dir[MAX_PATH];
	char Key[ARCHIVE_KEYLEN + 1];
	WCHAR *ProxyName;
	WCHAR *ProxyUser;
	WCHAR *ProxyPass;
	WCHAR *UserAgent;
	BOOL Image;
	BOOL Css;
	BOOL Js;
	int Generation;
	DWORD TimeOutMs;

	HINTERNET hSession;
	ARCHIVE_MEMO *Memo;
	ARCHIVE_ENTRY *Entries;
	int ResCount;
	LONGLONG Total;
	int CssDepth;
} ARCHIVE_JOB;

typedef struct _ARCHIVE_RESP {
	DWORD Status;
	char *Body;
	long Len;
	char Type[128];
	char Charset[64];
	char ETag[256];
	char LastMod[128];
	char *FinalUrl;
} ARCHIVE_RESP;


/**************************************************************************
	Global Variables
**************************************************************************/

static CRITICAL_SECTION ArchiveCs;
static BOOL ArchiveCsInit;
static HANDLE ArchiveEvent;
static HANDLE ArchiveThread;
static ARCHIVE_JOB *ArchiveQueue;
static int ArchiveQueueCnt;
static volatile LONG ArchiveStop;
static BOOL ArchiveRestart;						/* 停止後に再開する */
static volatile LONG ArchiveBusy;
static char ArchiveRunningKey[ARCHIVE_KEYLEN + 1];		/* 取得中のアイテムのキー (ArchiveCs で保護) */
static char ArchiveUserDir[MAX_PATH];

//外部参照
extern int TimeOut;
extern int Proxy;
extern char pServer[];
extern int pPort;
extern int pUsePass;
extern char pUser[];
extern char pPass[];
extern int ArchiveGlobal;


/**************************************************************************
	Helpers
**************************************************************************/

static char *ArchiveStrDup(const char *s)
{
	char *ret;

	if(s == NULL){
		return NULL;
	}
	ret = (char *)HeapAlloc(GetProcessHeap(), 0, lstrlen(s) + 1);
	if(ret != NULL){
		lstrcpy(ret, s);
	}
	return ret;
}

static WCHAR *WideDup(const char *s)
{
	WCHAR *ret;
	int len;

	if(s == NULL || (len = MultiByteToWideChar(CP_ACP, 0, s, -1, NULL, 0)) <= 0){
		return NULL;
	}
	ret = (WCHAR *)HeapAlloc(GetProcessHeap(), 0, len * sizeof(WCHAR));
	if(ret != NULL && MultiByteToWideChar(CP_ACP, 0, s, -1, ret, len) == 0){
		HeapFree(GetProcessHeap(), 0, ret);
		ret = NULL;
	}
	return ret;
}

static char *NarrowDup(const WCHAR *s)
{
	char *ret;
	int len;

	if(s == NULL || (len = WideCharToMultiByte(CP_ACP, 0, s, -1, NULL, 0, NULL, NULL)) <= 0){
		return NULL;
	}
	ret = (char *)HeapAlloc(GetProcessHeap(), 0, len);
	if(ret != NULL && WideCharToMultiByte(CP_ACP, 0, s, -1, ret, len, NULL, NULL) == 0){
		HeapFree(GetProcessHeap(), 0, ret);
		ret = NULL;
	}
	return ret;
}

static void MemFree(void *p)
{
	if(p != NULL){
		HeapFree(GetProcessHeap(), 0, p);
	}
}

static void WideFree(WCHAR *p)
{
	if(p != NULL){
		SecureZeroMemory(p, lstrlenW(p) * sizeof(WCHAR));
		HeapFree(GetProcessHeap(), 0, p);
	}
}

/* MD5 の16進表記 (digits 桁) */
static void HashHex(const void *data, long len, char *ret, int digits)
{
	MD5_CTX context;
	unsigned char digest[16];
	int i;

	MD5Init(&context);
	MD5Update(&context, (unsigned char *)data, len);
	MD5Final(digest, &context);
	for(i = 0; i < 16 && i * 2 < digits; i++){
		wsprintf(ret + i * 2, "%02x", digest[i]);
	}
	ret[digits] = '\0';
}

/* 制御文字を含まない文字列か */
static BOOL IsSafeText(const char *s)
{
	for(; *s != '\0'; s++){
		if((unsigned char)*s < 0x20 || *s == 0x7F){
			return FALSE;
		}
	}
	return TRUE;
}

static BOOL IsHttp(const char *url)
{
	return (url != NULL && (lstrcmpni((char *)url, "http://", 7) == 0 || lstrcmpni((char *)url, "https://", 8) == 0)) ? TRUE : FALSE;
}

/* ファイルの内容を読み込む */
static char *ReadAll(const char *path, long max, long *len)
{
	HANDLE hFile;
	DWORD size, ret;
	char *buf;

	hFile = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE){
		return NULL;
	}
	size = GetFileSize(hFile, NULL);
	if(size == INVALID_FILE_SIZE || size > (DWORD)max){
		CloseHandle(hFile);
		return NULL;
	}
	buf = (char *)HeapAlloc(GetProcessHeap(), 0, size + 1);
	if(buf == NULL || ReadFile(hFile, buf, size, &ret, NULL) == FALSE || ret != size){
		MemFree(buf);
		CloseHandle(hFile);
		return NULL;
	}
	CloseHandle(hFile);
	buf[size] = '\0';
	if(len != NULL){
		*len = size;
	}
	return buf;
}

/* 一時ファイルに書き込んでから置き換える */
static BOOL WriteAll(const char *path, const char *data, long len)
{
	char tmp[MAX_PATH];
	HANDLE hFile;
	DWORD ret;
	BOOL rc;

	if(lstrlen(path) + 4 >= MAX_PATH){
		return FALSE;
	}
	wsprintf(tmp, "%s.tmp", path);
	hFile = CreateFile(tmp, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE){
		return FALSE;
	}
	rc = (len == 0 || (WriteFile(hFile, data, len, &ret, NULL) == TRUE && ret == (DWORD)len)) ? TRUE : FALSE;
	CloseHandle(hFile);
	if(rc == FALSE || MoveFileEx(tmp, path, MOVEFILE_REPLACE_EXISTING) == FALSE){
		DeleteFile(tmp);
		return FALSE;
	}
	return TRUE;
}

/* インターネットから取得したファイルであることを記録する (Zone.Identifier) */
static void SetZoneIdentifier(const char *path, const char *url)
{
	char stream[MAX_PATH + 32];
	ARCHIVE_BUF b;
	HANDLE hFile;
	DWORD ret;

	if(lstrlen(path) + 20 >= sizeof(stream)){
		return;
	}
	wsprintf(stream, "%s:Zone.Identifier", path);
	ZeroMemory(&b, sizeof(ARCHIVE_BUF));
	Buf_Str(&b, "[ZoneTransfer]\r\nZoneId=3\r\n");
	if(url != NULL && IsSafeText(url) == TRUE && lstrlen(url) < ARCHIVE_URL_MAX){
		Buf_Str(&b, "HostUrl=");
		Buf_Str(&b, url);
		Buf_Str(&b, "\r\n");
	}
	if(b.err == FALSE){
		hFile = CreateFile(stream, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if(hFile != INVALID_HANDLE_VALUE){
			WriteFile(hFile, b.p, b.len, &ret, NULL);
			CloseHandle(hFile);
		}
	}
	Buf_Free(&b);
}

static BOOL MemEqual(const char *a, const char *b, long len)
{
	long i;

	for(i = 0; i < len; i++){
		if(a[i] != b[i]){
			return FALSE;
		}
	}
	return TRUE;
}

static BOOL FileExists(const char *path)
{
	DWORD attr = GetFileAttributes(path);
	return (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY) == 0) ? TRUE : FALSE;
}

static BOOL EnsureDir(const char *path)
{
	DWORD attr;

	if(CreateDirectory(path, NULL) == TRUE){
		return TRUE;
	}
	attr = GetFileAttributes(path);
	return (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY) != 0) ? TRUE : FALSE;
}

/* 世代のファイル名か (YYYYMMDD-HHMMSS[-n].html または .txt) */
static BOOL IsGenName(const char *name)
{
	const char *p;
	int i;

	for(i = 0; i < 15; i++){
		if(name[i] == '\0' || (i == 8 && name[i] != '-') || (i != 8 && !(name[i] >= '0' && name[i] <= '9'))){
			return FALSE;
		}
	}
	p = name + 15;
	if(*p == '-'){
		for(p++; *p >= '0' && *p <= '9'; p++);
	}
	return (lstrcmpi(p, ".html") == 0 || lstrcmpi(p, ".txt") == 0) ? TRUE : FALSE;
}

static int CompareName(const void *a, const void *b)
{
	return lstrcmp(*(const char **)a, *(const char **)b);
}

/* 世代のファイル名の一覧 (古い順) */
static int ListGens(const char *dir, char ***list)
{
	WIN32_FIND_DATA fd;
	HANDLE hFind;
	char path[MAX_PATH];
	char **ret = NULL, **tmp;
	int cnt = 0, cap = 0;

	*list = NULL;
	if(lstrlen(dir) + 3 >= MAX_PATH){
		return 0;
	}
	wsprintf(path, "%s\\*", dir);
	hFind = FindFirstFile(path, &fd);
	if(hFind == INVALID_HANDLE_VALUE){
		return 0;
	}
	do{
		if((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0 || IsGenName(fd.cFileName) == FALSE){
			continue;
		}
		if(cnt >= ARCHIVE_MAX_GENFILES){
			break;
		}
		if(cnt >= cap){
			cap = (cap == 0) ? 16 : cap * 2;
			tmp = (ret == NULL) ? (char **)HeapAlloc(GetProcessHeap(), 0, sizeof(char *) * cap) :
				(char **)HeapReAlloc(GetProcessHeap(), 0, ret, sizeof(char *) * cap);
			if(tmp == NULL){
				break;
			}
			ret = tmp;
		}
		if((ret[cnt] = ArchiveStrDup(fd.cFileName)) != NULL){
			cnt++;
		}
	}while(FindNextFile(hFind, &fd) == TRUE);
	FindClose(hFind);
	if(cnt > 1){
		qsort(ret, cnt, sizeof(char *), CompareName);
	}
	*list = ret;
	return cnt;
}

static void FreeList(char **list, int cnt)
{
	int i;

	if(list == NULL){
		return;
	}
	for(i = 0; i < cnt; i++){
		MemFree(list[i]);
	}
	MemFree(list);
}


/**************************************************************************
	Paths
**************************************************************************/

/* WWWC のメインウィンドウを取得 */
static BOOL CALLBACK FindMainProc(HWND hWnd, LPARAM lParam)
{
	char cls[64];

	if(GetClassName(hWnd, cls, sizeof(cls)) > 0 && lstrcmp(cls, WWWC_WNDCLASS) == 0){
		*(HWND *)lParam = hWnd;
		return FALSE;
	}
	return TRUE;
}

static HWND GetMainWindow(HWND hWnd)
{
	HWND ret = NULL;
	char cls[64];

	if(hWnd != NULL){
		if(GetClassName(hWnd, cls, sizeof(cls)) > 0 && lstrcmp(cls, WWWC_WNDCLASS) == 0){
			return hWnd;
		}
		ret = GetAncestor(hWnd, GA_ROOTOWNER);
		if(ret != NULL && GetClassName(ret, cls, sizeof(cls)) > 0 && lstrcmp(cls, WWWC_WNDCLASS) == 0){
			return ret;
		}
		ret = NULL;
	}
	EnumThreadWindows(GetCurrentThreadId(), FindMainProc, (LPARAM)&ret);
	return ret;
}

/******************************************************************************

	Archive_GetRoot

	アーカイブを保存するディレクトリ (ユーザディレクトリと同じ場所の ユーザ名.archive)
	ユーザディレクトリの中はフォルダのツリーとして扱われるため使用しない

******************************************************************************/

BOOL Archive_GetRoot(HWND hWnd, char *ret)
{
	char buf[MAX_PATH * 2];
	char *p;
	HWND mWnd;

	*ret = '\0';
	if((mWnd = GetMainWindow(hWnd)) != NULL){
		*buf = '\0';
		SendMessage(mWnd, WM_WWWC_GETINIPATH, 0, (LPARAM)buf);
		buf[MAX_PATH - 1] = '\0';
		//末尾の \ を除く
		for(p = buf + lstrlen(buf); p > buf && *(p - 1) == '\\'; p--);
		*p = '\0';
		if(*buf != '\0'){
			lstrcpy(ArchiveUserDir, buf);
		}
	}
	if(*ArchiveUserDir == '\0' || lstrlen(ArchiveUserDir) + lstrlen(ARCHIVE_DIR_EXT) >= MAX_PATH){
		return FALSE;
	}
	wsprintf(ret, "%s%s", ArchiveUserDir, ARCHIVE_DIR_EXT);
	return TRUE;
}

/* アイテムのアーカイブのディレクトリ */
static BOOL GetItemDir(HWND hWnd, const char *url, char *ret, char *key)
{
	char root[MAX_PATH];
	char k[ARCHIVE_KEYLEN + 1];

	if(url == NULL || IsHttp(url) == FALSE || Archive_GetRoot(hWnd, root) == FALSE){
		return FALSE;
	}
	HashHex(url, lstrlen(url), k, ARCHIVE_KEYLEN);
	if(lstrlen(root) + 1 + ARCHIVE_KEYLEN + ARCHIVE_DIR_MARGIN >= MAX_PATH){
		return FALSE;
	}
	wsprintf(ret, "%s\\%s", root, k);
	if(key != NULL){
		lstrcpy(key, k);
	}
	return TRUE;
}


/**************************************************************************
	HTTP
**************************************************************************/

static void FreeResp(ARCHIVE_RESP *r)
{
	MemFree(r->Body);
	MemFree(r->FinalUrl);
	ZeroMemory(r, sizeof(ARCHIVE_RESP));
}

/* ヘッダの値を取得 */
static void QueryHeader(HINTERNET hReq, DWORD info, char *ret, int size)
{
	WCHAR *w;
	DWORD len = 0;
	char *s;

	*ret = '\0';
	if(WinHttpQueryHeaders(hReq, info, WINHTTP_HEADER_NAME_BY_INDEX, NULL, &len, WINHTTP_NO_HEADER_INDEX) == FALSE &&
		GetLastError() != ERROR_INSUFFICIENT_BUFFER){
		return;
	}
	if(len == 0 || len > 64 * 1024){
		return;
	}
	w = (WCHAR *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, len + sizeof(WCHAR));
	if(w == NULL){
		return;
	}
	if(WinHttpQueryHeaders(hReq, info, WINHTTP_HEADER_NAME_BY_INDEX, w, &len, WINHTTP_NO_HEADER_INDEX) == TRUE &&
		(s = NarrowDup(w)) != NULL){
		if(lstrlen(s) < size && IsSafeText(s) == TRUE){
			lstrcpy(ret, s);
		}
		MemFree(s);
	}
	MemFree(w);
}

/* Content-Type から種類と文字コードを取得 */
static void ParseContentType(ARCHIVE_RESP *r)
{
	char *p, *c;
	int i;

	for(p = r->Type; *p != '\0'; p++){
		*p = (*p >= 'A' && *p <= 'Z') ? *p - 'A' + 'a' : *p;
	}
	*r->Charset = '\0';
	if((p = StrStr(r->Type, ";")) != NULL){
		c = StrStr(p, "charset=");
		if(c != NULL){
			c += 8;
			if(*c == '"') c++;
			for(i = 0; c[i] != '\0' && c[i] != '"' && c[i] != ';' && c[i] != ' ' && i < sizeof(r->Charset) - 1; i++){
				r->Charset[i] = c[i];
			}
			r->Charset[i] = '\0';
		}
		*p = '\0';
	}
	for(p = r->Type + lstrlen(r->Type); p > r->Type && *(p - 1) == ' '; p--);
	*p = '\0';
}

/******************************************************************************

	ArchiveFetch

	URLを取得する (リダイレクトはここで処理し、認証情報は同じ接続先にのみ送る)

******************************************************************************/

static BOOL ArchiveFetch(ARCHIVE_JOB *job, const char *url, const char *lastmod, const char *etag, long maxsize, ARCHIVE_RESP *r)
{
	URL_COMPONENTS uc;
	HINTERNET hConnect = NULL, hReq = NULL;
	WCHAR host[256];
	WCHAR *wurl = NULL, *path = NULL, *header = NULL;
	ARCHIVE_BUF hb, body;
	char *cur, *next, loc[ARCHIVE_URL_MAX + 1];
	DWORD status, size, len, dwRead, flags;
	int hop;
	BOOL ret = FALSE;

	ZeroMemory(r, sizeof(ARCHIVE_RESP));
	cur = ArchiveStrDup(url);
	for(hop = 0; cur != NULL && hop <= ARCHIVE_MAX_REDIRECT && ArchiveStop == 0; hop++){
		ZeroMemory(&hb, sizeof(ARCHIVE_BUF));
		ZeroMemory(&body, sizeof(ARCHIVE_BUF));
		wurl = WideDup(cur);
		if(wurl == NULL){
			break;
		}
		len = lstrlenW(wurl) + 1;
		path = (WCHAR *)HeapAlloc(GetProcessHeap(), 0, len * sizeof(WCHAR));
		if(path == NULL){
			break;
		}
		ZeroMemory(&uc, sizeof(uc));
		uc.dwStructSize = sizeof(uc);
		uc.lpszHostName = host;
		uc.dwHostNameLength = 256;
		uc.lpszUrlPath = path;
		uc.dwUrlPathLength = len;
		if(WinHttpCrackUrl(wurl, 0, 0, &uc) == FALSE){
			break;
		}
		{
			WCHAR *f;
			for(f = path; *f != L'\0'; f++){
				if(*f == L'#'){
					*f = L'\0';
					break;
				}
			}
		}
		hConnect = WinHttpConnect(job->hSession, host, uc.nPort, 0);
		if(hConnect == NULL){
			break;
		}
		hReq = WinHttpOpenRequest(hConnect, L"GET", path, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
			(uc.nScheme == INTERNET_SCHEME_HTTPS) ? WINHTTP_FLAG_SECURE : 0);
		if(hReq == NULL){
			break;
		}
		flags = WINHTTP_OPTION_REDIRECT_POLICY_NEVER;
		WinHttpSetOption(hReq, WINHTTP_OPTION_REDIRECT_POLICY, &flags, sizeof(flags));
		//Windows のログオン情報を自動で送信しない
		flags = WINHTTP_AUTOLOGON_SECURITY_LEVEL_HIGH;
		WinHttpSetOption(hReq, WINHTTP_OPTION_AUTOLOGON_POLICY, &flags, sizeof(flags));
		if(job->ProxyUser != NULL && job->ProxyPass != NULL){
			WinHttpSetOption(hReq, WINHTTP_OPTION_PROXY_USERNAME, job->ProxyUser, lstrlenW(job->ProxyUser));
			WinHttpSetOption(hReq, WINHTTP_OPTION_PROXY_PASSWORD, job->ProxyPass, lstrlenW(job->ProxyPass));
		}
		//送信ヘッダ
		Buf_Str(&hb, "Accept: */*");
		if(hop == 0 && etag != NULL && *etag != '\0' && IsSafeText(etag) == TRUE){
			Buf_Str(&hb, "\r\nIf-None-Match: ");
			Buf_Str(&hb, etag);
		}
		if(hop == 0 && lastmod != NULL && *lastmod != '\0' && IsSafeText(lastmod) == TRUE){
			Buf_Str(&hb, "\r\nIf-Modified-Since: ");
			Buf_Str(&hb, lastmod);
		}
		header = (hb.err == FALSE) ? WideDup(hb.p) : NULL;
		if(hb.p != NULL){
			SecureZeroMemory(hb.p, hb.len);
		}
		Buf_Free(&hb);
		if(header == NULL){
			break;
		}
		if(WinHttpSendRequest(hReq, header, (DWORD)-1, WINHTTP_NO_REQUEST_DATA, 0, 0, 0) == FALSE ||
			WinHttpReceiveResponse(hReq, NULL) == FALSE){
			break;
		}
		size = sizeof(status);
		if(WinHttpQueryHeaders(hReq, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX,
			&status, &size, WINHTTP_NO_HEADER_INDEX) == FALSE){
			break;
		}
		if(status == 301 || status == 302 || status == 303 || status == 307 || status == 308){
			//リダイレクト (http, https のみ)
			QueryHeader(hReq, WINHTTP_QUERY_LOCATION, loc, sizeof(loc));
			next = (*loc != '\0') ? CreateMoveURL(cur, loc) : NULL;
			MemFree(cur);
			cur = NULL;
			if(next != NULL){
				if(lstrlen(next) <= ARCHIVE_URL_MAX){
					cur = ArchiveStrDup(next);
				}
				GlobalFree(next);
			}
			WinHttpCloseHandle(hReq);
			WinHttpCloseHandle(hConnect);
			hReq = hConnect = NULL;
			WideFree(header);
			MemFree(wurl);
			MemFree(path);
			header = wurl = path = NULL;
			continue;
		}
		r->Status = status;
		if(status == 200){
			QueryHeader(hReq, WINHTTP_QUERY_CONTENT_TYPE, r->Type, sizeof(r->Type));
			QueryHeader(hReq, WINHTTP_QUERY_ETAG, r->ETag, sizeof(r->ETag));
			QueryHeader(hReq, WINHTTP_QUERY_LAST_MODIFIED, r->LastMod, sizeof(r->LastMod));
			ParseContentType(r);
			//本文
			while(ArchiveStop == 0){
				size = 0;
				if(WinHttpQueryDataAvailable(hReq, &size) == FALSE){
					body.err = TRUE;
					break;
				}
				if(size == 0){
					break;
				}
				if(body.len + (LONGLONG)size > maxsize){
					body.err = TRUE;
					break;
				}
				{
					char *rb = (char *)HeapAlloc(GetProcessHeap(), 0, size);
					if(rb == NULL){
						body.err = TRUE;
						break;
					}
					dwRead = 0;
					if(WinHttpReadData(hReq, rb, size, &dwRead) == FALSE){
						HeapFree(GetProcessHeap(), 0, rb);
						body.err = TRUE;
						break;
					}
					Buf_Add(&body, rb, (long)dwRead);
					HeapFree(GetProcessHeap(), 0, rb);
					if(dwRead == 0){
						break;
					}
				}
			}
			if(body.err == TRUE || ArchiveStop != 0){
				Buf_Free(&body);
				break;
			}
			if(body.p == NULL){
				//本文が空
				body.p = (char *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 1);
				if(body.p == NULL){
					break;
				}
			}
			r->Body = body.p;
			r->Len = body.len;
			body.p = NULL;
		}
		r->FinalUrl = cur;
		cur = NULL;
		ret = TRUE;
		break;
	}
	if(hReq != NULL) WinHttpCloseHandle(hReq);
	if(hConnect != NULL) WinHttpCloseHandle(hConnect);
	WideFree(header);
	MemFree(wurl);
	MemFree(path);
	MemFree(cur);
	if(ret == FALSE){
		FreeResp(r);
	}
	return ret;
}


/**************************************************************************
	Resource index (res.dat)
**************************************************************************/

static void FreeEntries(ARCHIVE_ENTRY *e)
{
	ARCHIVE_ENTRY *n;

	for(; e != NULL; e = n){
		n = e->next;
		MemFree(e->url);
		MemFree(e->etag);
		MemFree(e->lastmod);
		MemFree(e->file);
		MemFree(e);
	}
}

static ARCHIVE_ENTRY *FindEntry(ARCHIVE_JOB *job, const char *url)
{
	ARCHIVE_ENTRY *e;

	for(e = job->Entries; e != NULL; e = e->next){
		if(lstrcmp(e->url, url) == 0){
			return e;
		}
	}
	return NULL;
}

/* リソースのファイル名か (16桁の16進 + 拡張子) */
static BOOL IsResName(const char *name)
{
	const char *p;
	int i;

	for(i = 0; i < ARCHIVE_KEYLEN; i++){
		if(!((name[i] >= '0' && name[i] <= '9') || (name[i] >= 'a' && name[i] <= 'f'))){
			return FALSE;
		}
	}
	if(name[ARCHIVE_KEYLEN] != '.'){
		return FALSE;
	}
	for(p = name + ARCHIVE_KEYLEN + 1, i = 0; *p != '\0'; p++, i++){
		if(!((*p >= 'a' && *p <= 'z') || (*p >= '0' && *p <= '9')) || i >= 4){
			return FALSE;
		}
	}
	return (i >= 2) ? TRUE : FALSE;
}

static void SetEntry(ARCHIVE_JOB *job, const char *url, const char *etag, const char *lastmod, const char *file)
{
	ARCHIVE_ENTRY *e;

	e = FindEntry(job, url);
	if(e == NULL){
		e = (ARCHIVE_ENTRY *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(ARCHIVE_ENTRY));
		if(e == NULL || (e->url = ArchiveStrDup(url)) == NULL){
			MemFree(e);
			return;
		}
		e->next = job->Entries;
		job->Entries = e;
	}
	MemFree(e->etag);
	MemFree(e->lastmod);
	MemFree(e->file);
	e->etag = ArchiveStrDup(etag);
	e->lastmod = ArchiveStrDup(lastmod);
	e->file = ArchiveStrDup(file);
}

static void LoadEntries(ARCHIVE_JOB *job)
{
	char path[MAX_PATH];
	char *buf, *p, *line, *f[4];
	long len;
	int i;

	wsprintf(path, "%s\\%s", job->Dir, ARCHIVE_RES_DAT);
	if((buf = ReadAll(path, ARCHIVE_MAX_DATFILE, &len)) == NULL){
		return;
	}
	for(p = buf; *p != '\0';){
		line = p;
		for(; *p != '\0' && *p != '\r' && *p != '\n'; p++);
		if(*p != '\0'){
			*(p++) = '\0';
		}
		for(; *p == '\r' || *p == '\n'; p++);
		f[0] = line;
		for(i = 1; i < 4; i++){
			f[i] = StrChr(f[i - 1], '\t');
			if(f[i] == NULL){
				break;
			}
			*(f[i]++) = '\0';
		}
		if(i < 4 || IsHttp(f[0]) == FALSE || IsResName(f[3]) == FALSE || lstrlen(f[0]) > ARCHIVE_URL_MAX ||
			lstrlen(f[1]) >= 256 || lstrlen(f[2]) >= 128){
			continue;
		}
		SetEntry(job, f[0], f[1], f[2], f[3]);
	}
	MemFree(buf);
}

static void SaveEntries(ARCHIVE_JOB *job)
{
	ARCHIVE_ENTRY *e;
	ARCHIVE_BUF b;
	char path[MAX_PATH];

	ZeroMemory(&b, sizeof(ARCHIVE_BUF));
	for(e = job->Entries; e != NULL; e = e->next){
		if(e->file == NULL){
			continue;
		}
		Buf_Str(&b, e->url);
		Buf_Str(&b, "\t");
		Buf_Str(&b, (e->etag != NULL) ? e->etag : "");
		Buf_Str(&b, "\t");
		Buf_Str(&b, (e->lastmod != NULL) ? e->lastmod : "");
		Buf_Str(&b, "\t");
		Buf_Str(&b, e->file);
		Buf_Str(&b, "\r\n");
	}
	wsprintf(path, "%s\\%s", job->Dir, ARCHIVE_RES_DAT);
	if(b.err == FALSE){
		WriteAll(path, (b.p != NULL) ? b.p : "", b.len);
	}
	Buf_Free(&b);
}


/**************************************************************************
	Resources
**************************************************************************/

/* 保存するリソースの拡張子 (保存しない種類は NULL) */
static const char *ResExt(int type, ARCHIVE_RESP *r, const char *url)
{
	static const struct { const char *type; const char *ext; } img[] = {
		{"image/png", "png"}, {"image/apng", "png"}, {"image/jpeg", "jpg"}, {"image/jpg", "jpg"}, {"image/pjpeg", "jpg"},
		{"image/gif", "gif"}, {"image/webp", "webp"}, {"image/svg+xml", "svg"}, {"image/x-icon", "ico"},
		{"image/vnd.microsoft.icon", "ico"}, {"image/bmp", "bmp"}, {"image/avif", "avif"},
	};
	static const char *js[] = {
		"application/javascript", "text/javascript", "application/x-javascript", "application/ecmascript", "text/ecmascript",
	};
	int i;

	switch(type)
	{
	case RES_IMAGE:
		for(i = 0; i < sizeof(img) / sizeof(img[0]); i++){
			if(lstrcmp(r->Type, img[i].type) == 0){
				return img[i].ext;
			}
		}
		break;

	case RES_CSS:
		if(lstrcmp(r->Type, "text/css") == 0){
			return "css";
		}
		break;

	case RES_JS:
		for(i = 0; i < sizeof(js) / sizeof(js[0]); i++){
			if(lstrcmp(r->Type, js[i]) == 0){
				return "js";
			}
		}
		break;
	}
	return NULL;
}

static const char *AddMemo(ARCHIVE_JOB *job, const char *url, int type, const char *file)
{
	ARCHIVE_MEMO *m;

	m = (ARCHIVE_MEMO *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(ARCHIVE_MEMO));
	if(m == NULL || (m->url = ArchiveStrDup(url)) == NULL){
		MemFree(m);
		return NULL;
	}
	m->type = type;
	m->file = ArchiveStrDup(file);
	m->next = job->Memo;
	job->Memo = m;
	return m->file;
}

/******************************************************************************

	ResolveRes

	リソースを取得して保存し、ファイル名を返す (書き換え処理から呼ばれる)

******************************************************************************/

static const char *ResolveRes(void *ctx, const char *url, int type)
{
	ARCHIVE_JOB *job = (ARCHIVE_JOB *)ctx;
	ARCHIVE_MEMO *m;
	ARCHIVE_ENTRY *e;
	ARCHIVE_RESP r;
	char path[MAX_PATH];
	char name[ARCHIVE_KEYLEN + 8];
	char *data, *css = NULL;
	const char *ext, *file = NULL;
	long len;
	BOOL cond;

	if(ArchiveStop != 0 || (type == RES_IMAGE && job->Image == FALSE) ||
		(type == RES_CSS && job->Css == FALSE) || (type == RES_JS && job->Js == FALSE) ||
		(type != RES_IMAGE && type != RES_CSS && type != RES_JS)){
		return NULL;
	}
	for(m = job->Memo; m != NULL; m = m->next){
		if(m->type == type && lstrcmp(m->url, url) == 0){
			return m->file;
		}
	}
	if(job->ResCount >= ARCHIVE_MAX_RES || job->Total >= ARCHIVE_MAX_TOTAL){
		return NULL;
	}
	job->ResCount++;

	//前回取得したものがある場合は変更があったときのみ取得する (CSSは中のURLを処理するため毎回取得)
	e = FindEntry(job, url);
	cond = FALSE;
	if(type != RES_CSS && e != NULL && e->file != NULL){
		wsprintf(path, "%s\\%s\\%s", job->Dir, ARCHIVE_RES_DIR, e->file);
		cond = FileExists(path);
	}
	if(ArchiveFetch(job, url, (cond == TRUE) ? e->lastmod : NULL, (cond == TRUE) ? e->etag : NULL,
		ARCHIVE_MAX_RESSIZE, &r) == FALSE){
		return AddMemo(job, url, type, NULL);
	}
	if(r.Status == 304 && cond == TRUE){
		file = e->file;
	}else if(r.Status == 200 && (ext = ResExt(type, &r, url)) != NULL){
		data = r.Body;
		len = r.Len;
		if(type == RES_CSS){
			//CSS内のURLを書き換える (深い @import は絶対URLにするのみ)
			job->CssDepth++;
			css = Archive_RewriteCss(r.Body, r.Len, r.FinalUrl, "",
				(job->CssDepth < ARCHIVE_CSS_DEPTH) ? ResolveRes : NULL, job, &len);
			job->CssDepth--;
			if(css == NULL){
				FreeResp(&r);
				return AddMemo(job, url, type, NULL);
			}
			data = css;
		}
		HashHex(data, len, name, ARCHIVE_KEYLEN);
		lstrcat(name, ".");
		lstrcat(name, ext);
		wsprintf(path, "%s\\%s\\%s", job->Dir, ARCHIVE_RES_DIR, name);
		if(FileExists(path) == TRUE || WriteAll(path, data, len) == TRUE){
			job->Total += len;
			SetEntry(job, url, (type == RES_CSS) ? "" : r.ETag, (type == RES_CSS) ? "" : r.LastMod, name);
			file = name;
		}
		Archive_Free(css);
	}
	file = AddMemo(job, url, type, file);
	FreeResp(&r);
	return file;
}


/**************************************************************************
	Generations
**************************************************************************/

/* 参照されているリソースの印を付ける */
static void MarkRefs(const char *data, long len, char **names, BOOL *mark, int cnt)
{
	const char *p, *end = data + len;
	int i, n;

	for(p = data; p + ARCHIVE_KEYLEN + 3 <= end; p++){
		for(n = 0; n < ARCHIVE_KEYLEN && p + n < end && ((p[n] >= '0' && p[n] <= '9') || (p[n] >= 'a' && p[n] <= 'f')); n++);
		if(n < ARCHIVE_KEYLEN || p[ARCHIVE_KEYLEN] != '.'){
			continue;
		}
		for(i = 0; i < cnt; i++){
			if(mark[i] == FALSE && lstrcmpn((char *)p, names[i], ARCHIVE_KEYLEN + 1) == 0){
				mark[i] = TRUE;
			}
		}
		p += ARCHIVE_KEYLEN;
	}
}

/* 使われていないリソースを削除する */
static void CleanResources(ARCHIVE_JOB *job, char **gens, int gcnt)
{
	WIN32_FIND_DATA fd;
	HANDLE hFind;
	ARCHIVE_ENTRY *e;
	char path[MAX_PATH];
	char **names = NULL, **tmp;
	BOOL *mark = NULL, *scanned = NULL, again;
	char *buf;
	long len;
	int cnt = 0, cap = 0, i;

	wsprintf(path, "%s\\%s\\*", job->Dir, ARCHIVE_RES_DIR);
	hFind = FindFirstFile(path, &fd);
	if(hFind == INVALID_HANDLE_VALUE){
		return;
	}
	do{
		if((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0 || IsResName(fd.cFileName) == FALSE){
			continue;
		}
		if(cnt >= cap){
			cap = (cap == 0) ? 64 : cap * 2;
			tmp = (names == NULL) ? (char **)HeapAlloc(GetProcessHeap(), 0, sizeof(char *) * cap) :
				(char **)HeapReAlloc(GetProcessHeap(), 0, names, sizeof(char *) * cap);
			if(tmp == NULL){
				break;
			}
			names = tmp;
		}
		if((names[cnt] = ArchiveStrDup(fd.cFileName)) != NULL){
			cnt++;
		}
	}while(FindNextFile(hFind, &fd) == TRUE);
	FindClose(hFind);
	if(cnt == 0){
		MemFree(names);
		return;
	}
	mark = (BOOL *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(BOOL) * cnt);
	scanned = (BOOL *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(BOOL) * cnt);
	if(mark == NULL || scanned == NULL){
		goto end;
	}
	//世代のページから参照されているもの
	for(i = 0; i < gcnt; i++){
		wsprintf(path, "%s\\%s", job->Dir, gens[i]);
		if((buf = ReadAll(path, MAX_RECVSIZE * 2, &len)) == NULL){
			//読めない場合は削除しない
			goto end;
		}
		MarkRefs(buf, len, names, mark, cnt);
		MemFree(buf);
	}
	//CSSから参照されているもの
	do{
		again = FALSE;
		for(i = 0; i < cnt; i++){
			if(mark[i] == TRUE && scanned[i] == FALSE){
				scanned[i] = TRUE;
				if(lstrcmpi(names[i] + ARCHIVE_KEYLEN, ".css") != 0){
					continue;
				}
				wsprintf(path, "%s\\%s\\%s", job->Dir, ARCHIVE_RES_DIR, names[i]);
				if((buf = ReadAll(path, ARCHIVE_MAX_RESSIZE * 2, &len)) != NULL){
					MarkRefs(buf, len, names, mark, cnt);
					MemFree(buf);
					again = TRUE;
				}
			}
		}
	}while(again == TRUE);
	for(i = 0; i < cnt; i++){
		if(mark[i] == TRUE){
			continue;
		}
		wsprintf(path, "%s\\%s\\%s", job->Dir, ARCHIVE_RES_DIR, names[i]);
		DeleteFile(path);
		for(e = job->Entries; e != NULL; e = e->next){
			if(e->file != NULL && lstrcmp(e->file, names[i]) == 0){
				MemFree(e->file);
				e->file = NULL;
			}
		}
	}
end:
	MemFree(mark);
	MemFree(scanned);
	FreeList(names, cnt);
}

/* HTMLの特殊文字を変換して追加 */
static void AddHtmlText(ARCHIVE_BUF *b, const char *s)
{
	for(; *s != '\0'; s++){
		switch(*s)
		{
		case '&': Buf_Str(b, "&amp;"); break;
		case '<': Buf_Str(b, "&lt;"); break;
		case '>': Buf_Str(b, "&gt;"); break;
		case '"': Buf_Str(b, "&quot;"); break;
		default:
			if((unsigned char)*s >= 0x20 || *s == '\t'){
				Buf_Add(b, s, 1);
			}
			break;
		}
	}
}

/* 世代の一覧ページを作成する (UTF-8) */
static void WriteIndex(ARCHIVE_JOB *job, char **gens, int gcnt)
{
	ARCHIVE_BUF b;
	char path[MAX_PATH];
	char date[64];
	WCHAR *w;
	char *u;
	int i, len;

	wsprintf(path, "%s\\%s", job->Dir, ARCHIVE_INDEX);
	if(job->Generation <= 1 || gcnt <= 1){
		DeleteFile(path);
		return;
	}
	ZeroMemory(&b, sizeof(ARCHIVE_BUF));
	Buf_Str(&b, "<!DOCTYPE html>\r\n<html><head><meta charset=\"utf-8\">\r\n"
		"<meta http-equiv=\"Content-Security-Policy\" content=\"default-src 'none'; style-src 'unsafe-inline'\">\r\n<title>");
	Buf_Str(&b, STR_ARCHIVE_INDEX_TITLE);
	AddHtmlText(&b, (job->Title != NULL) ? job->Title : job->Url);
	Buf_Str(&b, "</title>\r\n<style>body{font-family:sans-serif;margin:2em}li{margin:.4em 0}</style>\r\n</head><body>\r\n<h1>");
	AddHtmlText(&b, (job->Title != NULL) ? job->Title : job->Url);
	Buf_Str(&b, "</h1>\r\n<p>");
	AddHtmlText(&b, job->Url);
	Buf_Str(&b, "</p>\r\n<ul>\r\n");
	for(i = gcnt - 1; i >= 0; i--){
		//YYYYMMDD-HHMMSS を表示用にする
		wsprintf(date, "%.4s/%.2s/%.2s %.2s:%.2s:%.2s", gens[i], gens[i] + 4, gens[i] + 6, gens[i] + 9, gens[i] + 11, gens[i] + 13);
		Buf_Str(&b, "<li><a href=\"");
		AddHtmlText(&b, gens[i]);
		Buf_Str(&b, "\">");
		AddHtmlText(&b, date);
		Buf_Str(&b, (i == gcnt - 1) ? STR_ARCHIVE_INDEX_LATEST : "");
		Buf_Str(&b, "</a></li>\r\n");
	}
	Buf_Str(&b, "</ul>\r\n</body></html>\r\n");
	if(b.err == FALSE && (w = WideDup(b.p)) != NULL){
		len = WideCharToMultiByte(CP_UTF8, 0, w, -1, NULL, 0, NULL, NULL);
		if(len > 0 && (u = (char *)HeapAlloc(GetProcessHeap(), 0, len)) != NULL){
			WideCharToMultiByte(CP_UTF8, 0, w, -1, u, len, NULL, NULL);
			WriteAll(path, u, len - 1);
			MemFree(u);
		}
		MemFree(w);
	}
	Buf_Free(&b);
}

/* 新しい世代のファイル名 */
static BOOL NewGenName(ARCHIVE_JOB *job, const char *ext, char *ret)
{
	SYSTEMTIME st;
	char path[MAX_PATH];
	int i;

	GetLocalTime(&st);
	for(i = 1; i < 100; i++){
		if(i == 1){
			wsprintf(ret, "%04d%02d%02d-%02d%02d%02d.%s", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, ext);
		}else{
			wsprintf(ret, "%04d%02d%02d-%02d%02d%02d-%d.%s", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, i, ext);
		}
		wsprintf(path, "%s\\%s", job->Dir, ret);
		if(FileExists(path) == FALSE){
			return TRUE;
		}
	}
	return FALSE;
}


/******************************************************************************

	RunJob

	ページを取得してアーカイブする (スレッドで実行)

******************************************************************************/

static void RunJob(ARCHIVE_JOB *job)
{
	ARCHIVE_RESP r;
	char path[MAX_PATH], gen[64];
	char md5[33], oldmd5[33], oldgen[64];
	char *out = NULL, *latest, *old, *p;
	char **gens = NULL;
	const char *ext;
	long outlen = 0, oldlen;
	int gcnt = 0, i;
	BOOL html, write = TRUE, found;

	ZeroMemory(&r, sizeof(ARCHIVE_RESP));
	//Archive フォルダ、アイテムのフォルダ、リソースのフォルダの順に作成する
	lstrcpy(path, job->Dir);
	for(p = path + lstrlen(path); p > path && *p != '\\'; p--);
	*p = '\0';
	if(*path == '\0' || EnsureDir(path) == FALSE || EnsureDir(job->Dir) == FALSE){
		return;
	}
	wsprintf(path, "%s\\%s", job->Dir, ARCHIVE_RES_DIR);
	if(EnsureDir(path) == FALSE){
		return;
	}
	job->hSession = WinHttpOpen((job->UserAgent != NULL) ? job->UserAgent : USER_AGENT, (job->ProxyName != NULL) ? WINHTTP_ACCESS_TYPE_NAMED_PROXY : WINHTTP_ACCESS_TYPE_NO_PROXY,
		(job->ProxyName != NULL) ? job->ProxyName : WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
	if(job->hSession == NULL){
		return;
	}
	if(job->TimeOutMs > 0){
		WinHttpSetTimeouts(job->hSession, job->TimeOutMs, job->TimeOutMs, job->TimeOutMs, job->TimeOutMs);
	}
	LoadEntries(job);

	//ページの取得 (成功した場合のみアーカイブする)
	if(ArchiveFetch(job, job->Url, NULL, NULL, MAX_RECVSIZE, &r) == FALSE || r.Status != 200 || ArchiveStop != 0){
		goto end;
	}
	if(lstrcmp(r.Type, "text/html") == 0 || lstrcmp(r.Type, "application/xhtml+xml") == 0){
		html = TRUE;
		ext = "html";
	}else if(lstrcmp(r.Type, "text/plain") == 0){
		html = FALSE;
		ext = "txt";
	}else{
		goto end;
	}
	HashHex(r.Body, r.Len, md5, 32);

	//UTF-16 は書き換えずに保存する
	if(html == TRUE && !(r.Len >= 2 && (((unsigned char)r.Body[0] == 0xFF && (unsigned char)r.Body[1] == 0xFE) ||
		((unsigned char)r.Body[0] == 0xFE && (unsigned char)r.Body[1] == 0xFF)))){
		out = Archive_RewriteHtml(r.Body, r.Len, r.FinalUrl, r.Charset, job->Js, ResolveRes, job, &outlen);
		if(out == NULL || ArchiveStop != 0){
			goto end;
		}
	}

	//前回の世代と比較する
	*oldmd5 = *oldgen = '\0';
	wsprintf(path, "%s\\%s", job->Dir, ARCHIVE_LATEST);
	if((latest = ReadAll(path, 1024, NULL)) != NULL){
		if(lstrlen(latest) > 33 && latest[32] == '\t'){
			lstrcpyn(oldmd5, latest, 33);
			lstrcpyn(oldgen, latest + 33, sizeof(oldgen));
			for(p = oldgen; *p != '\0' && *p != '\r' && *p != '\n'; p++);
			*p = '\0';
		}
		MemFree(latest);
	}
	gcnt = ListGens(job->Dir, &gens);
	found = FALSE;
	for(i = 0; i < gcnt; i++){
		if(lstrcmpi(gens[i], oldgen) == 0){
			found = TRUE;
		}
	}
	if(found == TRUE && IsGenName(oldgen) == TRUE && lstrcmp(oldmd5, md5) == 0){
		//ページは変わっていない場合はリソースが変わったときのみ置き換える
		lstrcpy(gen, oldgen);
		wsprintf(path, "%s\\%s", job->Dir, gen);
		if((old = ReadAll(path, MAX_RECVSIZE * 2, &oldlen)) != NULL){
			if((out != NULL && oldlen == outlen && MemEqual(old, out, outlen) == TRUE) ||
				(out == NULL && oldlen == r.Len && MemEqual(old, r.Body, r.Len) == TRUE)){
				write = FALSE;
			}
			MemFree(old);
		}
	}else if(NewGenName(job, ext, gen) == FALSE){
		goto end;
	}
	if(write == TRUE){
		wsprintf(path, "%s\\%s", job->Dir, gen);
		if(WriteAll(path, (out != NULL) ? out : r.Body, (out != NULL) ? outlen : r.Len) == FALSE){
			goto end;
		}
		SetZoneIdentifier(path, job->Url);
		{
			char line[128];
			wsprintf(line, "%s\t%s\r\n", md5, gen);
			wsprintf(path, "%s\\%s", job->Dir, ARCHIVE_LATEST);
			WriteAll(path, line, lstrlen(line));
		}
	}

	//世代数を超えたものを削除
	FreeList(gens, gcnt);
	gcnt = ListGens(job->Dir, &gens);
	for(i = 0; i < gcnt - job->Generation; i++){
		wsprintf(path, "%s\\%s", job->Dir, gens[i]);
		DeleteFile(path);
	}
	if(gcnt > job->Generation){
		FreeList(gens, gcnt);
		gcnt = ListGens(job->Dir, &gens);
	}
	WriteIndex(job, gens, gcnt);
	CleanResources(job, gens, gcnt);
	SaveEntries(job);

	//元のURL (確認用)
	{
		ARCHIVE_BUF b;
		ZeroMemory(&b, sizeof(ARCHIVE_BUF));
		Buf_Str(&b, job->Url);
		Buf_Str(&b, "\r\n");
		if(job->Title != NULL && IsSafeText(job->Title) == TRUE){
			Buf_Str(&b, job->Title);
			Buf_Str(&b, "\r\n");
		}
		wsprintf(path, "%s\\%s", job->Dir, ARCHIVE_URLFILE);
		if(b.err == FALSE){
			WriteAll(path, b.p, b.len);
		}
		Buf_Free(&b);
	}
end:
	FreeList(gens, gcnt);
	Archive_Free(out);
	FreeResp(&r);
	FreeEntries(job->Entries);
	job->Entries = NULL;
	WinHttpCloseHandle(job->hSession);
	job->hSession = NULL;
}


/**************************************************************************
	Queue
**************************************************************************/

static void FreeJob(ARCHIVE_JOB *job)
{
	ARCHIVE_MEMO *m, *n;

	for(m = job->Memo; m != NULL; m = n){
		n = m->next;
		MemFree(m->url);
		MemFree(m->file);
		MemFree(m);
	}
	FreeEntries(job->Entries);
	MemFree(job->Url);
	MemFree(job->Title);
	WideFree(job->ProxyName);
	WideFree(job->ProxyUser);
	WideFree(job->ProxyPass);
	WideFree(job->UserAgent);
	MemFree(job);
}

static DWORD WINAPI ArchiveThreadProc(LPVOID lpParam)
{
	HMODULE hModule = (HMODULE)lpParam;
	ARCHIVE_JOB *job;

	while(ArchiveStop == 0){
		EnterCriticalSection(&ArchiveCs);
		job = ArchiveQueue;
		if(job != NULL){
			ArchiveQueue = job->next;
			ArchiveQueueCnt--;
			//取り出しと同時に処理中にする (終了時の待機で空と判定されないよう)
			InterlockedExchange(&ArchiveBusy, 1);
			lstrcpy(ArchiveRunningKey, job->Key);
		}
		LeaveCriticalSection(&ArchiveCs);
		if(job == NULL){
			WaitForSingleObject(ArchiveEvent, INFINITE);
			continue;
		}
		RunJob(job);
		FreeJob(job);
		EnterCriticalSection(&ArchiveCs);
		*ArchiveRunningKey = '\0';
		LeaveCriticalSection(&ArchiveCs);
		InterlockedExchange(&ArchiveBusy, 0);
	}
	FreeLibraryAndExitThread(hModule, 0);
	return 0;
}


/******************************************************************************

	Archive_Init

	初期化

******************************************************************************/

void Archive_Init(void)
{
	if(ArchiveCsInit == TRUE){
		return;
	}
	InitializeCriticalSection(&ArchiveCs);
	ArchiveEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	ArchiveCsInit = TRUE;
}


/******************************************************************************

	Archive_End

	終了 (取得中のものは中止する)

******************************************************************************/

void Archive_End(void)
{
	ARCHIVE_JOB *job, *next;

	DWORD start;
	BOOL empty;

	if(ArchiveCsInit == FALSE){
		return;
	}
	//チェック後に終了する場合などのため、取得待ちのものを一定時間まで処理する
	if(ArchiveThread != NULL){
		start = GetTickCount();
		while(GetTickCount() - start < ARCHIVE_FINISH_WAIT){
			EnterCriticalSection(&ArchiveCs);
			empty = (ArchiveQueue == NULL) ? TRUE : FALSE;
			LeaveCriticalSection(&ArchiveCs);
			if(empty == TRUE && ArchiveBusy == 0){
				break;
			}
			if(WaitForSingleObject(ArchiveThread, 100) != WAIT_TIMEOUT){
				break;
			}
		}
	}
	InterlockedExchange(&ArchiveStop, 1);
	EnterCriticalSection(&ArchiveCs);
	job = ArchiveQueue;
	ArchiveQueue = NULL;
	ArchiveQueueCnt = 0;
	LeaveCriticalSection(&ArchiveCs);
	for(; job != NULL; job = next){
		next = job->next;
		FreeJob(job);
	}
	if(ArchiveThread != NULL){
		SetEvent(ArchiveEvent);
		//スレッドはDLLの参照を持っているため、待機時間内に終わらない場合もDLLは解放されない
		if(WaitForSingleObject(ArchiveThread, ARCHIVE_END_WAIT) != WAIT_TIMEOUT){
			CloseHandle(ArchiveThread);
			ArchiveThread = NULL;
		}
	}
	ArchiveRestart = FALSE;
}


/******************************************************************************

	ResumeArchive

	停止したアーカイブの取得を再開する (スレッドが終了していない場合は再開しない)

******************************************************************************/

static BOOL ResumeArchive(void)
{
	if(ArchiveStop == 0){
		return TRUE;
	}
	if(ArchiveRestart == FALSE){
		return FALSE;
	}
	if(ArchiveThread != NULL){
		if(WaitForSingleObject(ArchiveThread, 0) == WAIT_TIMEOUT){
			return FALSE;
		}
		CloseHandle(ArchiveThread);
		ArchiveThread = NULL;
	}
	EnterCriticalSection(&ArchiveCs);
	*ArchiveRunningKey = '\0';
	LeaveCriticalSection(&ArchiveCs);
	InterlockedExchange(&ArchiveBusy, 0);
	InterlockedExchange(&ArchiveStop, 0);
	ArchiveRestart = FALSE;
	return TRUE;
}


/******************************************************************************

	Archive_Restart

	プロトコルの再読み込みで終了通知の後に呼ばれた場合にアーカイブの取得を再開する

******************************************************************************/

void Archive_Restart(void)
{
	if(ArchiveCsInit == FALSE || ArchiveStop == 0){
		return;
	}
	ArchiveRestart = TRUE;
	ResumeArchive();
}


/******************************************************************************

	Archive_IsAuthItem

	認証を設定したアイテムか (認証が必要なページはアーカイブしない)

******************************************************************************/

BOOL Archive_IsAuthItem(struct TPITEM *tpItemInfo)
{
	char *sv, *user;
	BOOL ret = FALSE;

	if(tpItemInfo == NULL){
		return FALSE;
	}
	if(GetOptionInt(tpItemInfo->Option2, OP2_USEPASS) == 1){
		return TRUE;
	}
	if(tpItemInfo->CheckURL == NULL){
		return FALSE;
	}
	//URL にユーザ名を含む場合
	sv = (char *)GlobalAlloc(GPTR, lstrlen(tpItemInfo->CheckURL) + 1);
	user = (char *)GlobalAlloc(GPTR, lstrlen(tpItemInfo->CheckURL) + 1);
	if(sv == NULL || user == NULL){
		ret = TRUE;
	}else if(GetURL(tpItemInfo->CheckURL, sv, NULL, 80, user, NULL) != -1 && *user != '\0'){
		ret = TRUE;
	}
	if(sv != NULL) GlobalFree(sv);
	if(user != NULL) GlobalFree(user);
	return ret;
}

/* アイテムのプロキシ */
static void GetProxy(struct TPITEM *tpItemInfo, ARCHIVE_JOB *job)
{
	char ItemProxy[BUFSIZE], ItemPort[BUFSIZE], sv[BUFSIZE], proxy[BUFSIZE * 2];
	char *px = NULL;
	int pt = 0, port, flag;

	flag = GetOptionInt(tpItemInfo->Option2, OP2_NOPROXY);
	if(flag == 0 && GetOptionInt(tpItemInfo->Option2, OP2_SETPROXY) == 1){
		GetOptionString(tpItemInfo->Option2, ItemProxy, OP2_PROXY);
		GetOptionString(tpItemInfo->Option2, ItemPort, OP2_PORT);
		px = ItemProxy;
		pt = atoi(ItemPort);
	}else if(flag == 0 && Proxy == 1){
		px = pServer;
		pt = pPort;
	}
	if(px == NULL || *px == '\0'){
		return;
	}
	port = GetURL(px, sv, NULL, pt, NULL, NULL);
	if(port == -1){
		lstrcpyn(sv, px, BUFSIZE);
		port = pt;
	}
	wsprintf(proxy, "http://%s:%d", sv, port);
	job->ProxyName = WideDup(proxy);
	if(pUsePass == 1){
		job->ProxyUser = WideDup(pUser);
		job->ProxyPass = WideDup(pPass);
	}
}

/* アーカイブしたページがあるか */
static BOOL HasPage(const char *dir)
{
	char **gens;
	int cnt;

	cnt = ListGens(dir, &gens);
	FreeList(gens, cnt);
	return (cnt > 0) ? TRUE : FALSE;
}


/******************************************************************************

	Archive_Request

	チェックに成功したページのアーカイブを要求する (メインスレッドで実行)

******************************************************************************/

void Archive_Request(HWND hWnd, struct TPITEM *tpItemInfo, BOOL Update)
{
	ARCHIVE_JOB *job, *j;
	char dir[MAX_PATH];
	char key[ARCHIVE_KEYLEN + 1];
	char ua[BUFSIZE];
	HMODULE hModule;
	DWORD id;
	int gen;

	if(ArchiveGlobal == 0 || ArchiveCsInit == FALSE || ResumeArchive() == FALSE ||
		tpItemInfo == NULL || tpItemInfo->Param3 != 0 ||
		GetOptionInt(tpItemInfo->Option2, OP2_NOARCHIVE) == 1 || Archive_IsAuthItem(tpItemInfo) == TRUE ||
		IsWebURL(tpItemInfo->CheckURL) == FALSE || lstrlen(tpItemInfo->CheckURL) > ARCHIVE_URL_MAX){
		return;
	}
	if(GetItemDir(hWnd, tpItemInfo->CheckURL, dir, key) == FALSE){
		return;
	}
	//更新が無い場合はアーカイブが無いときのみ
	if(Update == FALSE && HasPage(dir) == TRUE){
		return;
	}

	EnterCriticalSection(&ArchiveCs);
	for(j = ArchiveQueue; j != NULL; j = j->next){
		if(lstrcmp(j->Key, key) == 0){
			break;
		}
	}
	LeaveCriticalSection(&ArchiveCs);
	if(j != NULL || ArchiveQueueCnt >= ARCHIVE_MAX_QUEUE){
		return;
	}

	job = (ARCHIVE_JOB *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(ARCHIVE_JOB));
	if(job == NULL){
		return;
	}
	job->Url = ArchiveStrDup(tpItemInfo->CheckURL);
	job->Title = ArchiveStrDup(tpItemInfo->Title);
	lstrcpy(job->Dir, dir);
	lstrcpy(job->Key, key);
	job->Image = (GetOptionInt(tpItemInfo->Option2, OP2_ARCHIVE_NOIMAGE) == 0) ? TRUE : FALSE;
	job->Css = (GetOptionInt(tpItemInfo->Option2, OP2_ARCHIVE_NOCSS) == 0) ? TRUE : FALSE;
	job->Js = (GetOptionInt(tpItemInfo->Option2, OP2_ARCHIVE_JS) == 1) ? TRUE : FALSE;
	gen = GetOptionInt(tpItemInfo->Option2, OP2_ARCHIVE_GEN);
	job->Generation = (gen < 1) ? 1 : ((gen > ARCHIVE_GEN_MAX) ? ARCHIVE_GEN_MAX : gen);
	job->TimeOutMs = (TimeOut > 0) ? TimeOut * 1000 : 0;
	GetProxy(tpItemInfo, job);
	//チェックと同じ User-Agent を使う
	if(GetOptionString(tpItemInfo->Option2, ua, OP2_USERAGENT) == TRUE){
		NormalizeUserAgent(ua);
		if(*ua != '\0'){
			job->UserAgent = WideDup(ua);
		}
	}
	if(job->Url == NULL){
		FreeJob(job);
		return;
	}

	EnterCriticalSection(&ArchiveCs);
	job->next = NULL;
	if(ArchiveQueue == NULL){
		ArchiveQueue = job;
	}else{
		for(j = ArchiveQueue; j->next != NULL; j = j->next);
		j->next = job;
	}
	ArchiveQueueCnt++;
	LeaveCriticalSection(&ArchiveCs);

	if(ArchiveThread == NULL){
		//スレッドの実行中にDLLが解放されないよう参照を取得する
		if(GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCSTR)ArchiveThreadProc, &hModule) == FALSE){
			return;
		}
		ArchiveThread = CreateThread(NULL, 0, ArchiveThreadProc, (LPVOID)hModule, 0, &id);
		if(ArchiveThread == NULL){
			FreeLibrary(hModule);
			return;
		}
		SetThreadPriority(ArchiveThread, THREAD_PRIORITY_BELOW_NORMAL);
	}
	SetEvent(ArchiveEvent);
}


/******************************************************************************

	Archive_Exists

	アイテムのアーカイブがあるか

******************************************************************************/

BOOL Archive_Exists(HWND hWnd, struct TPITEM *tpItemInfo)
{
	char dir[MAX_PATH];

	if(tpItemInfo == NULL || GetItemDir(hWnd, tpItemInfo->CheckURL, dir, NULL) == FALSE){
		return FALSE;
	}
	return HasPage(dir);
}


/* ディレクトリ内のファイルを削除する (サブディレクトリは削除しない) */
static void DeleteFiles(const char *dir)
{
	WIN32_FIND_DATA fd;
	HANDLE hFind;
	char path[MAX_PATH];

	if(lstrlen(dir) + 3 >= MAX_PATH){
		return;
	}
	wsprintf(path, "%s\\*", dir);
	hFind = FindFirstFile(path, &fd);
	if(hFind == INVALID_HANDLE_VALUE){
		return;
	}
	do{
		if((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0 || lstrlen(dir) + 1 + lstrlen(fd.cFileName) >= MAX_PATH){
			continue;
		}
		wsprintf(path, "%s\\%s", dir, fd.cFileName);
		if((fd.dwFileAttributes & FILE_ATTRIBUTE_READONLY) != 0){
			SetFileAttributes(path, fd.dwFileAttributes & ~FILE_ATTRIBUTE_READONLY);
		}
		DeleteFile(path);
	}while(FindNextFile(hFind, &fd) == TRUE);
	FindClose(hFind);
}


/******************************************************************************

	Archive_Delete

	アイテムのアーカイブを削除する
	(戻り値 1: 削除した 0: アーカイブが無い -1: 取得中 -2: 削除できなかった)

******************************************************************************/

int Archive_Delete(HWND hWnd, struct TPITEM *tpItemInfo)
{
	ARCHIVE_JOB *job, *prev, *next, *removed = NULL;
	char dir[MAX_PATH], path[MAX_PATH];
	char key[ARCHIVE_KEYLEN + 1];
	DWORD attr;

	if(tpItemInfo == NULL || GetItemDir(hWnd, tpItemInfo->CheckURL, dir, key) == FALSE){
		return 0;
	}
	attr = GetFileAttributes(dir);
	if(attr == INVALID_FILE_ATTRIBUTES || (attr & FILE_ATTRIBUTE_DIRECTORY) == 0){
		return 0;
	}
	if(ArchiveCsInit == TRUE){
		EnterCriticalSection(&ArchiveCs);
		//取得中のものは削除しない
		if(lstrcmp(ArchiveRunningKey, key) == 0){
			LeaveCriticalSection(&ArchiveCs);
			return -1;
		}
		//取得待ちのものは取り消す
		for(prev = NULL, job = ArchiveQueue; job != NULL; job = next){
			next = job->next;
			if(lstrcmp(job->Key, key) == 0){
				if(prev == NULL){
					ArchiveQueue = next;
				}else{
					prev->next = next;
				}
				ArchiveQueueCnt--;
				job->next = removed;
				removed = job;
			}else{
				prev = job;
			}
		}
		LeaveCriticalSection(&ArchiveCs);
		for(job = removed; job != NULL; job = next){
			next = job->next;
			FreeJob(job);
		}
	}

	//リソース、ページの順に削除する (作成したもの以外のサブディレクトリは削除しない)
	wsprintf(path, "%s\\%s", dir, ARCHIVE_RES_DIR);
	DeleteFiles(path);
	RemoveDirectory(path);
	DeleteFiles(dir);
	RemoveDirectory(dir);
	return (GetFileAttributes(dir) == INVALID_FILE_ATTRIBUTES) ? 1 : -2;
}


/******************************************************************************

	Archive_Open

	アイテムのアーカイブを開く

******************************************************************************/

int Archive_Open(HWND hWnd, struct TPITEM *tpItemInfo)
{
	char dir[MAX_PATH], path[MAX_PATH];
	char **gens;
	int cnt, rc;

	if(tpItemInfo == NULL || GetItemDir(hWnd, tpItemInfo->CheckURL, dir, NULL) == FALSE ||
		(cnt = ListGens(dir, &gens)) <= 0){
		MessageBox(hWnd, STR_ERR_MSG_ARCHIVE_NONE, STR_ERR_TITLE_ARCHIVE, MB_ICONEXCLAMATION);
		return -1;
	}
	//複数の世代がある場合は一覧を開く
	wsprintf(path, "%s\\%s", dir, ARCHIVE_INDEX);
	if(cnt <= 1 || FileExists(path) == FALSE){
		wsprintf(path, "%s\\%s", dir, gens[cnt - 1]);
	}
	FreeList(gens, cnt);
	rc = ExecItem(hWnd, path, NULL);
	return (rc == -1) ? -1 : 0;
}
/* End of source */
