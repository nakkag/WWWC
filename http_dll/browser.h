/**************************************************************************

	WWWC (wwwc.dll)

	browser.h

	Copyright (C) 1996-2026 by Ohno Tomoaki. All rights reserved.
		https://www.nakka.com/
		nakka@nakka.com

**************************************************************************/

#ifndef _INC_BROWSER_H
#define _INC_BROWSER_H

/**************************************************************************
	Struct
**************************************************************************/

/* ブラウザで表示しているページ */
typedef struct _BROWSER_PAGE {
	char *Title;						/* ページのタイトル */
	char *Url;							/* URL (http, https のみ) */
	char Browser[64];					/* ブラウザ名 */
} BROWSER_PAGE;


/**************************************************************************
	Function Prototypes
**************************************************************************/

int Browser_GetPages(BROWSER_PAGE **ret, BOOL First);
void Browser_FreePage(BROWSER_PAGE *page);
void Browser_FreePages(BROWSER_PAGE *pages, int cnt);

#endif
/* End of source */
