/**************************************************************************

	WWWC (dll)

	wwwcdll.h
	Ver 1.1.0

	Copyright (C) 1996-2018 by Ohno Tomoaki. All rights reserved.
		https://www.nakka.com/
		nakka@nakka.com

**************************************************************************/

#ifndef _INC_WWWCDLL_H
#define _INC_WWWCDLL_H

/**************************************************************************
	Define
**************************************************************************/

/* �E�B���h�E���̃R���g���[�� */
#define WWWC_TB						102					/* ToolBar */
#define WWWC_SB						103					/* StatusBar */
#define WWWC_LIST					1002				/* ListView */
#define WWWC_TREE					1114				/* TreeView */

/* �\�P�b�g���b�Z�[�W */
#define WM_WSOCK_GETHOST			(WM_USER + 1)		/* �z�X�g�����擾���郁�b�Z�[�W */
#define WM_WSOCK_SELECT				(WM_USER + 2)		/* �񓯊�select�̃��b�Z�[�W */

/* �`�F�b�N���b�Z�[�W */
#define WM_CHECK_RESULT				(WM_USER + 3)		/* �A�C�e���̃`�F�b�N���ʂ�m�点�郁�b�Z�[�W */
#define WM_ITEMCHECK				(WM_USER + 4)		/* �A�C�e���̃`�F�b�N�J�n�v�� */
#define WM_NEXTCHECK				(WM_USER + 9)		/* ���̃`�F�b�N */
#define WM_CHECK_END				(WM_USER + 60)		/* �A�C�e���̃`�F�b�N�I����m�点�郁�b�Z�[�W */

/* �f�[�^�v�����b�Z�[�W */
#define WM_GETVERSION				(WM_USER + 5)		/* WWWC�̃o�[�W�������擾 */
#define WM_GETCHECKLIST				(WM_USER + 6)		/* �`�F�b�N���̃A�C�e�����X�g���擾 */
#define WM_GETCHECKLISTCNT			(WM_USER + 7)		/* �`�F�b�N���̃A�C�e�����X�g�̐����擾 */
#define WM_SETITEMLIST				(WM_USER + 8)		/* �A�C�e���̒ǉ� */
#define WM_GETMAINWINDOW			(WM_USER + 10)		/* ���C���E�B���h�E�̃n���h�����擾 */
#define WM_GETMAINITEM				(WM_USER + 11)		/* �A�C�e���̎��̂��擾 */
#define WM_ITEMEXEC					(WM_USER + 12)		/* �A�C�e�������s */
#define WM_ITEMINIT					(WM_USER + 13)		/* �A�C�e���������� */
#define WM_GETUPWINDOW				(WM_USER + 14)		/* �X�V���b�Z�[�W�̃n���h�����擾 */
#define WM_GETFINDWINDOW			(WM_USER + 15)		/* �����E�B���h�E�̃n���h�����擾 */

/* INI�t�@�C���������b�Z�[�W */
#define WM_WWWC_GETINI				(WM_USER + 20)		/* INI�t�@�C������ݒ��ǂݒ��� */
#define WM_WWWC_PUTINI				(WM_USER + 21)		/* INI�t�@�C���ɐݒ���������� */
#define WM_WWWC_GETINIPATH			(WM_USER + 22)		/* INI�t�@�C���̕ۑ���̃p�X���擾 */

/* �t�H���_���상�b�Z�[�W */
#define WM_FOLDER_SAVE				(WM_USER + 31)		/* �J���Ă���t�H���_�̓��e��ۑ� */
#define WM_FOLDER_LOAD				(WM_USER + 32)		/* �J���Ă���t�H���_�̓��e��ǂݒ��� */
#define WM_FOLDER_GETPATH			(WM_USER + 33)		/* �t�H���_�̃p�X���擾 */
#define WM_FOLDER_GETWWWCPATH		(WM_USER + 34)		/* �t�H���_��WWWC���ł̃p�X���擾 */
#define WM_FOLDER_SELECT			(WM_USER + 35)		/* �t�H���_��I�����Ă��̃p�X���擾 */
#define WM_FOLDER_REFRESH			(WM_USER + 36)

