#include"EasyTcyClient.hpp"
#include<iostream>
#include<thread>
#include<string>
#include<string.h>
using namespace std;
void cmdThread(EasyTcpClient *client);    //�����߳�

int main() {
    EasyTcpClient client;
    client.InitSocket();
    client.Connect("127.0.0.1", 4568);  //���ӷ�����
    
    //����������߳�
    std::thread t1(cmdThread, &client);
    t1.detach();

    while (true) {
        if (!client.OnRun()) {
            break;
        };
    }

    client.Close();
    //Sleep(10000);
    return 0;
}

//�ͻ��������߳�
void cmdThread(EasyTcpClient *client) {
    while (true) {
        string cmdBuf;
        std::cout << "���������" << std::endl;
        cin >> cmdBuf;
        if (cmdBuf == "exit") {
            std::cout << "�˳�cmdThread�߳�..." << std::endl;
            break;
        }
        else if (cmdBuf == "login") {
            Login login;
            strcpy(login.UserName, "ZhangSan");
            strcpy(login.PassWord, "12345678");
            client->SendData(&login);
        }
        else if (cmdBuf == "logout") {
            Logout logout;
            strcpy(logout.UserName, "LiSi");
            client->SendData(&logout);
        }
        else {
            std::cout << "��֧�ָ�����..." << std::endl;
        }
    }
}