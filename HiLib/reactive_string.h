#pragma once
#include "reactive_container.h"
#include <string>

template<typename T,
	class Traits = std::char_traits<T>,
	class Allocator = std::allocator<T>>
	class reactive_basic_string : public reactive_container<std::basic_string<T, Traits, Allocator>>
{
public:
	using base = typename reactive_container<std::basic_string<T, Traits, Allocator>>;
	using string_type = typename std::basic_string<T, Traits, Allocator>;
	using container_type = base::container_type;
	using value_type = base::value_type;
	using size_type = base::size_type;
	using notify_type = base::notify_type;
	using const_pointer = base::const_pointer;

	using base::reactive_container;

	/*********/
	/* Const */
	/*********/
	container_type substr(size_type pos = 0, size_type n = npos) const
	{
		return this->m_value.substr(pos, n);
	}

	const_pointer c_str() const noexcept
	{
		return this->m_value.c_str();
	}

	/***********/
	/* Unconst */
	/***********/

	reactive_basic_string& erase(size_type index = 0, size_type count = npos)
	{
		container_type old(this->m_value.substr(index, count));
		this->m_value.erase(index, count);
		this->m_subject.on_next(notify_type
		{
			notify_container_changed_action::erase,
			string_type(),
			-1,
			old,
			(int)index,
			this->get_const()
		});
		return *this;
	}

	reactive_basic_string& replace(size_type pos, size_type count, const container_type& value)
	{
		container_type old(this->m_value.substr(pos, count));
		this->m_value.replace(pos, count, value);
		this->m_subject.on_next(notify_type
		{
			notify_container_changed_action::replace,
			value,
			(int)pos,
			old,
			(int)pos,
			this->get_const()
		});
		return *this;
	}

	reactive_basic_string& replace(size_type pos, size_type n1, const value_type* s, size_type n2)
	{
		string_type neww(s, n2);
		string_type old(this->m_value.substr(pos, n1));
		this->m_value.replace(pos, n1, s, n2);
		this->m_pSubject->OnNext(notify_type{
			notify_container_changed_action::Replace,
			neww,
			old,
			(int)pos,
			(int)pos
		});
		return *this;
	}

	void observe_string(const notify_type& notify)
	{
		if (this->get_const() == notify.all_items) { return; }

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
		this->set(boost::lexical_cast<container_type>(value));
	}


	friend void to_json(json& j, const reactive_basic_string& o)
	{
		j = {
			{"Value", o.get_const()}
		};
	}

	friend void from_json(const json& j, reactive_basic_string& o)
	{
		container_type value;
		o.set(j.at("Value").get_to(value));
	}
};

template<typename T,
	class Traits = std::char_traits<T>,
	class Allocator = std::allocator<T>>
	using reactive_basic_string_ptr = std::shared_ptr<reactive_basic_string<T, Traits, Allocator>>;
using reactive_wstring = reactive_basic_string<wchar_t>;
using reactive_wstring_ptr = reactive_basic_string_ptr<wchar_t>;

template<class... Args>
auto make_reactive_wstring(Args&&... args) -> reactive_wstring_ptr
{
	return std::make_shared<reactive_wstring>(std::forward<Args>(args)...);
}