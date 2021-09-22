#include <Windows.h>
#include <WindowsX.h>
#include <CommCtrl.h>
#include <tchar.h>
#include <stdio.h>
#include <math.h>
#include "resource.h"
#include "../sgshared/sgshared.h"


//MFC��Դ��ʼ���������

//https://docs.microsoft.com/zh-cn/cpp/mfc/tn024-mfc-defined-messages-and-resources ����������
#define RT_DLGINIT MAKEINTRESOURCE(240)

typedef struct
{
	WORD iControlID;
	WORD iMessage;
	DWORD dwSizeOfData;
}DLGINITHEADER;

BOOL CALLBACK MFCInitDlgCallback(HMODULE hModule, LPCWSTR lpType, LPWSTR lpName, LONG_PTR lParam)
{
	HRSRC hRsrc = FindResource(hModule, lpName, lpType);
	LPVOID ptr = LockResource(LoadResource(hModule, hRsrc));
	size_t sz = SizeofResource(hModule, hRsrc);
	size_t pos = 0;
	while (sz - pos >= sizeof(DLGINITHEADER))
	{
		DLGINITHEADER* hdr = (DLGINITHEADER*)((LPBYTE)ptr + pos);
		SendDlgItemMessageA((HWND)lParam, hdr->iControlID, CB_ADDSTRING, 0, (LPARAM)((LPBYTE)hdr + sizeof(DLGINITHEADER)));
		pos = pos + sizeof(DLGINITHEADER) + hdr->dwSizeOfData;
	}
	return TRUE;
}

BOOL OnMFCInitDialog(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	EnumResourceNames(GetModuleHandle(NULL), RT_DLGINIT, MFCInitDlgCallback, (LPARAM)hWnd);
	return FALSE;
}

//MFC��Դ��ʼ������������



//�Ի���ص�������

#define ONCBK(x) if(x(hWnd,msg,wParam,lParam)==TRUE)return TRUE
#define DECLCBK(x) BOOL x(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)

DECLCBK(OnInitDialog);
DECLCBK(OnMinimize);
DECLCBK(OnRestore);
DECLCBK(OnEndDialog);
DECLCBK(OnCheckTurnOnOff);
DECLCBK(OnSliderSpeed);
DECLCBK(OnButtonModeHelp);
DECLCBK(OnButtonStatusHelp);
DECLCBK(OnButtonStatusFont);
DECLCBK(OnButtonSpeedText);
DECLCBK(OnComboMode);
DECLCBK(OnComboStatusPosition);
DECLCBK(OnComboStatusBackground);
DECLCBK(OnComboStatusFormat);
DECLCBK(OnPaint);
INT_PTR OnCtlColorStaticStatusPreview(HWND, UINT, WPARAM, LPARAM);

INT_PTR CALLBACK DlgCallback(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:ONCBK(OnInitDialog);break;
	case WM_COMMAND:
		//HIWORD(wParam):0=�˵� 1=������ ����=�ؼ�֪ͨ��
		//lParam:�ؼ����ھ��
		switch (LOWORD(wParam))
		{
		case IDCANCEL:EndDialog(hWnd, 0);ONCBK(OnEndDialog);break;
		case IDC_CHECK_TURN_ON_OFF:ONCBK(OnCheckTurnOnOff);break;
		case IDC_SLIDER_SPEED:ONCBK(OnSliderSpeed);break;
		case IDC_BUTTON_MODE_HELP:ONCBK(OnButtonModeHelp);break;
		case IDC_BUTTON_STATUS_HELP:ONCBK(OnButtonStatusHelp);break;
		case IDC_BUTTON_STATUS_FONT:ONCBK(OnButtonStatusFont);break;
		case IDC_BUTTON_SPEED_TEXT:ONCBK(OnButtonSpeedText);break;
		case IDC_COMBO_MODE:ONCBK(OnComboMode);break;
		case IDC_COMBO_STATUS_POSITION:ONCBK(OnComboStatusPosition);break;
		case IDC_COMBO_STATUS_BACKGROUND:ONCBK(OnComboStatusBackground);break;
		case IDC_COMBO_STATUS_FORMAT:ONCBK(OnComboStatusFormat);break;
		}
		break;
	case WM_PAINT:ONCBK(OnPaint);break;
	case WM_SYSCOMMAND:
		switch (wParam)
		{
		case SC_MINIMIZE:ONCBK(OnMinimize);break;
		case SC_RESTORE:ONCBK(OnRestore);break;
		}
		break;
	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code)
		{
		case NM_CUSTOMDRAW:
			switch (((LPNMHDR)lParam)->idFrom)
			{
			case IDC_SLIDER_SPEED:ONCBK(OnSliderSpeed);break;
			}
			break;
		}
		break;
	case WM_CTLCOLORSTATIC:
		if (GetDlgCtrlID(lParam) == IDC_STATIC_STATUS_PREVIEW)
			return OnCtlColorStaticStatusPreview(hWnd, msg, wParam, lParam);
		break;
	}
	return 0;
}

