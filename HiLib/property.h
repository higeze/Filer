#pragma once

template<class TRect>
class property
{
protected:
	std::function<TRect()> m_get = nullptr;
	std::function<void(const TRect&)> m_set = nullptr;

public:
	TRect value;
	property(const std::function<TRect()>& get = nullptr, const std::function<void(const TRect&)>& set = nullptr)
		: m_get(get), m_set(set){}
	void operator()(const std::function<TRect()>& get, const std::function<void(const TRect&)>& set)
	{
		m_get = get; m_set = set;
	}
	TRect operator()() const { return m_get(); }
	operator TRect() const { return m_get(); }
	TRect operator ->() { return m_get(); }

	TRect operator()(TRect const& val) { return m_set(val); }
	TRect operator=(TRect const& val) { m_set(val); return value; }

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
