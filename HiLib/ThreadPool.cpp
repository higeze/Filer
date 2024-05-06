#include "ThreadPool.h"
#include "se_exception.h"
#include <format>
#include "MyString.h"

// the constructor just launches some amount of workers
CThreadPool::CThreadPool(size_t threads)
	: m_stop(false)
{
	for (size_t i = 0; i < threads; ++i)
		m_workers.emplace_back(
			[this] {
		//CoInitialize
		CCoInitializer coinit;
		//Catch SEH exception as CEH
		scoped_se_translator se_trans;
		for (;;) {
			//Get task from queue
			std::shared_ptr<CTask> task;
			{
				std::unique_lock<std::shared_mutex> lock(m_queued_mutex);
				m_condition.wait(lock, [this] { return m_stop || !m_queued_tasks.empty(); });
				if (m_stop && m_queued_tasks.empty()) {
					return;
				}
				task = m_queued_tasks.front();
				m_queued_tasks.erase(m_queued_tasks.begin());
			}
			//Add to runnning task
			{
				std::lock_guard<std::shared_mutex> lock(m_running_mutex);
				m_running_tasks.push_back(task);
			}
			//Run
			try {
				task->Task();
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
			//Remove from runnning task
			{
				std::lock_guard<std::shared_mutex> lock(m_running_mutex);
				m_running_tasks.erase(std::remove(m_running_tasks.begin(), m_running_tasks.end(), task), m_running_tasks.end());
			}
		}
	}
	);
}

// the destructor joins all threads
CThreadPool::~CThreadPool()
{
	{
		std::lock_guard<std::shared_mutex> lock(m_queued_mutex);
		m_stop = true;
	}
	m_condition.notify_all();
	for (std::thread& worker : m_workers) {
		worker.join();
	}
}

const std::wstring CThreadPool::OutputString()
{
	std::wstring log(L"/* ThreadPool Information */\n");
	log += std::format(L"Thread Pool Count:\t{}\n", CThreadPool::GetInstance()->GetTotalThreadCount());

	log += std::format(L"Queued Task:\t{}\n", CThreadPool::GetInstance()->GetQueuedTaskCount());
	auto queuedTasks = CThreadPool::GetInstance()->GetQueuedTasks();
	for (size_t i = 0; i < queuedTasks.size(); i++) {
		log += std::format(L"[{:2}]:{}\n", i, str2wstr(queuedTasks[i]->Name));
	}

	log += std::format(L"Running Task:\t{}\n", CThreadPool::GetInstance()->GetRunnningTaskCount());
	auto runningTasks = CThreadPool::GetInstance()->GetRunningTasks();
	for (size_t i = 0; i < runningTasks.size(); i++) {
		log += std::format(L"[{:2}]:{}\n", i, str2wstr(runningTasks[i]->Name));
	}
	return log;
}

CThreadPool* CThreadPool::GetInstance()
{
	static CThreadPool pool(16);
	return &pool;
}




