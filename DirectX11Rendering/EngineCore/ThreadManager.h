#pragma once
#include "ThreadPool.h"

class ThreadManager
{
public:
	ThreadManager();

	~ThreadManager();

public:
	uint32 GetMaxThreadCount() { return m_maxThreadCount; }

public:

	void Launch(function<bool(ThreadParam, promise<bool>&&)> callback, ThreadParam Param);

	void Join();


private:
	ThreadPool m_threadPool;
	Vector<future<bool>> m_workerStatus;

private:
	uint32 m_maxThreadCount;

private:
	mutex		   m_lock;
};

