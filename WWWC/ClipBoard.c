/**************************************************************************

	WWWC

	ClipBoard.c

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

/**************************************************************************
	Global Variables
**************************************************************************/

static HTREEITEM CutItem;
static BOOL TreeEditFlag;
static BOOL CutCntFlag;
static HWND ClipNext;

static struct TPITEM **tpDeleteItemList;
static int tpDeleteItemListCnt;

//�O���Q��
extern char CuDir[];
extern HWND TbWnd;
extern HMENU hPOPUP;
extern struct TP_PROTOCOL *tpProtocol;
extern int ProtocolCnt;
extern HTREEITEM RootItem;
extern HTREEITEM RecyclerItem;
extern HTREEITEM HiTestItem;
extern UINT WWWC_ClipFormat;
extern int UpdateItemFlag;


/**************************************************************************
	Local Function Prototypes
**************************************************************************/
static HANDLE Clipboard_Set_WF_FolderPath(HWND hWnd, int mode, HTREEITEM ClipItem);
static HANDLE Clipboard_Set_TEXT_FolderPath(HWND hWnd, HTREEITEM hItem);
static HANDLE Clipboard_Set_TEXT_ItemInfo(struct TPITEM *tpItemInfo);
static BOOL Clipboard_SetTreeData(HWND hWnd, int mode);
static BOOL CopyFolder(HWND hWnd, HTREEITEM hItem, char *FromPath, char *Name, int mode, int DnDFlag);
static BOOL CopyItemInfo(HWND hWnd, HTREEITEM hItem, struct TPITEM *NewItemInfo, char *SourcePath, int mode);
static int StringToItem(HWND hWnd, HTREEITEM hItem, char *Data, int DnDFlag);


/******************************************************************************

	Clipboard_SetChain

	�N���b�v�{�[�h�r���[�A�`�F�[���ɃE�B���h�E���Z�b�g����

******************************************************************************/

void Clipboard_SetChain(HWND hWnd)
{
	ClipNext = SetClipboardViewer(hWnd);
}


/******************************************************************************

	Clipboard_DeleteChain

	�N���b�v�{�[�h�r���[�A�`�F�[���ɃE�B���h�E���폜����

******************************************************************************/

void Clipboard_DeleteChain(HWND hWnd)
{
	if(ClipNext != NULL){
		ChangeClipboardChain(hWnd, ClipNext);
		ClipNext = NULL;
	}
}


/******************************************************************************

	Clipboard_ChangeChain

	�N���b�v�{�[�h�r���[�A�`�F�[���̕ύX�ݒ�

******************************************************************************/

void Clipboard_ChangeChain(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if((HWND)wParam == ClipNext){
		ClipNext = (HWND)lParam;

	}else if(ClipNext != NULL){
		SendMessage(ClipNext, uMsg, wParam, lParam);
	}
}


/******************************************************************************

	Clipboard_Draw

	�N���b�v�{�[�h�̓��e�̕ω�

******************************************************************************/

void Clipboard_Draw(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HTREEITEM pItem;
	int ret;

	if(ClipNext != NULL){
		SendMessage(ClipNext, uMsg, wParam, lParam);
	}
	ret = (Clipboard_CheckFormat() <= 0) ? 1 : 0;
	EnableMenuItem(GetSubMenu(GetMenu(hWnd), MENU_HWND_EDIT), ID_MENUITEM_PASTE, ret);
	EnableMenuItem(GetSubMenu(hPOPUP, MENU_POP_FOLDER), ID_MENUITEM_PASTE, ret);
	SendMessage(TbWnd, TB_ENABLEBUTTON, ID_MENUITEM_PASTE, (LPARAM) MAKELONG(!(ret), 0));

	if(CutCntFlag == FALSE && CutItem != NULL){
		//���ɍ폜���ꂽ�t�H���_���c���[�r���[����폜
		pItem = TreeView_GetParent(GetDlgItem(hWnd, WWWC_TREE), CutItem);

		TreeView_NoFileDelete(hWnd, CutItem, 0);
		ListView_RefreshFolder(hWnd);
		if(TreeEditFlag == FALSE){
			//���ɍ폜���ꂽ�A�C�e�����폜
			Item_Select(hWnd, CutItem);
			TreeView_SetIconState(hWnd, CutItem, 0);
		}else{
			TreeView_SetIconState(hWnd, pItem, 0);
		}
		//�؂���}�[�N�̉���
		Clipboard_DeleteCutStatus(hWnd);

		CutItem = NULL;
		TreeEditFlag = FALSE;
	}
	CutCntFlag = FALSE;
}


/******************************************************************************

	Clipboard_DeleteCutStatus

	�؂���}�[�N�̏���

******************************************************************************/

