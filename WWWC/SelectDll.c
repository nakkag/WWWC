/**************************************************************************

	WWWC

	SelectDll.c

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

//�O���Q��
extern HINSTANCE g_hinst;				//�A�v���P�[�V�����̃C���X�^���X�n���h��
extern HWND WWWCWnd;					//�{��

extern HWND ProtocolWnd;
extern HWND ToolWnd;

extern struct TP_PROTOCOL *tpProtocol;
extern int ProtocolCnt;

extern struct TP_TOOLS *ToolList;
extern int ToolListCnt;

extern int LvLIconSize;
extern int LvSIconSize;


/**************************************************************************
	Local Function Prototypes
**************************************************************************/

static BOOL AddProtocol(struct TP_PROTOCOL *NewProtocol);
static BOOL CALLBACK SelectDllProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);


/******************************************************************************

	AddProtocol

	�v���g�R���̒ǉ�

******************************************************************************/

static BOOL AddProtocol(struct TP_PROTOCOL *NewProtocol)
{
	struct TP_PROTOCOL *TmpProtocol;
	int Cnt;

	Cnt = ProtocolCnt;
	TmpProtocol = (struct TP_PROTOCOL *)GlobalAlloc(GPTR, sizeof(struct TP_PROTOCOL) * (Cnt + 1));
	if(TmpProtocol == NULL){
		return FALSE;
	}
	if(tpProtocol != NULL){
		CopyMemory(TmpProtocol, tpProtocol, sizeof(struct TP_PROTOCOL) * Cnt);
	}
	CopyMemory(TmpProtocol + Cnt, NewProtocol, sizeof(struct TP_PROTOCOL));

	if(tpProtocol != NULL){
		GlobalFree(tpProtocol);
	}
	tpProtocol = TmpProtocol;
	ProtocolCnt = Cnt + 1;
	return TRUE;
}


/******************************************************************************

	AddTool

	�v���g�R���̒ǉ�

******************************************************************************/

static BOOL AddTool(struct TP_TOOLS *NewTool)
{
	struct TP_TOOLS *TmpTool;
	int Cnt;

	Cnt = ToolListCnt;
	TmpTool = (struct TP_TOOLS *)GlobalAlloc(GPTR, sizeof(struct TP_TOOLS) * (Cnt + 1));
	if(TmpTool == NULL){
		return FALSE;
	}
	if(ToolList != NULL){
		CopyMemory(TmpTool, ToolList, sizeof(struct TP_TOOLS) * Cnt);
	}
	CopyMemory(TmpTool + Cnt, NewTool, sizeof(struct TP_TOOLS));

	if(ToolList != NULL){
		GlobalFree(ToolList);
	}
	ToolList = TmpTool;
	ToolListCnt = Cnt + 1;
	return TRUE;
}


/******************************************************************************

	SelectDllProc

	DLL���v���g�R���A�c�[���I���E�B���h�E�v���V�[�W��

******************************************************************************/

