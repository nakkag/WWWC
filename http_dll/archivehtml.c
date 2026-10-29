/**************************************************************************

	WWWC (wwwc.dll)

	archivehtml.c

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
#include <shlwapi.h>

#include "archive.h"

#pragma comment(lib, "shlwapi.lib")


/**************************************************************************
	Define
**************************************************************************/

#define BUF_LIMIT			(256 * 1024 * 1024)	/* 書き換え結果の最大 */
#define MAX_ATTR			64

#define IS_SPACE(c)			((c) == ' ' || (c) == '\t' || (c) == '\r' || (c) == '\n' || (c) == '\f')
#define IS_ALPHA(c)			(((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z'))
#define IS_DIGIT(c)			((c) >= '0' && (c) <= '9')
#define TO_LOWER(c)			(((c) >= 'A' && (c) <= 'Z') ? ((c) - 'A' + 'a') : (c))

#define A_NONE				0
#define A_URL				1
#define A_RENAME			2
#define A_STYLE				3
#define A_DELETE			4

typedef struct _ATTR {
	const char *name;
	long nlen;
	const char *val;					/* 引用符を除いた値 (無い場合は NULL) */
	long vlen;
	const char *start;					/* 属性の先頭 */
	const char *end;					/* 属性の終わりの次 */
} ATTR;


/**************************************************************************
	Buffer
**************************************************************************/

static BOOL Buf_Reserve(ARCHIVE_BUF *b, long add)
{
	char *p;
	long cap;

	if(b->err == TRUE){
		return FALSE;
	}
	if(add < 0 || b->len + add + 1 > BUF_LIMIT){
		b->err = TRUE;
		return FALSE;
	}
	if(b->len + add + 1 <= b->cap){
		return TRUE;
	}
	cap = (b->cap == 0) ? 4096 : b->cap;
	while(cap < b->len + add + 1){
		cap = (cap > BUF_LIMIT / 2) ? BUF_LIMIT : cap * 2;
	}
	p = (b->p == NULL) ? (char *)HeapAlloc(GetProcessHeap(), 0, cap) : (char *)HeapReAlloc(GetProcessHeap(), 0, b->p, cap);
	if(p == NULL){
		b->err = TRUE;
		return FALSE;
	}
	b->p = p;
	b->cap = cap;
	return TRUE;
}

void Buf_Add(ARCHIVE_BUF *b, const char *s, long n)
{
	if(n <= 0 || Buf_Reserve(b, n) == FALSE){
		return;
	}
	CopyMemory(b->p + b->len, s, n);
	b->len += n;
	b->p[b->len] = '\0';
}

void Buf_Str(ARCHIVE_BUF *b, const char *s)
{
	Buf_Add(b, s, lstrlen(s));
}

void Buf_Free(ARCHIVE_BUF *b)
{
	if(b->p != NULL){
		HeapFree(GetProcessHeap(), 0, b->p);
	}
	ZeroMemory(b, sizeof(ARCHIVE_BUF));
}

/* 結果を取り出す (エラーの場合は NULL) */
static char *Buf_Detach(ARCHIVE_BUF *b, long *outlen)
{
	char *p;

	if(b->err == TRUE || Buf_Reserve(b, 0) == FALSE){
		Buf_Free(b);
		return NULL;
	}
	p = b->p;
	p[b->len] = '\0';
	if(outlen != NULL){
		*outlen = b->len;
	}
	b->p = NULL;
	b->len = b->cap = 0;
	return p;
}

void Archive_Free(void *p)
{
	if(p != NULL){
		HeapFree(GetProcessHeap(), 0, p);
	}
}


/**************************************************************************
	String helpers
**************************************************************************/

/* 大文字小文字を区別せずに先頭が一致するか (範囲内) */
static BOOL StartsWithI(const char *p, const char *end, const char *s)
{
	for(; *s != '\0'; p++, s++){
		if(p >= end || TO_LOWER((unsigned char)*p) != TO_LOWER((unsigned char)*s)){
			return FALSE;
		}
	}
	return TRUE;
}

static BOOL EqualI(const char *p, long n, const char *s)
{
	return (lstrlen(s) == n && StartsWithI(p, p + n, s) == TRUE) ? TRUE : FALSE;
}

/* 範囲内で大文字小文字を区別せずに検索 */
static const char *FindI(const char *p, const char *end, const char *s)
{
	for(; p < end; p++){
		if(StartsWithI(p, end, s) == TRUE){
			return p;
		}
	}
	return NULL;
}

