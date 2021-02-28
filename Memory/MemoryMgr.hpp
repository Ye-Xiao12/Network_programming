#ifndef _MemoryMgr_hpp_
#define _MemoryMgr_hpp_
#include<stdlib.h>
#include<stdio.h>
#include<assert.h>

//�����������
#ifdef _DEBUG
	#define xPrintf(...) printf(__VA_ARGS__)
#else
	#define xPrintf(...)
#endif

#define MAX_MEMORY_SZIE 64	//�ڴ���ܹ��������ڴ���С

class MemoryAlloc;

//�ڴ���ʶ�࣬��¼��Ƭ�ڴ����Ϣ
class MemoryBlock
{
public:
	MemoryBlock();	//���캯��,��ʼ����Ա����

	MemoryAlloc* pAlloc;	//�������ڴ�飨�أ�
	MemoryBlock* pNext;	//��һ��λ��
	int nID;	//�ڴ����
	int nRef;	//�����ô���
	bool bPool;	//�Ƿ����ڴ����
private:
};
//���캯��,��ʼ����Ա����
MemoryBlock::MemoryBlock() {	
	nID = 0;
	nRef = 0;
	bPool = true;
	pAlloc = nullptr;
	pNext = nullptr;
}
//const int MemoryBlockSize = sizeof(MemoryBlock);

//�ڴ��
class MemoryAlloc
{
public:
	MemoryAlloc();
	~MemoryAlloc();

	void* allocMemory(size_t nSize);	//�����ڴ�
	void freeMemory(void* pMem);	//�ͷ��ڴ�
	void initMemory();	//��ʼ��
protected:
	char* _pBuf;	//�ڴ�ص�ַ
	MemoryBlock* _pHeader;	//ͷ���ڴ浥Ԫ
	size_t _nSize;	//�ڴ浥Ԫ�Ĵ�С
	size_t _nBlockSzie;	//�ڴ浥Ԫ������
};
//���캯��
MemoryAlloc::MemoryAlloc()
{
	_pBuf = nullptr;
	_pHeader = nullptr;
	_nSize = 0;
	_nBlockSzie = 0;
}
//��������
MemoryAlloc::~MemoryAlloc()
{
	if (_pBuf) {	
		free(_pBuf);
	}
}
//�����ڴ�
void* MemoryAlloc::allocMemory(size_t nSize)
{
	if (!_pBuf)
	{
		initMemory();
	}

	MemoryBlock* pReturn = nullptr;
	if (nullptr == _pHeader)	//�ڴ���е��ڴ���Ѿ���������
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
//�ͷ��ڴ�
void MemoryAlloc::freeMemory(void* pMem)
{
	MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));	//MemoryBlock�ڷ����ڴ��ǰ�棬���ָ��Ҫǰ��
	assert(1 == pBlock->nRef);
	//���ÿ��ڴ��ɶ��ָ�빲����ֻ�轫���������ȥ1����
	if (--pBlock->nRef != 0)
	{
		return;
	}
	
	if (pBlock->bPool)
	{
		//������ջ����ʽ�����ͷŵ��ڴ浱��ͷ���ڴ浥Ԫ�Ż��ڴ��
		pBlock->pNext = _pHeader;
		_pHeader = pBlock;
	}
	else {
		//�������ڴ�ص��ڴ棬ֱ���ͷ�
		free(pBlock);
	}
}
//�ڴ�س�ʼ��
void MemoryAlloc::initMemory()
{	//����
	assert(nullptr == _pBuf);

	if (_pBuf)
		return;	//�Ѿ���ʼ����ֱ���˳�
	//�����ڴ�صĴ�С
	size_t bufSize = (_nSize + sizeof(MemoryBlock)) * _nBlockSzie;
	//��ϵͳ����ص��ڴ�
	_pBuf = (char*)malloc(bufSize);

	//��ʼ���ڴ��
	_pHeader = (MemoryBlock*)_pBuf;
	_pHeader->bPool = true;
	_pHeader->nID = 0;
	_pHeader->nRef = 0;
	_pHeader->pAlloc = this;
	_pHeader->pNext = nullptr;
	//�����ڴ����г�ʼ��
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

//������������Ա����ʱ��ʼ��MemoryAlloc�ĳ�Ա����
template<size_t nSzie, size_t nBlockSzie>
class MemoryAlloctor :public MemoryAlloc
{
public:
	MemoryAlloctor()
	{
		//����64λϵͳ��32λϵͳ�������ڴ�ֱ���8B��4B����
		const size_t n = sizeof(void*);

		_nSize = (nSzie / n) * n + (nSzie % n ? n : 0);
		_nBlockSzie = nBlockSzie;
	}
};

//�ڴ������
class MemoryMgr
{
private:
	MemoryMgr() {
		init(0, 64, &_mem64);
	}
	~MemoryMgr() {}	//˽�л��������������
public:
	static MemoryMgr& Instance();	//����ģʽ��ʹ�þ�̬��Ա����һ����̬�����
	void* allocMem(size_t nSize);	//�����ڴ�
	void freeMem(void* pMem);	//�ͷ��ڴ�
	void addRef(void* pMem);	//�����ڴ������ü���
private:
	void init(int nBegin, int nEnd, MemoryAlloc* pMemA);	//��ʼ��_szAllocӳ������
private:
	MemoryAlloctor<64, 10>_mem64;
	MemoryAlloc* _szAlloc[MAX_MEMORY_SZIE + 1];	//�Դ���Ĵ�С��ӳ�䵽��Ӧ�ڴ���������ڴ��
};

//����ģʽ��ʹ�þ�̬��Ա����һ����̬�����
MemoryMgr& MemoryMgr::Instance()
{//����ģʽ ��̬
	static MemoryMgr mgr;
	return mgr;
}
//��ʼ��_szAllocӳ������
void MemoryMgr::init(int nBegin, int nEnd, MemoryAlloc* pMemA)
{
	for (int i = nBegin; i <= nEnd; ++i)
	{
		_szAlloc[i] = pMemA;
	}
}
//�����ڴ�
//���������ڴ���С������MAX_MEMORY_SIZE������ϵͳ��malloc()�����ڴ�
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
//�ͷ��ڴ�
void MemoryMgr::freeMem(void* pMem)
{
	MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));
	xPrintf("freeMem: %llx, id=%d\n", pBlock, pBlock->nID);
	if (pBlock->bPool)	//���ڴ������ڴ�ط����
	{
		pBlock->pAlloc->freeMemory(pMem);
	}
	else
	{
		if (--pBlock->nRef == 0)
			free(pBlock);
	}
}
//�����ڴ������ü���
void MemoryMgr::addRef(void* pMem) {
	MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));
	++pBlock->nRef;
}

#endif // !_MemoryMgr_hpp_
