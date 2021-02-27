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

#ifndef RECV_BUFF_SIZE	//	���ܻ�������С
#define RECV_BUFF_SIZE 10240
#endif
#define SEND_BUFF_SIZE RECV_BUFF_SIZE
class ClientSocket;
class INetEvent;
class CellServer;
class EasyTcpServer;

//�ͻ�socket��
class ClientSocket {
public:
	//���캯��
	ClientSocket(SOCKET sockfd = INVALID_SOCKET) {
		_sockfd = sockfd;
		//���������������ڴ������ʼ������
		//Ϊʲôû�г�ʼ���Ͳ�����memcpy()���������ڴ棿������
		//��һ���ǹؼ�����������
		memset(_szMsgBuf, 0, sizeof(_szMsgBuf));
		memset(_szSendBuf, 0, sizeof(_szSendBuf));
		_lastPtr = 0;
		_lastSendPtr = 0;	//�Ҳ���ԭ����һ�����ǹؼ�������������
	}
	//���ظÿͻ���socket�ļ���ʶ��
	SOCKET sockfd() {
		return _sockfd;
	}
	//���ظ�socket�������ļ�
	char* msgBuf() {
		return _szMsgBuf;
	}
	//���ػ�����ָ��
	size_t getLastPtr() {
		return _lastPtr;
	}
	//������ָ������Ϊpos
	void setLastPtr(size_t pos) {
		_lastPtr = pos;
	}
	//��������
	int sendData(DataHeader* header) {
		int ret = SOCKET_ERROR;
		//Ҫ���ͳ���
		int nSendLen = header->datalength;
		//Ҫ��������
		const char* pSendData = (const char*)header;
		while (true) {
			if (_lastSendPtr + nSendLen >= SEND_BUFF_SIZE) {
				int nCopyLen = SEND_BUFF_SIZE - _lastSendPtr;
				memcpy(_szSendBuf + _lastSendPtr, pSendData, (size_t)nCopyLen);
				//����ʣ������
				pSendData += nCopyLen;
				//����ʣ�����ݳ���
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
	SOCKET _sockfd;	//�ͻ���socket���
	char _szMsgBuf[RECV_BUFF_SIZE] = {};	//�ͻ��˽��ܻ�����
	char _szSendBuf[SEND_BUFF_SIZE] = {};	//�ͻ��˷��ͻ�����
	size_t _lastPtr;	//ָ�룬ָ��ͻ��˻�����ĩβ
	size_t _lastSendPtr;	//ָ�룬ָ��ͻ��˷��ͻ�������ĩβ
};
//CELLserver���е���EasyTcpServer��Ա�����Ľӿ�
class INetEvent {
public:
	//���麯�����������������ඨ��
	virtual void OnNetLeave(ClientSocket* pClient) = 0;	//����һ������
	virtual void OnNetJoin(ClientSocket* pClient) = 0;	//����һ������
	virtual void OnNetRecv(ClientSocket* pClient) = 0; //�������ݴ���
	virtual void OnNetMsg(CellServer* pCellServer, ClientSocket* pClient, DataHeader* header) = 0;	//�������ݴ���
private:
};

//������Ϣ��������
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
	//ִ������
	void doTask()
	{
		_pClient->sendData(_pHeader);
		delete _pHeader;
	}
};

//��ɷ������˶����ݴ���Ĳ��ַ���
class CellServer {
public:
	CellServer(SOCKET sock = INVALID_SOCKET);
	~CellServer();

	void setEvent(INetEvent* event);	//��ʼ��ָ��_pNetEvent
	void Close();	//�ر����пͻ�������
	int getMsgCount();	//��ȡ��������������������Ϊ0
	int getClientCount();	//��ȡ���߳̿ͻ�������
	void addSendTask(ClientSocket* pClient, DataHeader* header);	//�������
	virtual void OnNetMsg(ClientSocket* pClient, DataHeader* header);	//���������ݵ���Ӧ
	int RecvData(ClientSocket* pClient);
	void addClient(ClientSocket* pClient);	//��ӿͻ��ˣ�������
	bool isRun();	//�жϷ�����sock�Ƿ����ڹ�����
	bool onRun();	//��������Ա������������
	void Start();	//�ó�Ա���������߳�
	int sendData(SOCKET cSock, DataHeader* header);	//���ڷ�������
private:
	SOCKET _sock;	//�����socket
	bool _client_Change = false;	//��־λ������ͻ����������ָı䣬����Ϊtrue
	fd_set _fdRead_Back;	//fdread�ı��ݣ�����ͻ���û�иı䣬ֱ�ӿ�����fd_set
	char _szRecv[RECV_BUFF_SIZE] = {};	//��Ϣ������
	std::vector<ClientSocket*>_clients;	//��Ҫ����Ŀͻ��˶���
	std::vector<ClientSocket*>_clientsBuff;	//���ӵĿͻ��˻�����,�в�ֹ1���̶߳������Դ�����������Ҫ����
	std::mutex _mutex;	//���ڿͻ�����Դ�ٽ�������
	std::atomic_int _recvCount = 0;	//��������
	INetEvent* _pNetEvent;	//ʹ�ô��麯��������EasyTcpServer�е�OnNetMsg(),OnLeave()��Ա�������ﵽ������Ŀ��
	std::thread* _pThread;	//ָ���߳�ָ��
	CellTaskServer _taskServer;	//�����������߳�
};
//�����ǿɶ���Ĭ�ϲ��������庯��ʱ����Ҫ���¶���Ĭ�ϲ���
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
//��ʼ��ָ��_pNetEvent
void CellServer::setEvent(INetEvent* event) {
	_pNetEvent = event;
}
//�ر����пͻ�������
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
//��ȡ��������������������Ϊ0
int CellServer::getMsgCount() {
	int num = _recvCount;
	_recvCount = 0;
	return num;
}
//��ȡ���߳̿ͻ�������
int CellServer::getClientCount() {
	size_t num = _clients.size() + _clientsBuff.size();
	return (int)num;
}
void CellServer::addSendTask(ClientSocket* pClient, DataHeader* header)
{
	CellSendMsg2ClientTask* task = new CellSendMsg2ClientTask(pClient, header);
	_taskServer.addTask(task);
}
//���������ݵ���Ӧ
void CellServer::OnNetMsg(ClientSocket* pClient, DataHeader* header) {
	_pNetEvent->OnNetMsg(this,pClient, header);
}
//�������� ����ճ�� ��ְ�
int CellServer::RecvData(ClientSocket* pClient) {
	_pNetEvent->OnNetRecv(pClient);
	int nLen = recv(pClient->sockfd(), _szRecv, RECV_BUFF_SIZE, 0);
	if (nLen <= 0) {
		std::cout << "���տͻ���<" << pClient->sockfd() << ">";
		std::cout << "ʧ��..." << std::endl;
		return -1;
	}
	//���յ������ݿ�������Ϣ������
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
		else {	//��Ϣ��������������һ��������Ϣ
			break;
		}
	}
	return 1;
}
//���ڷ�������
int CellServer::sendData(SOCKET cSock, DataHeader* header) {
	if (header) {
		return send(cSock, (const char*)header, header->datalength, 0);
	}
	return SOCKET_ERROR;
}
//�������ӵĿͻ�����ӵ��ͻ������ٵ��߳���
void CellServer::addClient(ClientSocket* pClient) {
	std::lock_guard<std::mutex>lock(_mutex);	//�ٽ���Դ����
	_clientsBuff.push_back(pClient);
}
//�жϷ�����sock�Ƿ����ڹ�����
bool CellServer::isRun() {
	return _sock != INVALID_SOCKET;
}
//��������Ա����
bool CellServer::onRun() {
	while (isRun()) {
		if (!_clientsBuff.empty()) {
			//���������Ŀͻ��˼��ص���ǰ����ͻ���
			std::lock_guard<std::mutex>lock(_mutex);
			for (auto pClient : _clientsBuff) {
				_clients.push_back(pClient);
			}
			_clientsBuff.clear();
			_client_Change = true;
		}
		//��������ͻ��˶���Ϊ�գ��߳�����1ms
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
		//selectģʽ�жϽӿ��Ƿ��пɶ���Ϣ
		timeval t = { 0,1 };
		int ret = select(max_sock + 1, &fdReads, 0, 0, &t);
		if (ret < 0) {
			printf("select �������...");
			return false;
		}
		else if(ret == 0) {
			continue;
		}

		//���ÿ���˿ڣ�ɾ���쳣�˿�
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
					break;	//һ��ֻɾ��һ��socket
				}
			}
		}
	}
	return false;
}
//�ó�Ա���������߳�
void CellServer::Start() {
	//_pThread = new std::thread(&CellServer::onRun, this);
	//����Ա����ת��Ϊ��������ʹ�ö���ָ���������ý��а�
	_pThread = new std::thread(std::mem_fn(&CellServer::onRun), this);
	_pThread->detach();
	_taskServer.Start();
}

