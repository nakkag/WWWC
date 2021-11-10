/**************************************************************************

	WWWC (wwwc.dll)

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
#undef  _INC_OLE
#include <winsock.h>
#include <commctrl.h>
#include <richedit.h>
#include <DDEML.H>
#include <process.h>
#include <time.h>

#include "wwwcdll.h"
#include "resource.h"

#include "http.h"

/**************************************************************************
	Define
**************************************************************************/

#define ESC					0x1B		/* �G�X�P�[�v�R�[�h */

//RTF�w�b�_
#define RTF_HEAD	"{\\rtf1\\ansi\\ansicpg932\\deff0\\deflang1033\\deflangfe1041"\
	"{\\fonttbl{\\f0\\fnil\\fprq2\\fcharset128 \\'82\\'6c\\'82\\'72 \\'83\\'53\\'83\\'56\\'83\\'62\\'83\\'4e;}"\
	"{\\f1\\froman\\fprq1\\fcharset128 \\'82\\'6c\\'82\\'72 \\'82\\'6f\\'83\\'53\\'83\\'56\\'83\\'62\\'83\\'4e;}}\r\n"\
	"{\\colortbl ;\\red0\\green0\\blue128;\\red0\\green128\\blue128;\\red0\\green0\\blue0;\\red0\\green128\\blue0;}\r\n"\
	"\\viewkind4\\uc1\\pard\\cf1\\lang1041\\f0\\fs18 "

#define to_lower(c)		((c >= TEXT('A') && c <= TEXT('Z')) ? (c - TEXT('A') + TEXT('a')) : c)


/**************************************************************************
	Global Variables
**************************************************************************/

//�O���Q��
extern int TimeZone;

extern char DateFormat[];
extern char TimeFormat[];


/**************************************************************************
	Local Function Prototypes
**************************************************************************/


/******************************************************************************

	BadNameCheck

	�t�@�C�����Ƃ��ĕs���ȕ�����ϊ�����

******************************************************************************/

void BadNameCheck(char *buf, char NewChar)
{
	char *p;

	p = buf;
	while(*p != '\0'){
		if(IsDBCSLeadByte((BYTE)*p) == FALSE){
			if(*p == '\\' || *p == '/' || *p == ':' || *p == ',' || *p == ';' || *p == '*' ||
				*p == '?' || *p == '\"' || *p == '<' || *p == '>' || *p == '|'){
				*p = NewChar;
			}
		}else{
			p++;
		}
		p++;
	}
}


/******************************************************************************

	ePass

	��������Í�������

******************************************************************************/

void ePass(char *buf, char *ret)
{
	char *p;
	char *retp;

	for(p = buf, retp = ret; *p != '\0'; p++, retp += 2){
		wsprintf(retp, "%02x", (*p) ^ 0xFF);
	}
	*retp = '\0';

}


/******************************************************************************

	dPass

	������𕡍�������

******************************************************************************/

void dPass(char *buf, char *ret)
{
	char *p;
	char tmp[3];
	int i;

	tmp[2] = '\0';

	i = 0;
	for(p = buf; *p != '\0'; p += 2){
		*tmp = *p;
		*(tmp + 1) = *(p + 1);
		ret[i++] = (unsigned char)strtol(tmp, NULL, 16) ^ 0xFF;
	}
	ret[i] = '\0';
}


/******************************************************************************

	x2d

	16�i�\�L�̕�����𐔒l�ɕϊ�����

******************************************************************************/

long x2d(const char *str)
{
	int num = 0;
	int m;

	for(; *str != '\0'; str++){
		if(*str >= '0' && *str <= '9'){
			m = *str - '0';
		}else if(*str >= 'A' && *str <= 'F'){
			m = *str - 'A' + 10;
		}else if(*str >= 'a' && *str <= 'f'){
			m = *str - 'a' + 10;
		}else{
			break;
		}
		num = 16 * num + m;
		if(num > 32768){
			break;
		}
	}
	return num;
}


/******************************************************************************

	str2hash

	������̃n�b�V���l���擾

******************************************************************************/

UINT str2hash(const TCHAR *str)
{
	UINT hash = 0;

	for (; *str != TEXT('\0'); str++) {
		if (*str != TEXT(' ')) {
			hash ^= ((hash << 4) + to_lower(*str));
		}
	}
	return hash;
}


/******************************************************************************

	GetStrLen

	��r�p�̕�����̒������擾����

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

	�w�蕶�����Ŕ�r

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

	�啶���Ə���������ʂ��Ȃ���r

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

	iStrCpy

	��������R�s�[���čŌ�̕����̃A�h���X��Ԃ�

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

	iStrCpyW

	��������R�s�[���čŌ�̕����̃A�h���X��Ԃ�

******************************************************************************/

WCHAR *iStrCpyW(WCHAR *ret, char *buf)
{
	int wlen;
	if (buf == NULL) {
		return ret;
	}
	wlen = MultiByteToWideChar(CP_ACP, 0, buf, -1, NULL, 0);
	MultiByteToWideChar(CP_ACP, 0, buf, -1, ret, wlen);
	return ret + wlen - 1;
}

/******************************************************************************

	str_match

	2�̕���������C���h�J�[�h(*)���g���Ĕ�r���s��

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

	LresultCmp

	���ʂƌÂ����ʂ��r����

******************************************************************************/

int LresultCmp(char *Oldbuf, char *Newbuf)
{
	int len;

	if(Oldbuf == NULL || *Oldbuf == '\0'){
		return 0;
	}
	len = lstrlen(Oldbuf);
	if(*(Oldbuf + len - 1) == '*'){
		/* �X�V�}�[�N���������� */
		len--;
	}
	if(lstrcmpn(Oldbuf, Newbuf, len) != 0){
		/* �X�V�}�[�N��t���� */
		lstrcat(Newbuf, "*");
		return 1;
	}
	return 0;
}


/******************************************************************************

	GetURL

	URL����T�[�o���ƃp�X�𒊏o

******************************************************************************/