int WINAPI _tWinMain(_In_ HINSTANCE hInst, _In_opt_ HINSTANCE hPrevInst, _In_ LPTSTR param, _In_ int iShow)
{
	return DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG_MAIN), NULL, DlgCallback);
}



//������Ҫ����ʵ��

BOOL InitSpeedSlider(HWND hwnd)
{
	HWND hSlider = GetDlgItem(hwnd, IDC_SLIDER_SPEED);
	SendMessage(hSlider, TBM_SETRANGE, FALSE, MAKELPARAM(0, 256));
	SendMessage(hSlider, TBM_SETPOS, TRUE, 128);
	return TRUE;
}

void SetSpeedSlider(HWND hwnd, float speed)
{
	char buf[16];
	sprintf_s(buf, ARRAYSIZE(buf), "%.3f&x", speed);
	SetDlgItemTextA(hwnd, IDC_BUTTON_SPEED_TEXT, buf);
	SendMessage(GetDlgItem(hwnd, IDC_SLIDER_SPEED), TBM_SETPOS, TRUE, (LPARAM)(128.0f + log2f(speed) * 128.0f / 3.0f));
}

float GetSpeedSlider(HWND hwnd)
{
	return powf(2.0f, (SendMessage(GetDlgItem(hwnd, IDC_SLIDER_SPEED), TBM_GETPOS, 0, 0) - 128.0f) * 3.0f / 128.0f);
}

char _iniSaveIntBuf[16];
#define INI_READ_INT(key) GetPrivateProfileIntA("SpeedGear",key,0,".\\sg.ini")
#define INI_SAVE_INT(key,value) wsprintfA(_iniSaveIntBuf,"%d",value);WritePrivateProfileStringA("SpeedGear",key,_iniSaveIntBuf,".\\sg.ini")
#define INI_READ_STR(key,pStr,szStr) GetPrivateProfileStringA("SpeedGear",key,"",pStr,szStr,".\\sg.ini")
#define INI_SAVE_STR(key,pStr) WritePrivateProfileStringA("SpeedGear",key,pStr,".\\sg.ini")

LOGFONTA g_logFont = { 0 };
CHOOSEFONTA g_cf = { sizeof(CHOOSEFONTA),0,0,&g_logFont,0,CF_INITTOLOGFONTSTRUCT | CF_EFFECTS | CF_SCREENFONTS,0 };

