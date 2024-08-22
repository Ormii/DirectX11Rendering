#include "pch.h"
#include "ThreadPool.h"

ThreadPool::ThreadPool()
	:m_numThreads(0), m_bStop(false)
{

}


ThreadPool::~ThreadPool()
{
	ShutDown();
}


void ThreadPool::Initialize(int32 numThreads)
{
	m_numThreads = numThreads;
	for (int i = 0; i < m_numThreads; ++i)
		m_workers.emplace_back([this]() {WorkerThreads(); });
}


void ThreadPool::Enqueue(tuple<function<bool(ThreadParam, std::promise<bool>&&)>, ThreadParam, std::promise<bool>> taskInfo)
{
	{
		std::unique_lock lock(m_mutex);

		m_taskQue.push(move(taskInfo));
	}

	m_cv.notify_one();
}

inline void ThreadPool::ShutDown()
{
	{
		std::unique_lock lock(m_mutex);
		m_bStop = true;
	}

	m_cv.notify_all();

	for (auto& worker : m_workers)
		worker.join();

	m_workers.clear();
}

inline void ThreadPool::WorkerThreads()
{
	while (true)
	{
		tuple<function<bool(ThreadParam, std::promise<bool>&&)>, ThreadParam, std::promise<bool>> taskinfo;

		{
			std::unique_lock lock(m_mutex);

			m_cv.wait(lock, [this]()->bool {
				return m_bStop || !m_taskQue.empty();
				});

			if (m_bStop == true && m_taskQue.empty())
				break;
		
			taskinfo = move(m_taskQue.front());
			m_taskQue.pop();
		}

		auto [task, param, pm] = move(taskinfo);
		task(param, move(pm));
	}
}