int GetURL(char *URL, char *server, char *path, int DefPort, char *user, char *pass)
{
	char *p, *r, *t, wk[10];		/* ��Ɨp�̈� */
	char us[BUFSIZE], ps[BUFSIZE];
	int port;

	port = DefPort;

	/* �X�L�[�������擾���� */
	if(lstrcmpni(URL, "http://", 7) == 0){
		p = URL + 7;
	}else if(lstrcmpni(URL, "https://", 8) == 0){
		p = URL + 8;
	}else{
		return -1;
	}

	/* �T�[�o�����擾���� */
	for(r = server; *p != '/' && *p != '\0'; p++, r++){
		*r = *p;
	}
	*r = '\0';

	if(path != NULL){
		/* �擾����y�[�W�̃p�X���擾���� */
		for(r = path; *p != '#' && *p != '\0'; p++, r++){
			*r = *p;
		}
		*r = '\0';
	}

	for(p = server; *p != '@' && *p != '\0'; p++);
	if(*p == '@'){
		r = us;
		for(p = server; *p != ':' && *p != '@' && *p != '\0'; p++){
			*(r++) = *p;
		}
		*r = '\0';
		if(*p == ':'){
			p++;
			r = ps;
			for(; *p != '@' && *p != '\0'; p++){
				*(r++) = *p;
			}
			*r = '\0';
		}
		if(user != NULL){
			lstrcpy(user, us);
		}
		if(pass != NULL){
			lstrcpy(pass, ps);
		}
		if(*p == '@'){
			p++;
		}
		lstrcpy(server, p);
	}

	/* �T�[�o��:�|�[�g �ƂȂ��Ă���ꍇ�͕������� */
	for(p = server; *p != ':' && *p != '\0'; p++);
	if(*p == ':'){
		for(t = p + 1, r = wk; *t != '\0'; t++, r++){
			if(*t < '0' || *t > '9'){
				return -1;
			}
			*r = *t;
		}
		*r = '\0';
		port = atoi(wk);

		*p = '\0';
	}

	/* �y�[�W�̎w�肪�����ꍇ�́A/ (���[�g)��ݒ肷�� */
	if(path != NULL && *path == '\0'){
		lstrcpy(path, "/");
	}
	return port;
}


/******************************************************************************

	CreateMoveURL

	�ړ����URL���쐬

******************************************************************************/

char *CreateMoveURL(char *FromURL, char *ToURL)
{
	char *ret;
	char *p, *r;

	if(lstrcmpni(ToURL, "http://", 7) == 0 ||
		lstrcmpni(ToURL, "https://", 8) == 0){
		//���S��URL
		ret = GlobalAlloc(GPTR, lstrlen(ToURL) + 1);
		if(ret != NULL) lstrcpy(ret, ToURL);
		return ret;
	}

	if(lstrcmpni(FromURL, "http://", 7) == 0){
		r = FromURL + 7;
	}else if(lstrcmpni(FromURL, "https://", 8) == 0){
		r = FromURL + 8;
	}else{
		return NULL;
	}
	if(*ToURL == '/'){
		//���[�g����ύX
		for(p = r; *p != '\0' && *p != '/'; p++);

	}else{
		//�y�[�W�̂ݕύX
		for(p = FromURL + lstrlen(FromURL); p > r && *p != '/'; p--);
		if(p == r){
			//�y�[�W�w��Ȃ�
			ret = GlobalAlloc(GPTR, lstrlen(FromURL) + 1 + lstrlen(ToURL) + 1);
			if(ret == NULL){
				return NULL;
			}
			p = iStrCpy(ret, FromURL);
			p = iStrCpy(p, "/");
			p = iStrCpy(p, ToURL);
			return ret;
		}else{
			p++;
		}
	}

	ret = GlobalAlloc(GPTR, (p - FromURL + 1) + lstrlen(ToURL) + 1);
	if(ret == NULL){
		return NULL;
	}
	lstrcpyn(ret, FromURL, p - FromURL + 1);
	lstrcpy(ret + (p - FromURL), ToURL);
	return ret;
}


/******************************************************************************

	GetOptionString

	�I�v�V�����̕�������擾����

******************************************************************************/

BOOL GetOptionString(char *buf, char *ret, int num)
{
	char *p, *r;
	int i = 0;

	*ret = '\0';
	if(buf == NULL){
		return FALSE;
	}
	r = ret;
	for(p = buf; *p != '\0'; p++){
		if(*p == ';' && *(p + 1) == ';'){
			*r = '\0';

			if(i == num){
				return ((*ret == '\0') ? FALSE : TRUE);
			}
			i++;
			p++;
			*ret = '\0';
			r = ret;
		}else{
			*(r++) = *p;
		}
	}
	*r = '\0';
	return ((i == num) ? ((*ret == '\0') ? FALSE : TRUE) : FALSE);
}


/******************************************************************************

	GetOptionInt

	�I�v�V�����̐��l���擾����

******************************************************************************/

int GetOptionInt(char *buf, int num)
{
	char ret[BUFSIZE];
	char *p, *r;
	int i = 0;

	*ret = '\0';
	if(buf == NULL){
		return 0;
	}
	r = ret;
	for(p = buf; *p != '\0'; p++){
		if(*p == ';' && *(p + 1) == ';'){
			*r = '\0';

			if(i == num){
				return atoi(ret);
			}
			i++;
			p++;
			*ret = '\0';
			r = ret;
		}else{
			*(r++) = *p;
		}
	}
	*r = '\0';
	return ((i == num) ? atoi(ret) : 0);
}


/******************************************************************************

	DelCrLf

	CR, LF ���폜����

******************************************************************************/

void DelCrLf(char *buf)
{
	char *p, *r, *ret;

	ret = (char *)GlobalAlloc(GMEM_FIXED, lstrlen(buf) + 1);

	for(p = buf, r = ret; *p != '\0'; p++){
		if(*p != '\r' && *p != '\n'){
			*(r++) = *p;
		}
	}
	*r = '\0';

	lstrcpy(buf, ret);
	GlobalFree(ret);
}


/******************************************************************************

	StrNextContent

	�w�b�_���̎��̃R���e���c�̐擪�Ɉړ�����

******************************************************************************/

char *StrNextContent(char *p)
{
	while(1){
		for(; *p != '\r' && *p != '\n' && *p != '\0'; p++);
		if(*p == '\0'){
			break;
		}
		if(*p == '\r' && *(p + 1) == '\n'){
			p += 2;
		}else{
			p++;
		}
		if(*p == ' ' || *p == '\t'){
			continue;
		}
		break;
	}
	return p;
}


