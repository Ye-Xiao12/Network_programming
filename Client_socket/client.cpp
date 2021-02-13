#ifndef FD_SETSIZE
#define FD_SETSIZE      10240
#endif
#include"EasyTcyClient.hpp"
#include<iostream>
#include<thread>
#include<string>
#include<string.h>
using namespace std;
bool g_bRun = true;
const int cCount = 4000;    //模拟客户端数目
const int tCount = 2;   //线程数
char ip[] = "127.0.0.1";
unsigned short port = 4568;
EasyTcpClient* client[cCount];
void cmdThread();    //输入线程
void sendThread(int index); //工作线程

int main() {
    //负责输入的线程
    std::thread runThread[tCount];
    std::thread t1(cmdThread);
    t1.detach();

    for (int i = 0; i < tCount; ++i) {
        runThread[i] = std::thread(sendThread, i + 1);
    }

    for (int i = 0; i < tCount; ++i) {
        runThread[i].join();
    }

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

//工作线程
void sendThread(int index) {
    int c = cCount / tCount;
    int begin = (index - 1) * c;
    int end = index * c;

    for (int i = begin; i < end; ++i) {
        client[i] = new EasyTcpClient();
        client[i]->InitSocket();
        client[i]->Connect(ip, port);
    }

    Login login[10];
    //strcpy(login.UserName, "ZhangSan");
    //strcpy(login.PassWord, "12345678");

    while (g_bRun) {
        for (int i = begin; i < end; ++i) {
            client[i]->SendData(login,5);
            client[i]->OnRun();
        }
    }

    for (int i = begin; i < end; ++i) {
        client[i]->Close();
    }
}