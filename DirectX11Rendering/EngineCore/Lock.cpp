#include "pch.h"
#include "Lock.h"

void Lock::WriteLock()
{
	const uint32 CurrentWriteThread = (LockFlag.load() & LOCK_MASK_WRITE_THREAD_ID)>>16;
	if (CurrentWriteThread == LthreadID)
	{
		WriteCount++;
		return;
	}

	uint64 prevTick = GetTickCount64();
	while (true)
	{
		uint32 desired = ((LthreadID << 16)& LOCK_MASK_WRITE_THREAD_ID);

		for (uint32 SpinCount = 0; SpinCount < Lock::MaxSpinCount; SpinCount++)
		{
			uint32 expected = EMPTY_MASK;
			if (LockFlag.compare_exchange_strong(expected, desired))
			{
				WriteCount++;
				return;
			}
		}

		uint64 currentTick = GetTickCount64();

		if (currentTick - prevTick >= Lock::TimeoutTick)
		{
			CRASH("WRITELOCK TIMEOUT");
		}

		this_thread::yield();
	}
}

void Lock::WriteUnlock()
{
	if ((LockFlag.load() & LOCK_MASK_READ_THREAD_COUNT) != 0)
		CRASH("INVALID_UNLOCK_ORDER");

	const int32 lockCount = --WriteCount;
	if (lockCount == 0)
	{
		LockFlag.store(EMPTY_MASK);
	}
}

void Lock::ReadLock()
{
	const uint32 CurrentWriteThread = (LockFlag.load() & LOCK_MASK_WRITE_THREAD_ID) >> 16;
	if (CurrentWriteThread == LthreadID)
	{
		LockFlag.fetch_add(1);
		return;
	}

	uint64 prevTick = GetTickCount64();
	while (true)
	{
		for (uint32 SpinCount = 0; SpinCount < Lock::MaxSpinCount; ++SpinCount)
		{
			uint32 expected = (LockFlag.load() & LOCK_MASK_READ_THREAD_COUNT);
			uint32 desired = expected + 1;
			if (LockFlag.compare_exchange_strong(expected, desired))
			{
				return;
			}
		}

		uint64 currentTick = GetTickCount64();

		if (currentTick - prevTick > Lock::TimeoutTick)
		{
			CRASH("READ LOCK TIMEOUT");
		}

		this_thread::yield();
	}
}

void Lock::ReadUnLock()
{
	uint32 prevReadThreadCount = static_cast<uint32>(LockFlag.fetch_sub(1));
	if ((prevReadThreadCount & LOCK_MASK_READ_THREAD_COUNT) == 0)
		CRASH("DUPLICATE READ UNLOCK");

}