/******************************************************************************

	GetHeadContentSize

	�w�b�_���̎w��̏��𒷂����擾����

******************************************************************************/

int GetHeadContentSize(char *buf, char *str)
{
	char *p;
	int i;

	p = buf;
	while(1){
		if(lstrcmpni(p, str, lstrlen(str)) != 0){
			//���̃R���e���c�Ɉړ�����
			p = StrNextContent(p);
			if(*p == '\0' || *p == '\r' || *p == '\n'){
				break;
			}
			continue;
		}

		p += lstrlen(str);
		for(; *p == ' ' || *p == '\t'; p++);

		i = 0;
		while(*p != '\0'){
			if(*p == '\r' || *p == '\n'){
				if(*p == '\r' && *(p + 1) == '\n'){
					p += 2;
				}else{
					p++;
				}
				if(*p != ' ' && *p != '\t'){
					break;
				}
				p++;
				continue;
			}
			p++;
			i++;
		}
		return i;
	}
	return 0;
}


/******************************************************************************

	GetHeadContent

	�w�b�_���̎w��̏����擾����

******************************************************************************/

BOOL GetHeadContent(char *buf, char *str, char *ret)
{
	char *p, *r;

	p = buf;
	while(1){
		if(lstrcmpni(p, str, lstrlen(str)) != 0){
			//���̃R���e���c�Ɉړ�����
			p = StrNextContent(p);
			if(*p == '\0' || *p == '\r' || *p == '\n'){
				break;
			}
			continue;
		}

		p += lstrlen(str);
		for(; *p == ' ' || *p == '\t'; p++);

		r = ret;
		while(*p != '\0'){
			if(*p == '\r' || *p == '\n'){
				if(*p == '\r' && *(p + 1) == '\n'){
					p += 2;
				}else{
					p++;
				}
				if(*p != ' ' && *p != '\t'){
					break;
				}
				p++;
				continue;
			}
			*(r++) = *(p++);
		}
		*r = '\0';
		return TRUE;
	}
	return FALSE;
}


/******************************************************************************

	HeaderSize

	�w�b�_�̃T�C�Y���擾����

******************************************************************************/

long HeaderSize(char *buf)
{
	char *p;

	p = buf;
	while(1){
		for(; *p != '\r' && *p != '\n' && *p != '\0'; p++);
		if(*p == '\0'){
			break;
		}
		if(*p == '\r' && *(p + 1) == '\n'){
			p += 2;
		}else{
			p++;
		}
		if(*p == '\r' || *p == '\n'){
			if(*p == '\r' && *(p + 1) == '\n'){
				p += 2;
			}else{
				p++;
			}
			break;
		}
	}
	return p - buf;
}


/******************************************************************************

	GetTagType

	META�^�O�̃^�C�v���擾

******************************************************************************/

static BOOL GetTagType(char *t, char *type, char *ret)
{
	char *r;

	//type���ƃ}�b�`������̂�����
	while(*t != '\0'){
		for(; *t == ' ' || *t == '\r' || *t == '\n' || *t == '\t'; t++);
		if(lstrcmpni(t, type, lstrlen(type)) == 0){
			break;
		}
		for(; *t != ' ' && *t != '\r' && *t != '\n' && *t != '\t' && *t != '\0'; t++){
			if(*t == '\"'){
				t++;
				for(; *t != '\"' && *t != '\0'; t++);
			}
		}
	}
	if(*t == '\0'){
		return FALSE;
	}

	t = t + lstrlen(type);

	//�^�C�v�̓��e���擾
	if(*t != '\0' && (*t == ' ' || *t == '\r' || *t == '\n' || *t == '\t' || *t == '=')){
		for(; *t != '\"' && *t != '\0'; t++);
		if(*t == '\0'){
			return FALSE;
		}

		t++;

		for(r = ret; *t != '\"' && *t != '\0'; t++){
			*(r++) = *t;
		}
		*r = '\0';
		return TRUE;
	}
	return FALSE;
}


/******************************************************************************

	GetMETATag

	META�^�O�̓��e���擾

******************************************************************************/

char *GetMETATag(char *buf, char *type, char *name, char *contentname)
{
	char *content;
	char *TagContent;
	char *TagType;
	char *p, *r, *t;
	int i = 0;

	p = buf;
	while(*p != '\0'){
		if(*p != '<'){
			//�^�O�̊J�n�܂ňړ�����
			if(IsDBCSLeadByte((BYTE)*p) == TRUE){
				p++;
			}
			p++;
			continue;
		}
		p++;
		for(; *p == ' '; p++);

		//HEAD�̏I���̏ꍇ�͏����𔲂���
		if(lstrcmpni(p, "/head", 5) == 0){
			return NULL;
		}

		//�^�O�̏I���܂ňړ�����
		for(r = p; *r != '>' && *r != '\0'; r++){
			if(IsDBCSLeadByte((BYTE)*r) == TRUE){
				r++;
			}
		}

		if(lstrcmpni(p, "meta", 4) == 0){
			//META�^�O�̏ꍇ
			TagContent = (char *)GlobalAlloc(GPTR, r - p + 2);
			if(TagContent == NULL){
				return NULL;
			}
			TagType = (char *)GlobalAlloc(GPTR, r - p + 2);
			if(TagType == NULL){
				GlobalFree(TagContent);
				return NULL;
			}
			content = (char *)GlobalAlloc(GPTR, r - p + 2);
			if(content == NULL){
				GlobalFree(TagContent);
				GlobalFree(TagType);
				return NULL;
			}

			//�^�O�̒��g���R�s�[
			t = TagContent;
			while(r > p){
				*(t++) = *(p++);
			}
			*t = '\0';

			//META�^�O�̓��e��WWWC��META���ǂ����`�F�b�N����
			if(GetTagType(TagContent + 4, type, TagType) == TRUE && lstrcmpi(TagType, name) == 0 &&
				GetTagType(TagContent + 4, contentname, content) == TRUE){
				GlobalFree(TagContent);
				GlobalFree(TagType);
				return content;
			}
			GlobalFree(TagContent);
			GlobalFree(TagType);
			GlobalFree(content);
		}
		p = (*r != '\0') ? r + 1 : r;
	}
	return NULL;
}


/******************************************************************************

	GetTagContentSize

	�w��̃^�O�ɋ��܂ꂽ������̃T�C�Y

******************************************************************************/

