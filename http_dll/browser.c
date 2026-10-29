/**************************************************************************

	WWWC (wwwc.dll)

	browser.c

	Copyright (C) 1996-2026 by Ohno Tomoaki. All rights reserved.
		https://www.nakka.com/
		nakka@nakka.com

**************************************************************************/


/**************************************************************************
	Include Files
**************************************************************************/

#define COBJMACROS
#define CINTERFACE
#include <windows.h>
#include <ole2.h>
#include <oleacc.h>
#include <UIAutomationClient.h>

#include "browser.h"

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")
#pragma comment(lib, "oleacc.lib")
#pragma comment(lib, "uuid.lib")


/**************************************************************************
	Define
**************************************************************************/

#define CLASS_CHROMIUM			"Chrome_WidgetWin_1"			/* Chrome, Edge など */
#define CLASS_FIREFOX			"MozillaWindowClass"
#define CLASS_CHROMIUM_RENDER	"Chrome_RenderWidgetHostHWND"	/* ページを表示するウィンドウ */
#define CLASS_OMNIBOX			L"OmniboxViewViews"				/* Chromium のアドレスバー */
#define AID_FIREFOX_URLBAR		L"urlbar-input"					/* Firefox のアドレスバー */

#define BROWSER_MAX_WINDOWS		64
#define BROWSER_URL_MAX			4096
#define BROWSER_TITLE_MAX		512								/* タイトルの最大文字数 (ワイド文字) */
#define BROWSER_DOC_WAIT		1500							/* ページの情報が取得できるまで待つ時間 */
#define BROWSER_DOC_INTERVAL	250

typedef struct _WINLIST {
	HWND hWnd[BROWSER_MAX_WINDOWS];
	int cnt;
} WINLIST;



/**************************************************************************
	Helpers
**************************************************************************/

/* ブラウザのウィンドウを重なり順に列挙する */
static BOOL CALLBACK EnumBrowserProc(HWND hWnd, LPARAM lParam)
{
	WINLIST *wl = (WINLIST *)lParam;
	char cls[64];

	if(wl->cnt >= BROWSER_MAX_WINDOWS){
		return FALSE;
	}
	if(IsWindowVisible(hWnd) == FALSE || GetWindow(hWnd, GW_OWNER) != NULL ||
		GetClassName(hWnd, cls, sizeof(cls)) == 0){
		return TRUE;
	}
	if(lstrcmp(cls, CLASS_CHROMIUM) == 0 || lstrcmp(cls, CLASS_FIREFOX) == 0){
		wl->hWnd[wl->cnt++] = hWnd;
	}
	return TRUE;
}

/* 文字列のプロパティ (無い場合は NULL) */
static BSTR GetStringProperty(IUIAutomationElement *e, PROPERTYID id)
{
	VARIANT v;
	BSTR ret = NULL;

	VariantInit(&v);
	if(SUCCEEDED(IUIAutomationElement_GetCurrentPropertyValue(e, id, &v)) && v.vt == VT_BSTR && v.bstrVal != NULL){
		ret = SysAllocString(v.bstrVal);
	}
	VariantClear(&v);
	return ret;
}

/* 条件に一致する最初の子孫 */
static IUIAutomationElement *FindFirst(IUIAutomation *uia, IUIAutomationElement *root, IUIAutomationCondition *cond)
{
	IUIAutomationElement *e = NULL;

	if(cond == NULL || FAILED(IUIAutomationElement_FindFirst(root, TreeScope_Descendants, cond, &e))){
		return NULL;
	}
	return e;
}

static IUIAutomationCondition *StringCondition(IUIAutomation *uia, PROPERTYID id, const WCHAR *s)
{
	IUIAutomationCondition *cond = NULL;
	VARIANT v;

	v.vt = VT_BSTR;
	v.bstrVal = SysAllocString(s);
	if(v.bstrVal == NULL){
		return NULL;
	}
	IUIAutomation_CreatePropertyCondition(uia, id, v, &cond);
	SysFreeString(v.bstrVal);
	return cond;
}

static IUIAutomationCondition *TypeCondition(IUIAutomation *uia, CONTROLTYPEID type)
{
	IUIAutomationCondition *cond = NULL;
	VARIANT v;

	v.vt = VT_I4;
	v.lVal = type;
	IUIAutomation_CreatePropertyCondition(uia, UIA_ControlTypePropertyId, v, &cond);
	return cond;
}

static BOOL HasCharW(const WCHAR *s, WCHAR c)
{
	for(; *s != L'\0'; s++){
		if(*s == c){
			return TRUE;
		}
	}
	return FALSE;
}

