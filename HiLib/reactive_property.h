#pragma once
#include "rxcpp/rx.hpp"
#include "JsonSerializer.h"

template <class T>
class reactive_property
{
private:
	rxcpp::subjects::subject<T> m_subject;
	std::shared_ptr<T> m_spValue;
public:
	explicit reactive_property(const T& value = T())
		: m_spValue(std::make_shared<T>(value)) {}

	virtual ~reactive_property() = default;

	auto operator<=>(const reactive_property& rhs) const
	{
		return this->get() <=> rhs.get();
	}
	bool operator==(const reactive_property& rhs) const
	{
		return this->get() == rhs.get();
	}
	reactive_property(const reactive_property& val) = default;
	reactive_property& operator=(const reactive_property& val) = default;
	reactive_property(reactive_property&& val) noexcept = default;
	reactive_property& operator=(reactive_property&& val) noexcept = default;
	//reactive_property& operator=(const T& val) 
	//{
	//	set(val);
	//	return *this;
	//}
	//reactive_property& operator=(T&& val)
	//{
	//	set(val);
	//	return *this;
	//}
	mutable std::shared_ptr<int> m_pCount = std::make_shared<int>(0);
	int get_subscriber_count() const { return *m_pCount; }

	template<class... ArgN>
	auto subscribe(ArgN&&... an) const -> rxcpp::composite_subscription
	{
		(*m_pCount)++;
		return m_subject.get_observable().subscribe(std::forward<ArgN>(an)...);
	}


	const T& get() const 
	{ 
		return *m_spValue;
	}
	T& get_unconst() 
	{ 
		return *m_spValue;
	}
	virtual void set(const T& value)
	{
		if (*m_spValue != value) {
			*m_spValue = value;
			m_subject.get_subscriber().on_next(value);
		}
	}

	friend void to_json(json& j, const reactive_property<T>& o)
	{
		j = {
			{"Value", o.get()}
		};
	}

	friend void from_json(const json& j, reactive_property<T>& o)
	{
		T value;
		o.set(j.at("Value").get_to(value));
		//o.set(value);
	}
};
