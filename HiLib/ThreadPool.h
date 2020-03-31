#pragma once
#include "Debug.h"
#include "MyWin32.h"

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>
#include "MyCom.h"

class CThreadPool
{
private:
	// need to keep track of threads so we can join them
	std::vector< std::thread > workers;
	// the task queue
	std::queue< std::function<void()> > tasks;
	// active thread count
	std::atomic<int> activeCount = 0;

	// synchronization
	std::mutex queue_mutex;
	std::condition_variable condition;
	bool stop;

public:
	CThreadPool(size_t);
	~CThreadPool();

// add new work item to the pool
	template<class F, class... Args>
	auto enqueue(F&& f, Args&&... args)
		-> std::future<typename std::invoke_result_t<F, Args...>>
	{
		using return_type = typename std::invoke_result_t<F, Args...>;

		auto task = std::make_shared< std::packaged_task<return_type()> >(
			std::bind(std::forward<F>(f), std::forward<Args>(args)...)
			);

		std::future<return_type> res = task->get_future();
		{
			std::unique_lock<std::mutex> lock(queue_mutex);

			// don't allow enqueueing after stopping the pool
			if (stop)
				throw std::runtime_error("enqueue on stopped ThreadPool");

			tasks.emplace([task]() { (*task)(); });
		}
		condition.notify_one();
		return res;
	}

	static CThreadPool* GetInstance();
};




