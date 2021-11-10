/**************************************************************************

	WWWC

	StrTbl.h

	Copyright (C) 1996-2003 by Nakashima Tomoaki. All rights reserved.
		http://www.nakka.com/
		nakka@nakka.com

**************************************************************************/

#ifndef _INC_STRTBL_H
#define _INC_STRTBL_H

/**************************************************************************
	Define
**************************************************************************/

//General
#define NEWITEMNAME						"�V�����A�C�e��"
#define FOLDERNAME						"�t�H���_"

#define STR_COPYNAME					"�R�s�[ �` "
#define STR_COPYNAME_CNT				"�R�s�[ (%d) �` "

#define EMSG_PROP_TITLE					"�v���p�e�B"
#define EMSG_PROP						"�ݒ�ł���v���p�e�B�͂���܂���B"

#define QMSG_DELETE_TITLE				"�폜�̊m�F"
#define QMSG_DELETE						"�I������Ă��鍀�ڂ��폜���Ă���낵���ł����H"

//main.c
#define CLIPBOARDFORMAT					"WWWC ItemData"
#define DEFAULTUSER						"DEFAULT"

#define MENU_STR_NEWITEM				"�V�K�쐬(&N)"

#define ABOUTTITLE						"�o�[�W�������"

#define IMSG_DOUBLESTART				"���ɋN�����Ă��܂�"

#define QMSG_ALLINITICON_TITLE			"�A�C�R���̏������̊m�F"
#define QMSG_ALLINITICON				"���ׂẴA�C�R�������������Ă���낵���ł����H"
#define QMSG_END_TITLE					"�I��"
#define QMSG_END_CHECKCANCEL			"�`�F�b�N���̃A�C�e�������݂��܂����A�I�����Ă���낵���ł����H"

#define EMSG_OLEINIT					"OLE�̏������Ɏ��s���܂����B"
#define EMSG_WINSOCKINIT				"WinSock�̏������Ɏ��s���܂����B"
#define EMSG_ATEXIT						"atexit�Ɏ��s���܂����B"
#define EMSG_REGWINDOWCLASS				"WindowClass�̓o�^�Ɏ��s���܂����B"
#define EMSG_CREATEWINDOW				"Window�̍쐬�Ɏ��s���܂����B"
#define EMSG_CREATECONTROL				"�R���g���[���̍쐬�Ɏ��s���܂����B"
#define EMSG_CREATEUPMSG				"�X�V���b�Z�[�W�̍쐬�Ɏ��s���܂����B"

//AutoCheck.c
#define LISTCOL_TITLE					"����"

#define STRWEEK_STRLEN					3
#define STRWEEK_SUN						"��"
#define STRWEEK_MON						"��"
#define STRWEEK_TUE						"��"
#define STRWEEK_WED						"��"
#define STRWEEK_THU						"��"
#define STRWEEK_FRI						"��"
#define STRWEEK_SAT						"�y"

#define LISTSTR_MIN						"%d���� �Ƀ`�F�b�N"
#define LISTSTR_NOU						"%d���Ԗ� �Ƀ`�F�b�N"
#define LISTSTR_DAY						"%02d�� %02d�� �Ƀ`�F�b�N"
#define LISTSTR_WEEK					"%s�j�� %02d�� %02d�� �Ƀ`�F�b�N"
#define LISTSTR_MON						"%d�� %02d�� %02d�� �Ƀ`�F�b�N"

#define TIMEOPTIONSTR_HOU				"��"
#define TIMEOPTIONSTR_MIN				"��"
#define TIMEOPTIONSTR_EVERYHOU			"���Ԗ�"
#define TIMEOPTIONSTR_EVERYMIN			"����"

#define TIMEOPTIONERRMSG_TITLE			"�����`�F�b�N����"
#define TIMEOPTIONERRMSG_DAY			"\"��\" �����͂���Ă��܂���B"
#define TIMEOPTIONERRMSG_HOU			"\"��\" �����͂���Ă��܂���B"
#define TIMEOPTIONERRMSG_MIN			"\"��\" �����͂���Ă��܂���B"

