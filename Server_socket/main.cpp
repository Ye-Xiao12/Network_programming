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

    //建立一个socket套接字
    SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    //绑定端口号
    sockaddr_in _sin = {};
    _sin.sin_family = AF_INET;
    _sin.sin_port = htons(4567); //host to net unsigned short
    _sin.sin_addr.S_un.S_addr = INADDR_ANY;
    if (SOCKET_ERROR == bind(_sock, (sockaddr*)&_sin, sizeof(_sin)))
    {
        std::cout << "ERROR: 绑定用于接受客户端失败" << std::endl;
    }
    else {
        std::cout << "绑定用于接受客户端成功" << std::endl;
    }
    //3. listen 监听网络端口
    if (SOCKET_ERROR == listen(_sock, 5))
    {
        std::cout << "ERROR: 监听网络端口失败..." << std::endl;
    }
    else {
        std::cout << "监听网络端口成功.." << std::endl;
    }
    //accept 等待接受客户端连接
    sockaddr_in cAddr = {};
    SOCKET _cSock = INVALID_SOCKET;
    int nAddrLen = sizeof(sockaddr_in);
    _cSock = accept(_sock, (sockaddr*)&cAddr, &nAddrLen);
    if (INVALID_SOCKET == _cSock) {
        std::cout << "错误，接受到无效客户端SOCKET..." << std::endl;
    }

    cout << "新客户端加入，IP = " << cAddr.sin_family << "; port = " << cAddr.sin_port << endl;


    while (true) {
        char recvBuf[128];
        int nLen = recv(_cSock, recvBuf, 128, 0);
        if (nLen == 0) {
            std::cout << "客户端已退出，任务结束..." << std::endl;
        }
        //服务端处理客户端发送的请求
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

    //关闭socket
    closesocket(_sock);
    //
    WSACleanup();
    return 0;
}