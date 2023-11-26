#pragma once
#include "subject.h"
#include "JsonSerializer.h"

template <class T>
class reactive_command
{
	template <class U> friend class reactive_command_ptr;
private:
	subject<T> m_subject;
public:
	reactive_command():
		m_subject(){};
	template<class... Args>
	explicit reactive_command(Args&&... args)
		: m_subject()
	{
		m_subject.subscribe(std::forward<Args>(args)...);
	}
	virtual ~reactive_command() = default;

	reactive_command(const reactive_command& val) = default;
	reactive_command(reactive_command&& val) = default;
	reactive_command& operator=(const reactive_command& val) = default;
	reactive_command& operator=(reactive_command&& val) = default;

	void execute(const T& value) const
	{
		m_subject.on_next(value);
	}
};

template <>
class reactive_command<void>
{
	template <class U> friend class reactive_command_ptr;
private:
	subject<> m_subject;
public:
	reactive_command(): m_subject(){};

	template<class... Args>
	explicit reactive_command(Args&&... args) : reactive_command()
	{
		m_subject.subscribe(std::forward<Args>(args)...);
	}

	virtual ~reactive_command() = default;

	reactive_command(const reactive_command& val) = default;
	reactive_command(reactive_command&& val) = default;
	reactive_command& operator=(const reactive_command& val) = default;
	reactive_command& operator=(reactive_command&& val) = default;

	void execute(void) const
	{
		m_subject.on_next();
	}
};

template <class T>
class reactive_command_ptr
{
public:
	using reactive_command_type = typename reactive_command<T>;
private:
	std::shared_ptr<reactive_command_type> m_preactive;
public:
	reactive_command_ptr()
		:m_preactive(std::make_shared<reactive_command_type>()){}

	template<class... Args>
	explicit reactive_command_ptr(Args&&... args)
		: m_preactive(std::make_shared<reactive_command_type>(std::forward<Args>(args)...)) {}

	virtual ~reactive_command_ptr() = default;

	reactive_command_ptr(const reactive_command_ptr& val) = default;
	reactive_command_ptr(reactive_command_ptr&& val) = default;
	reactive_command_ptr& operator=(const reactive_command_ptr& val) = default;
	reactive_command_ptr& operator=(reactive_command_ptr&& val) = default;

	template<class... Args>
	auto subscribe(Args&&... args) -> sigslot::connection
	{
		return m_preactive->m_subject.subscribe(std::forward<Args>(args)...);
	}

	void execute(const T& value) const
	{
		m_preactive->m_subject.on_next(value);
	}

	sigslot::connection binding(reactive_command_ptr& dst)
	{
		return dst.subscribe(&reactive_command_type::execute, m_preactive);
	}
};

template <>
class reactive_command_ptr<void>
{
public:
	using reactive_command_type = typename reactive_command<void>;
private:
	std::shared_ptr<reactive_command_type> m_preactive;
public:
	reactive_command_ptr()
		:m_preactive(std::make_shared<reactive_command_type>()){}

	template<class... Args>
	explicit reactive_command_ptr(Args&&... args)
		: m_preactive(std::make_shared<reactive_command_type>(std::forward<Args>(args)...)) {}

	virtual ~reactive_command_ptr() = default;

	reactive_command_ptr(const reactive_command_ptr& val) = default;
	reactive_command_ptr(reactive_command_ptr&& val) = default;
	reactive_command_ptr& operator=(const reactive_command_ptr& val) = default;
	reactive_command_ptr& operator=(reactive_command_ptr&& val) = default;

	template<class... Args>
	auto subscribe(Args&&... args) -> sigslot::connection
	{
		return m_preactive->m_subject.subscribe(std::forward<Args>(args)...);
	}

	void execute(void) const
	{
		m_preactive->m_subject.on_next();
	}

	sigslot::connection binding(reactive_command_ptr& dst)
	{
		return dst.subscribe(&reactive_command_type::execute, m_preactive);
	}
};

