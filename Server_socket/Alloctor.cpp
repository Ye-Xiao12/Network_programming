#include"Alloctor.h"
#include"MemoryMgr.hpp"

//重载new操作符
void* operator new(size_t nSize)
{
	return MemoryMgr::Instance().allocMem(nSize);
}
//重载delete操作符
void operator delete(void* p)
{
	MemoryMgr::Instance().freeMem(p);
}
//重载new[]操作符
void* operator new[](size_t nSize)
{
	return MemoryMgr::Instance().allocMem(nSize);
}
//重载delete[]操作符
void operator delete[](void* p)
{
	MemoryMgr::Instance().freeMem(p);
}
//定义内存分配函数
void* mem_alloc(size_t size)
{
	return malloc(size);
}
//定义内存释放函数
void mem_free(void* p)
{
	free(p);
}
