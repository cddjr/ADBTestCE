// ADBTestCEDlg.cpp : 实现文件
//

#include "stdafx.h"
#include <string>
#include <json\json.h>
#include "ADBTestCE.h"
#include "ADBTestCEDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#include "adb_api.h"
#include "adb\sysdeps.h"

#include "adb\adb.h"
#include "adb\adb_client.h"
#include "adb\file_sync_service.h"
#include "EditLog.h"


#define SPINT_ACTIVE 0x00000001 
#define SPINT_DEFAULT 0x00000002 
#define SPINT_REMOVED 0x00000004

// Android ADB interface identifier
const GUID kAdbInterfaceId = ANDROID_USB_CLASS_ID;

// Number of interfaces detected in TestEnumInterfaces.
int interface_count = 0;

// Constants used to initialize a "handshake" message
#define MAX_PAYLOAD 4096
#define A_SYNC 0x434e5953
#define A_CNXN 0x4e584e43
#define A_OPEN 0x4e45504f
#define A_OKAY 0x59414b4f
#define A_CLSE 0x45534c43
#define A_WRTE 0x45545257
#define A_VERSION 0x01000000

// Formats message sent to USB device
struct message {
    unsigned int command;       /* command identifier constant      */
    unsigned int arg0;          /* first argument                   */
    unsigned int arg1;          /* second argument                  */
    unsigned int data_length;   /* length of payload (0 is allowed) */
    unsigned int data_crc32;    /* crc32 of data payload            */
    unsigned int magic;         /* command ^ 0xffffffff             */
};

// CADBTestCEDlg 对话框

CADBTestCEDlg::CADBTestCEDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CADBTestCEDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CADBTestCEDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_displayInfo);
}

BEGIN_MESSAGE_MAP(CADBTestCEDlg, CDialog)
#if defined(_DEVICE_RESOLUTION_AWARE) && !defined(WIN32_PLATFORM_WFSP)
	ON_WM_SIZE()
#endif
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON1, &CADBTestCEDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CADBTestCEDlg::OnBnClickedButton2)
END_MESSAGE_MAP()


// CADBTestCEDlg 消息处理程序
void startServer() 
{
	adb_main(1, 5037);
}

BOOL CADBTestCEDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标
	setMainDialogPointer(this);
	// TODO: 在此添加额外的初始化代码
	adb_sysdeps_init();
    adb_trace_init();
	HANDLE serverHandle = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)startServer, NULL, NULL,NULL);
	m_displayInfo.SetWindowTextW(L"appstart!");
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

#if defined(_DEVICE_RESOLUTION_AWARE) && !defined(WIN32_PLATFORM_WFSP)
void CADBTestCEDlg::OnSize(UINT /*nType*/, int /*cx*/, int /*cy*/)
{
	if (AfxIsDRAEnabled())
	{
		DRA::RelayoutDialog(
			AfxGetResourceHandle(), 
			this->m_hWnd, 
			DRA::GetDisplayMode() != DRA::Portrait ? 
			MAKEINTRESOURCE(IDD_ADBTESTCE_DIALOG_WIDE) : 
			MAKEINTRESOURCE(IDD_ADBTESTCE_DIALOG));
	}
}
#endif



extern int interface_count;

void forwardTcp()
{
	// TODO: 在此添加控件通知处理程序代码
	 // Test enum interfaces.
	char *argv0 = "forward";
	char *argv1 = "tcp:6666";
	char *argv2 = "tcp:9999";
	char *argv[3];

	argv[0] = argv0;
	argv[1] = argv1;
	argv[2] = argv2;

	adb_commandline(3, (char**)&argv);
}

void CADBTestCEDlg::OnBnClickedButton1()
{
	HANDLE serverHandle = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)forwardTcp, NULL, NULL,NULL);
	return ;
}

void CADBTestCEDlg::logcat(TCHAR* log)
{
	m_logBuffer.Append(log);
	//m_logBuffer = log;
	m_logBuffer.Append(L"\r\n");
	m_displayInfo.SetWindowTextW(m_logBuffer);
}
void receiveFunction(CADBTestCEDlg* dlg);
void processDownload(const char* data, CADBTestCEDlg* dlg) ;
void CADBTestCEDlg::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码
	sockClient=socket(AF_INET,SOCK_STREAM,0);// AF_INET ..tcp连接
	//初始化连接与端口号
	SOCKADDR_IN addrSrv;
	addrSrv.sin_addr.S_un.S_addr=inet_addr("127.0.0.1");//本机地址，服务器在本机开启
	addrSrv.sin_family=AF_INET;
	addrSrv.sin_port=htons(6666);// 设置端口号
	if (connect(sockClient,(SOCKADDR*)&addrSrv,sizeof(SOCKADDR)) != 0) {
		int lastError = WSAGetLastError();
		wchar_t wLog[MAX_PATH];
		_stprintf(wLog, L"connect failed! error = %d", lastError);
		MessageBox(wLog);
	}
	closeHandle = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)receiveFunction, this, NULL,NULL);
}

void receiveFunction(CADBTestCEDlg* dlg) 
{
	const int arrayLength = 4096;
	char recvBuf[arrayLength];
	//while(true) {
		int recvLen = recv(dlg->sockClient,recvBuf,arrayLength,0);
		if (recvLen == SOCKET_ERROR) {
			int lastError = WSAGetLastError();
			wchar_t wLog[MAX_PATH];
			_stprintf(wLog, L"connect failed! error = %d", lastError);
			MessageBox(NULL, wLog, NULL, MB_OK);
		}
		recvBuf[recvLen] = '\0';
		wchar_t wLog[512];
		MultiByteToWideChar(GetACP(), 0, recvBuf, -1, wLog, 512);
		logcat(wLog);
		//processDownload(recvBuf, dlg);
	//}
		//std::string success_str("transfaner ok");
		//send(dlg->sockClient, success_str.c_str(), success_str.length(), 0);
		char testBuf[4096];
		memset(testBuf, 1, 4096);
		send(dlg->sockClient, testBuf, 4096, 0);
	closesocket(dlg->sockClient);

		//dlg->displayInfoBuffer.Append(L"全部下载完成!");
		//dlg->displayInfoBuffer.Append(L"\r\n");
		//dlg->displayInfo.SetWindowTextW(dlg->displayInfoBuffer);
	/*
	printf("%s\n",recvBuf);
	TCHAR wLog[MAX_PATH];
	MultiByteToWideChar(CP_ACP, 0, recvBuf, recvLen, wLog, MAX_PATH);
	displayInfoBuffer.Append(wLog);
	displayInfoBuffer.Append(L"\r\n");
	displayInfo.SetWindowTextW(displayInfoBuffer);
	send(sockClient,"hello",strlen("hello")+1,0);//发送数据
	closesocket(sockClient);//关闭连接
	WSACleanup();
	*/
	
}

void processDownload(const char* data, CADBTestCEDlg* dlg) 
{
	Json::Reader reader;
	Json::Value json_object;
	if (!reader.parse(data, json_object)) {
		MessageBox(NULL, L"json parse error!", NULL, MB_OK);
	}
	Json::Value file_list;
	file_list = json_object["filelist"];
	int file_count = file_list.size();
	
	std::vector<std::string> file_list_container;

	for (int i = 0; i < file_count; ++i) {
		std::string file_name = file_list[i]["name"].asString();
		file_list_container.push_back(file_name);
		//if (!downloadFile(file_name, dlg)) {
			return;
		//}
	}
	/*
	for (auto fileName : file_list_container) {

	}
	*/
}