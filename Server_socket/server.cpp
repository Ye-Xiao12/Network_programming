#ifndef FD_SETSIZE
#define FD_SETSIZE      10001
#endif

#include"EasyTcpServer.hpp"
bool g_bRun = true;
class MyServer : public EasyTcpServer
{
public:
	//只会被一个线程触发 安全
	virtual void OnNetJoin(ClientSocket* pClient);	//增加一个连接
	virtual void OnNetLeave(ClientSocket* pClient);	//减少一个连接
	virtual void OnNetMsg(CellServer* pCellServer, ClientSocket* pClient, DataHeader* header);	//处理一次数据
	virtual void OnNetRecv(ClientSocket* pClient);	//接收一次数据
private:
};
//增加一个连接
void MyServer :: OnNetJoin(ClientSocket* pClient)
{
	std::lock_guard<std::mutex>lock(_mutex);
	_clients.push_back(pClient);
	_clientCount++;
}
//减少一个连接
void MyServer::OnNetLeave(ClientSocket* pClient)
{
	std::lock_guard<std::mutex>lock(_mutex);
	auto it = find(_clients.begin(), _clients.end(), pClient);
	_clients.erase(it);
	_clientCount--;
}
//处理一次数据
void MyServer::OnNetMsg(CellServer* pCellServer, ClientSocket* pClient, DataHeader* header)
{
	_msgCount++;
	switch (header->cmd)
	{
	case CMD_LOGIN:
	{
		//send recv 
		Login* login = (Login*)header;
		LogoutResult *loginResult = new LogoutResult();
		pCellServer->addSendTask(pClient, loginResult);
	}//接收 消息---处理 发送   生产者 数据缓冲区  消费者 
	break;
	case CMD_LOGOUT:
	{
		Logout* logout = (Logout*)header;
	}
	break;
	default:
	{
		printf("<socket=%d>收到未定义消息,数据长度：%lld\n", pClient->sockfd(), header->datalength);
	}
	break;
	}
}
//接收一次数据
void MyServer::OnNetRecv(ClientSocket* pClient)
{
	_recvCount++;
}

int main() {
	MyServer server;
    server.InitSocket();
    server.Bind(NULL, 4568);
    server.Listen(5);
    server.Start(4);    //创建4个处理报文的线程
   
    while (g_bRun) {
        server.OnRun();
    }
    server.Close();
    return 0;
}