#define WM_DOEVENTS					(WM_USER + 50)		/* �{�̂ɃC�x���g������������ */


/* �A�C�e���̏�� */
#define ST_DEFAULT					0					/* �ʏ� */
#define ST_UP						1					/* UP */
#define ST_ERROR					2					/* �G���[ */
#define ST_TIMEOUT					4					/* �^�C���A�E�g */

/* �A�C�R���̏�� */
#define ICON_ST_NOCHECK				1					/* �`�F�b�N�ҋ@ */
#define ICON_ST_CHECK				2					/* �`�F�b�N�� */

/* �߂�l */
#define CHECK_ERROR					-1					/* �G���[ */
#define CHECK_SUCCEED				0					/* �������� */
#define CHECK_NO					1					/* �ҋ@ */
#define CHECK_END					2					/* �`�F�b�N�I�� */

/* �h���b�v�t�@�C���̖߂�l */
#define DROPFILE_NONE				0					/* ������ */
#define DROPFILE_NEWITEM			1					/* �A�C�e���쐬 */

/* �c�[�����s�t���O */
#define TOOL_EXEC_PORP				0					/* �v���p�e�B */
#define TOOL_EXEC_ITEMMENU			1					/* �A�C�e�����j���[ */
#define TOOL_EXEC_WINDOWMENU		2					/* �E�B���h�E���j���[ */
#define TOOL_EXEC_START				4					/* �N���� */
#define TOOL_EXEC_END				8					/* �I���� */
#define TOOL_EXEC_CHECKSTART		16					/* �`�F�b�N�J�n�� */
#define TOOL_EXEC_CHECKEND			32					/* �`�F�b�N�I���� */
#define TOOL_EXEC_CHECKENDUP		64					/* �`�F�b�N�I�����ōX�V����̂Ƃ� */
#define TOOL_EXEC_CHECKENDNOUP		128					/* �`�F�b�N�I�����ōX�V�Ȃ��̂Ƃ� */
#define TOOL_EXEC_SYNC				256					/* �c�[�����I������܂őҋ@ (DLL�̏ꍇ�͖���) */
#define TOOL_EXEC_INITITEM			512					/* ���s��ɃA�C�e���������� */
#define TOOL_EXEC_MENUDEFAULT		1024				/* �f�t�H���g���� */
#define TOOL_EXEC_NOTCHECK			2048				/* �`�F�b�N���͎��s���Ȃ� */
#define TOOL_EXEC_SAVEFOLDER		4096				/* ���s�O�Ƀt�H���_�̓��e��ۑ����Ď��s��ɓǂݒ��� */

/* �c�[����\���t���O */
#define TOOL_HIDE_ITEMMENU			1
#define TOOL_HIDE_WINDOWMENU		2
#define TOOL_HIDE_START				4
#define TOOL_HIDE_END				8
#define TOOL_HIDE_CHECKSTART		16
#define TOOL_HIDE_CHECKEND			32
#define TOOL_HIDE_CHECKENDUP		64
#define TOOL_HIDE_CHECKENDNOUP		128
#define TOOL_HIDE_SYNC				256
#define TOOL_HIDE_INITITEM			512
#define TOOL_HIDE_MENUDEFAULT		1024
#define TOOL_HIDE_NOTCHECK			2048
#define TOOL_HIDE_SAVEFOLDER		4096
#define TOOL_HIDE_HOTKEY			8192				/* �z�b�g�L�[ */
#define TOOL_HIDE_MENUINDEX			16384				/* ���j���[Index */
#define TOOL_HIDE_PROTOCOL			32768				/* �v���g�R���I�� */

/* �`�F�b�N�̎�� */
#define CHECKTYPE_ITEM				1					/* �A�C�e�����`�F�b�N */
#define CHECKTYPE_ALL				2					/* �S�ẴA�C�e�����`�F�b�N */
#define CHECKTYPE_TREE				4					/* �K�w�`�F�b�N */
#define CHECKTYPE_ERROR				8					/* �G���[�A�C�e�����`�F�b�N */
#define CHECKTYPE_AUTO				16					/* �����`�F�b�N */


