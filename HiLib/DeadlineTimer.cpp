#include "DeadlineTimer.h"
#include "ThreadPool.h"

void CDeadlineTimer::run(std::function<void()> action, const std::chrono::milliseconds& ms)
{
	if (m_running.load()) {
		std::lock_guard<std::mutex> guard(m_mtx);
		m_action = action;
		m_cv.notify_one();
	} else {
		m_action = action;
		m_future = CThreadPool::GetInstance()->enqueue([this, ms] {
			m_running.store(true);
			std::unique_lock<std::mutex> lock(m_mtx);
			while (!m_stop.load()) {
				if (m_cv.wait_for(lock, ms) == std::cv_status::timeout) {
					if (m_action) {
						m_action();
					}
					break;
				}
			}
			m_running.store(false);
			return;
		});
	}
}

void CDeadlineTimer::stop()
{
	if (m_running.load()) {
		{
			std::lock_guard<std::mutex> guard(m_mtx);
			m_stop.store(true);
			m_cv.notify_one();
		}
		m_future.get();

	}
}