static void ReleaseCondition(IUIAutomationCondition *cond)
{
	if(cond != NULL){
		IUIAutomationCondition_Release(cond);
	}
}

/* http, https の URL をASCIIの文字列にする (8ビット文字は UTF-8 で % エンコード) */
static char *ToUrl(const WCHAR *w)
{
	char *u, *ret, *r;
	const unsigned char *p;
	int len;

	if(w == NULL || lstrlenW(w) == 0 || lstrlenW(w) > BROWSER_URL_MAX){
		return NULL;
	}
	if(CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, w, 7, L"http://", 7) != CSTR_EQUAL &&
		CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, w, 8, L"https://", 8) != CSTR_EQUAL){
		return NULL;
	}
	len = WideCharToMultiByte(CP_UTF8, 0, w, -1, NULL, 0, NULL, NULL);
	if(len <= 0 || (u = (char *)HeapAlloc(GetProcessHeap(), 0, len)) == NULL){
		return NULL;
	}
	WideCharToMultiByte(CP_UTF8, 0, w, -1, u, len, NULL, NULL);
	ret = (char *)HeapAlloc(GetProcessHeap(), 0, len * 3 + 1);
	if(ret != NULL){
		for(p = (const unsigned char *)u, r = ret; *p != '\0'; p++){
			if(*p < 0x20 || *p == 0x7F){
				//制御文字を含むものは使用しない
				HeapFree(GetProcessHeap(), 0, ret);
				ret = NULL;
				break;
			}
			if(*p >= 0x80 || *p == ' ' || *p == '"' || *p == '<' || *p == '>'){
				wsprintf(r, "%%%02X", *p);
				r += 3;
			}else{
				*(r++) = (char)*p;
			}
		}
		if(ret != NULL){
			*r = '\0';
		}
	}
	HeapFree(GetProcessHeap(), 0, u);
	return ret;
}

/* タイトルを ANSI の文字列にする (制御文字は空白にする) */
static char *ToTitle(const WCHAR *w)
{
	WCHAR *tmp;
	char *ret, *p;
	int wl, len;

	if(w == NULL){
		return NULL;
	}
	//前後の空白を除いて最大文字数までにする
	for(; *w == L' ' || *w == L'\t'; w++);
	wl = lstrlenW(w);
	for(; wl > 0 && (w[wl - 1] == L' ' || w[wl - 1] == L'\t'); wl--);
	if(wl == 0){
		return NULL;
	}
	if(wl > BROWSER_TITLE_MAX){
		wl = BROWSER_TITLE_MAX;
	}
	tmp = (WCHAR *)HeapAlloc(GetProcessHeap(), 0, (wl + 1) * sizeof(WCHAR));
	if(tmp == NULL){
		return NULL;
	}
	CopyMemory(tmp, w, wl * sizeof(WCHAR));
	tmp[wl] = L'\0';
	len = WideCharToMultiByte(CP_ACP, 0, tmp, -1, NULL, 0, NULL, NULL);
	ret = (len > 0) ? (char *)HeapAlloc(GetProcessHeap(), 0, len) : NULL;
	if(ret != NULL){
		WideCharToMultiByte(CP_ACP, 0, tmp, -1, ret, len, NULL, NULL);
		for(p = ret; *p != '\0'; p++){
			if(IsDBCSLeadByte((BYTE)*p) == TRUE && *(p + 1) != '\0'){
				p++;
			}else if((unsigned char)*p < 0x20 || *p == 0x7F){
				*p = ' ';
			}
		}
	}
	HeapFree(GetProcessHeap(), 0, tmp);
	return ret;
}

/* ウィンドウのタイトルからブラウザ名を除く (最後の " - " または全角ダッシュ (U+2014) 以降) */
static WCHAR *TitleFromWindow(HWND hWnd)
{
	WCHAR *buf, *p, *last = NULL;
	int len;

	len = GetWindowTextLengthW(hWnd);
	if(len <= 0){
		return NULL;
	}
	buf = (WCHAR *)HeapAlloc(GetProcessHeap(), 0, (len + 1) * sizeof(WCHAR));
	if(buf == NULL){
		return NULL;
	}
	GetWindowTextW(hWnd, buf, len + 1);
	for(p = buf; *p != L'\0'; p++){
		if(*p == L' ' && (*(p + 1) == L'-' || *(p + 1) == 0x2014) && *(p + 2) == L' '){
			last = p;
		}
	}
	if(last != NULL){
		*last = L'\0';
	}
	return buf;
}

