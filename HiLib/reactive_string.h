#pragma once
#include "reactive_container.h"
#include <string>


template<class _Elem,
	class _Traits = std::char_traits<_Elem>,
	class _Alloc = std::allocator<_Elem>>
class reactive_basic_string
{
public:
	using container_type = typename std::basic_string<_Elem, _Traits, _Alloc>;
	using notify_type = typename notify_container_changed_event_args<container_type>;
	using subject_type = typename subject<notify_type>;

	using value_type = container_type::value_type;
	using size_type = container_type::size_type;

	using iterator = container_type::iterator;
	using const_iterator = container_type::const_iterator;
	using reference = container_type::reference;
	using const_reference = container_type::const_reference;
	using const_pointer = container_type::const_pointer;

public:

	subject_type m_subject;
	container_type m_value;
public:
	explicit reactive_basic_string()
		: m_subject(), 
		m_value(){};

	template<class... Args>
	explicit reactive_basic_string(const Args&... args)
		: m_subject(), 
		m_value(args...){};

	template<class... Args>
	explicit reactive_basic_string(Args&&... args)
		: m_subject(), 
		m_value(std::forward<Args>(args)...){};

	virtual ~reactive_basic_string() = default;

	reactive_basic_string(const reactive_basic_string&) = default;
	reactive_basic_string& operator=(const reactive_basic_string&) = default;
	reactive_basic_string(reactive_basic_string&&) noexcept = default;
	reactive_basic_string& operator=(reactive_basic_string&&) noexcept = default;

	void set(const container_type& value)
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
		set(value);
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

	container_type erase(size_type index = 0, size_type count = npos)
	{
		container_type old(this->m_value.substr(index, count));
		auto ret = this->m_value.erase(index, count);
		this->m_subject.on_next(notify_type
		{
			notify_container_changed_action::erase,
			{},
			-1,
			old,
			(int)index,
			this->m_value
		});
		return ret;
	}

	container_type replace(size_type pos, size_type count, const container_type& value)
	{
		container_type old(this->m_value.substr(pos, count));
		auto ret = this->m_value.replace(pos, count, value);
		this->m_subject.on_next(notify_type
		{
			notify_container_changed_action::replace,
			value,
			(int)pos,
			old,
			(int)pos,
			this->m_value
		});
		return ret;
	}

	container_type replace(size_type pos, size_type n1, const value_type* s, size_type n2)
	{
		container_type neww(s, n2);
		container_type old(this->m_value.substr(pos, n1));
		auto ret = this->m_value.replace(pos, n1, s, n2);
		this->m_subject.on_next(notify_type
		{
			notify_container_changed_action::Replace,
			neww,
			old,
			(int)pos,
			(int)pos,
			this->m_value
		});
		return ret;
	}

	void observe_string(notify_type notify)
	{
		if (this->m_value == notify.all_items) { return; }

		switch (notify.action) {
			case notify_container_changed_action::insert://new,null,idx,-1
				this->insert(notify.new_starting_index, notify.new_items);
				break;
			case notify_container_changed_action::erase://null,old,-1, idx
				this->erase(notify.old_starting_index,notify.old_items.size());
				break;
			case notify_container_changed_action::replace://new,old,idx,idx
				this->replace(notify.old_starting_index, notify.old_items.size(), notify.new_items);
				break;
			case notify_container_changed_action::reset://new,old,0,0
				this->assign(notify.new_items);
				break;
			default:
				break;
		}
	}

