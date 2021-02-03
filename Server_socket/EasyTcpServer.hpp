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

class EasyTcpServer {
public:
	EasyTcpServer();
	~EasyTcpServer();

	void InitSocket();	//��ʼ��Socket
	void Close();	//�ر�Socket
	int Bind(const char*ip,unsigned short port);	//�󶨶˿�
	int Listen(int n);	//�����˿�
	int RecvData(ClientSocket* pClient);
	SOCKET Accept();	//���ܿͻ��˵�����
	bool isRun();	//�ж�_sock�Ƿ��ڹ�����
	bool OnRun();	//������
	int SendData(SOCKET _cSock,DataHeader* header);	//��ͻ��˷�������
	void SendDataToAll(DataHeader* header);	//��ǰ���ӵ����пͻ��˷�������
	virtual void OnNetMsg(DataHeader* header);
private:
	SOCKET _sock;
	char _szRecv[RECV_BUFF_SIZE] = {};
	std::vector<ClientSocket *>_gClient;
	int _numMessage;	//����������������
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
//��������
int EasyTcpServer::RecvData(ClientSocket *pClient) {
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
			OnNetMsg(header);
			memcpy(pClient->msgBuf(), pClient->msgBuf() + header->datalength, nSize);
			pClient->setLastPtr(nSize);
		}
		else {	//��Ϣ��������������һ��������Ϣ
			break;
		}
	}
	return 1;
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
		std::cout << "ERROR,���յ�����Ŀͻ���..." << std::endl;
	}
	else {
		//NewUserJoin userjoin;
		//SendDataToAll(&userjoin);
		_gClient.push_back(new ClientSocket(cSock));
		//std::cout << "�¿ͻ��˼��룺socket: " << (int)cSock;
		//std::cout << " ,Ip=" << inet_ntoa(clientAddr.sin_addr) << std::endl;
	}
	return cSock;
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
	for (size_t i = 0; i < _gClient.size(); ++i) {
		SendData(_gClient[i]->sockfd(), header);
	}
}
//�ж�_sock�Ƿ��ڹ�����
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
		//selectģʽ�жϽӿ��Ƿ��пɶ���Ϣ
		timeval t = { 0,0 };
		int ret = select(max_sock + 1, &fdReads, 0, 0, &t);
		if (ret < 0) {
			std::cout << "select �������..." << std::endl;
			return false;
		}

		if (FD_ISSET(_sock, &fdReads)) {
			FD_CLR(_sock, &fdReads);
			Accept();
		}
		//���ÿ���˿ڣ�ɾ���쳣�˿�
		for (size_t i = 0; i < _gClient.size(); ++i) {
			if (FD_ISSET(_gClient[i]->sockfd(), &fdReads)) {
				FD_CLR(_gClient[i]->sockfd(), &fdReads);
				if (-1 == RecvData(_gClient[i])) {
					auto it = _gClient.begin() + i;
					if (it != _gClient.end()) {
						delete _gClient[i];
						_gClient.erase(it);
					}
					break;	//һ��ֻɾ��һ��socket
				}
			}
		}
		return true;
	}
	return false;
}
void EasyTcpServer::OnNetMsg(DataHeader* header) {
	//���������ÿ����ձ�������
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
#endif