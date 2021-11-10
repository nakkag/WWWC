/**************************************************************************

	WWWC

	File.c

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
#include <shlobj.h>

#include "General.h"


/**************************************************************************
	Define
**************************************************************************/

#define CREATEFILE_MAX			10
#define COPYMAX					100

typedef DWORD (WINAPI *FUNC_GETDISKFREESPACEEX) (LPCTSTR, PULARGE_INTEGER, PULARGE_INTEGER, PULARGE_INTEGER);


/**************************************************************************
	Global Variables
**************************************************************************/

static HINSTANCE hDll = NULL;
static FUNC_GETDISKFREESPACEEX _GetDiskFreeSpaceEx = NULL;

//�O���Q��
extern HINSTANCE g_hinst;
extern char CuDir[];
extern HWND WWWCWnd;					//�{��
extern HTREEITEM RootItem;
extern HTREEITEM RecyclerItem;
extern int UpdateItemFlag;
extern int SaveDriveCheck;


/**************************************************************************
	Local Function Prototypes
**************************************************************************/

static char *AllocItemStr(char *t, char **ret);
static void SetExecCommandLine(HWND hWnd, char *buf, char *ret, struct TPITEM *tpItemInfo);


/******************************************************************************

	GetFileSerchSize

	�t�@�C���̃T�C�Y���擾����

******************************************************************************/

