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
   
    while (true) {
        server.OnRun();

#ifdef _WIN32
        //Sleep(3000);
#else
        sleep(4);
#endif
        //std::cout << "服务器处理其他空闲任务..." << std::endl;
    }
    server.Close();
    return 0;
}