int GetTagContentSize(char *buf, char *tag)
{
	char *p;
	int ret = 0;
	BOOL ContentFlag = FALSE;

	p = buf;
	while(*p != '\0'){
		if(*p == '<'){
			p++;
			for(; *p == ' '; p++);
			ContentFlag = (lstrcmpni(p, tag, lstrlen(tag)) == 0) ? TRUE : FALSE;

			//�^�O�̏I���̏ꍇ�͏����𔲂���
			if(*p == '/' && lstrcmpni(p + 1, tag, lstrlen(tag)) == 0){
				return ret;
			}

			for(; *p != '>' && *p != '\0'; p++){
				if(IsDBCSLeadByte((BYTE)*p) == TRUE){
					p++;
				}
			}
			if(*p != '\0'){
				p++;
			}
			continue;
		}

		//�w��̃^�O�łȂ��ꍇ�̓X�L�b�v
		if(ContentFlag == FALSE){
			if(IsDBCSLeadByte((BYTE)*p) == TRUE){
				p++;
			}
			p++;
			continue;
		}

		//�w��̃^�O�̏ꍇ�̓R�s�[���s��
		switch(*p)
		{
		case '\r': case '\n':
			p++;
			break;

		case '\t':
			ret++;
			p++;
			break;

		default:
			if(IsDBCSLeadByte((BYTE)*p) == TRUE){
				ret++;
				p++;
			}
			ret++;
			p++;
			break;
		}
	}
	return 0;
}


/******************************************************************************

	GetTagContent

	�w��̃^�O�ɋ��܂ꂽ��������擾

******************************************************************************/

BOOL GetTagContent(char *buf, char *tag, char *content)
{
	char *p, *r;
	BOOL ContentFlag = FALSE;

	p = buf;
	r = content;
	while(*p != '\0'){
		if(*p == '<'){
			p++;
			for(; *p == ' '; p++);
			ContentFlag = (lstrcmpni(p, tag, lstrlen(tag)) == 0) ? TRUE : FALSE;

			//�^�O�̏I���̏ꍇ�͏����𔲂���
			if(*p == '/' && lstrcmpni(p + 1, tag, lstrlen(tag)) == 0){
				*r = '\0';
				return ((*content == '\0') ? FALSE : TRUE);
			}

			for(; *p != '>' && *p != '\0'; p++){
				if(IsDBCSLeadByte((BYTE)*p) == TRUE){
					p++;
				}
			}
			if(*p != '\0'){
				p++;
			}
			continue;
		}

		//�w��̃^�O�łȂ��ꍇ�̓X�L�b�v
		if(ContentFlag == FALSE){
			if(IsDBCSLeadByte((BYTE)*p) == TRUE){
				p++;
			}
			p++;
			continue;
		}

		//�w��̃^�O�̏ꍇ�̓R�s�[���s��
		switch(*p)
		{
		case '\r': case '\n':
			p++;
			break;

		case '\t':
			*(r++) = ' ';
			p++;
			break;

		default:
			if(IsDBCSLeadByte((BYTE)*p) == TRUE){
				*(r++) = *(p++);
			}
			*(r++) = *(p++);
			break;
		}
	}
	*r = '\0';
	return FALSE;
}


/******************************************************************************

	DelTagSize

	HTML�^�O�����������T�C�Y���擾

******************************************************************************/

int DelTagSize(char *buf)
{
	char *p;
	int retlen = 0;
	BOOL CommentFlag = 0;
	BOOL Tag = FALSE;

	p = buf;
	while(*p != '\0'){
		switch(*p)
		{
		//�^�O�̊J�n
		case '<':
			if(lstrcmpn(p,"<!--",4) == 0){
				//�R�����g�J�n
				CommentFlag = TRUE;
				break;
			}
			if(CommentFlag == FALSE){
				Tag = TRUE;
			}
			break;

		//�^�O�̏I��
		case '>':
			if(CommentFlag == TRUE){
				if(lstrcmpn(p - 2,"-->",3) != 0){
					break;
				}
				//�R�����g�I��
				CommentFlag = FALSE;
				break;
			}
			Tag = FALSE;
			break;

		//���s�Ƌ󔒂𖳎�
		case '\r':
		case '\n':
		case '\t':
		case ' ':
			break;

		default:
			if(Tag == FALSE && CommentFlag == FALSE){
				//�^�O���ł͖����ꍇ
				if(IsDBCSLeadByte((BYTE)*p) == TRUE){
					p++;
					retlen++;
				}
				retlen++;
				break;
			}
			//�^�O���̏ꍇ
			if(IsDBCSLeadByte((BYTE)*p) == TRUE){
				p++;
			}
			break;
		}
		p++;
	}
	return retlen;
}


/******************************************************************************

	ConvHTMLChar

	HTML�̓��ꕶ���̕ϊ�

******************************************************************************/

void ConvHTMLChar(char *buf)
{
	char *r, *t, *s;

	r = t = buf;

	while(*r != '\0'){
		if(IsDBCSLeadByte((BYTE)*r) == TRUE){
			*(t++) = *(r++);
			*(t++) = *(r++);
			continue;
		}
		switch(*r)
		{
		case '\r':
		case '\n':
		case '\t':
			*(t++) = ' ';
			r++;
			break;

		case '&':
			r++;
			if(lstrcmpni(r, "amp;", lstrlen("amp;")) == 0){
				r += lstrlen("amp;");
				*(t++) = '&';
			}else if(lstrcmpni(r, "quot;", lstrlen("quot;")) == 0){
				r += lstrlen("quot;");
				*(t++) = '"';
			}else if(lstrcmpni(r, "lt;", lstrlen("lt;")) == 0){
				r += lstrlen("lt;");
				*(t++) = '<';
			}else if(lstrcmpni(r, "gt;", lstrlen("gt;")) == 0){
				r += lstrlen("gt;");
				*(t++) = '>';
			}else if(lstrcmpni(r, "nbsp;", lstrlen("nbsp;")) == 0){
				r += lstrlen("nbsp;");
				*(t++) = ' ';
			}else if(lstrcmpni(r, "reg;", lstrlen("reg;")) == 0){
				r += lstrlen("reg;");
				*(t++) = 'R';
			}else if(lstrcmpni(r, "copy;", lstrlen("copy;")) == 0){
				r += lstrlen("copy;");
				*(t++) = 'c';
			}else if(*r == '#'){
				for(s = r + 1; *s >= '0' && *s <= '9'; s++);
				if(*s != ';'){
					r--;
					*(t++) = *(r++);
				}else{
					*(t++) = atoi(r + 1);
					r = s + 1;
				}
			}else{
				r--;
				*(t++) = *(r++);
			}
			break;

		default:
			*(t++) = *(r++);
		}
	}
	*t = '\0';
}


