#pragma once
#include <vector>

template<class T, class Allocator = std::allocator<T>>
class index_vector :public std::vector<T, Allocator>
{
public:
	using size_type = typename std::vector<T, Allocator>::size_type;
	using iterator = typename std::vector<T, Allocator>::iterator;
	using const_iterator = typename std::vector<T, Allocator>::const_iterator;
public:
	index_vector(const std::function<void(T&, size_type idx)>& setter)
		:std::vector<T, Allocator>(), m_setter(setter){}

	std::function<void(T&, size_type idx)> m_setter;

	void idx_push_back(const T& x)
	{
		this->push_back(x);
		m_setter(this->operator[](this->size() - 1), this->size() - 1);
	}

	iterator idx_insert(const_iterator position, const T& x)
	{
		size_type posIdx = std::distance(this->cbegin(), position);
		auto ret = this->insert(position, x);
		for (size_type i = posIdx; i < this->size(); i++) {
			m_setter(this->operator[](i), i);
		}
		return ret;
	}

	iterator idx_erase(const_iterator position)
	{
		size_type posIdx = std::distance(this->cbegin(), position);
		iterator ret = this->erase(position);
		for (size_type i = posIdx; i < this->size(); i++) {
			m_setter(this->operator[](i), i);
		}
		return ret;
	}

	iterator idx_erase(const_iterator first, const_iterator last)
	{
		size_type firstIdx = std::distance(this->cbegin(), first);
		iterator ret = this->erase(first, last);
		for (size_type i = firstIdx; i < this->size(); i++) {
			m_setter(this->operator[](i), i);
		}
		return ret;
	}

	template <class RandomAccessIterator, class Compare>
	void stable_sort(RandomAccessIterator first, RandomAccessIterator last, Compare comp)
	{
		std::stable_sort(first, last, comp);
	}

	template <class RandomAccessIterator, class Compare>
	void idx_stable_sort(RandomAccessIterator first, RandomAccessIterator last, Compare comp){
		std::stable_sort(first, last, comp);

		for (size_type i = std::distance(this->begin(), first), lastPos = std::distance(this->begin(), last); i < lastPos; i++) {
			m_setter(this->operator[](i), i);
		}
	}
};
