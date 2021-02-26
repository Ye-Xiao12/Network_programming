#ifndef FD_SETSIZE
#define FD_SETSIZE      10001
#endif

#include"EasyTcpServer.hpp"
bool g_bRun = true;
class MyServer : public EasyTcpServer
{
public:
	//ֻ�ᱻһ���̴߳��� ��ȫ
	virtual void OnNetJoin(ClientSocket* pClient);	//����һ������
	virtual void OnNetLeave(ClientSocket* pClient);	//����һ������
	virtual void OnNetMsg(CellServer* pCellServer, ClientSocket* pClient, DataHeader* header);	//����һ������
	virtual void OnNetRecv(ClientSocket* pClient);	//����һ������
private:
};
//����һ������
void MyServer :: OnNetJoin(ClientSocket* pClient)
{
	std::lock_guard<std::mutex>lock(_mutex);
	_clients.push_back(pClient);
	_clientCount++;
}
//����һ������
void MyServer::OnNetLeave(ClientSocket* pClient)
{
	std::lock_guard<std::mutex>lock(_mutex);
	auto it = find(_clients.begin(), _clients.end(), pClient);
	_clients.erase(it);
	_clientCount--;
}
//����һ������
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
	}//���� ��Ϣ---���� ����   ������ ���ݻ�����  ������ 
	break;
	case CMD_LOGOUT:
	{
		Logout* logout = (Logout*)header;
	}
	break;
	default:
	{
		printf("<socket=%d>�յ�δ������Ϣ,���ݳ��ȣ�%lld\n", pClient->sockfd(), header->datalength);
	}
	break;
	}
}
//����һ������
void MyServer::OnNetRecv(ClientSocket* pClient)
{
	_recvCount++;
}

int main() {
	MyServer server;
    server.InitSocket();
    server.Bind(NULL, 4568);
    server.Listen(5);
    server.Start(4);    //����4�������ĵ��߳�
   
    while (g_bRun) {
        server.OnRun();
    }
    server.Close();
    return 0;
}

