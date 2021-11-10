/**************************************************************************

	WWWC

	TreeView.c

	Copyright (C) 1996-2008 by Nakashima Tomoaki. All rights reserved.
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
#include "OleDragDrop.h"


/**************************************************************************
	Define
**************************************************************************/

#define HISTORYCNT					100


/**************************************************************************
	Global Variables
**************************************************************************/

HTREEITEM RootItem;
HTREEITEM RecyclerItem;
HTREEITEM HistoryItem[HISTORYCNT + 1];
int HistoryIndex;
BOOL HistoryFlag;

//�O���Q��
extern HINSTANCE g_hinst;			//�A�v���P�[�V�����̃C���X�^���X�n���h��
extern HWND WWWCWnd;				//�{��
extern HTREEITEM DgdpItem;
extern char CuDir[];
extern int gCheckFlag;
extern BOOL WWWCDropFlag;
extern UINT WWWC_ClipFormat;

extern char RootTitle[];
extern char RecyclerTitle[];
extern int SucceedFromParent;
extern int TvWndStyle;
extern char TvBkColor[];
extern int TvIconSize;

extern char Check[];
extern int CheckIndex;
extern char NoCheck[];
extern int NoCheckIndex;
extern char DirUP[];
extern int DirUPIndex;
extern char Dir[];
extern int DirIndex;
extern char DirUPchild[];
extern int DirUPchildIndex;
extern char CheckChild[];
extern int CheckChildIndex;
extern char Recycler[];
extern int RecyclerIndex;
extern char RecyclerFull[];
extern int RecyclerFullIndex;

extern char Inet[];
extern int InetIndex;
extern char DirOpen[];
extern int DirOpenIndex;

extern int DragItemIndex;


/**************************************************************************
	Local Function Prototypes
**************************************************************************/

static HTREEITEM TreeView_SetNewItem(HWND hTreeView, char *buf, HTREEITEM SetItem,
									 int Icon, int IconSel, HTREEITEM After);
static BOOL TreeView_SetItemText(HWND hTreeView, HTREEITEM hItem, char *buf);
static BOOL TreeView_SetIcon(HWND hTreeView, HTREEITEM hItem, int Icon, int SelIcon);
static void TreeView_SetParentIcon(HWND hWnd, HTREEITEM hItem, int Icon);
static void TreeView_DeleteParentIcon(HWND hWnd, HTREEITEM hItem, int Icon1, int Icon2);
static void TreeView_SetHistory(HTREEITEM NewItem);


/******************************************************************************

	CallTreeItem

	���ׂẴc���r���[�̃A�C�e���Ɏw��֐������s

******************************************************************************/

void CallTreeItem(HWND hWnd, HTREEITEM hItem, FARPROC Func, long Param)
{
	HTREEITEM cItem;

	//�֐��̌Ăяo��
	Func(hWnd, hItem, Param);

	cItem = TreeView_GetChild(GetDlgItem(hWnd, WWWC_TREE), hItem);
	while(cItem != NULL){
		//�ċA
		CallTreeItem(hWnd, cItem, Func, Param);
		cItem = TreeView_GetNextSibling(GetDlgItem(hWnd, WWWC_TREE), cItem);
	}
}


/******************************************************************************

	CreateTreeView

	�c���[�r���[�̍쐬

******************************************************************************/

HWND CreateTreeView(HWND hWnd)
{
	return CreateWindowEx(WS_EX_NOPARENTNOTIFY,
		WC_TREEVIEW, (LPSTR)NULL, WS_VISIBLE | WS_CHILD | WS_TABSTOP | TvWndStyle,
		0, 0, 0, 0, hWnd, (HMENU)WWWC_TREE, g_hinst, NULL);
}


/******************************************************************************

	TreeView_Initialize

	�c���[�r���[�̏�����

******************************************************************************/

void TreeView_Initialize(HWND hWnd)
{
	HIMAGELIST IconList;
	HIMAGELIST TmpIconList;
	HICON TmpIcon;

	IconList = ImageList_Create(TvIconSize, TvIconSize, ILC_COLOR16 | ILC_MASK, 0, 0);
	if(*TvBkColor != '\0'){
		ImageList_SetBkColor(IconList, strtol(TvBkColor, NULL, 0));
	}else{
		ImageList_SetBkColor(IconList, GetSysColor(COLOR_WINDOW));
	}
	//general
	ImageListIconAdd(IconList, IDI_ICON_CHECK, TvIconSize, Check, CheckIndex);
	ImageListIconAdd(IconList, IDI_ICON_NOCHECK, TvIconSize, NoCheck, NoCheckIndex);
	ImageList_SetOverlayImage(IconList, ICON_NOCHECK, 1);
	ImageListIconAdd(IconList, IDI_ICON_DIRUP, TvIconSize, DirUP, DirUPIndex);
	ImageListFileIconAdd(IconList, CuDir, 0, TvIconSize, Dir, DirIndex);
	ImageListIconAdd(IconList, IDI_ICON_UPCHILD, TvIconSize, DirUPchild, DirUPchildIndex);
	ImageListIconAdd(IconList, IDI_ICON_CHECKCHILD, TvIconSize, CheckChild, CheckChildIndex);

	TmpIconList = ImageList_Merge(IconList, ICON_DIR_CLOSE, IconList, ICON_CHECK, 0, 0);
	TmpIcon = ImageList_GetIcon(TmpIconList, 0, ILD_NORMAL);
	ImageList_AddIcon(IconList, TmpIcon);
	DestroyIcon(TmpIcon);
	ImageList_Destroy((void *)TmpIconList);

	TmpIconList = ImageList_Merge(IconList, ICON_DIR_CLOSE, IconList, ICON_DIRUP, 0, 0);
	TmpIcon = ImageList_GetIcon(TmpIconList, 0, ILD_NORMAL);
	ImageList_AddIcon(IconList, TmpIcon);
	DestroyIcon(TmpIcon);
	ImageList_Destroy((void *)TmpIconList);

	TmpIconList = ImageList_Merge(IconList, ICON_DIR_CLOSE, IconList, ICON_DIRUPCHILD, 0, 0);
	TmpIcon = ImageList_GetIcon(TmpIconList, 0, ILD_NORMAL);
	ImageList_AddIcon(IconList, TmpIcon);
	DestroyIcon(TmpIcon);
	ImageList_Destroy((void *)TmpIconList);

	TmpIconList = ImageList_Merge(IconList, ICON_DIR_CLOSE, IconList, ICON_CHECKCHILD, 0, 0);
	TmpIcon = ImageList_GetIcon(TmpIconList, 0, ILD_NORMAL);
	ImageList_AddIcon(IconList, TmpIcon);
	DestroyIcon(TmpIcon);
	ImageList_Destroy((void *)TmpIconList);

	ImageListIconAdd(IconList, IDI_ICON_RECYCLER, TvIconSize, Recycler, RecyclerIndex);
	ImageListIconAdd(IconList, IDI_ICON_RECYCLER_USE, TvIconSize, RecyclerFull, RecyclerFullIndex);

	//TreeView
	ImageListIconAdd(IconList, IDI_ICON_INET, TvIconSize, Inet, InetIndex);

	ImageListFileIconAdd(IconList, CuDir, SHGFI_OPENICON, TvIconSize, DirOpen, DirOpenIndex);

	TmpIconList = ImageList_Merge(IconList, ICON_DIR_OPEN, IconList, ICON_CHECK, 0, 0);
	TmpIcon = ImageList_GetIcon(TmpIconList, 0, ILD_NORMAL);
	ImageList_AddIcon(IconList, TmpIcon);
	DestroyIcon(TmpIcon);
	ImageList_Destroy((void *)TmpIconList);

	TmpIconList = ImageList_Merge(IconList, ICON_DIR_OPEN, IconList, ICON_DIRUP, 0, 0);
	TmpIcon = ImageList_GetIcon(TmpIconList, 0, ILD_NORMAL);
	ImageList_AddIcon(IconList, TmpIcon);
	DestroyIcon(TmpIcon);
	ImageList_Destroy((void *)TmpIconList);

	TmpIconList = ImageList_Merge(IconList, ICON_DIR_OPEN, IconList, ICON_DIRUPCHILD, 0, 0);
	TmpIcon = ImageList_GetIcon(TmpIconList, 0, ILD_NORMAL);
	ImageList_AddIcon(IconList, TmpIcon);
	DestroyIcon(TmpIcon);
	ImageList_Destroy((void *)TmpIconList);

	TmpIconList = ImageList_Merge(IconList, ICON_DIR_OPEN, IconList, ICON_CHECKCHILD, 0, 0);
	TmpIcon = ImageList_GetIcon(TmpIconList, 0, ILD_NORMAL);
	ImageList_AddIcon(IconList, TmpIcon);
	DestroyIcon(TmpIcon);
	ImageList_Destroy((void *)TmpIconList);

	TreeView_SetImageList(GetDlgItem(hWnd, WWWC_TREE), IconList, TVSIL_NORMAL);

	//���[�g�A�C�e��
	RootItem = TreeView_AllocItem(GetDlgItem(hWnd, WWWC_TREE), (HTREEITEM)TVI_ROOT,
		(HTREEITEM)TVI_LAST, RootTitle, ICON_INET, ICON_INET);
	GetDirInfo(GetDlgItem(hWnd, WWWC_TREE), CuDir, RootItem);

	//���ݔ�
	RecyclerItem = TreeView_AllocItem(GetDlgItem(hWnd, WWWC_TREE), (HTREEITEM)RootItem,
		(HTREEITEM)TVI_LAST, RecyclerTitle, ICON_DIR_RECYCLER, ICON_DIR_RECYCLER);

	//���f�B���N�g���\������t�H���_�̍쐬
	SetDirTree(GetDlgItem(hWnd, WWWC_TREE), CuDir, RootItem);

	//���ݔ��̏�Ԃ��X�V
	TreeView_SetIconState(hWnd, RecyclerItem, 0);
}