BOOL Clipboard_DeleteCutStatus(HWND hWnd)
{
	if(CutItem == NULL){
		return TRUE;
	}
	if(TreeEditFlag == TRUE){
		//�c���[�ɐ؂���}�[�N������ꍇ
		TreeEditFlag = FALSE;
		TreeView_SetState(GetDlgItem(hWnd, WWWC_TREE), CutItem, 0, TVIS_CUT);
		CutItem = NULL;
		return TRUE;
	}

	ListView_SetItemState(GetDlgItem(hWnd, WWWC_LIST), -1, 0, LVIS_CUT);

	CallTreeItem(hWnd, CutItem, (FARPROC)TreeView_FreeItem, 1);
	CutItem = NULL;
	return TRUE;
}


/******************************************************************************

	Clipboard_Set_WF_FolderPath

	�t�H���_�̏��𕶎���̃n���h���ɐݒ�

******************************************************************************/

static HANDLE Clipboard_Set_WF_FolderPath(HWND hWnd, int mode, HTREEITEM ClipItem)
{
	HANDLE hMem;
	char *buf;
	char ppath[BUFSIZE];
	char path[BUFSIZE];
	char name[BUFSIZE];
	int ErrCode;

	if(ClipItem == RootItem || ClipItem == RecyclerItem){
		return NULL;
	}
	//�e�t�H���_�̃p�X
	TreeView_GetPath(GetDlgItem(hWnd, WWWC_TREE), TreeView_GetParent(GetDlgItem(hWnd, WWWC_TREE), ClipItem), ppath, CuDir);
	lstrcat(ppath, "\\"DATAFILENAME);
	//�t�H���_�̃p�X
	TreeView_GetPath(GetDlgItem(hWnd, WWWC_TREE), ClipItem, path, CuDir);
	//�t�H���_�̖��O
	TreeView_GetItemInfo(GetDlgItem(hWnd, WWWC_TREE), ClipItem, name);

	if((hMem = GlobalAlloc(GHND, 1 + lstrlen(ppath) + 2 + 1 + lstrlen(path) + 1 + lstrlen(name) + 3)) == NULL){
		ErrMsg(hWnd, GetLastError(), NULL);
		return NULL;
	}
	if((buf = GlobalLock(hMem)) == NULL){
		ErrCode = GetLastError();
		GlobalFree(hMem);
		ErrMsg(hWnd, ErrCode, NULL);
		return NULL;
	}
	//�t�H���_��񕶎���̍쐬
	wsprintf(buf,"%c%s\r\n%c%s\t%s\r\n",mode, ppath, FLAG_FOLDER, path, name);

	GlobalUnlock(hMem);
	return hMem;
}


/******************************************************************************

	Clipboard_Set_WF_ItemList

	�A�C�e���̏��𕶎���̃n���h���ɐݒ�
	(WWWC_ClipFormat)

******************************************************************************/

HANDLE Clipboard_Set_WF_ItemList(HWND hWnd, int mode, HTREEITEM ClipItem)
{
	struct TPITEM *tpItemInfo;
	HTREEITEM hItem;
	HANDLE hMem;
	char TmpBuf[BUFSIZE];
	char SourcePath[BUFSIZE];
	char *buf, *p, *r;
	int SelectItem;
	int ErrCode;
	int cnt;

	if(ClipItem != NULL){
		if(ClipItem == RecyclerItem){
			return NULL;
		}
		return Clipboard_Set_WF_FolderPath(hWnd, mode, ClipItem);
	}

	//�쐬���镶����̃T�C�Y�̎擾
	TreeView_GetPath(GetDlgItem(hWnd, WWWC_TREE), TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)), TmpBuf, CuDir);
	wsprintf(SourcePath, "%s\\"DATAFILENAME, TmpBuf);
	cnt = lstrlen(SourcePath) + 3;			//mode + Path + '\r' + '\n'

	SelectItem = -1;
	while((SelectItem = ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), SelectItem, LVNI_SELECTED)) != -1){
		tpItemInfo = (struct TPITEM *)ListView_GetlParam(GetDlgItem(hWnd, WWWC_LIST), SelectItem);
		if(tpItemInfo != NULL){				//�A�C�e���̏ꍇ
			//�A�C�e��������̃T�C�Y
			cnt += ItemSize(tpItemInfo);
			continue;
		}

		//�t�H���_������̃T�C�Y
		hItem = TreeView_GetIndexToItem(GetDlgItem(hWnd, WWWC_TREE),
			TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)), SelectItem);
		if(hItem == RecyclerItem){
			continue;
		}
		TreeView_GetPath(GetDlgItem(hWnd, WWWC_TREE), hItem, TmpBuf, CuDir);
		cnt += lstrlen(TmpBuf) + 2;			//mode + Path + '\t'
		TreeView_GetItemInfo(GetDlgItem(hWnd, WWWC_TREE), hItem, TmpBuf);
		cnt += lstrlen(TmpBuf) + 2;			//name + '\r' + '\n'
	}

	//�m��
	if((hMem = GlobalAlloc(GHND, cnt + 1)) == NULL){
		ErrMsg(hWnd, GetLastError(), NULL);
		return NULL;
	}
	if((buf = GlobalLock(hMem)) == NULL){
		ErrCode = GetLastError();
		GlobalFree(hMem);
		ErrMsg(hWnd, ErrCode, NULL);
		return NULL;
	}

	p = buf;

	TreeView_GetPath(GetDlgItem(hWnd, WWWC_TREE), TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)), TmpBuf, CuDir);
	wsprintf(SourcePath, "%s\\"DATAFILENAME, TmpBuf);
	*(p++) = mode;
	for(r = SourcePath;*r != '\0';r++) *(p++) = *r;
	*(p++) = '\r', *(p++) = '\n';

	SelectItem = -1;
	while((SelectItem = ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), SelectItem, LVNI_SELECTED)) != -1){
		tpItemInfo = (struct TPITEM *)ListView_GetlParam(GetDlgItem(hWnd, WWWC_LIST), SelectItem);
		if(tpItemInfo != NULL){				//�A�C�e���̏ꍇ
			//�A�C�e�����𕶎���ɂ���
			p = SetItemString(tpItemInfo, p);
			continue;
		}

		//�t�H���_������̒ǉ�
		hItem = TreeView_GetIndexToItem(GetDlgItem(hWnd, WWWC_TREE),
			TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)), SelectItem);
		if(hItem == RecyclerItem){
			continue;
		}

		//�t�H���_�t���O
		*(p++) = FLAG_FOLDER;
		//�p�X
		TreeView_GetPath(GetDlgItem(hWnd, WWWC_TREE), hItem, TmpBuf, CuDir);
		for(r = TmpBuf;*r != '\0';r++) *(p++) = *r;
		*(p++) = '\t';
		//���O
		TreeView_GetItemInfo(GetDlgItem(hWnd, WWWC_TREE), hItem, TmpBuf);
		for(r = TmpBuf;*r != '\0';r++) *(p++) = *r;
		*(p++) = '\r', *(p++) = '\n';
	}
	*p = '\0';

	GlobalUnlock(hMem);
	return hMem;
}