/******************************************************************************

	DateAdd

	���t�̌v�Z���s��

******************************************************************************/

void DateAdd(SYSTEMTIME *sTime, int time)
{
	struct tm ltime;

	memset(&ltime, 0, sizeof(struct tm));

	ltime.tm_year = sTime->wYear - 1900;
	ltime.tm_mon = sTime->wMonth - 1;
	ltime.tm_mday = sTime->wDay;
	ltime.tm_hour = sTime->wHour + time;
	ltime.tm_min = sTime->wMinute;
	ltime.tm_sec = sTime->wSecond;

	mktime(&ltime);

	sTime->wYear = ltime.tm_year + 1900;
	sTime->wMonth = ltime.tm_mon + 1;
	sTime->wDay = ltime.tm_mday;
	sTime->wHour = ltime.tm_hour;
	sTime->wMinute = ltime.tm_min;
	sTime->wSecond = ltime.tm_sec;
}


/******************************************************************************

	FormatDateConv

	�t�H�[�}�b�g�ɂ��������ē��t�̓W�J���s��

******************************************************************************/

static int FormatDateConv(char *format, char *buf, SYSTEMTIME *gTime)
{
#define IS_ALPHA(c)		((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
#define IS_NUM(c)		(c >= '0' && c <= '9')
#define IS_ALNUM(c)		(IS_NUM(c) || IS_ALPHA(c) || c == '+' || c == '-')
	int i;
	char *p, *r, *t;
	char tmp[BUFSIZE];
	char tz[BUFSIZE];
	char month[12][4] = {
		{"Jan"}, {"Feb"}, {"Mar"}, {"Apr"},
		{"May"}, {"Jun"}, {"Jul"}, {"Aug"},
		{"Sep"}, {"Oct"}, {"Nov"}, {"Dec"},
	};

	memset(gTime, 0, sizeof(SYSTEMTIME));

	*tz = '\0';
	p = format;
	r = buf;
	while(*p != '\0'){
		switch(*p)
		{
		case 'w':	//�j��
			for(; IS_ALPHA(*r); r++);
			break;

		case 'd':	//��
			for(t = tmp; IS_NUM(*r); r++){
				*(t++) = *r;
			}
			*t = '\0';
			gTime->wDay = atoi(tmp);
			if(gTime->wDay == 0){
				return -1;
			}
			break;

		case 'm':	//��
			for(t = tmp; IS_ALPHA(*r); r++){
				*(t++) = *r;
			}
			*t = '\0';
			for(i = 0; i < 12; i++){
				if(lstrcmpni(*(month + i), tmp, lstrlen(tmp) + 1) == 0){
					break;
				}
			}
			if(i >= 12){
				return -1;
			}
			gTime->wMonth = i + 1;
			break;

		case 'y':	//�N
			for(t = tmp; IS_NUM(*r); r++){
				*(t++) = *r;
			}
			*t = '\0';
			gTime->wYear = atoi(tmp);
			if(gTime->wYear < 1900){
				if(gTime->wYear > 70){
					gTime->wYear += 1900;
				}else{
					gTime->wYear += 2000;
				}
			}
			break;

		case 'h':	//��
			for(t = tmp; IS_NUM(*r); r++){
				*(t++) = *r;
			}
			*t = '\0';
			gTime->wHour = atoi(tmp);
			break;

		case 'n':	//��
			for(t = tmp; IS_NUM(*r); r++){
				*(t++) = *r;
			}
			*t = '\0';
			gTime->wMinute = atoi(tmp);
			break;

		case 's':	//�b
			for(t = tmp; IS_NUM(*r); r++){
				*(t++) = *r;
			}
			*t = '\0';
			gTime->wSecond = atoi(tmp);
			break;

		case 't':	//TZ
			for(t = tz; IS_ALNUM(*r); r++){
				*(t++) = *r;
			}
			*t = '\0';
			break;

		case ' ':	//Space
			for(; *r == ' '; r++);
			break;

		default:
			if(*p == *r){
				r++;
			}else{
				return -1;
			}
			break;
		}
		p++;
	}

	if(lstrcmpi(tz, "GMT") == 0){
		DateAdd(gTime, TimeZone);
	}
	return 0;
}


/******************************************************************************

	DateConv

	���t�̕ϊ����s��

******************************************************************************/

int DateConv(char *buf, char *ret)
{
	SYSTEMTIME gTime;
	char *p;
	char fDay[BUFSIZE];
	char fTime[BUFSIZE];
	int i;

	if(lstrlen(buf) > BUFSIZE - 1){
		return -1;
	}

	//RFC 1123
	i = FormatDateConv("w, d m y h:n:s t", buf, &gTime);

	if(i == -1){
		//RFC 1036
		i = FormatDateConv("w, d-m-y h:n:s t", buf, &gTime);
	}

	if(i == -1){
		//ANSI C's asctime() format
		i = FormatDateConv("w m d h:n:s y", buf, &gTime);
		if(i == 0){
			DateAdd(&gTime, TimeZone);
		}
	}
	if(i == -1){
		return -1;
	}

	if(*DateFormat == '\0'){
		p = NULL;
	}else{
		p = DateFormat;
	}
	if(GetDateFormat(0, 0, &gTime, p, fDay, BUFSIZE - 1) == 0){
		return -1;
	}

	if(*TimeFormat == '\0'){
		p = NULL;
	}else{
		p = TimeFormat;
	}
	if(GetTimeFormat(0, 0, &gTime, p, fTime, BUFSIZE - 1) == 0){
		return -1;
	}

	wsprintf(ret, "%s %s", fDay, fTime);
	return 0;
}


/******************************************************************************

	CreateDateTime

	���t�A���Ԃ��w��̌`���ŕԂ�

******************************************************************************/

