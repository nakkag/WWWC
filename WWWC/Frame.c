/**************************************************************************

	WWWC

	Frame.c

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

#define NOMOVESIZE		6		//フレームの移動制限値


/**************************************************************************
	Global Variables
**************************************************************************/

static RECT *FrmRect;			//フレームの位置情報


/******************************************************************************

	FrameInitialize

	フレーム描画用構造体の初期化

******************************************************************************/

BOOL FrameInitialize(HWND hWnd)
{
	if(FrmRect == NULL){
		FrmRect = (RECT *)GlobalAlloc(GPTR, sizeof(RECT) * FRAME_CNT);
		if(FrmRect == NULL){
			return FALSE;
		}
	}
	ZeroMemory(FrmRect, sizeof(RECT) * FRAME_CNT);

	SetCapture(hWnd);
	return TRUE;
}


/******************************************************************************

	FrameFree

	フレーム描画用構造体の解放

******************************************************************************/

void FrameFree(void)
{
	if(FrmRect != NULL){
		GlobalFree(FrmRect);
		FrmRect = NULL;
	}
	ReleaseCapture();
}


/******************************************************************************

	FrameDraw

	フレームの描画

******************************************************************************/

int FrameDraw(HWND hWnd)
{
	RECT WindowRect, TreeViewRect;
	POINT apos;
	HDC hdc;
	int drawCnt;

	GetCursorPos((LPPOINT)&apos);
	GetWindowRect(hWnd, (LPRECT)&WindowRect);
	GetWindowRect(GetDlgItem(hWnd, WWWC_TREE), (LPRECT)&TreeViewRect);

	//フレームの移動制限
	if(apos.x <= (WindowRect.left + NOMOVESIZE + GetSystemMetrics(SM_CXFRAME))){
		apos.x = WindowRect.left + NOMOVESIZE + GetSystemMetrics(SM_CXFRAME);

	}else if(apos.x >= (WindowRect.right - (NOMOVESIZE + (FRAME_CNT * 2)) - GetSystemMetrics(SM_CXFRAME))){
		apos.x = WindowRect.right - (NOMOVESIZE + (FRAME_CNT * 2)) - GetSystemMetrics(SM_CXFRAME);
	}

	//前回の位置と比較
	if(apos.x == FrmRect[0].left){
		return 1;
	}

	hdc = GetWindowDC(hWnd);

	//前回描画分を消去
	for(drawCnt = 0;drawCnt < FRAME_CNT;drawCnt++){
		DrawFocusRect(hdc, (LPRECT)&FrmRect[drawCnt]);
	}

	//フレームの描画
	for(drawCnt = 0;drawCnt < FRAME_CNT;drawCnt++){
		(FrmRect + drawCnt)->left = apos.x + drawCnt - WindowRect.left;
		(FrmRect + drawCnt)->right = (FrmRect + drawCnt)->left + FRAME_CNT + 1;
		(FrmRect + drawCnt)->top = TreeViewRect.top - WindowRect.top;
		(FrmRect + drawCnt)->bottom = TreeViewRect.bottom - WindowRect.top;

		DrawFocusRect(hdc, (LPRECT)(FrmRect + drawCnt));
	}
	ReleaseDC(hWnd, hdc);
	return 0;
}


/******************************************************************************

	FrameDrawEnd

	フレームの描画終了、フレームの最終位置を返す

******************************************************************************/

int FrameDrawEnd(HWND hWnd)
{
	HDC hdc;
	int drawCnt;
	int ret;

	if(FrmRect[0].left == 0 &&
		FrmRect[0].right == 0 &&
		FrmRect[0].top == 0 &&
		FrmRect[0].bottom == 0){

		FrameFree();
		return -1;
	}

	//前回描画分を消去
	hdc = GetWindowDC(hWnd);
	for(drawCnt = 0;drawCnt < FRAME_CNT;drawCnt++){
		DrawFocusRect(hdc, (LPRECT)&FrmRect[drawCnt]);
	}
	ReleaseDC(hWnd, hdc);

	//境界位置の取得
	ret = FrmRect[0].left - GetSystemMetrics(SM_CXFRAME);

	FrameFree();
	return ret;
}
/* End of source */
