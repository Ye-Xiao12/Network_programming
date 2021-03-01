#include"Alloctor.h"
#include"MemoryMgr.hpp"

//����new������
void* operator new(size_t nSize)
{
	return MemoryMgr::Instance().allocMem(nSize);
}
//����delete������
void operator delete(void* p)
{
	MemoryMgr::Instance().freeMem(p);
}
//����new[]������
void* operator new[](size_t nSize)
{
	return MemoryMgr::Instance().allocMem(nSize);
}
//����delete[]������
void operator delete[](void* p)
{
	MemoryMgr::Instance().freeMem(p);
}
//�����ڴ���亯��
void* mem_alloc(size_t size)
{
	return malloc(size);
}
//�����ڴ��ͷź���
void mem_free(void* p)
{
	free(p);
}
