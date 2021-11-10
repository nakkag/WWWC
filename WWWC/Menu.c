/**************************************************************************

	WWWC

	Menu.c

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

#define WKMENU_CNT						2

#define NEWMENU_INDEX					2

#define MENUFLAG_FOLDER					-2
#define MENUFLAG_RECYCLER				-3
#define MENUFLAG_TREEFOLDER				-4
#define MENUFLAG_TREERECYCLER			-5
#define MENUFLAG_NON					-6


/**************************************************************************
	Global Variables
**************************************************************************/

struct TPWINMENU{
	HMENU Menu;
	int OldProtocolIndex;
};
static struct TPWINMENU tpWinMenu[WKMENU_CNT];

//�O���Q��
extern struct TP_PROTOCOL *tpProtocol;
extern int ProtocolCnt;
extern struct TP_TOOLS *ToolList;
extern int ToolListCnt;

extern HTREEITEM RootItem;
extern HTREEITEM RecyclerItem;

extern int gCheckFlag;
extern HMENU hPOPUP;
extern HWND TbWnd;


/******************************************************************************

	ShowMenu

	�}�E�X�̈ʒu�Ƀ��j���[��\������

******************************************************************************/

void ShowMenu(HWND hWnd, HMENU hMenu, int mpos)
{
	HMENU hShowMenu;
	POINT apos;

	_SetForegroundWindow(hWnd);

	//�}�E�X�ʒu�Ɏw�胁�j���[��\��
	GetCursorPos((LPPOINT)&apos);
	hShowMenu = GetSubMenu(hMenu, mpos);
	TrackPopupMenu(hShowMenu, TPM_TOPALIGN | TPM_RIGHTBUTTON | TPM_LEFTBUTTON,
		apos.x, apos.y, 0, hWnd, NULL);

	PostMessage(hWnd, WM_NULL, 0, 0);
}


/******************************************************************************

	ShowMenuCommand

	�}�E�X�̈ʒu�Ƀ��j���[��\������

******************************************************************************/

UINT ShowMenuCommand(HWND hWnd, HMENU hMenu, int mpos)
{
	HMENU hShowMenu;
	POINT apos;
	UINT ret;

	_SetForegroundWindow(hWnd);

	//�}�E�X�ʒu�Ɏw�胁�j���[��\��
	GetCursorPos((LPPOINT)&apos);
	hShowMenu = GetSubMenu(hMenu, mpos);
	ret = (UINT)TrackPopupMenu(hShowMenu, TPM_TOPALIGN | TPM_RIGHTBUTTON | TPM_LEFTBUTTON | TPM_RETURNCMD,
		apos.x, apos.y, 0, hWnd, NULL);

	PostMessage(hWnd, WM_NULL, 0, 0);
	return ret;
}


/******************************************************************************

	SetFolderEnableMenu

	�t�H���_�̃��j���[���ڂ̎g�p�\�^�g�p�s�\��Ԃ�ݒ肷��

******************************************************************************/

void SetFolderEnableMenu(HWND hWnd, HTREEITEM hItem)
{
	struct TPTREE *tpTreeInfo;
	int Root, Recy;
	int CheckNoCheck;
	int CheckFolder = 0;
	int PropFolder = 0;

	//���[�g�A�C�e��
	Root = (hItem == RootItem) ? 1 : 0;
	//���ݔ�
	Recy = (hItem == RecyclerItem) ? 1 : 0;

	//�`�F�b�N
	tpTreeInfo = (struct TPTREE *)TreeView_GetlParam(GetDlgItem(hWnd, WWWC_TREE), hItem);
	CheckNoCheck = (tpTreeInfo != NULL && tpTreeInfo->CheckFlag != 0) ? 1 : 0;
	CheckFolder = FindCheckItem(hWnd, hItem);
	PropFolder = FindPropItem(hWnd, hItem);

	EnableMenuItem(GetSubMenu(hPOPUP, MENU_POP_ITEM), ID_MENUITEM_CHECKEND_POP, !CheckNoCheck);
	EnableMenuItem(GetSubMenu(hPOPUP, MENU_POP_ITEM), ID_MENUITEM_INITICON_POP, CheckNoCheck);

	//�ҏW
	EnableMenuItem(GetSubMenu(hPOPUP, MENU_POP_ITEM), ID_MENUITEM_CUT_POP, Root || Recy);
	EnableMenuItem(GetSubMenu(hPOPUP, MENU_POP_ITEM), ID_MENUITEM_COPY_POP, Root || Recy);
	EnableMenuItem(GetSubMenu(hPOPUP, MENU_POP_ITEM), ID_MENUITEM_DELETE_POP,
		Root || Recy || CheckFolder || PropFolder);
	EnableMenuItem(GetSubMenu(hPOPUP, MENU_POP_ITEM), ID_MENUITEM_EDITNAME_POP, Root || Recy);

	EnableMenuItem(GetSubMenu(hPOPUP, MENU_POP_ITEM), ID_MENUITEM_CHECKSWITCH_POP, Recy);
}


