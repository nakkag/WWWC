/**************************************************************************

	WWWC

	Frame.c

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


/**************************************************************************
	Define
**************************************************************************/

#define NOMOVESIZE		6		//フレームの移動制限値


/**************************************************************************
	Global Variables
**************************************************************************/

static RECT *FrmRect;			//フレームの位置情報 (ウィンドウ座標)
static int FrmPos;				//境界の位置 (クライアント座標)
static int DragOffset;			//境界の左端からドラッグを開始した位置までの距離


/******************************************************************************

	FrameInitialize

	フレーム描画用構造体の初期化

******************************************************************************/

BOOL FrameInitialize(HWND hWnd)
{
	RECT TreeViewRect;
	POINT apos;

	if(FrmRect == NULL){
		FrmRect = (RECT *)GlobalAlloc(GPTR, sizeof(RECT) * FRAME_CNT);
		if(FrmRect == NULL){
			return FALSE;
		}
	}
	ZeroMemory(FrmRect, sizeof(RECT) * FRAME_CNT);
	FrmPos = -1;

	//境界 (ツリービューの右端) のどこをつかんだかを記憶して、押しただけでは境界が動かないようにする
	GetCursorPos(&apos);
	GetWindowRect(GetDlgItem(hWnd, WWWC_TREE), &TreeViewRect);
	DragOffset = apos.x - TreeViewRect.right;

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
	RECT WindowRect, ClientRect, TreeViewRect;
	POINT apos, ClientPos;
	HDC hdc;
	int drawCnt;
	int x;

	GetCursorPos((LPPOINT)&apos);
	ScreenToClient(hWnd, &apos);
	GetClientRect(hWnd, (LPRECT)&ClientRect);
	GetWindowRect(hWnd, (LPRECT)&WindowRect);
	GetWindowRect(GetDlgItem(hWnd, WWWC_TREE), (LPRECT)&TreeViewRect);

	//つかんだ位置からの移動量で境界の位置を求める
	x = apos.x - DragOffset;

	//フレームの移動制限
	if(x <= NOMOVESIZE){
		x = NOMOVESIZE;

	}else if(x >= ClientRect.right - (NOMOVESIZE + (FRAME_CNT * 2))){
		x = ClientRect.right - (NOMOVESIZE + (FRAME_CNT * 2));
	}

	//前回の位置と比較
	if(x == FrmPos){
		return 1;
	}
	FrmPos = x;

	//ウィンドウ座標に変換するためのクライアント領域の左上の位置
	ClientPos.x = ClientPos.y = 0;
	ClientToScreen(hWnd, &ClientPos);

	hdc = GetWindowDC(hWnd);

	//前回描画分を消去
	for(drawCnt = 0;drawCnt < FRAME_CNT;drawCnt++){
		DrawFocusRect(hdc, (LPRECT)&FrmRect[drawCnt]);
	}

	//フレームの描画
	for(drawCnt = 0;drawCnt < FRAME_CNT;drawCnt++){
		(FrmRect + drawCnt)->left = ClientPos.x - WindowRect.left + x + drawCnt;
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

	if(FrmPos == -1){
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
	ret = FrmPos;

	FrameFree();
	return ret;
}
/* End of source */
