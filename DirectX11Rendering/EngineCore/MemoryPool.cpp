#include "pch.h"
#include "MemoryPool.h"

MemoryPool::MemoryPool(int32 size)
	:m_allocSize(size)
{
	InitializeSListHead(&m_header);
}

MemoryPool::~MemoryPool()
{
	while (true)
	{
		MemoryHeader* header = static_cast<MemoryHeader*>(InterlockedPopEntrySList(&m_header));
		if (header == nullptr)
			break;
		
		_aligned_free(header);
	}
}

void MemoryPool::Push(MemoryHeader* header)
{
	InterlockedPushEntrySList(&m_header, static_cast<PSLIST_ENTRY>(header));
}

MemoryHeader* MemoryPool::Pop()
{
	MemoryHeader* header = static_cast<MemoryHeader*>(InterlockedPopEntrySList(&m_header));

	if (header == nullptr)
	{
		header = static_cast<MemoryHeader*>(_aligned_malloc(m_allocSize, MemoryManager::MemoryAlign));
	}

	return header;
}