/******************************************************************************

	SetEnableMenu

	���j���[���ڂƃc�[���o�[�̎g�p�\�^�g�p�s�\��Ԃ�ݒ肷��

******************************************************************************/

void SetEnableMenu(HWND hWnd)
{
	struct TPTREE *tpTreeInfo;
	struct TPITEM *tpItemInfo;
	HTREEITEM hItem;
	char *UndoTitle;
	int SelectItem;
	int Root, Recy, CheckNoCheck, Check, CheckFolder, Tree, List, SelItem, CheckItem, Item, Undo;
	int HNext, HPrev;
	int CheckMenuFlag;
	static int UndoType = -1;

	//���[�g�A�C�e���A���ݔ�
	hItem = TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE));
	Root = (hItem == RootItem || hItem == RecyclerItem) ? 1 : 0;

	//�`�F�b�N
	tpTreeInfo = (struct TPTREE *)TreeView_GetlParam(GetDlgItem(hWnd, WWWC_TREE), hItem);
	CheckNoCheck = (tpTreeInfo != NULL && tpTreeInfo->CheckFlag != 0) ? 1 : 0;
	Check = gCheckFlag;
	CheckFolder = FindCheckItem(hWnd, hItem);

	//TreeView��Focus������ꍇ
	Tree = (GetFocus() == GetDlgItem(hWnd, WWWC_TREE)) ? 1 : 0;
	//ListView��Focus������ꍇ
	List = (GetFocus() == GetDlgItem(hWnd, WWWC_LIST)) ? 1 : 0;

	SelItem = 0;
	Item = 0;
	CheckItem = 0;
	Recy = 0;
	CheckFolder = 0;

	if(UndoType != CheckUndo()){
		UndoType = CheckUndo();
		switch(UndoType)
		{
		case UNDO_ITEM:
			UndoTitle = MENU_STR_UNDO_MOVE;
			break;

		case UNDO_ITEM_COPY:
			UndoTitle = MENU_STR_UNDO_COPY;
			break;

		case UNDO_ITEM_DELETE:
			UndoTitle = MENU_STR_UNDO_DELETE;
			break;

		case UNDO_LV_TITLE:
		case UNDO_TV_TITLE:
			UndoTitle = MENU_STR_UNDO_NAME;
			break;

		default:
			Undo = 0;
			UndoTitle = MENU_STR_UNDO_NON;
			break;
		}
		ModifyMenu(GetSubMenu(GetMenu(hWnd), MENU_HWND_EDIT), ID_MENUITEM_UNDO, MF_BYCOMMAND | MF_STRING, ID_MENUITEM_UNDO, UndoTitle);
		ModifyMenu(GetSubMenu(hPOPUP, MENU_POP_FOLDER), ID_MENUITEM_UNDO, MF_BYCOMMAND | MF_STRING, ID_MENUITEM_UNDO, UndoTitle);
	}
	Undo = (UndoType == 0) ? 0 : 1;

	HNext = TreeView_CheckHistory(hWnd, TRUE);
	HPrev = TreeView_CheckHistory(hWnd, FALSE);

	//�I���A�C�e���̏��
	if(List == 1 && ListView_GetSelectedCount(GetDlgItem(hWnd, WWWC_LIST)) > 0){
		SelItem = 1;

		SelectItem = -1;
		while((SelectItem = ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), SelectItem, LVNI_SELECTED)) != -1){
			tpItemInfo = (struct TPITEM *)ListView_GetlParam(GetDlgItem(hWnd, WWWC_LIST), SelectItem);
			if(tpItemInfo != NULL){
				Item = 1;
				if(tpItemInfo->IconStatus != ST_DEFAULT){
					CheckItem = 1;
					break;
				}
			}else{
				hItem = TreeView_GetIndexToItem(GetDlgItem(hWnd, WWWC_TREE),
					TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)), SelectItem);
				//���ݔ�
				if(hItem == RecyclerItem){
					Recy = 1;
				}
				tpTreeInfo = (struct TPTREE *)TreeView_GetlParam(GetDlgItem(hWnd, WWWC_TREE), hItem);
				if(tpTreeInfo != NULL && tpTreeInfo->CheckFlag != 0){
					CheckItem = 1;
					break;
				}
			}
		}
	}

	CheckMenuFlag = ((Tree && CheckNoCheck) || (List && CheckItem) || (List && !SelItem && CheckNoCheck));

	//�E�B���h�E���j���[
	//���̊K�w
	if(TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)) == RootItem){
		SendMessage(TbWnd, TB_ENABLEBUTTON, ID_KEY_UPDIR, (LPARAM) MAKELONG(0, 0));
	}else{
		SendMessage(TbWnd, TB_ENABLEBUTTON, ID_KEY_UPDIR, (LPARAM) MAKELONG(1, 0));
	}

	//�ҏW
	EnableMenuItem(GetSubMenu(GetMenu(hWnd), MENU_HWND_FILE), ID_MENUITEM_DELETE,
		!((Tree && !Root && !CheckFolder) || (List && SelItem && !Recy)));
	SendMessage(TbWnd, TB_ENABLEBUTTON, ID_MENUITEM_DELETE,
		(LPARAM)MAKELONG(((Tree && !Root && !CheckFolder) || (List && SelItem && !Recy)), 0));
	EnableMenuItem(GetSubMenu(GetMenu(hWnd), MENU_HWND_FILE), ID_MENUITEM_EDITNAME,
		!((Tree && !Root) || (List && SelItem && !Recy)));

	EnableMenuItem(GetSubMenu(GetMenu(hWnd), MENU_HWND_EDIT), ID_MENUITEM_UNDO, !Undo);
	SendMessage(TbWnd, TB_ENABLEBUTTON, ID_MENUITEM_UNDO,
		(LPARAM)MAKELONG(Undo, 0));
	EnableMenuItem(GetSubMenu(GetMenu(hWnd), MENU_HWND_EDIT), ID_MENUITEM_CUT,
		!((Tree && !Root) || (List && SelItem)));
	SendMessage(TbWnd, TB_ENABLEBUTTON, ID_MENUITEM_CUT,
		(LPARAM)MAKELONG(((Tree && !Root) || (List && SelItem)), 0));
	EnableMenuItem(GetSubMenu(GetMenu(hWnd), MENU_HWND_EDIT), ID_MENUITEM_COPY,
		!((Tree && !Root) || (List && SelItem)));
	SendMessage(TbWnd, TB_ENABLEBUTTON, ID_MENUITEM_COPY,
		(LPARAM)MAKELONG(((Tree && !Root) || (List && SelItem)), 0));

	//�ړ�
	EnableMenuItem(GetSubMenu(GetMenu(hWnd), MENU_HWND_VIEW), ID_MENUITEM_PREV_HISTORY, !HPrev);
	SendMessage(TbWnd, TB_ENABLEBUTTON, ID_MENUITEM_PREV_HISTORY,
		(LPARAM)MAKELONG(HPrev, 0));
	EnableMenuItem(GetSubMenu(GetMenu(hWnd), MENU_HWND_VIEW), ID_MENUITEM_NEXT_HISTORY, !HNext);
	SendMessage(TbWnd, TB_ENABLEBUTTON, ID_MENUITEM_NEXT_HISTORY,
		(LPARAM)MAKELONG(HNext, 0));

	//�`�F�b�N
	EnableMenuItem(GetSubMenu(GetMenu(hWnd), MENU_HWND_CHECK), ID_MENUITEM_CHECKEND, !CheckMenuFlag);
	SendMessage(TbWnd, TB_ENABLEBUTTON, ID_MENUITEM_CHECKEND, (LPARAM) MAKELONG(CheckMenuFlag, 0));
	EnableMenuItem(GetSubMenu(GetMenu(hWnd), MENU_HWND_CHECK), ID_MENUITEM_ALLCHECKEND, !Check);
	SendMessage(TbWnd, TB_ENABLEBUTTON, ID_MENUITEM_ALLCHECKEND, (LPARAM) MAKELONG(Check, 0));
	EnableMenuItem(GetSubMenu(GetMenu(hWnd), MENU_HWND_CHECK), ID_MENUITEM_INITICON, CheckMenuFlag);
	SendMessage(TbWnd, TB_ENABLEBUTTON, ID_MENUITEM_INITICON, (LPARAM) MAKELONG(!CheckMenuFlag, 0));
	EnableMenuItem(GetSubMenu(GetMenu(hWnd), MENU_HWND_CHECK), ID_MENUITEM_ALLINITICON, Check);
	SendMessage(TbWnd, TB_ENABLEBUTTON, ID_MENUITEM_ALLINITICON, (LPARAM) MAKELONG(!Check, 0));

	//�A�C�R���ύX
	EnableMenuItem(GetSubMenu(GetMenu(hWnd), MENU_HWND_CHECK), ID_MENUITEM_ICON_UP, !(List && Item));
	EnableMenuItem(GetSubMenu(GetMenu(hWnd), MENU_HWND_CHECK), ID_MENUITEM_ICON_ERROR, !(List && Item));
	EnableMenuItem(GetSubMenu(GetMenu(hWnd), MENU_HWND_CHECK), ID_MENUITEM_ICON_TIMEOUT, !(List && Item));
	EnableMenuItem(GetSubMenu(GetMenu(hWnd), MENU_HWND_CHECK), ID_MENUITEM_CHECKSWITCH,
		!((Tree && !(hItem == RecyclerItem)) || (List && SelItem && !Recy)));

	//�|�b�v�A�b�v���j���[
	//�`�F�b�N
	EnableMenuItem(GetSubMenu(hPOPUP, MENU_POP_ITEM), ID_MENUITEM_CHECKEND_POP, !CheckMenuFlag);
	EnableMenuItem(GetSubMenu(hPOPUP, MENU_POP_ITEM), ID_MENUITEM_INITICON_POP, CheckMenuFlag);

	//�ҏW
	EnableMenuItem(GetSubMenu(hPOPUP, MENU_POP_ITEM), ID_MENUITEM_CUT_POP,
		!((Tree && !Root) || (List && SelItem)));
	EnableMenuItem(GetSubMenu(hPOPUP, MENU_POP_ITEM), ID_MENUITEM_COPY_POP,
		!((Tree && !Root) || (List && SelItem)));
	EnableMenuItem(GetSubMenu(hPOPUP, MENU_POP_ITEM), ID_MENUITEM_DELETE_POP,
		!((Tree && !Root && !CheckFolder) || (List && SelItem && !Recy)));
	EnableMenuItem(GetSubMenu(hPOPUP, MENU_POP_ITEM), ID_MENUITEM_EDITNAME_POP,
		!((Tree && !Root) || (List && SelItem && !Recy)));

	//�A�C�R���ύX
	EnableMenuItem(GetSubMenu(hPOPUP, MENU_POP_ITEM), ID_MENUITEM_ICON_UP, !(List && Item));
	EnableMenuItem(GetSubMenu(hPOPUP, MENU_POP_ITEM), ID_MENUITEM_ICON_ERROR, !(List && Item));
	EnableMenuItem(GetSubMenu(hPOPUP, MENU_POP_ITEM), ID_MENUITEM_ICON_TIMEOUT, !(List && Item));
	EnableMenuItem(GetSubMenu(hPOPUP, MENU_POP_ITEM), ID_MENUITEM_CHECKSWITCH_POP,
		!((Tree) || (List && SelItem && !Recy)));

	//�t�H���_���j���[
	//�`�F�b�N
	EnableMenuItem(GetSubMenu(hPOPUP, MENU_POP_FOLDER), ID_MENUITEM_CHECKEND, !CheckNoCheck);
	EnableMenuItem(GetSubMenu(hPOPUP, MENU_POP_FOLDER), ID_MENUITEM_INITICON, CheckFolder);

	EnableMenuItem(GetSubMenu(hPOPUP, MENU_POP_FOLDER), ID_MENUITEM_UNDO, !Undo);
}


