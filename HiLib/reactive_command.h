#pragma once
#include "subject.h"
#include "JsonSerializer.h"

template <class T>
class reactive_command
{
private:
	subject<T> m_subject;
public:
	reactive_command():
		m_subject(){};
	virtual ~reactive_command() = default;
	reactive_command(const reactive_command& val) = default;
	reactive_command(reactive_command&& val) = default;
	reactive_command& operator=(const reactive_command& val) = default;
	reactive_command& operator=(reactive_command&& val) = default;

	template<class... Args>
	auto subscribe(Args&&... args) const -> sigslot::connection
	{
		return m_subject.subscribe(std::forward<Args>(args)...);
	}

	void execute(const T& value) const
	{
		m_subject.on_next(value);
	}
};

template<class T> using reactive_command_ptr = std::shared_ptr<reactive_command<T>>;

template<class T, class... Args>
auto make_reactive_command(Args&&... args) -> reactive_command_ptr<T>
{
	return std::make_shared<reactive_command<T>>(std::forward<Args>(args)...);
}