/******************************************************************************

	TreeView_SetNewItem

	�c���[�r���[�A�C�e���̒ǉ�

******************************************************************************/

static HTREEITEM TreeView_SetNewItem(HWND hTreeView, char *buf, HTREEITEM SetItem,
									 int Icon, int IconSel, HTREEITEM After)
{
	TV_INSERTSTRUCT tvitn = { 0 };
	TV_ITEM tvit = { 0 };

	tvit.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
	tvit.cchTextMax = BUFSIZE - 1;
	tvitn.hInsertAfter = After;
	tvit.iImage = Icon;
	tvit.iSelectedImage = IconSel;
	tvit.hItem = NULL;
	tvit.state = 0;
	tvit.stateMask = 0;
	tvit.cChildren = 0;
	tvit.lParam = 0;
	tvit.pszText = buf;

	tvitn.hParent = SetItem;
	tvitn.item = tvit;
	return TreeView_InsertItem(hTreeView, &tvitn);
}


/******************************************************************************

	TreeView_AllocItem

	�c���[���̊m��

******************************************************************************/

HTREEITEM TreeView_AllocItem(HWND hTreeView, HTREEITEM pItem, HTREEITEM After, char *Title,
							 int Icon, int IconSel)
{
	struct TPTREE *tpTreeInfo;
	HTREEITEM NewItem;

	tpTreeInfo = (struct TPTREE *)GlobalAlloc(GPTR, sizeof(struct TPTREE));
	if(tpTreeInfo == NULL){
		abort();
	}
	tpTreeInfo->ItemList = NULL;
	tpTreeInfo->ItemListCnt = 0;
	tpTreeInfo->CheckFlag = 0;
	tpTreeInfo->MemFlag = 0;
	tpTreeInfo->Icon = 0;
	tpTreeInfo->CheckSt = 0;
	tpTreeInfo->AutoCheckSt = 1;
	tpTreeInfo->Comment = NULL;

	NewItem = TreeView_SetNewItem(hTreeView, Title, pItem, Icon, IconSel, (HTREEITEM)After);

	//�c���[�r���[�A�C�e����lParam�Ƀc���[���̃A�h���X�������
	TreeView_SetlParam(hTreeView, NewItem, (long)tpTreeInfo);

	return NewItem;
}


/******************************************************************************

	TreeView_NewFolderItem

	�c���[�r���[�ɐV�����t�H���_��ǉ�����

******************************************************************************/

BOOL TreeView_NewFolderItem(HWND hWnd)
{
	HTREEITEM hItem;
	char name[BUFSIZE];
	char buf[BUFSIZE];
	char path[BUFSIZE];
	int i;

	//�V�����t�H���_�̖��O���쐬���Ď��ۂɃt�H���_���쐬����
	TreeView_GetPath(GetDlgItem(hWnd, WWWC_TREE),
		TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)), buf, CuDir);
	lstrcpy(name, NEWFOLDER);
	wsprintf(path, "%s\\%s", buf, name);

	//���Ƀt�H���_�����݂��Ă���ꍇ�͔ԍ���t����
	i = 1;
	while(CreateDirectory(path, NULL) == FALSE){
		i++;
		wsprintf(name, "%s (%d)", NEWFOLDER, i);
		wsprintf(path, "%s\\%s", buf, name);
	}

	//�c���[�Ƀt�H���_��ǉ�����
	hItem = TreeView_AllocItem(GetDlgItem(hWnd, WWWC_TREE),
		TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)),
		(HTREEITEM)TVI_SORT, name, ICON_DIR_CLOSE, ICON_DIR_OPEN);

	if(SucceedFromParent == 1){
		//�e���̌p��
		CopyAutoTime(hWnd, hItem, (long)TreeView_GetlParam(GetDlgItem(hWnd, WWWC_TREE),
			TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE))));
	}

	if(hItem == NULL){
		return FALSE;
	}

	InvalidateRect(GetDlgItem(hWnd, WWWC_TREE), NULL, FALSE);
	UpdateWindow(GetDlgItem(hWnd, WWWC_TREE));

	if(GetFocus() == GetDlgItem(hWnd, WWWC_TREE)){
		//�c���[�Ƀt�H�[�J�X������ꍇ�́A�ǉ������A�C�e����I�����ă��x����ҏW���[�h�ɂ���
		TreeView_SelectItem(GetDlgItem(hWnd, WWWC_TREE), hItem);
		TreeView_EditLabel(GetDlgItem(hWnd, WWWC_TREE), hItem);
		return TRUE;
	}
	if(GetFocus() == GetDlgItem(hWnd, WWWC_LIST)){
		//���X�g�r���[�Ƀt�H�[�J�X������ꍇ�́A���X�g�Ƀt�H���_��ǉ����ĕҏW���[�h�ɂ���
		ListView_RefreshFolder(hWnd);

		if((i = TreeView_GetListIndex(GetDlgItem(hWnd, WWWC_TREE), hItem)) != -1){
			//���X�g�r���[�A�C�e����ҏW���[�h�ɂ���
			ListView_EditLabel(GetDlgItem(hWnd, WWWC_LIST), i);
		}
	}
	return TRUE;
}


