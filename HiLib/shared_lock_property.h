#pragma once

#include <shared_mutex>

template<typename T>
class shared_lock_property
{
private:
	T m_value;
	std::shared_mutex m_mtx;
public:
	shared_lock_property()
		:m_value(){}
	shared_lock_property(const T& value)
		:m_value(value){}

	const T& get()
	{
		std::shared_lock<std::shared_mutex> lock(m_mtx);
		return m_value;
	}

	void set(const T& value)
	{
		std::lock_guard <std::shared_mutex> lock(m_mtx);
		m_value = value;
	}

};
