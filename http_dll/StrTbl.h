/**************************************************************************

	WWWC (wwwc.dll)

	StrTbl.h

	Copyright (C) 1996-2003 by Nakashima Tomoaki. All rights reserved.
		http://www.nakka.com/
		nakka@nakka.com

**************************************************************************/

#ifndef _INC_DLL_STRTBL_H
#define _INC_DLL_STRTBL_H

/**************************************************************************
	Define
**************************************************************************/

//general
#define STR_NEWITEMNAME					"�V�����A�C�e��"

#define STR_Q_TITLE_DEL					"�폜"
#define STR_Q_MSG_DEL					"�폜���Ă���낵���ł����H"

#define STR_TITLE_FILESELECT			"�t�@�C���̑I��"

#define STR_ERROR_MEMORY				"�������̊m�ۂɎ��s���܂����B"

//main.c
#define STR_TOOL_TITLE_BROWSINFO		"�u���E�U���擾(&B)"
#define STR_TOOL_TITLE_BROWSINFOLIST	"���ׂẴu���E�U��񂩂�I��(&A)..."
#define STR_TOOL_TITLE_OPENURL			"�����̃u���E�U�ɊJ��(&D)"
#define STR_TOOL_TITLE_FILTERRELOAD		"�t�B���^�ēǂݍ���(&F)"

#define STR_ERR_TITLE_OPEN				"�J��"

#define STR_FILTER_EXESELECT			"���s�t�@�C�� (*.exe)\0*.exe\0\0"

//http.c
#define STR_TITLE_HEADVIEW				"�w�b�_�[�\�� - [%s]"

#define STR_TITLE_SOURCEVIEW			"�\�[�X�\��%s - [%s]"
#define STR_TITLE_FILTER				" (�t�B���^)"
#define STR_TITLE_FILTERERR				" (�t�B���^ �G���[)"

#define STR_ERR_TITLE_URLOPEN			"URL���J��"
#define STR_ERR_MSG_URLOPEN				"URL���ݒ肳��Ă��܂���B"

#define STR_ERROR_URL					"URL��W�J�ł��܂���ł����B"
#define STR_ERROR_GETHOST_TH			"gethostbyname�p�X���b�h�̍쐬�Ɏ��s���܂����B"
#define STR_ERROR_GETHOST				"IP�A�h���X�̎擾�Ɏ��s���܂����B"
#define STR_ERROR_SOCKET				"soket�̍쐬�Ɏ��s���܂����B"
#define STR_ERROR_SELECT				"WSAAsyncSelect�Ɏ��s���܂����B"
#define STR_ERROR_CONNECT				"�ڑ��Ɏ��s���܂����B"
#define STR_ERROR_SEND					"���M�Ɏ��s���܂����B"
#define STR_ERROR_RECV					"��M�Ɏ��s���܂����B"
#define STR_ERROR_HEADER				"�w�b�_����͂ł��܂���ł����B"
#define STR_ERROR_MOVE					"�]���G���["
#define STR_ERROR_TIMEOUT				"�^�C���A�E�g���܂����B"

#define STR_STATUS_GETHOST				"�z�X�g�����擾���Ă��܂�..."
#define STR_STATUS_CONNECT				"�ڑ���..."
#define STR_STATUS_RESPONSE				"�T�[�o����̉�����҂��Ă��܂�..."
#define STR_STATUS_RECV					"%ld �o�C�g��M"
#define STR_STATUS_CHECKEND				"�`�F�b�N�I��"

#define STR_GETINFO_HTTP_NEWMENU		"HTTP�A�C�e���̒ǉ�(&H)..."
#define STR_GETINFO_HTTP_OPEN			"�J��(&O)"
#define STR_GETINFO_HTTP_CHECKOPEN		"�`�F�b�N����URL�ŊJ��(&U)"
#define STR_GETINFO_HTTP_HEADER			"�w�b�_�[�\��(&H)"
#define STR_GETINFO_HTTP_SOURCE			"�\�[�X�\��(&S)"
#define STR_GETINFO_HTTP_GETTITLE		"�^�C�g���擾(&L)"

//httpprop.c
#define STR_REQTYPE_AUTO_GET			"����(&A) (���� GET)"

#define STR_TITLE_PROP					"%s�̃v���p�e�B"
#define STR_TITLE_ADDITEM				"HTTP�A�C�e���̒ǉ�"

#define STR_TITLE_SETHTTP				"HTTP�v���g�R���̐ݒ�"

//httptools.c
#define STR_ERR_TITLE_DDEEINIT			"DDE������"
#define STR_ERR_MSG_DDEEINIT			"DDE�̏������Ɏ��s���܂����B"

#define STR_ERR_TITLE_DDE				"�u���E�U���̎擾"
#define STR_ERR_MSG_DDE					"�u���E�U���N������Ă��Ȃ����A�u���E�U���̎擾�Ɏ��s���܂����B"

#define ST_LV_COL_TITLE					"�^�C�g��"
#define ST_LV_COL_FLAME					"�t���[��"
#define ST_LV_COL_URL					"URL"

//file.c
#define STR_ERR_TITLE_FILEOPEN			"�t�@�C�����J��"
#define STR_ERR_MSG_FILEOPEN			"�t�@�C�����ݒ肳��Ă��܂���B"

#define STR_FILTER_FILESELECT			"���ׂẴt�@�C�� (*.*)\0*.*\0\0"

#define STR_GETINFO_FILE_NEWMENU		"�t�@�C���̒ǉ�(&I)..."
#define STR_GETINFO_FILE_OPEN			"�J��(&O)"

#endif
/* End of source */