/* 区切り (" - ") の位置を後ろから検索 */
static WCHAR *FindLastSeparator(WCHAR *s, WCHAR *end)
{
	WCHAR *p;

	for(p = end - 3; p >= s; p--){
		if(*p == L' ' && *(p + 1) == L'-' && *(p + 2) == L' '){
			return p;
		}
	}
	return NULL;
}

/* タブ名の末尾のメモリ使用量の表示 (" - 名前 - 23.0 MB") を除く */
static void StripTabSuffix(WCHAR *s)
{
	WCHAR *sep, *p;

	sep = FindLastSeparator(s, s + lstrlenW(s));
	if(sep == NULL){
		return;
	}
	p = sep + 3;
	if(!(*p >= L'0' && *p <= L'9')){
		return;
	}
	for(; (*p >= L'0' && *p <= L'9') || *p == L'.' || *p == L','; p++);
	if(*p == L' '){
		p++;
	}
	if(!((*p == L'K' || *p == L'M' || *p == L'G') && *(p + 1) == L'B' && *(p + 2) == L'\0')){
		return;
	}
	*sep = L'\0';
	sep = FindLastSeparator(s, sep);
	if(sep != NULL){
		*sep = L'\0';
	}
}

/* 選択しているタブの名前 */
static WCHAR *SelectedTabTitle(IUIAutomation *uia, IUIAutomationElement *root)
{
	IUIAutomationCondition *c1, *c2 = NULL, *cond = NULL;
	IUIAutomationElement *tab;
	VARIANT v;
	BSTR name;
	WCHAR *ret = NULL;

	c1 = TypeCondition(uia, UIA_TabItemControlTypeId);
	v.vt = VT_BOOL;
	v.boolVal = VARIANT_TRUE;
	IUIAutomation_CreatePropertyCondition(uia, UIA_SelectionItemIsSelectedPropertyId, v, &c2);
	if(c1 != NULL && c2 != NULL){
		IUIAutomation_CreateAndCondition(uia, c1, c2, &cond);
	}
	ReleaseCondition(c1);
	ReleaseCondition(c2);
	tab = FindFirst(uia, root, cond);
	ReleaseCondition(cond);
	if(tab == NULL){
		return NULL;
	}
	name = GetStringProperty(tab, UIA_NamePropertyId);
	if(name != NULL && (ret = (WCHAR *)HeapAlloc(GetProcessHeap(), 0, (lstrlenW(name) + 1) * sizeof(WCHAR))) != NULL){
		lstrcpyW(ret, name);
		StripTabSuffix(ret);
	}
	SysFreeString(name);
	IUIAutomationElement_Release(tab);
	return ret;
}

/* 実行ファイル名からブラウザ名を取得 */
static void GetBrowserName(HWND hWnd, char *ret, int size)
{
	static const struct { const WCHAR *exe; const char *name; } names[] = {
		{L"chrome.exe", "Google Chrome"}, {L"msedge.exe", "Microsoft Edge"}, {L"firefox.exe", "Firefox"},
		{L"brave.exe", "Brave"}, {L"vivaldi.exe", "Vivaldi"}, {L"opera.exe", "Opera"}, {L"chromium.exe", "Chromium"},
	};
	WCHAR path[MAX_PATH], *p, *f;
	HANDLE hProcess;
	DWORD pid = 0, len = MAX_PATH;
	int i;

	lstrcpyn(ret, "", size);
	GetWindowThreadProcessId(hWnd, &pid);
	hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
	if(hProcess == NULL){
		return;
	}
	if(QueryFullProcessImageNameW(hProcess, 0, path, &len) == TRUE){
		for(f = p = path; *p != L'\0'; p++){
			if(*p == L'\\'){
				f = p + 1;
			}
		}
		for(i = 0; i < sizeof(names) / sizeof(names[0]); i++){
			if(lstrcmpiW(f, names[i].exe) == 0){
				lstrcpyn(ret, names[i].name, size);
				break;
			}
		}
		if(i >= sizeof(names) / sizeof(names[0])){
			WideCharToMultiByte(CP_ACP, 0, f, -1, ret, size, NULL, NULL);
			ret[size - 1] = '\0';
		}
	}
	CloseHandle(hProcess);
}

