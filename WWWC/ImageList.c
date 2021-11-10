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

//�O���Q��
extern HINSTANCE g_hinst;			//�A�v���P�[�V�����̃C���X�^���X�n���h��


/******************************************************************************

	ImageListIconAdd

	�C���[�W���X�g�ɃA�C�R����ǉ�
	�t�@�C�����w�肳��Ă��Ȃ��ꍇ�̓��\�[�X����擾

******************************************************************************/

int ImageListIconAdd(HIMAGELIST IconList, int Index, int IconSize, char *buf, int iIndex)
{
	HICON hIcon = NULL;
	HICON hsIcon = NULL;
	int ret;

	if(*buf != '\0'){
		//�A�C�R���t�@�C�����w�肳��Ă���ꍇ�̓t�@�C������A�C�R�����擾
		ExtractIconEx(buf, iIndex, &hIcon, &hsIcon, 1);
		if(IconSize >= LICONSIZE){
			DestroyIcon(hsIcon);
		}else{
			DestroyIcon(hIcon);
			hIcon = hsIcon;
		}
	}
	if(hIcon == NULL){
		//�A�C�R�����擾����Ă��Ȃ��ꍇ�̓��\�[�X����ǂݍ���
		hIcon = (HICON)LoadImage(g_hinst, MAKEINTRESOURCE(Index), IMAGE_ICON,
			IconSize, IconSize, 0);
	}

	//�C���[�W���X�g�ɃA�C�R����ǉ�
	ret = ImageList_AddIcon(IconList, hIcon);

	DestroyIcon(hIcon);
	return ret;
}


/******************************************************************************

	ImageListFileIconAdd

	�C���[�W���X�g�ɃA�C�R����ǉ�
	�t�@�C�����w�肳��Ă��Ȃ��ꍇ�͊֘A�t�����ꂽ�A�C�R�����擾

******************************************************************************/

int ImageListFileIconAdd(HIMAGELIST IconList, char *FileName, UINT uFlags, int IconSize, char *buf, int iIndex)
{
	SHFILEINFO  shfi;
	HICON hIcon = NULL;
	HICON hsIcon = NULL;
	int ret, IconFlag;

	if(*buf != '\0'){
		//�A�C�R���t�@�C�����w�肳��Ă���ꍇ�̓t�@�C������A�C�R�����擾
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
		//�A�C�R�����擾����Ă��Ȃ��ꍇ�̓��\�[�X����ǂݍ���
		SHGetFileInfo(FileName, SHGFI_USEFILEATTRIBUTES, &shfi, sizeof(SHFILEINFO), IconFlag);
		hIcon = shfi.hIcon;
	}
	if(hIcon == NULL){
		hIcon = (HICON)LoadImage(g_hinst, MAKEINTRESOURCE(IDI_ICON_NON), IMAGE_ICON, IconSize, IconSize, 0);
	}

	//�C���[�W���X�g�ɃA�C�R����ǉ�
	ret = ImageList_AddIcon(IconList, hIcon);

	DestroyIcon(hIcon);
	return ret;
}
/* End of source */
