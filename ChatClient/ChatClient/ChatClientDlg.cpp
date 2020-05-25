
// ChatClientDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ChatClient.h"
#include "ChatClientDlg.h"
#include "afxdialogex.h"
#include "CJsonObject.hpp"
#include <string>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CChatClientDlg 对话框



CChatClientDlg::CChatClientDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_CHATCLIENT_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CChatClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//DDX_Control(pDX, LSTB_CLIENTS, m_NameListBox);
	DDX_Control(pDX, LST_NAME, m_NameListBox);
}

BEGIN_MESSAGE_MAP(CChatClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(BTN_LOGIN, &CChatClientDlg::OnBnClickedLogin)
	ON_BN_CLICKED(BTN_SEND, &CChatClientDlg::OnBnClickedSend)
	ON_WM_CLOSE()
	ON_BN_CLICKED(BTN_LOGOUT, &CChatClientDlg::OnBnClickedLogout)
	ON_WM_TIMER()
	ON_BN_CLICKED(BTN_SENDTO, &CChatClientDlg::OnBnClickedSendto)
	ON_NOTIFY(NM_CLICK, LST_NAME, &CChatClientDlg::OnNMClickName)
END_MESSAGE_MAP()


// CChatClientDlg 消息处理程序


BOOL CChatClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	EnableSendUI(FALSE);
	SetDlgItemInt(EDT_PORT, 9527);

	m_NameListBox.ModifyStyle(LVS_TYPEMASK, //清掉以前的风格 
		LVS_REPORT | LVS_SINGLESEL | LVS_SORTASCENDING); //设置为报表风格

	m_NameListBox.SetExtendedStyle(m_NameListBox.GetExtendedStyle() | LVS_EX_FULLROWSELECT);

	int nItemIdx = 0;
	m_NameListBox.InsertColumn(nItemIdx++, "姓名", LVCFMT_LEFT, 70);

	HANDLE hTread = CreateThread(NULL, 0, RecvThread, this, 0, NULL);
	CloseHandle(hTread);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CChatClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CChatClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CChatClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

DWORD WINAPI CChatClientDlg::RecvThread(LPVOID pParam)
{
	CChatClientDlg* pThis = (CChatClientDlg*)pParam;

	while (TRUE)
	{
		//接受数据包
		DATAPACKAGE package = { 0 };
		if (!pThis->m_sockClient.RecvPackage(&package))
		{
			//接受失败, 不处理数据, 从新继续接受数据
			continue;
		}

		switch (package.m_hdr.m_nDataType)
		{
		case DT_MSG:
		{
			//解析json字符串
			neb::CJsonObject json;
			json.Parse(package.m_pBuff);

			std::string strType;
			json.Get("type", strType);

			//消息
			std::string strMSg;
			json.Get("msg", strMSg);

			//昵称
			std::string strUsername;
			json.Get("name", strUsername);

			//显示数据
			CString csText;
			pThis->GetDlgItemText(EDT_SHOW, csText);

			if (strType == "public")//群发
			{
				//构造成 XXX 说: XXX的格式
				std::string strToShow = strUsername + " 说: " + strMSg;
				csText += "\r\n";
				csText += strToShow.c_str();	
			}
			else if (strType == "private") //私聊 
			{
				std::string strToUsername;
				json.Get("to", strToUsername);
			
				if (!strcmp(strToUsername.c_str(), pThis->m_Name))
				{
					std::string strToShow = strToUsername + " 对你说: " + strMSg;
					csText += "\r\n";
					csText += strToShow.c_str();
				}
				
			}
			pThis->SetDlgItemText(EDT_SHOW, csText);

			break;
		}
		case DT_FLUSH_NAME:
		{
			pThis->m_NameListBox.DeleteAllItems();
			DATAPACKAGE package;
			package.m_hdr.m_nDataLen = pThis->m_Name.GetLength() + 1;
			package.m_hdr.m_nDataType = DT_ADD_NAME;
			package.m_pBuff = pThis->m_Name.GetBuffer(0);
			pThis->m_sockClient.SendPackage(&package);
			break;
		}
		case DT_ADD_NAME:
		{
			pThis->m_NameListBox.InsertItem(0, package.m_pBuff);
			break;
		}
		case DT_DEL_NAME:
		{
			CString szName;
			int nCount = pThis->m_NameListBox.GetItemCount();
			for (int i = 0; i < nCount; i++)
			{
				szName = pThis->m_NameListBox.GetItemText(i, 0);
				if (szName == package.m_pBuff)
				{
					pThis->m_NameListBox.DeleteItem(i);
					break;
				}
			}
			break;
		}

		}
		//释放缓冲区
		if (package.m_pBuff != NULL)
		{
			delete[] package.m_pBuff;
		}
	}
}


void CChatClientDlg::EnableSendUI(BOOL bEnable)
{
	//跟登录相关
	GetDlgItem(EDT_USERNAME)->EnableWindow(!bEnable);
	GetDlgItem(EDT_PORT)->EnableWindow(!bEnable);
	GetDlgItem(BTN_LOGIN)->EnableWindow(!bEnable);

	//跟发送相关的
	GetDlgItem(EDT_SHOW)->EnableWindow(bEnable);
	GetDlgItem(EDT_SEND)->EnableWindow(bEnable);
	GetDlgItem(BTN_LOGOUT)->EnableWindow(bEnable);
	GetDlgItem(BTN_SEND)->EnableWindow(bEnable);
}

void CChatClientDlg::OnBnClickedLogin()
{
	if (!m_sockClient.CreateSocket())
	{
		AfxMessageBox("创建socket失败, 请重试!!");
	}

	//连接服务器
	CString csUsername;
	GetDlgItemText(EDT_USERNAME, csUsername);
	int nPort = GetDlgItemInt(EDT_PORT);
	m_Name = csUsername;

	BOOL bRet = m_sockClient.Connect("127.0.0.1", nPort);
	////向服务器发送登录包
	DATAPACKAGE package;
	package.m_hdr.m_nDataLen = csUsername.GetLength() + 1;
	package.m_hdr.m_nDataType = DT_LOGIN;
	package.m_pBuff = csUsername.GetBuffer(0);
	m_sockClient.SendPackage(&package);
	EnableSendUI(TRUE);

	

}

void CChatClientDlg::OnBnClickedSend()
{
	//向服务器发送数据
	CString csText;
	GetDlgItemText(EDT_SEND, csText);


	CString csUsername;
	GetDlgItemText(EDT_USERNAME, csUsername);
	neb::CJsonObject jsonSend;
	jsonSend.Add("type", "public");
	jsonSend.Add("name", csUsername.GetBuffer(0));
	jsonSend.Add("msg", csText.GetBuffer(0));
	std::string strText = jsonSend.ToString();

	//发送JSON
	DATAPACKAGE package;
	package.m_hdr.m_nDataLen = strText.size() + 1;
	package.m_hdr.m_nDataType = DT_MSG;
	package.m_pBuff = (char*)strText.c_str();
	m_sockClient.SendPackage(&package);

}


void CChatClientDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	DATAPACKAGE package;

	package.m_hdr.m_nDataLen = m_Name.GetLength() + 1;
	package.m_hdr.m_nDataType = DT_DEL_NAME;
	package.m_pBuff = m_Name.GetBuffer(0);
	m_sockClient.SendPackage(&package);

	package.m_hdr.m_nDataLen = 0;
	package.m_hdr.m_nDataType = DT_LOGOUT;
	package.m_pBuff = NULL;
	m_sockClient.SendPackage(&package);

	
	m_sockClient.CloseSocket();

	CDialogEx::OnClose();
}