/******************************************************************************

	Clipboard_Set_TEXT_FolderPath

	�t�H���_�̃p�X�𕶎���̃n���h���ɐݒ�
	(CF_TEXT)

******************************************************************************/

static HANDLE Clipboard_Set_TEXT_FolderPath(HWND hWnd, HTREEITEM hItem)
{
	HANDLE hMemText;
	char path[BUFSIZE];
	char *buf;

	//�t�H���_�̃p�X���擾
	TreeView_GetPath(GetDlgItem(hWnd, WWWC_TREE), hItem, path, CuDir);

	if((hMemText = GlobalAlloc(GHND, lstrlen(path) + 1)) == NULL){
		return NULL;
	}
	if((buf = GlobalLock(hMemText)) == NULL){
		GlobalFree(hMemText);
		return NULL;
	}

	//�p�X���R�s�[
	lstrcpy(buf, path);

	GlobalUnlock(hMemText);
	return hMemText;
}


/******************************************************************************

	Clipboard_Set_TEXT_ItemInfo

	�P��A�C�e���̏��𕶎���̃n���h���ɐݒ肷��
	(CF_TEXT)

******************************************************************************/

static HANDLE Clipboard_Set_TEXT_ItemInfo(struct TPITEM *tpItemInfo)
{
	FARPROC Func_GetItemText;
	HANDLE hMemText;
	char buf[BUFSIZE];
	char *p;
	int ProtocolIndex;

	if(tpItemInfo == NULL){
		return NULL;
	}
	ProtocolIndex = GetProtocolIndex(tpItemInfo->CheckURL);
	if(ProtocolIndex == -1 || tpProtocol[ProtocolIndex].lib == NULL){
		//�v���g�R�������ʂł��Ȃ������ꍇ�́A�`�F�b�N����URL��ݒ�
		if(tpItemInfo->CheckURL == NULL || tpItemInfo->CheckURL[0] == '\0'){
			return NULL;
		}
		if((hMemText = GlobalAlloc(GHND, lstrlen(tpItemInfo->CheckURL) + 1)) == NULL){
			return NULL;
		}
		if((p = GlobalLock(hMemText)) == NULL){
			GlobalFree(hMemText);
			return NULL;
		}
		lstrcpy(p, tpItemInfo->CheckURL);

		GlobalUnlock(hMemText);
		return hMemText;
	}

	//�v���g�R��DLL�̃A�C�e���̕�������擾����֐����Ă�
	wsprintf(buf, "%sGetItemText", tpProtocol[ProtocolIndex].FuncHeader);
	Func_GetItemText = GetProcAddress((HMODULE)tpProtocol[ProtocolIndex].lib, buf);
	if(Func_GetItemText == NULL){
		return NULL;
	}
	return (HANDLE)Func_GetItemText(tpItemInfo);
}


/******************************************************************************

	Clipboard_Set_TEXT

	CF_TEXT�̕�������쐬

******************************************************************************/

