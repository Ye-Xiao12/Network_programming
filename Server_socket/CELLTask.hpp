#ifndef _CELL_TASK_H_

#include<thread>
#include<mutex>
#include<functional>
#include<list>

//��������-����
class CellTask
{
public:
	//����
	CellTask(){}
	//������
	virtual ~CellTask(){}
	//ִ������
	virtual void doTask(){}
private:

};

//ִ������ķ�������
class CellTaskServer
{
private:
	std::list<CellTask*> _tasks; //��������
	std::list<CellTask*> _tasksBuf;	//�������ݻ�����
	std::mutex _mutex;	//�ı����ݻ�����ʱ��Ҫ����
public:
	void addTask(CellTask* task); //�������
	void Start();	//���������߳�
protected:
	void OnRun();	//��������
};
//�������
void CellTaskServer::addTask(CellTask* task)
{
	std::lock_guard<std::mutex> lock(_mutex);
	_tasksBuf.push_back(task);
}
//���������߳�
void CellTaskServer::Start()
{
	//�߳�
	std::thread t(std::mem_fn(&CellTaskServer::OnRun), this);
	t.detach();
}
//��������
void CellTaskServer::OnRun()
{
	while (true)
	{
		//�ӻ�����ȡ������
		if (!_tasksBuf.empty())
		{
			std::lock_guard<std::mutex> lock(_mutex);
			for (auto pTask : _tasksBuf)
			{
				_tasks.push_back(pTask);
			}
			_tasksBuf.clear();
		}
		//���û������
		if (_tasks.empty())
		{
			std::chrono::milliseconds t(1);
			std::this_thread::sleep_for(t);
			continue;
		}
		//��������
		for (auto pTask : _tasks)
		{
			pTask->doTask();
			delete pTask;
		}
		//�������
		_tasks.clear();
	}
}

#endif // !_CELL_TASK_H_