/******************************************************************************

	SetEnableToolMenu

	�E�B���h�E�̃c�[�����j���[�̎g�p�\�^�g�p�s�\��Ԃ�ݒ肷��

******************************************************************************/

void SetEnableToolMenu(HWND hWnd)
{
	int i;

	for(i = 0;i < ToolListCnt;i++){
		if((ToolList[i].Action & TOOL_EXEC_WINDOWMENU) != 0 &&
			(ToolList[i].Action & TOOL_EXEC_NOTCHECK) != 0){
			EnableMenuItem(GetSubMenu(GetMenu(hWnd), MENU_HWND_TOOL), ID_MENU_TOOL_ACTION + i, gCheckFlag);
		}else{
			EnableMenuItem(GetSubMenu(GetMenu(hWnd), MENU_HWND_TOOL), ID_MENU_TOOL_ACTION + i, MF_ENABLED);
		}
	}
}


/******************************************************************************

	SetEnableToolItemMenu

	�A�C�e���̃c�[�����j���[�̎g�p�\�^�g�p�s�\��Ԃ�ݒ肷��

******************************************************************************/

void SetEnableToolItemMenu(HMENU hMenu)
{
	int i;

	for(i = 0;i < ToolListCnt;i++){
		if((ToolList[i].Action & TOOL_EXEC_ITEMMENU) != 0 &&
			(ToolList[i].Action & TOOL_EXEC_NOTCHECK) != 0){
			EnableMenuItem(hMenu, ID_MENU_TOOL_ACTION + i, gCheckFlag);
		}else{
			EnableMenuItem(hMenu, ID_MENU_TOOL_ACTION + i, MF_ENABLED);
		}
	}
}


