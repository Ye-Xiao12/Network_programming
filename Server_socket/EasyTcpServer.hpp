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
#include"CELLTask.hpp"

#ifndef RECV_BUFF_SIZE	//	接受缓存区大小
#define RECV_BUFF_SIZE 10240
#endif
#define SEND_BUFF_SIZE RECV_BUFF_SIZE
class ClientSocket;
class INetEvent;
class CellServer;
class EasyTcpServer;

//客户socket类
class ClientSocket {
public:
	//构造函数
	ClientSocket(SOCKET sockfd = INVALID_SOCKET) {
		_sockfd = sockfd;
		//用来对申请来的内存块做初始化操作
		//为什么没有初始化就不能用memcpy()函数操作内存？？？？
		//这一步是关键操作！！！
		memset(_szMsgBuf, 0, sizeof(_szMsgBuf));
		memset(_szSendBuf, 0, sizeof(_szSendBuf));
		_lastPtr = 0;
		_lastSendPtr = 0;	//我擦，原来这一步才是关键操作！！！！
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
	//发送数据
	int sendData(DataHeader* header) {
		int ret = SOCKET_ERROR;
		//要发送长度
		int nSendLen = header->datalength;
		//要发送数据
		const char* pSendData = (const char*)header;
		while (true) {
			if (_lastSendPtr + nSendLen >= SEND_BUFF_SIZE) {
				int nCopyLen = SEND_BUFF_SIZE - _lastSendPtr;
				memcpy(_szSendBuf + _lastSendPtr, pSendData, (size_t)nCopyLen);
				//计算剩余数据
				pSendData += nCopyLen;
				//计算剩余数据长度
				nSendLen -= nCopyLen;
				ret = send(_sockfd, _szSendBuf, SEND_BUFF_SIZE, 0);
				_lastSendPtr = 0;
				if (SOCKET_ERROR == ret) {
					return ret;
				}
			}
			else {
				memcpy(_szSendBuf + _lastSendPtr, pSendData , nSendLen);
				_lastSendPtr += nSendLen;
				break;
			}
		}
		return ret;
	}
private:
	SOCKET _sockfd;	//客户端socket编号
	char _szMsgBuf[RECV_BUFF_SIZE] = {};	//客户端接受缓冲区
	char _szSendBuf[SEND_BUFF_SIZE] = {};	//客户端发送缓冲区
	size_t _lastPtr;	//指针，指向客户端缓冲区末尾
	size_t _lastSendPtr;	//指针，指向客户端发送缓冲区的末尾
};
//CELLserver类中调用EasyTcpServer成员函数的接口
class INetEvent {
public:
	//纯虚函数，留待后续派生类定义
	virtual void OnNetLeave(ClientSocket* pClient) = 0;	//减少一个连接
	virtual void OnNetJoin(ClientSocket* pClient) = 0;	//增加一个连接
	virtual void OnNetRecv(ClientSocket* pClient) = 0; //接收数据次数
	virtual void OnNetMsg(CellServer* pCellServer, ClientSocket* pClient, DataHeader* header) = 0;	//处理数据次数
private:
};

//网络消息发送任务
class CellSendMsg2ClientTask :public CellTask
{
private:
	ClientSocket* _pClient;
	DataHeader* _pHeader;
public:
	CellSendMsg2ClientTask(ClientSocket* pClient, DataHeader* header)
	{
		_pClient = pClient;
		_pHeader = header;
	}
	//执行任务
	void doTask()
	{
		_pClient->sendData(_pHeader);
		delete _pHeader;
	}
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
	void addSendTask(ClientSocket* pClient, DataHeader* header);	//添加任务
	virtual void OnNetMsg(ClientSocket* pClient, DataHeader* header);	//对网络数据的响应
	int RecvData(ClientSocket* pClient);
	void addClient(ClientSocket* pClient);	//添加客户端：生产者
	bool isRun();	//判断服务器sock是否正在工作中
	bool onRun();	//主工作成员函数：消费者
	void Start();	//用成员函数创建线程
	int sendData(SOCKET cSock, DataHeader* header);	//用于发送数据
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
	CellTaskServer _taskServer;	//处理发送任务线程
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
void CellServer::addSendTask(ClientSocket* pClient, DataHeader* header)
{
	CellSendMsg2ClientTask* task = new CellSendMsg2ClientTask(pClient, header);
	_taskServer.addTask(task);
}
//对网络数据的响应
void CellServer::OnNetMsg(ClientSocket* pClient, DataHeader* header) {
	_pNetEvent->OnNetMsg(this,pClient, header);
}
//接收数据 处理粘包 拆分包
int CellServer::RecvData(ClientSocket* pClient) {
	_pNetEvent->OnNetRecv(pClient);
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
			OnNetMsg(pClient,header);
			memcpy(pClient->msgBuf(), pClient->msgBuf() + header->datalength, nSize);
			pClient->setLastPtr(nSize);
		}
		else {	//消息缓存区不够发送一条完整消息
			break;
		}
	}
	return 1;
}
//用于发送数据
int CellServer::sendData(SOCKET cSock, DataHeader* header) {
	if (header) {
		return send(cSock, (const char*)header, header->datalength, 0);
	}
	return SOCKET_ERROR;
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
		timeval t = { 0,1 };
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
							_pNetEvent->OnNetLeave(_clients[i]);
						}
						_pNetEvent->OnNetLeave(_clients[i]);
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
	_taskServer.Start();
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
	void OnNetLeave(ClientSocket* pClient);	//减少一个连接
	void OnNetJoin(ClientSocket* pClient);	//增加一个连接
	void OnNetRecv(ClientSocket* pClient); //接收数据次数
	void OnNetMsg(CellServer* pCellServer, ClientSocket* pClient, DataHeader* header);	//处理一次数据
