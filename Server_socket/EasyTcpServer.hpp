#ifndef _EASYTCPSERVER
#define _EASYTCPSERVER

#ifdef _WIN32
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
#include<vector>
#include"Message.hpp"

#ifndef RECV_BUFF_SIZE
#define RECV_BUFF_SIZE 102400
#endif

class EasyTcpServer {
public:
	EasyTcpServer();
	~EasyTcpServer();

	void InitSocket();	//初始化Socket
	void Close();	//关闭Socket
	int Bind(const char*ip,unsigned short port);	//绑定端口
	int Listen(int n);	//监听端口
	int RecvData(SOCKET _cSock);
	void Accept();	//接受客户端的连接
	bool isRun();	//判断_sock是否在工作中
	void OnRun();	//工作中
	int SendData(SOCKET _cSock,DataHeader* header);	//向客户端发送数据
	virtual void OnNetMsg(DataHeader* header);
private:
	SOCKET _sock;
	std::vector<SOCKET>g_client;
};
//构造函数
EasyTcpServer::EasyTcpServer() {
	_sock = INVALID_SOCKET;
}
//系构函数
EasyTcpServer::~EasyTcpServer() {
	Close();
}
//初始化Socket
void EasyTcpServer::InitSocket() {
#ifdef _WIN32
	WORD var = MAKEWORD(2, 3);
	WSADATA dat;
	WSAStartup(var, &dat);
#endif
	//建立一个socket套接字
	_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
}
//关闭Socket
void EasyTcpServer::Close() {
#ifdef _WIN32
	for (size_t i = 0; i < g_client.size() - 1; ++i) {
		if(g_client[i] != INVALID_SOCKET)
			closesocket(g_client[i]);
	}
	WSACleanup();
#else
	for (size_t i = 0; i < g_client.size() - 1; ++i) {
		if (g_client[i] != INVALID_SOCKET)
			close(g_client[i]);
	}
#endif
	g_client.clear();
}
//绑定端口
int EasyTcpServer::Bind(const char* ip, unsigned short port) {
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4568); //host to net unsigned short
#ifdef _WIN32
	if (ip) {
		_sin.sin_addr.S_un.S_addr = inet_addr(ip);
	}
	else {
		_sin.sin_addr.S_un.S_addr = INADDR_ANY;
	}
#else
	if (ip) {
		_sin.sin_addr.s_addr = inet_addr(ip);
	}
	else {
		_sin.sin_addr.s_addr = INADDR_ANY;
	}
#endif
	int ret = bind(_sock, (sockaddr*)&_sin, sizeof(_sin));
	if (SOCKET_ERROR == ret)
	{
		std::cout << "ERROR: 绑定用于接受客户端失败" << std::endl;
	}
	else {
		std::cout << "绑定用于接受客户端成功" << std::endl;
	}
	return ret;
}
//监听端口
int EasyTcpServer::Listen(int n) {
	//3. listen 监听网络端口
	int ret = listen(_sock, n);
	if (SOCKET_ERROR == ret)
	{
		std::cout << "Socket=<" << _sock << ">";
		std::cout << "ERROR: 监听网络端口失败..." << std::endl;
	}
	else {
		std::cout << "Socket=<" << _sock << ">";
		std::cout << "监听网络端口成功.." << std::endl;
	}
	return ret;
}
//接收数据
int EasyTcpServer::RecvData(SOCKET _cSock) {
	char szBuff[1024];
	int nLen = recv(_cSock, szBuff, sizeof(DataHeader), 0);
	if (nLen < 0) {
		std::cout << "接收客户端<" << _cSock << ">";
		std::cout << "失败..." << std::endl;
		return -1;
	}
	DataHeader* header = (DataHeader*)szBuff;
	//把剩余的消息也接收掉
	recv(_cSock, szBuff + sizeof(DataHeader), header->datalength - sizeof(DataHeader), 0);
	
	LoginResult* logResult = new LoginResult;
	DataHeader* dp = (DataHeader*)logResult;
	SendData(_cSock, dp);

	OnNetMsg(header);
	return 1;
}
//接受客户端的连接
void EasyTcpServer::Accept() {
	sockaddr_in clientAddr = {};
	int nAddrlen = sizeof(sockaddr_in);
	SOCKET _cSock = INVALID_SOCKET;
#ifdef _WIN32
	_cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrlen);
#else
	_cSock = accept(_sock, (sockaddr*)&clientAddr, (socklen_t*)&nAddrlen);
#endif
	if (INVALID_SOCKET == _cSock) {
		std::cout << "ERROR,接收到错误的客户端..." << std::endl;
	}
	else {
		NewUserJoin userjoin;
		DataHeader* header = (DataHeader*)&userjoin;
		for (size_t i = 0; i < g_client.size(); ++i) {
			SendData(g_client[i], header);
		}
		g_client.push_back(_cSock);
		std::cout << "新客户端加入：socket: " << (int)_cSock;
		std::cout << " ,Ip=" << inet_ntoa(clientAddr.sin_addr) << std::endl;
	}
}
//向客户端发送数据
int EasyTcpServer::SendData(SOCKET _cSock, DataHeader* header) {
	if (header) {
		return send(_cSock, (const char*)header, header->datalength, 0);
	}
	return SOCKET_ERROR;
}
//判断_sock是否在工作中
bool EasyTcpServer::isRun()
{
	return _sock != INVALID_SOCKET;
}
void EasyTcpServer::OnRun() {
	if (isRun()) {
		fd_set fdReads;
		FD_ZERO(&fdReads);
		FD_SET(_sock, &fdReads);
		SOCKET max_sock = _sock;
		for (size_t i = 0; i < g_client.size(); ++i) {
#ifdef __linux__
			if (max_sock < g_client[i])
				max_sock = g_client[i];
#endif
			FD_SET(g_client[i], &fdReads);
		}
		//select模式判断接口是否有可读消息
		timeval t = { 0,0 };
		int ret = select(max_sock + 1, &fdReads, 0, 0, &t);
		if (ret < 0) {
			std::cout << "select 任务结束..." << std::endl;
			return;
		}

		if (FD_ISSET(_sock, &fdReads)) {
			FD_CLR(_sock, &fdReads);
			Accept();
		}
		//检查每个端口，删除异常端口
		for (size_t i = 0; i < g_client.size(); ++i) {
			if (FD_ISSET(g_client[i], &fdReads)) {
				if (-1 == RecvData(g_client[i])) {
					FD_CLR(g_client[i], &fdReads);
					auto it = g_client.begin() + i;
					if (it != g_client.end())
						g_client.erase(it);
					break;	//一次只删除一个socket
				}
			}
		}
	}
}
void EasyTcpServer::OnNetMsg(DataHeader* header) {
	switch (header->cmd) {
	case CMD_LOGIN:
	{
		//类内元素存放位置应该是按照类的声明顺序存放在内存中存放
		Login* login = (Login*)header;
		std::cout << "收到login消息...";
		std::cout << login->UserName << "  " << login->PassWord << std::endl;
	}
	break;
	case CMD_LOGOUT:
	{
		Logout* logout = (Logout*)header;
		std::cout << "收到logout消息..." << std::endl;
	}
	break;
	default:
	{
		std::cout << "收到信息不属于任何类别..." << std::endl;
	}
		break;
	}
}
#endif