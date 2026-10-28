/**************************************************************************

	WWWC

	Font.c

	Copyright (C) 1996-2026 by Ohno Tomoaki. All rights reserved.
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


/******************************************************************************

	CreateListFont

	ÉtÉHÉìÉgÇçÏê¨Ç∑ÇÈ

******************************************************************************/

HFONT CreateListFont(char *FontName, int FontSize, int CharSet)
{
	LOGFONT lf;
	HDC hdc;

	ZeroMemory(&lf, sizeof(LOGFONT));

	hdc = GetDC(NULL);
	lf.lfHeight = -(int)((FontSize * GetDeviceCaps(hdc,LOGPIXELSY)) / 72);
	ReleaseDC(NULL, hdc);

	lf.lfWidth = 0;
	lf.lfEscapement = 0;
	lf.lfOrientation = 0;
	lf.lfWeight = 0;
	lf.lfItalic = FALSE;
	lf.lfUnderline = FALSE;
	lf.lfStrikeOut = FALSE;
	lf.lfCharSet = CharSet;
	lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
	lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lf.lfQuality = DEFAULT_QUALITY;
	lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
	lstrcpyn(lf.lfFaceName, FontName, LF_FACESIZE);
	return CreateFontIndirect((CONST LOGFONT *)&lf);
}
/* End of source */
