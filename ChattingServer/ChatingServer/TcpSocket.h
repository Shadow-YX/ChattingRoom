#pragma once
#include <windows.h>
#include "common.h"

typedef struct tagDataPackage
{
	PACKAGEHEADER m_hdr;	//数据包的包头
	char* m_pBuff;			//数据包的包体
}DATAPACKAGE, *PDATAPACKAGE;


class CTcpSocket
{
public:
	CTcpSocket();
	~CTcpSocket();

	BOOL CreateSocket();
	BOOL BindListen(char* szIp, u_short nPort);
	BOOL Accept(CTcpSocket* pTcpSocket );
	BOOL Connect(char* szIp, u_short nPort);
	BOOL Recv(char* pBuff, int* pLen);//收数据
	BOOL Send(char* pBuff, int* pLen);//发数据


	BOOL RecvPackage(DATAPACKAGE* pPackage);
	BOOL SendPackage(DATAPACKAGE* pPackage);


	void CloseSocket();

	const sockaddr_in& GetSockaddrIn()const;
	SOCKET m_socket;

private:

	sockaddr_in m_si;
};

