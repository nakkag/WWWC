/**************************************************************************

	WWWC

	undo.c

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

struct TPUNDO {
	int UndoType;
	int UndoCnt;
	HTREEITEM FromItem;
	HTREEITEM ToItem;
	char *UndoBuf;
	char *OldTitle;
	char *NewTitle;
	char *UndoURL;
};

struct TPUNDO CurUndo;		//Undo�o�b�t�@
struct TPUNDO OldUndo;		//�Ҕ�p

//�O���Q��
extern char CuDir[];
extern HTREEITEM RootItem;
extern HTREEITEM RecyclerItem;
extern int UpdateItemFlag;


/**************************************************************************
	Local Function Prototypes
**************************************************************************/


/******************************************************************************

	FreeUndoinfo

	Undo���̉��

******************************************************************************/

static void FreeUndoinfo(struct TPUNDO *undoinfo)
{
	//UNDO�p�o�b�t�@�̉��
	if(undoinfo->UndoBuf != NULL){
		GlobalFree(undoinfo->UndoBuf);
	}
	if(undoinfo->OldTitle != NULL){
		GlobalFree(undoinfo->OldTitle);
	}
	if(undoinfo->NewTitle != NULL){
		GlobalFree(undoinfo->NewTitle);
	}
	if(undoinfo->UndoURL != NULL){
		GlobalFree(undoinfo->UndoURL);
	}
	ZeroMemory(undoinfo, sizeof(struct TPUNDO));
}


/******************************************************************************

	FreeUndo

	Undo�o�b�t�@�̉��

******************************************************************************/

void FreeUndo(void)
{
	FreeUndoinfo(&OldUndo);
	FreeUndoinfo(&CurUndo);
}


/******************************************************************************

	CheckUndo

	Undo���\���`�F�b�N

******************************************************************************/

int CheckUndo(void)
{
	struct TPUNDO *undoinfo = &CurUndo;

	if(undoinfo->FromItem == NULL || undoinfo->UndoType == 0){
		return 0;
	}
	if(undoinfo->UndoType == UNDO_ITEM && undoinfo->UndoCnt == 0){
		undoinfo = &OldUndo;
	}
	if(undoinfo->UndoType == UNDO_ITEM){
		if(undoinfo->FromItem == RecyclerItem){
			return UNDO_ITEM_COPY;
		}
		if(undoinfo->ToItem == RecyclerItem){
			return UNDO_ITEM_DELETE;
		}
	}
	return undoinfo->UndoType;
}


/******************************************************************************

	InitUndo

	Undo�̏�����

******************************************************************************/

void InitUndo(HWND hWnd, char *path, HTREEITEM hItem, int mode)
{
	char buf[BUFSIZE];
	char tmp[BUFSIZE];
	char *p, *r;
	int len;

	if(CurUndo.UndoType != 0 && !(CurUndo.UndoType == UNDO_ITEM && CurUndo.UndoCnt == 0)){
		FreeUndoinfo(&OldUndo);
		//UNDO���̃R�s�[
		CopyMemory(&OldUndo, &CurUndo, sizeof(struct TPUNDO));
		ZeroMemory(&CurUndo, sizeof(struct TPUNDO));
	}else{
		FreeUndoinfo(&CurUndo);
	}

	if(mode == FLAG_COPY){
		//�R�s�[�̏ꍇ�̓S�~���Ɉړ�
		mode = FLAG_CUT;
		CurUndo.FromItem = RecyclerItem;
	}else{
		//�ړ����̃p�X����TreeView�̃A�C�e��������
		if(lstrcmpni(path, CuDir, lstrlen(CuDir)) != 0){
			return;
		}

		wsprintf(buf, "\\\\Internet%s", path + lstrlen(CuDir));
		p = r = buf;
		while(*p != '\0'){
			if(IsDBCSLeadByte((BYTE)*p) == TRUE){
				p++;
			}else if(*p == '\\' || *p == '/'){
				r = p;
			}
			p++;
		}
		*r = '\0';
		CurUndo.FromItem = TreeView_FindItemPath(GetDlgItem(hWnd, WWWC_TREE), RootItem, buf);
	}
	CurUndo.ToItem = hItem;

	//UNDO�p�o�b�t�@�̍쐬
	TreeView_GetPath(GetDlgItem(hWnd, WWWC_TREE), hItem, buf, CuDir);
	wsprintf(tmp, "%s\\"DATAFILENAME, buf);
	len = lstrlen(tmp) + 3;			//mode + Path + '\r' + '\n'

	CurUndo.UndoBuf = (char *)GlobalAlloc(GPTR, len + 1);
	if(CurUndo.UndoBuf == NULL){
		return;
	}
	wsprintf(CurUndo.UndoBuf, "%c%s\r\n", mode, tmp);
	CurUndo.UndoType = UNDO_ITEM;
}


