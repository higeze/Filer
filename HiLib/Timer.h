#pragma once
#include <mutex>
#include <chrono>
#include <condition_variable>
#include <atomic>
#include <future>

class CTimer
{
private:
	std::mutex m_mtx;
	std::condition_variable m_cv;
	std::future<void> m_future;
	std::function<void()> m_action;
	std::atomic<bool> m_stop;
	//std::atomic<bool> m_running;
public:
	CTimer() :m_stop(false)/*, m_running(false)*/ {}
	~CTimer() { stop(); }

	void run(std::function<void()> action, const std::chrono::milliseconds& ms);
	void stop();
};

