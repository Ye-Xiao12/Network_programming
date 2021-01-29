#ifndef _EASY_CONNECT_Client
#define _EASY_CONNECT_Client

#ifdef _WIN32   //windwows操作系统下
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS 
#include<windows.h>
#include<WinSock2.h>
#else
#include<unistd.h>  //unix std
#include<arpa/inet.h>
#define SOCKET int
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)
#endif
#include<iostream>
#include"Message.hpp"

class EasyTcpClient
{
public:
	EasyTcpClient();
	virtual ~EasyTcpClient();

	void InitSocket();	//初始化_sock
	int Connect(const char* ip, unsigned short port);	//绑定端口号，连接服务器
	void Close();	//关闭套接字_socket

	bool OnRun();	//_sock工作流程
	bool IsRun();	//判断_sock是否占用
	int RecvData(SOCKET _sock);	//收数据
	void OnNetMsg(DataHeader* header);	//根据收到的报头，客户端做出反应
	int SendData(DataHeader* header);	//回复消息
private:
	SOCKET _sock;
	char szRecv[4096];
};
//构造函数
EasyTcpClient::EasyTcpClient() {
	_sock = INVALID_SOCKET;
}
//析构函数
EasyTcpClient::~EasyTcpClient() {
	Close();
}

//初始化连接
void EasyTcpClient::InitSocket() {
#ifdef _WIN32
	WORD var = MAKEWORD(2, 3);
	WSADATA dat;
	WSAStartup(var, &dat);
#endif
	//如果_sock没释放，则释放旧连接
	if (INVALID_SOCKET != _sock) {
		std::cout << "<Socket=" << _sock << ">   关闭旧连接..." << std::endl;
		_sock = INVALID_SOCKET;
	}
	_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == _sock) {
		std::cout << "Error! 创建套接字失败" << std::endl;
	}
	else {
		std::cout << "成功创建Socket..." << std::endl;
	}
}

//连接socket，输入ip和port
int EasyTcpClient::Connect(const char* ip, unsigned short port) {
	//绑定端口号(服务器端口)
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(port);
#ifdef _WIN32
	_sin.sin_addr.S_un.S_addr = inet_addr(ip);
#else
	_sin.sin_addr.s_addr = inet_addr(ip);
#endif
	int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr));
	if (SOCKET_ERROR == ret) {
		std::cout << "连接服务器失败..." << std::endl;
		return -1;
	}
	else {
		std::cout << "连接服务器成功..." << std::endl;
	}
	return 1;
}

//关闭连接
void EasyTcpClient::Close() {
	if (INVALID_SOCKET != _sock) {
#ifdef _WIN32
		closesocket(_sock);
		WSACleanup();
#else
		close(_sock);
#endif
	}
}
//客户端工作流程
bool EasyTcpClient::OnRun() {
	if (IsRun()) {
		fd_set fdReads;
		FD_ZERO(&fdReads);
		FD_SET(_sock, &fdReads);
		timeval t = { 1,0 };
		int ret = select(_sock + 1, &fdReads, 0, 0, &t);
		if (ret < 0) {
			std::cout << "<socket=" << _sock << ">select任务结束" << std::endl;
			return false;
		}

		if (FD_ISSET(_sock, &fdReads)) {
			FD_CLR(_sock, &fdReads);
			if (-1 == RecvData(_sock)) {
				std::cout << "<socket=" << _sock << ">select任务结束" << std::endl;
				return false;
			}
		}
		return true;
	}
	return false;
}
//判断_sock是否占用
bool EasyTcpClient::IsRun() {
	return INVALID_SOCKET != _sock;
}
//收数据
int EasyTcpClient::RecvData(SOCKET _sock) {
	int nLen = recv(_sock, szRecv, sizeof(DataHeader), 0);
	DataHeader* dh = (DataHeader*)szRecv;
	if (nLen <= 0) {
		std::cout << "与服务器断开连接，任务结束" << std::endl;
		return -1;
	}
	recv(_sock, szRecv + dh->datalength, dh->datalength - sizeof(DataHeader), 0);
	OnNetMsg(dh);
	return 0;
}
//根据收到的报头，客户端做出反应
void EasyTcpClient::OnNetMsg(DataHeader* header) {
	switch (header->cmd) {
	case CMD_LOGIN_RESULT:
	{
		LoginResult* login = (LoginResult*)szRecv;
		std::cout << "接收到服务端信息: LoginResult" << std::endl;
	}
	break;
	case CMD_LOGOUT_RESULT:
	{
		LogoutResult* logout = (LogoutResult*)szRecv;
		std::cout << "接收到服务端信息: LogoutResult" << std::endl;
	}
	break;
	case CMD_NEW_USER_JOIN:
	{
		NewUserJoin* userJoin = (NewUserJoin*)szRecv;
		std::cout << "新加入一个玩家..." << std::endl;
	}
	break;
	}
}
//回复消息
int EasyTcpClient::SendData(DataHeader* header) {
	if (IsRun() && header) {
		return send(_sock, (const char*)header, header->datalength, 0);
	}
	return SOCKET_ERROR;
}
#endif