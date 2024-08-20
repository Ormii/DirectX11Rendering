#include "pch.h"
#include "MemoryPool.h"

MemoryManager::MemoryManager()
{
    uint32 offset = MemoryManager::MemoryAlign*4;
    for (int i = 0; i <= MemoryManager::MaxPoolLevel; ++i)
    {
        uint32 allocSize = offset * (int32)pow(2, i);
        m_pool[i] = new MemoryPool(allocSize);
    }
}

MemoryManager::~MemoryManager()
{
    for (int i = 0; i <= MemoryManager::MaxPoolLevel; ++i)
        delete m_pool[i];
}

void* MemoryManager::Allocate(uint32 size)
{
    if (size <= 0)
        return nullptr;

    uint32 offset = MemoryManager::MemoryAlign * 4;
    MemoryHeader* header = nullptr;

    uint32 actualSize = sizeof(MemoryHeader) + size;
    
    for (int i = 0; i <= MemoryManager::MaxPoolLevel; ++i)
    {
        uint32 allocSize = offset * (int32)pow(2, i);
        if (actualSize <= allocSize)
        {
            header = m_pool[i]->Pop();
            break;
        }
    }

    if (header == nullptr)
        header = static_cast<MemoryHeader*>(_aligned_malloc(actualSize, MemoryManager::MemoryAlign));
    
    return MemoryHeader::AttachHeader(header, actualSize);
}

void MemoryManager::Release(void* ptr)
{
    if (ptr == nullptr)
        return;

    MemoryHeader* header = MemoryHeader::DetachHeader(ptr);

    uint32 size = header->GetAllocSize();
    uint32 offset = MemoryManager::MemoryAlign * 4;
    for (int i = 0; i <= MemoryManager::MaxPoolLevel; ++i)
    {
        uint32 allocSize = offset * (int32)pow(2, i);
        if (size <= allocSize)
        {
            m_pool[i]->Push(header);
            return;
        }
    }

    if (header != nullptr)
        _aligned_free(header);
    
    return;
}
