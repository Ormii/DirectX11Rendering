#pragma once
#include "Allocator.h"

class MemoryPool;

class MemoryManager
{
public:
	MemoryManager();
	~MemoryManager();

public:
	constexpr static uint32 MemoryAlign = 16;
	constexpr static uint32 MaxPoolLevel = 7;

public:
	void* Allocate(uint32 size);
	void Release(void* ptr);

private:
	MemoryPool* m_pool[MaxPoolLevel+1];
};

template <typename Type, typename... Args>
Type* customnew(Args&&... args)
{
	Type* memory = static_cast<Type*>(PoolAllocator::Alloc(sizeof(Type)));
	new(memory)Type(args...);

	return memory;
}
template <typename Type>
void customdelete(Type* obj)
{
	if (obj == nullptr)
		return;

	obj->~Type();
	PoolAllocator::Release(obj);
}

template <typename Type, typename... Args>
shared_ptr<Type> MakeShared(Args... args)
{
	shared_ptr<Type> ptr = { customnew<Type>(forward<Args>(args)...),  customdelete<Type> };
	return ptr;
}