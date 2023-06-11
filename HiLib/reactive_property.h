#pragma once
#include "rxcpp/rx.hpp"
#include "JsonSerializer.h"

template <class T>
class reactive_property
{
private:
	rxcpp::subjects::subject<T> m_subject;
	T m_value;
public:
	reactive_property(const T& value = T())
		: m_value(value) {}

	virtual ~reactive_property() = default;

	//auto operator<=>(const reactive_property&) const = default;
	bool operator==(const reactive_property& val) const
	{
		return this->get() == val.get();
	}
	reactive_property(const reactive_property& val) = default;
	reactive_property(reactive_property&& val) = default;
	reactive_property& operator=(const reactive_property& val) = default;
	reactive_property& operator=(reactive_property&& val) = default;
	reactive_property& operator=(const T& val) 
	{
		set(val);
		return *this;
	}
	reactive_property& operator=(T&& val)
	{
		set(val);
		return *this;
	}
	template<class... ArgN>
	auto subscribe(ArgN&&... an) const -> rxcpp::composite_subscription
	{
		return m_subject.get_observable().subscribe(std::forward<ArgN>(an)...);
	}
	const T& get() const 
	{ 
		return m_value;
	}
	T& get_unconst() 
	{ 
		return m_value;
	}
	virtual void set(const T& value)
	{
		if (m_value != value) {
			m_value = value;
			m_subject.get_subscriber().on_next(value);
		}
	}

	friend void to_json(json& j, const reactive_property<T>& o)
	{
		j = {
			{"Value", o.get()},
		};
	}

	friend void from_json(const json& j, reactive_property<T>& o)
	{
		T value;
		j.at("Value").get_to(value);
		o.set(value);
	}
};
