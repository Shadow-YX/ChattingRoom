#include "Server.h"

Server::Server()
{
}

Server::~Server()
{
}

BOOL Server::CreateServer(const char* szIp, u_short nPort)
{
	//1.创建tcp客户端
	BOOL bRet = m_tcpSocket.CreateSocket();
	if (!bRet)
	{
		cout << "tcp客户端创建失败" << endl;
		return FALSE;
	}

	//2.绑定端口
	bRet = m_tcpSocket.BindListen((char*)"127.0.0.1", 9527);
	if (!bRet)
	{
		cout << "绑定端口监听失败" << endl;
		return FALSE;
	}

	//3.创建一个线程，用来接收客户端数据
	HANDLE hThread = CreateThread(NULL, 0, HandleClientsThread, (LPVOID)this, 0, NULL);
	CloseHandle(hThread);

	return TRUE;
}

BOOL Server::RunServer()
{
	//接受来自客户端的数据
	while (TRUE)
	{
		ClientInfo* pCI = new ClientInfo(clock(), &m_lstClients);
		BOOL bRet = m_tcpSocket.Accept(&pCI->m_tcpsocketClient);
		if (!bRet)
		{
			break;
		}
		printf("IP:%s port:%d 连接到服务器. \r\n",
			inet_ntoa(pCI->m_tcpsocketClient.GetSockaddrIn().sin_addr),
			ntohs(pCI->m_tcpsocketClient.GetSockaddrIn().sin_port));

		m_lstClients.push_back(pCI);

	}
	return 0;
}

DWORD WINAPI Server::HandleClientsThread(LPVOID pParam)
{
	Server* pThis = (Server*)pParam;

	while (TRUE)
	{
		//接收数据包
		fd_set fdRead;
		FD_ZERO(&fdRead); //初始化

		//将所有的socket放入数组
		pThis->g_lock.Lock();
		for (auto client : pThis->m_lstClients)
		{
			FD_SET(client->m_tcpsocketClient.m_socket, &fdRead);
		}

		pThis->g_lock.UnLock();

		//检测指定的socket
		timeval tv = { 1, };
		int nRet = select(fdRead.fd_count, &fdRead, NULL, NULL, &tv);
		if (nRet == 0 || nRet == SOCKET_ERROR)
		{
			continue;
		}

		pThis->g_lock.Lock();
		for (auto itr = pThis->m_lstClients.begin(); itr != pThis->m_lstClients.end(); itr++)
		{
			if (FD_ISSET((*itr)->m_tcpsocketClient.m_socket, &fdRead))
			{
				if (!pThis->HandleData(*itr))
				{
					pThis->m_lstClients.erase(itr);
					break;
				}
			}
		}
		pThis->g_lock.UnLock();
	}
	

	return 0;
}

bool Server::HandleData(ClientInfo* pCL)
{
	DATAPACKAGE pkgRecv;
	BOOL bRet = pCL->m_tcpsocketClient.RecvPackage(&pkgRecv);

	if (!bRet)
	{
		return 0;
	}

	//判断数据类型
	switch (pkgRecv.m_hdr.m_nDataType)
	{
	case DT_LOGIN:
	{
		//保存新客户端到链表
		pCL->m_clockHeartTime = clock();

		//向客户端发送登录结果
		DATAPACKAGE package;
		package.m_hdr.m_nDataType = DT_FLUSH_NAME;
		package.m_hdr.m_nDataLen = 0;
		package.m_pBuff = NULL;
		for (auto ci : *(pCL->m_pLstClients))
		{
			BOOL bRet = ci->m_tcpsocketClient.SendPackage(&package);
		}
		break;
	}
	case DT_ADD_NAME:
	{
		for (auto ci : *(pCL->m_pLstClients))
		{
			BOOL bRet = ci->m_tcpsocketClient.SendPackage(&pkgRecv);
		}
		break;
	}
	case DT_DEL_NAME:
	{
		for (auto ci : *(pCL->m_pLstClients))
		{
			BOOL bRet = ci->m_tcpsocketClient.SendPackage(&pkgRecv);
		}
		break;
	}
	case DT_MSG:
	{
		//判断是群发还是私聊
		neb::CJsonObject json;
		json.Parse(pkgRecv.m_pBuff);

		string strType;
		json.Get("type", strType);

		if (strType == "public")//群发
		{
			//将消息转发给其他客户端
			for (auto ci : *(pCL->m_pLstClients))
			{
				BOOL bRet = ci->m_tcpsocketClient.SendPackage(&pkgRecv);
			}
		}
		else if (strType == "private") //私聊 
		{
			//将消息转发给其他客户端
			for (auto ci : *(pCL->m_pLstClients))
			{
				BOOL bRet = ci->m_tcpsocketClient.SendPackage(&pkgRecv);
			}
		}
		break;
	}
	case DT_LOGOUT:
	{
		for (auto itr = pCL->m_pLstClients->begin(); itr != pCL->m_pLstClients-> end(); itr++)
		{
			if ((*itr) == pCL)
			{
				pCL->m_pLstClients->erase(itr);
				delete pCL;

				printf("ip:%s 端口号: %d 下线了 \r\n",
					inet_ntoa(pCL->m_tcpsocketClient.GetSockaddrIn().sin_addr),
					ntohs(pCL->m_tcpsocketClient.GetSockaddrIn().sin_port));
				return false;
			}
		}
		break;
	}
	}

	if (pkgRecv.m_pBuff != NULL)
	{
		delete[] pkgRecv.m_pBuff;
	}
	return TRUE;
}
