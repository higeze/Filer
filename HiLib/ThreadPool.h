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
	//CThreadPool(size_t);
	//~CThreadPool();
// the constructor just launches some amount of workers
	CThreadPool(size_t threads)
		: stop(false)
	{
		for (size_t i = 0; i < threads; ++i)
			workers.emplace_back(
			[this] {
			CCoInitializer coinit;
			for (;;) {
				try {
					std::function<void()> task;

					{
						std::unique_lock<std::mutex> lock(this->queue_mutex);
						this->condition.wait(lock,
							[this] { return this->stop || !this->tasks.empty(); });
						if (this->stop && this->tasks.empty())
							return;
						task = std::move(this->tasks.front());
						this->tasks.pop();
					}
					activeCount++;
					task();
					activeCount--;
				} catch (std::exception& ex) {
					std::string msg = (boost::format(
						"What:%1%\r\n"
						"Last Error:%2%\r\n"
					) % ex.what() % GetLastErrorString()).str();

					::MessageBoxA(nullptr, msg.c_str(), "Exception in Thread Pool task", MB_ICONWARNING);
				}
			}
		}
		);
	}

	// the destructor joins all threads
	~CThreadPool()
	{
		{
			std::unique_lock<std::mutex> lock(queue_mutex);
			stop = true;
		}
		condition.notify_all();
		for (std::thread &worker : workers)
			worker.join();
	}

	//template<class F, class... Args>
	//auto enqueue(F&& f, Args&&... args)
	//	->std::future<typename std::result_of<F(Args...)>::type>;

// add new work item to the pool
	template<class F, class... Args>
	auto enqueue(F&& f, Args&&... args)
		-> std::future<typename std::result_of<F(Args...)>::type>
	{
		using return_type = typename std::result_of<F(Args...)>::type;

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

	static CThreadPool* CThreadPool::GetInstance()
	{
		static CThreadPool pool(32);
		return &pool;
	}
};