//ClipBoard.c
#define EMSG_FOLDERCOPY_TITLE			"�t�H���_�̃R�s�[�̃G���["
#define EMSG_FOLDERCOPY_CUR				"%s ���R�s�[�ł��܂���B���葤�Ǝ󂯑��̃t�H���_�������ł��B"
#define EMSG_FOLDERCOPY_SUB				"%s ���R�s�[�ł��܂���B�󂯑��̃t�H���_�́A���葤�t�H���_�̃T�u�t�H���_�ł��B"
#define EMSG_FOLDERCOPY_ERR				"�t�H���_�̃R�s�[�Ɏ��s���܂����B"

#define EMSG_MOVEDIR_TITLE				"�t�H���_�̈ړ��̃G���["

//DragDrop.c
#define EMSG_ITEMREAD_TITLE				"�A�C�e���̓ǂݍ���"
#define EMSG_ITEMREAD					"�A�C�e�����t�@�C�����J���܂���ł����B"

//File.c
#define FILEDATE_FORMAT					"%s %s"

#define STR_NOTITLE						"(no-title)"

#define ICON_FILE_FILTER				"*.*", "���ׂẴt�@�C�� (*.*)\0*.*\0\0"

#define EMSG_SAVE_TITLE					"�ۑ��G���["
#define EMSG_EXEC_TITLE					"���s�G���["

#define EMSG_DISKFULL					"�󂫃f�B�X�N�e�ʂ�����Ȃ����߃A�C�e������ۑ��ł��܂���B"

//Item.c
#define STR_UPDATEMSG					"�t�H���_ '%s' �ɂ͊��� '%s' �A�C�e�������݂��܂��B"
#define STR_UPDATEINFO					"�T�C�Y: %s\n�X�V����: %s"

#define QMSG_FOLDERDELETE				"�t�H���_ '%s' �ƃt�H���_���̂��ׂẴA�C�e�����폜���Ă���낵���ł����H"
#define QMSG_ITEMDELETE					"'%s' ���폜���Ă���낵���ł����H"
#define QMSG_MANYITEMDELETE				"������ %d �̃A�C�e�����폜���Ă���낵���ł����H"

#define QMSG_CLEARRECYCLER_TITLE		"���ݔ�����ɂ���"
#define QMSG_CLEARRECYCLER				"���ݔ�����ɂ��Ă���낵���ł����H"

#define EMSG_FOLDERDELETE_TITLE			"�t�H���_�̍폜"
#define EMSG_FOLDERCHECK				"�`�F�b�N���̃t�H���_�͍폜�ł��܂���B"
#define EMSG_FOLDERPROP					"�������̃A�C�e�������݂��邽�ߍ폜�ł��܂���ł����B"
#define EMSG_FOLDERNODELETE				"�폜�ł��܂���ł����B"

#define EMSG_ITEMDELETE_TITLE			"�A�C�e���̍폜"
#define EMSG_ITEMCHECK					"�`�F�b�N���̃A�C�e���͍폜�ł��܂���B"
#define EMSG_ITEMNODELETE				"�폜�ł��܂���ł����B"

#define EMSG_CLEARRECYCLER_TITLE		"���ݔ�����ɂ���"
#define EMSG_CLEARRECYCLER				"�`�F�b�N���̃A�C�e�������݂��邽�߂��ݔ�����ɂł��܂���ł����B"
#define EMSG_PROPRECYCLER				"�������̃A�C�e�������݂��邽�߂��ݔ�����ɂł��܂���ł����B"

#define EMSG_CHECKUPDATE_TITLE			"�㏑��"
#define EMSG_CHECKUPDATE				"�`�F�b�N���̃A�C�e���̂��ߏ㏑���ł��܂���ł����B"

#define EMSG_NOPROPITEM					"�A�C�e����������܂���ł����B"

//ListView.c
#define COLUMN_NAME						"���O"
#define COLUMN_CHECKURL					"�`�F�b�N����URL"
#define COLUMN_SIZE						"�T�C�Y"
#define COLUMN_DATE						"�X�V����"
#define COLUMN_CHECKDATE				"�`�F�b�N����"
#define COLUMN_OLDSIZE					"���T�C�Y"
#define COLUMN_OLDDATE					"���X�V����"
#define COLUMN_VIEWURL					"�\������URL"
#define COLUMN_OPTION1					"�I�v�V����1"
#define COLUMN_OPTION2					"�I�v�V����2"
#define COLUMN_COMMENT					"�R�����g"
#define COLUMN_ERROR					"�G���[���"
#define COLUMN_PATH						"�p�X"
#define COLUMN_DLLDATA1					"�v���g�R�����1"
#define COLUMN_DLLDATA2					"�v���g�R�����2"

