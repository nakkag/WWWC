/**************************************************************************

	WWWC

	Dialog.c

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

#define TITLESIZE_X			333		//�^�C�g���E�B���h�E�̃T�C�Y
#define TITLESIZE_Y			282

#define CPTEXT_X			77		//Copyright�̃e�L�X�g�ʒu
#define CPTEXT_Y			3
#define CPTEXT_X2			(CPTEXT_X + 250)
#define CPTEXT_Y2			(CPTEXT_Y + 30)
#define CPTEXT_FONT			"Times New Roman"
#define CPTEXT_FONTSIZE		9

#define UMI_X				0		//�^�C�g���E�B���h�E�ɕ\������r�b�g�}�b�v�̈ʒu
#define UMI_Y				0
#define SIGN_X				295
#define SIGN_Y				36
#define TXT_X				125
#define TXT_Y				220


/**************************************************************************
	Global Variables
**************************************************************************/

//�O���Q��
extern HINSTANCE g_hinst;
extern int gCheckFlag;			//�`�F�b�N�t���O
extern char AniIcon[];			//�A�j���[�V����bitmap


/**************************************************************************
	Local Function Prototypes
**************************************************************************/

static HPALETTE SetPalette(HWND hWnd, LPBITMAPINFOHEADER lpBi);
static void ShowStretchDIB(HDC hdc, char *mDIB,
						   int x, int y, int x2, int y2, int xx, int yy);
static BOOL CALLBACK TitleProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);


/******************************************************************************

	SetPalette

	�p���b�g�̍쐬

******************************************************************************/

static HPALETTE SetPalette(HWND hWnd, LPBITMAPINFOHEADER lpBi)
{
	LPLOGPALETTE lpPal;
	LPRGBQUAD lpRGB;
	HANDLE hPal;
	WORD i;
	DWORD dwClrUsed;
	HPALETTE hPalette;

	dwClrUsed = lpBi->biClrUsed;

	hPal = GlobalAlloc(GHND, sizeof(LOGPALETTE) + dwClrUsed * sizeof(PALETTEENTRY));
	if(hPal == NULL){
		return NULL;
	}
	lpPal = (LPLOGPALETTE)GlobalLock(hPal);

	lpPal->palVersion = 0x300;
	lpPal->palNumEntries = (WORD)dwClrUsed;

	lpRGB = (LPRGBQUAD)((LPSTR)lpBi + lpBi->biSize);

	//�p���b�g�̃R�s�[
	for (i = 0;i < dwClrUsed;i++, lpRGB++) {
		(lpPal->palPalEntry + i)->peRed = lpRGB->rgbRed;
		(lpPal->palPalEntry + i)->peGreen = lpRGB->rgbGreen;
		(lpPal->palPalEntry + i)->peBlue = lpRGB->rgbBlue;
		(lpPal->palPalEntry + i)->peFlags = 0;
	}
	GlobalUnlock(hPal);

	//�p���b�g�̍쐬
	hPalette = CreatePalette(lpPal);

	GlobalFree(hPal);
	return hPalette;
}


/******************************************************************************

	ShowStretchDIB

	DIB�̕`��

******************************************************************************/

static void ShowStretchDIB(HDC hdc, char *mDIB, int x, int y, int x2, int y2, int xx, int yy)
{
	PBITMAPINFO pbi;
	PBITMAPINFOHEADER pbih;
	char *lpBits;
	int cClrBits;

	pbi = (BITMAPINFO *)mDIB;
	pbih = (PBITMAPINFOHEADER)pbi;

	//�r�b�g�}�b�v�r�b�g�̈ʒu���擾
	if((cClrBits = pbih->biClrUsed) == 0){
		cClrBits = pbih->biPlanes * pbih->biBitCount;
		if (cClrBits == 1){
			cClrBits = 2;
		}else if (cClrBits <= 4){
			cClrBits = 16;
		}else if (cClrBits <= 8){
			cClrBits = 256;
		}else if (cClrBits <= 16){
			cClrBits = 3;
		}else if (cClrBits <= 24){
			cClrBits = 0;
		}else{
			cClrBits = 3;
		}
	}
	lpBits = (char *)mDIB + sizeof(BITMAPINFOHEADER) + cClrBits * sizeof(RGBQUAD);

	//�`�惂�[�h�̐ݒ�
	SetStretchBltMode(hdc, COLORONCOLOR);
	//DIB�̕`��
	StretchDIBits(hdc, x, y, x2, y2, xx, yy,
		pbih->biWidth, pbih->biHeight, lpBits, pbi, DIB_RGB_COLORS, SRCCOPY);
}


