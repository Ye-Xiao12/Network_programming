#ifndef _MemoryMgr_hpp_
#define _MemoryMgr_hpp_
#include<stdlib.h>
#include<stdio.h>
#include<assert.h>

//用作调试输出
#ifdef _DEBUG
	#define xPrintf(...) printf(__VA_ARGS__)
#else
	#define xPrintf(...)
#endif

#define MAX_MEMORY_SZIE 64	//内存池能管理的最大内存块大小

class MemoryAlloc;

//内存块标识类，记录这片内存的信息
class MemoryBlock
{
public:
	MemoryBlock();	//构造函数,初始化成员变量

	MemoryAlloc* pAlloc;	//所属大内存块（池）
	MemoryBlock* pNext;	//下一块位置
	int nID;	//内存块编号
	int nRef;	//被引用次数
	bool bPool;	//是否在内存池中
private:
};
//构造函数,初始化成员变量
MemoryBlock::MemoryBlock() {	
	nID = 0;
	nRef = 0;
	bPool = true;
	pAlloc = nullptr;
	pNext = nullptr;
}
//const int MemoryBlockSize = sizeof(MemoryBlock);

//内存池
class MemoryAlloc
{
public:
	MemoryAlloc();
	~MemoryAlloc();

	void* allocMemory(size_t nSize);	//申请内存
	void freeMemory(void* pMem);	//释放内存
	void initMemory();	//初始化
protected:
	char* _pBuf;	//内存池地址
	MemoryBlock* _pHeader;	//头部内存单元
	size_t _nSize;	//内存单元的大小
	size_t _nBlockSzie;	//内存单元的数量
};
//构造函数
MemoryAlloc::MemoryAlloc()
{
	_pBuf = nullptr;
	_pHeader = nullptr;
	_nSize = 0;
	_nBlockSzie = 0;
}
//析构函数
MemoryAlloc::~MemoryAlloc()
{
	if (_pBuf) {	
		free(_pBuf);
	}
}
//申请内存
void* MemoryAlloc::allocMemory(size_t nSize)
{
	if (!_pBuf)
	{
		initMemory();
	}

	MemoryBlock* pReturn = nullptr;
	if (nullptr == _pHeader)	//内存池中的内存池已经分配完了
	{
		pReturn = (MemoryBlock*)malloc(nSize + sizeof(MemoryBlock));
		pReturn->bPool = false;
		pReturn->nID = -1;
		pReturn->nRef = 0;
		pReturn->pAlloc = this;
		pReturn->pNext = nullptr;
	}
	else {
		pReturn = _pHeader;
		_pHeader = _pHeader->pNext;
		assert(0 == pReturn->nRef);
		pReturn->nRef = 1;
	}
	xPrintf("allocMem: %llx, id=%d, size=%d\n", pReturn, pReturn->nID, nSize);
	return ((char *)pReturn + sizeof(MemoryBlock));
}
//释放内存
void MemoryAlloc::freeMemory(void* pMem)
{
	MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));	//MemoryBlock在分配内存的前面，因此指针要前移
	assert(1 == pBlock->nRef);
	//若该块内存由多个指针共享，则只需将共享计数减去1即可
	if (--pBlock->nRef != 0)
	{
		return;
	}
	
	if (pBlock->bPool)
	{
		//类似于栈的形式，将释放的内存当做头部内存单元放回内存池
		pBlock->pNext = _pHeader;
		_pHeader = pBlock;
	}
	else {
		//不属于内存池的内存，直接释放
		free(pBlock);
	}
}
//内存池初始化
void MemoryAlloc::initMemory()
{	//断言
	assert(nullptr == _pBuf);

	if (_pBuf)
		return;	//已经初始化，直接退出
	//计算内存池的大小
	size_t bufSize = (_nSize + sizeof(MemoryBlock)) * _nBlockSzie;
	//向系统申请池的内存
	_pBuf = (char*)malloc(bufSize);

	//初始化内存池
	_pHeader = (MemoryBlock*)_pBuf;
	_pHeader->bPool = true;
	_pHeader->nID = 0;
	_pHeader->nRef = 0;
	_pHeader->pAlloc = this;
	_pHeader->pNext = nullptr;
	//遍历内存块进行初始化
	MemoryBlock* pTemp1 = _pHeader;
	for (size_t n = 1; n < _nBlockSzie; n++)
	{
		MemoryBlock* pTemp2 = (MemoryBlock*)(_pBuf + (n * (_nSize + sizeof(MemoryBlock))));
		pTemp2->bPool = true;
		pTemp2->nID = n;
		pTemp2->nRef = 0;
		pTemp2->pAlloc = this;
		pTemp2->pNext = nullptr;
		pTemp1->pNext = pTemp2;
		pTemp1 = pTemp2;
	}
}

