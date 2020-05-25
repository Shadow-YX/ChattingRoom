// ChatingServer.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include"Server.h"
using namespace std;

int main()
{
	Server  tcpServer;
	tcpServer.CreateServer("127.0.0.1", 9527);
	tcpServer.RunServer();

}

