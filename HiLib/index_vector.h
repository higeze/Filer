#pragma once
#include <vector>

template<class T, class Allocator = std::allocator<T>>
class index_vector :public std::vector<T, Allocator>
{
public:
	index_vector(const std::function<void(T&, size_type idx)>& setter):std::vector<T, Allocator>(), m_setter(setter){}
	std::function<void(T&, size_type idx)> m_setter;

	void idx_push_back(const T& x)
	{
		push_back(x);
		m_setter(operator[](size() - 1), size() - 1);
	}

	iterator idx_insert(const_iterator position, const T& x)
	{
		auto ret = insert(position, x);
		for (size_type i = std::distance((const_iterator)begin(), position); i < size(); i++) {
			m_setter(operator[](i), i);
		}
		return ret;
	}

	iterator idx_erase(const_iterator position)
	{
		iterator ret = erase(position);
		for (size_type i = std::distance((const_iterator)begin(), position); i < size(); i++) {
			m_setter(operator[](i), i);
		}
		return ret;
	}

	iterator idx_erase(const_iterator first, const_iterator last)
	{
		iterator ret = erase(first, last);
		for (size_type i = std::distance((const_iterator)begin(), first); i < size(); i++) {
			m_setter(operator[](i), i);
		}
		return ret;
	}
};