/* 空白区切りのトークンに含まれるか (rel 属性用) */
static BOOL HasToken(const char *p, long n, const char *token)
{
	const char *end = p + n;
	const char *s;

	while(p < end){
		for(; p < end && IS_SPACE(*p); p++);
		for(s = p; p < end && !IS_SPACE(*p); p++);
		if(EqualI(s, (long)(p - s), token) == TRUE){
			return TRUE;
		}
	}
	return FALSE;
}

/* HTMLの文字参照を展開する (ASCII の範囲のみ) */
static void DecodeEntities(const char *s, long n, char *ret, long size)
{
	const char *end = s + n;
	const char *t;
	char *r = ret;
	long v;
	int i;
	static const struct { const char *name; char c; } ent[] = {
		{"amp;", '&'}, {"lt;", '<'}, {"gt;", '>'}, {"quot;", '"'}, {"apos;", '\''},
	};

	while(s < end && r < ret + size - 1){
		if(*s != '&'){
			*(r++) = *(s++);
			continue;
		}
		for(i = 0; i < sizeof(ent) / sizeof(ent[0]); i++){
			if(StartsWithI(s + 1, end, ent[i].name) == TRUE){
				*(r++) = ent[i].c;
				s += 1 + lstrlen(ent[i].name);
				break;
			}
		}
		if(i < sizeof(ent) / sizeof(ent[0])){
			continue;
		}
		if(s + 2 < end && *(s + 1) == '#'){
			v = 0;
			t = s + 2;
			if(*t == 'x' || *t == 'X'){
				for(t++; t < end && t < s + 10 && ((*t >= '0' && *t <= '9') || (*t >= 'a' && *t <= 'f') || (*t >= 'A' && *t <= 'F')); t++){
					v = v * 16 + (IS_DIGIT(*t) ? *t - '0' : TO_LOWER(*t) - 'a' + 10);
				}
			}else{
				for(; t < end && t < s + 10 && IS_DIGIT(*t); t++){
					v = v * 10 + (*t - '0');
				}
			}
			if(v >= 0x20 && v < 0x7F && t > s + 2){
				*(r++) = (char)v;
				s = (t < end && *t == ';') ? t + 1 : t;
				continue;
			}
		}
		*(r++) = *(s++);
	}
	*r = '\0';
}

/* 属性値として出力 */
static void AddAttrValue(ARCHIVE_BUF *b, const char *s)
{
	for(; *s != '\0'; s++){
		switch(*s)
		{
		case '&': Buf_Str(b, "&amp;"); break;
		case '"': Buf_Str(b, "&quot;"); break;
		case '<': Buf_Str(b, "&lt;"); break;
		case '>': Buf_Str(b, "&gt;"); break;
		default: Buf_Add(b, s, 1); break;
		}
	}
}

/* http, https の URL か */
static BOOL IsHttpUrl(const char *url)
{
	return (StartsWithI(url, url + lstrlen(url), "http://") == TRUE ||
		StartsWithI(url, url + lstrlen(url), "https://") == TRUE) ? TRUE : FALSE;
}


/******************************************************************************

	Archive_ResolveUrl

	基準のURLから相対URLを解決する (http, https 以外は FALSE)

******************************************************************************/

BOOL Archive_ResolveUrl(const char *base, const char *rel, char *ret, int size)
{
	char *tmp, *t;
	const char *p, *end;
	DWORD len;
	BOOL rc = FALSE;

	if(base == NULL || rel == NULL || size <= 0){
		return FALSE;
	}
	*ret = '\0';
	//前後の空白を除く
	for(p = rel; IS_SPACE(*p); p++);
	for(end = p + lstrlen(p); end > p && IS_SPACE(*(end - 1)); end--);
	if(p >= end || *p == '#'){
		return FALSE;
	}
	//スキームがある場合は http, https のみ
	for(t = (char *)p; t < end && (IS_ALPHA(*t) || IS_DIGIT(*t) || *t == '+' || *t == '-' || *t == '.'); t++);
	if(t > p && t < end && *t == ':' && StartsWithI(p, end, "http:") == FALSE && StartsWithI(p, end, "https:") == FALSE){
		return FALSE;
	}
	if(end - p > ARCHIVE_URL_MAX){
		return FALSE;
	}
	//改行等を除き、空白と8ビット文字は % エンコードする
	tmp = (char *)HeapAlloc(GetProcessHeap(), 0, (end - p) * 3 + 1);
	if(tmp == NULL){
		return FALSE;
	}
	for(t = tmp; p < end; p++){
		if(*p == '\r' || *p == '\n' || *p == '\t'){
			continue;
		}
		if((unsigned char)*p >= 0x80 || *p == ' ' || *p == '"' || *p == '<' || *p == '>' || (unsigned char)*p < 0x20){
			wsprintf(t, "%%%02X", (unsigned char)*p);
			t += 3;
		}else{
			*(t++) = *p;
		}
	}
	*t = '\0';
	len = size;
	if(UrlCombineA(base, tmp, ret, &len, 0) == S_OK && IsHttpUrl(ret) == TRUE){
		rc = TRUE;
	}else{
		*ret = '\0';
	}
	HeapFree(GetProcessHeap(), 0, tmp);
	return rc;
}


