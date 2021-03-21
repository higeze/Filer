#include "Timer.h"

void CTimer::run(std::function<void()> action, const std::chrono::milliseconds& ms)
{
	//Thread started, not finished
	if (m_future.valid() && m_future.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready) {
		std::lock_guard<std::mutex> guard(m_mtx);
		m_stop.store(false);
		m_action = action;
		m_cv.notify_one();
	} else {
		m_stop.store(false);
		m_action = action;
		m_future = std::async(std::launch::async, [this, ms] {
			std::unique_lock<std::mutex> lock(m_mtx);
			while (!m_stop.load()) {
				auto abs_time = std::chrono::steady_clock::now() + ms;
				auto status = m_cv.wait_until(lock, abs_time);
				if (status == std::cv_status::timeout) {//timeout
					if (m_action) {
						m_action();
					}
				} else if(m_stop.load()){//no_timeout. stop
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
