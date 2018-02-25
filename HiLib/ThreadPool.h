#pragma once
#include <deque>
#include <vector>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <cassert>
#include "MyCom.h"


template <typename T> class Queue
{
public:
	Queue(size_t size)
		: size_(size)
	{}
	bool put(T&& data) {
		if (size_ <= deque_.size()) {
			return false;
		}
		deque_.emplace_back(std::move(data));
		return true;
	}
	bool put(const T& data) {
		if (size_ <= deque_.size()) {
			return false;
		}
		deque_.emplace_back(data);
		return true;
	}
	bool get(T& data) {
		if (deque_.empty()) {
			return false;
		}
		data = std::move(deque_.front());
		deque_.pop_front();
		return true;
	}
	bool empty() const {
		return deque_.empty();
	}
private:
	size_t size_;
	std::deque<T> deque_;
};

class ThreadPool
{
public:
	ThreadPool(size_t threadCount, size_t queueSize)
		: isTerminationRequested_(false)
		, queue_(queueSize)
	{
		for (size_t n = 0; n < threadCount; n++) {
			threads_.emplace_back(std::thread(main_));
		}
	}
	~ThreadPool() {
		{
			std::unique_lock<std::mutex> ul(mutex_);
			isTerminationRequested_ = true;
		}
		cv_.notify_all();
		const size_t size = threads_.size();
		for (size_t n = 0; n < size; n++) {
			threads_.at(n).join();
		}
	}
	bool add(std::function<void()> &&func) {
		{
			std::unique_lock<std::mutex> ul(mutex_);
			if (!queue_.put(std::move(func))) { return false; }
		}
		cv_.notify_all();
		return true;
	}
	bool add(const std::function<void()> &func) {
		{
			std::unique_lock<std::mutex> ul(mutex_);
			if (!queue_.put(func)) { return false; }
		}
		cv_.notify_all();
		return true;
	}
private:
	std::function<void()> main_ = [this]()
	{
		CCoInitializer(COINIT_APARTMENTTHREADED);
		while (1) {
			std::function<void()> func;
			{
				std::unique_lock<std::mutex> ul(mutex_);
				while (queue_.empty()) {
					if (isTerminationRequested_) { return; }
					cv_.wait(ul);
				}
				const bool result = queue_.get(func);
				assert(result);
			}
			func();
		}
	};
	bool isTerminationRequested_;
	Queue<std::function<void()>> queue_;
	std::mutex mutex_;
	std::condition_variable cv_;
	std::vector<std::thread> threads_;
};