class EasyTcpServer: public INetEvent {
public:
	EasyTcpServer();
	~EasyTcpServer();

	void InitSocket();	//��ʼ��Socket
	void Close();	//�ر�Socket
	int Bind(const char*ip,unsigned short port);	//�󶨶˿�
	int Listen(int n);	//�����˿�
	void Start(int nCellServer);	//����n�������ķ�����߳�
	SOCKET Accept();	//���ܿͻ��˵�����
	void addClientToCellSever(ClientSocket* pClient);	//Ϊ�����߳����ӿͻ���
	bool isRun();	//�ж�_sock�Ƿ��ڹ�����
	bool OnRun();	//������
	int SendData(SOCKET _cSock,DataHeader* header);	//��ͻ��˷�������
	void SendDataToAll(DataHeader* header);	//��ǰ���ӵ����пͻ��˷�������
	void timePerMsg();	//����ÿ�봦��������
	void OnNetLeave(ClientSocket* pClient);	//����һ������
	void OnNetJoin(ClientSocket* pClient);	//����һ������
	void OnNetRecv(ClientSocket* pClient); //�������ݴ���
	void OnNetMsg(CellServer* pCellServer, ClientSocket* pClient, DataHeader* header);	//����һ������
private:
	SOCKET _sock;
	std::vector<CellServer*>_cellServer;
	CELLTimestamp _Time;	//����ʱ�����
protected:
	std::mutex _mutex;
	std::vector<ClientSocket*>_clients;	//�洢Ŀǰ�������ӿͻ���
	std::atomic_int _msgCount;	//�������ݴ���
	std::atomic_int _recvCount;	//�յ�����Ϣ����
	std::atomic_int _clientCount;	//�ͻ�������
};
//���캯��
EasyTcpServer::EasyTcpServer() {
	_sock = INVALID_SOCKET;
	_msgCount = 0;
	_recvCount = 0;
	_clientCount = 0;
}
//ϵ������
EasyTcpServer::~EasyTcpServer() {
	Close();
	for (auto server : _cellServer) {
		delete server;
	}
}

