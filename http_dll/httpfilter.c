/**************************************************************************

	WWWC (wwwc.dll)

	httpfilter.c

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
#include <commctrl.h>

#include "String.h"
#include "httpfilter.h"
#include "http.h"
#include "wwwcdll.h"
#include "resource.h"


/**************************************************************************
	Define
**************************************************************************/

#define FILE_FILTER					"filter.txt"


/**************************************************************************
	Global Variables
**************************************************************************/

char fpath[BUFSIZE];

FILTER *FilterInfo;
int FilterCount;

//外部参照
extern HINSTANCE ghinst;


/**************************************************************************
	Local Function Prototypes
**************************************************************************/

static char *FindStr(char *buf, char *Str, int len, BOOL first, BOOL icase);
static char *ConvStr(char *buf);
static BOOL FindFIlterString(char *buf, char *sStr, char **ts, char **te, BOOL icase);
static BOOL StrFilter(char *buf, char *sStr, char *eStr, int f, BOOL icase);
static char *AllocItemStr(char *buf, char **ret);


/******************************************************************************

	FindStr

	タグ単位で文字列の検索

******************************************************************************/

static char *FindStr(char *buf, char *Str, int len, BOOL first, BOOL icase)
{
	char *p;
	BOOL TagFlag = FALSE;

	for(p = buf; *p != '\0'; p++){
		if(((icase == FALSE) ? lstrcmpn : lstrcmpni)(Str, p, len) == 0){
			return p;
		}
		if(IsDBCSLeadByte((BYTE)*p) == TRUE){
			p++;
			continue;
		}
		if(first == TRUE){
			continue;
		}
		switch(*p)
		{
		case '<':
			TagFlag = TRUE;
			break;
		case '>':
			if(TagFlag != TRUE) return NULL;
			TagFlag = FALSE;
			break;
		}
	}
	return NULL;
}


/******************************************************************************

	ConvStr

	制御文字列の変換

******************************************************************************/

static char *ConvStr(char *buf)
{
	char *ret;
	char *p, *r;

	ret = (char *)LocalAlloc(LMEM_FIXED, lstrlen(buf) + 1);
	if(ret == NULL) return ret;

	for(p = buf, r = ret; *p != '\0'; p++){
		if(IsDBCSLeadByte((BYTE)*p) == TRUE){
			*(r++) = *(p++);
			*(r++) = *p;
		}else if(*p == '\\' && *(p + 1) != '\0'){
			p++;
			switch(*p){
			case '\\':
				*(r++) = '\\';
				break;
			case 'n':
				*(r++) = '\n';
				break;
			case 't':
				*(r++) = '\t';
				break;
			}
		}else{
			*(r++) = *p;
		}
	}
	*r = '\0';
	return ret;
}


/******************************************************************************

	CRLFConv

	改行文字の変換

******************************************************************************/

static void CRLFConv(char *buf)
{
	char *p, *r;

	for(p = r = buf; *p != '\0'; p++){
		if(IsDBCSLeadByte((BYTE)*p) == TRUE){
			*(r++) = *(p++);
			*(r++) = *p;
			continue;
		}
		if(*p == '\r'){
			*(r++) = '\n';
			if(*(p + 1) == '\n'){
				p++;
			}
			continue;
		}
		*(r++) = *p;
	}
	*r = '\0';
}


/******************************************************************************

	FindFIlterString

	文字列を検索して開始と終了位置を取得

******************************************************************************/

static BOOL FindFIlterString(char *buf, char *sStr, char **ts, char **te, BOOL icase)
{
	char *p, *r, *t;
	char *cStr;
	int len;

	if(sStr == NULL || *sStr == '\0'){
		return FALSE;
	}

	t = buf;

	cStr = ConvStr(sStr);
	if(cStr == NULL) return FALSE;
	for(p = cStr; *p == '*'; p++);

	while(*p != '\0'){
		for(r = p; *r != '\0' && *r != '*'; r++);
		len = r - p;

		t = FindStr(t, p, len, ((*ts == NULL) ? TRUE : FALSE), icase);
		if(t == NULL){
			if(*ts == NULL){
				LocalFree(cStr);
				return FALSE;
			}
			r = cStr;
			t = *ts + 1;
			*ts = NULL;
		}else{
			if(*ts == NULL) *ts = t;
			t += len;
		}
		for(p = r; *p == '*'; p++);
	}
	if(p == cStr || *ts == NULL){
		LocalFree(cStr);
		return FALSE;
	}
	LocalFree(cStr);
	*te = t;
	return TRUE;
}


