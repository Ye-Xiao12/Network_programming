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
#include"CELLTimestamp.hpp"

#ifndef RECV_BUFF_SIZE
#define RECV_BUFF_SIZE 10240
#endif

class ClientSocket {
public:
	//构造函数
	ClientSocket(SOCKET sockfd = INVALID_SOCKET) {
		_sockfd = sockfd;
		memset(_szMsgBuf, 0, sizeof(_szMsgBuf));
		_lastPtr = 0;
	}
	//返回该客户端socket文件标识符
	SOCKET sockfd() {
		return _sockfd;
	}
	//返回该socket缓冲区文件
	char* msgBuf() {
		return _szMsgBuf;
	}
	//返回缓冲区指针
	size_t getLastPtr() {
		return _lastPtr;
	}
	//缓冲区指针重制为pos
	void setLastPtr(size_t pos) {
		_lastPtr = pos;
	}
private:
	SOCKET _sockfd;
	char _szMsgBuf[RECV_BUFF_SIZE * 10] = {};
	size_t _lastPtr;
};

class EasyTcpServer {
public:
	EasyTcpServer();
	~EasyTcpServer();

	void InitSocket();	//初始化Socket
	void Close();	//关闭Socket
	int Bind(const char*ip,unsigned short port);	//绑定端口
	int Listen(int n);	//监听端口
	int RecvData(ClientSocket* pClient);
	SOCKET Accept();	//接受客户端的连接
	bool isRun();	//判断_sock是否在工作中
	bool OnRun();	//工作中
	int SendData(SOCKET _cSock,DataHeader* header);	//向客户端发送数据
	void SendDataToAll(DataHeader* header);	//向当前连接的所有客户端发送数据
	virtual void OnNetMsg(DataHeader* header);
private:
	SOCKET _sock;
	char _szRecv[RECV_BUFF_SIZE] = {};
	std::vector<ClientSocket *>_gClient;
	int _numMessage;	//服务器处理报文条数
	CELLTimestamp _Time;	//计算时间的类
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
	for (size_t i = 0; i < _gClient.size() - 1; ++i) {
		if (_gClient[i]->sockfd() != INVALID_SOCKET) {
			closesocket(_gClient[i]->sockfd());
			delete _gClient[i];
		}
	}
	WSACleanup();
#else
	for (size_t i = 0; i < _gClient.size() - 1; ++i) {
		if (_gClient[i]->sockfd() != INVALID_SOCKET) {
			close(_gClient[i]->sockfd());
			delete _gClient[i];
		}
	}
#endif
	_gClient.clear();
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
int EasyTcpServer::RecvData(ClientSocket *pClient) {
	int nLen = recv(pClient->sockfd(), _szRecv, RECV_BUFF_SIZE, 0);
	if (nLen <= 0) {
		std::cout << "接收客户端<" << pClient->sockfd() << ">";
		std::cout << "失败..." << std::endl;
		return -1;
	}
	//将收到的数据拷贝到消息缓冲区
	memcpy(pClient->msgBuf() + pClient->getLastPtr(), _szRecv, nLen);
	pClient->setLastPtr(pClient->getLastPtr() + nLen);
	
	while (pClient->getLastPtr() >= sizeof(DataHeader)) {
		DataHeader* header = (DataHeader*)pClient->msgBuf();
		if (pClient->getLastPtr() >= header->datalength) {
			int nSize = pClient->getLastPtr() - header->datalength;
			OnNetMsg(header);
			memcpy(pClient->msgBuf(), pClient->msgBuf() + header->datalength, nSize);
			pClient->setLastPtr(nSize);
		}
		else {	//消息缓存区不够发送一条完整消息
			break;
		}
	}
	return 1;
}
//接受客户端的连接
SOCKET EasyTcpServer::Accept() {
	sockaddr_in clientAddr = {};
	int nAddrlen = sizeof(sockaddr_in);
	SOCKET cSock = INVALID_SOCKET;
#ifdef _WIN32
	cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrlen);
#else
	cSock = accept(_sock, (sockaddr*)&clientAddr, (socklen_t*)&nAddrlen);
#endif
	if (INVALID_SOCKET == cSock) {
		std::cout << "ERROR,接收到错误的客户端..." << std::endl;
	}
	else {
		//NewUserJoin userjoin;
		//SendDataToAll(&userjoin);
		_gClient.push_back(new ClientSocket(cSock));
		//std::cout << "新客户端加入：socket: " << (int)cSock;
		//std::cout << " ,Ip=" << inet_ntoa(clientAddr.sin_addr) << std::endl;
	}
	return cSock;
}
//向客户端发送数据
int EasyTcpServer::SendData(SOCKET _cSock, DataHeader* header) {
	if (header) {
		return send(_cSock, (const char*)header, header->datalength, 0);
	}
	return SOCKET_ERROR;
}
//向当前连接的所有客户端发送数据
void EasyTcpServer::SendDataToAll(DataHeader* header) {
	for (size_t i = 0; i < _gClient.size(); ++i) {
		SendData(_gClient[i]->sockfd(), header);
	}
}
//判断_sock是否在工作中
bool EasyTcpServer::isRun()
{
	return _sock != INVALID_SOCKET;
}
bool EasyTcpServer::OnRun() {
	if (isRun()) {
		fd_set fdReads;
		FD_ZERO(&fdReads);
		FD_SET(_sock, &fdReads);
		SOCKET max_sock = _sock;
		for (size_t i = 0; i < _gClient.size(); ++i) {
#ifdef __linux__
			if (max_sock < _gClient[i]->sockfd())
				max_sock = _gClient[i]->sockfd();
#endif
			FD_SET(_gClient[i]->sockfd(), &fdReads);
		}
		//select模式判断接口是否有可读消息
		timeval t = { 0,0 };
		int ret = select(max_sock + 1, &fdReads, 0, 0, &t);
		if (ret < 0) {
			std::cout << "select 任务结束..." << std::endl;
			return false;
		}

		if (FD_ISSET(_sock, &fdReads)) {
			FD_CLR(_sock, &fdReads);
			Accept();
		}
		//检查每个端口，删除异常端口
		for (size_t i = 0; i < _gClient.size(); ++i) {
			if (FD_ISSET(_gClient[i]->sockfd(), &fdReads)) {
				FD_CLR(_gClient[i]->sockfd(), &fdReads);
				if (-1 == RecvData(_gClient[i])) {
					auto it = _gClient.begin() + i;
					if (it != _gClient.end()) {
						delete _gClient[i];
						_gClient.erase(it);
					}
					break;	//一次只删除一个socket
				}
			}
		}
		return true;
	}
	return false;
}
void EasyTcpServer::OnNetMsg(DataHeader* header) {
	//计算服务器每秒接收报文数量
	++_numMessage;
	double t1 = _Time.getElapsedSecond();
	if (t1 >= 1.0) {
		printf("time:<%lf>; socket<%d>", t1,_sock);
		std::cout << " Client Size:<" << _gClient.size() << ">" << " recvMessage:<" << _numMessage << ">" << std::endl;
		_numMessage = 0;
		_Time.update();
	}

	switch (header->cmd) {
	case CMD_LOGIN:
	{
		//类内元素存放位置应该是按照类的声明顺序存放在内存中存放
		Login* login = (Login*)header;
		/*std::cout << "收到login消息...";
		std::cout << login->UserName << "  " << login->PassWord << std::endl;*/
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