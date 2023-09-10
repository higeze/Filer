#pragma once
#include <vector>
#include <ranges>
#include <algorithm>
#include <sigslot/signal.hpp>

class scoped_connections
{
private:
	std::vector<sigslot::connection> m_connections;
	std::vector < std::pair<sigslot::connection, sigslot::connection>> m_pair_connections;
public:
	scoped_connections() {}
	~scoped_connections()
	{
		clear();
	}

	template<class...Items>
	void push_back(Items&&... items)
	{
		push_back_head_tail(std::forward<Items>(items)...);
	}

	void clear()
	{
		auto disconnect = [](sigslot::connection& con) { return con.disconnect(); };
		auto disconnect2 = [](std::pair<sigslot::connection, sigslot::connection>& paircon) { return paircon.first.disconnect() && paircon.second.disconnect(); };
		std::ranges::for_each(m_connections, disconnect);
		std::ranges::for_each(m_pair_connections, disconnect2);
		m_connections.clear();
		m_pair_connections.clear();
	}

private:
	void push_back_head_tail() {}

	template <class Head, class... Tail>
	void push_back_head_tail(Head&& head, Tail&&... tail)
	{
		push_back_item(std::forward<Head>(head));
		push_back_head_tail(std::forward<Tail>(tail)...);
	}

	void push_back_item(sigslot::connection&& connection)
	{
		m_connections.push_back(std::forward<sigslot::connection>(connection));
	}

	void push_back_item(std::pair<sigslot::connection, sigslot::connection>&& pair_connection)
	{
		m_pair_connections.push_back(std::forward<std::pair<sigslot::connection, sigslot::connection>>(pair_connection));
	}

};