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

#ifndef RECV_BUFF_SIZE
#define RECV_BUFF_SIZE 10240
#endif
//�ͻ�socket��
class ClientSocket {
public:
	//���캯��
	ClientSocket(SOCKET sockfd = INVALID_SOCKET) {
		_sockfd = sockfd;
		memset(_szMsgBuf, 0, sizeof(_szMsgBuf));
		_lastPtr = 0;
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
private:
	SOCKET _sockfd;
	char _szMsgBuf[RECV_BUFF_SIZE * 10] = {};
	size_t _lastPtr;
};
//ִ��OnNetMsg�������ܻ���
class INetEvent {
public:
	//���麯�����������������ඨ��
	virtual void OnLeave(ClientSocket* pClient) = 0;
private:
};
//��ɷ������˶����ݴ���ķ���
class CellServer {
public:
	CellServer(SOCKET sock = INVALID_SOCKET);
	~CellServer();

	void setEvent(INetEvent* event);	//��ʼ��ָ��_pNetEvent
	void Close();	//�ر����пͻ�������
	int getMsgCount();	//��ȡ��������������������Ϊ0
	int getClientCount();	//��ȡ���߳̿ͻ�������
	virtual void OnNetMsg(SOCKET cSock, DataHeader* header);	//���������ݵ���Ӧ
	int RecvData(ClientSocket* pClient);
	void addClient(ClientSocket* pClient);
	bool isRun();	//�жϷ�����sock�Ƿ����ڹ�����
	bool onRun();	//��������Ա����
	void Start();	//�ó�Ա���������߳�
private:
	SOCKET _sock; 
	char _szRecv[RECV_BUFF_SIZE] = {};
	std::vector<ClientSocket*>_clients;	//��Ҫ����Ŀͻ��˶���
	std::vector<ClientSocket*>_clientsBuff;	//���ӵĿͻ��˻�����
	std::mutex _mutex;	//���ڿͻ�����Դ�ٽ�������
	std::atomic_int _recvCount = 0;	//��������
	INetEvent* _pNetEvent;	//ʹ�ô��麯��������EasyTcpServer�е�OnNetMsg(),OnLeave()��Ա�������ﵽ������Ŀ��
	std::thread* _pThread;	//ָ���߳�ָ��
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
	int num = _clients.size() + _clientsBuff.size();
	return num;
}
//���������ݵ���Ӧ
void CellServer::OnNetMsg(SOCKET cSock, DataHeader* header) {
	_recvCount++;	//ԭ�Ӳ���
	switch (header->cmd) {
	case CMD_LOGIN:
	{
		//����Ԫ�ش��λ��Ӧ���ǰ����������˳�������ڴ��д��
		Login* login = (Login*)header;
		/*std::cout << "�յ�login��Ϣ...";
		std::cout << login->UserName << "  " << login->PassWord << std::endl;*/
	}
	break;
	case CMD_LOGOUT:
	{
		Logout* logout = (Logout*)header;
		std::cout << "�յ�logout��Ϣ..." << std::endl;
	}
	break;
	default:
	{
		std::cout << "�յ���Ϣ�������κ����..." << std::endl;
	}
	break;
	}
}
//�������� ����ճ�� ��ְ�
int CellServer::RecvData(ClientSocket* pClient) {
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
			OnNetMsg(pClient->sockfd(),header);
			memcpy(pClient->msgBuf(), pClient->msgBuf() + header->datalength, nSize);
			pClient->setLastPtr(nSize);
		}
		else {	//��Ϣ��������������һ��������Ϣ
			break;
		}
	}
	return 1;
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
		}
		//��������ͻ��˶���Ϊ�գ��߳�����1ms
		if (_clients.empty()) {
			std::chrono::milliseconds t(1);
			std::this_thread::sleep_for(t);
			continue;
		}

		fd_set fdReads;
		FD_ZERO(&fdReads);
		FD_SET(_sock, &fdReads);
		SOCKET max_sock = _clients[0]->sockfd();
		for (size_t i = 0; i < _clients.size(); ++i) {
#ifdef __linux__
			if (max_sock < _clients[i]->sockfd())
				max_sock = _clients[i]->sockfd();
#endif
			FD_SET(_clients[i]->sockfd(), &fdReads);
		}
		//selectģʽ�жϽӿ��Ƿ��пɶ���Ϣ
		timeval t = { 0,10 };
		int ret = select(max_sock + 1, &fdReads, 0, 0, &t);
		if (ret < 0) {
			printf("select �������...");
			return false;
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
							_pNetEvent->OnLeave(_clients[i]);
						}
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
	void OnLeave(ClientSocket* pClient);	//ɾ��ָ���ͻ���
private:
	SOCKET _sock;
	char _szRecv[RECV_BUFF_SIZE] = {};
	std::vector<ClientSocket*>_clients;	//�洢Ŀǰ�������ӿͻ���
	std::vector<CellServer*>_cellServer;
	std::mutex _mutex;
	CELLTimestamp _Time;	//����ʱ�����
};
//���캯��
EasyTcpServer::EasyTcpServer() {
	_sock = INVALID_SOCKET;
}
//ϵ������
EasyTcpServer::~EasyTcpServer() {
	Close();
}

//��ʼ��Socket
void EasyTcpServer::InitSocket() {
#ifdef _WIN32
	WORD var = MAKEWORD(2, 3);
	WSADATA dat;
	WSAStartup(var, &dat);
#endif
	//����һ��socket�׽���
	_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
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
void EasyTcpServer::Start(int nCellServer) {
	for (int i = 0; i < nCellServer; ++i) {
		auto ser = new CellServer(_sock);
		_cellServer.push_back(ser);
		ser->setEvent(this);
		ser->Start();
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
		int numMsg = 0;
		int numClient = 0;
		for (auto cell : _cellServer) {
			numMsg += cell->getMsgCount();
			numClient += cell->getClientCount();
		}
		printf("Time:<%lf>; thread:<%d>; clients:<%d>; Message:<%d>\n",t,_cellServer.size(),numClient,numMsg);
		_Time.update();
	}
}
//ɾ��ָ���ͻ���
void EasyTcpServer::OnLeave(ClientSocket* pClient) {
	std::lock_guard<std::mutex>lock(_mutex);
	auto it = find(_clients.begin(), _clients.end(), pClient);
	_clients.erase(it);
}
#endif