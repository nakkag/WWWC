/**************************************************************************

	WWWC (wwwc.dll)

	String.h

	Copyright (C) 1996-2026 by Ohno Tomoaki. All rights reserved.
		http://www.nakka.com/
		nakka@nakka.com

**************************************************************************/

#ifndef _INC_STRING_H
#define _INC_STRING_H

/**************************************************************************
	Function Prototypes
**************************************************************************/

void BadNameCheck(char *buf, char NewChar);
void EncodePassword(char *pass, char *ret, int size);
BOOL DecodePassword(char *buf, char *ret, int size);
UINT str2hash(const TCHAR *str);
int lstrcmpn(char *buf1, char *buf2, int len);
int lstrcmpni(char *buf1, char *buf2, int len);
char *iStrCpy(char *ret, char *buf);
WCHAR *iStrCpyW(WCHAR *ret, char *buf);
BOOL str_match(const TCHAR *ptn, const TCHAR *str);
int LresultCmp(char *Oldbuf, char *Newbuf);
int GetURL(char *URL, char *server, char *path, int DefPort, char *user, char *pass);
BOOL IsWebURL(char *URL);
char *CreateMoveURL(char *FromURL, char *ToURL);
BOOL GetOptionString(char *buf, char *ret, int num);
BOOL GetOptionStringEx(char *buf, char *ret, int size, int num);
char *ReplaceOptionFields(char *buf, int start, int cnt, char **vals);
void NormalizeUserAgent(char *buf);
int GetOptionInt(char *buf, int num);
void DelCrLf(char *buf);
void DelCtrlChar(char *buf);
char *StrNextContent(char *p);
int GetHeadContentSize(char *buf, char *str);
BOOL GetHeadContent(char *buf, char *str, char *ret);
long HeaderSize(char *buf);
int GetTagContentSize(char *buf, char *tag);
BOOL GetTagContent(char *buf, char *tag, char *content);
char *GetMETATag(char *buf, char *type, char *name, char *contentname);
int DelTagSize(char *buf);
void ConvHTMLChar(char *buf);
int DateConv(char *buf, char *ret);
int CreateDateTime(char *ret);
void eBase(char *buf, char *ret);
void SjisShift(int *ph, int *pl);
char *JIS_SJIS(char *buf, char *ret);
char *EUC_SJIS(char *buf, char *ret);
int GetKanjiType(unsigned char *str);
char *ConvStrCode(char *buf, char *ret);
char *SrcConv(char *buf, long Size);
int GetHtml2RtfSize(char *buf);
void Html2Rtf(char *buf, char *ret);
void SetResLanguage(void);
char *GetResStr(UINT id);
void FreeResStr(void);

#endif
/* End of source */
