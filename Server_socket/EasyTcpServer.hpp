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
#include<chrono>
#include<vector>
#include<thread>
#include<mutex>
#include<functional>
#include"Message.hpp"
#include"CELLTimestamp.hpp"

#ifndef RECV_BUFF_SIZE	//	select模型最大可处理连接数
#define RECV_BUFF_SIZE 10240
#endif
//客户socket类
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
	SOCKET _sockfd;	//客户端socket编号
	char _szMsgBuf[RECV_BUFF_SIZE * 10] = {};	//客户端缓冲区
	size_t _lastPtr;	//指针，指向客户端缓冲区末尾
};
//CELLserver类中调用EasyTcpServer成员函数的接口
class INetEvent {
public:
	//纯虚函数，留待后续派生类定义
	virtual void OnLeave(ClientSocket* pClient) = 0;
private:
};
//完成服务器端对数据处理的部分服务
class CellServer {
public:
	CellServer(SOCKET sock = INVALID_SOCKET);
	~CellServer();

	void setEvent(INetEvent* event);	//初始化指针_pNetEvent
	void Close();	//关闭所有客户端连接
	int getMsgCount();	//获取处理报文数量，并将其置为0
	int getClientCount();	//获取该线程客户端数量
	virtual void OnNetMsg(SOCKET cSock, DataHeader* header);	//对网络数据的响应
	int RecvData(ClientSocket* pClient);
	void addClient(ClientSocket* pClient);	//添加客户端：生产者
	bool isRun();	//判断服务器sock是否正在工作中
	bool onRun();	//主工作成员函数：消费者
	void Start();	//用成员函数创建线程
private:
	SOCKET _sock;	//服务端socket
	bool _client_Change = false;	//标志位，如果客户端数量出现改变，则置为true
	fd_set _fdRead_Back;	//fdread的备份，如果客户端没有改变，直接拷贝给fd_set
	char _szRecv[RECV_BUFF_SIZE] = {};	//消息缓冲区
	std::vector<ClientSocket*>_clients;	//需要处理的客户端队列
	std::vector<ClientSocket*>_clientsBuff;	//连接的客户端缓存区,有不止1个线程对这个资源操作，因此需要加锁
	std::mutex _mutex;	//用于客户端资源临界区的锁
	std::atomic_int _recvCount = 0;	//处理报文数
	INetEvent* _pNetEvent;	//使用纯虚函数，调用EasyTcpServer中的OnNetMsg(),OnLeave()成员函数，达到计数的目的
	std::thread* _pThread;	//指向线程指针
};
//声明是可定义默认参数，定义函数时不需要重新定义默认参数
CellServer::CellServer(SOCKET sock ) {
	_sock = sock;
	_pThread = nullptr;
	_recvCount = 0;
	_pNetEvent = nullptr;
}
CellServer::~CellServer() {
	Close();
	_sock = INVALID_SOCKET;
}
//初始化指针_pNetEvent
void CellServer::setEvent(INetEvent* event) {
	_pNetEvent = event;
}
//关闭所有客户端连接
void CellServer::Close() {
	if (_sock != INVALID_SOCKET) {
#ifdef _WIN32
		for (size_t i = 0; i < _clients.size() - 1; ++i) {
			if (_clients[i]->sockfd() != INVALID_SOCKET) {
				closesocket(_clients[i]->sockfd());
				delete _clients[i];
			}
		}
		WSACleanup();
#else
		for (size_t i = 0; i < _clients.size() - 1; ++i) {
			if (_clients[i]->sockfd() != INVALID_SOCKET) {
				close(_clients[i]->sockfd());
				delete _clients[i];
			}
		}
#endif
		_sock = INVALID_SOCKET;
		_clients.clear();
	}
}
//获取处理报文数量，并将其置为0
int CellServer::getMsgCount() {
	int num = _recvCount;
	_recvCount = 0;
	return num;
}
//获取该线程客户端数量
int CellServer::getClientCount() {
	size_t num = _clients.size() + _clientsBuff.size();
	return (int)num;
}
//对网络数据的响应
void CellServer::OnNetMsg(SOCKET cSock, DataHeader* header) {
	_recvCount++;	//原子操作
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
//接收数据 处理粘包 拆分包
int CellServer::RecvData(ClientSocket* pClient) {
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
			OnNetMsg(pClient->sockfd(),header);
			memcpy(pClient->msgBuf(), pClient->msgBuf() + header->datalength, nSize);
			pClient->setLastPtr(nSize);
		}
		else {	//消息缓存区不够发送一条完整消息
			break;
		}
	}
	return 1;
}
//将新连接的客户端添加到客户端最少的线程中
void CellServer::addClient(ClientSocket* pClient) {
	std::lock_guard<std::mutex>lock(_mutex);	//临界资源保护
	_clientsBuff.push_back(pClient);
}
//判断服务器sock是否正在工作中
bool CellServer::isRun() {
	return _sock != INVALID_SOCKET;
}
//主工作成员函数
bool CellServer::onRun() {
	while (isRun()) {
		if (!_clientsBuff.empty()) {
			//将缓存区的客户端加载到当前任务客户端
			std::lock_guard<std::mutex>lock(_mutex);
			for (auto pClient : _clientsBuff) {
				_clients.push_back(pClient);
			}
			_clientsBuff.clear();
			_client_Change = true;
		}
		//若待处理客户端队列为空，线程阻塞1ms
		if (_clients.empty()) {
			std::chrono::milliseconds t(1);
			std::this_thread::sleep_for(t);
			continue;
		}

		fd_set fdReads;
		FD_ZERO(&fdReads);
		SOCKET max_sock = (_clients.empty()) ? 0 : _clients[0]->sockfd();
		if (_client_Change) {
			_client_Change = false;
			for (size_t i = 0; i < _clients.size(); ++i) {
#ifdef __linux__
				if (max_sock < _clients[i]->sockfd())
					max_sock = _clients[i]->sockfd();
#endif
				FD_SET(_clients[i]->sockfd(), &fdReads);
			}
			memcpy(&_fdRead_Back, &fdReads, sizeof(fd_set));
		}
		else {
			memcpy(&fdReads, &_fdRead_Back, sizeof(fd_set));
		}
		//select模式判断接口是否有可读消息
		timeval t = { 0,0 };
		int ret = select(max_sock + 1, &fdReads, 0, 0, &t);
		if (ret < 0) {
			printf("select 任务结束...");
			return false;
		}
		else if(ret == 0) {
			continue;
		}

		//检查每个端口，删除异常端口
		for (size_t i = 0; i < _clients.size(); ++i) {
			if (FD_ISSET(_clients[i]->sockfd(), &fdReads)) {
				FD_CLR(_clients[i]->sockfd(), &fdReads);
				if (-1 == RecvData(_clients[i])) {
					auto it = _clients.begin() + i;
					if (it != _clients.end()) {
						delete _clients[i];
						if (_pNetEvent != nullptr) {
							_pNetEvent->OnLeave(_clients[i]);
						}
						_clients.erase(it);
					}
					break;	//一次只删除一个socket
				}
			}
		}
	}
	return false;
}
//用成员函数创建线程
void CellServer::Start() {
	//_pThread = new std::thread(&CellServer::onRun, this);
	//将成员函数转化为函数对象，使用对象指针或对象引用进行绑定
	_pThread = new std::thread(std::mem_fn(&CellServer::onRun), this);
	_pThread->detach();
}

