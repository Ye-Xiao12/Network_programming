#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS 
#include<windows.h>
#include<WinSock2.h>
#include<iostream>
#include<string>
#include<string.h>
using namespace std;

//��������
enum CMD {
    CMD_LOGIN,
    CMD_LOGIN_RESULT,
    CMD_LOGOUT,
    CMD_LOGOUT_RESULT,
    CMD_ERROR,
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

int main() {
    WORD var = MAKEWORD(2, 3);
    WSADATA dat;
    WSAStartup(var, &dat);
    //
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

    while (true) {
        //�����˷�������
        std::string cmdBuf;
        std::cout << "����������";
        std::cin >> cmdBuf;

        if (cmdBuf == "exit") {
            std::cout << "�յ�exit����������" << std::endl;
        }
        else if (cmdBuf == "login") {
            Login login;
            strcpy(login.UserName, "XiaoMing");
            strcpy(login.PassWord, "12345678");
            //���������������
            send(_sock, (const char*)&login, login.datalength, 0);
            //���շ�������������
            LoginResult loginResult;
            recv(_sock, (char*)&loginResult, sizeof(LoginResult), 0);
            std::cout << loginResult.result << std::endl;
        }
        else if (cmdBuf == "logout") {
            Logout logout;
            strcpy(logout.UserName,"���ڹ�");
            //�������������������
            send(_sock, (char*)&logout, logout.datalength, 0);
            //���շ���������
            LogoutResult logoutRet;
            recv(_sock, (char*)&logoutRet, sizeof(LogoutResult), 0);
            std::cout << logoutRet.result << std::endl;
        }
        else {
            std::cout << "��֧��������������룺" << std::endl;
        }
    }
    // �ر�socket
    closesocket(_sock);
    //���windows socket����
    WSACleanup();
    return 0;
}