/******************************************************************************

	TreeView_FreeItem

	�c���[�r���[�A�C�e���Ɋ֘A�t����ꂽ�A�C�e�����̉��

******************************************************************************/

void CALLBACK TreeView_FreeItem(HWND hWnd, HTREEITEM hItem, long Param)
{
	struct TPTREE *tpTreeInfo;
	int i;

	if(Param == 1){
		WaitCursor(TRUE);
		//�A�C�e�����̕ۑ�
		SaveTreeMem(hWnd, GetDlgItem(hWnd, WWWC_TREE), hItem);
		WaitCursor(FALSE);
	}

	//���݊J���Ă���t�H���_�̏ꍇ�͊֐��𔲂���
	if(Param != 2 && hItem == TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE))){
		return;
	}

	//�c���[���̎擾
	tpTreeInfo = (struct TPTREE *)TreeView_GetlParam(GetDlgItem(hWnd, WWWC_TREE), hItem);
	if(tpTreeInfo == NULL){
		return;
	}
	if(tpTreeInfo->ItemList == NULL){
		return;
	}

	//�t�H���_�̃v���p�e�B���J���Ă���Ȃǂ̏�
	if(tpTreeInfo->MemFlag > 0){
		return;
	}
	tpTreeInfo->MemFlag = 0;

	//�`�F�b�N���̏ꍇ�͊֐��𔲂���
	for(i = 0;i < tpTreeInfo->ItemListCnt;i++){
		if((*(tpTreeInfo->ItemList + i)) == NULL){
			continue;
		}
		if((*(tpTreeInfo->ItemList + i))->IconStatus == ST_CHECK ||
			(*(tpTreeInfo->ItemList + i))->IconStatus == ST_NOCHECK){
			return;
		}
	}

	//�A�C�e�����X�g�̉��
	FreeItemList(tpTreeInfo->ItemList, tpTreeInfo->ItemListCnt, TRUE);
	GlobalFree(tpTreeInfo->ItemList);
	tpTreeInfo->ItemList = NULL;
	tpTreeInfo->ItemListCnt = 0;
}


/******************************************************************************

	TreeView_FreeTreeItem

	�c���[���̉��

******************************************************************************/

void CALLBACK TreeView_FreeTreeItem(HWND hWnd, HTREEITEM hItem, long Param)
{
	struct TPTREE *tpTreeInfo;

	//�c���[���̎擾
	tpTreeInfo = (struct TPTREE *)TreeView_GetlParam(GetDlgItem(hWnd, WWWC_TREE), hItem);
	if(tpTreeInfo == NULL){
		return;
	}
	tpTreeInfo->MemFlag = 0;

	//�c���[���̎擾
	if(Param == 0){
		SaveTreeMem(hWnd, GetDlgItem(hWnd, WWWC_TREE), hItem);
	}

	//�A�C�e�����X�g�̉��
	if(tpTreeInfo->ItemList != NULL){
		FreeItemList(tpTreeInfo->ItemList, tpTreeInfo->ItemListCnt, TRUE);
		GlobalFree(tpTreeInfo->ItemList);
		tpTreeInfo->ItemList = NULL;
		tpTreeInfo->ItemListCnt = 0;
	}

	//�����`�F�b�N���̉��
	if(tpTreeInfo->tpCheckTime != NULL){
		GlobalFree(tpTreeInfo->tpCheckTime);
		tpTreeInfo->tpCheckTime = NULL;
		tpTreeInfo->tpCheckTimeCnt = 0;
	}

	//�t�H���_�̃R�����g�̉��
	if(tpTreeInfo->Comment != NULL){
		GlobalFree(tpTreeInfo->Comment);
		tpTreeInfo->Comment = NULL;
	}

	//�c���[���̉��
	GlobalFree(tpTreeInfo);
	TreeView_SetlParam(GetDlgItem(hWnd, WWWC_TREE), hItem, (long)NULL);
}


/******************************************************************************

	TreeView_DeleteTreeInfo

	�c���[�r���[�A�C�e���̍폜

******************************************************************************/

void TreeView_DeleteTreeInfo(HWND hWnd, HTREEITEM hItem)
{
	HTREEITEM pItem;
	char buf[BUFSIZE];

	//���f�B���N�g���̍폜
	TreeView_GetPath(GetDlgItem(hWnd, WWWC_TREE), hItem, buf, CuDir);
	DeleteDirTree(buf, FALSE);
	RemoveDirectory(buf);

	//�t�H���_�̍폜
	CallTreeItem(hWnd, hItem, (FARPROC)TreeView_FreeTreeItem, 1);
	pItem = TreeView_GetParent(GetDlgItem(hWnd, WWWC_TREE), hItem);
	TreeView_DeleteItem(GetDlgItem(hWnd, WWWC_TREE), hItem);
	TreeView_SetIconState(hWnd, pItem, 0);
}


/******************************************************************************

	TreeView_AllExpand

	�A�C�e����W�J����

******************************************************************************/

void CALLBACK TreeView_AllExpand(HWND hWnd, HTREEITEM hItem, long Param)
{
	struct TPTREE *tpTreeInfo;

	if(Param == 1){
		TreeView_Expand(GetDlgItem(hWnd, WWWC_TREE), hItem, TVE_EXPAND);
	}else{
		//�c���[���̎擾
		tpTreeInfo = (struct TPTREE *)TreeView_GetlParam(GetDlgItem(hWnd, WWWC_TREE), hItem);
		if(tpTreeInfo == NULL){
			return;
		}
		if(tpTreeInfo->Expand != 0){
			TreeView_Expand(GetDlgItem(hWnd, WWWC_TREE), hItem, TVE_EXPAND);
		}
	}
}


/******************************************************************************

	TreeView_FindItemPath

	�p�X����A�C�e��������

******************************************************************************/