int CreateDateTime(char *ret)
{
	char fDay[BUFSIZE];
	char fTime[BUFSIZE];
	char *p;

	if(*DateFormat == '\0'){
		p = NULL;
	}else{
		p = DateFormat;
	}
	if(GetDateFormat(0, 0, NULL, p, fDay, BUFSIZE - 1) == 0){
		return -1;
	}

	if(*TimeFormat == '\0'){
		p = NULL;
	}else{
		p = TimeFormat;
	}
	if(GetTimeFormat(0, 0, NULL, p, fTime, BUFSIZE - 1) == 0){
		return -1;
	}

	wsprintf(ret, "%s %s", fDay, fTime);
	return 0;
}


/******************************************************************************

	eBase

	Base64�G���R�[�h

******************************************************************************/

void eBase(char *buf, char *ret)
{
	char tmp, tmp2;
	char *r;
	int c, i;
	const char Base[] =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
		"0123456789+/";

	i = 0;
	r = ret;
	while(*(buf + i) != '\0'){
		c = (*(buf + i) & 0xFC) >> 2;
		*(r++) = *(Base + c);
		i++;

		if(*(buf + i) == '\0'){
			*(buf + i) = 0;
			tmp2 = (char)(*(buf + i - 1) << 4) & 0x30;
			tmp = (char)(*(buf + i) >> 4) & 0xF;
			c = tmp2 | tmp;
			*(r++) = *(Base + c);
			*(r++) = '=';
			*(r++) = '=';
			break;
		}
		tmp2 = (char)(*(buf + i - 1) << 4) & 0x30;
		tmp = (char)(*(buf + i) >> 4) & 0xF;
		c = tmp2 | tmp;
		*(r++) = *(Base + c);

		if(*(buf + i + 1) == '\0'){
			*(buf + i + 1) = 0;
			tmp2 = (char)(*(buf + i) << 2) & 0x3C;
			tmp = (char)(*(buf + i + 1) >> 6) & 0x3;
			c = tmp2 | tmp;
			*(r++) = *(Base + c);
			*(r++) = '=';
			break;
		}

		tmp2 = (char)(*(buf + i) << 2) & 0x3C;
		tmp = (char)(*(buf + i + 1) >> 6) & 0x3;
		c = tmp2 | tmp;
		*(r++) = *(Base + c);
		i++;

		c = *(buf + i) & 0x3F;
		*(r++) = *(Base + c);
		i++;
	}
	*r = '\0';
}


/******************************************************************************

	SjisShift

	JIS��2�o�C�g��SJIS�ɕϊ�����

******************************************************************************/

void SjisShift(int *ph, int *pl)
{
	if(*ph & 1){
		if(*pl < 0x60){
			*pl+=0x1F;
		}else{
			*pl+=0x20;
		}
	}else{
		*pl+=0x7E;
	}
	if(*ph < 0x5F){
		*ph = (*ph + 0xE1) >> 1;
	}else{
		*ph = (*ph + 0x161) >> 1;
	}
}


/******************************************************************************

	JIS_SJIS

	JIS��SJIS�ɕϊ�����

******************************************************************************/

char *JIS_SJIS(char *buf, char *ret)
{
	unsigned int c, d, j;
	BOOL jiskanji, hankaku;
	char *p, *r;

	*ret = '\0';

	p = buf;
	r = ret;
	j = 0;
	jiskanji = FALSE;
	hankaku = FALSE;
	while(*p != '\0'){
		j++;
		c = *(p++);
		if(c == ESC){
			if((c = *(p++)) == '$'){
				if((c = *(p++)) == '@' || c == 'B'){
					jiskanji = TRUE;
				}else{
					*(r++) = ESC;
					*(r++) = '$';
					if(c != '\0'){
						*(r++) = c;
					}
				}
			}else if(c == '('){
				if((c = *(p++)) == 'H' || c == 'J' || c == 'B'){
					jiskanji = FALSE;
				}else{
					*(r++) = ESC;
					*(r++) = '(';
					if(c != '\0'){
						*(r++) = c;
					}
				}
			}else if(c == '*'){
				if((c = *(p++)) == 'B'){
					hankaku = FALSE;
				}else if(c == 'I'){
					hankaku = TRUE;
				}
			}else if(hankaku == TRUE && c == 'N'){
				c = *(p++);
				*(r++) = c + 0x80;
			}else if(c == 'K'){
				jiskanji = TRUE;
			}else if(c == 'H'){
				jiskanji = FALSE;
			}else{
				*(r++) = ESC;
				if(c != '\0'){
					*(r++) = c;
				}
			}
		}else if(jiskanji && c >= 0x21 && c <= 0x7E){
			if((d = *(p++)) >= 0x21 && d <= 0x7E){
				SjisShift((int *)&c, (int *)&d);
			}
			*(r++) = c;
			if(d != '\0'){
				*(r++) = d;
			}
		}else if(c >= 0xA1 && c <= 0xFE){
			if((d = *(p++)) >= 0xA1 && d <= 0xFE){
				d &= 0x7E;
				c &= 0x7E;
				SjisShift((int *)&c, (int *)&d);
			}
			*(r++) = c;
			if(d != '\0'){
				*(r++) = d;
			}
		}else if(c == 0x0E){
			while(*p != '\0' && *p != 0x0F){
				*(r++) = *(p++) + 0x80;
			}
			if(*p == 0x0F){
				p++;
			}
		}else{
			*(r++) = c;
		}
	}
	*r = '\0';
	return r;
}


/******************************************************************************

	EUC_SJIS

	EUC��SJIS�ɕϊ�����

******************************************************************************/

char *EUC_SJIS(char *buf, char *ret)
{
	char *p, *r;
	unsigned int c, d;

	p = buf;
	r = ret;
	while(*p != '\0'){
		c = *(p++) & 0xFF;
		d = *p & 0xFF;
		if ((c >= 0xa1 && c <= 0xfe) && (d >= 0xa1 && d <= 0xfe)){
			//EUC��JIS�ɕϊ�
			c &= ~0x80;
			d &= ~0x80;

			//JIS��SJIS�ɕϊ�
			SjisShift((int *)&c, (int *)&d);

			*(r++) = (char)c;
			*(r++) = (char)d;
			p++;
		}else{
			*(r++) = (char)c;
		}
	}
	*r = '\0';
	return r;
}


