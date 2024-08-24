#pragma once

class ThreadPool
{
public:
	ThreadPool();
	~ThreadPool();

public:
	void Initialize(int32 numThreads);

public:
	void Enqueue(tuple<function<bool(ThreadParam, std::promise<bool>&&)>, ThreadParam, std::promise<bool>> Task);
	void ShutDown();


private:
	void WorkerThreads();

	Vector<thread> m_workers;
	Queue<tuple<function<bool(ThreadParam, std::promise<bool>&&)>, ThreadParam, std::promise<bool>>> m_taskQue;

	mutex	m_mutex;
	condition_variable m_cv;
	bool m_bStop;
private:
	int32 m_numThreads;
	atomic<int32>  m_threadID;
};
