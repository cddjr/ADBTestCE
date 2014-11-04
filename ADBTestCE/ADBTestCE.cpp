// ADBTestCE.cpp : 定义应用程序的类行为。
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


// CADBTestCEApp 构造
CADBTestCEApp::CADBTestCEApp()
	: CWinApp()
{
	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}


// 唯一的一个 CADBTestCEApp 对象
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

// CADBTestCEApp 初始化

BOOL CADBTestCEApp::InitInstance()
{

	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}
	RegUsrExceptDmp();
	// 标准初始化
	// 如果未使用这些功能并希望减小
	// 最终可执行文件的大小，则应移除下列
	// 不需要的特定初始化例程
	// 更改用于存储设置的注册表项
	// TODO: 应适当修改该字符串，
	// 例如修改为公司或组织名
	SetRegistryKey(_T("应用程序向导生成的本地应用程序"));

	CADBTestCEDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: 在此处放置处理何时用“确定”来关闭
		//  对话框的代码
	}

	// 由于对话框已关闭，所以将返回 FALSE 以便退出应用程序，
	//  而不是启动应用程序的消息泵。
	return FALSE;
}
