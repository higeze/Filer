#pragma once

#include <vector>

enum class NotifyVectorChangedAction
{
	Add,
	Move,
	Remove,
	Replace,
	Reset,
};

template<typename T>
struct NotifyVectorChangedEventArgs
{
	NotifyVectorChangedAction Action;
	std::vector<T> NewItems;
	int NewStartingIndex = -1;
	std::vector<T> OldItems;
	int OldStartingIndex = -1;
};

template<class T, class Allocator = std::allocator<T>>
class observable_vector:public std::vector<T, Allocator>
{
public:
	std::vector<T, Allocator>::vector;

	boost::signals2::signal<void(const NotifyVectorChangedEventArgs<T>&)> VectorChanged;

	void notify_push_back(const T& x)
	{
		std::vector<T, Allocator>::push_back(x);
		VectorChanged(NotifyVectorChangedEventArgs<T>
		{
			NotifyVectorChangedAction::Add,
			{ x },
			(int)size() - 1,
			{},
			-1
		});
	}

	iterator notify_erase(const_iterator where)
	{
		auto oldItem = *where;
		auto index = std::distance((const_iterator)std::vector<T, Allocator>::begin(), where);
		iterator ret = std::vector<T, Allocator>::erase(where);
		VectorChanged(NotifyVectorChangedEventArgs<T>
		{
			NotifyVectorChangedAction::Remove,
			{},
			-1,
			{oldItem},
			index
		});

		return ret;
	}

	iterator notify_erase(const_iterator first, const_iterator last)
	{
		auto oldItems = std::vector<T>(first, last);
		auto index = std::distance((const_iterator)std::vector<T, Allocator>::begin(), first);
		iterator ret = std::vector<T, Allocator>::erase(first, last);
		VectorChanged(NotifyVectorChangedEventArgs<T>
			{
				NotifyVectorChangedAction::Remove,
				{},
				-1,
				oldItems,
				index
			});

		return ret;
	}


	void notiry_clear()
	{
		clear();
		VectorChanged(NotifyVectorChangedEventArgs<T>
			{
				NotifyVectorChangedAction::Reset,
				{},
				-1,
				{},
				-1
			});
		return std::vector::ret;
	}

};

template<typename T>
struct NotifyChangedEventArgs
{
	T OldValue;
};





template<typename T>
class observable
{
private:
	T m_value;

public:
	observable(){}
	observable(const T& value) :m_value(value){}

	boost::signals2::signal<void(const NotifyChangedEventArgs<T>&)> Changed;

	void notify_set(const T& value)
	{
		if (m_value != value) {
			auto oldValue = m_value;
			m_value = value;
			Changed(NotifyChangedEventArgs<T>{ oldValue });
		}
	}

	T get()const { return m_value; }
	void set(const T& value) { m_value = value; }
};

template<typename T>
std::wostream& operator<<(std::wostream& os, const observable<T>& observable)
{
	os << observable.get();
	return os;
}

template <typename T>
std::wistream& operator>>(std::wistream& is, observable<T>& observable)
{
	T value;
	is >> value;
	observable.set(value);
	return is;
}