/******************************************************************************

	SetNewItemMenu

	���j���[�� �u�V�K�쐬�v �̐ݒ�

******************************************************************************/

void SetNewItemMenu(HWND hWnd)
{
	int i;
	int MenuCnt;
	HMENU NewMenu;

	NewMenu = GetSubMenu(hPOPUP, MENU_POP_NEW);
	MenuCnt = GetMenuItemCount(NewMenu);

	if(MenuCnt > NEWMENU_INDEX){
		for(i = NEWMENU_INDEX;i < MenuCnt;i++){
			DeleteMenu(NewMenu, 2, MF_BYPOSITION);
		}
	}

	for(i = 0;i < ProtocolCnt;i++){
		AppendMenu(NewMenu, MF_STRING, ID_MENU_NEWITEM + i, tpProtocol[i].NewMenu);
	}
}


/******************************************************************************

	ModifyCheckMenuItem

	���j���[�̕ύX�`�F�b�N

******************************************************************************/

static BOOL ModifyCheckMenuItem(HWND hWnd, HMENU pMenu)
{
	struct TPITEM *tpItemInfo;
	HTREEITEM HTItem;
	int TmpIndex = MENUFLAG_NON;
	int FolderFlag = MENUFLAG_FOLDER;
	int SelectItem;
	int i;

	//���j���[�����ʂ���
	for(i = 0;i < WKMENU_CNT;i++){
		if(tpWinMenu[i].Menu == pMenu){
			break;
		}
		if(tpWinMenu[i].Menu == NULL){
			tpWinMenu[i].Menu = pMenu;
			break;
		}
	}

	if(GetFocus() == GetDlgItem(hWnd, WWWC_TREE)){
		FolderFlag = MENUFLAG_TREEFOLDER;
		HTItem = (HTREEITEM)SendDlgItemMessage(hWnd, WWWC_TREE, TVM_GETNEXTITEM, TVGN_DROPHILITE, 0);
		if(HTItem == NULL){
			HTItem = TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE));
		}
		if(HTItem == RecyclerItem){
			FolderFlag = MENUFLAG_TREERECYCLER;
		}
		if(tpWinMenu[i].OldProtocolIndex == FolderFlag){
			//�O��̃��j���[���ڂƓ���
			return FALSE;
		}
		tpWinMenu[i].OldProtocolIndex = FolderFlag;
		return TRUE;
	}

	if(ListView_GetSelectedCount(GetDlgItem(hWnd, WWWC_LIST)) == 0){
		tpWinMenu[i].OldProtocolIndex = TmpIndex;
		return TRUE;
	}

	//�O��̃��j���[�Ɠ����ꍇ�͊֐��𔲂���
	SelectItem = ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), -1, LVNI_FOCUSED);
	if(ListView_GetItemState(GetDlgItem(hWnd, WWWC_LIST), SelectItem, LVIS_SELECTED) != LVIS_SELECTED){
		SelectItem = ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), -1, LVNI_SELECTED);
	}

	tpItemInfo = (struct TPITEM *)ListView_GetlParam(GetDlgItem(hWnd, WWWC_LIST), SelectItem);
	if(tpItemInfo != NULL){
		if(tpWinMenu[i].OldProtocolIndex == GetProtocolIndex(tpItemInfo->CheckURL)){
			//�O��̃��j���[���ڂƓ���
			return FALSE;
		}else{
			TmpIndex = GetProtocolIndex(tpItemInfo->CheckURL);
		}
	}else{
		if(ListView_IsRecyclerItem(hWnd) == TRUE){
			FolderFlag = MENUFLAG_RECYCLER;
		}
		if(tpWinMenu[i].OldProtocolIndex == FolderFlag){
			//�O��̃��j���[���ڂƓ���
			return FALSE;
		}else{
			TmpIndex = FolderFlag;
		}
	}
	tpWinMenu[i].OldProtocolIndex = TmpIndex;
	return TRUE;
}


