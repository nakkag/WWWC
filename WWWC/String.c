/**************************************************************************

	WWWC

	String.c

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


/**************************************************************************
	Define
**************************************************************************/

#define to_lower(c)				((c >= TEXT('A') && c <= TEXT('Z')) ? (c - TEXT('A') + TEXT('a')) : c)

/**************************************************************************
	Global Variables
**************************************************************************/

extern char DateFormat[];
extern char TimeFormat[];


/******************************************************************************

	AllocCopy

	バッファを確保して文字列のコピー

******************************************************************************/

char *AllocCopy(char *buf)
{
	char *ret;

	if(buf == NULL){
		return NULL;
	}
	ret = (char *)GlobalAlloc(GMEM_FIXED, lstrlen(buf) + 1);
	if(ret == NULL){
		return NULL;
	}
	lstrcpy(ret, buf);
	return ret;
}


/******************************************************************************

	Trim

	文字列の前後の空白, Tabを除去する

******************************************************************************/

BOOL Trim(char *buf)
{
	char *p, *r;

	//前後の空白を除いたポインタを取得
	for(p = buf;(*p == ' ' || *p == '\t') && *p != '\0';p++);
	for(r = buf + lstrlen(buf) - 1; r > p && (*r == ' ' || *r == '\t'); r--);
	*(r + 1) = '\0';

	//元の文字列にコピーを行う
	MoveMemory(buf, p, lstrlen(p) + 1);
	return TRUE;
}


/******************************************************************************

	iStrCpy

	文字列をコピーして最後の文字のアドレスを返す

******************************************************************************/

char *iStrCpy(char *ret, char *buf)
{
	if(buf == NULL){
		return ret;
	}
	while(*buf != '\0'){
		*(ret++) = *(buf++);
	}
	*ret = '\0';
	return ret;
}


/******************************************************************************

	GetStrLen

	比較用の文字列の長さを取得する

******************************************************************************/

static int GetStrLen(const char *buf, int len)
{
	int i;

	if(len < 0) return len;

	for(i = 0; i < len; i++){
		if(*buf == '\0'){
			break;
		}
		buf++;
	}
	return i;
}


/******************************************************************************

	lstrcmpn

	２つの文字列を文字数分比較を行う

******************************************************************************/

int lstrcmpn(char *buf1, char *buf2, int len)
{
	int ret;
	int len1, len2;

	len1 = GetStrLen(buf1, len);
	len2 = GetStrLen(buf2, len);

	ret = CompareString(MAKELCID(MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), SORT_DEFAULT),
		0, buf1, len1, buf2, len2);

	return ret - 2;
}


/******************************************************************************

	lstrcmpni

	2つの文字列を大文字、小文字を区別しないで比較を行う

******************************************************************************/

int lstrcmpni(char *buf1, char *buf2, int len)
{
	int ret;
	int len1, len2;

	len1 = GetStrLen(buf1, len);
	len2 = GetStrLen(buf2, len);

	ret = CompareString(MAKELCID(MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), SORT_DEFAULT),
		NORM_IGNORECASE, buf1, len1, buf2, len2);

	return ret - 2;
}


/******************************************************************************

	str_match

	2つの文字列をワイルドカード(*)を使って比較を行う

******************************************************************************/

BOOL str_match(const TCHAR *ptn, const TCHAR *str)
{
	switch (*ptn) {
	case TEXT('\0'):
		return (*str == TEXT('\0'));
	case TEXT('*'):
		if (*(ptn + 1) == TEXT('\0')) {
			return TRUE;
		}
		if (str_match(ptn + 1, str) == TRUE) {
			return TRUE;
		}
		while (*str != TEXT('\0')) {
			str++;
			if (str_match(ptn + 1, str) == TRUE) {
				return TRUE;
			}
		}
		return FALSE;
	case TEXT('?'):
		return (*str != TEXT('\0')) && str_match(ptn + 1, str + 1);
	default:
		while (to_lower(*ptn) == to_lower(*str)) {
			if (*ptn == TEXT('\0')) {
				return TRUE;
			}
			ptn++;
			str++;
			if (*ptn == TEXT('*') || *ptn == TEXT('?')) {
				return str_match(ptn, str);
			}
		}
		return FALSE;
	}
}


/******************************************************************************

	strlistcmp

	フォーマット内の複数の文字列に指定の文字列が含まれているか調べる
	(フォーマットにはワイルドカード使用可)
	 Sep は区切り文字

******************************************************************************/

BOOL strlistcmp(char *buf, char *Format, char Sep)
{
	char *p, *r;
	char *tmp;

	if(*Format == '\0'){
		return TRUE;
	}

	tmp = (char *)GlobalAlloc(GPTR, lstrlen(Format) + 1);
	if(tmp == NULL){
		return FALSE;
	}

	p = Format;
	r = tmp;
	while(1){
		//区切り文字か文字列の終端だった場合
		if(*p == '\0' || *p == Sep){
			*r = '\0';

			Trim(tmp);

			if(str_match(tmp, buf) == TRUE){
				GlobalFree(tmp);
				return TRUE;
			}

			if(*p == '\0' || *(p++) == '\0'){
				GlobalFree(tmp);
				return FALSE;
			}

			//次の文字列を取得
			r = tmp;
		}else{
			*(r++) = *(p++);
		}
	}
}


/******************************************************************************

	FileNameCheck

	ファイル名にできない文字列が含まれていないかチェックを行う

******************************************************************************/

