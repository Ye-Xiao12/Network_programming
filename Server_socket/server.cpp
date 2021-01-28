#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #define _CRT_SECURE_NO_WARNINGS
    #define _WINSOCK_DEPRECATED_NO_WARNINGS
    #include<windows.h>
    #include<WinSock2.h>
#else
    #include<unistd.h>  //unix std
    #include<arpa/inet.h>
    #define SOCKET int
    #define INVALID_SOCKET  (SOCKET)(~0)
    #define SOCKET_ERROR            (-1)
#endif
#include<vector>
#include<string.h>
#include<iostream>
#include<algorithm>
using namespace std;
vector<SOCKET>g_client;
int processor(SOCKET _cSock);

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
#ifdef _WIN32
    WORD var = MAKEWORD(2, 3);
    WSADATA dat;
    WSAStartup(var, &dat);
#endif
    //建立一个socket套接字
    SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    //绑定端口号
    sockaddr_in _sin = {};
    _sin.sin_family = AF_INET;
    _sin.sin_port = htons(4568); //host to net unsigned short
#ifdef _WIN32
    _sin.sin_addr.S_un.S_addr = INADDR_ANY;
#else
    _sin.sin_addr.s_addr = INADDR_ANY;
#endif
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

    while (true) {
        fd_set fdRead;
        //FD_ZERO 宏定义，为所有fd_set.count置0
        FD_ZERO(&fdRead);
        //FD_SEt 宏定义，将fd数组中的首个元素置为_sock
        FD_SET(_sock, &fdRead);
        //将g_client数组中所有SOCKET放入对应集合
        for (int n = (int)g_client.size() - 1; n >= 0; --n) {
            FD_SET(g_client[n], &fdRead);
        }
        //nfds是一个整数值，是指fd_set集合中（SOCKET）描述符范围
        //nfds在windows中不生效
        //最后一个等待时间,若为NULL,则为select模式
        timeval t = { 0,0 };
        int ret = select(_sock + 1, &fdRead, 0, 0, &t);
        if (ret < 0) {
            std::cout << "select 任务结束..." << std::endl;
            break;
        }
        //判断_sock是否在fdRead集合中
        //_sock是否有accept动作
        if (FD_ISSET(_sock, &fdRead)) {
            FD_CLR(_sock, &fdRead);
            //等待接受客户端请求
            sockaddr_in clientAddr = {};
            int nAddrlen = sizeof(sockaddr_in);
            SOCKET _cSock = INVALID_SOCKET;
#ifdef _WIN32
            _cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrlen);
#else
            _cSock = accept(_sock, (sockaddr*)&clientAddr, (socklen_t*)&nAddrlen);
#endif
            if (INVALID_SOCKET == _cSock) {
                std::cout << "ERROR,接收到错误的客户端..." << std::endl;
            }
            else {
                NewUserJoin userjoin;
                for (size_t i = 0; i < g_client.size(); ++i) {
                    send(g_client[i], (const char*)&userjoin, sizeof(NewUserJoin), 0);
                }
                g_client.push_back(_cSock);
                std::cout << "新客户端加入：socket: " << (int)_cSock << " ,Ip=" << inet_ntoa(clientAddr.sin_addr) << std::endl;
            }
        }

        //检查每个_cSock，是否有recv任务
        //若出现异常，在集合中删除这个socket
        for (size_t i = 0; i < g_client.size(); ++i) {
            if (FD_ISSET(g_client[i], &fdRead)) {
                FD_CLR(g_client[i], &fdRead);
                if (-1 == processor(g_client[i])) {
                    auto it = g_client.begin() + i;
                    if (it != g_client.end())
                        g_client.erase(it);
                }
            }
        }
#ifdef _WIN32
        Sleep(1000);
#else
        sleep(4);
#endif
        std::cout << "服务器处理其他空闲任务..." << std::endl;
    }
#ifdef _WIN32
    //关闭socket
    for (size_t i = 0; i < g_client.size() - 1; ++i) {
        closesocket(g_client[i]);
    }
    //
    WSACleanup();
#else
    for (size_t i = 0; i < g_client.size() - 1; ++i) {
        close(g_client[i]);
    }
#endif
    return 0;
}

int processor(SOCKET _cSock) {
    char szBuff[1024] = {};

    //接收客户端信息
    int nLen = recv(_cSock, szBuff, sizeof(DataHeader), 0);
    DataHeader* header = (DataHeader*)szBuff;

    if (nLen <= 0) {
        std::cout << "接收失败" << std::endl;
        return -1;
    }
    else {
        std::cout << "收到客户端命令: " << header->cmd << " 命令长度： " << header->datalength << std::endl;
    }
    switch (header->cmd) {
    case CMD_LOGIN:
    {
        //类内元素存放位置应该是按照类的声明顺序存放在内存中存放
        recv(_cSock, szBuff + sizeof(DataHeader), sizeof(Login) - sizeof(DataHeader), 0);
        Login* login = (Login*)szBuff;
        std::cout << login->UserName << "  " << login->PassWord << std::endl;
        //处理部分
        LoginResult res;
        res.result = 1;
        send(_cSock, (char*)&res, sizeof(LoginResult), 0);
    }
    break;
    case CMD_LOGOUT:
    {
        recv(_cSock, szBuff + sizeof(DataHeader), sizeof(Logout) - sizeof(DataHeader), 0);
        Logout* logout = (Logout*)szBuff;
        std::cout << logout->UserName << std::endl;
        //忽略判断用户密码是否正确的过程
        LogoutResult ret;
        send(_cSock, (char*)&ret, sizeof(LogoutResult), 0);
    }
    break;
    default:
        header->cmd = CMD_ERROR;
        header->datalength = 0;
        send(_cSock, (char*)&header, sizeof(header), 0);
        break;
    }
    return 1;
}