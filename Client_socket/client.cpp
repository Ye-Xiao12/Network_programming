#include"EasyTcyClient.hpp"
#include<iostream>
#include<thread>
#include<string>
#include<string.h>
using namespace std;
void cmdThread(EasyTcpClient *client);    //输入线程

int main() {
    EasyTcpClient client;
    client.InitSocket();
    client.Connect("127.0.0.1", 4568);  //连接服务器
    
    //负责输入的线程
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

//客户端输入线程
void cmdThread(EasyTcpClient *client) {
    while (true) {
        string cmdBuf;
        std::cout << "请输入命令：" << std::endl;
        cin >> cmdBuf;
        if (cmdBuf == "exit") {
            std::cout << "退出cmdThread线程..." << std::endl;
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
            std::cout << "不支持该命令..." << std::endl;
        }
    }
}