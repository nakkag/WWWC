/**************************************************************************

	WWWC (wwwc.dll)

	archive.h

	Copyright (C) 1996-2026 by Ohno Tomoaki. All rights reserved.
		https://www.nakka.com/
		nakka@nakka.com

**************************************************************************/

#ifndef _INC_ARCHIVE_H
#define _INC_ARCHIVE_H

/**************************************************************************
	Define
**************************************************************************/

#define ARCHIVE_ACTION				"openarchive"			/* アーカイブを開くメニューのアクション */
#define ARCHIVE_RES_PREFIX			"res/"				/* ページから見たリソースの場所 */
#define ARCHIVE_URL_MAX				4096				/* 扱うURLの最大長 */
#define ARCHIVE_GEN_MAX				99					/* 保存する世代数の上限 */

/* リソースの種類 */
#define RES_LINK					0					/* 絶対URLにするのみ */
#define RES_IMAGE					1
#define RES_CSS						2
#define RES_JS						3


/**************************************************************************
	Struct
**************************************************************************/

struct TPITEM;

typedef struct _ARCHIVE_BUF {
	char *p;
	long len;
	long cap;
	BOOL err;
} ARCHIVE_BUF;

/* 絶対URLと種類からアーカイブしたファイル名を返す (アーカイブしない場合は NULL) */
typedef const char *(*ARCHIVE_RESOLVE)(void *ctx, const char *url, int type);


/**************************************************************************
	Function Prototypes
**************************************************************************/

//archivehtml.c
void Buf_Add(ARCHIVE_BUF *b, const char *s, long n);
void Buf_Str(ARCHIVE_BUF *b, const char *s);
void Buf_Free(ARCHIVE_BUF *b);
void Archive_Free(void *p);
BOOL Archive_ResolveUrl(const char *base, const char *rel, char *ret, int size);
char *Archive_RewriteCss(const char *css, long len, const char *base, const char *prefix,
					   ARCHIVE_RESOLVE cb, void *ctx, long *outlen);
char *Archive_RewriteHtml(const char *html, long len, const char *url, const char *charset, BOOL js,
						ARCHIVE_RESOLVE cb, void *ctx, long *outlen);

//archive.c
void Archive_Init(void);
void Archive_End(void);
void Archive_Restart(void);
BOOL Archive_IsAuthItem(struct TPITEM *tpItemInfo);
void Archive_Request(HWND hWnd, struct TPITEM *tpItemInfo, BOOL Update);
BOOL Archive_Exists(HWND hWnd, struct TPITEM *tpItemInfo);
int Archive_Delete(HWND hWnd, struct TPITEM *tpItemInfo);
int Archive_Open(HWND hWnd, struct TPITEM *tpItemInfo);
BOOL Archive_GetRoot(HWND hWnd, char *ret);

#endif
/* End of source */
