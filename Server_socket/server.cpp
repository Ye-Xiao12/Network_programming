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
#ifdef _WIN32
    WORD var = MAKEWORD(2, 3);
    WSADATA dat;
    WSAStartup(var, &dat);
#endif
    //����һ��socket�׽���
    SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    //�󶨶˿ں�
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

    while (true) {
        fd_set fdRead;
        //FD_ZERO �궨�壬Ϊ����fd_set.count��0
        FD_ZERO(&fdRead);
        //FD_SEt �궨�壬��fd�����е��׸�Ԫ����Ϊ_sock
        FD_SET(_sock, &fdRead);
        //��g_client����������SOCKET�����Ӧ����
        for (int n = (int)g_client.size() - 1; n >= 0; --n) {
            FD_SET(g_client[n], &fdRead);
        }
        //nfds��һ������ֵ����ָfd_set�����У�SOCKET����������Χ
        //nfds��windows�в���Ч
        //���һ���ȴ�ʱ��,��ΪNULL,��Ϊselectģʽ
        timeval t = { 0,0 };
        int ret = select(_sock + 1, &fdRead, 0, 0, &t);
        if (ret < 0) {
            std::cout << "select �������..." << std::endl;
            break;
        }
        //�ж�_sock�Ƿ���fdRead������
        //_sock�Ƿ���accept����
        if (FD_ISSET(_sock, &fdRead)) {
            FD_CLR(_sock, &fdRead);
            //�ȴ����ܿͻ�������
            sockaddr_in clientAddr = {};
            int nAddrlen = sizeof(sockaddr_in);
            SOCKET _cSock = INVALID_SOCKET;
#ifdef _WIN32
            _cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrlen);
#else
            _cSock = accept(_sock, (sockaddr*)&clientAddr, (socklen_t*)&nAddrlen);
#endif
            if (INVALID_SOCKET == _cSock) {
                std::cout << "ERROR,���յ�����Ŀͻ���..." << std::endl;
            }
            else {
                NewUserJoin userjoin;
                for (size_t i = 0; i < g_client.size(); ++i) {
                    send(g_client[i], (const char*)&userjoin, sizeof(NewUserJoin), 0);
                }
                g_client.push_back(_cSock);
                std::cout << "�¿ͻ��˼��룺socket: " << (int)_cSock << " ,Ip=" << inet_ntoa(clientAddr.sin_addr) << std::endl;
            }
        }

        //���ÿ��_cSock���Ƿ���recv����
        //�������쳣���ڼ�����ɾ�����socket
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
        std::cout << "����������������������..." << std::endl;
    }
#ifdef _WIN32
    //�ر�socket
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

    //���տͻ�����Ϣ
    int nLen = recv(_cSock, szBuff, sizeof(DataHeader), 0);
    DataHeader* header = (DataHeader*)szBuff;

    if (nLen <= 0) {
        std::cout << "����ʧ��" << std::endl;
        return -1;
    }
    else {
        std::cout << "�յ��ͻ�������: " << header->cmd << " ����ȣ� " << header->datalength << std::endl;
    }
    switch (header->cmd) {
    case CMD_LOGIN:
    {
        //����Ԫ�ش��λ��Ӧ���ǰ����������˳�������ڴ��д��
        recv(_cSock, szBuff + sizeof(DataHeader), sizeof(Login) - sizeof(DataHeader), 0);
        Login* login = (Login*)szBuff;
        std::cout << login->UserName << "  " << login->PassWord << std::endl;
        //������
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
        //�����ж��û������Ƿ���ȷ�Ĺ���
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