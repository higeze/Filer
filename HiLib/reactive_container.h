#pragma once
#include "notify_container_changed.h"
#include "subject.h"
#include "JsonSerializer.h"
#include "Debug.h"

template<class TContainer>
class reactive_container
{
public:
	using subject_type = typename subject<notify_container_changed_event_args<TContainer>>;
	using container_type = typename TContainer;
	using value_type = typename TContainer::value_type;
	using notify_type = typename notify_container_changed_event_args<TContainer>;
	using size_type = typename TContainer::size_type;
	using iterator = typename TContainer::iterator;
	using const_iterator = typename TContainer::const_iterator;
	using const_reference = typename TContainer::const_reference;
	using const_pointer = typename TContainer::const_pointer;
	using reference = typename TContainer::reference;

	subject_type m_subject;
	container_type m_value;

public:

	explicit reactive_container()
		: m_subject(), 
		m_value(){};

	template<class... Args>
	explicit reactive_container(const Args&... args)
		: m_subject(), 
		m_value(args...){};

	template<class... Args>
	explicit reactive_container(Args&&... args)
		: m_subject(), 
		m_value(std::forward<Args>(args)...){};

	virtual ~reactive_container() = default;

	reactive_container(const reactive_container&) = default;
	reactive_container& operator=(const reactive_container&) = default;
	reactive_container(reactive_container&&) noexcept = default;
	reactive_container& operator=(reactive_container&&) noexcept = default;

	//const container_type& operator*() const noexcept
	//{
	//	return m_pvalue.operator*();
	//}

	//const container_type* operator->() const noexcept
	//{
	//	return m_pvalue.operator->();
	//}

	//container_type* get_unconst() const noexcept
	//{
	//	return m_pvalue.operator->();
	//}

	//auto operator<=>(const reactive_container_ptr& rhs) const
	//{
	//	return *m_pvalue <=> *rhs.m_pvalue;
	//}
	//bool operator==(const reactive_container_ptr& rhs) const
	//{
	//	return *m_pvalue == *rhs.m_pvalue;
	//}

	//void disconnect_all()
	//{
	//	this->m_psubject->disconnect_all();
	//	this->m_pvalue = std::make_shared<container_type>(*m_pvalue);
	//}

	//void block_subject()
	//{
	//	m_psubject->block();
	//}

	//void unblock_subject()
	//{
	//	m_psubject->unblock();
	//}

	void observe_container(const container_type& value)
	{
		if (m_value != value) {
			container_type old(m_value);
			m_value.assign(value.cbegin(), value.cend());
			m_subject.on_next(notify_type
			{
				notify_container_changed_action::reset,
				m_value,
				0,
				old,
				0,
				m_value
			});
		}
	}
	/***********/
	/* Unconst */
	/***********/
	
	void assign(const container_type& value)
	{
		if (m_value != value) {
			m_value = value;
			m_subject.on_next(value);
		}
	}

	void push_back(const value_type& x)
	{
		this->m_value.push_back(x);
		this->m_subject.on_next(notify_type
		{
			notify_container_changed_action::push_back,
			{ this->m_value.back() },
			(int)m_value.size() - 1,
			{},
			-1,
			this->m_value
		});
	}

	auto insert(size_type index, const container_type& value)
	{
		auto ret = this->m_value.insert(index, value);
		this->m_subject.on_next(notify_type
		{
			notify_container_changed_action::insert,
			value,
			(int)index, 
			container_type(),
			-1,
			this->m_value
		});
		return ret;
	}

	iterator insert(const_iterator position, const value_type& x)
	{
		auto ret = this->m_value.insert(position, x);
		auto index = std::distance(this->m_value.begin(), ret);
		this->m_subject.on_next(notify_type
		{
			notify_container_changed_action::insert,
			{ *ret },
			index,
			{},
			-1,
			this->m_value
		});
		return ret;
	}

	iterator replace(iterator position, const value_type& x)
	{
		auto oldItem = *position;
		*position = x;
		auto index = std::distance(this->m_value.begin(), position);
		this->m_subject.on_next(notify_type
		{
			notify_container_changed_action::replace,
			{ *position },
			index,
			{ oldItem},
			-1,
			this->m_value
		});
		return position;
	}