//便于在声明成员变量时初始化MemoryAlloc的成员数据
template<size_t nSzie, size_t nBlockSzie>
class MemoryAlloctor :public MemoryAlloc
{
public:
	MemoryAlloctor()
	{
		//对于64位系统和32位系统，分配内存分别向8B和4B对齐
		const size_t n = sizeof(void*);

		_nSize = (nSzie / n) * n + (nSzie % n ? n : 0);
		_nBlockSzie = nBlockSzie;
	}
};

//内存管理工具
class MemoryMgr
{
private:
	MemoryMgr() {
		init(0, 64, &_mem64);
	}
	~MemoryMgr() {}	//私有化构造和析构函数
public:
	static MemoryMgr& Instance();	//单例模式，使用静态成员返回一个静态类对象
	void* allocMem(size_t nSize);	//申请内存
	void freeMem(void* pMem);	//释放内存
	void addRef(void* pMem);	//增加内存块的引用计数
private:
	void init(int nBegin, int nEnd, MemoryAlloc* pMemA);	//初始化_szAlloc映射数组
private:
	MemoryAlloctor<64, 10>_mem64;
	MemoryAlloc* _szAlloc[MAX_MEMORY_SZIE + 1];	//对传入的大小，映射到对应内存池来分配内存块
};

//单例模式，使用静态成员返回一个静态类对象
MemoryMgr& MemoryMgr::Instance()
{//单例模式 静态
	static MemoryMgr mgr;
	return mgr;
}
//初始化_szAlloc映射数组
void MemoryMgr::init(int nBegin, int nEnd, MemoryAlloc* pMemA)
{
	for (int i = nBegin; i <= nEnd; ++i)
	{
		_szAlloc[i] = pMemA;
	}
}
//申请内存
//如果申请的内存块大小超出了MAX_MEMORY_SIZE，调用系统的malloc()分配内存
void* MemoryMgr::allocMem(size_t nSize)
{
	if (nSize <= MAX_MEMORY_SZIE)
	{
		return _szAlloc[nSize]->allocMemory(nSize);
	}
	else
	{
		MemoryBlock* pReturn = (MemoryBlock*)malloc(nSize + sizeof(MemoryBlock));
		pReturn->bPool = false;
		pReturn->nID = -1;
		pReturn->nRef = 1;
		pReturn->pAlloc = nullptr;
		pReturn->pNext = nullptr;
		xPrintf("allocMem: %llx, id=%d, size=%d\n", pReturn, pReturn->nID, nSize);
		return ((char*)pReturn + sizeof(MemoryBlock));
	}
}
//释放内存
void MemoryMgr::freeMem(void* pMem)
{
	MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));
	xPrintf("freeMem: %llx, id=%d\n", pBlock, pBlock->nID);
	if (pBlock->bPool)	//该内存是由内存池分配的
	{
		pBlock->pAlloc->freeMemory(pMem);
	}
	else
	{
		if (--pBlock->nRef == 0)
			free(pBlock);
	}
}
//增加内存块的引用计数
void MemoryMgr::addRef(void* pMem) {
	MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));
	++pBlock->nRef;
}

#endif // !_MemoryMgr_hpp_
