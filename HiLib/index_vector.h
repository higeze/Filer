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
		size_type posIdx = std::distance(cbegin(), position);
		auto ret = insert(position, x);
		for (size_type i = posIdx; i < size(); i++) {
			m_setter(operator[](i), i);
		}
		return ret;
	}

	iterator idx_erase(const_iterator position)
	{
		size_type posIdx = std::distance(cbegin(), position);
		iterator ret = erase(position);
		for (size_type i = posIdx; i < size(); i++) {
			m_setter(operator[](i), i);
		}
		return ret;
	}

	iterator idx_erase(const_iterator first, const_iterator last)
	{
		size_type firstIdx = std::distance(cbegin(), first);
		iterator ret = erase(first, last);
		for (size_type i = firstIdx; i < size(); i++) {
			m_setter(operator[](i), i);
		}
		return ret;
	}
};
