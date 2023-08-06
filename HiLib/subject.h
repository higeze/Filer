#pragma once
#include <sigslot/signal.hpp>

template <typename... T>
class subject
{
private:
	mutable sigslot::signal<T...> m_on_next;
public:
	subject() {}
	virtual ~subject() = default;

	template <typename... T>
	void on_next(T... value) const
	{
		m_on_next(std::forward<T>(value)...);
	}

	//template<class Callable>
	//virtual sigslot::connection subscribe(Callable&& next)
	//{
	//	return m_on_next.connect(std::forward<Callable>(next));
	//}

	//template<class... Args>
	//auto subscribe(Args&&... args) const -> sigslot::connection
	//{
	//	return m_on_next.connect(std::forward<Args>(args)...);
	//}
	using arg_list = sigslot::trait::typelist<T...>;

	template <typename Pmf, typename Ptr>
    std::enable_if_t<!sigslot::trait::is_callable_v<arg_list, Pmf> &&
                     sigslot::trait::is_weak_ptr_compatible_v<Ptr>, sigslot::connection>
	subscribe(Pmf&& pmf, Ptr&& ptr)
	{
		return m_on_next.connect(std::forward<Pmf>(pmf), std::forward<Ptr>(ptr));
	}

    template <typename Callable, typename Trackable>
    std::enable_if_t<sigslot::trait::is_callable_v<arg_list, Callable> &&
                     sigslot::trait::is_weak_ptr_compatible_v<Trackable>, sigslot::connection>
	subscribe(Callable&& c, Trackable&& ptr)
	{
		return m_on_next.connect(std::forward<Callable>(c), std::forward<Trackable>(ptr));
	}

	void block()
	{
		return m_on_next.block();
	}

	void unblock()
	{
		return m_on_next.unblock();
	}

};