void CChatClientDlg::OnBnClickedLogout()
{
	// 下线请求
	DATAPACKAGE package;

	package.m_hdr.m_nDataLen = m_Name.GetLength() + 1;
	package.m_hdr.m_nDataType = DT_DEL_NAME;
	package.m_pBuff = m_Name.GetBuffer(0);
	m_sockClient.SendPackage(&package);

	package.m_hdr.m_nDataLen = 0;
	package.m_hdr.m_nDataType = DT_LOGOUT;
	package.m_pBuff = NULL;
	m_sockClient.SendPackage(&package);


	m_sockClient.CloseSocket();
	EnableSendUI(FALSE);

}


void CChatClientDlg::OnTimer(UINT_PTR nIDEvent)
{
	//向服务器发送心跳包, 每隔1秒
	DATAPACKAGE package;
	package.m_hdr.m_nDataLen = 0;
	package.m_hdr.m_nDataType = DT_HEART;
	package.m_pBuff = NULL;
	m_sockClient.SendPackage(&package);

	CDialogEx::OnTimer(nIDEvent);
}


void CChatClientDlg::OnBnClickedSendto()
{
	// TODO: 在此添加控件通知处理程序代码
	CString csText;
	GetDlgItemText(EDT_SEND, csText);

	CString csUsername;
	
	GetDlgItemText(EDT_USERNAME, csUsername);
	m_Name = csUsername;
	
	neb::CJsonObject jsonSend;
	jsonSend.Add("type", "private");
	jsonSend.Add("name", csUsername.GetBuffer(0));
	jsonSend.Add("msg", csText.GetBuffer(0));
	jsonSend.Add("to", m_csToUsername.GetBuffer(0));
	std::string strText = jsonSend.ToString();

	//发送
	DATAPACKAGE package;
	package.m_hdr.m_nDataLen = strText.size() + 1;
	package.m_hdr.m_nDataType = DT_MSG;
	package.m_pBuff = (char*)strText.c_str();
	m_sockClient.SendPackage(&package);
}


void CChatClientDlg::OnNMClickName(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	int nItem = pNMItemActivate->iItem;
	if (nItem != -1)
	{
		m_csToUsername = m_NameListBox.GetItemText(nItem, 0);
	}
	
}