/******************************************************************************

	StrFilter

	文字列にフィルタをかける

******************************************************************************/

static BOOL StrFilter(char *buf, char *sStr, char *eStr, int f, BOOL icase)
{
	char *s1 = NULL, *e1 = NULL;
	char *s2 = NULL, *e2 = NULL;

	//開始位置の検索
	if(FindFIlterString(buf, sStr, &s1, &e1, icase) == FALSE){
		return FALSE;
	}

	//終了位置の検索
	if(eStr != NULL && *eStr != '\0'){
		if(FindFIlterString(e1, eStr, &s2, &e2, icase) == FALSE){
			return FALSE;
		}
		e1 = e2;
	}

	if(f == 0){
		//文字列の削除
		lstrcpy(s1, e1);
	}else{
		//文字列の抽出
		lstrcpyn(buf, s1, e1 - s1 + 1);
	}
	return TRUE;
}


/******************************************************************************

	FilterMatch

	フィルタ対象文字列のチェック

******************************************************************************/

BOOL FilterMatch(char *url)
{
	int i;

	if(FilterInfo == NULL) return FALSE;

	for(i = 0; i < FilterCount; i++){
		if((FilterInfo + i)->url == NULL){
			continue;
		}
		if(str_match((FilterInfo + i)->url, url) == TRUE){
			return TRUE;
		}
	}
	return FALSE;
}


/******************************************************************************

	FilterCheck

	フィルタをかける

******************************************************************************/

BOOL FilterCheck(char *url, char *buf, int Size)
{
	char *tmp = NULL;
	BOOL rc = TRUE;
	int i;

	if(FilterInfo == NULL) return FALSE;

	for(i = 0; i < FilterCount; i++){
		if((FilterInfo + i)->url == NULL){
			continue;
		}
		if(str_match((FilterInfo + i)->url, url) == TRUE){
			if(tmp == NULL){
				//ソース変換
				tmp = SrcConv(buf, Size);
				if(tmp == NULL){
					break;
				}
				CRLFConv(tmp);
			}
			//フィルタの適用
			if(StrFilter(tmp, (FilterInfo + i)->string1, (FilterInfo + i)->string2, 
				(FilterInfo + i)->flag, (FilterInfo + i)->IgnoreCase) == FALSE){
				//抽出エラーは空にする設定
				if((FilterInfo + i)->flag == 1 && (FilterInfo + i)->ErrorEmptyBody == TRUE){
					*tmp = '\0';
					continue;
				}
				if((FilterInfo + i)->IgnoreError == FALSE){
					rc = FALSE;
				}
			}else if((FilterInfo + i)->flag == 0 && (FilterInfo + i)->Repeat == TRUE){
				//繰り返してフィルタの適用
				while(StrFilter(tmp, (FilterInfo + i)->string1, (FilterInfo + i)->string2, 
					(FilterInfo + i)->flag, (FilterInfo + i)->IgnoreCase) == TRUE);
			}
		}
	}
	if(tmp != NULL){
		lstrcpy(buf, tmp);
		GlobalFree(tmp);
		return rc;
	}
	return FALSE;
}


/******************************************************************************

	AllocItemStr

	文字列を確保してコピー

******************************************************************************/

static char *AllocItemStr(char *buf, char **ret)
{
	char *p, *r;
	char *EndPoint;

	if(*buf == '\0'){
		return buf;
	}

	//文字の区切り位置の取得
	for(EndPoint = buf; *EndPoint != '\t' && *EndPoint != '\r' && *EndPoint != '\0'; EndPoint++);

	*ret = (char *)LocalAlloc(LMEM_FIXED, EndPoint - buf + 2);
	if(*ret == NULL){
		return NULL;
	}
	p = buf;
	r = *ret;
	while(EndPoint > p){
		*(r++) = *(p++);
	}
	*r = '\0';
	return EndPoint;
}


/******************************************************************************

	ReadFilterFile

	フィルタの読み込み

******************************************************************************/

