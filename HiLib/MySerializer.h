#pragma once
#include "MyMPL.h"
//HAS_MEMBER_FUNCTION1(serialize)
//
//HAS_MEMBER_FUNCTION1(save)
//
//HAS_MEMBER_FUNCTION1(load)

struct has_serialize_impl{ \
template <class TRect, class U>\
static auto check(TRect*, U*)->decltype(std::declval<TRect>().serialize(std::declval<U&>()), std::true_type{});\
\
template <class TRect, class U>\
static auto check(...)->std::false_type;\
};\
\
template <class TRect, class U>\
struct has_serialize :\
	public decltype(has_serialize_impl::check<TRect, U>(nullptr, nullptr)) {};

struct has_save_impl {
	\
		template <class TRect, class U>\
		static auto check(TRect*, U*)->decltype(std::declval<TRect>().save(std::declval<U&>()), std::true_type{}); \
		\
		template <class TRect, class U>\
		static auto check(...)->std::false_type; \
}; \
\
template <class TRect, class U>\
struct has_save :\
	public decltype(has_save_impl::check<TRect, U>(nullptr, nullptr)) {};

struct has_load_impl {
	\
		template <class TRect, class U>\
		static auto check(TRect*, U*)->decltype(std::declval<TRect>().load(std::declval<U&>()), std::true_type{}); \
		\
		template <class TRect, class U>\
		static auto check(...)->std::false_type; \
}; \
	\
		template <class TRect, class U>\
		struct has_load :\
		public decltype(has_load_impl::check<TRect, U>(nullptr, nullptr)) {};
