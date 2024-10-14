#pragma once
#include "Debug.h"
#include "MyWin32.h"

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>
#include "MyCom.h"

class CThreadPool
{
public:
	struct CTask
	{
		std::string Name;
		int Priority;
		std::function<void()> Task;

		template<typename F>
		CTask(const std::string& name, const int& priority, F&& task)
			:Name(name), Priority(priority),Task(std::forward<F>(task))
		{}
		//auto operator<=>(const CTask& rhs)
		//{
		//	return Priority <=> rhs.Priority;
		//}
	};
private:
	//struct compare_shared_task
	//{
	//	bool operator()(const std::shared_ptr<CTask>& left, const std::shared_ptr<CTask>& right) {
	//		return left->Priority < right->Priority;
	//	};
	//};
	using queued_container_type = std::vector<std::shared_ptr<CTask>>;
	using running_container_type = std::vector<std::shared_ptr<CTask>>;
private:
	std::vector<std::thread> m_workers;
	// the task queue


	//std::priority_queue<
	//	std::pair<std::function<void()>, int>,
	//	std::vector<std::pair<std::function<void()>, int>>,
	//	CThreadPool::compare_task
	//> tasks;
	mutable std::shared_mutex m_queued_mutex;
	queued_container_type m_queued_tasks;

	mutable std::shared_mutex m_running_mutex;
	running_container_type m_running_tasks;

	// synchronization
	std::condition_variable_any m_condition;
	bool m_stop;

public:
	CThreadPool(size_t = std::thread::hardware_concurrency() * 2);
	~CThreadPool();

	int GetTotalThreadCount() const { return m_workers.size(); }
	int GetRunnningTaskCount() const 
	{ 
		std::shared_lock<std::shared_mutex> lock(m_running_mutex);
		return m_running_tasks.size();
	}
	int GetQueuedTaskCount() const
	{
		std::shared_lock<std::shared_mutex> lock(m_queued_mutex);
		return m_queued_tasks.size();
	}
	std::thread::id GetThreadId(int n) { return m_workers[n].get_id(); }
	const running_container_type& GetRunningTasks() const 
	{
		std::shared_lock<std::shared_mutex> lock(m_running_mutex);
		return m_running_tasks;
	}
	const queued_container_type& GetQueuedTasks() const
	{ 
		std::shared_lock<std::shared_mutex> lock(m_queued_mutex);
		return m_queued_tasks;
	}

// add new work item to the pool
	template<class F, class... Args>
	auto enqueue(const char* name, int&& priority, F&& f, Args&&... args)
		-> std::future<typename std::invoke_result_t<F, Args...>>
	{
		using return_type = typename std::invoke_result_t<F, Args...>;

		auto task = std::make_shared< std::packaged_task<return_type()> >(
			std::bind(std::forward<F>(f), std::forward<Args>(args)...)
			);

		std::future<return_type> res = task->get_future();
		{
			std::lock_guard<std::shared_mutex> lock(m_queued_mutex);

			// don't allow enqueueing after stopping the pool
			if (m_stop) {
				throw std::runtime_error("enqueue on stopped ThreadPool");
			}

			m_queued_tasks.push_back(std::make_shared<CTask>(name, priority, [task]() { (*task)(); }));
			std::stable_sort(m_queued_tasks.begin(), m_queued_tasks.end(),
				[](const std::shared_ptr<CTask>& left, const std::shared_ptr<CTask>& right) {
					return left->Priority < right->Priority;
				}
			);
		}
		m_condition.notify_one();
		return res;
	}

	const std::wstring OutputString();

	static CThreadPool* GetInstance();
};




