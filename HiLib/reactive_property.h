#pragma once
#include "subject.h"
#include "notify_container_changed.h"
#include "reactive_string.h"
#include "JsonSerializer.h"

template <class T>
class reactive_property
{
private:
	subject<T> m_subject;
	T m_value;
public:
	explicit reactive_property(const T& value = T())
		: m_value(value) {}

	explicit reactive_property(T&& value)
		: m_value(std::forward<T>(value)) {}

	virtual ~reactive_property() = default;

	auto operator<=>(const reactive_property& rhs) const
	{
		return this->get_const() <=> rhs.get_const();
	}
	bool operator==(const reactive_property& rhs) const
	{
		return this->get_const() == rhs.get_const();
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


	const T& get_const() const 
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
			m_subject.on_next(value);
		}
	}

	//template<class U, class Traits, class Allocator>
	void observe_string(reactive_wstring::notify_type& notify)
	{
		this->set(boost::lexical_cast<T>(notify.all_items));
	}

	friend void to_json(json& j, const reactive_property<T>& o)
	{
		j = {
			{"Value", o.get_const()}
		};
	}

	friend void from_json(const json& j, reactive_property<T>& o)
	{
		T value;
		o.set(j.at("Value").get_to(value));
		//o.set(value);
	}
};

template<class T> using reactive_property_ptr = std::shared_ptr<reactive_property<T>>;

template<class T, class... Args>
auto make_reactive_property(Args&&... args) -> reactive_property_ptr<T>
{
	return std::make_shared<reactive_property<T>>(std::forward<Args>(args)...);
}

template<class T>
auto operator<=>(const reactive_property_ptr<T>& lhs, const reactive_property_ptr<T>& rhs)
{
	return *lhs.get() <=> *rhs.get();
}


template<class T>
bool operator==(const reactive_property_ptr<T>& lhs, const reactive_property_ptr<T>& rhs)
{
	return *lhs.get() == *rhs.get();
}