HANDLE Clipboard_Set_TEXT(HWND hWnd, HTREEITEM hStrItem)
{
	struct TPITEM *tpItemInfo;
	HTREEITEM hItem;
	int SelectItem;

	if(hStrItem != NULL){
		if(hStrItem == RootItem || hStrItem == RecyclerItem){
			return NULL;
		}
		//�t�H���_�̃p�X��ݒ�
		return Clipboard_Set_TEXT_FolderPath(hWnd, hStrItem);
	}

	//�t�H�[�J�X�����A�C�e���̃C���f�b�N�X���擾
	if((SelectItem = ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), -1, LVNI_FOCUSED)) == -1){
		SelectItem = ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), -1, LVNI_SELECTED);
	}
	if(SelectItem == -1){
		return NULL;
	}

	tpItemInfo = (struct TPITEM *)ListView_GetlParam(GetDlgItem(hWnd, WWWC_LIST), SelectItem);
	if(tpItemInfo == NULL){
		hItem = TreeView_GetIndexToItem(GetDlgItem(hWnd, WWWC_TREE),
			TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)), SelectItem);
		//�t�H���_�̃p�X��ݒ�
		return Clipboard_Set_TEXT_FolderPath(hWnd, hItem);

	}else{
		//�A�C�e���̏���ݒ�
		return Clipboard_Set_TEXT_ItemInfo(tpItemInfo);
	}
	return NULL;
}


/******************************************************************************

	Clipboard_SetTreeData

	�N���b�v�{�[�h�Ƀc���[�A�C�e����ݒ肷��

******************************************************************************/

static BOOL Clipboard_SetTreeData(HWND hWnd, int mode)
{
	HANDLE hMem, hMemText, hMemDrop;
	int ErrCode;

	if(HiTestItem == RootItem || HiTestItem == RecyclerItem){
		return FALSE;
	}

	//�t�H���_���̕ۑ�
	SaveTreeMem(hWnd, GetDlgItem(hWnd, WWWC_TREE), HiTestItem);

	if(mode == FLAG_CUT){
		//�؂���̃}�X�N��ݒ�
		TreeView_SetState(GetDlgItem(hWnd, WWWC_TREE), HiTestItem, TVIS_CUT, TVIS_CUT);
		CutCntFlag = TRUE;
		CutItem = HiTestItem;
	}

	//�N���b�v�{�[�h���J��
	if(OpenClipboard(hWnd) == FALSE){
		ErrMsg(hWnd, GetLastError(), NULL);
		CutItem = NULL;
		return FALSE;
	}
	if(EmptyClipboard() == FALSE){
		ErrCode = GetLastError();
		CloseClipboard();
		ErrMsg(hWnd, ErrCode, NULL);
		CutItem = NULL;
		return FALSE;
	}

	//�t�H���_���
	hMem = Clipboard_Set_WF_ItemList(hWnd, mode, HiTestItem);
	if(hMem == NULL){
		CloseClipboard();
		CutItem = NULL;
		return FALSE;
	}
	//�Ǝ��t�H�[�}�b�g�̐ݒ�
	if(SetClipboardData(WWWC_ClipFormat, hMem) == NULL){
		ErrCode = GetLastError();
		CloseClipboard();
		ErrMsg(hWnd, ErrCode, NULL);
		CutItem = NULL;
		return FALSE;
	}

	//�h���b�v�t�@�C��
	if(DragDrop_CreateTreeDropFiles(hWnd, HiTestItem, Clip_DirName) == FALSE){
		return FALSE;
	}
	if((hMemDrop = DragDrop_SetDropFileMem(hWnd)) != NULL){
		SetClipboardData(CF_HDROP, hMemDrop);
	}
	FreeDropFiles();

	//�t�H���_�̃p�X
	hMemText = Clipboard_Set_TEXT(hWnd, HiTestItem);
	if(hMemText != NULL){
		SetClipboardData(CF_TEXT, hMemText);
	}

	CloseClipboard();
	//�c���[�ł̏����������t���O��ݒ�
	TreeEditFlag = TRUE;
	return TRUE;
}


/******************************************************************************

	Clipboard_SetItemData

	�N���b�v�{�[�h�Ƀf�[�^��ݒ肷��

******************************************************************************/

