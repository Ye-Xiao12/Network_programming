#include<string.h>
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
