#pragma once
#include "notify_container_changed.h"
#include "notify_property_changed.h"
#include "subject.h"
#include "JsonSerializer.h"
#include "Debug.h"
#include <vector>

class CFavorite;

template<typename T>
struct adl_vector_item
{
	static T clone(const T& item) { return T(item); }
	static void bind(T& src, T& dst) { return src.binding(dst); }
};

template<typename V>
typename std::enable_if_t<std::is_base_of_v<notify_property_changed, V>>
bind_value(V& left, V& right)
{
	left.bind<V>(right);
}

template<typename V>
typename std::enable_if_t<!std::is_base_of_v<notify_property_changed, V>>
bind_value(V& left, V& right)
{
	adl_vector_item<V>::bind(left, right);
}

template<class _Ty, class _Alloc = std::allocator<_Ty>>
class reactive_vector
{
public:
	using container_type = typename std::vector<_Ty, _Alloc>;
	using notify_type = typename notify_container_changed_event_args<std::vector, _Ty, _Alloc>;
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
	explicit reactive_vector()
		: m_subject(), 
		m_value(){};

	template<class... Args>
	explicit reactive_vector(const Args&... args)
		: m_subject(), 
		m_value(args...){};

	template<class... Args>
	explicit reactive_vector(Args&&... args)
		: m_subject(), 
		m_value(std::forward<Args>(args)...){};

	virtual ~reactive_vector() = default;

	reactive_vector(const reactive_vector&) = default;
	reactive_vector& operator=(const reactive_vector&) = default;
	reactive_vector(reactive_vector&&) noexcept = default;
	reactive_vector& operator=(reactive_vector&&) noexcept = default;

	/***********/
	/* Unconst */
	/***********/

	value_type& at(size_type index)
	{
		return this->m_value.at(index);
	}

	value_type& front()
	{
		return this->m_value.front();
	}

	value_type& back()
	{
		return this->m_value.back();
	}

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

