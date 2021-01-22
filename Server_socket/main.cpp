#define WIN32_LEAN_AND_MEAN
#include<windows.h>
#include<WinSock2.h>
#include<string.h>
#include<iostream>
using namespace std;

int main() {
    WORD var = MAKEWORD(2, 3);
    WSADATA dat;
    WSAStartup(var, &dat);

    //����һ��socket�׽���
    SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    //�󶨶˿ں�
    sockaddr_in _sin = {};
    _sin.sin_family = AF_INET;
    _sin.sin_port = htons(4567); //host to net unsigned short
    _sin.sin_addr.S_un.S_addr = INADDR_ANY;
    if (SOCKET_ERROR == bind(_sock, (sockaddr*)&_sin, sizeof(_sin)))
    {
        std::cout << "ERROR: �����ڽ��ܿͻ���ʧ��" << std::endl;
    }
    else {
        std::cout << "�����ڽ��ܿͻ��˳ɹ�" << std::endl;
    }
    //3. listen ��������˿�
    if (SOCKET_ERROR == listen(_sock, 5))
    {
        std::cout << "ERROR: ��������˿�ʧ��..." << std::endl;
    }
    else {
        std::cout << "��������˿ڳɹ�.." << std::endl;
    }
    //accept �ȴ����ܿͻ�������
    sockaddr_in cAddr = {};
    SOCKET _cSock = INVALID_SOCKET;
    int nAddrLen = sizeof(sockaddr_in);
    _cSock = accept(_sock, (sockaddr*)&cAddr, &nAddrLen);
    if (INVALID_SOCKET == _cSock) {
        std::cout << "���󣬽��ܵ���Ч�ͻ���SOCKET..." << std::endl;
    }

    cout << "�¿ͻ��˼��룬IP = " << cAddr.sin_family << "; port = " << cAddr.sin_port << endl;


    while (true) {
        char recvBuf[128];
        int nLen = recv(_cSock, recvBuf, 128, 0);
        if (nLen == 0) {
            std::cout << "�ͻ������˳����������..." << std::endl;
        }
        //����˴���ͻ��˷��͵�����
        if (0 == strcmp(recvBuf, "GetName")) {
            char myBuf[] = "YeXiao";
            send(_cSock, myBuf, strlen(myBuf) + 1, 0);
        }
        else if (0 == strcmp(recvBuf, "GetAge")) {
            char myBuf[] = "27";
            send(_cSock, myBuf, strlen(myBuf) + 1, 0);
        }
        else {
            char myBuf[] = "???";
            send(_cSock, myBuf, strlen(myBuf) + 1, 0);
        }
    }

    //�ر�socket
    closesocket(_sock);
    //
    WSACleanup();
    return 0;
}