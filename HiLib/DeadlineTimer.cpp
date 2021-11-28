#include "DeadlineTimer.h"
#include "async_catch.h"

void CDeadlineTimer::run(std::function<void()> action, const std::chrono::milliseconds& ms)
{
	if (m_future.valid() && m_future.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready) {
		std::lock_guard<std::mutex> guard(m_mtx);
		m_action = action;
		m_cv.notify_one();
	} else {
		m_action = action;
		auto fun = [this, ms]
		{
			std::unique_lock<std::mutex> lock(m_mtx);
			while (!m_stop.load()) {
				auto abs_time = std::chrono::steady_clock::now() + ms;
				auto status = m_cv.wait_until(lock, abs_time);
				if (status == std::cv_status::timeout) { //timeout
					if (m_action) {
						m_action();
					}
					break;
				} else if (m_stop.load()) {//no_timeout. stop
					break;
				}
			}
			return;
		};
		m_future = std::async(
			std::launch::async,
			async_action_wrap<decltype(fun)>,
			fun);
	}
}

void CDeadlineTimer::run_oneshot(std::function<void()> action, const std::chrono::milliseconds& ms)
{
	//Thread started, not finished
	if (m_future.valid() && m_future.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready) {
		std::lock_guard<std::mutex> guard(m_mtx);
		m_action = action;
	} else {
		m_action = action;
		auto fun = [this, ms]()
		{
			std::this_thread::sleep_for(ms);
			std::lock_guard<std::mutex> guard(m_mtx);
			if (m_action && !m_stop.load()) {
				m_action();
			}
			return;
		};
		m_future = std::async(
			std::launch::async,
			async_action_wrap<decltype(fun)>,
			fun);
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

