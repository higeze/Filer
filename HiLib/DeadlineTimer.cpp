#include "DeadlineTimer.h"
#include "ThreadPool.h"

void CDeadlineTimer::run(std::function<void()> action, const std::chrono::milliseconds& ms)
{
	if (m_future.valid() && m_future.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready) {
		std::lock_guard<std::mutex> guard(m_mtx);
		m_action = action;
		m_cv.notify_one();
	} else {
		m_action = action;
		m_future = CThreadPool::GetInstance()->enqueue([this, ms] {
//			m_running.store(true);
			std::unique_lock<std::mutex> lock(m_mtx);
			while (!m_stop.load()) {
				if (m_cv.wait_for(lock, ms) == std::cv_status::timeout) {
					if (m_action) {
						m_action();
					}
					break;
				}
			}
//			m_running.store(false);
			return;
		});
	}
}

void CDeadlineTimer::runperiodic(std::function<void()> action, const std::chrono::milliseconds& ms)
{
	if (m_future.valid() && m_future.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready) {
		std::lock_guard<std::mutex> guard(m_mtx);
		m_action = action;
	} else {
		m_action = action;
		m_future = CThreadPool::GetInstance()->enqueue([this, ms] {
			std::this_thread::sleep_for(ms);
			std::lock_guard<std::mutex> guard(m_mtx);
			if (m_action && !m_stop.load()) {
				spdlog::info("runperiodic");
				m_action();
			}
			return;
		});
	}
}

void CDeadlineTimer::stop()
{
	if (m_future.valid() && m_future.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready) {
		{
			std::lock_guard<std::mutex> guard(m_mtx);
			m_stop.store(true);
			m_cv.notify_one();
		}
		m_future.get();
		m_stop.store(false);
	}
}

