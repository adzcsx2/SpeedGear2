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
DECLCBK(OnComboMode);
DECLCBK(OnComboStatusPosition);
DECLCBK(OnComboStatusBackground);
DECLCBK(OnComboStatusFormat);

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
		case IDC_COMBO_MODE:ONCBK(OnComboMode);break;
		case IDC_COMBO_STATUS_POSITION:ONCBK(OnComboStatusPosition);break;
		case IDC_COMBO_STATUS_BACKGROUND:ONCBK(OnComboStatusBackground);break;
		case IDC_COMBO_STATUS_FORMAT:ONCBK(OnComboStatusFormat);break;
		}
		break;
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
	SendMessage(hSlider, TBM_SETRANGE, FALSE, MAKELPARAM(0, 6));
	SendMessage(hSlider, TBM_SETPOS, TRUE, 3);
	SetDlgItemTextA(hwnd, IDC_STATIC_SPEED_TEXT, "1.00x");
	return TRUE;
}

void SetSpeedSlider(HWND hwnd, float speed)
{
	char buf[16];
	sprintf_s(buf, ARRAYSIZE(buf), "%.3fx", speed);
	SetDlgItemTextA(hwnd, IDC_STATIC_SPEED_TEXT, buf);
	SendMessage(GetDlgItem(hwnd, IDC_SLIDER_SPEED), TBM_SETPOS, TRUE, 3 + (int)log2f(speed));
}

float GetSpeedSlider(HWND hwnd)
{
	return powf(2.0f, (float)SendMessage(GetDlgItem(hwnd, IDC_SLIDER_SPEED), TBM_GETPOS, 0, 0) - 3);
}

char _iniSaveIntBuf[16];
#define INI_READ_INT(key) GetPrivateProfileIntA("SpeedGear",key,0,".\\sg.ini")
#define INI_SAVE_INT(key,value) wsprintfA(_iniSaveIntBuf,"%d",value);WritePrivateProfileStringA("SpeedGear",key,_iniSaveIntBuf,".\\sg.ini")
#define INI_READ_STR(key,pStr,szStr) GetPrivateProfileStringA("SpeedGear",key,"",pStr,szStr,".\\sg.ini")
#define INI_SAVE_STR(key,pStr) WritePrivateProfileStringA("SpeedGear",key,pStr,".\\sg.ini")

LOGFONTA g_logFont = { 0 };
CHOOSEFONTA g_cf = { sizeof(CHOOSEFONTA),0,0,&g_logFont,0,CF_INITTOLOGFONTSTRUCT | CF_EFFECTS | CF_SCREENFONTS,0 };

void SetButtonFontText(HWND hwnd)
{
	char buf[256];
	wsprintfA(buf, "��������(&T) [%s,%d%s,%d,#%06x]", g_logFont.lfFaceName, g_logFont.lfWeight, g_logFont.lfItalic ? ",��б" : "", FONTHEIGHT_TO_POUND(g_logFont.lfHeight), g_cf.rgbColors);
	if (lstrlenA(g_logFont.lfFaceName) == 0)
		*strchr(buf, ' ') = 0;
	SetDlgItemTextA(hwnd, IDC_BUTTON_STATUS_FONT, buf);
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
	GetEditComboBoxText(GetDlgItem(hwnd, IDC_COMBO_STATUS_FORMAT), pMem->statusFormat, ARRAYSIZE(pMem->statusFormat) - 1);
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
	INI_READ_STR("statusFormat", pMem->statusFormat, ARRAYSIZE(pMem->statusFormat) - 1);
	pMem->statusPosition = INI_READ_INT("statusPosition");
	INI_READ_STR("fontName", pMem->fontName, ARRAYSIZE(pMem->fontName) - 1);
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
	if (SpeedGear_ReleaseSharedMemory() == FALSE)
	{
		MessageBox(hWnd, TEXT("�ͷŹ����ڴ�ʧ�ܡ�"), NULL, MB_ICONERROR);
	}
	return FALSE;
}

BOOL OnCheckTurnOnOff(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return FALSE;
}

BOOL OnSliderSpeed(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	char buf[16];
	sprintf_s(buf, ARRAYSIZE(buf), "%.3fx", GetSpeedSlider(hWnd));
	SetDlgItemTextA(hWnd, IDC_STATIC_SPEED_TEXT, buf);
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
	if (ChooseFontA(&g_cf))
	{
		SetButtonFontText(hWnd);
		GuiSaveMem(hWnd);
		MemSaveIni();
		return TRUE;
	}
	return FALSE;
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
	return TRUE;
}

BOOL OnComboStatusBackground(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	//Update Static
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
		break;
	}
	return TRUE;
}