private:
	SOCKET _sock;
	std::vector<CellServer*>_cellServer;
	CELLTimestamp _Time;	//计算时间的类
protected:
	std::mutex _mutex;
	std::vector<ClientSocket*>_clients;	//存储目前所有连接客户端
	std::atomic_int _msgCount;	//接收数据次数
	std::atomic_int _recvCount;	//收到的消息总数
	std::atomic_int _clientCount;	//客户端数量
};
//构造函数
EasyTcpServer::EasyTcpServer() {
	_sock = INVALID_SOCKET;
	_msgCount = 0;
	_recvCount = 0;
	_clientCount = 0;
}
//系构函数
EasyTcpServer::~EasyTcpServer() {
	Close();
	for (auto server : _cellServer) {
		delete server;
	}
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
		OnNetJoin(pClient);
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
		printf("Time:<%lf>; thread:<%d>; clients:<%d>; recv<%d>; Message:<%d>\n",t,(int)_cellServer.size(), _clientCount.load(), (int)(_recvCount.load() / t),(int)(_msgCount.load() / t));
		_Time.update();
		_recvCount = 0;
		_msgCount = 0;
	}
}
//减少一个连接
void EasyTcpServer::OnNetLeave(ClientSocket* pClient) {
	std::lock_guard<std::mutex>lock(_mutex);
	auto it = find(_clients.begin(), _clients.end(), pClient);
	_clients.erase(it);
	_clientCount--;
}
//增加一个连接
void EasyTcpServer::OnNetJoin(ClientSocket* pClient) {
	std::lock_guard<std::mutex>lock(_mutex);
	_clients.push_back(pClient);
	_clientCount++;
}
//接收数据次数
void EasyTcpServer::OnNetRecv(ClientSocket* pClient) {
	_msgCount++;
}
//处理一次数据
void EasyTcpServer::OnNetMsg(CellServer* pCellServer, ClientSocket* pClient, DataHeader* header) {
	_recvCount++;
}
#endif