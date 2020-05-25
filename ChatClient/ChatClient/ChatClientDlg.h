
// ChatClientDlg.h : 头文件
//

#pragma once
#include "common.h"
#include "TcpSocket.h"

// CChatClientDlg 对话框
class CChatClientDlg : public CDialogEx
{
// 构造
public:
	CChatClientDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CHATCLIENT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;
	CTcpSocket m_sockClient;
	sockaddr_in m_siServer;//服务器地址
	CString m_csToUsername;
	CString m_Name;

	static DWORD WINAPI RecvThread(LPVOID pParam);
	void EnableSendUI(BOOL bEnable);

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedLogin();
	afx_msg void OnBnClickedSend();
	afx_msg void OnClose();
	afx_msg void OnBnClickedLogout();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedSendto();
	CListCtrl m_NameListBox;
	afx_msg void OnNMClickName(NMHDR* pNMHDR, LRESULT* pResult);
};
