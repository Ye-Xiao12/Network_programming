#ifndef FD_SETSIZE
#define FD_SETSIZE      10001
#endif

#include"EasyTcpServer.hpp"
#include<vector>
#include<string.h>
#include<iostream>
#include<algorithm>
using namespace std;

int main() {
    EasyTcpServer server;
    server.InitSocket();
    server.Bind(NULL, 4568);
    server.Listen(5);
    server.Start(1);    //����4�������ĵ��߳�
   
    while (true) {
        server.OnRun();
    }
    server.Close();
    return 0;
}