HTREEITEM TreeView_FindItemPath(HWND hTreeView, HTREEITEM hItem, char *path)
{
	HTREEITEM cItem;
	char *p, *r;
	char buf[BUFSIZE];

	if(*path == '\0'){
		return NULL;
	}

	p = path;

	//���[�g�̏ꍇ
	if((*p == '\\' && *(p + 1) == '\\') || (*p == '/' && *(p + 1) == '/')){
		for(p = path + 2; *p != '\0'; p++){
			if(IsDBCSLeadByte((BYTE)*p) == FALSE){
				if(*p == '\\' || *p == '/'){
					break;
				}
			}else{
				p++;
			}
		}
	}

	if(*p == '\\' || *p == '/'){
		p++;
	}
	if(*p == '\0'){
		return hItem;
	}

	//�p�X��W�J
	for(r = buf; *p != '\0'; p++, r++){
		if(IsDBCSLeadByte((BYTE)*p) == FALSE){
			if(*p == '\\' || *p == '/'){
				break;
			}
			*r = *p;
		}else{
			*(r++) = *(p++);
			*r = *p;
		}
	}
	*r = '\0';

	//���O����A�C�e�����擾
	cItem = TreeView_CheckName(hTreeView, hItem, buf);
	if(cItem == NULL){
		return NULL;
	}

	if(*p != '\0'){
		//�ċA
		return TreeView_FindItemPath(hTreeView, cItem, p);
	}
	return cItem;
}


/******************************************************************************

	FindTreeItem

	�A�C�e�������݂��邩���ׂ�

******************************************************************************/

void CALLBACK FindTreeItem(HWND hWnd, HTREEITEM hItem, long Param)
{
	HTREEITEM pItem;

	pItem = *((HTREEITEM *)Param);

	if(pItem == hItem){
		*((HTREEITEM *)Param) = NULL;
	}
}
BOOL IsTreeItem(HWND hWnd, HTREEITEM hItem)
{
	HTREEITEM RetItem;

	RetItem = hItem;
	CallTreeItem(WWWCWnd, RootItem, (FARPROC)FindTreeItem, (long)&RetItem);
	if(RetItem != NULL){
		return FALSE;
	}
	return TRUE;
}


/******************************************************************************

	TreeView_CheckName

	�w��̖��O�̃c���[�r���[�A�C�e���̃n���h�����擾

******************************************************************************/

HTREEITEM TreeView_CheckName(HWND hTreeView, HTREEITEM hItem, char *str)
{
	HTREEITEM cItem;
	char buf[BUFSIZE];

	cItem = TreeView_GetChild(hTreeView, hItem);
	while(cItem != NULL){
		TreeView_GetItemInfo(hTreeView, cItem, buf);
		if(lstrcmpi(str, buf) == 0){
			return cItem;
		}
		cItem = TreeView_GetNextSibling(hTreeView, cItem);
	}
	return NULL;
}


/******************************************************************************

	TreeView_GetChildCount

	�c���[�r���[�A�C�e���̎q�������擾

******************************************************************************/

int TreeView_GetChildCount(HWND hTreeView, HTREEITEM hItem)
{
	HTREEITEM cItem;
	int ret = 0;

	//�c���[�r���[�ɕ\������Ă���t�H���_�̐�
	cItem = TreeView_GetChild(hTreeView, hItem);
	while(cItem != NULL){
		ret++;
		cItem = TreeView_GetNextSibling(hTreeView, cItem);
	}
	return ret;
}


/******************************************************************************

	TreeView_IsRecyclerItem

	�c���[�r���[�A�C�e�������ݔ������ݔ��̒��̃A�C�e���Ȃ̂��`�F�b�N����

******************************************************************************/

BOOL TreeView_IsRecyclerItem(HWND hTreeView, HTREEITEM hItem)
{
	HTREEITEM cItem;

	cItem = hItem;
	while(cItem != RootItem){
		if(cItem == RecyclerItem){
			//���ݔ��̏ꍇ
			return TRUE;
		}
		cItem = TreeView_GetParent(hTreeView, cItem);
	}
	return FALSE;
}


/******************************************************************************

	TreeView_GetPath

	�c���[�r���[�A�C�e���̃p�X���擾����

******************************************************************************/

void TreeView_GetPath(HWND hTreeView, HTREEITEM hItem, char *ret, char *RootString)
{
	HTREEITEM pItem;
	char buf[BUFSIZE];
	char tvName[BUFSIZE];
	char work[BUFSIZE];

	//���[�g�A�C�e���̏ꍇ�͂��̂܂܃p�X��Ԃ�
	if(hItem == RootItem){
		lstrcpy(ret, RootString);
		return;
	}

	if(TreeView_GetItemInfo(hTreeView, hItem, buf) == -1){
		*ret = '\0';
		return;
	}
	pItem = TreeView_GetParent(hTreeView, hItem);
	while(pItem != RootItem){
		TreeView_GetItemInfo(hTreeView, pItem, tvName);
		wsprintf(work, "%s\\%s", tvName, buf);
		lstrcpy(buf, work);

		pItem = TreeView_GetParent(hTreeView, pItem);
	}
	//�w��̕�����ƌ���
	wsprintf(ret, "%s\\%s", RootString, buf);
}


/******************************************************************************

	TreeView_SetItemText

	�A�C�e���̃^�C�g����ݒ�

******************************************************************************/

static BOOL TreeView_SetItemText(HWND hTreeView, HTREEITEM hItem, char *buf)
{
	TV_ITEM tvItem = { 0 };

	tvItem.mask = TVIF_TEXT;
	tvItem.hItem = hItem;
	tvItem.cchTextMax = BUFSIZE - 1;
	tvItem.pszText = buf;

	return TreeView_SetItem(hTreeView, &tvItem);
}


/******************************************************************************

	TreeView_SetName

	�f�B���N�g�����ƃc���[�r���[�A�C�e���̖��O��ݒ肷��

******************************************************************************/

BOOL TreeView_SetName(HWND hWnd, HTREEITEM hItem, char *NewName)
{
	char buf[BUFSIZE];
	char Tmpbuf[BUFSIZE];
	char OldFileName[BUFSIZE];
	char NewFileName[BUFSIZE];

	//�s���ȃt�@�C�����̏ꍇ�̓G���[���o��
	if(FileNameCheck(NewName) == FALSE){
		MessageBox(hWnd, EMSG_CHANGEFILENAME, EMSG_CHANGEFILENAME_TITLE, MB_ICONSTOP);
		return FALSE;
	}

	//�p�X���擾����
	*buf = '\0';
	TreeView_GetPath(GetDlgItem(hWnd, WWWC_TREE),
		TreeView_GetParent(GetDlgItem(hWnd, WWWC_TREE), hItem), buf, CuDir);

	//�Â��p�X�ƐV�����p�X���쐬����
	TreeView_GetItemInfo(GetDlgItem(hWnd, WWWC_TREE), hItem, Tmpbuf);
	wsprintf(OldFileName, "%s\\%s", buf, Tmpbuf);
	wsprintf(NewFileName, "%s\\%s", buf, NewName);

	//�t�@�C������ύX����
	if(MoveFile(OldFileName, NewFileName) == FALSE){
		//���s�����ꍇ�̓G���[���o�͂���
		ErrMsg(hWnd, GetLastError(), EMSG_CHANGEFILENAME_TITLE);
		return FALSE;
	}

	//�c���[�̃^�C�g����ύX���ă\�[�g����
	TreeView_SetItemText(GetDlgItem(hWnd, WWWC_TREE), hItem, NewName);
	TreeView_SortChildren(GetDlgItem(hWnd, WWWC_TREE),
		TreeView_GetParent(GetDlgItem(hWnd, WWWC_TREE), hItem), 0);

	//Undo�o�b�t�@�ɃZ�b�g
	SetTVTitleToUndo(hItem, Tmpbuf);
	return TRUE;
}