/******************************************************************************

	SetFolderToUndo

	Undo�o�b�t�@�Ƀt�H���_����ǉ�

******************************************************************************/

BOOL SetFolderToUndo(char *path, char *name)
{
	char *TmpBuf, *p;
	int len = 0;

	if(CurUndo.FromItem == NULL){
		CurUndo.UndoType = 0;
		return FALSE;
	}
	if(CurUndo.UndoBuf != NULL){
		len = lstrlen(CurUndo.UndoBuf);
	}
	len += 1 + lstrlen(path) + 1 + lstrlen(name) + 2;

	TmpBuf = (char *)GlobalAlloc(GPTR, len + 1);
	if(TmpBuf == NULL){
		CurUndo.UndoType = 0;
		return FALSE;
	}

	p = TmpBuf;
	if(CurUndo.UndoBuf != NULL){
		p = iStrCpy(p, CurUndo.UndoBuf);
	}

	//�t�H���_�t���O
	*(p++) = FLAG_FOLDER;
	//�p�X
	p = iStrCpy(p, path);
	*(p++) = '\t';
	p = iStrCpy(p, name);
	p = iStrCpy(p, "\r\n");
	*p = '\0';

	if(CurUndo.UndoBuf != NULL){
		GlobalFree(CurUndo.UndoBuf);
	}
	CurUndo.UndoBuf = TmpBuf;
	CurUndo.UndoCnt++;
	return TRUE;
}


/******************************************************************************

	SetItemToUndo

	Undo�o�b�t�@�ɃA�C�e������ǉ�

******************************************************************************/

BOOL SetItemToUndo(struct TPITEM *tpItemInfo)
{
	char *TmpBuf, *p;
	int len = 0;

	if(CurUndo.FromItem == NULL){
		return FALSE;
	}
	if(CurUndo.UndoBuf != NULL){
		len = lstrlen(CurUndo.UndoBuf);
	}
	len += ItemSize(tpItemInfo);

	TmpBuf = (char *)GlobalAlloc(GPTR, len + 1);
	if(TmpBuf == NULL){
		CurUndo.UndoType = 0;
		return FALSE;
	}

	p = TmpBuf;
	if(CurUndo.UndoBuf != NULL){
		p = iStrCpy(p, CurUndo.UndoBuf);
	}
	p = SetItemString(tpItemInfo, p);
	*p = '\0';

	if(CurUndo.UndoBuf != NULL){
		GlobalFree(CurUndo.UndoBuf);
	}
	CurUndo.UndoBuf = TmpBuf;
	CurUndo.UndoCnt++;
	return TRUE;
}


/******************************************************************************

	SetLVTitleToUndo

	Undo�o�b�t�@��ListView�̃^�C�g���ύX���Z�b�g

******************************************************************************/

BOOL SetLVTitleToUndo(HTREEITEM hItem, char *Old, char *New, char *URL)
{
	FreeUndo();

	CurUndo.OldTitle = AllocCopy(Old);
	CurUndo.NewTitle = AllocCopy(New);
	CurUndo.UndoURL = AllocCopy(URL);
	if(CurUndo.OldTitle == NULL || CurUndo.NewTitle == NULL || CurUndo.UndoURL == NULL){
		FreeUndoinfo(&CurUndo);
		return FALSE;
	}
	CurUndo.FromItem = hItem;
	CurUndo.UndoType = UNDO_LV_TITLE;
	return TRUE;
}


/******************************************************************************

	SetTVTitleToUndo

	Undo�o�b�t�@��TreeView�̃^�C�g���ύX���Z�b�g

******************************************************************************/

BOOL SetTVTitleToUndo(HTREEITEM hItem, char *Old)
{
	FreeUndo();

	CurUndo.OldTitle = AllocCopy(Old);
	if(CurUndo.OldTitle == NULL){
		return FALSE;
	}
	CurUndo.FromItem = hItem;
	CurUndo.UndoType = UNDO_TV_TITLE;
	return TRUE;
}


/******************************************************************************

	ExecUndo

	Undo�����s

******************************************************************************/

