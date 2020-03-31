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
			std::unique_lock<std::mutex> lock(m_mtx);
			while (!m_stop.load()) {
				if (m_cv.wait_for(lock, ms) == std::cv_status::timeout) {
					if (m_action) {
						m_action();
					}
					break;
				}
			}
			return;
		});
	}
}

void CDeadlineTimer::run_interval(std::function<void()> action, const std::chrono::milliseconds& ms)
{
	if (m_future.valid() && m_future.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready) {
		SPDLOG_INFO("run_interval request update");
		std::lock_guard<std::mutex> guard(m_mtx);
		m_action = action;
		m_update.store(true);
		m_cv.notify_one();
	} else {
		m_action = action;
		m_future = CThreadPool::GetInstance()->enqueue([this, ms] {
			while (!m_stop.load()) {
				std::unique_lock<std::mutex> lock(m_mtx);
				if (!m_cv.wait_for(lock, ms, [this]()->bool {return m_stop.load() || m_update.load(); })){
					if (m_action) {
						SPDLOG_INFO("run_interval action");
						m_action();
					}
				} else if(m_stop.load()){
					SPDLOG_INFO("run_interval stop");
					break;
				} else if (m_update.load()) {
					SPDLOG_INFO("run_interval update");
					m_update.store(false);
				}

			}
			return;
			});
	}
}

void CDeadlineTimer::run_oneshot(std::function<void()> action, const std::chrono::milliseconds& ms)
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
				SPDLOG_INFO("run_oneshot");
				m_action();
			}
			return;
		});
	}
}

void CDeadlineTimer::stop()
{
	SPDLOG_INFO("run stop");
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