//��ʼ��Socket
void EasyTcpServer::InitSocket() {
#ifdef _WIN32
	WORD var = MAKEWORD(2, 3);
	WSADATA dat;
	if (WSAStartup(var, &dat) != 0) {
		printf("ERROR: Open WSAStartup!\n");
	}
#endif
	//����һ��socket�׽���
	_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
}
//�󶨶˿�
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
		std::cout << "ERROR: �����ڽ��ܿͻ���ʧ��" << std::endl;
	}
	else {
		std::cout << "�����ڽ��ܿͻ��˳ɹ�" << std::endl;
	}
	return ret;
}
//�����˿�
int EasyTcpServer::Listen(int n) {
	//3. listen ��������˿�
	int ret = listen(_sock, n);
	if (SOCKET_ERROR == ret)
	{
		std::cout << "Socket=<" << _sock << ">";
		std::cout << "ERROR: ��������˿�ʧ��..." << std::endl;
	}
	else {
		std::cout << "Socket=<" << _sock << ">";
		std::cout << "��������˿ڳɹ�.." << std::endl;
	}
	return ret;
}
//����n�������ķ�����߳�
void EasyTcpServer::Start(int nCellServer) {
	for (int i = 0; i < nCellServer; ++i) {
		auto ser = new CellServer(_sock);
		_cellServer.push_back(ser);
		ser->setEvent(this);
		ser->Start();
	}
}
//�ر�Socket
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
//���ܿͻ��˵�����
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
		printf("ERROR,���յ�����Ŀͻ���...\n");
	}
	else {
		//NewUserJoin userjoin;
		//SendDataToAll(&userjoin);
		auto pClient = new ClientSocket(cSock);
		_clients.push_back(pClient);
		addClientToCellSever(pClient);
		OnNetJoin(pClient);
		//std::cout << "�¿ͻ��˼��룺socket: " << (int)cSock;
		//std::cout << " ,Ip=" << inet_ntoa(clientAddr.sin_addr) << std::endl;
	}
	return cSock;
}
//Ϊ�����߳����ӿͻ���
void EasyTcpServer::addClientToCellSever(ClientSocket* pClient) {
	auto minSizeCell = _cellServer[0];
	//���Ҵ���ͻ������ٵ��̣߳����¼���Ŀͻ�����ӵ����߳�
	for (auto server : _cellServer) {
		if (server->getClientCount() < minSizeCell->getClientCount()) {
			minSizeCell = server;
		}
	}
	minSizeCell->addClient(pClient);
}
//��ͻ��˷�������
int EasyTcpServer::SendData(SOCKET _cSock, DataHeader* header) {
	if (header) {
		return send(_cSock, (const char*)header, header->datalength, 0);
	}
	return SOCKET_ERROR;
}
//��ǰ���ӵ����пͻ��˷�������
void EasyTcpServer::SendDataToAll(DataHeader* header) {
	for (auto client : _clients) {
		send(client->sockfd(), (const char*)header, header->datalength, 0);
	}
}
//�ж�_sock�Ƿ��ڹ�����
bool EasyTcpServer::isRun()
{
	return _sock != INVALID_SOCKET;
}
bool EasyTcpServer::OnRun() {
	if (isRun()) {
		//��ӡÿ�봦��������
		timePerMsg();

		fd_set fdReads;
		FD_ZERO(&fdReads);
		FD_SET(_sock, &fdReads);
		SOCKET max_sock = _sock;
		//selectģʽ�жϽӿ��Ƿ��пɶ���Ϣ
		timeval t = { 0,10 };
		int ret = select(max_sock + 1, &fdReads, 0, 0, &t);
		if (ret < 0) {
			printf("select �������...\n");
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
//����ÿ�봦��������
void EasyTcpServer::timePerMsg() {
	auto t = _Time.getElapsedSecond();
	if (t >= 1.0) {
		printf("Time:<%lf>; thread:<%d>; clients:<%d>; recv<%d>; Message:<%d>\n",t,(int)_cellServer.size(), _clientCount.load(), (int)(_recvCount.load() / t),(int)(_msgCount.load() / t));
		_Time.update();
		_recvCount = 0;
		_msgCount = 0;
	}
}
//����һ������
void EasyTcpServer::OnNetLeave(ClientSocket* pClient) {
	std::lock_guard<std::mutex>lock(_mutex);
	auto it = find(_clients.begin(), _clients.end(), pClient);
	_clients.erase(it);
	_clientCount--;
}
//����һ������
void EasyTcpServer::OnNetJoin(ClientSocket* pClient) {
	std::lock_guard<std::mutex>lock(_mutex);
	_clients.push_back(pClient);
	_clientCount++;
}
//�������ݴ���
void EasyTcpServer::OnNetRecv(ClientSocket* pClient) {
	_msgCount++;
}
//����һ������
void EasyTcpServer::OnNetMsg(CellServer* pCellServer, ClientSocket* pClient, DataHeader* header) {
	_recvCount++;
}
#endif