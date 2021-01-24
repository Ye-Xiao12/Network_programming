#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS 
#include<windows.h>
#include<WinSock2.h>
#include<iostream>
#include<string>
#include<string.h>
using namespace std;

//请求类型
enum CMD {
    CMD_LOGIN,
    CMD_LOGIN_RESULT,
    CMD_LOGOUT,
    CMD_LOGOUT_RESULT,
    CMD_ERROR,
};
//报头
struct DataHeader {
    short datalength;
    short cmd;
};

//Login DataPackage
struct Login : public DataHeader {
    Login() {
        strcpy(this->UserName, "XiaoGang");
        strcpy(this->PassWord, "12345678");
        datalength = sizeof(Login);
        cmd = CMD_LOGIN;
    }
    char UserName[32];
    char PassWord[32];
};
//登录请求结果
struct LoginResult : public DataHeader {
    LoginResult() {
        datalength = sizeof(LoginResult);
        cmd = CMD_LOGIN_RESULT;
    }
    int result;
};
//登出
struct Logout : public DataHeader {
    Logout() {
        datalength = sizeof(Logout);
        cmd = CMD_LOGOUT;
    }
    char UserName[32];
};

struct LogoutResult : public DataHeader {
    LogoutResult() {
        datalength = sizeof(LogoutResult);
        cmd = CMD_LOGOUT_RESULT;
        result = 0;
    }
    int result;
};

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

    while (true) {
        //向服务端发送请求
        std::string cmdBuf;
        std::cout << "请输入命令";
        std::cin >> cmdBuf;

        if (cmdBuf == "exit") {
            std::cout << "收到exit命令，任务结束" << std::endl;
        }
        else if (cmdBuf == "login") {
            Login login;
            strcpy(login.UserName, "XiaoMing");
            strcpy(login.PassWord, "12345678");
            //向服务器发送请求
            send(_sock, (const char*)&login, login.datalength, 0);
            //接收服务器返回数据
            LoginResult loginResult;
            recv(_sock, (char*)&loginResult, sizeof(LoginResult), 0);
            std::cout << loginResult.result << std::endl;
        }
        else if (cmdBuf == "logout") {
            Logout logout;
            strcpy(logout.UserName,"大乌龟");
            //向服务器发送请求命令
            send(_sock, (char*)&logout, logout.datalength, 0);
            //接收服务器命令
            LogoutResult logoutRet;
            recv(_sock, (char*)&logoutRet, sizeof(LogoutResult), 0);
            std::cout << logoutRet.result << std::endl;
        }
        else {
            std::cout << "不支持命令，请重新输入：" << std::endl;
        }
    }
    // 关闭socket
    closesocket(_sock);
    //清除windows socket环境
    WSACleanup();
    return 0;
}