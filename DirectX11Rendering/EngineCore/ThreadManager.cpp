#include "pch.h"
#include "ThreadManager.h"
#include "ThreadPool.h"

ThreadManager::ThreadManager()
{
	m_maxThreadCount = 5;

	m_threadPool.Initialize(m_maxThreadCount);
}

ThreadManager::~ThreadManager()
{
	m_threadPool.ShutDown();
}


void ThreadManager::Launch(function<bool(ThreadParam, promise<bool>&&)> callback, ThreadParam Param)
{
	promise<bool> pm;
	m_workerStatus.push_back(pm.get_future());
	lock_guard<mutex> guard(m_lock);

	m_threadPool.Enqueue(make_tuple(callback, Param, move(pm)));
}

void ThreadManager::Join()
{
	for (auto& status : m_workerStatus)
	{
		status.get();
	}

	m_workerStatus.clear();
}