/* Chromium のページの情報を有効にする (支援技術からの問い合わせで有効になる) */
static void WakeAccessibility(HWND hWnd)
{
	IAccessible *acc;
	HWND child = NULL;
	int i;

	for(i = 0; i < 16 && (child = FindWindowEx(hWnd, child, CLASS_CHROMIUM_RENDER, NULL)) != NULL; i++){
		acc = NULL;
		if(SUCCEEDED(AccessibleObjectFromWindow(child, (DWORD)OBJID_CLIENT, &IID_IAccessible, (void **)&acc)) && acc != NULL){
			IAccessible_Release(acc);
		}
	}
}


/******************************************************************************

	GetWebDocument

	URL が http, https の最初の文書から URL とタイトルを取得する

******************************************************************************/

static BOOL GetWebDocument(IUIAutomation *uia, IUIAutomationElement *root, IUIAutomationCondition *cdoc, BROWSER_PAGE *page)
{
	IUIAutomationElementArray *arr = NULL;
	IUIAutomationElement *doc;
	BSTR url, name;
	int cnt = 0, i;

	if(FAILED(IUIAutomationElement_FindAll(root, TreeScope_Descendants, cdoc, &arr)) || arr == NULL){
		return FALSE;
	}
	IUIAutomationElementArray_get_Length(arr, &cnt);
	for(i = 0; i < cnt && page->Url == NULL; i++){
		doc = NULL;
		if(FAILED(IUIAutomationElementArray_GetElement(arr, i, &doc)) || doc == NULL){
			continue;
		}
		url = GetStringProperty(doc, UIA_ValueValuePropertyId);
		page->Url = ToUrl(url);
		if(page->Url != NULL){
			name = GetStringProperty(doc, UIA_NamePropertyId);
			page->Title = ToTitle(name);
			SysFreeString(name);
		}
		SysFreeString(url);
		IUIAutomationElement_Release(doc);
	}
	IUIAutomationElementArray_Release(arr);
	return (page->Url != NULL) ? TRUE : FALSE;
}


/******************************************************************************

	GetWindowPage

	ブラウザのウィンドウで表示しているページを取得する

******************************************************************************/

static BOOL GetWindowPage(IUIAutomation *uia, HWND hWnd, BROWSER_PAGE *page)
{
	IUIAutomationElement *root = NULL, *bar = NULL;
	IUIAutomationCondition *c1, *c2, *cbar = NULL, *cdoc;
	BSTR value = NULL;
	WCHAR *tmp = NULL, *wt;
	DWORD start;
	BOOL ret = FALSE;

	ZeroMemory(page, sizeof(BROWSER_PAGE));
	if(FAILED(IUIAutomation_ElementFromHandle(uia, hWnd, &root)) || root == NULL){
		return FALSE;
	}
	//アドレスバーがあるウィンドウのみ (ダイアログやブラウザ以外のアプリを除く)
	c1 = StringCondition(uia, UIA_ClassNamePropertyId, CLASS_OMNIBOX);
	c2 = StringCondition(uia, UIA_AutomationIdPropertyId, AID_FIREFOX_URLBAR);
	if(c1 != NULL && c2 != NULL){
		IUIAutomation_CreateOrCondition(uia, c1, c2, &cbar);
	}
	ReleaseCondition(c1);
	ReleaseCondition(c2);
	bar = FindFirst(uia, root, cbar);
	ReleaseCondition(cbar);
	if(bar == NULL){
		IUIAutomationElement_Release(root);
		return FALSE;
	}

	//ページの文書から URL とタイトルを取得する
	//(ブラウザ内部の画面 (同期の案内など) も文書になるため URL が http, https のものを使用する)
	cdoc = TypeCondition(uia, UIA_DocumentControlTypeId);
	if(cdoc != NULL){
		if(GetWebDocument(uia, root, cdoc, page) == FALSE){
			WakeAccessibility(hWnd);
			for(start = GetTickCount(); GetTickCount() - start < BROWSER_DOC_WAIT;){
				Sleep(BROWSER_DOC_INTERVAL);
				if(GetWebDocument(uia, root, cdoc, page) == TRUE){
					break;
				}
			}
		}
		ReleaseCondition(cdoc);
	}

	//取得できない場合はアドレスバーとウィンドウのタイトルを使用する
	if(page->Url == NULL && (value = GetStringProperty(bar, UIA_ValueValuePropertyId)) != NULL){
		page->Url = ToUrl(value);
		if(page->Url == NULL){
			//スキームを省略して表示されている場合 (http から https へは移動されるため http とする)
			BOOL host = (value[0] != L'\0') ? TRUE : FALSE;
			for(wt = value; *wt != L'\0' && *wt != L'/'; wt++){
				if(*wt == L' ' || (*wt == L':' && !(*(wt + 1) >= L'0' && *(wt + 1) <= L'9'))){
					host = FALSE;
				}
			}
			if(host == TRUE && HasCharW(value, L'.') == TRUE &&
				(tmp = (WCHAR *)HeapAlloc(GetProcessHeap(), 0, (lstrlenW(value) + 8) * sizeof(WCHAR))) != NULL){
				lstrcpyW(tmp, L"http://");
				lstrcatW(tmp, value);
				page->Url = ToUrl(tmp);
				HeapFree(GetProcessHeap(), 0, tmp);
			}
		}
	}
	//タイトルは選択しているタブの名前、無い場合はウィンドウのタイトルを使用する
	if(page->Url != NULL && page->Title == NULL && (wt = SelectedTabTitle(uia, root)) != NULL){
		page->Title = ToTitle(wt);
		HeapFree(GetProcessHeap(), 0, wt);
	}
	if(page->Url != NULL && page->Title == NULL && (wt = TitleFromWindow(hWnd)) != NULL){
		page->Title = ToTitle(wt);
		HeapFree(GetProcessHeap(), 0, wt);
	}
	if(page->Url != NULL){
		if(page->Title == NULL){
			page->Title = (char *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 1);
		}
		GetBrowserName(hWnd, page->Browser, sizeof(page->Browser));
		ret = (page->Title != NULL) ? TRUE : FALSE;
	}
	if(ret == FALSE){
		Browser_FreePage(page);
	}

	SysFreeString(value);
	IUIAutomationElement_Release(bar);
	IUIAutomationElement_Release(root);
	return ret;
}


