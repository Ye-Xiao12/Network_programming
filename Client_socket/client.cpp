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
void cmdThread(SOCKET _sock);    //�����߳�

//��������
enum CMD {
    CMD_LOGIN,
    CMD_LOGIN_RESULT,
    CMD_LOGOUT,
    CMD_LOGOUT_RESULT,
    CMD_ERROR,
    CMD_NEW_USER_JOIN,
};
//��ͷ
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
//��¼������
struct LoginResult : public DataHeader {
    LoginResult() {
        datalength = sizeof(LoginResult);
        cmd = CMD_LOGIN_RESULT;
    }
    int result;
};
//�ǳ�
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
    //����һ��socket
    SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    //�󶨶˿ں�(�������˿�)
    sockaddr_in _sin = {};
    _sin.sin_family = AF_INET;
    _sin.sin_port = htons(4567);
    _sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
    //���ӷ�����
    int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr));
    if (SOCKET_ERROR == ret) {
        std::cout << "ERRO: ����ʧ��" << std::endl;
    }
    else {
        std::cout << "���ӳɹ�..." << std::endl;
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
            std::cout << "select�������..." << std::endl;
        }
        if (FD_ISSET(_sock, &fdReads)) {
            FD_CLR(_sock, &fdReads);
            if (-1 == processor(_sock)) {
                std::cout << "select �������..." << std::endl;
                break;
            }
        }
        Sleep(1000);
        //std::cout << "����ʱ�䣬����client��������..." << std::endl;
    }
    // �ر�socket
    closesocket(_sock);
    //���windows socket����
    WSACleanup();
    return 0;
}

int processor(SOCKET _sock) {
    //�����˷�������
    char recvBuf[1024];
    int ret = recv(_sock, recvBuf, sizeof(DataHeader),0);
    DataHeader* dh = (DataHeader*)recvBuf;

    if (ret < 0) {
        std::cout << "�յ�exit����������" << std::endl;
        return -1;
    }
    switch (dh->cmd) {
    case CMD_LOGIN_RESULT:
    {
        recv(_sock, recvBuf + sizeof(DataHeader), sizeof(LoginResult) - sizeof(DataHeader), 0);
        LoginResult* login = (LoginResult*)recvBuf;
        std::cout << "���յ��������Ϣ: LoginResult" << std::endl;
    }
        break;
    case CMD_LOGOUT_RESULT:
    {
        recv(_sock, recvBuf + sizeof(DataHeader), sizeof(LogoutResult) - sizeof(DataHeader), 0);
        LogoutResult* logout = (LogoutResult*)recvBuf;
        std::cout << "���յ��������Ϣ: LogoutResult" << std::endl;
    }
        break;
    case CMD_NEW_USER_JOIN:
    {
        recv(_sock, recvBuf + sizeof(DataHeader), sizeof(NewUserJoin) - sizeof(DataHeader), 0);
        NewUserJoin* userJoin = (NewUserJoin*)recvBuf;
        std::cout << "�¼���һ�����..." << std::endl;
    }
        break;
    }
    return 1;
}

//�ͻ��������߳�
void cmdThread(SOCKET _sock) {
    while (true) {
        string cmdBuf;
        std::cout << "���������" << std::endl;
        cin >> cmdBuf;
        if (cmdBuf == "exit") {
            g_bRun = false;
            std::cout << "�Ƴ�cmdThread�߳�..." << std::endl;
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
            std::cout << "��֧�ָ�����..." << std::endl;
        }
    }
}