	iterator erase(const_iterator where)
	{
		auto oldItem = *where;
		auto index = std::distance(this->m_value.cbegin(), where);
		iterator ret = this->m_value.erase(where);
		this->m_subject.on_next(notify_type
		{
			notify_container_changed_action::erase,
			{},
			-1,
			{ oldItem },
			index,
			this->m_value
		});
		return ret;
	}

	iterator erase(const_iterator first, const_iterator last)
	{
		auto oldItems = container_type(first, last);
		auto index = std::distance(this->m_value.cbegin(), first);
		iterator ret = this->m_value.erase(first, last);
		this->m_subject.on_next(notify_type
		{
			notify_container_changed_action::erase,
			{},
			-1,
			oldItems,
			index,
			this->m_value
		});
		return ret;
	}


	void clear()
	{
		auto old = this->m_value;
		this->m_value.clear();
		this->m_subject.on_next(notify_type
		{
			notify_container_changed_action::reset,
			{},
			-1,
			old,
			0,
			this->m_value
		});
		return;
	}
};

template<class TReactiveContainer>
class reactive_container_ptr
{
public:
	using reactive_container_type = typename TReactiveContainer;
	using container_type = reactive_container_type::container_type;
	using subject_type = typename subject<notify_container_changed_event_args<container_type>>;
	using value_type = typename container_type::value_type;
	using notify_type = typename notify_container_changed_event_args<container_type>;
	using size_type = typename container_type::size_type;
	using iterator = typename container_type::iterator;
	using const_iterator = typename container_type::const_iterator;
	using const_reference = typename container_type::const_reference;
	using const_pointer = typename container_type::const_pointer;
	using reference = typename container_type::reference;
protected:
	std::shared_ptr<reactive_container_type> m_preactive;

public:
	explicit reactive_container_ptr()
		: m_preactive(std::make_shared<TReactiveContainer>()){};

	template<class... Args>
	explicit reactive_container_ptr(const Args&... args)
		: m_preactive(std::make_shared<TReactiveContainer>(args...)){};

	template<class... Args>
	explicit reactive_container_ptr(Args&&... args)
		: m_preactive(std::make_shared<TReactiveContainer>(std::forward<Args>(args)...)){}

	virtual ~reactive_container_ptr() = default;

	reactive_container_ptr(const reactive_container_ptr&) = default;
	reactive_container_ptr& operator=(const reactive_container_ptr&) = default;
	reactive_container_ptr(reactive_container_ptr&&) noexcept = default;
	reactive_container_ptr& operator=(reactive_container_ptr&&) noexcept = default;

	const container_type& operator*() const noexcept
	{
		return m_preactive->m_value;
	}

	const container_type* operator->() const noexcept
	{
		return &m_preactive->m_value;
	}

	container_type* get_unconst() const noexcept
	{
		return &m_preactive->m_value;
	}

	auto operator<=>(const reactive_container_ptr& rhs) const
	{
		return m_preactive <=> rhs.m_preactive;
	}
	bool operator==(const reactive_container_ptr& rhs) const
	{
		return m_preactive == rhs.m_preactive;
	}

	explicit operator bool() const noexcept
	{
		return static_cast<bool>(m_preactive);
	}

	template<class... Args>
	auto subscribe(Args&&... args) -> sigslot::connection
	{
		return m_preactive->m_subject.subscribe(std::forward<Args>(args)...);
	}

	template<class... Args>
	auto disconnect(Args&&... args) -> size_t
	{
		return m_preactive->m_subject.disconnect(std::forward<Args>(args)...);
	}

	void set(const container_type& value)
	{
		m_preactive->observe_container(value);
	}

	void assign(const container_type& value)
	{
		this->set(value);
	}

	void push_back(const value_type& value)
	{
		return m_preactive->push_back(value);
	}

	auto insert(size_type index, const container_type& value)
	{
		return m_preactive->insert(index, value);
	}

	iterator insert(const_iterator position, const value_type& value)
	{
		return m_preactive->insert(position, value);
	}

	iterator replace(iterator position, const value_type& value)
	{
		return m_preactive->replace(position, value);
	}

	iterator erase(const_iterator where)
	{
		return m_preactive->erase(where);
	}

	iterator erase(const_iterator first, const_iterator last)
	{
		return m_preactive->erase(first, last);
	}


	void clear()
	{
		return m_preactive->clear();
	}


};