/******************************************************************************

	TreeView_GetItemInfo

	�A�C�e���̃^�C�g���ƃA�C�R�����擾

******************************************************************************/

int TreeView_GetItemInfo(HWND hTreeView, HTREEITEM hItem, char *buf)
{
	TV_ITEM tvItem = { 0 };

	tvItem.mask = TVIF_TEXT | TVIF_IMAGE;
	tvItem.hItem = hItem;
	tvItem.cchTextMax = BUFSIZE - 1;
	tvItem.pszText = buf;
	tvItem.iImage = 0;
	tvItem.iSelectedImage = 0;

	if(TreeView_GetItem(hTreeView, &tvItem) == FALSE){
		return -1;
	}
	return tvItem.iImage;
}


/******************************************************************************

	TreeView_GetListIndex

	�A�C�e���̈ʒu���擾

******************************************************************************/

int TreeView_GetListIndex(HWND hTreeView, HTREEITEM hItem)
{
	HTREEITEM pItem, cItem;
	int i;

	pItem = TreeView_GetParent(hTreeView, hItem);
	cItem = TreeView_GetChild(hTreeView, pItem);
	i = 0;
	while(cItem != NULL){
		if(cItem == hItem){
			return i;
		}
		cItem = TreeView_GetNextSibling(hTreeView, cItem);
		i++;
	}
	return -1;
}


/******************************************************************************

	TreeView_GetIndexToItem

	�w��ʒu�̃A�C�e�����擾

******************************************************************************/

HTREEITEM TreeView_GetIndexToItem(HWND hTreeView, HTREEITEM hItem, int Index)
{
	HTREEITEM cItem;
	int i;

	i = 0;
	cItem = TreeView_GetChild(hTreeView, hItem);
	while(cItem != NULL){
		if(Index == i){
			return cItem;
		}
		i++;
		cItem = TreeView_GetNextSibling(hTreeView, cItem);
	}
	return NULL;
}


/******************************************************************************

	TreeView_GetHiTestItem

	�}�E�X�̉��̃A�C�e�����擾

******************************************************************************/

HTREEITEM TreeView_GetHiTestItem(HWND hTreeView)
{
	TV_HITTESTINFO tvht = { 0 };
	POINT apos;
	RECT TvRect;

	if(hTreeView == NULL){
		return NULL;
	}

	//�}�E�X�̈ʒu���擾
	GetCursorPos((LPPOINT)&apos);
	GetWindowRect(hTreeView, (LPRECT)&TvRect);
	apos.x = apos.x - TvRect.left;
	apos.y = apos.y - TvRect.top;

	tvht.pt = apos;
	tvht.flags = TVHT_NOWHERE;
	tvht.hItem = NULL;
	return TreeView_HitTest(hTreeView, &tvht);
}


/******************************************************************************

	TreeView_SetlParam

	�A�C�e���ɏ����֘A����

******************************************************************************/

BOOL TreeView_SetlParam(HWND hTreeView, HTREEITEM hItem, long lParam)
{
	TV_ITEM tvit = { 0 };

	tvit.mask = TVIF_PARAM;
	tvit.hItem = hItem;
	tvit.lParam = (LPARAM)lParam;
	return TreeView_SetItem(hTreeView, &tvit);
}


/******************************************************************************

	TreeView_GetlParam

	�A�C�e���Ɋ֘A�t����ꂽ���̎擾

******************************************************************************/

void *TreeView_GetlParam(HWND hTreeView, HTREEITEM hItem)
{
	TV_ITEM tvit = { 0 };

	if(hItem == NULL){
		return NULL;
	}
	tvit.mask = TVIF_PARAM;
	tvit.hItem = hItem;
	tvit.lParam = 0;

	if(TreeView_GetItem(hTreeView, &tvit) == TRUE && tvit.lParam != 0){
		return (void *)tvit.lParam;
	}
	return NULL;
}


/******************************************************************************

	TreeView_SetState

	�A�C�e���̏�Ԃ̐ݒ�

******************************************************************************/

BOOL TreeView_SetState(HWND hTreeView, HTREEITEM hItem, int State, int mask)
{
	TV_ITEM tvit = { 0 };

	tvit.mask = TVIF_STATE;
	tvit.hItem = hItem;
	tvit.state = State;
	tvit.stateMask = mask;
	return TreeView_SetItem(hTreeView, &tvit);
}


/******************************************************************************

	TreeView_GetState

	�A�C�e���̏�Ԃ̎擾

******************************************************************************/

int TreeView_GetState(HWND hTreeView, HTREEITEM hItem, int mask)
{
	TV_ITEM tvit = { 0 };

	tvit.mask = TVIF_STATE;
	tvit.hItem = hItem;
	tvit.state = 0;
	tvit.stateMask = mask;

	TreeView_GetItem(hTreeView, &tvit);
	return tvit.state;
}


/******************************************************************************

	TreeView_SetIcon

	�A�C�e���̃A�C�R����ݒ�

******************************************************************************/

static BOOL TreeView_SetIcon(HWND hTreeView, HTREEITEM hItem, int Icon, int SelIcon)
{
	TV_ITEM tvit = { 0 };

	tvit.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE;
	tvit.hItem = hItem;
	tvit.iImage = Icon;
	tvit.iSelectedImage = SelIcon;

	return TreeView_SetItem(hTreeView, &tvit);
}


/******************************************************************************

	TreeView_UpdateList

	�c���[�r���[�̃A�C�e���ɑΉ��������X�g�r���[�A�C�e�����X�V

******************************************************************************/

BOOL TreeView_UpdateList(HWND hWnd, HTREEITEM hItem)
{
	int ListIndex;

	//�e�E�B���h�E���I���t�H���_�ł͂Ȃ��ꍇ�͊֐��𔲂���
	if(TreeView_GetParent(GetDlgItem(hWnd, WWWC_TREE), hItem) !=
		TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE))){
		return FALSE;
	}
	//���X�g�r���[�̃C���f�b�N�X���擾
	ListIndex = TreeView_GetListIndex(GetDlgItem(hWnd, WWWC_TREE), hItem);

	ListView_SetItemText(GetDlgItem(hWnd, WWWC_LIST), ListIndex, 0, LPSTR_TEXTCALLBACK);
	ListView_RedrawItems(GetDlgItem(hWnd, WWWC_LIST), ListIndex, ListIndex);
	UpdateWindow(GetDlgItem(hWnd, WWWC_LIST));
	return TRUE;
}


/******************************************************************************

	SetItemIcon

	�A�C�e���̏�Ԃɉ����ăA�C�R����ݒ肷��

******************************************************************************/

