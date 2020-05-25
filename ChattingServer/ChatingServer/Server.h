#pragma once
#include "common.h"
#include "CJsonObject.hpp"
#include "TcpSocket.h"
#include "CLock.h"
#include <time.h>
#include <string>
#include <list>
#include <utility>
#include <algorithm>
#include<iostream>

using namespace std;

class Server
{
public:
	Server();
	~Server();

	BOOL  CreateServer(const char* szIp, u_short nPort);
	BOOL  RunServer();
	CLock g_lock;
private:
	static DWORD WINAPI HandleClientsThread(LPVOID pParam);

private:
	class ClientInfo
	{
	public:
		ClientInfo(clock_t cc,list<ClientInfo*>*pLstClients):  //初始化
			m_clockHeartTime(cc),m_pLstClients(pLstClients)
		{}
		clock_t m_clockHeartTime; //心跳包，上次心跳时间
		list<ClientInfo*>* m_pLstClients;
		CTcpSocket m_tcpsocketClient;
	};

private:
	CTcpSocket m_tcpSocket;
	list<ClientInfo*> m_lstClients;

public:
	bool HandleData(ClientInfo* pCL);

};

