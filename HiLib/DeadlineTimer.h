#pragma once
#include <mutex>
#include <chrono>
#include <condition_variable>
#include <atomic>
#include <future>

class CDeadlineTimer
{
private:
	std::mutex m_mtx;
	std::condition_variable m_cv;
	std::future<void> m_future;
	std::function<void()> m_action;
	std::atomic<bool> m_stop;
	std::atomic<bool> m_update;
	//std::atomic<bool> m_running;
public:
	CDeadlineTimer() :m_stop(false), m_update(false)/*, m_running(false)*/ {}
	~CDeadlineTimer() { stop(); }

	void run(std::function<void()> action, const std::chrono::milliseconds& ms);

	void run_oneshot(std::function<void()> action, const std::chrono::milliseconds& ms);

	void stop();
};