class EasyTcpServer: public INetEvent {
public:
	EasyTcpServer();
	~EasyTcpServer();

	void InitSocket();	//初始化Socket
	void Close();	//关闭Socket
	int Bind(const char*ip,unsigned short port);	//绑定端口
	int Listen(int n);	//监听端口
	void Start(int nCellServer);	//启动n个处理报文服务的线程
	SOCKET Accept();	//接受客户端的连接
	void addClientToCellSever(ClientSocket* pClient);	//为处理线程增加客户端
	bool isRun();	//判断_sock是否在工作中
	bool OnRun();	//工作中
	int SendData(SOCKET _cSock,DataHeader* header);	//向客户端发送数据
	void SendDataToAll(DataHeader* header);	//向当前连接的所有客户端发送数据
	void timePerMsg();	//计算每秒处理报文数量
	void OnLeave(ClientSocket* pClient);	//删除指定客户端
private:
	SOCKET _sock;
	std::vector<ClientSocket*>_clients;	//存储目前所有连接客户端
	std::vector<CellServer*>_cellServer;
	std::mutex _mutex;
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
	if (WSAStartup(var, &dat) != 0) {
		printf("ERROR: Open WSAStartup!\n");
	}
#endif
	//建立一个socket套接字
	_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
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
//启动n个处理报文服务的线程
void EasyTcpServer::Start(int nCellServer) {
	for (int i = 0; i < nCellServer; ++i) {
		auto ser = new CellServer(_sock);
		_cellServer.push_back(ser);
		ser->setEvent(this);
		ser->Start();
	}
}
//关闭Socket
void EasyTcpServer::Close() {
	if (_sock != INVALID_SOCKET) {
		for (auto ser : _cellServer) {
			ser->Close();
		}
#ifdef _WIN32
		closesocket(_sock);
		WSACleanup();
#else
		close(_sock);
#endif
	}
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
		printf("ERROR,接收到错误的客户端...\n");
	}
	else {
		//NewUserJoin userjoin;
		//SendDataToAll(&userjoin);
		auto pClient = new ClientSocket(cSock);
		_clients.push_back(pClient);
		addClientToCellSever(pClient);
		//std::cout << "新客户端加入：socket: " << (int)cSock;
		//std::cout << " ,Ip=" << inet_ntoa(clientAddr.sin_addr) << std::endl;
	}
	return cSock;
}
//为处理线程增加客户端
void EasyTcpServer::addClientToCellSever(ClientSocket* pClient) {
	auto minSizeCell = _cellServer[0];
	//查找处理客户端最少的线程，将新加入的客户端添加到该线程
	for (auto server : _cellServer) {
		if (server->getClientCount() < minSizeCell->getClientCount()) {
			minSizeCell = server;
		}
	}
	minSizeCell->addClient(pClient);
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
	for (auto client : _clients) {
		send(client->sockfd(), (const char*)header, header->datalength, 0);
	}
}
//判断_sock是否在工作中
bool EasyTcpServer::isRun()
{
	return _sock != INVALID_SOCKET;
}
bool EasyTcpServer::OnRun() {
	if (isRun()) {
		//打印每秒处理报文数量
		timePerMsg();

		fd_set fdReads;
		FD_ZERO(&fdReads);
		FD_SET(_sock, &fdReads);
		SOCKET max_sock = _sock;
		//select模式判断接口是否有可读消息
		timeval t = { 0,10 };
		int ret = select(max_sock + 1, &fdReads, 0, 0, &t);
		if (ret < 0) {
			printf("select 任务结束...\n");
			return false;
		}
		if (FD_ISSET(_sock, &fdReads)) {
			FD_CLR(_sock, &fdReads);
			Accept();
		}
		return true;
	}
	return false;
}
//计算每秒处理报文数量
void EasyTcpServer::timePerMsg() {
	auto t = _Time.getElapsedSecond();
	if (t >= 1.0) {
		int numMsg = 0;
		int numClient = 0;
		for (auto cell : _cellServer) {
			numMsg += cell->getMsgCount();
			numClient += cell->getClientCount();
		}
		printf("Time:<%lf>; thread:<%d>; clients:<%d>; Message:<%d>\n",t,(int)_cellServer.size(),numClient,numMsg);
		_Time.update();
	}
}
//删除指定客户端
void EasyTcpServer::OnLeave(ClientSocket* pClient) {
	std::lock_guard<std::mutex>lock(_mutex);
	auto it = find(_clients.begin(), _clients.end(), pClient);
	_clients.erase(it);
}
#endif