long GetFileSerchSize(char *FileName)
{
	WIN32_FIND_DATA FindData;
	HANDLE hFindFile;

	if((hFindFile = FindFirstFile(FileName, &FindData)) == INVALID_HANDLE_VALUE){
		return -1;
	}
	FindClose(hFindFile);

	if((FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0){
		//�f�B���N�g���ł͂Ȃ��ꍇ�̓T�C�Y��Ԃ�
		return (long)FindData.nFileSizeLow;
	}
	return -1;
}


/******************************************************************************

	GetFileMakeDay

	�t�@�C���̍쐬�������擾����

******************************************************************************/

BOOL GetFileMakeDay(char *FileName, char *buf)
{
	WIN32_FIND_DATA FindData;
	HANDLE hFindFile;
	FILETIME lFileTime;
	SYSTEMTIME fSysTime;
	char fDay[BUFSIZE];
	char fTime[BUFSIZE];

	if((hFindFile = FindFirstFile(FileName, &FindData)) == INVALID_HANDLE_VALUE){
		return FALSE;
	}
	FindClose(hFindFile);

	FileTimeToLocalFileTime(&(FindData.ftCreationTime), &lFileTime);
	FileTimeToSystemTime(&lFileTime, &fSysTime);

	if(GetDateFormat(0, DATE_LONGDATE, &fSysTime, NULL, fDay, BUFSIZE - 1) == 0){
		return FALSE;
	}
	if(GetTimeFormat(0, 0, &fSysTime, NULL, fTime, BUFSIZE - 1) == 0){
		return FALSE;
	}
	wsprintf(buf, FILEDATE_FORMAT, fDay, fTime);

	return TRUE;
}


/******************************************************************************

	GetDirSerch

	�f�B���N�g�������݂��邩�`�F�b�N����

******************************************************************************/

BOOL GetDirSerch(char *fPath)
{
	WIN32_FIND_DATA FindData;
	HANDLE hFindFile;

	if((hFindFile = FindFirstFile(fPath, &FindData)) == INVALID_HANDLE_VALUE){
		return FALSE;
	}
	FindClose(hFindFile);

	if(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
		//�f�B���N�g�������݂����ꍇ
		return TRUE;
	}
	return FALSE;
}


/******************************************************************************

	GetPathToFilename

	�p�X����t�@�C�������擾����

******************************************************************************/

BOOL GetPathToFilename(char *fPath, char *ret)
{
	WIN32_FIND_DATA FindData;
	HANDLE hFindFile;

	if((hFindFile = FindFirstFile(fPath, &FindData)) == INVALID_HANDLE_VALUE){
		return FALSE;
	}
	FindClose(hFindFile);

	lstrcpy(ret, FindData.cFileName);
	return TRUE;
}


/******************************************************************************

	CopyDirTree

	�f�B���N�g���̃R�s�[

******************************************************************************/

int CopyDirTree(HWND hWnd, char *Path, char *NewPath, char *name, char *ret)
{
	struct TPITEM **tpFromItemInfo;
	WIN32_FIND_DATA FindData;
	HANDLE hFindFile;
	HTREEITEM hItem;
	char sPath[MAX_PATH];
	char nPath[MAX_PATH];
	char buf[MAX_PATH];
	char buf2[MAX_PATH];
	char *ItemPath;
	int cnt = 1;
	int FromCnt, i;

	if(GetDirSerch(Path) == FALSE){
		return -1;
	}

	//�R�s�[��̖��O�̐ݒ�
	wsprintf(nPath, "%s\\%s", NewPath, name);
	if(lstrcmpi(nPath, Path) == 0){
		wsprintf(buf, STR_COPYNAME"%s", name);
		while(GetDirSerch(nPath) == TRUE){
			wsprintf(nPath, "%s\\%s", NewPath, buf);
			if(ret != NULL){
				lstrcpy(ret, buf);
			}
			//�R�s�[�Ɏ��s��������ꍇ�̓G���[�Ƃ���
			cnt++;
			if(cnt > COPYMAX){
				return -1;
			}
			wsprintf(buf, STR_COPYNAME_CNT"%s", cnt, name);
		}
		CreateDirectory(nPath, NULL);

	}else{
		if(GetDirSerch(nPath) == FALSE){
			CreateDirectory(nPath, NULL);
		}
	}
	//�����p�X
	wsprintf(sPath, "%s\\*", Path);

	if((hFindFile = FindFirstFile(sPath, &FindData)) == INVALID_HANDLE_VALUE){
		return 0;
	}

	do{
		//�f�B���N�g���̃R�s�[
		if(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
			if(lstrcmp(FindData.cFileName, ".") != 0 && lstrcmp(FindData.cFileName, "..") != 0){
				wsprintf(buf, "%s\\%s", Path, FindData.cFileName);
				if(CopyDirTree(hWnd, buf, nPath, FindData.cFileName, NULL) == -1){
					FindClose(hFindFile);
					return -1;
				}
			}
			continue;
		}

		wsprintf(buf, "%s\\%s", Path, FindData.cFileName);
		wsprintf(buf2, "%s\\%s", nPath, FindData.cFileName);

		//�t�@�C���̃R�s�[
		if(lstrcmpi(FindData.cFileName, DATAFILENAME) != 0 ||
			lstrcmpni(nPath, CuDir, lstrlen(CuDir)) != 0 || hWnd == NULL){
			CopyFile(buf, buf2, TRUE);
			continue;
		}

		//�]����Ƀt�@�C�������݂��Ȃ��ꍇ�́A�t�@�C���̃R�s�[���s��
		ItemPath = nPath + lstrlen(CuDir);
		hItem = TreeView_FindItemPath(GetDlgItem(hWnd, WWWC_TREE), RootItem, ItemPath);
		if(hItem == NULL){
			CopyFile(buf, buf2, TRUE);
			continue;
		}
		//�]�����̃t�@�C������A�C�e�����X�g���擾����
		tpFromItemInfo = ReadItemList(buf, &FromCnt, NULL);
		if(tpFromItemInfo == NULL){
			continue;
		}
		//�]����ɃA�C�e����ǉ�����
		for(i = 0;i < FromCnt;i++){
			if(*(tpFromItemInfo + i) == NULL){
				continue;
			}

			//�A�C�e���̒ǉ�
			if(Item_Add(hWnd, hItem, *(tpFromItemInfo + i)) != -1){
				*(tpFromItemInfo + i) = NULL;
			}
			//�R�s�[���L�����Z������Ă���ꍇ
			if(UpdateItemFlag == UF_CANCEL){
				break;
			}
		}
		FreeItemList(tpFromItemInfo, FromCnt, FALSE);
		GlobalFree(tpFromItemInfo);

		TreeView_SetIconState(hWnd, hItem, 0);
		TreeView_FreeItem(hWnd, hItem, 1);

		if(UpdateItemFlag == UF_CANCEL){
			FindClose(hFindFile);
			return -1;
		}
	} while(FindNextFile(hFindFile, &FindData) == TRUE);

	FindClose(hFindFile);
	return 0;
}


/******************************************************************************

	DeleteDirTree

	�f�B���N�g���̍폜

******************************************************************************/

BOOL DeleteDirTree(char *Path, BOOL AllFlag)
{
	WIN32_FIND_DATA FindData;
	HANDLE hFindFile;
	char sPath[MAX_PATH];
	char buf[MAX_PATH];

	if(Path == NULL || *Path == '\0'){
		return FALSE;
	}
	wsprintf(sPath, "%s\\*", Path);

	if((hFindFile = FindFirstFile(sPath, &FindData)) == INVALID_HANDLE_VALUE){
		return FALSE;
	}

	do{
		if(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
			if(lstrcmp(FindData.cFileName, ".") != 0 && lstrcmp(FindData.cFileName, "..") != 0){
				wsprintf(buf, "%s\\%s", Path, FindData.cFileName);
				DeleteDirTree(buf, AllFlag);
				RemoveDirectory(buf);
			}
		}else{
			if(AllFlag == TRUE ||
				lstrcmpi(FindData.cFileName, DATAFILENAME) == 0 ||
				lstrcmpi(FindData.cFileName, FOLDERFILENAME) == 0){
				wsprintf(buf, "%s\\%s", Path, FindData.cFileName);
				DeleteFile(buf);
			}
		}
	} while(FindNextFile(hFindFile, &FindData) == TRUE);

	FindClose(hFindFile);
	return TRUE;
}


/******************************************************************************

	SetDirTree

	�f�B���N�g������c���[�A�C�e�����쐬����

******************************************************************************/

void SetDirTree(HWND hTreeView, char *Path, HTREEITEM hItem)
{
	WIN32_FIND_DATA FindData;
	HANDLE hFindFile;
	char sPath[MAX_PATH];
	char buf[MAX_PATH];
	HTREEITEM pItem;

	wsprintf(sPath, "%s\\*", Path);

	if((hFindFile = FindFirstFile(sPath, &FindData)) == INVALID_HANDLE_VALUE){
		return;
	}

	do{
		if((FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0){
			continue;
		}
		if(lstrcmp(FindData.cFileName, ".") == 0 || lstrcmp(FindData.cFileName, "..") == 0){
			continue;
		}
		wsprintf(buf, "%s\\%s", Path, FindData.cFileName);

		//�c���[�A�C�e�������݂��Ȃ��ꍇ�͒ǉ����s��
		if((pItem = TreeView_CheckName(hTreeView, hItem, FindData.cFileName)) == NULL){
			pItem = TreeView_AllocItem(hTreeView, hItem, (HTREEITEM)TVI_SORT, FindData.cFileName,
				ICON_DIR_CLOSE, ICON_DIR_OPEN);
			GetDirInfo(hTreeView, buf, pItem);
			TreeView_SetIconState(GetParent(hTreeView), pItem, 0);
		}
		if(RecyclerItem == pItem){
			GetDirInfo(hTreeView, buf, pItem);
		}

		//�ċA
		SetDirTree(hTreeView, buf, pItem);
	} while(FindNextFile(hFindFile, &FindData) == TRUE);

	FindClose(hFindFile);
}


/******************************************************************************

	FindTreeItem

	�A�C�e�������݂��邩���ׂ�

******************************************************************************/

void CALLBACK FindTreeDir(HWND hWnd, HTREEITEM hItem, long Param)
{
	char buf[BUFSIZE];

	if(hItem == RootItem || hItem == RecyclerItem){
		return;
	}

	TreeView_GetPath(GetDlgItem(hWnd, WWWC_TREE), hItem, buf, CuDir);

	if(GetDirSerch(buf) == FALSE){
		//�c���[�̍폜
		CallTreeItem(hWnd, hItem, (FARPROC)TreeView_FreeItem, 0);
		TreeView_DeleteItem(GetDlgItem(hWnd, WWWC_TREE), hItem);
	}
}


/******************************************************************************

	SaveDirTree

	�c���[�A�C�e������ۑ�

******************************************************************************/

void SaveDirTree(HWND hTreeView, char *Path, HTREEITEM hItem)
{
	WIN32_FIND_DATA FindData;
	HANDLE hFindFile;
	char sPath[MAX_PATH];
	char buf[MAX_PATH];
	HTREEITEM pItem;

	wsprintf(sPath, "%s\\*", Path);

	if((hFindFile = FindFirstFile(sPath, &FindData)) == INVALID_HANDLE_VALUE){
		return;
	}

	do{
		if((FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0){
			continue;
		}
		if(lstrcmp(FindData.cFileName, ".") == 0 || lstrcmp(FindData.cFileName, "..") == 0){
			continue;
		}
		//�t�H���_���̕ۑ�
		wsprintf(buf, "%s\\%s", Path, FindData.cFileName);
		if((pItem = TreeView_CheckName(hTreeView, hItem, FindData.cFileName)) != NULL){
			PutDirInfo(hTreeView, buf, pItem);
		}

		//�ċA
		SaveDirTree(hTreeView, buf, pItem);
	} while(FindNextFile(hFindFile, &FindData) == TRUE);

	FindClose(hFindFile);
}


/******************************************************************************

	mkDirStr

	�f�B���N�g���̊K�w���쐬����

******************************************************************************/

void mkDirStr(char *buf)
{
	char tmp[MAX_PATH];
	char *p, *r;

	p = buf;
	r = tmp;
	while(*p != '\0'){
		if(IsDBCSLeadByte((BYTE)*p) == TRUE){
			*(r++) = *(p++);
			*(r++) = *(p++);
		}else if(*p == '\\' || *p == '/'){
			*r = '\0';
			//�f�B���N�g���̍쐬
			if(GetDirSerch(tmp) == FALSE){
				CreateDirectory(tmp, NULL);
			}
			*(r++) = *(p++);
		}else{
			*(r++) = *(p++);
		}
	}
	*r = '\0';
	if(GetDirSerch(tmp) == FALSE){
		CreateDirectory(tmp, NULL);
	}
}


/******************************************************************************

	AllocItemStr

	��������m�ۂ��ăR�s�[

******************************************************************************/

static char *AllocItemStr(char *buf, char **ret)
{
	char *p, *r;
	char *EndPoint;

	if(*buf == '\0'){
		return buf;
	}

	//�����̋�؂�ʒu�̎擾
	for(EndPoint = buf; *EndPoint != '\t' && *EndPoint != '\r' && *EndPoint != '\n' && *EndPoint != '\0'; EndPoint++);

	*ret = (char *)GlobalAlloc(GMEM_FIXED, EndPoint - buf + 2);
	if(*ret == NULL){
		abort();
	}
	p = buf;
	r = *ret;
	while(EndPoint > p){
		*(r++) = *(p++);
	}
	*r = '\0';
	return EndPoint;
}


/******************************************************************************

	LineSetItemInfo

	�A�C�e���̕����񂩂�A�C�e�����쐬����

******************************************************************************/

char *LineSetItemInfo(struct TPITEM *tpItemList, char *t)
{
	char Tmpbuf[BUFSIZE];
	char *r;

	if(tpItemList == NULL){
		return t;
	}

	//Title
	t = AllocItemStr(t, &tpItemList->Title);
	if(tpItemList->Title == NULL){
		tpItemList->Title = (char *)GlobalAlloc(GMEM_FIXED, lstrlen(STR_NOTITLE) + 1);
		if(tpItemList->Title == NULL){
			abort();
		}
		lstrcpy(tpItemList->Title, STR_NOTITLE);
	}
	if(*t != '\t') return t;

	//CheckURL
	t = AllocItemStr(++t, &tpItemList->CheckURL);
	if(*t != '\t') return t;

	//Size
	t = AllocItemStr(++t, &tpItemList->Size);
	if(*t != '\t') return t;

	//Date
	t = AllocItemStr(++t, &tpItemList->Date);
	if(*t != '\t') return t;

	//Status
	for(t++, r = Tmpbuf;*t != '\t' && *t != '\r' && *t != '\n' && *t != '\0';t++, r++){
		*r = *t;
	}
	*r = '\0';
	tpItemList->Status = atoi(Tmpbuf);
	if(*t != '\t') return t;

	//CheckDate
	t = AllocItemStr(++t, &tpItemList->CheckDate);
	if(*t != '\t') return t;

	//OldSize
	t = AllocItemStr(++t, &tpItemList->OldSize);
	if(*t != '\t') return t;

	//OldDate
	t = AllocItemStr(++t, &tpItemList->OldDate);
	if(*t != '\t') return t;

	//ViewURL
	t = AllocItemStr(++t, &tpItemList->ViewURL);
	if(*t != '\t') return t;

	//Option1
	t = AllocItemStr(++t, &tpItemList->Option1);
	if(*t != '\t') return t;

	//Option2
	t = AllocItemStr(++t, &tpItemList->Option2);
	if(*t != '\t') return t;

	//Comment
	t = AllocItemStr(++t, &tpItemList->Comment);
	if(*t != '\t') return t;

	//Status
	for(t++, r = Tmpbuf;*t != '\t' && *t != '\r' && *t != '\n' && *t != '\0';t++, r++){
		*r = *t;
	}
	*r = '\0';
	tpItemList->CheckSt = atoi(Tmpbuf);
	if(*t != '\t') return t;

	//ErrStatus
	t = AllocItemStr(++t, &tpItemList->ErrStatus);
	if(*t != '\t') return t;

	//DLLData1
	t = AllocItemStr(++t, &tpItemList->DLLData1);
	if(*t != '\t') return t;

	//DLLData2
	t = AllocItemStr(++t, &tpItemList->DLLData2);
	for(; *t != '\0' && *t != '\r' && *t != '\n'; t++);
	return t;
}


/******************************************************************************

	ReadItemList

	�t�@�C������A�C�e�����X�g���쐬����

******************************************************************************/

struct TPITEM **ReadItemList(char *FileName, int *cnt, HTREEITEM hItem)
{
	struct TPITEM **tpRetItemList;
	struct TPITEM **tpItemList;
	HANDLE hFile;
	char *buf;
	char *p;
	long FileSize;
	int LineCnt = 0;
	DWORD ret;

	*cnt = 0;

	FileSize = GetFileSerchSize(FileName);
	if(FileSize <= 0){
		tpItemList = (struct TPITEM **)GlobalAlloc(GPTR, 0);
		if(tpItemList == NULL){
			abort();
		}
		return tpItemList;
	}

	//�t�@�C�����J��
	hFile = CreateFile(FileName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == NULL || hFile == (HANDLE)-1){
		return NULL;
	}
	buf = (char *)GlobalAlloc(GMEM_FIXED, FileSize + 1);
	if(buf == NULL){
		CloseHandle(hFile);
		return NULL;
	}
	//�t�@�C����ǂݍ���
	if(ReadFile(hFile, buf, FileSize, &ret, NULL) == FALSE){
		GlobalFree(buf);
		CloseHandle(hFile);
		return NULL;
	}
	CloseHandle(hFile);

	FileSize = ret;
	*(buf + FileSize) = '\0';

	//�s���̃J�E���g
	for(p = buf; *p != '\0'; p++){
		if(*p == '\n'){
			LineCnt++;
		}
	}
	*cnt = LineCnt;

	//�ǂݍ��ݎ��s
	if(FileSize != 0 && LineCnt == 0){
		GlobalFree(buf);
		return NULL;
	}

	//�A�C�e�������m��
	tpRetItemList = (struct TPITEM **)GlobalAlloc(GPTR, sizeof(struct TPITEM *) * (*cnt));
	if(tpRetItemList == NULL){
		abort();
	}
	tpItemList = tpRetItemList;
	//�A�C�e�������擾
	p = buf;
	while(*p != '\0'){
		if((*tpItemList = (struct TPITEM *)GlobalAlloc(GPTR, sizeof(struct TPITEM))) == NULL){
			abort();
		}
		(*tpItemList)->iSize = sizeof(struct TPITEM);
		(*tpItemList)->hItem = hItem;

		//�A�C�e����񕶎��񂩂�A�C�e�����쐬
		p = LineSetItemInfo(*tpItemList, p);
		for(; (*p == '\r' || *p == '\n'); p++);
		tpItemList++;
	}

	GlobalFree(buf);
	return tpRetItemList;
}


/******************************************************************************

	ReadTreeMem

	�t�@�C������A�C�e������ǂݍ���

******************************************************************************/

BOOL ReadTreeMem(HWND hWnd, HTREEITEM hItem)
{
	struct TPTREE *tpTreeInfo;
	char buf[MAX_PATH];
	char FileName[MAX_PATH];

	tpTreeInfo = (struct TPTREE *)TreeView_GetlParam(GetDlgItem(hWnd, WWWC_TREE), hItem);
	if(tpTreeInfo == NULL){
		return FALSE;
	}

	//���ɓǂݍ��܂�Ă���ꍇ
	if(tpTreeInfo->ItemList != NULL){
		return TRUE;
	}

	TreeView_GetPath(GetDlgItem(hWnd, WWWC_TREE), hItem, buf, CuDir);
	mkDirStr(buf);
	wsprintf(FileName, "%s\\"DATAFILENAME, buf);

	tpTreeInfo->ItemList = ReadItemList(FileName, &tpTreeInfo->ItemListCnt, hItem);
	if(tpTreeInfo->ItemList == NULL){
		return FALSE;
	}

	//�`�F�b�N�ݒ�
	FolderCheckIni(hWnd, hItem);
	return TRUE;
}


/******************************************************************************

	ItemSize

	�A�C�e����񂩂�A�C�e���𕶎���ɂ����Ƃ��̃T�C�Y���擾����

******************************************************************************/

int ItemSize(struct TPITEM *tpItemInfo)
{
	#define GETSTRLEN(str)	((str != NULL) ? (cnt + lstrlen(str) + 1) : (cnt + 1))

	int cnt = 0;

	if(tpItemInfo == NULL){
		return 0;
	}

	cnt = GETSTRLEN(tpItemInfo->Title);
	cnt = GETSTRLEN(tpItemInfo->CheckURL);
	cnt = GETSTRLEN(tpItemInfo->Size);
	cnt = GETSTRLEN(tpItemInfo->Date);

//	wsprintf(Tmpbuf, "%ld", tpItemInfo->Status);
//	cnt += lstrlen(Tmpbuf) + 1;
	cnt += 2;

	cnt = GETSTRLEN(tpItemInfo->CheckDate);
	cnt = GETSTRLEN(tpItemInfo->OldSize);
	cnt = GETSTRLEN(tpItemInfo->OldDate);
	cnt = GETSTRLEN(tpItemInfo->ViewURL);
	cnt = GETSTRLEN(tpItemInfo->Option1);
	cnt = GETSTRLEN(tpItemInfo->Option2);
	cnt = GETSTRLEN(tpItemInfo->Comment);

//	wsprintf(Tmpbuf, "%ld", tpItemInfo->CheckSt);
//	cnt += lstrlen(Tmpbuf) + 1;
	cnt += 2;

	cnt = GETSTRLEN(tpItemInfo->ErrStatus);
	cnt = GETSTRLEN(tpItemInfo->DLLData1);
	if(tpItemInfo->DLLData2 != NULL) cnt += lstrlen(tpItemInfo->DLLData2);

	cnt += 2;
	return cnt;
}


/******************************************************************************

	SetItemString

	�A�C�e����񂩂�A�C�e���̕�������쐬����

******************************************************************************/

char *SetItemString(struct TPITEM *tpItemInfo, char *p)
{
	char Tmpbuf[BUFSIZE];

	if(tpItemInfo == NULL){
		return p;
	}

	p = iStrCpy(p, tpItemInfo->Title);
	*(p++) = '\t';

	p = iStrCpy(p, tpItemInfo->CheckURL);
	*(p++) = '\t';

	p = iStrCpy(p, tpItemInfo->Size);
	*(p++) = '\t';

	p = iStrCpy(p, tpItemInfo->Date);
	*(p++) = '\t';

	wsprintf(Tmpbuf, "%ld", (tpItemInfo->Status < 10) ? tpItemInfo->Status : 0);
	p = iStrCpy(p, Tmpbuf);
	*(p++) = '\t';

	p = iStrCpy(p, tpItemInfo->CheckDate);
	*(p++) = '\t';

	p = iStrCpy(p, tpItemInfo->OldSize);
	*(p++) = '\t';

	p = iStrCpy(p, tpItemInfo->OldDate);
	*(p++) = '\t';

	p = iStrCpy(p, tpItemInfo->ViewURL);
	*(p++) = '\t';

	p = iStrCpy(p, tpItemInfo->Option1);
	*(p++) = '\t';

	p = iStrCpy(p, tpItemInfo->Option2);
	*(p++) = '\t';

	p = iStrCpy(p, tpItemInfo->Comment);
	*(p++) = '\t';

	wsprintf(Tmpbuf, "%ld", (tpItemInfo->CheckSt < 10) ? tpItemInfo->CheckSt : 0);
	p = iStrCpy(p, Tmpbuf);
	*(p++) = '\t';

	p = iStrCpy(p, tpItemInfo->ErrStatus);
	*(p++) = '\t';

	p = iStrCpy(p, tpItemInfo->DLLData1);
	*(p++) = '\t';

	p = iStrCpy(p, tpItemInfo->DLLData2);

	*(p++) = '\r';
	*(p++) = '\n';
	return p;
}


/******************************************************************************

	SaveItemList

	�A�C�e�����X�g���t�@�C���ɕۑ�����

******************************************************************************/

int SaveItemList(HWND hWnd, char *FileName, struct TPITEM **tpItemList, int ItemCnt)
{
	HANDLE hFile;
	char *buf;
	char *p;
	int i, sLen;
	int ErrorCode;
	DWORD ret;

	if(*FileName == '\0'){
		return -1;
	}

	//�ۑ�����T�C�Y���擾
	sLen = 0;
	for(i = 0;i < ItemCnt;i++){
		sLen += ItemSize(*(tpItemList + i));
	}
	if(sLen == 0){
		hFile = CreateFile(FileName, GENERIC_READ | GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if(hFile != NULL && hFile != (HANDLE)-1){
			CloseHandle(hFile);
		}
		return 1;
	}

	//�f�B�X�N�e�ʂ��`�F�b�N����
	SetCurrentDirectory(CuDir);
	if(SaveDriveCheck == 1){
		if(_GetDiskFreeSpaceEx != NULL){
			ULARGE_INTEGER FreeBytesAvailableToCaller;
			ULARGE_INTEGER TotalNumberOfBytes;
			ULARGE_INTEGER TotalNumberOfFreeBytes;

			if((*_GetDiskFreeSpaceEx)(NULL, &FreeBytesAvailableToCaller, &TotalNumberOfBytes, &TotalNumberOfFreeBytes) == 0){
				ErrMsg(hWnd, GetLastError(), EMSG_SAVE_TITLE);
				return -1;
			}
			if(FreeBytesAvailableToCaller.QuadPart < (DWORDLONG)sLen){
				MessageBox(hWnd, EMSG_DISKFULL, EMSG_SAVE_TITLE, MB_ICONERROR);
				return -1;
			}

		}else{
			DWORD SectorsPerCluster;
			DWORD BytesPerSectors;
			DWORD NumberOfFreeClusters;
			DWORD TotalNumberOfClusters;

			if(GetDiskFreeSpace(NULL, &SectorsPerCluster, &BytesPerSectors, &NumberOfFreeClusters, &TotalNumberOfClusters) == FALSE){
				ErrMsg(hWnd, GetLastError(), EMSG_SAVE_TITLE);
				return -1;
			}
			if((SectorsPerCluster * BytesPerSectors * NumberOfFreeClusters) < (unsigned)sLen){
				MessageBox(hWnd, EMSG_DISKFULL, EMSG_SAVE_TITLE, MB_ICONERROR);
				return -1;
			}
		}
	}

	buf = (char *)GlobalAlloc(GMEM_FIXED, sLen);
	if(buf == NULL){
		ErrMsg(hWnd, GetLastError(), EMSG_SAVE_TITLE);
		return 0;
	}
	*buf = '\0';

	//�A�C�e����񕶎�����쐬
	p = buf;
	for(i = 0;i < ItemCnt;i++){
		p = SetItemString(*(tpItemList + i), p);
	}

	//�ۑ�����t�@�C�����J��
	hFile = CreateFile(FileName, GENERIC_READ | GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == NULL || hFile == (HANDLE)-1){
		ErrMsg(hWnd, GetLastError(), EMSG_SAVE_TITLE);
		return 0;
	}

	if(WriteFile(hFile, buf, sLen, &ret, NULL) == FALSE){
		ErrorCode = GetLastError();
		CloseHandle(hFile);
		ErrMsg(hWnd, ErrorCode, EMSG_SAVE_TITLE);
		return 0;
	}
	GlobalFree(buf);
	CloseHandle(hFile);
	return 1;
}


/******************************************************************************

	SaveTreeMem

	�A�C�e�������t�@�C���ɕۑ�����

******************************************************************************/

BOOL SaveTreeMem(HWND hWnd, HWND hTreeView, HTREEITEM hItem)
{
	struct TPTREE *tpTreeInfo;
	char FileName[MAX_PATH];
	char buf[MAX_PATH];
	int ret, f;

	tpTreeInfo = (struct TPTREE *)TreeView_GetlParam(hTreeView, hItem);
	if(tpTreeInfo == NULL){
		return FALSE;
	}

	if(tpTreeInfo->ItemList == NULL){
		return TRUE;
	}

	TreeView_GetPath(hTreeView, hItem, buf, CuDir);
	mkDirStr(buf);
	wsprintf(FileName, "%s\\"DATAFILENAME, buf);

	//�A�C�e�����̕ۑ�
	//�ۑ��Ɏ��s�����ꍇ�́A"Item.bak + ����" �ŕۑ����s��
	f = 0;
	while((ret = SaveItemList(hWnd, FileName, tpTreeInfo->ItemList, tpTreeInfo->ItemListCnt)) != 1){
		if(ret == -1){
			return FALSE;
		}
		f++;
		if(f >= CREATEFILE_MAX){
			return FALSE;
		}
		wsprintf(FileName, "%s\\Item.bak%d", buf, f);
	}

	//�t�H���_���̕ۑ�
	PutDirInfo(hTreeView, buf, hItem);
	return TRUE;
}


/******************************************************************************

	InitGetDiskFreeSpaceEx

	GetDiskFreeSpaceEx �̃A�h���X���擾

******************************************************************************/

void InitGetDiskFreeSpaceEx()
{
	hDll = LoadLibrary("kernel32.dll");
	if(hDll == NULL){
		return;
	}
	_GetDiskFreeSpaceEx = (FUNC_GETDISKFREESPACEEX)GetProcAddress(hDll, "GetDiskFreeSpaceExA");
	if(_GetDiskFreeSpaceEx == NULL){
		FreeLibrary(hDll);
		hDll = NULL;
	}
}


/******************************************************************************

	FreeGetDiskFreeSpaceEx

	���C�u�����̉��

******************************************************************************/

void FreeGetDiskFreeSpaceEx()
{
	if(hDll != NULL){
		FreeLibrary(hDll);
		hDll = NULL;
	}
}


/******************************************************************************

	FileSelect

	�t�@�C���I���_�C�A���O�̕\��

******************************************************************************/

int FileSelect(HWND hDlg, char *oFile, char *oFilter, char *oTitle, char *ret, char *def, int Index)
{
	OPENFILENAME of;
	char tmp[MAX_PATH];

	lstrcpy(tmp, oFile);

	of.lStructSize = sizeof(of);
	of.hInstance = NULL;
	of.hwndOwner = hDlg;
	of.lpstrFilter = oFilter;
	of.lpstrTitle = oTitle;
	of.lpstrCustomFilter = NULL;
	of.nMaxCustFilter = 40;
	of.nFilterIndex = Index;
	of.lpstrFile = tmp;
	of.nMaxFile = MAX_PATH - 1;
	of.lpstrFileTitle = NULL;
	of.nMaxFileTitle = 0;
	of.lpstrInitialDir = NULL;
	of.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	of.nFileOffset = 0;
	of.nFileExtension = 0;
	of.lpstrDefExt = def;
	of.lCustData = 0;
	of.lpfnHook = NULL;
	of.lpTemplateName = NULL;

	if(GetOpenFileName((LPOPENFILENAME)&of) == TRUE){
		lstrcpy(ret, of.lpstrFile);
		return of.nFilterIndex;
	}
	return -1;
}


/******************************************************************************

	SetExecCommandLine

	�R�}���h���C���̍쐬

******************************************************************************/

static void SetExecCommandLine(HWND hWnd, char *buf, char *ret, struct TPITEM *tpItemInfo)
{
	char FolderPath[MAX_PATH];
	char *p, *r, *t;

	if(buf == NULL){
		return;
	}

	// %���� �̕ϊ����s��
	for(p = buf, r = ret;*p != '\0';p++){
		if(IsDBCSLeadByte((BYTE)*p) == TRUE){
			*(r++) = *(p++);
			*(r++) = *p;

		}else if(*p != '%'){
			*(r++) = *p;

		}else{
			p++;
			if(*p == '\0'){
				break;
			}
			t = NULL;
			switch(*p)
			{
			//%
			case '%':
				t = "%";
				break;

			//�\������URL,��̏ꍇ�̓`�F�b�N����URL
			case 'u': case 'U':
				if(tpItemInfo == NULL){
					break;
				}
				if(tpItemInfo->ViewURL != NULL && *tpItemInfo->ViewURL != '\0'){
					t = tpItemInfo->ViewURL;
				}else if(tpItemInfo->CheckURL != NULL){
					t = tpItemInfo->CheckURL;
				}
				break;

			//�\������URL,��̏ꍇ�͖���
			case 'v': case 'V':
				if(tpItemInfo == NULL){
					break;
				}
				t = tpItemInfo->ViewURL;
				break;

			//�`�F�b�N����URL
			case 'c': case 'C':
				if(tpItemInfo == NULL){
					break;
				}
				t = tpItemInfo->CheckURL;
				break;

			//�A�C�e���̃^�C�g��
			case 't': case 'T':
				if(tpItemInfo == NULL){
					break;
				}
				t = tpItemInfo->Title;
				break;

			//�A�C�e���̃T�C�Y
			case 's': case 'S':
				if(tpItemInfo == NULL){
					break;
				}
				t = tpItemInfo->Size;
				break;

			//�A�C�e���̍X�V����(ini�t�@�C���̃t�H�[�}�b�g�Ɉˑ�)
			case 'd': case 'D':
				if(tpItemInfo == NULL){
					break;
				}
				t = tpItemInfo->Date;
				break;

			//�A�C�e���̃`�F�b�N����(ini�t�@�C���̃t�H�[�}�b�g�Ɉˑ�)
			case 'k': case 'K':
				if(tpItemInfo == NULL){
					break;
				}
				t = tpItemInfo->CheckDate;
				break;

			//�A�C�e���ɐݒ肳��Ă���R�����g
			case 'm': case 'M':
				if(tpItemInfo == NULL){
					break;
				}
				t = tpItemInfo->Comment;
				break;

			//���݊J���Ă���t�H���_�̎��ۂ̃p�X (�����`��)
			case 'f':
				SendMessage(WWWCWnd, WM_FOLDER_GETPATH,
					(WPARAM)TreeView_GetSelection(GetDlgItem(WWWCWnd, WWWC_TREE)), (LPARAM)FolderPath);
				t = FolderPath;
				break;

			//���݊J���Ă���t�H���_�̎��ۂ̃p�X (�Z���`��)
			case 'F':
				SendMessage(WWWCWnd, WM_FOLDER_GETPATH,
					(WPARAM)TreeView_GetSelection(GetDlgItem(WWWCWnd, WWWC_TREE)), (LPARAM)FolderPath);
				GetShortPathName(FolderPath, FolderPath, MAX_PATH - 1);
				t = FolderPath;
				break;

			//���[�U�f�B���N�g���̃p�X (�����`��)
			case 'p':
				t = CuDir;
				break;

			//���[�U�f�B���N�g���̃p�X (�Z���`��)
			case 'P':
				GetShortPathName(CuDir, FolderPath, MAX_PATH - 1);
				t = FolderPath;
				break;
			}
			if(t != NULL){
				while(*(r++) = *(t++));
				r--;
			}
		}
	}
	*r = '\0';
}


/******************************************************************************

	ExecItemFile

	�t�@�C���̋N��

******************************************************************************/

int ExecItemFile(HWND hWnd, char *FileName, char *CommandLine, struct TPITEM *tpItemInfo, int SyncFlag)
{
	SHELLEXECUTEINFO sei;
	char buf[MAX_PATH];
	char *ExeFilePath;
	char *CommnadLineString;
	int ret;
	BOOL b_ret;

	*buf = '\0';
	//�R�}���h���C���̍쐬
	SetExecCommandLine(hWnd, CommandLine, buf, tpItemInfo);

	ExeFilePath = FileName;
	CommnadLineString = buf;

	//�t�@�C�������w�肳��Ă��Ȃ��ꍇ�̓R�}���h���C����ݒ肷��i�֘A�t���Ŏ��s�j
	if(*ExeFilePath == '\0'){
		if(*buf == '\0'){
			return -1;
		}
		ExeFilePath = buf;
		CommnadLineString = "";
	}

	WaitCursor(TRUE);

	sei.cbSize = sizeof(sei);
	sei.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_NO_UI;
	sei.hwnd = NULL;
	sei.lpVerb = NULL;
	sei.lpFile = ExeFilePath;
	sei.lpParameters = CommnadLineString;
	sei.lpDirectory = NULL;
	sei.nShow = SW_SHOWNORMAL;
	sei.hInstApp = g_hinst;

	//�N��
	b_ret = ShellExecuteEx(&sei);
	if(WWWCWnd == NULL || IsWindow(WWWCWnd) == 0){
		//�v���Z�X�̋����I��
		exit(0);
	}
	if(b_ret == FALSE){
		ret = GetLastError();
		WaitCursor(FALSE);
		ErrMsg(hWnd, ret, EMSG_EXEC_TITLE);
		return -1;
	}
	WaitCursor(FALSE);

	//�������Ȃ��ꍇ�͊֐��𔲂���
	if(SyncFlag == 0){
		return 0;
	}

	//�N���A�v�����I������܂őҋ@����
	EnableWindow(hWnd, FALSE);
	while(WaitForSingleObject(sei.hProcess, 0) == WAIT_TIMEOUT){
		DoEvents();
	}
	EnableWindow(hWnd, TRUE);
	_SetForegroundWindow(hWnd);
	return 0;
}


/******************************************************************************

	CreateDropFileMem

	�h���b�v�t�@�C���̍쐬

******************************************************************************/

HDROP CreateDropFileMem(char **FileName, int cnt)
{
	HDROP hDrop;
	LPDROPFILES lpDropFile;
	char *buf;
	int flen = 0;
	int i;

	for (i = 0; i < cnt; i++) {
		flen += lstrlen(*(FileName + i)) + 1;
	}
	hDrop = (HDROP)GlobalAlloc(GHND, sizeof(DROPFILES) + flen + 1);
	if(hDrop == NULL){
		return NULL;
	}

	lpDropFile = (LPDROPFILES) GlobalLock(hDrop);
	lpDropFile->pFiles = sizeof(DROPFILES);		//�t�@�C�����̃��X�g�܂ł̃I�t�Z�b�g
	lpDropFile->pt.x = 0;
	lpDropFile->pt.y = 0;
	lpDropFile->fNC = FALSE;
	lpDropFile->fWide = FALSE;

	//�\���̂̌��Ƀt�@�C�����̃��X�g���R�s�[(�t�@�C����\0�t�@�C����\0�t�@�C����\0\0)
	buf = (char *)(lpDropFile + 1);
	for (i = 0; i < cnt; i++) {
		lstrcpy(buf, *(FileName + i));
		buf += lstrlen(*(FileName + i)) + 1;
	}
	buf = '\0';
	GlobalUnlock(hDrop);
	return(hDrop);
}
/* End of source */