BOOL ReadFilterFile(char *cpath)
{
	HANDLE hFile;
	char *buf, *p;
	char path[BUFSIZE];
	DWORD fSizeLow, fSizeHigh;
	DWORD ret;
	long FileSize;
	int i;

	lstrcpy(fpath, cpath);
	wsprintf(path, "%s\\%s", cpath, FILE_FILTER);

	/* ファイルを開く */
	hFile = CreateFile(path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == NULL || hFile == (HANDLE)-1){
		return FALSE;
	}
	/* 確保するサイズの取得 */
	fSizeLow = GetFileSize(hFile, &fSizeHigh);
	if(fSizeLow == 0xFFFFFFFF){
		CloseHandle(hFile);
		return FALSE;
	}
	FileSize = (long)fSizeLow;

	/* 読み取る領域の確保 */
	buf = (char *)LocalAlloc(LMEM_FIXED, FileSize + 1);
	if(buf == NULL){
		CloseHandle(hFile);
		return FALSE;
	}
	/* ファイルを読み込む */
	if(ReadFile(hFile, buf, fSizeLow, &ret, NULL) == FALSE){
		LocalFree(buf);
		CloseHandle(hFile);
		return FALSE;
	}
	CloseHandle(hFile);
	*(buf + FileSize) = '\0';

	//行数のカウント
	FilterCount = 0;
	for(p = buf; *p != '\0'; p++){
		if(*p == '\n'){
			FilterCount++;
		}
	}
	if(FilterCount == 0){
		LocalFree(buf);
		FilterInfo = NULL;
		return TRUE;
	}

	//フィルター情報の確保
	FilterInfo = (FILTER *)LocalAlloc(LPTR, sizeof(FILTER) * (FilterCount));
	if(FilterInfo == NULL){
		LocalFree(buf);
		return FALSE;
	}

	for(p = buf; *p == '\r' || *p == '\n'; p++);

	i = 0;
	while(*p != '\0' && i < FilterCount){
		if(*p == '#'){
			for(; *p != '\0' && *p != '\r' && *p != '\n'; p++);
			for(; *p == '\r' || *p == '\n'; p++);
			continue;
		}

		if(*p != '\t'){
			for(; *p != '\0' && *p != '\t'; p++){
				switch(*p)
				{
				//除去
				case 'D':
				case 'd':
				default:
					(FilterInfo + i)->flag = 0;
					break;

				//抽出
				case 'S':
				case 's':
					(FilterInfo + i)->flag = 1;
					break;

				//大文字と小文字を区別しない
				case 'C':
				case 'c':
					(FilterInfo + i)->IgnoreCase = TRUE;
					break;

				//エラー無視
				case 'I':
				case 'i':
					(FilterInfo + i)->IgnoreError = TRUE;
					break;

				//エラー時空にする (S 指定時)
				case 'E':
				case 'e':
					(FilterInfo + i)->ErrorEmptyBody = TRUE;
					break;

				//繰り返し
				case 'R':
				case 'r':
					(FilterInfo + i)->Repeat = TRUE;
					break;
				}
			}
		}

		if(*p == '\t'){
			p = AllocItemStr(++p, &(FilterInfo + i)->url);
			if(p == NULL) break;
		}

		if(*p == '\t'){
			p = AllocItemStr(++p, &(FilterInfo + i)->string1);
			if(p == NULL) break;
		}

		if(*p == '\t'){
			p = AllocItemStr(++p, &(FilterInfo + i)->string2);
			if(p == NULL) break;
		}

		for(; *p != '\0' && *p != '\r' && *p != '\n'; p++);
		for(; *p == '\r' || *p == '\n'; p++);
		i++;
	}
	FilterCount = i;

	LocalFree(buf);
	return TRUE;
}


/******************************************************************************

	FreeFilter

	フィルタ情報の解放

******************************************************************************/

void FreeFilter(void)
{
	int i;

	if(FilterInfo == NULL) return;

	for(i = 0; i < FilterCount; i++){
		if((FilterInfo + i)->url != NULL){
			LocalFree((FilterInfo + i)->url);
		}
		if((FilterInfo + i)->string1 != NULL){
			LocalFree((FilterInfo + i)->string1);
		}
		if((FilterInfo + i)->string2 != NULL){
			LocalFree((FilterInfo + i)->string2);
		}
	}
	LocalFree(FilterInfo);
	FilterInfo = NULL;
	FilterCount = 0;
}


/******************************************************************************

	FilterReload

	フィルタ再読み込みツール

******************************************************************************/

__declspec(dllexport) int CALLBACK FilterReload(HWND hWnd, struct TPITEM **ToolItemList,
													  int ToolItemListCnt, int type, int CheckType)
{
	char path[BUFSIZE];

	if(type == TOOL_EXEC_PORP){
		wsprintf(path, "%s\\%s", fpath, FILE_FILTER);
		return ExecItem(hWnd, path, NULL);
	}

	FreeFilter();
	ReadFilterFile(fpath);
	return 1;
}
/* End of source */
