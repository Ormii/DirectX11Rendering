#pragma once

template <typename T>
class ThreadManager
{
public:
	ThreadManager()
	{
		m_maxThreadCount = 1;
		if (m_maxThreadCount < (uint32)std::thread::hardware_concurrency)
			m_maxThreadCount = (uint32)std::thread::hardware_concurrency;

	}

	~ThreadManager()
	{
		Join();
	}

public:
	uint32 GetMaxThreadCount() { return m_maxThreadCount; }

public:

	void Launch(function<void(T&, ThreadParam)> callback, T* owner, ThreadParam Param)
	{
		lock_guard<mutex> guard(m_lock);
		m_threads.push_back(thread([=]() {
			callback(*owner, Param);
		}));
	}

	void Join()
	{
		for (auto& t : m_threads)
		{
			if (t.joinable())
				t.join();
		}

		m_threads.clear();
	}

private:
	uint32 m_maxThreadCount;

private:
	vector<thread> m_threads;
	mutex		   m_lock;
};

