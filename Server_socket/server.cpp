#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<windows.h>
#include<WinSock2.h>
#include<string.h>
#include<iostream>
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
struct LoginResult : public DataHeader{
    LoginResult() {
        datalength = sizeof(LoginResult);
        cmd = CMD_LOGIN_RESULT;
    }
    int result;
};
//�ǳ�
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

    //����һ��socket�׽���
    SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    //�󶨶˿ں�
    sockaddr_in _sin = {};
    _sin.sin_family = AF_INET;
    _sin.sin_port = htons(4567); //host to net unsigned short
    _sin.sin_addr.S_un.S_addr = INADDR_ANY;
    if (SOCKET_ERROR == bind(_sock, (sockaddr*)&_sin, sizeof(_sin)))
    {
        std::cout << "ERROR: �����ڽ��ܿͻ���ʧ��" << std::endl;
    }
    else {
        std::cout << "�����ڽ��ܿͻ��˳ɹ�" << std::endl;
    }
    //3. listen ��������˿�
    if (SOCKET_ERROR == listen(_sock, 5))
    {
        std::cout << "ERROR: ��������˿�ʧ��..." << std::endl;
    }
    else {
        std::cout << "��������˿ڳɹ�.." << std::endl;
    }
    //accept �ȴ����ܿͻ�������
    sockaddr_in cAddr = {};
    SOCKET _cSock = INVALID_SOCKET;
    int nAddrLen = sizeof(sockaddr_in);
    _cSock = accept(_sock, (sockaddr*)&cAddr, &nAddrLen);
    if (INVALID_SOCKET == _cSock) {
        std::cout << "���󣬽��ܵ���Ч�ͻ���SOCKET..." << std::endl;
    }
    else {
        std::cout << "�¿ͻ��˼��룬Socket: " << (int)_cSock << " IP: " << inet_ntoa(cAddr.sin_addr) << std::endl;
    }

    while (true) {
        DataHeader header = {};
        //���տͻ�����Ϣ
        int nLen = recv(_cSock, (char*)&header, sizeof(DataHeader), 0);
        if (nLen <= 0) {
            std::cout << "����ʧ��" << std::endl;
        }
        else {
            std::cout << "�յ��ͻ�������: " << header.cmd << " ����ȣ� " << header.datalength << std::endl;
        }
        switch (header.cmd) {
            case CMD_LOGIN: 
            {
                Login login ;
                recv(_cSock, (char*)&login + sizeof(DataHeader), sizeof(Login)-sizeof(DataHeader), 0);
                std::cout << login.UserName << "  "<< login.PassWord << std::endl;
                //������
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
                //�����ж��û������Ƿ���ȷ�Ĺ���
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

    //�ر�socket
    closesocket(_sock);
    //
    WSACleanup();
    return 0;
}