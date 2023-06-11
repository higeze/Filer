#pragma once
#include "rxcpp/rx.hpp"
#include "JsonSerializer.h"

template <class T>
class reactive_command
{
private:
	rxcpp::subjects::subject<T> m_subject;
public:
	reactive_command():
		m_subject(){};
	virtual ~reactive_command() = default;
	reactive_command(const reactive_command& val) = default;
	reactive_command(reactive_command&& val) = default;
	reactive_command& operator=(const reactive_command& val) = default;
	reactive_command& operator=(reactive_command&& val) = default;

	template<class... ArgN>
	auto subscribe(ArgN&&... an) const -> rxcpp::composite_subscription
	{
		return m_subject.get_observable().subscribe(std::forward<ArgN>(an)...);
	}

	void execute(const T& value)
	{
		m_subject.get_subscriber().on_next(value);
	}
};