BOOL Clipboard_SetItemData(HWND hWnd, int mode)
{
	HANDLE hMem, hMemText, hMemDrop;
	int SelectItem;
	int ErrCode;

	Clipboard_DeleteCutStatus(hWnd);
	CutItem = NULL;
	TreeEditFlag = FALSE;

	if(GetFocus() == GetDlgItem(hWnd, WWWC_TREE)){
		//�t�H���_
		return Clipboard_SetTreeData(hWnd, mode);
	}

	//�t�H���_���̕ۑ�
	SaveTreeMem(hWnd, GetDlgItem(hWnd, WWWC_TREE), TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)));

	SelectItem = -1;
	while((SelectItem = ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), SelectItem, LVNI_SELECTED)) != -1){
		if(mode == FLAG_CUT){
			//�؂���}�X�N��ݒ�
			ListView_SetItemState(GetDlgItem(hWnd, WWWC_LIST), SelectItem, LVIS_CUT, LVIS_CUT);
		}
	}
	if(mode == FLAG_CUT){
		//�؂���t���O�̐ݒ�
		CutCntFlag = TRUE;
		CutItem = TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE));
	}

	//�N���b�v�{�[�h���J��
	if(OpenClipboard(hWnd) == FALSE){
		ErrMsg(hWnd, GetLastError(), NULL);
		CutItem = NULL;
		return FALSE;
	}
	if(EmptyClipboard() == FALSE){
		ErrCode = GetLastError();
		CloseClipboard();
		ErrMsg(hWnd, ErrCode, NULL);
		CutItem = NULL;
		return FALSE;
	}

	//�A�C�e�����
	hMem = Clipboard_Set_WF_ItemList(hWnd, mode, NULL);
	if(hMem == NULL){
		CloseClipboard();
		return FALSE;
	}
	if(SetClipboardData(WWWC_ClipFormat, hMem) == NULL){
		ErrCode = GetLastError();
		CloseClipboard();
		ErrMsg(hWnd, ErrCode, NULL);
		return FALSE;
	}

	//�h���b�v�t�@�C��
	if(DragDrop_CreateDropFiles(hWnd, Clip_DirName) == FALSE){
		return FALSE;
	}
	if((hMemDrop = DragDrop_SetDropFileMem(hWnd)) != NULL){
		SetClipboardData(CF_HDROP, hMemDrop);
	}
	FreeDropFiles();

	//�A�C�e�����̕�����̃n���h��
	hMemText = Clipboard_Set_TEXT(hWnd, NULL);
	if(hMemText != NULL){
		SetClipboardData(CF_TEXT, hMemText);
	}

	CloseClipboard();
	return TRUE;
}


/******************************************************************************

	CopyFolder

	�t�H���_�̃R�s�[

******************************************************************************/

static BOOL CopyFolder(HWND hWnd, HTREEITEM hItem, char *FromPath, char *Name, int mode, int DnDFlag)
{
	char Msg[BUFSIZE * 2];
	char Path[BUFSIZE];
	char DirName[BUFSIZE];
	char CopyPath[BUFSIZE];
	char CopyName[BUFSIZE];
	int i;

	TreeView_GetPath(GetDlgItem(hWnd, WWWC_TREE), hItem, Path, CuDir);

	if(*Path == '\0' || *FromPath == '\0'){
		MessageBox(hWnd, EMSG_FOLDERCOPY_ERR, EMSG_FOLDERCOPY_TITLE, MB_ICONSTOP);
		return FALSE;
	}

	//���葤�Ǝ󂯑��̃t�H���_������
	if(lstrcmp(Path, FromPath) == 0){
		if(mode == FLAG_COPY){
			wsprintf(Msg, EMSG_FOLDERCOPY_CUR, Name);
			MessageBox(hWnd, Msg, EMSG_FOLDERCOPY_TITLE, MB_ICONSTOP);
		}
		return FALSE;
	}
	//�󂯑��̃t�H���_�́A���葤�t�H���_�̃T�u�t�H���_
	if(lstrcmpn(Path, FromPath, lstrlen(FromPath)) == 0 &&
		(Path[lstrlen(FromPath)] == '\\' || Path[lstrlen(FromPath)] == '/')){
		wsprintf(Msg, EMSG_FOLDERCOPY_SUB, Name);
		MessageBox(hWnd, Msg, EMSG_FOLDERCOPY_TITLE, MB_ICONSTOP);
		return FALSE;
	}

	lstrcpy(DirName, Name);
	wsprintf(CopyPath, "%s\\%s", Path, DirName);

	if(DnDFlag & DND_RECY || hItem == RecyclerItem){
		//���ɑ��݂���t�H���_���̏ꍇ�͔ԍ���t����
		i = 1;
		lstrcpy(CopyName, DirName);
		while(GetDirSerch(CopyPath) == TRUE){
			wsprintf(CopyName, "%s_%d", DirName, i++);
			wsprintf(CopyPath, "%s\\%s", Path, CopyName);
		}
		lstrcpy(DirName, CopyName);
		lstrcpy(Name, CopyName);
	}
	if(mode == FLAG_COPY){
		//�f�B���N�g���̃R�s�[
		if(CopyDirTree(hWnd, FromPath, Path, DirName, Name) == -1){
			return FALSE;
		}

	}else{
		//�ړ���ƈړ����������ꍇ�͏������s��Ȃ�
		if(lstrcmp(FromPath, CopyPath) == 0){
			return FALSE;
		}

		//���ɑ��݂���f�B���N�g���̏ꍇ�̓R�s�[��폜���s��
		if(GetDirSerch(CopyPath) == TRUE){
			//�f�B���N�g���̃R�s�[
			if(CopyDirTree(hWnd, FromPath, Path, DirName, NULL) == -1){
				return FALSE;
			}
			DeleteDirTree(FromPath, FALSE);
			RemoveDirectory(FromPath);

		}else{
			//�f�B���N�g���̈ړ�
			if(MoveFile(FromPath, CopyPath) == FALSE){
				ErrMsg(hWnd, GetLastError(), EMSG_MOVEDIR_TITLE);
				return FALSE;
			}
		}
	}
	SetDirTree(GetDlgItem(hWnd, WWWC_TREE), Path, hItem);
	SetFolderToUndo(CopyPath, Name);
	return TRUE;
}


