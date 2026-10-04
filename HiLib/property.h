#pragma once

template<class T>
class property
{
protected:
	std::function<T()> m_get = nullptr;
	std::function<void(const T&)> m_set = nullptr;

public:
	T value;
	property(const std::function<T()>& get = nullptr, const std::function<void(const T&)>& set = nullptr)
		: m_get(get), m_set(set){}
	void operator()(const std::function<T()>& get, const std::function<void(const T&)>& set)
	{
		m_get = get; m_set = set;
	}
	T operator()() const { return m_get(); }
	operator T() const { return m_get(); }
	T operator ->() { return m_get(); }

	T operator()(T const& val) { return m_set(val); }
	T operator=(T const& val) { m_set(val); return value; }

	//friend void to_json(json& j, const property<T>& o)
	//{
	//	to_json(j, m_get());
	//}

	//friend void from_json(const json& j, property<T>& o)
	//{
	//	T tmp;
	//	from_json(j, tmp);
	//	m_set(tmp);
	//}

};