void RefreshPreviewText(HWND hwnd)
{
	int p = ComboBox_GetCurSel(GetDlgItem(hwnd, IDC_COMBO_STATUS_POSITION));
	HWND hStatic = GetDlgItem(hwnd, IDC_STATIC_STATUS_PREVIEW);
	switch (p % 3)
	{
	case 0:SetWindowLong(hStatic, GWL_STYLE, (GetWindowStyle(hStatic) & 0xFFFFFFFC) | SS_LEFT); break;
	case 1:SetWindowLong(hStatic, GWL_STYLE, (GetWindowStyle(hStatic) & 0xFFFFFFFC) | SS_CENTER); break;
	case 2:SetWindowLong(hStatic, GWL_STYLE, (GetWindowStyle(hStatic) & 0xFFFFFFFC) | SS_RIGHT); break;
	}
	switch (p / 3)
	{
	case 0:SetWindowLong(hStatic, GWL_STYLE, GetWindowStyle(hStatic) & ~SS_CENTERIMAGE); break;
	case 1:SetWindowLong(hStatic, GWL_STYLE, GetWindowStyle(hStatic) | SS_CENTERIMAGE); break;
	case 2:SetWindowLong(hStatic, GWL_STYLE, GetWindowStyle(hStatic) | SS_CENTERIMAGE); break;
	}
	char text[256], fmttext[256];
	GetEditComboBoxText(GetDlgItem(hwnd, IDC_COMBO_STATUS_FORMAT), text, ARRAYSIZE(text));
	SpeedGear_FormatText(fmttext, ARRAYSIZE(fmttext), text, 1.0f, 60, 800, 600, 9, 0, 0);
	SetDlgItemTextA(hwnd, IDC_STATIC_STATUS_PREVIEW, fmttext);
}

void SetButtonFontText(HWND hwnd)
{
	char buf[256];
	wsprintfA(buf, "��������(&T) [%s,%d%s,%d,#%06X]", g_logFont.lfFaceName, g_logFont.lfWeight, g_logFont.lfItalic ? ",��б" : "", FONTHEIGHT_TO_POUND(g_logFont.lfHeight), g_cf.rgbColors);
	if (lstrlenA(g_logFont.lfFaceName) == 0)
		*strchr(buf, ' ') = 0;
	SetDlgItemTextA(hwnd, IDC_BUTTON_STATUS_FONT, buf);
	HFONT hFont = CreateFontA(g_logFont.lfHeight, 0, 0, 0, g_logFont.lfWeight, g_logFont.lfItalic, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, g_logFont.lfFaceName);
	HWND hStatic = GetDlgItem(hwnd, IDC_STATIC_STATUS_PREVIEW);
	SendMessage(hStatic, WM_SETFONT, (WPARAM)hFont, TRUE);
}

BOOL GuiReadMem(HWND hwnd)
{
	SPEEDGEAR_SHARED_MEMORY* pMem = SpeedGear_GetSharedMemory();
	ComboBox_SetCurSel(GetDlgItem(hwnd, IDC_COMBO_MODE), pMem->hookMode);
	SetWindowTextA(GetDlgItem(hwnd, IDC_COMBO_STATUS_FORMAT), pMem->statusFormat);
	ComboBox_SetCurSel(GetDlgItem(hwnd, IDC_COMBO_STATUS_POSITION), pMem->statusPosition);

	lstrcpyA(g_logFont.lfFaceName, pMem->fontName);
	g_logFont.lfWeight = pMem->fontWeight;
	g_logFont.lfItalic = pMem->fontItalic;
	g_logFont.lfHeight = pMem->fontHeight;
	g_cf.rgbColors = pMem->fontColor;

	SetButtonFontText(hwnd);

	CheckDlgButton(hwnd, IDC_CHECK_TURN_ON_OFF, pMem->hookIsOn);
	SetSpeedSlider(hwnd, pMem->hookSpeed);
	return TRUE;
}

int GetEditComboBoxText(HWND hwndCtrl, char* pBuf, int len)
{
	int s = ComboBox_GetCurSel(hwndCtrl);
	if (s == -1)
		return GetWindowTextA(hwndCtrl, pBuf, len);
	return (int)SendMessageA(hwndCtrl, CB_GETLBTEXT, (WPARAM)s, (LPARAM)pBuf);
}

