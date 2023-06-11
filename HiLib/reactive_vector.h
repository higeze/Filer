#pragma once
#include <vector>
#include "rxcpp/rx.hpp"
#include "JsonSerializer.h"

enum class notify_vector_changed_action
{
	push_back,
	insert,
	Move,
	erase,
	replace,
	reset,
};

template<typename T>
struct notify_vector_changed_event_args
{
	notify_vector_changed_action action;
	std::vector<T> new_items;
	int new_starting_index = -1;
	std::vector<T> old_items;
	int old_starting_index = -1;
};

template<class T, class Allocator = std::allocator<T>>
class reactive_vector
{
private:
	rxcpp::subjects::subject<notify_vector_changed_event_args<T>> m_subject;
	std::vector<T, Allocator> m_value;

public:
	using vector_type = std::vector<T, Allocator>;
	using notify_type = NotifyVectorChangedEventArgs<T>;
	using str_type = typename std::vector<T, Allocator>;
	using size_type = typename  std::vector<T, Allocator>::size_type;
	using iterator = typename std::vector<T, Allocator>::iterator;
	using const_iterator = typename std::vector<T, Allocator>::const_iterator;
	using const_reference = typename std::vector<T, Allocator>::const_reference;
	using reference = typename std::vector<T, Allocator>::reference;

public:
	reactive_vector(const vector_type& value = vector_type())
		:m_value(value){};

	virtual ~reactive_vector() = default;
	//auto operator<=>(const reactive_vector&) const = default;
	bool operator==(const reactive_vector& val) const
	{
		return this->m_value == val.m_value;
	}
	reactive_vector(const reactive_vector& val) = default;
	reactive_vector(reactive_vector&& val) = default;
	reactive_vector& operator=(const reactive_vector& val) = default;
	reactive_vector& operator=(reactive_vector&& val) = default;
	reactive_vector& operator=(const T& val) 
	{
		set(val);
		return *this;
	}
	reactive_vector& operator=(T&& val)
	{
		set(val);
		return *this;
	}
	template<class... ArgN>
	auto subscribe(ArgN&&... an) const -> rxcpp::composite_subscription
	{
		return m_subject.get_observable().subscribe(std::forward<ArgN>(an)...);
	}
	const vector_type& get() const 
	{ 
		return m_value;
	}
	vector_type& get_unconst() 
	{ 
		return m_value;
	}
	void set(const vector_type& value)
	{
		if (this->m_value != value) {
			vector_type old(this->m_value);
			this->m_value.assign(value.cbegin(), value.cend());
			this->m_subject.get_subscriber().on_next(notify_vector_changed_event_args<T>
			{
				notify_vector_changed_action::reset,
				this->m_value,
				0,
				old,
				0
			});
		}
	}

	const_reference operator[](size_type pos) const noexcept
	{
		return this->m_value.operator[](pos);
	}

	////TODO
	//reference operator[](size_type pos) noexcept
	//{
	//	return this->m_value.operator[](pos);
	//}
	////TODO
	//iterator begin() noexcept
	//{
	//	return this->m_value.begin();
	//}
	////TODO
	//iterator end() noexcept
	//{
	//	return this->m_value.end();
	//}

	const_iterator cbegin() const noexcept
	{
		return this->m_value.cbegin();
	}

	const_iterator cend() const noexcept
	{
		return this->m_value.cend();
	}

	size_type size() const noexcept
	{
		return this->m_value.size();
	}

	bool empty() const noexcept
	{
		return this->m_value.empty();
	}

	void push_back(const T& x)
	{
		this->m_value.push_back(x);
		this->m_subject.get_subscriber().on_next(notify_vector_changed_event_args<T>
		{
			notify_vector_changed_action::push_back,
			{ x },
			(int)size() - 1,
			{},
			-1
		});
	}

	iterator insert(const_iterator position, const T& x)
	{
		auto ret = this->m_value.insert(position, x);
		auto index = std::distance(this->m_value.begin(), ret);
		this->m_subject.get_subscriber.on_next(notify_vector_changed_event_args<T>
		{
			notify_vector_changed_action::insert,
			{ x },
			index,
			{},
			-1
		});
		return ret;
	}

	iterator replace(iterator position, const T& x)
	{
		auto oldItem = *position;
		*position = x;
		auto index = std::distance(this->m_value.begin(), position);
		this->m_subject.get_subscriber.on_next(notify_vector_changed_event_args<T>
		{
			notify_vector_changed_action::replace,
			{ x },
			index,
			{ oldItem},
			-1
		});
		return position;
	}

	iterator erase(const_iterator where)
	{
		auto oldItem = *where;
		auto index = std::distance(this->m_value.cbegin(), where);
		iterator ret = this->m_value.erase(where);
		this->m_subject.get_subscriber().on_next(notify_vector_changed_event_args<T>
		{
			notify_vector_changed_action::erase,
			{},
			-1,
			{ oldItem },
			index
		});
		return ret;
	}

	iterator erase(const_iterator first, const_iterator last)
	{
		auto oldItems = vector_type(first, last);
		auto index = std::distance(this->m_value.cbegin(), first);
		iterator ret = this->m_value.erase(first, last);
		this->m_pSubject->OnNext(NotifyVectorChangedEventArgs<T>
		{
			notify_vector_changed_action::erase,
			{},
				-1,
				oldItems,
				index
		});
		return ret;
	}


	void clear()
	{
		auto old = this->m_value;
		this->m_value.clear();
		this->m_subject.get_subscriber().on_next(notify_vector_changed_event_args<T>
		{
			notify_vector_changed_action::reset,
			{},
			-1,
			old,
			0
		});
		return;
	}

	friend void to_json(json& j, const reactive_vector<T, Allocator>& o)
	{
		j = {
			{"Value", o.get()}
		};
	}

	friend void from_json(const json& j, reactive_vector<T, Allocator>& o)
	{
		vector_type value;
		j.at("Value").get_to(value);
		o.set(value);
	}
};