/**************************************************************************
	CSS
**************************************************************************/

/* url() または @import の値を出力 */
static void CssEmitUrl(ARCHIVE_BUF *b, const char *base, const char *prefix, const char *val, long vlen,
					   int type, BOOL isfunc, ARCHIVE_RESOLVE cb, void *ctx, const char *orig, long olen)
{
	char *raw, *abs;
	const char *local = NULL;
	const char *p;
	long i;

	raw = (char *)HeapAlloc(GetProcessHeap(), 0, vlen + 1);
	abs = (char *)HeapAlloc(GetProcessHeap(), 0, ARCHIVE_URL_MAX + 1);
	if(raw == NULL || abs == NULL){
		if(raw != NULL) HeapFree(GetProcessHeap(), 0, raw);
		if(abs != NULL) HeapFree(GetProcessHeap(), 0, abs);
		Buf_Add(b, orig, olen);
		return;
	}
	CopyMemory(raw, val, vlen);
	raw[vlen] = '\0';
	//エスケープを含むものや data: などはそのまま
	for(i = 0; i < vlen && raw[i] != '\\'; i++);
	if(i < vlen || Archive_ResolveUrl(base, raw, abs, ARCHIVE_URL_MAX + 1) == FALSE){
		Buf_Add(b, orig, olen);
		HeapFree(GetProcessHeap(), 0, raw);
		HeapFree(GetProcessHeap(), 0, abs);
		return;
	}
	if(cb != NULL && type != RES_LINK){
		local = cb(ctx, abs, type);
	}
	Buf_Str(b, (isfunc == TRUE) ? "url(\"" : "\"");
	if(local != NULL){
		Buf_Str(b, prefix);
		Buf_Str(b, local);
	}else{
		for(p = abs; *p != '\0'; p++){
			if(*p == '"' || *p == '\\' || *p == ')' || *p == '(' || *p == '\''){
				char hex[4];
				wsprintf(hex, "%%%02X", (unsigned char)*p);
				Buf_Str(b, hex);
			}else{
				Buf_Add(b, p, 1);
			}
		}
	}
	Buf_Str(b, (isfunc == TRUE) ? "\")" : "\"");
	HeapFree(GetProcessHeap(), 0, raw);
	HeapFree(GetProcessHeap(), 0, abs);
}


/******************************************************************************

	Archive_RewriteCss

	CSS内のURLをアーカイブのファイル名 (prefix + 名前) または絶対URLに書き換える

******************************************************************************/