/******************************************************************************

	Browser_FreePage

******************************************************************************/

void Browser_FreePage(BROWSER_PAGE *page)
{
	if(page->Title != NULL){
		HeapFree(GetProcessHeap(), 0, page->Title);
	}
	if(page->Url != NULL){
		HeapFree(GetProcessHeap(), 0, page->Url);
	}
	ZeroMemory(page, sizeof(BROWSER_PAGE));
}


/******************************************************************************

	Browser_FreePages

******************************************************************************/

void Browser_FreePages(BROWSER_PAGE *pages, int cnt)
{
	int i;

	if(pages == NULL){
		return;
	}
	for(i = 0; i < cnt; i++){
		Browser_FreePage(&pages[i]);
	}
	HeapFree(GetProcessHeap(), 0, pages);
}


/******************************************************************************

	Browser_GetPages

	ブラウザのウィンドウで表示しているページを取得する
	(UI Automation を使用、First が TRUE の場合は最後にアクティブだったウィンドウのみ)

******************************************************************************/

int Browser_GetPages(BROWSER_PAGE **ret, BOOL First)
{
	IUIAutomation *uia = NULL;
	IUIAutomation2 *uia2 = NULL;
	WINLIST wl;
	BROWSER_PAGE *pages;
	HRESULT hr;
	int i, cnt = 0;

	*ret = NULL;
	ZeroMemory(&wl, sizeof(WINLIST));
	EnumWindows(EnumBrowserProc, (LPARAM)&wl);
	if(wl.cnt == 0){
		return 0;
	}
	pages = (BROWSER_PAGE *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(BROWSER_PAGE) * wl.cnt);
	if(pages == NULL){
		return 0;
	}
	hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	if(SUCCEEDED(CoCreateInstance(&CLSID_CUIAutomation, NULL, CLSCTX_INPROC_SERVER, &IID_IUIAutomation, (void **)&uia)) && uia != NULL){
		//応答しないブラウザで待ち続けないようにする
		if(SUCCEEDED(IUIAutomation_QueryInterface(uia, &IID_IUIAutomation2, (void **)&uia2)) && uia2 != NULL){
			IUIAutomation2_put_ConnectionTimeout(uia2, 2000);
			IUIAutomation2_put_TransactionTimeout(uia2, 5000);
			IUIAutomation2_Release(uia2);
		}
		for(i = 0; i < wl.cnt; i++){
			if(GetWindowPage(uia, wl.hWnd[i], &pages[cnt]) == TRUE){
				cnt++;
				if(First == TRUE){
					break;
				}
			}
		}
		IUIAutomation_Release(uia);
	}
	if(SUCCEEDED(hr)){
		CoUninitialize();
	}
	if(cnt == 0){
		HeapFree(GetProcessHeap(), 0, pages);
		return 0;
	}
	*ret = pages;
	return cnt;
}
/* End of source */
