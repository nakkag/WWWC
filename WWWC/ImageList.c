/**************************************************************************

	WWWC

	ImageList.c

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
	Global Variables
**************************************************************************/

//外部参照
extern HINSTANCE g_hinst;			//アプリケーションのインスタンスハンドル


/******************************************************************************

	ImageListIconAdd

	イメージリストにアイコンを追加
	ファイルが指定されていない場合はリソースから取得

******************************************************************************/

int ImageListIconAdd(HIMAGELIST IconList, int Index, int IconSize, char *buf, int iIndex)
{
	HICON hIcon = NULL;
	HICON hsIcon = NULL;
	int ret;

	if(*buf != '\0'){
		//アイコンファイルが指定されている場合はファイルからアイコンを取得
		ExtractIconEx(buf, iIndex, &hIcon, &hsIcon, 1);
		if(IconSize >= LICONSIZE){
			DestroyIcon(hsIcon);
		}else{
			DestroyIcon(hIcon);
			hIcon = hsIcon;
		}
	}
	if(hIcon == NULL){
		//アイコンが取得されていない場合はリソースから読み込む
		hIcon = (HICON)LoadImage(g_hinst, MAKEINTRESOURCE(Index), IMAGE_ICON,
			IconSize, IconSize, 0);
	}

	//イメージリストにアイコンを追加
	ret = ImageList_AddIcon(IconList, hIcon);

	DestroyIcon(hIcon);
	return ret;
}


/******************************************************************************

	ImageListFileIconAdd

	イメージリストにアイコンを追加
	ファイルが指定されていない場合は関連付けされたアイコンを取得

******************************************************************************/

int ImageListFileIconAdd(HIMAGELIST IconList, char *FileName, UINT uFlags, int IconSize, char *buf, int iIndex)
{
	SHFILEINFO  shfi;
	HICON hIcon = NULL;
	HICON hsIcon = NULL;
	int ret, IconFlag;

	if(*buf != '\0'){
		//アイコンファイルが指定されている場合はファイルからアイコンを取得
		ExtractIconEx(buf, iIndex, &hIcon, &hsIcon, 1);
		if(IconSize >= LICONSIZE){
			DestroyIcon(hsIcon);
		}else{
			DestroyIcon(hIcon);
			hIcon = hsIcon;
		}
	}
	if(hIcon == NULL){
		IconFlag = SHGFI_ICON | ((IconSize == SICONSIZE) ? SHGFI_SMALLICON : SHGFI_LARGEICON) | uFlags;
		//アイコンが取得されていない場合はリソースから読み込む
		SHGetFileInfo(FileName, SHGFI_USEFILEATTRIBUTES, &shfi, sizeof(SHFILEINFO), IconFlag);
		hIcon = shfi.hIcon;
	}
	if(hIcon == NULL){
		hIcon = (HICON)LoadImage(g_hinst, MAKEINTRESOURCE(IDI_ICON_NON), IMAGE_ICON, IconSize, IconSize, 0);
	}

	//イメージリストにアイコンを追加
	ret = ImageList_AddIcon(IconList, hIcon);

	DestroyIcon(hIcon);
	return ret;
}
/* End of source */