char *Archive_RewriteCss(const char *css, long len, const char *base, const char *prefix,
					   ARCHIVE_RESOLVE cb, void *ctx, long *outlen)
{
	ARCHIVE_BUF b;
	const char *p = css, *end = css + len;
	const char *s, *q, *v, *ve;
	BOOL import = FALSE;
	BOOL fontpending = FALSE;
	int fontdepth = -1;
	int depth = 0;
	char quote;

	ZeroMemory(&b, sizeof(ARCHIVE_BUF));
	Buf_Reserve(&b, len + len / 4);
	while(p < end && b.err == FALSE){
		//コメント
		if(*p == '/' && p + 1 < end && *(p + 1) == '*'){
			q = p + 2;
			for(; q + 1 < end && !(*q == '*' && *(q + 1) == '/'); q++);
			q = (q + 1 < end) ? q + 2 : end;
			Buf_Add(&b, p, (long)(q - p));
			p = q;
			continue;
		}
		//文字列
		if(*p == '"' || *p == '\''){
			quote = *p;
			for(q = p + 1; q < end && *q != quote && *q != '\n'; q++){
				if(*q == '\\' && q + 1 < end){
					q++;
				}
			}
			if(q >= end || *q != quote){
				Buf_Add(&b, p, (long)(((q < end) ? q : end) - p));
				p = (q < end) ? q : end;
				continue;
			}
			if(import == TRUE){
				CssEmitUrl(&b, base, prefix, p + 1, (long)(q - p - 1), RES_CSS, FALSE, cb, ctx, p, (long)(q + 1 - p));
				import = FALSE;
			}else{
				Buf_Add(&b, p, (long)(q + 1 - p));
			}
			p = q + 1;
			continue;
		}
		//url()
		if((*p == 'u' || *p == 'U') && StartsWithI(p, end, "url(") == TRUE &&
			(p == css || !(IS_ALPHA(*(p - 1)) || IS_DIGIT(*(p - 1)) || *(p - 1) == '-' || *(p - 1) == '_'))){
			s = p + 4;
			for(; s < end && IS_SPACE(*s); s++);
			if(s < end && (*s == '"' || *s == '\'')){
				quote = *s;
				v = s + 1;
				for(ve = v; ve < end && *ve != quote && *ve != '\n'; ve++){
					if(*ve == '\\' && ve + 1 < end){
						ve++;
					}
				}
				q = (ve < end && *ve == quote) ? ve + 1 : NULL;
			}else{
				v = s;
				for(ve = v; ve < end && *ve != ')' && !IS_SPACE(*ve) && *ve != '"' && *ve != '\'' && *ve != '('; ve++);
				q = ve;
			}
			if(q != NULL){
				for(; q < end && IS_SPACE(*q); q++);
			}
			if(q == NULL || q >= end || *q != ')'){
				Buf_Add(&b, p, 4);
				p += 4;
				continue;
			}
			CssEmitUrl(&b, base, prefix, v, (long)(ve - v),
				(import == TRUE) ? RES_CSS : ((fontdepth >= 0) ? RES_LINK : RES_IMAGE), TRUE, cb, ctx, p, (long)(q + 1 - p));
			import = FALSE;
			p = q + 1;
			continue;
		}
		if(*p == '@'){
			if(StartsWithI(p, end, "@import") == TRUE){
				import = TRUE;
				Buf_Add(&b, p, 7);
				p += 7;
				continue;
			}
			if(StartsWithI(p, end, "@font-face") == TRUE){
				fontpending = TRUE;
				Buf_Add(&b, p, 10);
				p += 10;
				continue;
			}
		}
		switch(*p)
		{
		case '{':
			if(fontpending == TRUE){
				fontdepth = depth;
				fontpending = FALSE;
			}
			depth++;
			break;
		case '}':
			if(depth > 0) depth--;
			if(fontdepth >= 0 && depth <= fontdepth){
				fontdepth = -1;
			}
			break;
		case ';':
			import = FALSE;
			break;
		}
		Buf_Add(&b, p, 1);
		p++;
	}
	return Buf_Detach(&b, outlen);
}


/**************************************************************************
	HTML
**************************************************************************/

/* タグ名の文字 */
static BOOL IsTagNameChar(char c)
{
	return (IS_ALPHA(c) || IS_DIGIT(c) || c == '-' || c == '_' || c == ':' || c == '.') ? TRUE : FALSE;
}

/* タグの解析 (成功した場合はタグの終わりの次を返す) */
static const char *ParseTag(const char *p, const char *end, const char **name, long *nlen, ATTR *attr, int *cnt, BOOL *over)
{
	const char *q = p + 1;
	ATTR a;
	char quote;

	*cnt = 0;
	if(over != NULL){
		*over = FALSE;
	}
	*name = q;
	for(; q < end && IsTagNameChar(*q); q++);
	*nlen = (long)(q - *name);
	while(q < end){
		for(; q < end && (IS_SPACE(*q) || *q == '/'); q++);
		if(q >= end){
			return NULL;
		}
		if(*q == '>'){
			return q + 1;
		}
		ZeroMemory(&a, sizeof(ATTR));
		a.start = a.name = q;
		for(; q < end && !IS_SPACE(*q) && *q != '=' && *q != '>' && *q != '/'; q++);
		if(q == a.name){
			q++;
			continue;
		}
		a.nlen = (long)(q - a.name);
		for(; q < end && IS_SPACE(*q); q++);
		if(q < end && *q == '='){
			for(q++; q < end && IS_SPACE(*q); q++);
			if(q >= end){
				return NULL;
			}
			if(*q == '"' || *q == '\''){
				quote = *q;
				a.val = ++q;
				for(; q < end && *q != quote; q++);
				if(q >= end){
					return NULL;
				}
				a.vlen = (long)(q - a.val);
				q++;
			}else{
				a.val = q;
				for(; q < end && !IS_SPACE(*q) && *q != '>'; q++);
				a.vlen = (long)(q - a.val);
			}
		}
		a.end = q;
		if(*cnt < MAX_ATTR){
			attr[(*cnt)++] = a;
		}else if(over != NULL){
			*over = TRUE;
		}
	}
	return NULL;
}

