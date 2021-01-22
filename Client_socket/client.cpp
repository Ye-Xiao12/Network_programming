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
    //创建一个socket
    SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    //绑定端口号(服务器端口)
    sockaddr_in _sin = {};
    _sin.sin_family = AF_INET;
    _sin.sin_port = htons(4567);
    _sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
    //连接服务器
    int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr));
    if (SOCKET_ERROR == ret) {
        std::cout << "ERRO: 连接失败" << std::endl;
    }
    else {
        std::cout << "连接成功..." << std::endl;
    }
    //接受服务器信息 recv
    //char recvBuf[256] = {};
    //recv(_sock, recvBuf, 256, 0);

    while (true) {
        //向服务端发送请求
        char sendBuf[128] = "GetAge";
        char reBuf[128];
        std::cout << "请输入请求 " << std::endl;
        scanf("%s", sendBuf);
        send(_sock, sendBuf, 128, 0);

        int rLen = recv(_sock, reBuf, 128, 0);
        printf("收到服务器回答：%s\n", reBuf);
    }
    // 关闭socket
    closesocket(_sock);
    /// </summary>
    /// <returns></returns>
    WSACleanup();
    return 0;
}