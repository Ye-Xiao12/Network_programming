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
const int cCount = 4000;    //ģ��ͻ�����Ŀ
const int tCount = 2;   //�߳���
char ip[] = "127.0.0.1";
unsigned short port = 4568;
EasyTcpClient* client[cCount];
void cmdThread();    //�����߳�
void sendThread(int index); //�����߳�

int main() {
    //����������߳�
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

//�ͻ��������߳�
void cmdThread() {
    while (true) {
        string cmdBuf;
        std::cout << "���������" << std::endl;
        cin >> cmdBuf;
        if (cmdBuf == "exit") {
            std::cout << "�˳�cmdThread�߳�..." << std::endl;
            g_bRun = false;
            break;
        }
        else {
            std::cout << "��֧�ָ�����..." << std::endl;
        }
    }
}

//�����߳�
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