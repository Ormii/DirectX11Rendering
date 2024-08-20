#include "pch.h"
#include "Allocator.h"

void* PoolAllocator::Alloc(int32 size)
{
    if (size <= 0)
        return nullptr;
    
    return g_MemoryManager->Allocate(size);
}

void PoolAllocator::Release(void* ptr)
{
    g_MemoryManager->Release(ptr);
}