/******************************************************************************

	TitleProc

	�^�C�g���E�B���h�E�̃v���V�[�W��

******************************************************************************/

static BOOL CALLBACK TitleProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PBITMAPINFO pbi;
	PAINTSTRUCT ps;
	HPALETTE hTmpPalette = NULL;
	static HPALETTE hPalette;
	static HBRUSH hBrush;
	static HFONT hFont;
	RECT DesktopRect, DrawRect, text_rect;
	HGLOBAL hMem;
	HRSRC hRsrc;
	HDC hdc;
	HFONT hRetFont;
	static char *mDIB_umi, *mDIB_txt, *mDIB_sign;

	switch (uMsg)
	{
	case WM_INITDIALOG:
		//�w�i�F
		hBrush = CreateSolidBrush(RGB(255,255,255));
		//�t�H���g
		hFont = CreateListFont(CPTEXT_FONT, CPTEXT_FONTSIZE, DEFAULT_CHARSET);

		//���\�[�X����DIB���擾
		hRsrc = FindResource(g_hinst, MAKEINTRESOURCE(IDB_BITMAP_UMI), RT_BITMAP);
		hMem = LoadResource(g_hinst, hRsrc);
		mDIB_umi = (char *)LockResource(hMem);

		hRsrc = FindResource(g_hinst, MAKEINTRESOURCE(IDB_BITMAP_TEXT), RT_BITMAP);
		hMem = LoadResource(g_hinst, hRsrc);
		mDIB_txt = (char *)LockResource(hMem);

		hRsrc = FindResource(g_hinst, MAKEINTRESOURCE(IDB_BITMAP_SIGN), RT_BITMAP);
		hMem = LoadResource(g_hinst, hRsrc);
		mDIB_sign = (char *)LockResource(hMem);

		//DIB����p���b�g���쐬
		pbi = (PBITMAPINFO)mDIB_umi;
		hPalette = SetPalette(hDlg, (PBITMAPINFOHEADER)pbi);

		GetWindowRect(GetDesktopWindow(), (LPRECT)&DesktopRect);	//�f�X�N�g�b�v�̃T�C�Y
		SetWindowPos(hDlg, HWND_TOP,
			(DesktopRect.right / 2) - (TITLESIZE_X / 2),
			(DesktopRect.bottom / 2) - (TITLESIZE_Y / 2),
			TITLESIZE_X, TITLESIZE_Y, SWP_HIDEWINDOW);

		ShowWindow(hDlg, SW_SHOW);
		InvalidateRect(hDlg, NULL, FALSE);
		UpdateWindow(hDlg);
		break;

	case WM_CLOSE:
		if(hPalette != NULL){
			//�p���b�g��j��
			DeleteObject(hPalette);
			hPalette = NULL;
		}
		if(hFont != NULL){
			DeleteObject(hFont);
			hFont = NULL;
		}
		if(hBrush != NULL){
			DeleteObject(hBrush);
			hBrush = NULL;
		}
		DestroyWindow(hDlg);
		break;

	case WM_PALETTECHANGED:
	case WM_QUERYNEWPALETTE:
		//�ĕ`��
		InvalidateRect(hDlg, NULL, FALSE);
		UpdateWindow(hDlg);
		break;

	case WM_PAINT:
		hdc = BeginPaint(hDlg, &ps);

		if(hPalette != NULL){
			hTmpPalette = SelectPalette(hdc, hPalette, FALSE);
			RealizePalette(hdc);
		}

		GetClientRect(hDlg, &DrawRect);

		//DIB��`��
		ShowStretchDIB(hdc, mDIB_umi, UMI_X, UMI_Y, ((PBITMAPINFOHEADER)mDIB_umi)->biWidth,
			((PBITMAPINFOHEADER)mDIB_umi)->biHeight, 0, 0);

		//Copyright�̕`��
		SetTextColor(hdc, RGB(0, 0, 0));
		SetBkMode(hdc, TRANSPARENT);
		SetRect(&text_rect, CPTEXT_X, CPTEXT_Y, CPTEXT_X2, CPTEXT_Y2);
		hRetFont = SelectObject(hdc, hFont);
		DrawText(hdc, APP_COPYRIGHT, lstrlen(APP_COPYRIGHT), &text_rect, DT_RIGHT | DT_WORDBREAK | DT_NOPREFIX);
		SelectObject(hdc, hRetFont);

		ShowStretchDIB(hdc, mDIB_sign, SIGN_X, SIGN_Y, ((PBITMAPINFOHEADER)mDIB_sign)->biWidth,
			((PBITMAPINFOHEADER)mDIB_sign)->biHeight, 0, 0);

		ShowStretchDIB(hdc, mDIB_txt, TXT_X, TXT_Y, ((PBITMAPINFOHEADER)mDIB_txt)->biWidth,
			((PBITMAPINFOHEADER)mDIB_txt)->biHeight, 0, 0);

		//�ȑO�̃p���b�g�ɖ߂�
		if(hTmpPalette != NULL){
			SelectPalette(hdc, hTmpPalette, FALSE);
		}

		EndPaint(hDlg, &ps);
		break;

	//�w�i��ݒ�
	case WM_CTLCOLORDLG:
		return (int)hBrush;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			SendMessage(hDlg, WM_CLOSE, 0, 0);
			break;
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}


