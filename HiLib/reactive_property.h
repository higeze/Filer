#pragma once
#include "subject.h"
#include "notify_container_changed.h"
#include "reactive_string.h"
#include "JsonSerializer.h"

template<class T>
class reactive_property
{
	template <class U, class Traits, class Allocator> friend class reactive_basic_string_ptr;
	template <class U> friend class reactive_property_ptr;
private:
	subject<T> m_subject;
	T m_value;
public:
	explicit reactive_property()
		: m_subject(),
		m_value() {}

	template<class... Args>
	explicit reactive_property(const Args&... args)
		: m_subject(),
		m_value(args...) {}

	template<class... Args>
	explicit reactive_property(Args&&... args)
		: m_subject(),
		m_value(std::forward<Args>(args)...) {}

	virtual ~reactive_property() = default;

	reactive_property(const reactive_property&) = default;
	reactive_property& operator=(const reactive_property&) = default;
	reactive_property(reactive_property&&) noexcept = default;
	reactive_property& operator=(reactive_property&&) noexcept = default;

	void observe_property(const T& value)
	{
		if (m_value != value) {
			m_value = value;
			m_subject.on_next(value);
		}
	}

	template<class _Elem, class _Traits, class _Alloc>
	void observe_string(reactive_basic_string_ptr<_Elem, _Traits, _Alloc>::notify_type& notify)
	{
		this->observe_property(boost::lexical_cast<T>(notify.all_items));
	}
};

template <class T>
class reactive_property_ptr
{
	template <class _Elem, class _Traits, class _Alloc> friend class reactive_basic_string_ptr;
private:
	std::shared_ptr<reactive_property<T>> m_preactive;
public:
	explicit reactive_property_ptr()
		: m_preactive(std::make_shared<reactive_property<T>>()) {}

	template<class... Args>
	explicit reactive_property_ptr(const Args&... args)
		: m_preactive(std::make_shared<reactive_property<T>>(args...)) {}

	template<class... Args>
	explicit reactive_property_ptr(Args&&... args)
		: m_preactive(std::make_shared<reactive_property<T>>(std::forward<Args>(args)...)) {}

	virtual ~reactive_property_ptr() = default;

	reactive_property_ptr(const reactive_property_ptr&) = default;
	reactive_property_ptr& operator=(const reactive_property_ptr&) = default;
	reactive_property_ptr(reactive_property_ptr&&) noexcept = default;
	reactive_property_ptr& operator=(reactive_property_ptr&&) noexcept = default;

	const std::shared_ptr<reactive_property<T>> life() const
	{
		return m_preactive;
	}

	const T& operator*() const noexcept
	{
		return m_preactive->m_value;
	}

	const T* operator->() const noexcept
	{
		return &m_preactive->m_value;
	}

	T* get_unconst() const noexcept
	{
		return &m_preactive->m_value;
	}

	auto operator<=>(const reactive_property_ptr& rhs) const
	{
		return m_preactive <=> rhs.m_preactive;
	}
	bool operator==(const reactive_property_ptr& rhs) const
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

	virtual void set(const T& value)
	{
		this->m_preactive->observe_property(value);
	}

	virtual void force_notify_set(const T& value)
	{
		this->m_preactive->m_value = value;
		this->m_preactive->m_subject.on_next(value);
	}

	void unbinding(reactive_property_ptr<T>& dst)
	{
		dst.disconnect(&reactive_property<T>::observe_property, this->m_preactive);
		this->disconnect(&reactive_property<T>::observe_property, dst.m_preactive);
	}

	std::pair<sigslot::connection, sigslot::connection> binding(reactive_property_ptr<T>& dst)
	{
		unbinding(dst);

		dst.force_notify_set(this->operator*());

		return std::make_pair(
			dst.subscribe(&reactive_property<T>::observe_property, this->m_preactive),
			this->subscribe(&reactive_property<T>::observe_property, dst.m_preactive));
	}

	template <class U, class Traits, class Allocator>
	void unbinding(reactive_basic_string_ptr<U, Traits, Allocator>& dst)
	{
		dst.disconnect(&reactive_property<T>::template observe_string<U, Traits, Allocator>, this->m_preactive);
		this->disconnect(&reactive_basic_string<U, Traits, Allocator>::template observe_property<T>, dst.m_preactive);
	}

	template <class U, class Traits, class Allocator>
	std::pair<sigslot::connection, sigslot::connection> binding(reactive_basic_string_ptr<U, Traits, Allocator>& dst)
	{
		unbinding(dst);

		dst.set(boost::lexical_cast<std::basic_string<U, Traits, Allocator>>(this->m_preactive->m_value));

		return std::make_pair(
			dst.subscribe(&reactive_property<T>::template observe_string<U, Traits, Allocator>, this->m_preactive),
			this->subscribe(&reactive_basic_string<U, Traits, Allocator>::template observe_property<T>, dst.m_preactive));
	}