BOOL GuiSaveMem(HWND hwnd)
{
	SPEEDGEAR_SHARED_MEMORY* pMem = SpeedGear_GetSharedMemory();
	pMem->hookMode = ComboBox_GetCurSel(GetDlgItem(hwnd, IDC_COMBO_MODE));
	GetEditComboBoxText(GetDlgItem(hwnd, IDC_COMBO_STATUS_FORMAT), pMem->statusFormat, ARRAYSIZE(pMem->statusFormat));
	pMem->statusPosition = ComboBox_GetCurSel(GetDlgItem(hwnd, IDC_COMBO_STATUS_POSITION));

	lstrcpyA(pMem->fontName, g_logFont.lfFaceName);
	pMem->fontWeight = g_logFont.lfWeight;
	pMem->fontItalic = g_logFont.lfItalic;
	pMem->fontHeight = g_logFont.lfHeight;
	pMem->fontColor = g_cf.rgbColors;

	pMem->hookIsOn = IsDlgButtonChecked(hwnd, IDC_CHECK_TURN_ON_OFF);
	pMem->hookSpeed = GetSpeedSlider(hwnd);
	return TRUE;
}

BOOL MemReadIni()
{
	SPEEDGEAR_SHARED_MEMORY* pMem = SpeedGear_GetSharedMemory();
	pMem->hookMode = INI_READ_INT("hookMode");
	INI_READ_STR("statusFormat", pMem->statusFormat, ARRAYSIZE(pMem->statusFormat));
	pMem->statusPosition = INI_READ_INT("statusPosition");
	INI_READ_STR("fontName", pMem->fontName, ARRAYSIZE(pMem->fontName));
	pMem->fontHeight = POUND_TO_FONTHEIGHT(INI_READ_INT("fontHeight"));
	pMem->fontItalic = INI_READ_INT("fontItalic");
	pMem->fontWeight = INI_READ_INT("fontWeight");
	pMem->fontColor = INI_READ_INT("fontColor");
	return TRUE;
}

BOOL MemSaveIni()
{
	SPEEDGEAR_SHARED_MEMORY* pMem = SpeedGear_GetSharedMemory();
	INI_SAVE_INT("hookMode", pMem->hookMode);
	INI_SAVE_STR("statusFormat", pMem->statusFormat);
	INI_SAVE_INT("statusPosition", pMem->statusPosition);
	INI_SAVE_STR("fontName", pMem->fontName);
	INI_SAVE_INT("fontHeight", FONTHEIGHT_TO_POUND(pMem->fontHeight));
	INI_SAVE_INT("fontItalic", pMem->fontItalic);
	INI_SAVE_INT("fontWeight", pMem->fontWeight);
	INI_SAVE_INT("fontColor", pMem->fontColor);
	return TRUE;
}

HHOOK hHookKb = NULL;

LRESULT CALLBACK KbHookProc(int nCode,WPARAM wParam,LPARAM lParam)
{
	if (nCode == HC_ACTION)
	{
		switch (wParam)
		{
		case WM_KEYUP:case WM_SYSKEYUP:
		{
			PKBDLLHOOKSTRUCT pk = (PKBDLLHOOKSTRUCT)lParam;
			//TODO:pk->vkCode...
		}
			break;
		}
	}
	return CallNextHookEx(hHookKb, nCode, wParam, lParam);
}

HHOOK hHookSGList[16] = { NULL };

BOOL InitKbHook()
{
	if (hHookKb)
		return FALSE;
	hHookKb = SetWindowsHookEx(WH_KEYBOARD_LL, KbHookProc, GetModuleHandle(NULL), NULL);
	return TRUE;
}

BOOL ReleaseKbHook()
{
	return UnhookWindowsHookEx(hHookKb);
}

