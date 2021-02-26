#ifndef _CELL_TASK_H_

#include<thread>
#include<mutex>
#include<functional>
#include<list>

//任务类型-基类
class CellTask
{
public:
	//析构
	CellTask(){}
	//虚析构
	virtual ~CellTask(){}
	//执行任务
	virtual void doTask(){}
private:

};

//执行任务的服务类型
class CellTaskServer
{
private:
	std::list<CellTask*> _tasks; //任务数据
	std::list<CellTask*> _tasksBuf;	//任务数据缓冲区
	std::mutex _mutex;	//改变数据缓冲区时需要加锁
public:
	void addTask(CellTask* task); //添加任务
	void Start();	//启动工作线程
protected:
	void OnRun();	//工作函数
};
//添加任务
void CellTaskServer::addTask(CellTask* task)
{
	std::lock_guard<std::mutex> lock(_mutex);
	_tasksBuf.push_back(task);
}
//启动工作线程
void CellTaskServer::Start()
{
	//线程
	std::thread t(std::mem_fn(&CellTaskServer::OnRun), this);
	t.detach();
}
//工作函数
void CellTaskServer::OnRun()
{
	while (true)
	{
		//从缓冲区取出数据
		if (!_tasksBuf.empty())
		{
			std::lock_guard<std::mutex> lock(_mutex);
			for (auto pTask : _tasksBuf)
			{
				_tasks.push_back(pTask);
			}
			_tasksBuf.clear();
		}
		//如果没有任务
		if (_tasks.empty())
		{
			std::chrono::milliseconds t(1);
			std::this_thread::sleep_for(t);
			continue;
		}
		//处理任务
		for (auto pTask : _tasks)
		{
			pTask->doTask();
			delete pTask;
		}
		//清空任务
		_tasks.clear();
	}
}

#endif // !_CELL_TASK_H_
