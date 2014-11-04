// ADBTestCE.cpp : ����Ӧ�ó��������Ϊ��
//

#include "stdafx.h"
#include "ADBTestCE.h"
#include "ADBTestCEDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CADBTestCEApp

BEGIN_MESSAGE_MAP(CADBTestCEApp, CWinApp)
END_MESSAGE_MAP()


// CADBTestCEApp ����
CADBTestCEApp::CADBTestCEApp()
	: CWinApp()
{
	// TODO: �ڴ˴���ӹ�����룬
	// ��������Ҫ�ĳ�ʼ�������� InitInstance ��
}


// Ψһ��һ�� CADBTestCEApp ����
CADBTestCEApp theApp;


////////////////////////////////////////////////////////////////////////////
/// declarations and definitions for reg usrexceptdmp.exe

#define KMOD_CORE                      ( 1  )
#define IOCTL_KLIB_SETJITDBGRPATH      ( 18 )

extern "C" BOOL WINAPI KernelLibIoControl(HANDLE hLib, DWORD dwIoControlCode, LPVOID lpInBuf, DWORD nInBufSize, LPVOID lpOutBuf, DWORD nOutBufSize, LPDWORD lpBytesReturned);

_inline BOOL SetJITDebuggerPath (LPCWSTR pszDbgrPath)
{
    return KernelLibIoControl((HANDLE) KMOD_CORE, IOCTL_KLIB_SETJITDBGRPATH, (LPVOID) pszDbgrPath, 0, NULL, 0, NULL);
}

inline void RegUsrExceptDmp( void )
{

	OSVERSIONINFO ver_data;

	ZeroMemory(&ver_data, sizeof(OSVERSIONINFO));

	ver_data.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	if( GetVersionEx(&ver_data) )
	{
		WCHAR  szImageName[ 260 ];
		PCWSTR lpszUsrExceptDmpFileName = NULL;

		switch ( ver_data.dwMajorVersion )
		{
		case 0x5:
			lpszUsrExceptDmpFileName = L"UsrExceptDmp.exe";
			break;

		case 0x6:
			lpszUsrExceptDmpFileName = L"UsrExceptDmp_CE6.exe";
			break;

		default:
			return;

		}	

		DWORD length = GetModuleFileNameW( NULL, szImageName, 200 );

		while ( length != 0 &&
				szImageName[ length - 1 ] != '\\' &&
				szImageName[ length - 1 ] != '/' )
		{
			--length;
		}

		if ( length != 0 )
		{
			szImageName[ length ] = '\0';
		}

		wcscat( szImageName, lpszUsrExceptDmpFileName );

		// if UsrExceptDmp not exist, then return.
		WIN32_FIND_DATAW findData;
		HANDLE           hFileFind;

		if ( INVALID_HANDLE_VALUE == 
			 ( hFileFind = FindFirstFileW(szImageName, &findData) ) )
		{
			return;
		}
		else
		{
			FindClose( hFileFind );
		}

		PROCESS_INFORMATION pi;
		
		if ( CreateProcessW( szImageName,
							 L"-RegDebugger",
							 NULL,
							 NULL,
							 FALSE,
							 0,
							 NULL,
							 NULL,
							 NULL,
							 &pi ) )
		{
			SetJITDebuggerPath( szImageName );
		}

	}
}

// CADBTestCEApp ��ʼ��

BOOL CADBTestCEApp::InitInstance()
{

	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}
	RegUsrExceptDmp();
	// ��׼��ʼ��
	// ���δʹ����Щ���ܲ�ϣ����С
	// ���տ�ִ���ļ��Ĵ�С����Ӧ�Ƴ�����
	// ����Ҫ���ض���ʼ������
	// �������ڴ洢���õ�ע�����
	// TODO: Ӧ�ʵ��޸ĸ��ַ�����
	// �����޸�Ϊ��˾����֯��
	SetRegistryKey(_T("Ӧ�ó��������ɵı���Ӧ�ó���"));

	CADBTestCEDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: �ڴ˴����ô����ʱ�á�ȷ�������ر�
		//  �Ի���Ĵ���
	}

	// ���ڶԻ����ѹرգ����Խ����� FALSE �Ա��˳�Ӧ�ó���
	//  ����������Ӧ�ó������Ϣ�á�
	return FALSE;
}