/**************************************************************************
	Struct
**************************************************************************/

/* �A�C�e����� */
struct TPITEM{
	long iSize;						/* �\���̂̃T�C�Y */

	long hItem;						/* �A�C�e�����i�[����Ă���t�H���_�̃n���h�� (HTREEITEM) */

	char *Title;					/* �^�C�g�� */
	char *CheckURL;					/* �`�F�b�N���s��URL */
	char *Size;						/* �T�C�Y */
	char *Date;						/* �X�V���� */
	int Status;						/* �A�C�e���̏�� (ST_) */

	char *CheckDate;				/* �`�F�b�N�������� */
	char *OldSize;					/* ���X�V�� */
	char *OldDate;					/* ���T�C�Y */

	char *ViewURL;					/* �\������URL */
	char *Option1;					/* �I�v�V���� 1 */
	char *Option2;					/* �I�v�V���� 2 */
	char *Comment;					/* �R�����g */

	int CheckSt;					/* �`�F�b�N�t���O */

	char *ErrStatus;				/* �G���[��� */
	char *DLLData1;					/* DLL�p�f�[�^ 1 */
	char *DLLData2;					/* DLL�p�f�[�^ 2 */

/* �ȉ��ۑ�����Ȃ���� */
	int IconStatus;					/* �A�C�R���̏�� (ICON_ST_) */

	int Soc1;						/* �\�P�b�g1 */
	int Soc2;						/* �\�P�b�g2 */
	HANDLE hGetHost1;				/* �z�X�g���̃n���h��1 */
	HANDLE hGetHost2;				/* �z�X�g���̃n���h��1 */
	long Param1;					/* DLL�plong�l1 */
	long Param2;					/* DLL�plong�l2 */
	long Param3;					/* DLL�plong�l3 */
	long Param4;					/* DLL�plong�l4 */
	int user1;						/* DLL�pint�l1 */
	int user2;						/* DLL�pint�l2 */

	BOOL RefreshFlag;				/* �ĕ`��t���O */
};


/* �v���g�R���ݒ� */
struct TPPROTOCOLSET{
	long iSize;						/* �\���̂̃T�C�Y */

	char Title[256];				/* �^�C�g�� */
	char FuncHeader[256];			/* �֐����̐擪�ɕt���镶�� */
	char IconFile[256];				/* �A�C�R���t�@�C���� */
	int IconIndex;					/* �A�C�R���C���f�b�N�X */
	char UpIconFile[256];			/* UP�A�C�R���t�@�C���� */
	int UpIconIndex;				/* UP�A�C�R���C���f�b�N�X */
};

/* �v���g�R�����j���[ */
struct TPPROTOCOLMENU{
	char Name[256];					/* ���j���[�� */
	char Action[256];				/* ���s���̃A�N�V���� */
	BOOL Default;					/* �f�t�H���g�t���O */
	int Flag;						/* ����t���O */
};

/* �v���g�R����� */
struct TPPROTOCOLINFO{
	long iSize;						/* �\���̂̃T�C�Y */

	char Scheme[256];				/* �Ή�����X�L�[�} (��: "http://\thttps://") */
	char NewMenu[256];				/* �V�K�쐬���j���[�� */
	char FileType[256];				/* D&D�ŏ����\�ȃt�@�C���̊g���q (��: "txt\tdoc\tdat") */
	struct TPPROTOCOLMENU *tpMenu;	/* �A�C�e�����j���[�ɒǉ����鍀�� */
	int tpMenuCnt;					/* �A�C�e�����j���[�ɒǉ����鍀�ڂ̐� */
};


/* �c�[���ݒ� */
struct TP_TOOLS{
	long iSize;						/* �\���̂̃T�C�Y */

	char title[256];				/* �^�C�g�� */
	char func[256];					/* �֐��� */
	char Protocol[256];				/* �Ή�����v���g�R�� (��: http,ftp) */
	int MenuIndex;					/* ���j���[�ʒu */
	int Action;						/* ���s�t���O */
};

#endif
/* End of source */