/* 属性の値 (文字参照を展開) */
static char *AttrValue(ATTR *a)
{
	char *ret;

	if(a->val == NULL){
		return NULL;
	}
	ret = (char *)HeapAlloc(GetProcessHeap(), 0, a->vlen + 1);
	if(ret != NULL){
		DecodeEntities(a->val, a->vlen, ret, a->vlen + 1);
	}
	return ret;
}

static ATTR *FindAttr(ATTR *attr, int cnt, const char *name)
{
	int i;

	for(i = 0; i < cnt; i++){
		if(EqualI(attr[i].name, attr[i].nlen, name) == TRUE){
			return &attr[i];
		}
	}
	return NULL;
}

/* 属性の処理方法 */
static int AttrAction(const char *tag, long tlen, ATTR *attr, int cnt, ATTR *a, int *type, BOOL js)
{
	ATTR *rel;
	char *v;
	int ret;

	*type = RES_LINK;
	if(js == FALSE){
		//イベントハンドラの属性は削除する
		if(a->nlen > 2 && TO_LOWER(a->name[0]) == 'o' && TO_LOWER(a->name[1]) == 'n'){
			return A_DELETE;
		}
		//iframe の sandbox 属性は付け直す
		if(EqualI(tag, tlen, "iframe") == TRUE && EqualI(a->name, a->nlen, "sandbox") == TRUE){
			return A_DELETE;
		}
		if(EqualI(a->name, a->nlen, "xlink:href") == TRUE){
			return (a->val != NULL) ? A_URL : A_NONE;
		}
	}
	if(EqualI(a->name, a->nlen, "style") == TRUE){
		return (a->val != NULL) ? A_STYLE : A_NONE;
	}
	if(EqualI(a->name, a->nlen, "srcset") == TRUE || EqualI(a->name, a->nlen, "imagesrcset") == TRUE){
		return A_RENAME;
	}
	if(EqualI(tag, tlen, "base") == TRUE && EqualI(a->name, a->nlen, "href") == TRUE){
		return A_RENAME;
	}
	if(EqualI(tag, tlen, "meta") == TRUE && EqualI(a->name, a->nlen, "http-equiv") == TRUE){
		v = AttrValue(a);
		ret = (v != NULL && (lstrcmpi(v, "refresh") == 0)) ? A_RENAME : A_NONE;
		if(v != NULL) HeapFree(GetProcessHeap(), 0, v);
		return ret;
	}
	if(a->val == NULL){
		return A_NONE;
	}
	if(EqualI(a->name, a->nlen, "background") == TRUE){
		*type = RES_IMAGE;
		return A_URL;
	}
	if(EqualI(a->name, a->nlen, "src") == TRUE || EqualI(a->name, a->nlen, "lowsrc") == TRUE){
		if(EqualI(tag, tlen, "img") == TRUE || EqualI(tag, tlen, "input") == TRUE){
			*type = RES_IMAGE;
		}else if(EqualI(tag, tlen, "script") == TRUE){
			*type = RES_JS;
		}
		return A_URL;
	}
	if(EqualI(a->name, a->nlen, "poster") == TRUE){
		*type = (EqualI(tag, tlen, "video") == TRUE) ? RES_IMAGE : RES_LINK;
		return A_URL;
	}
	if(EqualI(a->name, a->nlen, "href") == TRUE){
		if(EqualI(tag, tlen, "link") == TRUE && (rel = FindAttr(attr, cnt, "rel")) != NULL && rel->val != NULL){
			if(HasToken(rel->val, rel->vlen, "stylesheet") == TRUE){
				*type = RES_CSS;
			}else if(HasToken(rel->val, rel->vlen, "icon") == TRUE){
				*type = RES_IMAGE;
			}
		}
		return A_URL;
	}
	if(EqualI(a->name, a->nlen, "action") == TRUE || EqualI(a->name, a->nlen, "formaction") == TRUE ||
		EqualI(a->name, a->nlen, "cite") == TRUE || EqualI(a->name, a->nlen, "longdesc") == TRUE ||
		EqualI(a->name, a->nlen, "data") == TRUE){
		return A_URL;
	}
	return A_NONE;
}

