#ifndef _ALLOCTOR_H_
#define _ALLOCTOR_H_

//����new ��new[]�����
void* operator new(size_t size);
void operator delete(void* p);
void* operator new[](size_t size);
void operator delete[](void* p);

//�ڴ���亯������
void* mem_alloc(size_t size);
void mem_free(void* p);

#endif
