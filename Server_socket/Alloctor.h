#ifndef _ALLOCTOR_H_
#define _ALLOCTOR_H_

//重载new 和new[]运算符
void* operator new(size_t size);
void operator delete(void* p);
void* operator new[](size_t size);
void operator delete[](void* p);

//内存分配函数申明
void* mem_alloc(size_t size);
void mem_free(void* p);

#endif
