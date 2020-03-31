#pragma once

#include <vector>
#include <tuple>

enum class NotifyVectorChangedAction
{
	Add,
	Insert,
	Move,
	Remove,
	Replace,
	Reset,
};

enum class NotifyStringChangedAction
{
	Assign,
	Insert,
	Erase,
	Replace,
	Clear,
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

template<typename CharT,
	class Traits = std::char_traits<CharT>,
	class Allocator = std::allocator<CharT> >
struct NotifyStringChangedEventArgs
{
	NotifyStringChangedAction Action;
	std::basic_string<CharT, Traits, Allocator> NewString;
	std::basic_string<CharT, Traits, Allocator> OldString;
	int StartIndex = -1;
	int OldEndIndex = -1;
	int NewEndIndex = -1;
};


template <class CharT,
	class Traits = std::char_traits<CharT>,
	class Allocator = std::allocator<CharT> >
class observable_basic_string :public std::basic_string<CharT, Traits, Allocator>
{
public:
	std::basic_string<CharT, Traits, Allocator>::basic_string;

	boost::signals2::signal<void(const NotifyStringChangedEventArgs<CharT>&)> StringChanged;

	observable_basic_string<CharT, Traits, Allocator>& notify_assign(const std::basic_string<CharT, Traits, Allocator>& str)
	{
		std::basic_string<CharT, Traits, Allocator> old(*this);
		std::basic_string<CharT, Traits, Allocator>::operator=(str);
		StringChanged(NotifyStringChangedEventArgs<CharT, Traits, Allocator>{
			NotifyStringChangedAction::Assign,
			*this,
			old,
			0, (int)old.size(), (int)this->size()
		});
		return *this;
	}

	observable_basic_string<CharT, Traits, Allocator>& notify_insert(size_type index, const basic_string<CharT, Traits, Allocator>& str)
	{
		std::basic_string<CharT, Traits, Allocator> old(*this);
		insert(index, str);
		StringChanged(NotifyStringChangedEventArgs<CharT, Traits, Allocator>{
			NotifyStringChangedAction::Insert,
			*this,
			old,
			(int)index, (int)index, (int)(index + str.size())
		});
		return *this;
	}

	observable_basic_string<CharT, Traits, Allocator>& notify_erase(size_type index = 0, size_type count = npos)
	{
		std::basic_string<CharT, Traits, Allocator> old(*this);
		erase(index, count);
		StringChanged(NotifyStringChangedEventArgs<CharT, Traits, Allocator>{
			NotifyStringChangedAction::Erase,
			*this,
			old,
			(int)index, (int)(index + count), (int)(index)
		});
		return *this;
	}

	observable_basic_string<CharT, Traits, Allocator>& notify_replace(size_type pos, size_type count, const basic_string& str)
	{
		std::basic_string<CharT, Traits, Allocator> old(*this);
		replace(pos, count, str);
		StringChanged(NotifyStringChangedEventArgs<CharT, Traits, Allocator>{
			NotifyStringChangedAction::Replace,
			*this,
			old,
			(int)pos, (int)(pos + count), int(pos + str.size())
		});
		return *this;
	}

	void notify_clear()
	{
		std::basic_string<CharT, Traits, Allocator> old(*this);
		clear();
		StringChanged(NotifyStringChangedEventArgs<CharT, Traits, Allocator>{
			NotifyStringChangedAction::Clear,
			*this,
			old,
			0, (int)old.size(), 0
		});
	}
};

using observable_wstring = observable_basic_string<wchar_t>;


template<class T, class Allocator = std::allocator<T>>
class observable_vector:public std::vector<T, Allocator>
{
public:
	using std::vector<T, Allocator>::vector;
	virtual ~observable_vector<T, Allocator>(){}

	observable_vector<T, Allocator>(const observable_vector<T, Allocator>& val)
	{
		*this = val;
	}

	observable_vector<T, Allocator>(observable_vector<T, Allocator>&& val)
	{
		*this = val;
	}

	observable_vector<T, Allocator>& operator=(const observable_vector<T, Allocator>& val)
	{
		std::vector<T, Allocator>::operator=(val);
		VectorChanged.connect(val.VectorChanged);
		return *this;
	}

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

	iterator notify_insert(const_iterator position, const T& x)
	{
		auto ret = std::vector<T, Allocator>::insert(position, x);
		VectorChanged(NotifyVectorChangedEventArgs<T>
		{
			NotifyVectorChangedAction::Insert,
			{ x },
			(int)std::distance((const_iterator)std::vector<T, Allocator>::begin(), position),
			{},
				-1
		});
		return ret;
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


	void notify_clear()
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
		return;
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
	observable(const observable<T>& value) :m_value(value.m_value) {}
	observable(const T& value) :m_value(value){}
	observable<T>& operator=(const observable<T>& value) { m_value = value.m_value; return *this; }
	observable<T>& operator=(const T& value){ m_value = value; return *this; }

	boost::signals2::signal<void(const NotifyChangedEventArgs<T>&)> Changed;

	void notify_set(const T& value)
	{
		if (m_value != value) {
			auto oldValue = m_value;
			m_value = value;
			Changed(NotifyChangedEventArgs<T>{ oldValue });
		}
	}
	operator T () { return m_value; }
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

template<typename... Args>
class observable_tuple:public std::tuple<Args...>
{
public:
	observable_tuple(const Args& ...args) :std::tuple<Args...>(args...) {}

	boost::signals2::signal<void(const NotifyChangedEventArgs<std::tuple<Args...>>&)> Changed;

	void notify_set(const Args& ...args)
	{
		std::tuple<Args...> value = std::make_tuple(args...);
		if (static_cast<std::tuple<Args...>>(*this) != value) {
			std::tuple<Args...> oldValue = static_cast<std::tuple<Args...>>(*this);
			std::tuple<Args...>::operator=(value);
			Changed(NotifyChangedEventArgs<std::tuple<Args...>>{ oldValue });
		}
	}
};

//template<typename T>
//std::wostream& operator<<(std::wostream& os, const observable<T>& observable)
//{
//	os << observable.get();
//	return os;
//}
//
//template <typename T>
//std::wistream& operator>>(std::wistream& is, observable<T>& observable)
//{
//	T value;
//	is >> value;
//	observable.set(value);
//	return is;
//}