/* 絶対URLにできないURLをそのまま残せるか (JavaScript を保存しない場合) */
static BOOL KeepRawUrl(const char *v)
{
	static const char *scheme[] = { "mailto:", "tel:", "ftp:", "data:" };
	const char *end;
	int i;

	if(v == NULL){
		return FALSE;
	}
	for(; *v != '\0' && (unsigned char)*v <= 0x20; v++);
	if(*v == '\0' || *v == '#'){
		return TRUE;
	}
	end = v + lstrlen(v);
	for(i = 0; i < sizeof(scheme) / sizeof(scheme[0]); i++){
		if(StartsWithI(v, end, scheme[i]) == TRUE){
			return TRUE;
		}
	}
	//javascript: など
	return FALSE;
}

/* 属性を書き換えて出力 */
static void EmitAttr(ARCHIVE_BUF *b, ATTR *a, int action, int type, const char *base, ARCHIVE_RESOLVE cb, void *ctx, BOOL js)
{
	char *v, *abs, *css;
	const char *local = NULL;
	long clen;

	switch(action)
	{
	case A_DELETE:
		return;

	case A_RENAME:
		Buf_Str(b, "data-wwwc-");
		Buf_Add(b, a->start, (long)(a->end - a->start));
		return;

	case A_URL:
		v = AttrValue(a);
		abs = (char *)HeapAlloc(GetProcessHeap(), 0, ARCHIVE_URL_MAX + 1);
		if(v == NULL || abs == NULL || Archive_ResolveUrl(base, v, abs, ARCHIVE_URL_MAX + 1) == FALSE){
			if(js == TRUE || KeepRawUrl(v) == TRUE){
				Buf_Add(b, a->start, (long)(a->end - a->start));
			}
		}else{
			if(cb != NULL && type != RES_LINK){
				local = cb(ctx, abs, type);
			}
			Buf_Add(b, a->name, a->nlen);
			Buf_Str(b, "=\"");
			if(local != NULL){
				Buf_Str(b, ARCHIVE_RES_PREFIX);
				AddAttrValue(b, local);
			}else{
				AddAttrValue(b, abs);
			}
			Buf_Str(b, "\"");
		}
		if(v != NULL) HeapFree(GetProcessHeap(), 0, v);
		if(abs != NULL) HeapFree(GetProcessHeap(), 0, abs);
		return;

	case A_STYLE:
		v = AttrValue(a);
		css = (v != NULL) ? Archive_RewriteCss(v, lstrlen(v), base, ARCHIVE_RES_PREFIX, cb, ctx, &clen) : NULL;
		if(css == NULL){
			Buf_Add(b, a->start, (long)(a->end - a->start));
		}else{
			Buf_Add(b, a->name, a->nlen);
			Buf_Str(b, "=\"");
			AddAttrValue(b, css);
			Buf_Str(b, "\"");
			Archive_Free(css);
		}
		if(v != NULL) HeapFree(GetProcessHeap(), 0, v);
		return;
	}
	Buf_Add(b, a->start, (long)(a->end - a->start));
}

/* <base href> を取得する */
static void GetBaseHref(const char *html, const char *end, const char *url, char *ret, int size)
{
	const char *p, *q, *name;
	ATTR attr[MAX_ATTR], *a;
	long nlen;
	int cnt;
	char *v;

	lstrcpyn(ret, url, size);
	for(p = html; p < end && (p = FindI(p, end, "<base")) != NULL; p++){
		if(p + 5 < end && !IS_SPACE(*(p + 5)) && *(p + 5) != '>' && *(p + 5) != '/'){
			continue;
		}
		if((q = ParseTag(p, end, &name, &nlen, attr, &cnt, NULL)) == NULL){
			return;
		}
		if((a = FindAttr(attr, cnt, "href")) != NULL && (v = AttrValue(a)) != NULL){
			char *abs = (char *)HeapAlloc(GetProcessHeap(), 0, ARCHIVE_URL_MAX + 1);
			if(abs != NULL && Archive_ResolveUrl(url, v, abs, ARCHIVE_URL_MAX + 1) == TRUE){
				lstrcpyn(ret, abs, size);
			}
			if(abs != NULL) HeapFree(GetProcessHeap(), 0, abs);
			HeapFree(GetProcessHeap(), 0, v);
		}
		return;
	}
}

