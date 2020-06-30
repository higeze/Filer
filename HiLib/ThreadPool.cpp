#include "ThreadPool.h"
#include "SEHException.h"
#include <fmt/format.h>

// the constructor just launches some amount of workers
CThreadPool::CThreadPool(size_t threads)
	: stop(false)
{
	for (size_t i = 0; i < threads; ++i)
		workers.emplace_back(
			[this] {
		//CoInitialize
		CCoInitializer coinit;
		//Catch SEH exception as CEH
		_set_se_translator(CSEHException::TransferSEHtoCEH);
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
				try {
					task();
				}catch (std::exception& ex) {
					std::string msg = fmt::format(
						"What:{}\r\n"
						"Last Error:{}\r\n",
						ex.what(), GetLastErrorString());

					::MessageBoxA(nullptr, msg.c_str(), "Exception in Thread Pool", MB_ICONWARNING);
				}
				catch (...) {
					std::string msg = fmt::format(
						"Last Error:{}",
						GetLastErrorString());

					MessageBoxA(nullptr, msg.c_str(), "Unknown Exception in Thread Pool", MB_ICONWARNING);
				}
				activeCount--;
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

CThreadPool* CThreadPool::GetInstance()
{
	static CThreadPool pool(16);
	return &pool;
}




