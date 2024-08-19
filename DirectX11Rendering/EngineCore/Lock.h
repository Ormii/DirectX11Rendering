#pragma once

class Lock
{
	enum LockMask : uint32
	{
		LOCK_MASK_WRITE_THREAD_ID = 0xFFFF'0000,
		LOCK_MASK_READ_THREAD_COUNT = 0x0000'FFFF,
		EMPTY_MASK = 0x00000000,
	};

	static const int MaxSpinCount = 5000;
	static const int TimeoutTick = 10000;

public:
	void WriteLock();
	void WriteUnlock();
	void ReadLock();
	void ReadUnLock();

private:
	atomic<uint32> LockFlag;
	atomic<int32> WriteCount;
};

class ReadLockGuard
{
public:
	ReadLockGuard(Lock& lock, const string name) : _lock(lock), _lockName(name) { _lock.ReadLock(); }
	~ReadLockGuard() { _lock.ReadUnLock(); }
protected:
	Lock&	_lock;
	string _lockName;
};

class WriteLockGuard
{
public:
	WriteLockGuard(Lock& lock, const string name) : _lock(lock), _lockName(name) { _lock.WriteLock(); }
	~WriteLockGuard() { _lock.WriteUnlock(); }
protected:
	Lock& _lock;
	string _lockName;
};