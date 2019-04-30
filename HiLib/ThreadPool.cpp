#include "ThreadPool.h"

// the constructor just launches some amount of workers
CThreadPool::CThreadPool(size_t threads)
	: stop(false)
{
	for (size_t i = 0; i < threads; ++i)
		workers.emplace_back(
			[this] {
		for (;;) {
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
			BOOST_LOG_TRIVIAL(trace) << L"CThreadPool Active Count:" << activeCount.load();
			task();
			activeCount--;
			BOOST_LOG_TRIVIAL(trace) << L"CThreadPool Active Count:" << activeCount.load();
		}
	}
	);
}

// the destructor joins all threads
CThreadPool::~CThreadPool()
{
	{
		std::unique_lock<std::mutex> lock(queue_mutex);
		stop = true;
	}
	condition.notify_all();
	for (std::thread &worker : workers)
		worker.join();
}