/******************************************************************************

	CreateTitleWindow

	�^�C�g���E�B���h�E�̍쐬

******************************************************************************/

HWND CreateTitleWindow(HWND hWnd)
{
	//�^�C�g���E�B���h�E��\������
	return CreateDialogParam(g_hinst,MAKEINTRESOURCE(IDD_DIALOG_TITLE),
		hWnd, TitleProc, 0);
}


/******************************************************************************

	CloseTitleWindow

	�^�C�g���E�B���h�E�����

******************************************************************************/

void CloseTitleWindow(HWND tWnd)
{
	if(tWnd != NULL){
		InvalidateRect(tWnd, NULL, FALSE);
		UpdateWindow(tWnd);
		//�^�C�g���E�B���h�E�����
		SendMessage(tWnd, WM_CLOSE, 0, 0);
		tWnd = NULL;
	}
}


/******************************************************************************

	AniProc

	�A�j���[�V�����E�B���h�E�̃v���V�[�W��

******************************************************************************/

BOOL CALLBACK AniProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	HDC mdc;
	BITMAP bmp;
	HBITMAP hRetBmp;
	static int Step;
	static HBITMAP hBmp;

	switch (uMsg)
	{
	case WM_INITDIALOG:
		Step = 0;
		hBmp = NULL;
		//�\������r�b�g�}�b�v���w�肳��Ă���ꍇ
		if(*AniIcon != '\0'){
			hBmp = LoadImage(NULL, AniIcon, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		}
		if(hBmp == NULL){
			//���\�[�X���r�b�g�}�b�v��ǂݍ���
			hBmp = LoadBitmap(g_hinst, MAKEINTRESOURCE(IDB_BITMAP_ANI));
		}
		break;

	case WM_CLOSE:
		//�r�b�g�}�b�v��j��
		DeleteObject(hBmp);
		DestroyWindow(hDlg);
		break;

	case WM_LBUTTONDBLCLK:
		//�z�[���y�[�W
		ExecItemFile(hDlg, APP_URL, "", NULL, 0);
		break;

	case WM_MOUSEMOVE:
		SetCursor(LoadCursor(g_hinst, MAKEINTRESOURCE(IDC_CURSOR_HAND)));
		break;

	case WM_TIMER:
		switch(wParam)
		{
		case TIMER_ANI:
			if(gCheckFlag == 0){
				KillTimer(hDlg, TIMER_ANI);
				Step = 0;
			}else{
				//�`�F�b�N���̏ꍇ�Ƀr�b�g�}�b�v�����ɂ��炵�Ă���
				Step++;

				GetObject(hBmp, sizeof(BITMAP), &bmp);
				if(Step >= (bmp.bmWidth / STEPSIZE)){
					Step = 0;
				}
			}
			InvalidateRect(hDlg, NULL, FALSE);
			UpdateWindow(hDlg);
			break;
		}
		break;

	case WM_PAINT:
		hdc = BeginPaint(hDlg, &ps);

		//�w��ʒu�̃r�b�g�}�b�v��`��
		mdc = CreateCompatibleDC(hdc);
		hRetBmp = SelectObject(mdc, hBmp);
		BitBlt(hdc,
			ps.rcPaint.left,
			ps.rcPaint.top,
			ps.rcPaint.right,
			ps.rcPaint.bottom,
			mdc,
			ps.rcPaint.left + (Step * STEPSIZE),
			ps.rcPaint.top,
			SRCCOPY);

		SelectObject(mdc, hRetBmp);
		DeleteDC(mdc);

		EndPaint(hDlg, &ps);
		break;

	default:
		return FALSE;
	}
	return TRUE;
}
/* End of source */