BOOL ExecUndo(HWND hWnd)
{
	struct TPTREE *tpTreeInfo;
	HTREEITEM TmpToItem, TmpFromItem;
	HANDLE hMem;
	char *buf;
	int i;

	if(CurUndo.FromItem == NULL){
		return FALSE;
	}

	switch(CurUndo.UndoType)
	{
	case UNDO_ITEM:
		if(CurUndo.UndoCnt == 0){
			if(OldUndo.UndoType == 0){
				return FALSE;
			}
			FreeUndoinfo(&CurUndo);
			//UNDO���̃R�s�[
			CopyMemory(&CurUndo, &OldUndo, sizeof(struct TPUNDO));
			ZeroMemory(&OldUndo, sizeof(struct TPUNDO));
			return ExecUndo(hWnd);
		}
		if(CurUndo.UndoBuf == NULL){
			return FALSE;
		}

		if(CurUndo.FromItem == RecyclerItem){
			//�폜�m�F
			if(MessageBox(hWnd, QMSG_ITEMDELETE_UNDO, QMSG_DELETE_TITLE, MB_ICONEXCLAMATION | MB_YESNO) == IDNO){
				return FALSE;
			}
		}
		TmpToItem = CurUndo.ToItem;
		TmpFromItem = CurUndo.FromItem;

		SaveTreeMem(hWnd, GetDlgItem(hWnd, WWWC_TREE), TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)));

		if((hMem = GlobalAlloc(GHND, lstrlen(CurUndo.UndoBuf) + 1)) == NULL){
			ErrMsg(hWnd, GetLastError(), NULL);
			return FALSE;
		}
		if((buf = GlobalLock(hMem)) == NULL){
			ErrMsg(hWnd, GetLastError(), NULL);
			GlobalFree(hMem);
			return FALSE;
		}
		lstrcpy(buf, CurUndo.UndoBuf);
		GlobalUnlock(hMem);

		UpdateItemFlag = (TmpFromItem == RecyclerItem) ? UF_NOMSG : UF_COPY;
		if(DragDrop_GetDropItem(hWnd, hMem, DND_MOVE, TmpFromItem) != FALSE){
			TreeView_NoFileDelete(hWnd, TmpToItem, 0);
			ListView_RefreshFolder(hWnd);
			Item_Select(hWnd, TmpToItem);
		}
		GlobalFree(hMem);

		TreeView_SetIconState(hWnd, TmpToItem, 0);
		TreeView_SetIconState(hWnd, TmpFromItem, 0);

		SetFocus(GetDlgItem(hWnd, WWWC_LIST));
		break;

	case UNDO_LV_TITLE:
		if(CurUndo.OldTitle == NULL || CurUndo.NewTitle == NULL || CurUndo.UndoURL == NULL){
			return FALSE;
		}
		tpTreeInfo = (struct TPTREE *)TreeView_GetlParam(GetDlgItem(hWnd, WWWC_TREE), CurUndo.FromItem);
		if(tpTreeInfo == NULL){
			return FALSE;
		}
		if(tpTreeInfo->ItemList == NULL){
			if(ReadTreeMem(hWnd, CurUndo.FromItem) == FALSE){
				return FALSE;
			}
		}

		for(i = 0;i < tpTreeInfo->ItemListCnt;i++){
			if((*(tpTreeInfo->ItemList + i)) == NULL){
				continue;
			}
			if(lstrcmp((*(tpTreeInfo->ItemList + i))->Title, CurUndo.NewTitle) == 0 &&
				lstrcmp((*(tpTreeInfo->ItemList + i))->CheckURL, CurUndo.UndoURL) == 0){
				GlobalFree((*(tpTreeInfo->ItemList + i))->Title);
				(*(tpTreeInfo->ItemList + i))->Title = AllocCopy(CurUndo.OldTitle);
				if((*(tpTreeInfo->ItemList + i))->Title == NULL){
					ErrMsg(hWnd, GetLastError(), NULL);
					return FALSE;
				}
				(*(tpTreeInfo->ItemList + i))->RefreshFlag = TRUE;
				buf = CurUndo.NewTitle;
				CurUndo.NewTitle = CurUndo.OldTitle;
				CurUndo.OldTitle = buf;
				break;
			}
		}
		if(CurUndo.FromItem == TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE))){
			ListView_RefreshItem(GetDlgItem(hWnd, WWWC_LIST));
		}
		TreeView_FreeItem(hWnd, CurUndo.FromItem, 1);
		break;

	case UNDO_TV_TITLE:
		if(CurUndo.OldTitle == NULL){
			return FALSE;
		}
		TreeView_SetName(hWnd, CurUndo.FromItem, CurUndo.OldTitle);
		ListView_RefreshFolder(hWnd);
		break;

	default:
		return FALSE;
	}
	return TRUE;
}
/* End of source */
