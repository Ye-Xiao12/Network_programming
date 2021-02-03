#ifndef FD_SETSIZE
#define FD_SETSIZE      1024
#endif

#include"EasyTcyClient.hpp"
#include<iostream>
#include<thread>
#include<string>
#include<string.h>
using namespace std;
bool g_bRun = true;
void cmdThread();    //输入线程

int main() {
    const int cCount = FD_SETSIZE - 1;
    EasyTcpClient* client[cCount];
    for (int i = 0; i < cCount; ++i) {
        client[i] = new EasyTcpClient;
        client[i]->InitSocket();
        client[i]->Connect("127.0.0.1", 4568);
    }

    //负责输入的线程
    std::thread t1(cmdThread);
    t1.detach();
    Login login;
    strcpy(login.UserName, "ZhangSan");
    strcpy(login.PassWord, "12345678");

    while (g_bRun) {
        for (int i = 0; i < cCount; ++i) {
            client[i]->SendData(&login);
            client[i]->OnRun();
        }
    }

    for (int i = 0; i < cCount; ++i) {
        client[i]->Close();
    }

    Sleep(10000);
    return 0;
}

//客户端输入线程
void cmdThread() {
    while (true) {
        string cmdBuf;
        std::cout << "请输入命令：" << std::endl;
        cin >> cmdBuf;
        if (cmdBuf == "exit") {
            std::cout << "退出cmdThread线程..." << std::endl;
            g_bRun = false;
            break;
        }
        else {
            std::cout << "不支持该命令..." << std::endl;
        }
    }
}