/******************************************************************************

	CopyItemInfo

	�A�C�e�����̃R�s�[

******************************************************************************/

static BOOL CopyItemInfo(HWND hWnd, HTREEITEM hItem, struct TPITEM *NewItemInfo, char *SourcePath, int mode)
{
	struct TPTREE *tpTreeInfo;
	char TmpBuf[BUFSIZE];
	char ToPath[BUFSIZE];
	char *p, *r;
	char *TmpTitle;
	int cnt;
	int ret;
	int i;

	//�ړ���ƈړ����������ꍇ
	TreeView_GetPath(GetDlgItem(hWnd, WWWC_TREE), hItem, TmpBuf, CuDir);
	wsprintf(ToPath, "%s\\"DATAFILENAME, TmpBuf);
	if(lstrcmpi(ToPath, SourcePath) == 0){
		//�ړ��̏ꍇ�͏������s��Ȃ�
		if(mode == FLAG_CUT){
			return FALSE;
		}

		//�A�C�e�����X�g�擾
		tpTreeInfo = (struct TPTREE *)TreeView_GetlParam(GetDlgItem(hWnd, WWWC_TREE), hItem);
		if(tpTreeInfo == NULL){
			return FALSE;
		}
		//�A�C�e������ǂݍ���
		if(tpTreeInfo->ItemList == NULL){
			if(ReadTreeMem(hWnd, hItem) == FALSE){
				return FALSE;
			}
		}
		if(NewItemInfo->Title == NULL){
			return TRUE;
		}
		//�A�C�e���̖��O��ύX����
		TmpTitle = (char *)GlobalAlloc(GPTR, lstrlen(NewItemInfo->Title) + lstrlen(STR_COPYNAME) + 1);
		wsprintf(TmpTitle, STR_COPYNAME"%s", NewItemInfo->Title);
		cnt = 1;
		while(1){
			for(i = 0;i < tpTreeInfo->ItemListCnt;i++){
				if(*(tpTreeInfo->ItemList + i) == NULL ||
					(*(tpTreeInfo->ItemList + i))->Title == NULL ||
					lstrcmp((*(tpTreeInfo->ItemList + i))->Title, TmpTitle) != 0){
					continue;
				}

				if(TmpTitle != NULL){
					GlobalFree(TmpTitle);
				}
				if(cnt++ > 100){
					return FALSE;
				}
				//�����A�C�e�������݂���ꍇ�͔ԍ���t���čs��
				wsprintf(TmpBuf, STR_COPYNAME_CNT, cnt);
				TmpTitle = (char *)GlobalAlloc(GPTR, lstrlen(NewItemInfo->Title) + lstrlen(TmpBuf) + 1);
				wsprintf(TmpTitle, "%s%s", TmpBuf, NewItemInfo->Title);
				break;
			}
			if(i >= tpTreeInfo->ItemListCnt){
				break;
			}
		}
		GlobalFree(NewItemInfo->Title);
		NewItemInfo->Title = TmpTitle;
	}

	//�A�C�e���̒ǉ�
	ret = Item_Add(hWnd, hItem, NewItemInfo);
	if(ret == -1){
		return FALSE;
	}

	SetItemToUndo(NewItemInfo);

	//�R�s�[�̏ꍇ�̓A�C�e���폜���s��Ȃ�
	if(mode == FLAG_COPY || *SourcePath == '\0'){
		return TRUE;
	}

	//�ړ����ꂽ�A�C�e���̍폜
	if(tpDeleteItemList == NULL){
		return TRUE;
	}
	for(i = 0;i < tpDeleteItemListCnt;i++){
		if(*(tpDeleteItemList + i) == NULL){
			continue;
		}
		//�^�C�g���̔�r
		p = ((*(tpDeleteItemList + i))->Title == NULL) ? "" : (*(tpDeleteItemList + i))->Title;
		r = (NewItemInfo->Title == NULL) ? "" : NewItemInfo->Title;
		if(lstrcmp(p, r) != 0){
			continue;
		}
		//URL�̔�r
		p = ((*(tpDeleteItemList + i))->CheckURL == NULL) ? "" : (*(tpDeleteItemList + i))->CheckURL;
		r = (NewItemInfo->CheckURL == NULL) ? "" : NewItemInfo->CheckURL;
		if(lstrcmp(p, r) != 0){
			continue;
		}

		//�A�C�e���̍폜
		FreeItemInfo(*(tpDeleteItemList + i), TRUE);
		GlobalFree(*(tpDeleteItemList + i));
		*(tpDeleteItemList + i) = NULL;
		break;
	}
	return TRUE;
}


