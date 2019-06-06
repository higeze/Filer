#include "ThreadPool.h"
#include "SEHException.h"

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

				::MessageBoxA(nullptr, msg.c_str(), "Exception in Thread Pool", MB_ICONWARNING);
			} catch (...) {
				std::string msg = (boost::format(
					"Last Error:%2%\r\n"
				) % GetLastErrorString()).str();

				MessageBoxA(nullptr, msg.c_str(), "Unknown Exception in Thread Pool", MB_ICONWARNING);
			}
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
	static CThreadPool pool(32);
	return &pool;
}