BOOL SetItemIcon(HWND hWnd, HTREEITEM hItem)
{
	struct TPTREE *tpTreeInfo;
	char buf[BUFSIZE];
	int Icon;

	if(hItem == RootItem || hItem == RecyclerItem){
		return FALSE;
	}
	tpTreeInfo = (struct TPTREE *)TreeView_GetlParam(GetDlgItem(hWnd, WWWC_TREE), hItem);
	if(tpTreeInfo == NULL){
		return FALSE;
	}

	//���݂̃A�C�R�����擾
	Icon = TreeView_GetItemInfo(GetDlgItem(hWnd, WWWC_TREE), hItem, buf);

	//�`�F�b�N��
	if(tpTreeInfo->Icon & TREEICON_CH){
		if(Icon == ICON_DIR_CLOSE_CH){
			return FALSE;
		}
		TreeView_SetIcon(GetDlgItem(hWnd, WWWC_TREE), hItem,
			ICON_DIR_CLOSE_CH, ICON_DIR_OPEN_CH);
		TreeView_UpdateList(hWnd, hItem);
		return TRUE;
	}
	//�q�����`�F�b�N��
	if(tpTreeInfo->Icon & TREEICON_CHECKCHILD){
		if(Icon == ICON_DIR_CLOSE_CHECKCHILD){
			return FALSE;
		}
		TreeView_SetIcon(GetDlgItem(hWnd, WWWC_TREE), hItem,
			ICON_DIR_CLOSE_CHECKCHILD, ICON_DIR_OPEN_CHECKCHILD);
		TreeView_UpdateList(hWnd, hItem);
		return TRUE;
	}
	//UP�A�C�R������
	if(tpTreeInfo->Icon & TREEICON_UP){
		if(Icon == ICON_DIR_CLOSE_UP){
			return FALSE;
		}
		TreeView_SetIcon(GetDlgItem(hWnd, WWWC_TREE), hItem,
			ICON_DIR_CLOSE_UP, ICON_DIR_OPEN_UP);
		TreeView_UpdateList(hWnd, hItem);
		return TRUE;
	}
	//�q����UP�A�C�R������
	if(tpTreeInfo->Icon & TREEICON_UPCHILD){
		if(Icon == ICON_DIR_CLOSE_UPCHILD){
			return FALSE;
		}
		TreeView_SetIcon(GetDlgItem(hWnd, WWWC_TREE), hItem,
			ICON_DIR_CLOSE_UPCHILD, ICON_DIR_OPEN_UPCHILD);
		TreeView_UpdateList(hWnd, hItem);
		return TRUE;
	}

	//�W���A�C�R��
	if(Icon != ICON_DIR_CLOSE){
		TreeView_SetIcon(GetDlgItem(hWnd, WWWC_TREE), hItem,
			ICON_DIR_CLOSE, ICON_DIR_OPEN);
		TreeView_UpdateList(hWnd, hItem);
		return TRUE;
	}
	return FALSE;
}


/******************************************************************************

	TreeView_SetParentIcon

	�e�A�C�e���̏�Ԃ�ݒ肷��

******************************************************************************/

static void TreeView_SetParentIcon(HWND hWnd, HTREEITEM hItem, int Icon)
{
	struct TPTREE *TmptpTreeInfo;
	HTREEITEM pItem;

	pItem = TreeView_GetParent(GetDlgItem(hWnd, WWWC_TREE), hItem);
	while(pItem != RootItem && pItem != RecyclerItem){
		TmptpTreeInfo = (struct TPTREE *)TreeView_GetlParam(GetDlgItem(hWnd, WWWC_TREE), pItem);
		if(TmptpTreeInfo != NULL){
			if((TmptpTreeInfo->Icon & Icon) != 0){
				break;
			}
			//��Ԃ�ݒ�
			TmptpTreeInfo->Icon |= Icon;
			SetItemIcon(hWnd, pItem);
		}
		pItem = TreeView_GetParent(GetDlgItem(hWnd, WWWC_TREE), pItem);
	}
}


/******************************************************************************

	TreeView_DeleteParentIcon

	�e�A�C�e���̏�Ԃ��폜����

******************************************************************************/

static void TreeView_DeleteParentIcon(HWND hWnd, HTREEITEM hItem, int Icon1, int Icon2)
{
	struct TPTREE *TmptpTreeInfo;
	HTREEITEM pItem, cItem;

	pItem = hItem;

	while(pItem != RootItem && pItem != RecyclerItem){
		cItem = TreeView_GetChild(GetDlgItem(hWnd, WWWC_TREE), pItem);
		while(cItem != NULL){
			TmptpTreeInfo = (struct TPTREE *)TreeView_GetlParam(GetDlgItem(hWnd, WWWC_TREE), cItem);
			if(TmptpTreeInfo != NULL &&
				((TmptpTreeInfo->Icon & Icon1) || (TmptpTreeInfo->Icon & Icon2))){
				break;
			}
			cItem = TreeView_GetNextSibling(GetDlgItem(hWnd, WWWC_TREE), cItem);
		}
		if(cItem != NULL){
			break;
		}

		TmptpTreeInfo = (struct TPTREE *)TreeView_GetlParam(GetDlgItem(hWnd, WWWC_TREE), pItem);
		if(TmptpTreeInfo != NULL && TmptpTreeInfo->Icon & Icon2){
			//��Ԃ��폜
			TmptpTreeInfo->Icon ^= Icon2;
			SetItemIcon(hWnd, pItem);
		}
		pItem = TreeView_GetParent(GetDlgItem(hWnd, WWWC_TREE), pItem);
	}
}


/******************************************************************************

	TreeView_SetIconState

	�c���r���[�A�C�e���̃A�C�R����Ԃ�ݒ肷��

******************************************************************************/