/******************************************************************************

	StringToItem

	�A�C�e����񕶎��񂩂�A�C�e�������쐬����

******************************************************************************/

static int StringToItem(HWND hWnd, HTREEITEM hItem, char *Data, int DnDFlag)
{
	struct TPITEM *tpItemInfo;
	char buf[MAXSIZE];
	char FromPath[BUFSIZE];
	char SourcePath[BUFSIZE];
	char Name[BUFSIZE];
	char TmpBuf[BUFSIZE];
	char ToPath[BUFSIZE];
	char *p, *r, *t;
	int ret = FLAG_COPY;
	int ListFolderCnt;
	int TreeFolderCnt;
	int i;
	BOOL SelInitFlag = FALSE;

	*SourcePath = '\0';
	tpDeleteItemList = NULL;

	//�ړ���̃p�X
	TreeView_GetPath(GetDlgItem(hWnd, WWWC_TREE), hItem, TmpBuf, CuDir);
	wsprintf(ToPath, "%s\\"DATAFILENAME, TmpBuf);

	r = buf;
	for(p = Data;*p != '\0';p++){
		if(*p != '\r'){
			if(*p != '\n'){
				*(r++) = *p;
			}
			continue;
		}
		*r = '\0';
		if(*buf == '\0'){
			r = buf;
			continue;
		}

		if(*buf == FLAG_COPY || *buf == FLAG_CUT){
			//�ҏW�t���O + �ړ����t�@�C��
			t = buf + 1;
			//�p�X
			r = SourcePath;
			for(;*t != '\t' && *t != '\0';t++, r++){
				*r = *t;
			}
			*r = '\0';

			if(*buf == FLAG_COPY || DnDFlag & DND_COPY){
				//*SourcePath = '\0';
			}else{
				//�ړ����ɍ폜����A�C�e�����X�g
				tpDeleteItemList = ReadItemList(SourcePath, &tpDeleteItemListCnt, NULL);
				ret = FLAG_CUT;
			}

			//Undo�̏�����
			InitUndo(hWnd, SourcePath, hItem, ret);

		}else if(*buf == FLAG_FOLDER){
			//�t�H���_�t���O + �ړ����f�B���N�g�� + �t�H���_��
			t = buf + 1;

			r = FromPath;
			for(;*t != '\t' && *t != '\0';t++, r++){
				*r = *t;
			}
			*r = '\0';
			if(*t == '\t'){
				t++;
			}
			//���O
			r = Name;
			for(;*t != '\0';t++, r++){
				*r = *t;
			}
			*r = '\0';

			//�R�s�[�A�؂���
			if(CopyFolder(hWnd, hItem, FromPath, Name, ret, DnDFlag) == FALSE){
				ret = -1;
				break;
			}
			if(hItem == TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE))){
				//�I���̉���
				if(SelInitFlag == FALSE){
					ListView_SetItemState(GetDlgItem(hWnd, WWWC_LIST), -1, 0, LVIS_SELECTED);
					SelInitFlag = TRUE;
				}
				//���X�g�r���[�ɕ\������Ă���t�H���_�̐�
				ListFolderCnt = ListView_GetFolderCount(GetDlgItem(hWnd, WWWC_LIST));
				//�c���[�r���[�ɕ\������Ă���t�H���_�̐�
				TreeFolderCnt = TreeView_GetChildCount(GetDlgItem(hWnd, WWWC_TREE), hItem);

				if(ListFolderCnt < TreeFolderCnt){
					ListView_InsertItemEx(GetDlgItem(hWnd, WWWC_LIST), (char*)LPSTR_TEXTCALLBACK, I_IMAGECALLBACK,
						(long)NULL, ListFolderCnt);
				}

				i = TreeView_GetListIndex(GetDlgItem(hWnd, WWWC_TREE),
					TreeView_CheckName(GetDlgItem(hWnd, WWWC_TREE), hItem, Name));
				if(i != -1){
					ListView_SetItemState(GetDlgItem(hWnd, WWWC_LIST), i, LVIS_SELECTED, LVIS_SELECTED);
				}
			}

			if(UpdateItemFlag == UF_CANCEL){
				break;
			}

		}else{
			//�I���̉���
			if(SelInitFlag == FALSE && (ret == FLAG_COPY || lstrcmpi(ToPath, SourcePath) != 0)){
				ListView_SetItemState(GetDlgItem(hWnd, WWWC_LIST), -1, 0, LVIS_SELECTED);
				SelInitFlag = TRUE;
			}

			//�A�C�e�����
			tpItemInfo = (struct TPITEM *)GlobalAlloc(GPTR, sizeof(struct TPITEM));
			if(tpItemInfo != NULL){
				tpItemInfo->iSize = sizeof(struct TPITEM);
				tpItemInfo->hItem = hItem;
				LineSetItemInfo(tpItemInfo, buf);

				if(CopyItemInfo(hWnd, hItem, tpItemInfo, SourcePath, ret) == FALSE){
					FreeItemInfo(tpItemInfo, FALSE);
					GlobalFree(tpItemInfo);
					if(UpdateItemFlag == UF_CANCEL){
						break;
					}
				}
			}
		}
		r = buf;
	}
	if(tpDeleteItemList != NULL){
		SaveItemList(hWnd, SourcePath, tpDeleteItemList, tpDeleteItemListCnt);
		FreeItemList(tpDeleteItemList, tpDeleteItemListCnt, FALSE);
		GlobalFree(tpDeleteItemList);
	}
	return ret;
}


