#pragma once


DECLSPEC_ALIGN(MemoryManager::MemoryAlign)
struct MemoryHeader : public SLIST_ENTRY
{
public:
	MemoryHeader(int32 allocSize) : m_allocsize(allocSize){}


public:
	static void* AttachHeader(MemoryHeader* header, int32 size)
	{
		new(header)MemoryHeader(size);
		return reinterpret_cast<void*>(++header);
	}
	static MemoryHeader* DetachHeader(void* ptr)
	{
		return reinterpret_cast<MemoryHeader*>(ptr) - 1;
	}

public:
	int32 GetAllocSize() { return m_allocsize; }

private:
	int32 m_allocsize;
};

DECLSPEC_ALIGN(MemoryManager::MemoryAlign)
class MemoryPool
{
public:
	MemoryPool(int32 size);
	~MemoryPool();
public:
	void Push(MemoryHeader* header);
	MemoryHeader* Pop();

private:
	int32 m_allocSize;
	SLIST_HEADER m_header;
};