/* 先頭に挿入する文字列 */
static void MakeHead(ARCHIVE_BUF *h, const char *url, const char *charset, BOOL js)
{
	char buf[64];
	const char *p;
	BOOL valid = TRUE;

	//文字コード (HTTPヘッダで指定されたもの)
	if(charset != NULL && *charset != '\0' && lstrlen(charset) < 40){
		for(p = charset; *p != '\0'; p++){
			if(!(IS_ALPHA(*p) || IS_DIGIT(*p) || *p == '-' || *p == '_' || *p == '.' || *p == ':')){
				valid = FALSE;
			}
		}
		if(valid == TRUE){
			Buf_Str(h, "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=");
			Buf_Str(h, charset);
			Buf_Str(h, "\">\r\n");
		}
	}
	//スクリプトとプラグインの制限
	Buf_Str(h, "<meta http-equiv=\"Content-Security-Policy\" content=\"");
	Buf_Str(h, (js == TRUE) ? "object-src 'none'" : "script-src 'none'; object-src 'none'");
	Buf_Str(h, "\">\r\n");
	//取得元 (Mark of the Web)
	if(url != NULL && lstrlen(url) < 10000 && FindI(url, url + lstrlen(url), "--") == NULL &&
		FindI(url, url + lstrlen(url), ">") == NULL){
		wsprintf(buf, "<!-- saved from url=(%04d)", lstrlen(url));
		Buf_Str(h, buf);
		Buf_Str(h, url);
		Buf_Str(h, " -->\r\n");
	}
}

/* 生テキスト要素の終わりを検索 */
static const char *FindEndTag(const char *p, const char *end, const char *tag)
{
	const char *q;
	int tl = lstrlen(tag);

	for(q = p; q < end && (q = FindI(q, end, "</")) != NULL; q += 2){
		if(StartsWithI(q + 2, end, tag) == TRUE && (q + 2 + tl >= end || !IsTagNameChar(*(q + 2 + tl)))){
			return q;
		}
	}
	return end;
}


/******************************************************************************

	Archive_RewriteHtml

	HTML内のURLをアーカイブのファイル名または絶対URLに書き換える

******************************************************************************/