BOOL StopSpeedGear();
BOOL StartSpeedGear()
{
	int hookType = INI_READ_INT("hookType");
	if (hookType == 0)
		hookType = WH_CBT;
	char* dllName[] = {
#ifdef _M_IX86
		"sgd8.dll",
#endif
		"sgd9.dll","sgd11.dll","sggl.dll"
	};
	for (int i = 0; i < ARRAYSIZE(dllName); i++)
	{
		if (hHookSGList[i])
			return FALSE;
		HMODULE hDll = LoadLibraryA(dllName[i]);
		if (hDll == NULL)
		{
			StopSpeedGear();
			MessageBox(NULL, TEXT("�޷����� DLL �ļ���"), NULL, MB_ICONERROR);
			return FALSE;
		}
		HOOKPROC fProc = (HOOKPROC)GetProcAddress(hDll, SPEEDGEAR_PROC_STR);
		hHookSGList[i] = SetWindowsHookEx(hookType, fProc, hDll, 0);
		if (hHookSGList[i] == NULL)
		{
			StopSpeedGear();
			TCHAR msg[53];
			wsprintf(msg, TEXT("�޷�����Hook��%#x\n�볢�����������ó���������ǳ���������Ǹ����͵Ĺ��Ӳ���֧�֣��뵽�����ļ����޸�hookType������"), GetLastError());
			MessageBox(NULL, msg, NULL, MB_ICONERROR);
			return FALSE;
		}
	}
	return TRUE;
}

BOOL StopSpeedGear()
{
	for (int i = 0; i < ARRAYSIZE(hHookSGList); i++)
	{
		if (hHookSGList[i])
		{
			UnhookWindowsHookEx(hHookSGList[i]);
			hHookSGList[i] = NULL;
		}
	}
	return TRUE;
}


//�ص�����ʵ��

BOOL OnInitDialog(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	//����MFC��Դ�ĳ�ʼ��
	OnMFCInitDialog(hWnd, msg, wParam, lParam);

	InitSpeedSlider(hWnd);
	ComboBox_SetCurSel(GetDlgItem(hWnd, IDC_COMBO_STATUS_BACKGROUND), 0);

	if (SpeedGear_InitializeSharedMemory(TRUE) == FALSE)
	{
		MessageBox(hWnd, TEXT("���������ڴ�ʧ�ܡ�"), NULL, MB_ICONERROR);
		return FALSE;
	}
	SpeedGear_GetSharedMemory()->hookSpeed = 1.0f;
	MemReadIni();
	GuiReadMem(hWnd);
	RefreshPreviewText(hWnd);
	InitKbHook();

	return TRUE;
}

BOOL OnMinimize(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return FALSE;
}

BOOL OnRestore(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return FALSE;
}

BOOL OnEndDialog(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	ReleaseKbHook();
	if (SpeedGear_ReleaseSharedMemory() == FALSE)
	{
		MessageBox(hWnd, TEXT("�ͷŹ����ڴ�ʧ�ܡ�"), NULL, MB_ICONERROR);
	}
	return FALSE;
}

BOOL OnCheckTurnOnOff(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (IsDlgButtonChecked(hWnd, IDC_CHECK_TURN_ON_OFF))
	{
		if (!StartSpeedGear(hWnd))
			CheckDlgButton(hWnd, IDC_CHECK_TURN_ON_OFF, BST_UNCHECKED);
	}
	else
		StopSpeedGear(hWnd);
	return FALSE;
}

BOOL OnSliderSpeed(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	char buf[16];
	sprintf_s(buf, ARRAYSIZE(buf), "%.3f&x", GetSpeedSlider(hWnd));
	SetDlgItemTextA(hWnd, IDC_BUTTON_SPEED_TEXT, buf);
	GuiSaveMem(hWnd);
	MemSaveIni();
	return TRUE;
}

BOOL OnButtonModeHelp(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	MessageBoxA(hWnd, "����ģʽ��\n�޸�ʱ��API - �޸���ʱ����ص�APIʵ�ֱ���Ч��\n�޸�ͼ��API - �޸�ͼ��API��ʾ�߼�ʵ�ֱ���Ч��\n\n֧���޸ĵ�ͼ��API���£�\n"
#ifdef _M_IX86
		"Direct 3D 8, "
#endif
		"Direct 3D 9, Direct 3D 11, OpenGL", "����", MB_OK);
	return TRUE;
}

