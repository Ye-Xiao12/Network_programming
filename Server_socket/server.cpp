#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<windows.h>
#include<WinSock2.h>
#include<string.h>
#include<iostream>
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
struct LoginResult : public DataHeader{
    LoginResult() {
        datalength = sizeof(LoginResult);
        cmd = CMD_LOGIN_RESULT;
    }
    int result;
};
//登出
struct Logout : public DataHeader{
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
    else {
        std::cout << "新客户端加入，Socket: " << (int)_cSock << " IP: " << inet_ntoa(cAddr.sin_addr) << std::endl;
    }

    while (true) {
        DataHeader header = {};
        //接收客户端信息
        int nLen = recv(_cSock, (char*)&header, sizeof(DataHeader), 0);
        if (nLen <= 0) {
            std::cout << "接收失败" << std::endl;
        }
        else {
            std::cout << "收到客户端命令: " << header.cmd << " 命令长度： " << header.datalength << std::endl;
        }
        switch (header.cmd) {
            case CMD_LOGIN: 
            {
                Login login ;
                recv(_cSock, (char*)&login + sizeof(DataHeader), sizeof(Login)-sizeof(DataHeader), 0);
                std::cout << login.UserName << "  "<< login.PassWord << std::endl;
                //处理部分
                LoginResult res ;
                res.result = 1;
                send(_cSock, (char*)&res, sizeof(LoginResult), 0);
            }
            break;
            case CMD_LOGOUT: 
            {
                Logout logout = {};
                recv(_cSock, (char*)&logout + sizeof(DataHeader), sizeof(Logout)- sizeof(DataHeader), 0);
                std::cout << logout.UserName << std::endl;
                //忽略判断用户密码是否正确的过程
                LogoutResult ret;
                send(_cSock, (char*)&ret, sizeof(LogoutResult), 0);
            }
            break;
            default: 
                header.cmd = CMD_ERROR;
                header.datalength = 0;
                send(_cSock, (char*)&header, sizeof(header), 0);
                break;
        }
    }

    //关闭socket
    closesocket(_sock);
    //
    WSACleanup();
    return 0;
}