/******************************************************************************

	GetKanjiType

	�����R�[�h�̎�ނ��擾����

******************************************************************************/

int GetKanjiType(unsigned char *str)
{
    int val = 0;
    unsigned char b1, b2, b3;

    b1 = *str++;
    b2 = *str++;
    b3 = *str;
    if (b1 == 0x1b) {
        if (b2 == '$' && b3 == 'B') return 16;
        if (b2 == '$' && b3 == '@') return 32;
        return 0;
    }
    if ( b1 >= 0xa0 && b1 <= 0xdf) val |= 1;
    if ((b1 >= 0x81 && b1 <= 0x9f ||
         b1 >= 0xe0 && b1 <= 0xfc) &&
        (b2 >= 0x40 && b2 <= 0xfc && b2 != 0x7f)) val |= 2;
    if (b1 == 0x8e && (b2 >= 0xa0 && b2 <= 0xdf)) val |= 4;
    if ((b1 >= 0xa1 && b1 <= 0xfe) &&
        (b2 >= 0xa1 && b1 <= 0xfe)) val |= 8;

    return val;
}


/******************************************************************************

	ConvStrCode

	JIS��EUC�̊����R�[�h��SJIS�ɕϊ�����

******************************************************************************/

char *ConvStrCode(char *buf, char *ret)
{
	char *p;
	int rc;
	BOOL eucflag = FALSE;

	p = buf;
	while(*p != '\0'){
		rc = GetKanjiType(p);
		switch(rc)
		{
		case 0:		//ASCII
			p++;
			break;

		case 1:		//SJIS���p�J�i
		case 2:		//SJIS
			return iStrCpy(ret, buf);

		case 4:		//EUC���p�J�i
		case 8:		//EUC
			return EUC_SJIS(buf, ret);

		case 9:		//EUC��SJIS���p�J�i
			eucflag = TRUE;
			p += 2;
			break;

		case 16:	//�VJIS
		case 32:	//��JIS
			return JIS_SJIS(buf, ret);

		default:
			p++;
			if (*p != '\0') {
				p++;
			}
			break;
		}
	}
	if(eucflag == TRUE){
		return EUC_SJIS(buf, ret);
	}
	return iStrCpy(ret, buf);
}

/******************************************************************************

	convUtf8

	UTF-8�ɕϊ�

******************************************************************************/

static char *convUtf8(char *buf) {
	WCHAR *wbuf;
	int wlen;
	char *cbuf;
	int clen;

	wlen = MultiByteToWideChar(CP_UTF8, 0, buf, -1, NULL, 0);
	if (wlen <= 0) {
		return NULL;
	}
	wbuf = (WCHAR *)GlobalAlloc(GMEM_FIXED, (wlen + 1) * sizeof(WCHAR));
	MultiByteToWideChar(CP_UTF8, 0, buf, -1, wbuf, wlen);

	clen = WideCharToMultiByte(CP_UTF8, 0, wbuf, -1, NULL, 0, NULL, NULL);
	if (clen <= 0) {
		GlobalFree(wbuf);
		return NULL;
	}
	cbuf = (char *)GlobalAlloc(GMEM_FIXED, (clen + 1) * sizeof(char));
	WideCharToMultiByte(CP_UTF8, 0, wbuf, -1, cbuf, clen, NULL, NULL);
	if (lstrcmp(buf, cbuf) != 0) {
		GlobalFree(cbuf);
		GlobalFree(wbuf);
		return NULL;
	}
	GlobalFree(cbuf);

	clen = WideCharToMultiByte(CP_ACP, 0, wbuf, -1, NULL, 0, NULL, NULL);
	if (clen <= 0) {
		GlobalFree(wbuf);
		return NULL;
	}
	cbuf = (char *)GlobalAlloc(GMEM_FIXED, (clen + 1) * sizeof(char));
	WideCharToMultiByte(CP_ACP, 0, wbuf, -1, cbuf, clen, NULL, NULL);
	GlobalFree(wbuf);
	return cbuf;
}

/******************************************************************************

	SrcConv

	�\�[�X��SJIS�ɕϊ�����

******************************************************************************/

char *SrcConv(char *buf, long Size)
{
	char *tmp;

	tmp = convUtf8(buf);
	if (tmp != NULL) {
		return tmp;
	}
	tmp = (char *)GlobalAlloc(GMEM_FIXED, Size + 1);
	if(tmp == NULL){
		abort();
	}
	ConvStrCode(buf, tmp);
	return tmp;
}


/******************************************************************************

	DeleteSizeInfo

	HTTP1.1�̃T�C�Y��������

******************************************************************************/

long DeleteSizeInfo(char *buf, long size)
{
	char *p, *r;
	long len, i;

	if(lstrcmpni(buf, "HTTP/1.1", 8) != 0 ||
		GetHeadContentSize(buf, "Content-Length:") > 0){
		return size;
	}

	p = buf + HeaderSize(buf);
	r = p;

	while(*p != '\0'){
		for(; *p == '\r' || *p == '\n'; p++);
		len = x2d(p);
		if(len == 0) break;

		for(; *p != '\0' && *p != '\r' && *p != '\n'; p++);
		if(*p == '\0'){
			break;
		}
		if(*p == '\n'){
			p++;
		}else{
			p += 2;
		}

		for(i = 0; *p != '\0' && i < len; i++, p++, r++){
			*r = *p;
		}
		*r = '\0';
	}
	if(r != (buf + HeaderSize(buf))){
		return (r - buf);
	}
	return size;
}


/******************************************************************************

	GetHtml2RtfSize

	HTML��RTF�ɕϊ������Ƃ��̃T�C�Y�̎擾

******************************************************************************/