static BOOL CALLBACK SelectDllProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	struct TP_PROTOCOL tpProtocolInfo;
	struct TPPROTOCOLSET tpProtocolSet;
	struct TP_TOOLS tpSetTool;
	struct TP_GETTOOL GetTool;
	HINSTANCE hLib;
	FARPROC Func_GetProtocolList;
	FARPROC Func_GetToolList;
	static char DLLPath[BUFSIZE];
	int cnt = 0;
	int ret, i;

	switch (uMsg)
	{
	case WM_INITDIALOG:
		if(lParam == 0){
			EndDialog(hDlg, FALSE);
			break;
		}
		lstrcpy(DLLPath, (char *)lParam);
		SetWindowText(hDlg, DLLPath);

		hLib = LoadLibrary(DLLPath);
		if(hLib == NULL){
			ErrMsg(hDlg, GetLastError(), NULL);
			EndDialog(hDlg, FALSE);
			break;
		}

		//�v���g�R�����擾�֐��̃A�h���X���擾
		Func_GetProtocolList = GetProcAddress((HMODULE)hLib, "GetProtocolList");
		if(Func_GetProtocolList != NULL){
			//�v���g�R�������擾
			while(1){
				ZeroMemory(&tpProtocolSet, sizeof(struct TPPROTOCOLSET));
				tpProtocolSet.iSize = sizeof(struct TPPROTOCOLSET);
				ret = Func_GetProtocolList(cnt++, &tpProtocolSet);
				if(ret <= -1){
					break;
				}
				SendDlgItemMessage(hDlg, IDC_LIST_PROTOCOL, LB_ADDSTRING, 0, (LPARAM)tpProtocolSet.Title);
			}
		}

		//�c�[���擾�֐��̃A�h���X���擾
		Func_GetToolList = GetProcAddress((HMODULE)hLib, "GetToolList");
		if(Func_GetToolList != NULL){
			//�c�[�������擾
			cnt = 0;
			while(1){
				ZeroMemory(&GetTool, sizeof(struct TP_GETTOOL));
				GetTool.iSize = sizeof(struct TP_GETTOOL);
				ret = Func_GetToolList(cnt++, &GetTool);
				if(ret <= -1){
					break;
				}
				SendDlgItemMessage(hDlg, IDC_LIST_TOOL, LB_ADDSTRING, 0, (LPARAM)GetTool.title);
			}
		}
		FreeLibrary(hLib);

		if(SendDlgItemMessage(hDlg, IDC_LIST_PROTOCOL, LB_GETCOUNT, 0, 0) <= 0 &&
			SendDlgItemMessage(hDlg, IDC_LIST_TOOL, LB_GETCOUNT, 0, 0) <= 0){
			MessageBox(hDlg, EMSG_DLL, EMSG_DLL_TITLE, MB_OK | MB_ICONEXCLAMATION);
			EndDialog(hDlg, FALSE);
		}
		break;

	case WM_CLOSE:
		EndDialog(hDlg, FALSE);
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_BUTTON_SELECT_P:
			for(i = 0; i < SendDlgItemMessage(hDlg, IDC_LIST_PROTOCOL, LB_GETCOUNT, 0, 0); i++){
				SendDlgItemMessage(hDlg, IDC_LIST_PROTOCOL, LB_SETSEL, TRUE, i);
			}
			break;

		case IDC_BUTTON_SELECT_T:
			for(i = 0; i < SendDlgItemMessage(hDlg, IDC_LIST_TOOL, LB_GETCOUNT, 0, 0); i++){
				SendDlgItemMessage(hDlg, IDC_LIST_TOOL, LB_SETSEL, TRUE, i);
			}
			break;

		case IDCANCEL:
			SendMessage(hDlg, WM_CLOSE, 0, 0);
			break;

		case IDOK:
			if(SendDlgItemMessage(hDlg, IDC_LIST_PROTOCOL, LB_GETSELCOUNT, 0, 0) <= 0 &&
				SendDlgItemMessage(hDlg, IDC_LIST_TOOL, LB_GETSELCOUNT, 0, 0) <= 0){
				MessageBox(hDlg, EMSG_SELECT, EMSG_DLL_TITLE, MB_OK | MB_ICONEXCLAMATION);
				break;
			}

			hLib = LoadLibrary(DLLPath);
			if(hLib == NULL){
				ErrMsg(hDlg, GetLastError(), NULL);
				EndDialog(hDlg, FALSE);
				break;
			}

			//�v���g�R�����擾�֐��̃A�h���X���擾
			Func_GetProtocolList = GetProcAddress((HMODULE)hLib, "GetProtocolList");
			if(Func_GetProtocolList != NULL){
				//�v���g�R���̒ǉ�
				for(i = 0; i < SendDlgItemMessage(hDlg, IDC_LIST_PROTOCOL, LB_GETCOUNT, 0, 0); i++){
					if(SendDlgItemMessage(hDlg, IDC_LIST_PROTOCOL, LB_GETSEL, i, 0) == 0){
						continue;
					}
					ZeroMemory(&tpProtocolSet, sizeof(struct TPPROTOCOLSET));
					tpProtocolSet.iSize = sizeof(struct TPPROTOCOLSET);
					tpProtocolSet.UpIconIndex = -1;

					//�v���g�R�������擾
					ret = Func_GetProtocolList(i, &tpProtocolSet);
					if(ret <= -1){
						break;
					}
					ZeroMemory(&tpProtocolInfo, sizeof(struct TP_PROTOCOL));
					lstrcpy(tpProtocolInfo.DLL, DLLPath);
					lstrcpy(tpProtocolInfo.title, tpProtocolSet.Title);
					lstrcpy(tpProtocolInfo.FuncHeader, tpProtocolSet.FuncHeader);
					lstrcpy(tpProtocolInfo.IconFile, tpProtocolSet.IconFile);
					tpProtocolInfo.IconIndex = tpProtocolSet.IconIndex;
					lstrcpy(tpProtocolInfo.UpIconFile, tpProtocolSet.UpIconFile);
					tpProtocolInfo.UpIconIndex = tpProtocolSet.UpIconIndex;
					tpProtocolInfo.lib = hLib;

					if(AddProtocol(&tpProtocolInfo) == FALSE){
						ErrMsg(hDlg, GetLastError(), NULL);
						EndDialog(hDlg, FALSE);
						break;
					}
					if(ProtocolWnd != NULL){
						SendMessage(ProtocolWnd, WM_ADDPROTOCOL, 0, (LPARAM)&tpProtocolInfo);
					}
				}
			}

			//�c�[���擾�֐��̃A�h���X���擾
			Func_GetToolList = GetProcAddress((HMODULE)hLib, "GetToolList");
			if(Func_GetToolList != NULL){
				//�c�[���̒ǉ�
				for(i = 0; i < SendDlgItemMessage(hDlg, IDC_LIST_TOOL, LB_GETCOUNT, 0, 0); i++){
					if(SendDlgItemMessage(hDlg, IDC_LIST_TOOL, LB_GETSEL, i, 0) == 0){
						continue;
					}
					ZeroMemory(&GetTool, sizeof(struct TP_GETTOOL));
					GetTool.iSize = sizeof(struct TP_GETTOOL);
					GetTool.MenuIndex = -1;
					ret = Func_GetToolList(i, &GetTool);

					ZeroMemory(&tpSetTool, sizeof(struct TP_TOOLS));
					CopyMemory(&tpSetTool, &GetTool, sizeof(struct TP_GETTOOL));
					lstrcpy(tpSetTool.FileName, DLLPath);
					tpSetTool.lib = hLib;

					if(AddTool(&tpSetTool) == FALSE){
						ErrMsg(hDlg, GetLastError(), NULL);
						EndDialog(hDlg, FALSE);
						break;
					}
					if(ToolWnd != NULL){
						SendMessage(ToolWnd, WM_ADDTOOL, 0, (LPARAM)&tpSetTool);
					}
				}
			}

			//�C���[�W���X�g�̍X�V
			ListView_SetItemImage(GetDlgItem(WWWCWnd, WWWC_LIST), LvLIconSize, LVSIL_NORMAL);
			ListView_SetItemImage(GetDlgItem(WWWCWnd, WWWC_LIST), LvSIconSize, LVSIL_SMALL);
			SetProtocolImage(WWWCWnd);
			SetProtocolInfo();

			SetToolMenu(WWWCWnd);
			SetEnableToolMenu(WWWCWnd);
			SetNewItemMenu(WWWCWnd);
			SetProtocolMenu(WWWCWnd);

			EndDialog(hDlg, TRUE);
			break;
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}


/******************************************************************************

	SelectDll

	DLL�I���E�B���h�E�̕\��

******************************************************************************/

BOOL SelectDll(HWND hWnd, char *DLLPath)
{
	return DialogBoxParam(g_hinst, MAKEINTRESOURCE(IDD_DIALOG_SELECTDLL), hWnd, SelectDllProc, (LPARAM)DLLPath);
}

/* End of source */
