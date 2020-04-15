#include "Timer.h"
#include "ThreadPool.h"

void CTimer::run(std::function<void()> action, const std::chrono::milliseconds& ms)
{
	if (m_future.valid() && m_future.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready) {
		std::lock_guard<std::mutex> guard(m_mtx);
		m_action = action;
		m_cv.notify_one();
	} else {
		m_action = action;
		m_future = CThreadPool::GetInstance()->enqueue([this, ms] {
			std::unique_lock<std::mutex> lock(m_mtx);
			while (!m_stop.load()) {
				if (m_cv.wait_for(lock, ms) == std::cv_status::timeout) {
					if (m_action) {
						m_action();
					}
				} else {
					break;
				}
			}
			return;
			});
	}
}

void CTimer::stop()
{
	if (m_future.valid() /*&& m_future.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready*/) {
		{
			std::lock_guard<std::mutex> guard(m_mtx);
			m_stop.store(true);
			m_cv.notify_one();
		}
		m_future.get();
		m_stop.store(false);
	}
}