int GetHtml2RtfSize(char *buf)
{
	char *p, *t;
	BOOL TagFlag = FALSE;
	BOOL TagJoinFlag = FALSE;
	BOOL CommentFlag = FALSE;
	BOOL StringFlag = FALSE;
	BOOL cfFlag = FALSE;
	int len = 0;

	if(buf == NULL) return 0;

	len = lstrlen(RTF_HEAD);
	for(p = buf; *p != '\0'; p++){
		switch(*p)
		{
		case '\r':
		case '\n':
		case '\t':
			len += 4;
			cfFlag = TRUE;
			break;

		case '\\':
		case '{':
		case '}':
			if(cfFlag == TRUE){
				len++;
				cfFlag = FALSE;
			}
			len += 2;
			break;

		case '\"':
			if(TagFlag == TRUE && CommentFlag == FALSE){
				if(StringFlag == FALSE){
					StringFlag = TRUE;
					len += 6;
					cfFlag = FALSE;
				}else{
					StringFlag = FALSE;
					if(cfFlag == TRUE){
						len++;
						cfFlag = FALSE;
					}
					len += 5;
					cfFlag = TRUE;
				}
			}else{
				if(cfFlag == TRUE){
					len++;
					cfFlag = FALSE;
				}
				len++;
			}
			break;

		case '<':
			if(TagJoinFlag == TRUE){
				TagJoinFlag = FALSE;
				if(cfFlag == TRUE){
					len++;
					cfFlag = FALSE;
				}
				len++;
				break;
			}
			if(lstrcmpn(p, "<!--", 4) == 0){
				CommentFlag = TRUE;
				len += 6;
				cfFlag = FALSE;
			}else{
				if(CommentFlag == FALSE){
					TagFlag = TRUE;
					len += 6;
					cfFlag = FALSE;
				}else{
					if(cfFlag == TRUE){
						len++;
						cfFlag = FALSE;
					}
					len++;
				}
			}
			break;

		case '>':
			if(TagFlag == TRUE && CommentFlag == FALSE){
				for(t = p + 1; *t == ' '; t++);
				if(*t == '<' && lstrcmpn(t, "<!--", 4) != 0){
					len++;
					TagJoinFlag = TRUE;
					break;
				}
			}
			if(cfFlag == TRUE){
				len++;
				cfFlag = FALSE;
			}
			len++;
			if(CommentFlag == TRUE){
				if(lstrcmpn(p - 2, "-->", 3) != 0){
					break;
				}
				CommentFlag = FALSE;
			}else{
				TagFlag = FALSE;
			}
			len += 5;
			cfFlag = TRUE;
			break;

		default:
			if(IsDBCSLeadByte((BYTE)*p) == TRUE){
				len += 8;
			}else{
				if(cfFlag == TRUE){
					len++;
				}
				len++;
			}
			cfFlag = FALSE;
			break;
		}
	}
	len++;
	return len;
}


/******************************************************************************

	Html2Rtf

	HTML��RTF�ɕϊ�

******************************************************************************/

void Html2Rtf(char *buf, char *ret)
{
	char *p, *r, *t;
	BOOL TagFlag = FALSE;
	BOOL TagJoinFlag = FALSE;
	BOOL CommentFlag = FALSE;
	BOOL StringFlag = FALSE;
	BOOL cfFlag = FALSE;

	if(buf == NULL || ret == NULL) return;

	r = ret;
	r = iStrCpy(r, RTF_HEAD);
	for(p = buf; *p != '\0'; p++){
		switch(*p)
		{
		case '\r':
			if(*(p + 1) != '\n'){
				r = iStrCpy(r, "\\par");
				cfFlag = TRUE;
			}
			break;

		case '\n':
			r = iStrCpy(r, "\\par");
			cfFlag = TRUE;
			break;

		case '\t':
			r = iStrCpy(r, "\\tab");
			cfFlag = TRUE;
			break;

		case '\\':
		case '{':
		case '}':
			if(cfFlag == TRUE){
				*(r++) = ' ';
				cfFlag = FALSE;
			}
			*(r++) = '\\';
			*(r++) = *p;
			break;

		case '\"':
			if(TagFlag == TRUE && CommentFlag == FALSE){
				if(StringFlag == FALSE){
					StringFlag = TRUE;
					r = iStrCpy(r, "\\cf2 \"");
					cfFlag = FALSE;
				}else{
					StringFlag = FALSE;
					if(cfFlag == TRUE){
						*(r++) = ' ';
						cfFlag = FALSE;
					}
					r = iStrCpy(r, "\"\\cf1");
					cfFlag = TRUE;
				}
			}else{
				if(cfFlag == TRUE){
					*(r++) = ' ';
					cfFlag = FALSE;
				}
				*(r++) = *p;
			}
			break;

		case '<':
			if(TagJoinFlag == TRUE){
				TagJoinFlag = FALSE;
				if(cfFlag == TRUE){
					*(r++) = ' ';
					cfFlag = FALSE;
				}
				*(r++) = *p;
				break;
			}
			if(lstrcmpn(p, "<!--", 4) == 0){
				CommentFlag = TRUE;
				r = iStrCpy(r, "\\cf4 <");
				cfFlag = FALSE;
			}else{
				if(CommentFlag == FALSE){
					TagFlag = TRUE;
					r = iStrCpy(r, "\\cf1 <");
					cfFlag = FALSE;
				}else{
					if(cfFlag == TRUE){
						*(r++) = ' ';
						cfFlag = FALSE;
					}
					*(r++) = *p;
				}
			}
			break;

		case '>':
			if(TagFlag == TRUE && CommentFlag == FALSE){
				for(t = p + 1; *t == ' '; t++);
				if(*t == '<' && lstrcmpn(t, "<!--", 4) != 0){
					if(cfFlag == TRUE){
						*(r++) = ' ';
						cfFlag = FALSE;
					}
					*(r++) = *p;
					TagJoinFlag = TRUE;
					break;
				}
			}
			if(cfFlag == TRUE){
				*(r++) = ' ';
				cfFlag = FALSE;
			}
			*(r++) = *p;
			if(CommentFlag == TRUE){
				if(lstrcmpn(p - 2, "-->", 3) != 0){
					break;
				}
				CommentFlag = FALSE;
			}else{
				TagFlag = FALSE;
			}
			r = iStrCpy(r, "\\cf3");
			cfFlag = TRUE;
			break;

		default:
			if(IsDBCSLeadByte((BYTE)*p) == TRUE){
				wsprintf(r, "\\'%2x", (unsigned char)*(p++));
				r += 4;
				wsprintf(r, "\\'%2x", (unsigned char)*p);
				r += 4;
			}else{
				if(cfFlag == TRUE){
					*(r++) = ' ';
				}
				*(r++) = *p;
			}
			cfFlag = FALSE;
			break;
		}
	}
	*(r++) = '}';
	*r = '\0';
}
/* End of source */