char *Archive_RewriteHtml(const char *html, long len, const char *url, const char *charset, BOOL js,
						ARCHIVE_RESOLVE cb, void *ctx, long *outlen)
{
	ARCHIVE_BUF b, h;
	const char *p = html, *end = html + len;
	const char *q, *name, *seg, *te, *raw, *tail;
	ATTR attr[MAX_ATTR];
	char *base, *css;
	long nlen, clen, insertAt = -1, htmlPos = -1, doctypeEnd = -1;
	int cnt, i, action, type;
	BOOL jis = FALSE;
	BOOL endtag, over;

	base = (char *)HeapAlloc(GetProcessHeap(), 0, ARCHIVE_URL_MAX + 1);
	if(base == NULL){
		return NULL;
	}
	GetBaseHref(html, end, url, base, ARCHIVE_URL_MAX + 1);

	ZeroMemory(&b, sizeof(ARCHIVE_BUF));
	ZeroMemory(&h, sizeof(ARCHIVE_BUF));
	Buf_Reserve(&b, len + len / 4 + 1024);
	while(p < end && b.err == FALSE){
		//ISO-2022-JP の2バイトモードではタグとして扱わない
		if(*p == 0x1B && p + 2 < end){
			if(*(p + 1) == '$' && (*(p + 2) == 'B' || *(p + 2) == '@')){
				jis = TRUE;
			}else if(*(p + 1) == '(' && (*(p + 2) == 'B' || *(p + 2) == 'J')){
				jis = FALSE;
			}
			Buf_Add(&b, p, 3);
			p += 3;
			continue;
		}
		if(jis == TRUE || *p != '<'){
			for(q = p + 1; q < end && *q != '<' && *q != 0x1B; q++);
			if(jis == TRUE){
				for(q = p + 1; q < end && *q != 0x1B; q++);
			}
			Buf_Add(&b, p, (long)(q - p));
			p = q;
			continue;
		}
		//コメント
		if(StartsWithI(p, end, "<!--") == TRUE){
			q = FindI(p + 4, end, "-->");
			q = (q != NULL) ? q + 3 : end;
			Buf_Add(&b, p, (long)(q - p));
			p = q;
			continue;
		}
		//DOCTYPE 等
		if(p + 1 < end && (*(p + 1) == '!' || *(p + 1) == '?')){
			for(q = p; q < end && *q != '>'; q++);
			q = (q < end) ? q + 1 : end;
			Buf_Add(&b, p, (long)(q - p));
			if(doctypeEnd < 0 && StartsWithI(p, end, "<!doctype") == TRUE){
				doctypeEnd = b.len;
			}
			p = q;
			continue;
		}
		endtag = (p + 1 < end && *(p + 1) == '/') ? TRUE : FALSE;
		if(endtag == TRUE){
			for(q = p; q < end && *q != '>'; q++);
			q = (q < end) ? q + 1 : end;
			Buf_Add(&b, p, (long)(q - p));
			p = q;
			continue;
		}
		if(p + 1 >= end || !IS_ALPHA(*(p + 1)) || (te = ParseTag(p, end, &name, &nlen, attr, &cnt, &over)) == NULL){
			Buf_Add(&b, p, 1);
			p++;
			continue;
		}

		//挿入位置
		if(insertAt < 0){
			if(EqualI(name, nlen, "html") == TRUE){
				//html の後に head が無い場合に使用
			}else if(EqualI(name, nlen, "head") != TRUE){
				insertAt = (htmlPos >= 0) ? htmlPos : b.len;
			}
		}

		//JavaScript を保存しない場合は script 要素を削除する
		if(js == FALSE && EqualI(name, nlen, "script") == TRUE){
			if(*(te - 2) == '/'){
				//<script ... />
				p = te;
				continue;
			}
			q = FindEndTag(te, end, "script");
			for(; q < end && *q != '>'; q++);
			p = (q < end) ? q + 1 : end;
			continue;
		}

		//属性を書き換えてタグを出力
		seg = p;
		for(i = 0; i < cnt; i++){
			action = AttrAction(name, nlen, attr, cnt, &attr[i], &type, js);
			if(action == A_NONE){
				continue;
			}
			Buf_Add(&b, seg, (long)(attr[i].start - seg));
			EmitAttr(&b, &attr[i], action, type, base, cb, ctx, js);
			seg = attr[i].end;
		}
		if(js == TRUE){
			Buf_Add(&b, seg, (long)(te - seg));
		}else{
			//属性が多すぎる場合は処理していない属性を削除し、iframe ではスクリプトを禁止する
			tail = (over == TRUE) ? attr[cnt - 1].end : te - 1;
			if(tail > seg){
				Buf_Add(&b, seg, (long)(tail - seg));
			}
			if(EqualI(name, nlen, "iframe") == TRUE){
				Buf_Str(&b, " sandbox");
			}
			Buf_Str(&b, ">");
		}

		if(insertAt < 0){
			if(EqualI(name, nlen, "head") == TRUE){
				insertAt = b.len;
			}else if(EqualI(name, nlen, "html") == TRUE && htmlPos < 0){
				htmlPos = b.len;
			}
		}
		p = te;

		//生テキストの要素
		if(EqualI(name, nlen, "style") == TRUE){
			q = FindEndTag(p, end, "style");
			css = Archive_RewriteCss(p, (long)(q - p), base, ARCHIVE_RES_PREFIX, cb, ctx, &clen);
			if(css != NULL){
				Buf_Add(&b, css, clen);
				Archive_Free(css);
			}else{
				Buf_Add(&b, p, (long)(q - p));
			}
			p = q;
		}else if((raw = (EqualI(name, nlen, "script") == TRUE) ? "script" :
			(EqualI(name, nlen, "textarea") == TRUE) ? "textarea" :
			(EqualI(name, nlen, "title") == TRUE) ? "title" :
			(EqualI(name, nlen, "xmp") == TRUE) ? "xmp" : NULL) != NULL){
			q = FindEndTag(p, end, raw);
			Buf_Add(&b, p, (long)(q - p));
			p = q;
		}
	}
	HeapFree(GetProcessHeap(), 0, base);
	if(b.err == TRUE){
		Buf_Free(&b);
		return NULL;
	}

	//先頭に挿入
	if(insertAt < 0){
		insertAt = (htmlPos >= 0) ? htmlPos : ((doctypeEnd >= 0) ? doctypeEnd : 0);
	}
	MakeHead(&h, url, charset, js);
	if(h.err == TRUE || Buf_Reserve(&b, h.len) == FALSE){
		Buf_Free(&h);
		Buf_Free(&b);
		return NULL;
	}
	MoveMemory(b.p + insertAt + h.len, b.p + insertAt, b.len - insertAt);
	CopyMemory(b.p + insertAt, h.p, h.len);
	b.len += h.len;
	Buf_Free(&h);
	return Buf_Detach(&b, outlen);
}
/* End of source */