	friend void to_json(json& j, const reactive_property_ptr<T>& o)
	{
		j = {{"Value", *o}};
	}

	friend void from_json(const json& j, reactive_property_ptr<T>& o)
	{
		T value;
		o.set(j.at("Value").get_to(value));
	}

};

template <class T>
class reactive_property_ptr<std::shared_ptr<T>>
{
	template <class _Elem, class _Traits, class _Alloc> friend class reactive_basic_string_ptr;
private:
	std::shared_ptr<reactive_property<std::shared_ptr<T>>> m_preactive;
public:
	explicit reactive_property_ptr()
		: m_preactive(std::make_shared<reactive_property<std::shared_ptr<T>>>()) {}

	template<class... Args>
	explicit reactive_property_ptr(const Args&... args)
		: m_preactive(std::make_shared<reactive_property<std::shared_ptr<T>>>(args...)) {}

	template<class... Args>
	explicit reactive_property_ptr(Args&&... args)
		: m_preactive(std::make_shared<reactive_property<std::shared_ptr<T>>>(std::forward<Args>(args)...)) {}

	virtual ~reactive_property_ptr() = default;

	reactive_property_ptr(const reactive_property_ptr&) = default;
	reactive_property_ptr& operator=(const reactive_property_ptr&) = default;
	reactive_property_ptr(reactive_property_ptr&&) noexcept = default;
	reactive_property_ptr& operator=(reactive_property_ptr&&) noexcept = default;

	const std::shared_ptr<reactive_property<std::shared_ptr<T>>> life() const
	{
		return m_preactive;
	}
	//Special begin
	const std::shared_ptr<T>& get_shared_unconst() const noexcept
	{
		return m_preactive->m_value;
	}

	const T& operator*() const noexcept
	{
		return *m_preactive->m_value;
	}

	const T* operator->() const noexcept
	{
		return m_preactive->m_value.get();
	}

	T* get_unconst() const noexcept
	{
		return m_preactive->m_value.get();
	}
	//Special end

	auto operator<=>(const reactive_property_ptr& rhs) const
	{
		return m_preactive <=> rhs.m_preactive;
	}
	bool operator==(const reactive_property_ptr& rhs) const
	{
		return m_preactive == rhs.m_preactive;
	}

	explicit operator bool() const noexcept
	{
		return m_preactive.operator bool() && m_preactive->m_value.operator bool();
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

	virtual void set(const std::shared_ptr<T>& value)
	{
		this->m_preactive->observe_property(value);
	}

	virtual void force_notify_set(const std::shared_ptr<T>& value)
	{
		this->m_preactive->m_value = value;
		this->m_preactive->m_subject.on_next(value);
	}

	void unbinding(reactive_property_ptr<std::shared_ptr<T>>& dst)
	{
		dst.disconnect(&reactive_property<std::shared_ptr<T>>::observe_property, this->m_preactive);
		this->disconnect(&reactive_property<std::shared_ptr<T>>::observe_property, dst.m_preactive);
	}

	std::pair<sigslot::connection, sigslot::connection> binding(reactive_property_ptr<std::shared_ptr<T>>& dst)
	{
		unbinding(dst);

		dst.force_notify_set(this->get_shared_unconst());

		return std::make_pair(
			dst.subscribe(&reactive_property<std::shared_ptr<T>>::observe_property, this->m_preactive),
			this->subscribe(&reactive_property<std::shared_ptr<T>>::observe_property, dst.m_preactive));
	}

	template <class U, class Traits, class Allocator>
	void unbinding(reactive_basic_string_ptr<U, Traits, Allocator>& dst)
	{
		dst.disconnect(&reactive_property<std::shared_ptr<T>>::template observe_string<U, Traits, Allocator>, this->m_preactive);
		this->disconnect(&reactive_basic_string<U, Traits, Allocator>::template observe_property<std::shared_ptr<T>>, dst.m_preactive);
	}

	template <class U, class Traits, class Allocator>
	std::pair<sigslot::connection, sigslot::connection> binding(reactive_basic_string_ptr<U, Traits, Allocator>& dst)
	{
		unbinding(dst);

		dst.set(boost::lexical_cast<std::basic_string<U, Traits, Allocator>>(this->m_preactive->m_value));

		return std::make_pair(
			dst.subscribe(&reactive_property<std::shared_ptr<T>>::template observe_string<U, Traits, Allocator>, this->m_preactive),
			this->subscribe(&reactive_basic_string<U, Traits, Allocator>::template observe_property<std::shared_ptr<T>>, dst.m_preactive));
	}

	friend void to_json(json& j, const reactive_property_ptr<std::shared_ptr<T>>& o)
	{
		j = {{"Value", *o}};
	}

	friend void from_json(const json& j, reactive_property_ptr<std::shared_ptr<T>>& o)
	{
		std::shared_ptr<T> value;
		o.set(j.at("Value").get_to(value));
	}

};