void CALLBACK TreeView_SetIconState(HWND hWnd, HTREEITEM hItem, long Param)
{
	struct TPTREE *tpTreeInfo;
	int i, ItemCnt, Icon;
	int UPFlag = 0;
	char buf[BUFSIZE];

	//���[�g�A�C�e���̏ꍇ�͊֐��𔲂���
	if(hItem == RootItem){
		return;
	}

	//���ݔ��̏ꍇ�͂��ݔ��̒��g�ɂ���ăA�C�R����ω�������
	if(hItem == RecyclerItem){
		Icon = TreeView_GetItemInfo(GetDlgItem(hWnd, WWWC_TREE), hItem, buf);
		if(CheckRecycler(hWnd) == FALSE){
			if(Icon != ICON_DIR_RECYCLER){
				TreeView_SetIcon(GetDlgItem(hWnd, WWWC_TREE), hItem,
					ICON_DIR_RECYCLER, ICON_DIR_RECYCLER);
				Icon = -1;
			}
		}else{
			if(Icon != ICON_DIR_RECYCLER_USE){
				TreeView_SetIcon(GetDlgItem(hWnd, WWWC_TREE), hItem,
					ICON_DIR_RECYCLER_USE, ICON_DIR_RECYCLER_USE);
				Icon = -1;
			}
		}
		if(Icon == -1 && TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE)) == RootItem){
			i = TreeView_GetListIndex(GetDlgItem(hWnd, WWWC_TREE), RecyclerItem);
			ListView_RedrawItems(GetDlgItem(hWnd, WWWC_LIST), i, i);
			UpdateWindow(GetDlgItem(hWnd, WWWC_LIST));
		}
		return;
	}

	//�c���[�A�C�e�����̎擾
	tpTreeInfo = (struct TPTREE *)TreeView_GetlParam(GetDlgItem(hWnd, WWWC_TREE), hItem);
	if(tpTreeInfo == NULL){
		TreeView_SetIcon(GetDlgItem(hWnd, WWWC_TREE), hItem, ICON_DIR_CLOSE, ICON_DIR_OPEN);
		return;
	}

	//�A�C�e����񂪓ǂݍ��܂�Ă��Ȃ��ꍇ�͓ǂݍ���
	if(tpTreeInfo->ItemList == NULL){
		if(ReadTreeMem(hWnd, hItem) == FALSE){
			TreeView_SetIcon(GetDlgItem(hWnd, WWWC_TREE), hItem, ICON_DIR_CLOSE, ICON_DIR_OPEN);
			return;
		}
	}

	//�c���[�̃A�C�e���̏����擾
	Icon = TreeView_GetItemInfo(GetDlgItem(hWnd, WWWC_TREE), hItem, buf);

	ItemCnt = tpTreeInfo->ItemListCnt;
	for(i = 0;i < ItemCnt;i++){
		if((*(tpTreeInfo->ItemList + i)) == NULL){
			continue;
		}

		//�`�F�b�N���̃A�C�e��
		if((*(tpTreeInfo->ItemList + i))->IconStatus == ST_CHECK){
			tpTreeInfo->Icon |= TREEICON_CH;
			SetItemIcon(hWnd, hItem);

			//�e���`�F�b�N����A�C�R���ɂ���
			TreeView_SetParentIcon(hWnd, hItem, TREEICON_CHECKCHILD);
			return;
		}

		//UP�A�C�e��
		if((*(tpTreeInfo->ItemList + i))->Status == ST_UP){
			//UP�A�C�e�����݃t���O���Z�b�g
			UPFlag = 1;
			//�`�F�b�N���ł͂Ȃ��ꍇ�̓��[�v�𔲂���
			if(gCheckFlag == 0){
				break;
			}
		}
	}

	//�`�F�b�N���t���O
	if(tpTreeInfo->Icon & TREEICON_CH){
		//�`�F�b�N���t���O�𖳂���
		tpTreeInfo->Icon ^= TREEICON_CH;

		TreeView_DeleteParentIcon(hWnd,
			TreeView_GetParent(GetDlgItem(hWnd, WWWC_TREE), hItem),
			TREEICON_CH, TREEICON_CHECKCHILD);
	}

	//UP�A�C�e�������݂���ꍇ
	if(UPFlag == 1){
		//UP�A�C�R���ɕύX
		tpTreeInfo->Icon |= TREEICON_UP;
		SetItemIcon(hWnd, hItem);

		//�e��UP����A�C�R���ɂ���
		TreeView_SetParentIcon(hWnd, hItem, TREEICON_UPCHILD);
		return;
	}

	if(tpTreeInfo->Icon & TREEICON_UP){
		//UP�A�C�R���̏���
		tpTreeInfo->Icon ^= TREEICON_UP;
		//�e�̃A�C�R����ύX
		TreeView_DeleteParentIcon(hWnd,
			TreeView_GetParent(GetDlgItem(hWnd, WWWC_TREE), hItem),
			TREEICON_UP, TREEICON_UPCHILD);

	}else if(tpTreeInfo->Icon & TREEICON_UPCHILD){
		//�q���̊m�F
		TreeView_DeleteParentIcon(hWnd, hItem, TREEICON_UP, TREEICON_UPCHILD);
	}

	//�A�C�R���̕ύX
	SetItemIcon(hWnd, hItem);
}


/******************************************************************************

	TreeView_NoFileDelete

	�Ή�����t�H���_�����݂��Ȃ��A�C�e�����폜����

******************************************************************************/

void CALLBACK TreeView_NoFileDelete(HWND hWnd, HTREEITEM hItem, long Param)
{
	HTREEITEM cItem, TmpItem;
	char path[BUFSIZE];

	TreeView_GetPath(GetDlgItem(hWnd, WWWC_TREE), hItem, path, CuDir);
	if(GetDirSerch(path) == FALSE && FindCheckItem(hWnd, hItem) == 0 && FindPropItem(hWnd, hItem) == 0){
		CallTreeItem(hWnd, hItem, (FARPROC)TreeView_FreeTreeItem, 1);
		TreeView_DeleteItem(GetDlgItem(hWnd, WWWC_TREE), hItem);
		return;
	}

	cItem = TreeView_GetChild(GetDlgItem(hWnd, WWWC_TREE), hItem);
	while(cItem != NULL){
		//�ċA
		TmpItem = TreeView_GetNextSibling(GetDlgItem(hWnd, WWWC_TREE), cItem);
		TreeView_NoFileDelete(hWnd, cItem, Param);
		cItem = TmpItem;
	}

}


/******************************************************************************

	TreeView_StartDragItem

	�c���r���[�A�C�e���̃h���b�O���h���b�v�J�n

******************************************************************************/

void TreeView_StartDragItem(HWND hWnd, HTREEITEM hItem)
{
	HTREEITEM pItem;
	UINT cf[CLIPFORMAT_CNT];
	int Effect;
	int ret;
	int cfcnt;
	BOOL NoFileFlag;

	DgdpItem = hItem;
	if(DgdpItem == NULL || DgdpItem == RootItem){
		DgdpItem = NULL;
		return;
	}

	//Alt�L�[��������Ă���ꍇ�̓t�@�C��������Ȃ�
	NoFileFlag = (GetAsyncKeyState(VK_MENU) < 0) ? TRUE : FALSE;

	//�t�H���_���̕ۑ�
	SaveTreeMem(hWnd, GetDlgItem(hWnd, WWWC_TREE), DgdpItem);

	Effect = DROPEFFECT_COPY;
	if(FindCheckItem(hWnd, DgdpItem) != 1){
		Effect |= DROPEFFECT_MOVE;
	}
	if(FindPropItem(hWnd, DgdpItem) != 1){
		Effect |= DROPEFFECT_MOVE;
	}

	WWWCDropFlag = FALSE;

	if(NoFileFlag == FALSE){
		if(DragDrop_CreateTreeDropFiles(hWnd, DgdpItem, DnD_DirName) == FALSE){
			return;
		}
	}

	DragItemIndex = -1;

	cf[0] = WWWC_ClipFormat;
	cf[1] = CF_TEXT;
	cf[2] = CF_HDROP;
	cfcnt = (NoFileFlag == FALSE) ? CLIPFORMAT_CNT : CLIPFORMAT_CNT - 1;

	ret = OLE_IDropSource_Start(hWnd, WM_DATAOBJECT_GETDATA, cf, cfcnt, Effect);

	if(ret != -1 && ret == DROPEFFECT_MOVE && WWWCDropFlag == TRUE){
		pItem = TreeView_GetParent(GetDlgItem(hWnd, WWWC_TREE), DgdpItem);
		TreeView_NoFileDelete(hWnd, DgdpItem, 0);

		if(pItem == TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE))){
			ListView_RefreshFolder(hWnd);
		}
		TreeView_SetIconState(hWnd, pItem, 0);
		CallTreeItem(hWnd, pItem, (FARPROC)TreeView_FreeItem, 1);

	}else{
		CallTreeItem(hWnd, DgdpItem, (FARPROC)TreeView_FreeItem, 1);
	}
	if(NoFileFlag == FALSE){
		FreeDropFiles();
	}
}


