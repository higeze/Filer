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

protected:
	subject_type m_subject;
	container_type m_value;

public:
	explicit reactive_container(const container_type& value = container_type())
		:m_subject(), m_value(value){};

	virtual ~reactive_container() = default;
	auto operator<=>(const reactive_container& rhs) const
	{
		return this->get_const() <=> rhs.get_const();
	}
	bool operator==(const reactive_container& rhs) const
	{
		return this->get_const() == rhs.get_const();
	}
	reactive_container(const reactive_container& val) = delete;
	reactive_container& operator=(const reactive_container& val) = delete;
	reactive_container(reactive_container&& val) noexcept = default;
	reactive_container& operator=(reactive_container&& val) noexcept = default;

	template<class... Args>
	auto subscribe(Args&&... args) -> sigslot::connection
	{
		return m_subject.subscribe(std::forward<Args>(args)...);
	}
	//template<typename Pmf, typename Ptr>
	//auto subscribe(Pmf&& pmf, Ptr&& ptr)->sigslot::connection
	//{
	//	return m_subject.subscribe(std::forward<Pmf>(pmf), std::forward<Ptr>(ptr));
	//}
	const container_type& get_const() const 
	{ 
		return m_value;
	}
	container_type& get_unconst() 
	{ 
		return m_value;
	}

	void block_subject()
	{
		m_subject.block();
	}

	void unblock_subject()
	{
		m_subject.unblock();
	}

	void set(const container_type& value)
	{
		if (this->get_const() != value) {
			container_type old(this->get_const());
			this->m_value.assign(value.cbegin(), value.cend());
			this->m_subject.on_next(notify_type
			{
				notify_container_changed_action::reset,
				this->get_const(),
				0,
				old,
				0,
				this->get_const()
			});
		}
	}

	/*********/
	/* Const */
	/*********/

	const_reference operator[](size_type pos) const noexcept
	{
		return this->m_value.operator[](pos);
	}

	const_reference at(size_type pos) const
	{
		return this->m_value.at(pos);
	}

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

	const_reference front() const
	{
		return this->m_value.front();
	}
	
	const_reference back() const
	{
		return this->m_value.back();
	}

	const_pointer data() const noexcept
	{
		return this->m_value.data();
	}

	/***********/
	/* Unconst */
	/***********/
	
	void assign(const container_type& value)
	{
		this->set(value);
	}

	void push_back(const value_type& x)
	{
		this->m_value.push_back(x);
		this->m_subject.on_next(notify_type
		{
			notify_container_changed_action::push_back,
			{ this->m_value.back() },
			(int)size() - 1,
			{},
			-1,
			this->get_const()
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
			this->get_const()
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
			this->get_const()
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
			this->get_const()
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
			this->get_const()
		});
		return ret;
	}

	iterator erase(const_iterator first, const_iterator last)
	{
		auto oldItems = container_type(first, last);
		auto index = std::distance(this->m_value.cbegin(), first);
		iterator ret = this->m_value.erase(first, last);
		this->m_subject->on_next(notify_type
		{
			notify_container_changed_action::erase,
			{},
			-1,
			oldItems,
			index,
			this->get_const()
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
			this->get_const()
		});
		return;
	}

	friend void to_json(json& j, const reactive_container& o)
	{
		j = {
			{"Value", o.get_const()}
		};
	}

	friend void from_json(const json& j, reactive_container& o)
	{
		container_type value;
		o.set(j.at("Value").get_to(value));
	}
};


