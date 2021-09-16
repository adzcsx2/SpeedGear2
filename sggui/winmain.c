#include <Windows.h>
#include <WindowsX.h>
#include <tchar.h>
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
		case IDC_CHECK_TURN_ON_OFF:break;
		}
		break;
	case WM_SYSCOMMAND:
		switch (wParam)
		{
		case SC_MINIMIZE:ONCBK(OnMinimize);break;
		case SC_RESTORE:ONCBK(OnRestore);break;
		}
		break;
	}
	return 0;
}

int WINAPI _tWinMain(_In_ HINSTANCE hInst, _In_opt_ HINSTANCE hPrevInst, _In_ LPTSTR param, _In_ int iShow)
{
	return DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG_MAIN), NULL, DlgCallback);
}


//�ص�����ʵ��

BOOL OnInitDialog(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	//����MFC��Դ�ĳ�ʼ��
	OnMFCInitDialog(hWnd, msg, wParam, lParam);

	if (SpeedGear_InitializeSharedMemory(TRUE) == FALSE)
	{
		MessageBox(hWnd, TEXT("���������ڴ�ʧ�ܡ�"), NULL, MB_ICONERROR);
		return FALSE;
	}

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
