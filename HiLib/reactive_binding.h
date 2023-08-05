#pragma once
#include "reactive_property.h"
#include "reactive_command.h"
#include "reactive_vector.h"
#include "reactive_string.h"
#include <functional>

template<class T>
auto reactive_binding(reactive_property_ptr<T>& src, reactive_property_ptr<T>& dst) -> std::pair<sigslot::connection, sigslot::connection>
{
    dst->set(src->get_const());

    return std::make_pair(
        dst->subscribe(&reactive_property<T>::set, src),
        src->subscribe(&reactive_property<T>::set, dst));
}

template<class T>
auto reactive_command_binding(reactive_command_ptr<T> src, reactive_command_ptr<T> dst)->sigslot::connection
{
	return dst->subscribe(&reactive_command<T>::execute, src);
}

template<class T, class Allocator>
auto reactive_vector_binding(reactive_vector_ptr<T, Allocator>& src, reactive_vector_ptr<T, Allocator>& dst)
-> std::pair<sigslot::connection, sigslot::connection>
{
	dst->clear();
	for (size_t i = 0; i < src->size(); i++) {
		dst->push_back(adl_vector_item<T>::clone(src->get_unconst().operator[](i)));
		adl_vector_item<T>::bind(
				src->get_unconst().operator[](i),
				dst->get_unconst().operator[](i));
	}
	auto pair = std::make_pair(
		dst->subscribe(&reactive_vector<T, Allocator>::observe, src),
		src->subscribe(&reactive_vector<T, Allocator>::observe, dst));
	return pair;
}

template <class T, class Traits, class Allocator>
auto reactive_string_binding(
	reactive_basic_string_ptr<T, Traits, Allocator>& src, 
	reactive_basic_string_ptr<T, Traits, Allocator>& dst) -> std::pair<sigslot::connection, sigslot::connection>
{
	dst->set(src->get_const());

	return std::make_pair(
		dst->subscribe(&reactive_basic_string<T, Traits, Allocator>::observe_string, src),
		src->subscribe(&reactive_basic_string<T, Traits, Allocator>::observe_string, dst));
}

template <class T, class U, class Traits, class Allocator>
auto reactive_property_string_binding(
	reactive_property_ptr<T>& src, 
	reactive_basic_string_ptr<U, Traits, Allocator>& dst) /*-> std::pair<sigslot::connection, sigslot::connection>*/
{
	dst->set(boost::lexical_cast<std::basic_string<U, Traits, Allocator>>(src->get_const()));

	return std::make_pair(
		dst->subscribe(&reactive_property<T>::observe_string, src),
		src->subscribe(&reactive_basic_string<U, Traits, Allocator>::template observe_property<T>, dst));
}