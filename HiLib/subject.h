#pragma once
#include <sigslot/signal.hpp>

template <class T>
class subject
{
private:
	mutable sigslot::signal<T> m_on_next;
public:
	subject() {}
	virtual ~subject() = default;
	virtual void on_next(const T& value) const
	{
		m_on_next(value);
	}

	//template<class Callable>
	//virtual sigslot::connection subscribe(Callable&& next)
	//{
	//	return m_on_next.connect(std::forward<Callable>(next));
	//}

	template<class... Args>
	auto subscribe(Args&&... args) const -> sigslot::connection
	{
		return m_on_next.connect(std::forward<Args>(args)...);
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

