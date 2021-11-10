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

//�O���Q��
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

	�^�O�P�ʂŕ�����̌���

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

	���䕶����̕ϊ�

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

	���s�����̕ϊ�

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

	��������������ĊJ�n�ƏI���ʒu���擾

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

	������Ƀt�B���^��������

******************************************************************************/

static BOOL StrFilter(char *buf, char *sStr, char *eStr, int f, BOOL icase)
{
	char *s1 = NULL, *e1 = NULL;
	char *s2 = NULL, *e2 = NULL;

	//�J�n�ʒu�̌���
	if(FindFIlterString(buf, sStr, &s1, &e1, icase) == FALSE){
		return FALSE;
	}

	//�I���ʒu�̌���
	if(eStr != NULL && *eStr != '\0'){
		if(FindFIlterString(e1, eStr, &s2, &e2, icase) == FALSE){
			return FALSE;
		}
		e1 = e2;
	}

	if(f == 0){
		//������̍폜
		lstrcpy(s1, e1);
	}else{
		//������̒��o
		lstrcpyn(buf, s1, e1 - s1 + 1);
	}
	return TRUE;
}


/******************************************************************************

	FilterMatch

	�t�B���^�Ώە�����̃`�F�b�N

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

	�t�B���^��������

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
				//�\�[�X�ϊ�
				tmp = SrcConv(buf, Size);
				if(tmp == NULL){
					break;
				}
				CRLFConv(tmp);
			}
			//�t�B���^�̓K�p
			if(StrFilter(tmp, (FilterInfo + i)->string1, (FilterInfo + i)->string2, 
				(FilterInfo + i)->flag, (FilterInfo + i)->IgnoreCase) == FALSE){
				//���o�G���[�͋�ɂ���ݒ�
				if((FilterInfo + i)->flag == 1 && (FilterInfo + i)->ErrorEmptyBody == TRUE){
					*tmp = '\0';
					continue;
				}
				if((FilterInfo + i)->IgnoreError == FALSE){
					rc = FALSE;
				}
			}else if((FilterInfo + i)->flag == 0 && (FilterInfo + i)->Repeat == TRUE){
				//�J��Ԃ��ăt�B���^�̓K�p
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

	��������m�ۂ��ăR�s�[

******************************************************************************/

static char *AllocItemStr(char *buf, char **ret)
{
	char *p, *r;
	char *EndPoint;

	if(*buf == '\0'){
		return buf;
	}

	//�����̋�؂�ʒu�̎擾
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

	�t�B���^�̓ǂݍ���

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

	/* �t�@�C�����J�� */
	hFile = CreateFile(path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == NULL || hFile == (HANDLE)-1){
		return FALSE;
	}
	/* �m�ۂ���T�C�Y�̎擾 */
	fSizeLow = GetFileSize(hFile, &fSizeHigh);
	if(fSizeLow == 0xFFFFFFFF){
		CloseHandle(hFile);
		return FALSE;
	}
	FileSize = (long)fSizeLow;

	/* �ǂݎ��̈�̊m�� */
	buf = (char *)LocalAlloc(LMEM_FIXED, FileSize + 1);
	if(buf == NULL){
		CloseHandle(hFile);
		return FALSE;
	}
	/* �t�@�C����ǂݍ��� */
	if(ReadFile(hFile, buf, fSizeLow, &ret, NULL) == FALSE){
		LocalFree(buf);
		CloseHandle(hFile);
		return FALSE;
	}
	CloseHandle(hFile);
	*(buf + FileSize) = '\0';

	//�s���̃J�E���g
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

	//�t�B���^�[���̊m��
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
				//����
				case 'D':
				case 'd':
				default:
					(FilterInfo + i)->flag = 0;
					break;

				//���o
				case 'S':
				case 's':
					(FilterInfo + i)->flag = 1;
					break;

				//�啶���Ə���������ʂ��Ȃ�
				case 'C':
				case 'c':
					(FilterInfo + i)->IgnoreCase = TRUE;
					break;

				//�G���[����
				case 'I':
				case 'i':
					(FilterInfo + i)->IgnoreError = TRUE;
					break;

				//�G���[����ɂ��� (S �w�莞)
				case 'E':
				case 'e':
					(FilterInfo + i)->ErrorEmptyBody = TRUE;
					break;

				//�J��Ԃ�
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

	�t�B���^���̉��

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

	�t�B���^�ēǂݍ��݃c�[��

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