/******************************************************************************

	TreeView_SelItemChanging

	�c���[�r���[�A�C�e���̑I��ύX�`�F�b�N

******************************************************************************/

BOOL TreeView_SelItemChanging(HWND hWnd, HTREEITEM NewItem, HTREEITEM OldItem)
{
	if(NewItem == NULL){
		return FALSE;
	}
	WaitCursor(TRUE);

	//�t�@�C�����烊�X�g�̏����������ɓǂݍ���
	if(ReadTreeMem(hWnd, NewItem) == FALSE){
		WaitCursor(FALSE);
		MessageBox(hWnd, EMSG_DIRMOVE, EMSG_DIRMOVE_TITLE, MB_ICONSTOP);
		if(OldItem == NULL){
			return FALSE;
		}
		return TRUE;
	}
	WaitCursor(FALSE);
	return FALSE;
}


/******************************************************************************

	TreeView_SelItemChanged

	�c���[�r���[�A�C�e���̑I��ύX

******************************************************************************/

BOOL TreeView_SelItemChanged(HWND hWnd, HTREEITEM NewItem, HTREEITEM OldItem)
{
	//���X�g�r���[�̃A�C�e�������ׂč폜����
	if(NewItem == NULL){
		ListView_DeleteAllItems(GetDlgItem(hWnd, WWWC_LIST));
		return FALSE;
	}

	WaitCursor(TRUE);

	//���X�g�r���[�ɃA�C�e����\������
	ListView_ShowItem(hWnd, NewItem);
	ListView_SetItemState(GetDlgItem(hWnd, WWWC_LIST), 0, LVIS_FOCUSED, LVIS_FOCUSED);

	//�c���[�̃A�C�R����ݒ肷��
	TreeView_SetIconState(hWnd, NewItem, 0);

	//�O��I������Ă����A�C�e���̃��������������i�ۑ����[�h�j
	if(OldItem != NULL){
		TreeView_FreeItem(hWnd, OldItem, 1);
	}

	SetProtocolMenu(hWnd);
	SetSbText(hWnd);
	TreeView_SetHistory(NewItem);

	WaitCursor(FALSE);
	return FALSE;
}


/******************************************************************************

	TreeView_SetHistory

	TreeView�A�C�e���𗚗��ɒǉ�

******************************************************************************/

static void TreeView_SetHistory(HTREEITEM NewItem)
{
	int i;

	if(HistoryFlag == TRUE) return;
	if(HistoryIndex < HISTORYCNT){
		HistoryIndex++;
		HistoryItem[HistoryIndex] = NewItem;
		for(i = HistoryIndex + 1; i < HISTORYCNT; i++){
			HistoryItem[i] = NULL;
		}
		return;
	}
	for(i = 0; i < HISTORYCNT; i++){
		HistoryItem[i] = HistoryItem[i + 1];
	}
	HistoryItem[HISTORYCNT] = NewItem;
}


/******************************************************************************

	TreeView_CheckHistory

	�ړ��\���`�F�b�N

******************************************************************************/

BOOL TreeView_CheckHistory(HWND hWnd, BOOL NextFlag)
{
	HTREEITEM hItem;
	int i;

	hItem = TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE));
	i = HistoryIndex;

	if(NextFlag == TRUE){
		for(i = HistoryIndex + 1; i < HISTORYCNT &&
			(HistoryItem[i] == NULL || HistoryItem[i] == hItem ||
			IsTreeItem(hWnd, HistoryItem[i]) == FALSE); i++);
		if(i >= HISTORYCNT) return FALSE;
	}else{
		for(i = HistoryIndex - 1; i > 0 &&
			(HistoryItem[i] == NULL || HistoryItem[i] == hItem ||
			IsTreeItem(hWnd, HistoryItem[i]) == FALSE); i--);
		if(i <= 0) return FALSE;
	}
	return TRUE;
}


/******************************************************************************

	TreeView_PrevHistory

	�O�ɖ߂�

******************************************************************************/

BOOL TreeView_PrevHistory(HWND hWnd)
{
	HTREEITEM hItem;
	int i;
	BOOL ret;

	hItem = TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE));
	for(i = HistoryIndex - 1; i > 0 &&
		(HistoryItem[i] == NULL || HistoryItem[i] == hItem ||
		IsTreeItem(hWnd, HistoryItem[i]) == FALSE); i--);
	if(i <= 0) return FALSE;

	HistoryFlag = TRUE;
	ret = (BOOL)TreeView_SelectItem(GetDlgItem(hWnd, WWWC_TREE), HistoryItem[i]);
	HistoryFlag = FALSE;
	HistoryIndex = i;

	if(ret == FALSE){
		return TreeView_PrevHistory(hWnd);
	}
	return TRUE;
}


/******************************************************************************

	TreeView_NextHistory

	���ɐi��

******************************************************************************/

BOOL TreeView_NextHistory(HWND hWnd)
{
	HTREEITEM hItem;
	int i;
	BOOL ret;

	hItem = TreeView_GetSelection(GetDlgItem(hWnd, WWWC_TREE));
	for(i = HistoryIndex + 1; i < HISTORYCNT &&
		(HistoryItem[i] == NULL || HistoryItem[i] == hItem ||
		IsTreeItem(hWnd, HistoryItem[i]) == FALSE); i++);
	if(i >= HISTORYCNT) return FALSE;

	HistoryFlag = TRUE;
	ret = (BOOL)TreeView_SelectItem(GetDlgItem(hWnd, WWWC_TREE), HistoryItem[i]);
	HistoryFlag = FALSE;
	HistoryIndex = i;

	if(ret == FALSE){
		return TreeView_NextHistory(hWnd);
	}
	return TRUE;
}


/******************************************************************************

	TreeView_NotifyProc

	�c���[�r���[�C�x���g

******************************************************************************/

LRESULT TreeView_NotifyProc(HWND hWnd, LPARAM lParam)
{
	TV_DISPINFO *ptv = (TV_DISPINFO *)lParam;
	NM_TREEVIEW *Tv = (NM_TREEVIEW *)lParam;
	NMHDR *CForm = (NMHDR *)lParam;

	switch(ptv->hdr.code)
	{
	case TVN_BEGINLABELEDIT:
	case TVN_ENDLABELEDIT:
		return SendMessage(hWnd, WM_TV_EVENT, ptv->hdr.code, lParam);
	}

	switch(Tv->hdr.code)
	{
	case TVN_BEGINDRAG:
	case TVN_BEGINRDRAG:
	case TVN_SELCHANGING:
	case TVN_SELCHANGED:
		return SendMessage(hWnd, WM_TV_EVENT, Tv->hdr.code, lParam);
	}

	switch(CForm->code)
	{
	case NM_SETFOCUS:
	case NM_RCLICK:
		return SendMessage(hWnd, WM_TV_EVENT, CForm->code, lParam);
	}
	return FALSE;
}
/* End of source */
