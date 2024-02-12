#pragma once
#include <type_traits>
#include <vector>
#include <memory>

/************/
/* is_tuple */
/************/
//template <typename> struct is_tuple: std::false_type {};
//template <typename ...T> struct is_tuple<std::tuple<T...>>: std::true_type {};

template < typename T >
struct is_tuple : std::false_type{};

template < typename ...Types >
struct is_tuple<std::tuple<Types...>> : std::true_type {};

template < typename T >
constexpr bool is_tuple_v = is_tuple<T>::value;

/*************/
/* is_vector */
/*************/
template<typename> struct is_vector :std::false_type {};

template<typename T, typename A>
struct is_vector<std::vector<T, A>> :std::true_type {};

/*****************/
/* is_shared_ptr */
/*****************/
template<class TRect>
struct is_shared_ptr :std::false_type {};

template<class TRect>
struct is_shared_ptr<std::shared_ptr<TRect>> :std::true_type {};

/*******/
/* if_ */
/*******/

template<bool Cond, class Then, class Else>
struct if_;

template<class Then, class Else>
struct if_<true, Then, Else> {
	typedef Then type;
};

template<class Then, class Else>
struct if_<false, Then, Else> {
	typedef Else type;
};

/*************/
/* eval_if_c */
/*************/
template<bool Cond, class Then, class Else>
struct eval_if_c;

template<class Then, class Else>
struct eval_if_c<true, Then, Else> {
	typedef typename Then::type type;
};

template<class Then, class Else>
struct eval_if_c<false, Then, Else> {
	typedef typename Else::type type;
};

/***********/
/* eval_if */
/***********/
template <class Cond, class Then, class Else>
struct eval_if {
	typedef typename eval_if_c<Cond::value, Then, Else>::type type;
};


/*************/
/* and_ */
/*************/

template<class TRect, class U, class V = std::true_type>
struct and_
{
	static const bool value = TRect::value && U::value && V::value;
};

/*************/
/* not_ */
/*************/
template<class TRect>
struct not_
{
	static const bool value = !TRect::value;
};

/*********************************/
/* HAS_MEMBER_FUNCTION1(memfunc) */
/*********************************/

#define HAS_MEMBER_FUNCTION1(memfunc) \
struct has_##memfunc_impl {\
template <class T, class U>\
static auto check(T*, U*)->decltype(std::declval<T>().##memfunc(std::declval<U&>()), std::true_type{});\
\
template <class T, class U>\
static auto check(...)->std::false_type;\
};\
\
template <class T, class U>\
struct has_##memfunc :\
	public decltype(has_##memfunc_impl::check<T, U>(nullptr, nullptr)) {};

/******************/
/* CreateInstance */
/******************/

template<typename U, std::enable_if_t<is_shared_ptr<U>::value && !std::is_pointer<U>::value, std::nullptr_t> = nullptr>
std::shared_ptr<U::template element_type> CreateInstance() { return std::make_shared<U::template element_type>(); }

template<typename U, std::enable_if_t<!is_shared_ptr<U>::value && std::is_pointer<U>::value, std::nullptr_t> = nullptr>
U* CreateInstance() { return new U(); }

template<typename U, std::enable_if_t<!is_shared_ptr<U>::value && !std::is_pointer<U>::value, std::nullptr_t> = nullptr>
U CreateInstance() { return U(); }




//#define HAS_MEMBER_FUNCTION1(memfunc) \
//template<typename TObj,typename TArg1>\
//struct has_##memfunc\
//{\
//private:\
//    template<typename UObj,typename UArg1>\
//	static auto check(UObj& obj,UArg1 arg1)->decltype(obj.##memfunc(std::declval<UArg1>()),std::true_type());\
//	static std::false_type check(...) ;\
//public:\
//    static bool const value=std::identity<decltype(check(std::declval<TObj&>(),std::declval<TArg1>()))>::type::value;\
//};

//template<class T>
//struct has_hoge{
//
//	typedef char yes;
//	typedef struct{char a[2];} no;
//	template<class T> static yes check(T* p,T::hoge*=0);
//	template<class T> static yes check(T* p,decltype(p->hoge)*=NULL);
//	template<class T> static no check(...);
//	static const bool value=sizeof(check<T>(NULL)==yes);
//};