#pragma once

template <typename T>
class ThreadManager
{
public:
	ThreadManager()
	{
		InitTLS();
	}

	~ThreadManager()
	{
		Join();
	}

public:

	void Launch(function<void(T&, ThreadParam)> callback, T* owner, ThreadParam Param)
	{
		lock_guard<mutex> guard(m_lock);
		m_threads.push_back(thread([=]() {
			InitTLS();
			callback(*owner, Param);
			DestroyTLS();
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

	static void InitTLS()
	{

	}

	static void DestroyTLS()
	{

	}

private:
	vector<thread> m_threads;
	mutex		   m_lock;
};

