#pragma once

#include <string>

#include <sigslot/signal.hpp>

struct property_changed_event_args
{
    std::string property_name;
};

class notify_property_changed
{
private:
	class binder
	{
	private:
		notify_property_changed* m_ptr;
	public:
		binder(notify_property_changed* ptr) :m_ptr(ptr) {}

		template<typename T>
		void copy_to_this(void* ptr, const property_changed_event_args& e)
		{
			reinterpret_cast<T*>(m_ptr)->operator=(*(reinterpret_cast<T*>(ptr)));
		}
	};
private:
	std::shared_ptr<binder> m_binder;
public:
    sigslot::signal<void* , const property_changed_event_args&> property_changed;
public:
	notify_property_changed() :m_binder(std::make_shared<binder>(this)){}
	virtual ~notify_property_changed() = default;
	notify_property_changed(const notify_property_changed&) = default;
	notify_property_changed& operator=(const notify_property_changed&) = default;
	notify_property_changed(notify_property_changed&&) noexcept = default;
	notify_property_changed& operator=(notify_property_changed&&) noexcept = default;

    void raise_property_changed(const char* property_name)
    {
        on_property_changed(property_changed_event_args{property_name});
    }
    void on_property_changed(property_changed_event_args args)
    {
        property_changed(this, args);
    }

	template<typename T>
	void unbind(notify_property_changed& dst)
	{
		dst.property_changed.disconnect(&binder::copy_to_this<T>, this->m_binder);
		this->property_changed.disconnect(&binder::copy_to_this<T>, dst.m_binder);
	}

	template<typename T>
	std::pair<sigslot::connection, sigslot::connection> bind(notify_property_changed& dst)
	{
		unbind<T>(dst);

		dst.m_binder->copy_to_this<T>(this, property_changed_event_args{});

		return std::make_pair(
			dst.property_changed.connect(&binder::copy_to_this<T>, this->m_binder),
			this->property_changed.connect(&binder::copy_to_this<T>, dst.m_binder));
	}
};

#define RAISE_PROPERTY_CHANGED raise_property_changed(__FUNCTION__)
#define NOTIFIABLE_PROPERTY(type, name) \
private: type m_##name; \
public: const type& Get##name() const { return m_##name; } \
public: void Set##name(const type& value) \
{ \
	if (m_##name != value) { \
		m_##name = value; \
		raise_property_changed(#name); \
	} \
}