	template<class U>
	void observe_property(const U& value)
	{
		container_type str_val = std::to_wstring(value);
		if (m_value != str_val) {
			container_type old(m_value);
			m_value.assign(str_val.cbegin(), str_val.cend());
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
};

template<class _Elem,
	class _Traits = std::char_traits<_Elem>,
	class _Alloc = std::allocator<_Elem>>
class reactive_basic_string_ptr
{
	template <class U> friend class reactive_property_ptr;
public:
	using reactive_container_type = typename reactive_basic_string<_Elem, _Traits, _Alloc>;

	using container_type = reactive_container_type::container_type;
	using notify_type = reactive_container_type::notify_type;
	using subject_type = reactive_container_type::subject_type;

	using value_type = reactive_container_type::value_type;
	using size_type = reactive_container_type::size_type;

	using iterator = reactive_container_type::iterator;
	using const_iterator = reactive_container_type::const_iterator;
	using reference = reactive_container_type::reference;
	using const_reference = reactive_container_type::const_reference;
	using const_pointer = reactive_container_type::const_pointer;

protected:
	std::shared_ptr<reactive_container_type> m_preactive;

public:
	explicit reactive_basic_string_ptr()
		: m_preactive(std::make_shared<reactive_container_type>()){};

	template<class... Args>
	explicit reactive_basic_string_ptr(const Args&... args)
		: m_preactive(std::make_shared<reactive_container_type>(args...)){};

	template<class... Args>
	explicit reactive_basic_string_ptr(Args&&... args)
		: m_preactive(std::make_shared<reactive_container_type>(std::forward<Args>(args)...)){}

	virtual ~reactive_basic_string_ptr() = default;

	reactive_basic_string_ptr(const reactive_basic_string_ptr&) = default;
	reactive_basic_string_ptr& operator=(const reactive_basic_string_ptr&) = default;
	reactive_basic_string_ptr(reactive_basic_string_ptr&&) noexcept = default;
	reactive_basic_string_ptr& operator=(reactive_basic_string_ptr&&) noexcept = default;

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

	auto operator<=>(const reactive_basic_string_ptr& rhs) const
	{
		return m_preactive <=> rhs.m_preactive;
	}
	bool operator==(const reactive_basic_string_ptr& rhs) const
	{
		return m_preactive == rhs.m_preactive;
	}

	explicit operator bool() const noexcept
	{
		return m_preactive.operator bool();
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
		this->m_preactive->set(value);
	}

	void assign(const container_type& value)
	{
		this->m_preactive->set(value);
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

	container_type erase(size_type index = 0, size_type count = npos)
	{
		return this->m_preactive->erase(index, count);
	}

	container_type replace(size_type pos, size_type count, const container_type& value)
	{
		return this->m_preactive->replace(pos, count, value);
	}

	container_type replace(size_type pos, size_type n1, const value_type* s, size_type n2)
	{
		return this->m_preactive->replace(pos, n1, s, n2);
	}

	void unbinding(reactive_basic_string_ptr& dst)
	{
		dst.disconnect(&reactive_container_type::observe_string, this->m_preactive);
		this->disconnect(&reactive_container_type::observe_string, dst.m_preactive);
	}

	std::pair<sigslot::connection, sigslot::connection> binding(reactive_basic_string_ptr& dst)
	{
		unbinding(dst);

		dst.set(this->operator*());

		return std::make_pair(
			dst.subscribe(&reactive_container_type::observe_string, this->m_preactive),
			this->subscribe(&reactive_container_type::observe_string, dst.m_preactive));
	}

	//template <class U>
	//std::pair<sigslot::connection, sigslot::connection> binding(reactive_property_ptr<U>& dst)
	//{
	//	dst.set(boost::lexical_cast<U>(this->m_preactive->m_value));

	//	dst.disconnect(&reactive_property<T>::template observe_string<U, Traits, Allocator>, this->m_preactive);
	//	this->disconnect(&reactive_basic_string<U, Traits, Allocator>::template observe_property<T>, dst.m_preactive);

	//	return std::make_pair(
	//		dst.subscribe(&reactive_property<T>::template observe_string<U, Traits, Allocator>, this->m_preactive),
	//		this->subscribe(&reactive_basic_string<U, Traits, Allocator>::template observe_property<T>, dst.m_preactive));
	//}

public:
	friend void to_json(json& j, const reactive_basic_string_ptr& ptr)
	{
		json_safe_to(j, "Value", *ptr);
	}

	friend void from_json(const json& j, reactive_basic_string_ptr& ptr)
	{
		container_type value;
		ptr.set(json_safe_from(j, "Value", value));
	}
};

using reactive_wstring_ptr = reactive_basic_string_ptr<wchar_t>;