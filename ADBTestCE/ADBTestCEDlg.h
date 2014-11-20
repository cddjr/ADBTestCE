// ADBTestCEDlg.h : 头文件
//

//#pragma once
#include "afxwin.h"
#include "resource.h"
#include <winsock2.h>

// CADBTestCEDlg 对话框
class CADBTestCEDlg : public CDialog
{
// 构造
public:
	CADBTestCEDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_ADBTESTCE_DIALOG };


	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
#if defined(_DEVICE_RESOLUTION_AWARE) && !defined(WIN32_PLATFORM_WFSP)
	afx_msg void OnSize(UINT /*nType*/, int /*cx*/, int /*cy*/);
#endif
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();
	CEdit m_displayInfo;
	CString m_logBuffer;
	SOCKET sockClient;
	void logcat(TCHAR* log);
	afx_msg void OnBnClickedButton2();
	HANDLE closeHandle;
};