	iterator insert(const_iterator position, const_iterator first, const_iterator last)
	{
		auto ret = this->m_value.insert(position, first, last);
		auto index = std::distance(this->m_value.begin(), ret);
		this->m_subject.on_next(notify_type
		{
			notify_container_changed_action::insert,
			container_type(first, last),
			index, 
			{},
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

	const_iterator find(const value_type& value) const
	{
		auto iter = std::ranges::find(this->m_value, value);
		return iter;
	}

	template<class Predicate>
	const_iterator find_if(Predicate pred) const
	{
		auto iter = std::ranges::find_if(this->m_value, pred, {});
		return iter;
	}

	const_iterator cbegin() const
	{
		return this->m_value.cbegin();
	}

	const_iterator cend() const
	{
		return this->m_value.cend();
	}

    template <class... Args>
	void emplace_back(Args&&... args)
	{
		this->m_value.emplace_back(std::forward<Args>(args)...);
		this->m_subject.on_next(notify_type
		{
			notify_container_changed_action::push_back,
			{ this->m_value.back() },
			(int)this->m_value.size() - 1,
			{},
			-1,
			this->m_value
		});
	}

	void observe_vector(notify_type& notify)
	{
		//if (this->m_value.size() == notify.all_items.size()){ 
		//	return;
		//}

		//::OutputDebugStringW(std::format(L"notify Address: {}, Size: {}\n", reinterpret_cast<void*>(&(notify.all_items)), notify.all_items.size()).c_str());
		//
		//if constexpr (std::is_same_v < value_type, CFavorite>) {
		//	for (auto iter = notify.all_items.cbegin(); iter != notify.all_items.cend(); ++iter) {
		//		::OutputDebugStringW(std::format(L"{}\n", (*iter->Path)).c_str());
		//	}
		//}
		//::OutputDebugStringW(std::format(L"this Address: {}, Size: {}\n", reinterpret_cast<void*>(&m_value), m_value.size()).c_str());
		//if constexpr (std::is_same_v < value_type, CFavorite>) {
		//	for (auto iter = m_value.cbegin(); iter != m_value.cend(); ++iter) {
		//		::OutputDebugStringW(std::format(L"{}\n", (*iter->Path)).c_str());
		//	}
		//}

		if (this->m_value == notify.all_items) { return; }

		switch (notify.action) {
			case notify_container_changed_action::push_back:
				this->push_back(adl_vector_item<value_type>::clone(notify.all_items.back()));
					bind_value(notify.all_items.back(), this->m_value.back());
				break;
			case notify_container_changed_action::insert:
				for (auto i = 0; i != notify.new_items.size(); i++) {
					this->insert(this->m_value.cbegin() + notify.new_starting_index + i, adl_vector_item<value_type>::clone(notify.all_items.at(notify.new_starting_index + i)));
					bind_value(notify.all_items.at(notify.new_starting_index + i), this->m_value.at(notify.new_starting_index + i));
				}
				break;
			case notify_container_changed_action::Move:
				THROW_FILE_LINE_FUNC;
				break;
			case notify_container_changed_action::erase:
				this->erase(this->m_value.cbegin() + notify.old_starting_index);
				break;
			case notify_container_changed_action::replace:
				THROW_FILE_LINE_FUNC;
				break;
			case notify_container_changed_action::reset:
				for (size_t i = 0; i < notify.all_items.size(); i++) {
					this->push_back(adl_vector_item<value_type>::clone(notify.all_items[i]));
					bind_value(notify.all_items[i], this->m_value.at(i));
				}
				break;
		}
	}
};

template<class _Ty, class _Alloc = std::allocator<_Ty>>
class reactive_vector_ptr
{
public:
	using reactive_container_type = typename reactive_vector<_Ty, _Alloc>;

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
	explicit reactive_vector_ptr()
		: m_preactive(std::make_shared<reactive_container_type>()){};

	template<class... Args>
	explicit reactive_vector_ptr(const Args&... args)
		: m_preactive(std::make_shared<reactive_container_type>(args...)){};

	template<class... Args>
	explicit reactive_vector_ptr(Args&&... args)
		: m_preactive(std::make_shared<reactive_container_type>(std::forward<Args>(args)...)){}

	virtual ~reactive_vector_ptr() = default;

	reactive_vector_ptr(const reactive_vector_ptr&) = default;
	reactive_vector_ptr& operator=(const reactive_vector_ptr&) = default;
	reactive_vector_ptr(reactive_vector_ptr&&) noexcept = default;
	reactive_vector_ptr& operator=(reactive_vector_ptr&&) noexcept = default;

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

	auto operator<=>(const reactive_vector_ptr& rhs) const
	{
		return m_preactive <=> rhs.m_preactive;
	}
	bool operator==(const reactive_vector_ptr& rhs) const
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

	/***********/
	/* Unconst */
	/***********/
	value_type& at(size_type index)
	{
		return this->m_preactive->at(index);
	}

	value_type& front()
	{
		return this->m_preactive->front();
	}

	value_type& back()
	{
		return this->m_preactive->back();
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

	iterator insert(const_iterator position, const_iterator first, const_iterator last)
	{
		return m_preactive->insert(position, first, last);
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


    template <class... Args>
	void emplace_back(Args&&... args)
	{
		return this->m_preactive->emplace_back(std::forward<Args>(args)...);
	}

	const_iterator find(const value_type& value) const
	{
		return m_preactive->find(value);
	}

	template<class Predicate>
	const_iterator find_if(Predicate pred) const
	{
		return m_preactive->find_if(pred);
	}


	const_iterator cbegin() const
	{
		return m_preactive->cbegin();
	}

	const_iterator cend() const
	{
		return m_preactive->cend();
	}

	std::pair<sigslot::connection, sigslot::connection> binding(reactive_vector_ptr& dst)
	{
		dst.disconnect(&reactive_vector<_Ty, _Alloc>::observe_vector, this->m_preactive);
		this->disconnect(&reactive_vector<_Ty, _Alloc>::observe_vector, dst.m_preactive);

		dst.clear();
		for (size_t i = 0; i < this->m_preactive->m_value.size(); i++) {
			dst.push_back(adl_vector_item<_Ty>::clone(this->m_preactive->m_value.at(i)));
			bind_value(this->get_unconst()->at(i), dst.get_unconst()->at(i));
		}

		return std::make_pair(
			dst.subscribe(&reactive_vector<_Ty, _Alloc>::observe_vector, this->m_preactive),
			this->subscribe(&reactive_vector<_Ty, _Alloc>::observe_vector, dst.m_preactive));
	}
public:
	friend void to_json(json& j, const reactive_vector_ptr& ptr)
	{
		json_safe_to(j, "Value", *ptr);
	}

	friend void from_json(const json& j, reactive_vector_ptr& ptr)
	{
		container_type value;
		ptr.set(json_safe_from(j, "Value", value));
	}

};