BOOL OnButtonStatusHelp(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	MessageBoxA(hWnd, "��������ı����������б�����\n{fps},{speed},{width},{height},{hour},{minute},{second}\n����������ڱ�����ָ����ʽ������{fps:%3d}.", "����", MB_OK);
	return TRUE;
}

BOOL OnButtonStatusFont(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	g_cf.hwndOwner = hWnd;
	if (ChooseFontA(&g_cf))
	{
		SetButtonFontText(hWnd);
		GuiSaveMem(hWnd);
		MemSaveIni();
		InvalidateRect(hWnd, NULL, FALSE);
		return TRUE;
	}
	return FALSE;
}

BOOL OnButtonSpeedText(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	SpeedGear_GetSharedMemory()->hookSpeed = 1.0f;
	SetSpeedSlider(hWnd, 1.0f);
	return TRUE;
}

BOOL OnComboMode(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (HIWORD(wParam) != CBN_SELCHANGE)
		return FALSE;
	GuiSaveMem(hWnd);
	MemSaveIni();
	return TRUE;
}

BOOL OnComboStatusPosition(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (HIWORD(wParam) != CBN_SELCHANGE)
		return FALSE;
	GuiSaveMem(hWnd);
	MemSaveIni();
	RefreshPreviewText(hWnd);
	InvalidateRect(hWnd, NULL, FALSE);
	return TRUE;
}

BOOL OnComboStatusBackground(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	RefreshPreviewText(hWnd);
	InvalidateRect(hWnd, NULL, FALSE);
	return TRUE;
}

BOOL OnComboStatusFormat(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (HIWORD(wParam))
	{
	case CBN_EDITCHANGE:
	case CBN_SELCHANGE:
		GuiSaveMem(hWnd);
		MemSaveIni();
		RefreshPreviewText(hWnd);
		InvalidateRect(hWnd, NULL, FALSE);
		break;
	}
	return TRUE;
}

BOOL OnPaint(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int pos = ComboBox_GetCurSel(GetDlgItem(hWnd, IDC_COMBO_STATUS_POSITION));
	if (pos < 6)
		return FALSE;
	PAINTSTRUCT ps;
	HWND hStatic = GetDlgItem(hWnd, IDC_STATIC_STATUS_PREVIEW);
	HDC hdc = BeginPaint(hStatic, &ps);
	int sob[] = { BLACK_BRUSH,WHITE_BRUSH,GRAY_BRUSH,LTGRAY_BRUSH };
	HBRUSH hbr = GetStockObject(sob[ComboBox_GetCurSel(GetDlgItem(hWnd, IDC_COMBO_STATUS_BACKGROUND))]);
	RECT r;
	GetClientRect(hStatic, &r);
	FillRect(hdc, &r, hbr);
	char t[256];
	GetDlgItemTextA(hWnd, IDC_STATIC_STATUS_PREVIEW, t, ARRAYSIZE(t));
	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, g_cf.rgbColors);
	HFONT hOldFont = SelectFont(hdc, CreateFontIndirectA(&g_logFont));
	DrawTextA(hdc, t, lstrlenA(t), &r, DT_SINGLELINE | DT_BOTTOM | (pos - 6));
	DeleteObject(SelectFont(hdc, hOldFont));
	EndPaint(hStatic, &ps);
	return FALSE;
}

INT_PTR OnCtlColorStaticStatusPreview(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HDC hdcStatic = (HDC)wParam;
	SetBkMode(hdcStatic, TRANSPARENT);
	SetTextColor(hdcStatic, g_cf.rgbColors);
	int sob[] = { BLACK_BRUSH,WHITE_BRUSH,GRAY_BRUSH,LTGRAY_BRUSH };
	//SetBkColor(hdcStatic, RGB(0, 255, 0));//���ֱ���ɫ�����������BkModeΪ͸���Ļ�����Ҫ���ô���
	return GetStockObject(sob[ComboBox_GetCurSel(GetDlgItem(hWnd, IDC_COMBO_STATUS_BACKGROUND))]);//�հ�������ɫ
}
