#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS 
#include<windows.h>
#include<WinSock2.h>
#include<iostream>
#include<string>
#include <stdio.h>
#include <stdlib.h>

int main() {
    WORD var = MAKEWORD(2, 3);
    WSADATA dat;
    WSAStartup(var, &dat);
    //
    //����һ��socket
    SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    //�󶨶˿ں�(�������˿�)
    sockaddr_in _sin = {};
    _sin.sin_family = AF_INET;
    _sin.sin_port = htons(4567);
    _sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
    //���ӷ�����
    int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr));
    if (SOCKET_ERROR == ret) {
        std::cout << "ERRO: ����ʧ��" << std::endl;
    }
    else {
        std::cout << "���ӳɹ�..." << std::endl;
    }
    //���ܷ�������Ϣ recv
    //char recvBuf[256] = {};
    //recv(_sock, recvBuf, 256, 0);

    while (true) {
        //�����˷�������
        char sendBuf[128] = "GetAge";
        char reBuf[128];
        std::cout << "���������� " << std::endl;
        scanf("%s", sendBuf);
        send(_sock, sendBuf, 128, 0);

        int rLen = recv(_sock, reBuf, 128, 0);
        printf("�յ��������ش�%s\n", reBuf);
    }
    // �ر�socket
    closesocket(_sock);
    /// </summary>
    /// <returns></returns>
    WSACleanup();
    return 0;
}