//Menu.c
#define MENU_STR_OPEN					"�J��(&O)"
#define MENU_STR_CLEARRECY				"���ݔ�����ɂ���(&B)"
#define MENU_STR_FOLDERTREECHECK		"�K�w�`�F�b�N(&K)"
#define MENU_STR_SERACH					"����(&F)..."

#define MENU_STR_UNDO_TITLE				"���ɖ߂�"
#define MENU_STR_UNDO_KEY				"(&U)\tCtrl+Z"

#define MENU_STR_UNDO_NON				MENU_STR_UNDO_TITLE""MENU_STR_UNDO_KEY
#define MENU_STR_UNDO_MOVE				MENU_STR_UNDO_TITLE" - �ړ�"MENU_STR_UNDO_KEY
#define MENU_STR_UNDO_COPY				MENU_STR_UNDO_TITLE" - �R�s�["MENU_STR_UNDO_KEY
#define MENU_STR_UNDO_DELETE			MENU_STR_UNDO_TITLE" - �폜"MENU_STR_UNDO_KEY
#define MENU_STR_UNDO_NAME				MENU_STR_UNDO_TITLE" - ���O�̕ύX"MENU_STR_UNDO_KEY

//Option.c
#define STR_ITEMCNT						"�A�C�e����: %d�A�t�H���_��: %d"
#define STR_NOMAKEDATE					"(�s��)"
#define STR_PROPTITLE					"%s�̃v���p�e�B"
#define STR_OPTIONTITLE					"�I�v�V����"

#define FILEFILTER_WAV					"�T�E���h (*.wav)\0*.wav\0���ׂẴt�@�C�� (*.*)\0*.*\0\0"

//Protocol.c
#define PROTOCOL_FILEFILTER				"DLL (*.dll)\0*.dll\0���ׂẴt�@�C�� (*.*)\0*.*\0\0"

#define PROTOCOL_LISTCOL_TITLE			"�^�C�g��"
#define PROTOCOL_LISTCOL_DLL			"DLL"
#define PROTOCOL_LISTCOL_FUNCHEAD		"�w�b�_"
#define PROTOCOL_LISTCOL_ICONFILE		"�A�C�R���t�@�C��"
#define PROTOCOL_LISTCOL_ICONINDEX		"�A�C�R���C���f�b�N�X"
#define PROTOCOL_LISTCOL_UPICONFILE		"UP�A�C�R���t�@�C��"
#define PROTOCOL_LISTCOL_UPICONINDEX	"UP�A�C�R���C���f�b�N�X"

#define EMSG_SELECT_TITLE_PROTOCOL		"�v���g�R���I��"
#define EMSG_SELECT_PROTOCOL			"�ǉ�����v���g�R����I�����Ă��������B"

//SelectDll.c
#define EMSG_DLL_TITLE					"DLL"
#define EMSG_DLL						"DLL����v���g�R���ƃc�[���̏�񂪌�����܂���ł����B"
#define EMSG_SELECT						"�ǉ�����v���g�R���y�уc�[����I�����Ă��������B"

//SelectIcon.c
#define FILEFILTER_ICON					"�A�C�R�� �t�@�C��\0*.ico;*.exe;*.dll;*.icl\0" \
											"�v���O����\0*.exe\0���C�u����\0*.dll;*.icl\0" \
											"�A�C�R��\0*.ico\0���ׂẴt�@�C��\0*.*\0\0"

//StatusBar.c
#define ITEMMSG							"%d �̃A�C�e��"
#define ITEMMSG_UP						"%d �̃A�C�e�� (UP�A�C�e�� %d ��)"
#define ITEMSELECTMSG					"%d �̃A�C�e����I��"

//Tool.c
#define NOMENUITEM						"(�Ȃ�)"
#define TOOL_FILE_FILTER				"�c�[�� (*.exe;*.dll)\0*.exe;*.dll\0���ׂẴt�@�C�� (*.*)\0*.*\0\0"