/******************************************************************************

	DeleteProtocolMenuItem

	�v���g�R�����j���[���ڂ̍폜

******************************************************************************/

void DeleteProtocolMenuItem(HMENU pMenu)
{
	UINT MenuID;

	//���݂̃��j���[���ڂ��폜
	while(1){
		MenuID = GetMenuItemID(pMenu, 0);
		if(MenuID == ID_MENUITEM_CHECK_POP ||
			MenuID == ID_MENUITEM_CHECK ||
			MenuID == ID_MENUITEM_POP_NEWITEM ||
			MenuID == ID_MENUITEM_INITICON_POP ||
			MenuID == 0xFFFFFFFF){
			break;
		}
		DeleteMenu(pMenu, 0, MF_BYPOSITION);
	}
}


/******************************************************************************

	SetFolderItemMenu

	�t�H���_���j���[�̐ݒ�

******************************************************************************/

static BOOL SetFolderItemMenu(HWND hWnd, HMENU pMenu, BOOL ItemFlag)
{
	struct TPITEM *tpItemInfo;
	int SelectItem;
	HTREEITEM HTItem;

	//�c���[�r���[
	if(GetFocus() == GetDlgItem(hWnd, WWWC_TREE)){
		HTItem = (HTREEITEM)SendDlgItemMessage(hWnd, WWWC_TREE, TVM_GETNEXTITEM, TVGN_DROPHILITE, 0);
		if(HTItem == NULL){
			HTItem = TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE));
		}
		if(ItemFlag == 1){
			InsertMenu(pMenu, 0, MF_BYPOSITION | MF_STRING, ID_MENUITEM_FOLDERTREECHECK_POP, MENU_STR_FOLDERTREECHECK);
			InsertMenu(pMenu, 0, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
			InsertMenu(pMenu, 0, MF_BYPOSITION | MF_STRING, ID_MENUITEM_SERACH_POP, MENU_STR_SERACH);
		}

		if(HTItem == RecyclerItem){
			InsertMenu(pMenu, 0, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
			InsertMenu(pMenu, 0, MF_BYPOSITION | MF_STRING, ID_MENU_ACTION_RECY, MENU_STR_CLEARRECY);
			if(CheckRecycler(hWnd) == FALSE){
				EnableMenuItem(pMenu, ID_MENU_ACTION_RECY, 1);
			}
		}
		return TRUE;
	}

	//���X�g�r���[�őI���Ȃ�
	if(ListView_GetSelectedCount(GetDlgItem(hWnd, WWWC_LIST)) <= 0){
		if(TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)) == RecyclerItem){
			InsertMenu(pMenu, 0, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
			InsertMenu(pMenu, 0, MF_BYPOSITION | MF_STRING, ID_MENU_ACTION_RECY, MENU_STR_CLEARRECY);
			if(CheckRecycler(hWnd) == FALSE){
				EnableMenuItem(pMenu, ID_MENU_ACTION_RECY, 1);
			}
		}
		return TRUE;
	}

	SelectItem = ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), -1, LVNI_FOCUSED);
	if(ListView_GetItemState(GetDlgItem(hWnd, WWWC_LIST), SelectItem, LVIS_SELECTED) != LVIS_SELECTED){
		SelectItem = ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), -1, LVNI_SELECTED);
	}
	tpItemInfo = (struct TPITEM *)ListView_GetlParam(GetDlgItem(hWnd, WWWC_LIST), SelectItem);
	//���X�g�r���[�Ńt�H���_���I��
	if(tpItemInfo == NULL){
		if(ItemFlag == 1){
			InsertMenu(pMenu, 0, MF_BYPOSITION | MF_STRING, ID_MENUITEM_FOLDERTREECHECK_POP, MENU_STR_FOLDERTREECHECK);
			InsertMenu(pMenu, 0, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
			InsertMenu(pMenu, 0, MF_BYPOSITION | MF_STRING, ID_MENUITEM_SERACH_POP, MENU_STR_SERACH);
		}else{
			InsertMenu(pMenu, 0, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
		}

		if(ListView_IsRecyclerItem(hWnd) == TRUE){
			InsertMenu(pMenu, 0, MF_BYPOSITION | MF_STRING, ID_MENU_ACTION_RECY, MENU_STR_CLEARRECY);
			if(CheckRecycler(hWnd) == FALSE){
				EnableMenuItem(pMenu, ID_MENU_ACTION_RECY, 1);
			}
		}

		InsertMenu(pMenu, 0, MF_BYPOSITION | MF_STRING, ID_MENU_ACTION_OPEM, MENU_STR_OPEN);
		if(ItemFlag == 1){
			SetMenuDefaultItem(pMenu, ID_MENU_ACTION_OPEM, 0);
		}
		return TRUE;
	}
	return FALSE;
}


/******************************************************************************

	SetProtocolItemMenu

	�A�C�e�����j���[�̐ݒ�

******************************************************************************/

void SetProtocolItemMenu(HWND hWnd, HMENU pMenu, BOOL ItemFlag, BOOL UpWndFlag)
{
	struct TPITEM *tpItemInfo;
	int SelectItem;
	int ProtocolIndex;
	int pMenuCnt;
	int i, ii, j;
	int DefIndex = -1;
	int DefToolIndex = -1;

	SelectItem = ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), -1, LVNI_FOCUSED);
	if(ListView_GetItemState(GetDlgItem(hWnd, WWWC_LIST), SelectItem, LVIS_SELECTED) != LVIS_SELECTED){
		SelectItem = ListView_GetNextItem(GetDlgItem(hWnd, WWWC_LIST), -1, LVNI_SELECTED);
	}
	tpItemInfo = (struct TPITEM *)ListView_GetlParam(GetDlgItem(hWnd, WWWC_LIST), SelectItem);
	if(tpItemInfo == NULL){
		return;
	}

	ProtocolIndex = GetProtocolIndex(tpItemInfo->CheckURL);
	if(ProtocolIndex == -1 || tpProtocol[ProtocolIndex].tpMenu == NULL){
		InsertMenu(pMenu, 0, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
		InsertMenu(pMenu, 0, MF_BYPOSITION | MF_STRING, ID_MENU_ACTION_OPEM, MENU_STR_OPEN);
		if(ItemFlag == 1){
			SetMenuDefaultItem(pMenu, ID_MENU_ACTION_OPEM, 0);
		}
		return;
	}
	pMenuCnt = tpProtocol[ProtocolIndex].tpMenuCnt;
	if(pMenuCnt == 0){
		return;
	}
	for(ii = 0, i = 0; i <= pMenuCnt; i++){
		//�c�[�����j���[
		for(j = 0; j < ToolListCnt; j++){
			if((ToolList[j].Action & TOOL_EXEC_ITEMMENU) != 0 &&
				strlistcmp(tpProtocol[ProtocolIndex].title, ToolList[j].Protocol, ',') == TRUE &&
				(ToolList[j].MenuIndex == i || (i >= pMenuCnt && (ToolList[j].MenuIndex > i || ToolList[j].MenuIndex == -1)))){

				if(lstrcmp(ToolList[j].title, "-") == 0){
					InsertMenu(pMenu, ii++, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
				}else{
					InsertMenu(pMenu, ii++, MF_BYPOSITION | MF_STRING, ID_MENU_TOOL_ACTION + j, ToolList[j].title);
					if(ItemFlag == 1 && DefToolIndex == -1 && (ToolList[j].Action & TOOL_EXEC_MENUDEFAULT) != 0){
						DefToolIndex = ID_MENU_TOOL_ACTION + j;
					}
				}
			}
		}
		if(i >= pMenuCnt){
			break;
		}

		//�v���g�R�����j���[
		if(lstrcmp(tpProtocol[ProtocolIndex].tpMenu[i].name, "-") == 0){
			InsertMenu(pMenu, ii++, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
		}else{
			InsertMenu(pMenu, ii++, MF_BYPOSITION | MF_STRING, ID_MENU_ACTION + i,
				tpProtocol[ProtocolIndex].tpMenu[i].name);

			if(ItemFlag == 1 && DefIndex == -1 && tpProtocol[ProtocolIndex].tpMenu[i].Default == TRUE){
				DefIndex = ID_MENU_ACTION + i;
			}
			if(UpWndFlag == 1 && tpProtocol[ProtocolIndex].tpMenu[i].Flag == 1){
				EnableMenuItem(pMenu, ID_MENU_ACTION + i, 1);
			}
		}
	}
	InsertMenu(pMenu, ii, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);

	//�f�t�H���g���j���[�̐ݒ�
	if(DefToolIndex != -1){
		SetMenuDefaultItem(pMenu, DefToolIndex, 0);
	}else if(DefIndex != -1){
		SetMenuDefaultItem(pMenu, DefIndex, 0);
	}
}


/******************************************************************************

	SetProtocolMenu

	�v���g�R�����̃��j���[��ݒ�

******************************************************************************/

void SetProtocolMenu(HWND hWnd)
{
	//�E�B���h�E���j���[
	if(ModifyCheckMenuItem(hWnd, GetSubMenu(GetMenu(hWnd), MENU_HWND_FILE)) == TRUE){
		//���j���[�̍폜
		DeleteProtocolMenuItem(GetSubMenu(GetMenu(hWnd), MENU_HWND_FILE));
		if(SetFolderItemMenu(hWnd, GetSubMenu(GetMenu(hWnd), MENU_HWND_FILE), FALSE) == FALSE){
			SetProtocolItemMenu(hWnd, GetSubMenu(GetMenu(hWnd), MENU_HWND_FILE), FALSE, FALSE);
		}
	}
	//�|�b�v�A�b�v���j���[
	if(ModifyCheckMenuItem(hWnd, GetSubMenu(hPOPUP, MENU_POP_ITEM)) == TRUE){
		//���j���[�̍폜
		DeleteProtocolMenuItem(GetSubMenu(hPOPUP, MENU_POP_ITEM));
		if(SetFolderItemMenu(hWnd, GetSubMenu(hPOPUP, MENU_POP_ITEM), TRUE) == FALSE){
			SetProtocolItemMenu(hWnd, GetSubMenu(hPOPUP, MENU_POP_ITEM), TRUE, FALSE);
		}
	}
	SetEnableMenu(hWnd);
	SetEnableToolItemMenu(GetSubMenu(hPOPUP, MENU_POP_ITEM));
}
/* End of source */