int FileNameCheck(char *buf)
{
	char *p;

	p = buf;

	while(*p != '\0'){
		if(IsDBCSLeadByte((BYTE)*p) == TRUE){
			//２バイトコードの場合
			p++;

		}else{
			//ファイル名にできない文字の場合は偽を返す
			if(*p == '\\' ||
				*p == '/' ||
				*p == ':' ||
				*p == ',' ||
				*p == ';' ||
				*p == '*' ||
				*p == '?' ||
				*p == '\"' ||
				*p == '<' ||
				*p == '>' ||
				*p == '|'){

				return FALSE;
			}
		}
		p++;
	}
	return TRUE;
}


/******************************************************************************

	FileNameConv

	ファイル名にできない文字を変換する

******************************************************************************/

void FileNameConv(char *buf, char NewChar)
{
	char *p;

	p = buf;

	while(*p != '\0'){
		if(IsDBCSLeadByte((BYTE)*p) == TRUE){
			//２バイトコードの場合
			p++;

		}else{
			//ファイル名にできない文字は指定の文字に変換
			if(*p == '\\' ||
				*p == '/' ||
				*p == ':' ||
				*p == ',' ||
				*p == ';' ||
				*p == '*' ||
				*p == '?' ||
				*p == '\"' ||
				*p == '<' ||
				*p == '>' ||
				*p == '|'){

				*p = NewChar;
			}
		}
		p++;
	}
}


/******************************************************************************

	EscToCode

	エスケープ文字をコードに変換

******************************************************************************/

void EscToCode(char *buf)
{
	char *p, *r;

	for(p = buf, r = buf; *p != '\0'; p++){
		if(IsDBCSLeadByte(*p) == TRUE){
			*(r++) = *(p++);
			*(r++) = *p;
		}else if(*p == '\\' && *(p + 1) == 'n'){
			*(r++) = '\r';
			*(r++) = '\n';
			p++;
		}else if(*p == '\\' && *(p + 1) == '\\'){
			*(r++) = '\\';
			p++;
		}else{
			*(r++) = *p;
		}
	}
	*r = '\0';
}


/******************************************************************************

	CodeToEsc

	コードをエスケープ文字に変換

******************************************************************************/

char *CodeToEsc(char *buf)
{
	char *ret;
	char *p, *r;

	ret = (char *)GlobalAlloc(GPTR, (lstrlen(buf) * 2) + 1);
	if(ret == NULL){
		return NULL;
	}
	//改行、タブを変換
	for(p = buf, r = ret ;*p != '\0'; p++){
		if(IsDBCSLeadByte(*p) == TRUE){
			*(r++) = *(p++);
			*(r++) = *p;
			continue;
		}
		switch(*p)
		{
		case '\r':
			break;

		case '\n':
			*(r++) = '\\';
			*(r++) = 'n';
			break;

		case '\t':
			*(r++) = ' ';
			break;

		case '\\':
			*(r++) = '\\';
			*(r++) = '\\';
			break;

		default:
			*(r++) = *p;
			break;
		}
	}
	*r = '\0';
	return ret;
}


/******************************************************************************

	GetNumString

	数字文字のみを取得する

******************************************************************************/

void GetNumString(char *buf, char *ret)
{
	char *p, *r;

	//数字文字のみを戻り値のバッファに設定
	for(p = buf, r = ret;*p != '\0';p++){
		if(IsDBCSLeadByte((BYTE)*p) == TRUE){
			//２バイトコードの場合
			p++;

		}else if(*p >= '0' && *p <= '9'){
			*(r++) = *p;
		}
	}
	*r = '\0';
}


/******************************************************************************

	GetDateTime

	現在の日時を指定のフォーマットで作成する

******************************************************************************/

void GetDateTime(char *fDay, char *fTime)
{
	char *p;

	//フォーマットが設定されてない場合はシステムのフォーマットを使用する
	p = (*DateFormat == '\0') ? NULL : DateFormat;
	GetDateFormat(0, 0, NULL, p, fDay, BUFSIZE - 1);

	p = (*TimeFormat == '\0') ? NULL : TimeFormat;
	GetTimeFormat(0, 0, NULL, p, fTime, BUFSIZE - 1);
}


/******************************************************************************

	GetIndexToString

	インデックスより情報を取得する

******************************************************************************/

char *GetIndexToString(struct TPITEM *tpItemInfo, int Index)
{
	char *p = NULL;

	switch(Index)
	{
	case 0:
		p = tpItemInfo->Title;
		break;

	case 1:
		p = tpItemInfo->CheckURL;
		break;

	case 2:
		p = tpItemInfo->Size;
		break;

	case 3:
		p = tpItemInfo->Date;
		break;

	case 4:
		p = tpItemInfo->CheckDate;
		break;

	case 5:
		p = tpItemInfo->OldSize;
		break;

	case 6:
		p = tpItemInfo->OldDate;
		break;

	case 7:
		p = tpItemInfo->ViewURL;
		break;

	case 8:
		p = tpItemInfo->Option1;
		break;

	case 9:
		p = tpItemInfo->Option2;
		break;

	case 10:
		p = tpItemInfo->Comment;
		break;

	case 12:
		p = tpItemInfo->ErrStatus;
		break;

	case 13:
		p = tpItemInfo->DLLData1;
		break;

	case 14:
		p = tpItemInfo->DLLData2;
		break;
	}
	return ((p != NULL) ? p : "");
}
/* End of source */