#define TOOL_LISTCOL_TITLE				"�^�C�g��"
#define TOOL_LISTCOL_FILENAME			"�t�@�C����"
#define TOOL_LISTCOL_EXEOPTION			"���s�I�v�V����"
#define TOOL_LISTCOL_EXEPROTOCOL		"���s�v���g�R��"
#define TOOL_LISTCOL_MENUINDEX			"���j���[�ʒu"
#define TOOL_LISTCOL_SKEY				"�V���[�g�J�b�g�L�["
#define TOOL_LISTCOL_CTRL				"�L�[ 1"
#define TOOL_LISTCOL_KEY				"�L�[ 2"
#define TOOL_LISTCOL_COMMANDLINE		"�R�}���h���C��"
#define TOOL_LISTCOL_DLLFUNC			"�֐�(DLL)"

#define EMSG_NOINPUT_TITLE				"�c�[���ݒ�"
#define EMSG_NOINPUT					"\"�^�C�g��\" �����͂���Ă��܂���B"

#define EMSG_SELECT_TITLE_TOOL			"�c�[���I��"
#define EMSG_SELECT_TOOL				"�ǉ�����c�[����I�����Ă��������B"

#define TOOL_TITLE_COMMANDLINE			"�R�}���h���C��(&C):"
#define TOOL_TITLE_DLLFUNC				"���s����֐���:"

#define TOOL_PROTOCOL_ALL				"&0. ���ׂ� (*)"

#define COMMANDLINE_u					"&1. %u - �\������URL, ���ݒ莞�`�F�b�N����URL"
#define COMMANDLINE_c					"&2. %c - �`�F�b�N����URL"
#define COMMANDLINE_v					"&3. %v - �\������URL"
#define COMMANDLINE_t					"&4. %t - �A�C�e���̖��O"
#define COMMANDLINE_s					"&5. %s - �A�C�e���̃T�C�Y"
#define COMMANDLINE_d					"&6. %d - �A�C�e���̍X�V����"
#define COMMANDLINE_k					"&7. %k - �A�C�e���̃`�F�b�N����"
#define COMMANDLINE_m					"&8. %m - �A�C�e���ɐݒ肳��Ă���R�����g"
#define COMMANDLINE_f					"&9. %f - �J���Ă���t�H���_�̎��p�X"
#define COMMANDLINE_F					"&10. %F - �J���Ă���t�H���_�̎��p�X (�Z���`��)"
#define COMMANDLINE_p					"&11. %p - ���[�U�f�B���N�g���̃p�X"
#define COMMANDLINE_P					"&12. %P - ���[�U�f�B���N�g���̃p�X (�Z���`��)"

//TreeView.c
#define NEWFOLDER						"�V�����t�H���_"

#define EMSG_CHANGEFILENAME_TITLE		"���O�̕ύX"
#define EMSG_CHANGEFILENAME				"�t�@�C�����Ɏ��̕����͎g���܂���B\n\t\\ / : , ; * ? \" < > |"

#define EMSG_DIRMOVE_TITLE				"�t�H���_�̈ړ�"
#define EMSG_DIRMOVE					"�A�C�e�����t�@�C�����J���܂���ł����B"

//undo.c
#define QMSG_ITEMDELETE_UNDO			"�폜�����A�C�e��������܂�����낵���ł����H"

//UpMessage.c
#define STR_UPMSG						"%d ���X�V���ꂽ�\��������A�C�e��������܂��B\n\n%s %s"
#define STR_NOTUPMSG					"%d ���X�V���ꂽ�\��������A�C�e��������܂��B\n�� �Ō�̃`�F�b�N�ł͍X�V�͂���܂���ł����B\n%s %s"
#define STR_ZEROMSG						"�X�V���ꂽ�A�C�e���͂���܂���B\n\n%s %s"
#define STR_NOUPMSG						"�X�V���ꂽ�A�C�e���͂���܂���B"

#define BTN_UPINFO_ON					"�ڍ�(&D) >>"
#define BTN_UPINFO_OFF					"�ڍ�(&D) <<"

#define EMSG_MAINSHOW_TITLE				"�\��"
#define EMSG_NOMAINITEM					"�{�̂���A�C�e����������܂���ł����B"

#endif
/* End of source */
