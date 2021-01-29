#ifndef _EASY_CONNECT_Client
#define _EASY_CONNECT_Client

#ifdef _WIN32   //windwows����ϵͳ��
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

	void InitSocket();	//��ʼ��_sock
	int Connect(const char* ip, unsigned short port);	//�󶨶˿ںţ����ӷ�����
	void Close();	//�ر��׽���_socket

	bool OnRun();	//_sock��������
	bool IsRun();	//�ж�_sock�Ƿ�ռ��
	int RecvData(SOCKET _sock);	//������
	void OnNetMsg(DataHeader* header);	//�����յ��ı�ͷ���ͻ���������Ӧ
	int SendData(DataHeader* header);	//�ظ���Ϣ
private:
	SOCKET _sock;
	char szRecv[4096];
};
//���캯��
EasyTcpClient::EasyTcpClient() {
	_sock = INVALID_SOCKET;
}
//��������
EasyTcpClient::~EasyTcpClient() {
	Close();
}

//��ʼ������
void EasyTcpClient::InitSocket() {
#ifdef _WIN32
	WORD var = MAKEWORD(2, 3);
	WSADATA dat;
	WSAStartup(var, &dat);
#endif
	//���_sockû�ͷţ����ͷž�����
	if (INVALID_SOCKET != _sock) {
		std::cout << "<Socket=" << _sock << ">   �رվ�����..." << std::endl;
		_sock = INVALID_SOCKET;
	}
	_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == _sock) {
		std::cout << "Error! �����׽���ʧ��" << std::endl;
	}
	else {
		std::cout << "�ɹ�����Socket..." << std::endl;
	}
}

//����socket������ip��port
int EasyTcpClient::Connect(const char* ip, unsigned short port) {
	//�󶨶˿ں�(�������˿�)
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
		std::cout << "���ӷ�����ʧ��..." << std::endl;
		return -1;
	}
	else {
		std::cout << "���ӷ������ɹ�..." << std::endl;
	}
	return 1;
}

//�ر�����
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
//�ͻ��˹�������
bool EasyTcpClient::OnRun() {
	if (IsRun()) {
		fd_set fdReads;
		FD_ZERO(&fdReads);
		FD_SET(_sock, &fdReads);
		timeval t = { 1,0 };
		int ret = select(_sock + 1, &fdReads, 0, 0, &t);
		if (ret < 0) {
			std::cout << "<socket=" << _sock << ">select�������" << std::endl;
			return false;
		}

		if (FD_ISSET(_sock, &fdReads)) {
			FD_CLR(_sock, &fdReads);
			if (-1 == RecvData(_sock)) {
				std::cout << "<socket=" << _sock << ">select�������" << std::endl;
				return false;
			}
		}
		return true;
	}
	return false;
}
//�ж�_sock�Ƿ�ռ��
bool EasyTcpClient::IsRun() {
	return INVALID_SOCKET != _sock;
}
//������
int EasyTcpClient::RecvData(SOCKET _sock) {
	int nLen = recv(_sock, szRecv, sizeof(DataHeader), 0);
	DataHeader* dh = (DataHeader*)szRecv;
	if (nLen <= 0) {
		std::cout << "��������Ͽ����ӣ��������" << std::endl;
		return -1;
	}
	recv(_sock, szRecv + dh->datalength, dh->datalength - sizeof(DataHeader), 0);
	OnNetMsg(dh);
	return 0;
}
//�����յ��ı�ͷ���ͻ���������Ӧ
void EasyTcpClient::OnNetMsg(DataHeader* header) {
	switch (header->cmd) {
	case CMD_LOGIN_RESULT:
	{
		LoginResult* login = (LoginResult*)szRecv;
		std::cout << "���յ��������Ϣ: LoginResult" << std::endl;
	}
	break;
	case CMD_LOGOUT_RESULT:
	{
		LogoutResult* logout = (LogoutResult*)szRecv;
		std::cout << "���յ��������Ϣ: LogoutResult" << std::endl;
	}
	break;
	case CMD_NEW_USER_JOIN:
	{
		NewUserJoin* userJoin = (NewUserJoin*)szRecv;
		std::cout << "�¼���һ�����..." << std::endl;
	}
	break;
	}
}
//�ظ���Ϣ
int EasyTcpClient::SendData(DataHeader* header) {
	if (IsRun() && header) {
		return send(_sock, (const char*)header, header->datalength, 0);
	}
	return SOCKET_ERROR;
}
#endif