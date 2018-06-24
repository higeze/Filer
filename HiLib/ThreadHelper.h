#pragma once
#include <mutex>

template<typename Function>
struct function_traits :public function_traits<decltype(&Function::operator())> {};

template<typename ClassType, typename ReturnType, typename ...Args>
struct function_traits<ReturnType(ClassType::*)(Args...) const>
{
	typedef ReturnType(*pointer)(Args...);
	typedef std::function<ReturnType(Args...)> function;
};

template<typename Function>
typename function_traits<Function>::function to_function(Function & lambda) {
	return static_cast<typename function_traits<Function>::function>(lambda);
}

template<typename Lambda>
size_t getAddress(Lambda lambda) {
	auto function = new decltype(to_function(lambda))(to_function(lambda));
	void* func = static_cast<void*>(function);
	return (size_t)func;
}
//
//template<typename T, typename...U>
//size_t GetFunctionAddress(std::function<T(U...)> f)
//{
//	typedef T(fnType)(U...);
//	fnType ** fnPointer = f.template target<fnType*>();
//	return (size_t)*fnPointer;
//
//}


