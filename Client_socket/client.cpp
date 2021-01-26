#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS 
#include<windows.h>
#include<WinSock2.h>
#include<iostream>
#include<thread>
#include<string>
#include<string.h>
using namespace std;
bool g_bRun = true;
int processor(SOCKET _sock);
void cmdThread(SOCKET _sock);    //输入线程

//请求类型
enum CMD {
    CMD_LOGIN,
    CMD_LOGIN_RESULT,
    CMD_LOGOUT,
    CMD_LOGOUT_RESULT,
    CMD_ERROR,
    CMD_NEW_USER_JOIN,
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

struct NewUserJoin :public DataHeader {
    NewUserJoin() {
        datalength = sizeof(NewUserJoin);
        cmd = CMD_NEW_USER_JOIN;
        this->scok = 0;
    }
    int scok;
};

int main() {
    WORD var = MAKEWORD(2, 3);
    WSADATA dat;
    WSAStartup(var, &dat);
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

    std::thread t1(cmdThread, _sock);
    t1.detach();

    while (true) {
        fd_set fdReads;
        FD_ZERO(&fdReads);
        FD_SET(_sock, &fdReads);
        timeval t = { 0,0 };
        int ret = select(_sock, &fdReads, 0, 0, &t);
        if (ret < 0) {
            std::cout << "select任务结束..." << std::endl;
        }
        if (FD_ISSET(_sock, &fdReads)) {
            FD_CLR(_sock, &fdReads);
            if (-1 == processor(_sock)) {
                std::cout << "select 任务结束..." << std::endl;
                break;
            }
        }
        Sleep(1000);
        //std::cout << "空闲时间，处理client其他任务..." << std::endl;
    }
    // 关闭socket
    closesocket(_sock);
    //清除windows socket环境
    WSACleanup();
    return 0;
}

int processor(SOCKET _sock) {
    //向服务端发送请求
    char recvBuf[1024];
    int ret = recv(_sock, recvBuf, sizeof(DataHeader),0);
    DataHeader* dh = (DataHeader*)recvBuf;

    if (ret < 0) {
        std::cout << "收到exit命令，任务结束" << std::endl;
        return -1;
    }
    switch (dh->cmd) {
    case CMD_LOGIN_RESULT:
    {
        recv(_sock, recvBuf + sizeof(DataHeader), sizeof(LoginResult) - sizeof(DataHeader), 0);
        LoginResult* login = (LoginResult*)recvBuf;
        std::cout << "接收到服务端信息: LoginResult" << std::endl;
    }
        break;
    case CMD_LOGOUT_RESULT:
    {
        recv(_sock, recvBuf + sizeof(DataHeader), sizeof(LogoutResult) - sizeof(DataHeader), 0);
        LogoutResult* logout = (LogoutResult*)recvBuf;
        std::cout << "接收到服务端信息: LogoutResult" << std::endl;
    }
        break;
    case CMD_NEW_USER_JOIN:
    {
        recv(_sock, recvBuf + sizeof(DataHeader), sizeof(NewUserJoin) - sizeof(DataHeader), 0);
        NewUserJoin* userJoin = (NewUserJoin*)recvBuf;
        std::cout << "新加入一个玩家..." << std::endl;
    }
        break;
    }
    return 1;
}

//客户端输入线程
void cmdThread(SOCKET _sock) {
    while (true) {
        string cmdBuf;
        std::cout << "请输入命令：" << std::endl;
        cin >> cmdBuf;
        if (cmdBuf == "exit") {
            g_bRun = false;
            std::cout << "推出cmdThread线程..." << std::endl;
            break;
        }
        else if (cmdBuf == "login") {
            Login login;
            strcpy(login.UserName, "ZhangSan");
            strcpy(login.PassWord, "12345678");
            send(_sock, (const char*)&login, sizeof(Login), 0);
        }
        else if (cmdBuf == "logout") {
            Logout logout;
            strcpy(logout.UserName, "LiSi");
            send(_sock, (const char*)&logout, sizeof(Logout), 0);
        }
        else {
            std::cout << "不支持该命令..." << std::endl;
        }
    }
}