/******************************************************************************

	Clipboard_Get_WF_String

	�A�C�e����񕶎������������

******************************************************************************/

int Clipboard_Get_WF_String(HWND hWnd, HTREEITEM hItem, char *buf, int DnDFlag)
{
	int ret;

	ListView_SetRedraw(GetDlgItem(hWnd, WWWC_LIST), LDF_NODRAW);
	SendDlgItemMessage(hWnd, WWWC_TREE, WM_SETREDRAW, (WPARAM)FALSE, 0);

	//�A�C�e����ǉ�
	ret = StringToItem(hWnd, hItem, buf, DnDFlag);
	TreeView_FreeItem(hWnd, hItem, 1);

	//�ǉ����ꂽ�t�H���_�����X�g�r���[�ɔ��f
	ListView_FolderRedraw(hWnd, FALSE);

	//�I���̐擪�A�C�e���Ƀt�H�[�J�X��^����
	if(ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), -1, LVNI_SELECTED) != -1){
		ListView_SetItemState(GetDlgItem(hWnd, WWWC_LIST),
			ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), -1, LVNI_SELECTED),
			LVIS_SELECTED | LVIS_FOCUSED, LVIS_FOCUSED | LVIS_SELECTED);
		ListView_EnsureVisible(GetDlgItem(hWnd, WWWC_LIST),
			ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), -1, LVNI_SELECTED), TRUE);
	}

	ListView_SetRedraw(GetDlgItem(hWnd, WWWC_LIST), LDF_REDRAW);

	SendDlgItemMessage(hWnd, WWWC_TREE, WM_SETREDRAW, (WPARAM)TRUE, 0);
	UpdateWindow(GetDlgItem(hWnd, WWWC_TREE));
	SetProtocolMenu(hWnd);
	return ret;
}


/******************************************************************************

	Clipboard_CheckFormat

	�N���b�v�{�[�h�t�H�[�}�b�g�̃`�F�b�N

******************************************************************************/

int Clipboard_CheckFormat(void)
{
	UINT ClipFormat[CLIPFORMAT_CNT];

	ClipFormat[0] = WWWC_ClipFormat;
	ClipFormat[1] = CF_HDROP;
	ClipFormat[2] = CF_TEXT;
	return GetPriorityClipboardFormat(ClipFormat, CLIPFORMAT_CNT);
}


/******************************************************************************

	Clipboard_GetData

	�N���b�v�{�[�h����f�[�^���擾����������

******************************************************************************/

void Clipboard_GetData(HWND hWnd)
{
	HTREEITEM hItem;
	HANDLE hClip;
	char *buf;
	int ret;

	//�N���b�v�{�[�h�t�H�[�}�b�g�̃`�F�b�N
	ret = Clipboard_CheckFormat();
	if(ret <= 0){
		return;
	}
	//�N���b�v�{�[�h���J��
	if(OpenClipboard(hWnd) == FALSE){
		return;
	}

	switch(ret)
	{
	case CF_TEXT:		//�e�L�X�g
		hClip = GetClipboardData(CF_TEXT);
		if((buf = GlobalLock(hClip)) == NULL){
			break;
		}
		//�v���g�R�������ʂł����ꍇ�̓A�C�e���Ƃ��Ēǉ�
		if(GetProtocolIndex(buf) != -1){
			SetFocus(GetDlgItem(hWnd, WWWC_LIST));
			Item_UrlAdd(hWnd, NEWITEMNAME, buf, 0, NULL);
		}
		GlobalUnlock(hClip);
		break;

	case CF_HDROP:		//�h���b�v�t�@�C��
		hClip = GetClipboardData(CF_HDROP);
		DragDrop_GetDropFiles(hWnd, hClip, TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)));
		break;

	default:			//�Ǝ��`��
		hItem = TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE));

		hClip = GetClipboardData(WWWC_ClipFormat);
		if((buf = GlobalLock(hClip)) == NULL){
			break;
		}
		SetFocus(GetDlgItem(hWnd, WWWC_LIST));
		WaitCursor(TRUE);

		UpdateItemFlag = UF_COPY;
		//�A�C�e����ǉ�
		ret = Clipboard_Get_WF_String(hWnd, hItem, buf, 0);
		GlobalUnlock(hClip);

		if(ret == FLAG_CUT){
			EmptyClipboard();
		}

		TreeView_SetIconState(hWnd, TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)), 0);
		WaitCursor(FALSE);
		break;
	}
	CloseClipboard();
}
/* End of source */
