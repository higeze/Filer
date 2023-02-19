#pragma once
#include <future>
#include "se_exception.h"

template<class F, class... Args>
void async_action_wrap(F&& function, Args&&... arguments)
{
	scoped_se_translator scoped_se_trans;
	try {
		std::invoke(std::forward<F>(function), std::forward<Args>(arguments)...);
	}catch (std::exception& ex) {
		auto msg = exception_to_string(ex);
		MessageBoxA(NULL, msg.c_str(), "exception in thread", MB_ICONWARNING);//TODO Need to create D2D Modal Dialog
		SPDLOG_ERROR("exception in thread\r\n" + msg);
		::OutputDebugStringA(msg.c_str());
	}catch (...) {
		auto msg = GetLastErrorString();
		MessageBoxA(NULL, msg.c_str(), "unknown exception in thread", MB_ICONWARNING);
		SPDLOG_ERROR("unknown exception in thread\r\n" + msg);
		::OutputDebugStringA(msg.c_str());
	}
}

template<class F, class R, class... Args>
R async_function_wrap(F&& function, R&& error, Args&&... arguments)
{
	scoped_se_translator scoped_se_trans;
	try {
		return std::invoke(std::forward<F>(function), std::forward<Args>(arguments)...);
	}catch (std::exception& ex) {
		auto msg = exception_to_string(ex);
		MessageBoxA(NULL, msg.c_str(), "exception in thread", MB_ICONWARNING);
		SPDLOG_ERROR("exception in thread\r\n" + msg);
		::OutputDebugStringA(msg.c_str());
		return error;
	}catch (...) {
		auto msg = GetLastErrorString();
		MessageBoxA(NULL, msg.c_str(), "unknown exception in thread", MB_ICONWARNING);
		SPDLOG_ERROR("unknown exception in thread\r\n" + msg);
		::OutputDebugStringA(msg.c_str());
		return error;
	}
}


//template <class F, class... Args>
//std::future<void> async_action_nothrow(std::launch policy, F&& function, Args&&... arguments)
//{
//	//std::function<void(F&&, Args&&...)> async_action = [](F&& function, Args&&... arguments)->void
//	//{
//	//	scoped_se_translator scoped_se_trans;
//	//	try {
//	//		std::invoke(std::forward<F>(function), std::forward<Args>(arguments)...);
//	//	}
//	//	catch (std::exception& ex) {
//	//		auto msg = exception_to_string(ex);
//	//		MessageBoxA(NULL, msg.c_str(), "exception in thread", MB_ICONWARNING);
//	//		SPDLOG_ERROR("exception in thread\r\n" + msg);
//	//		::OutputDebugStringA(msg.c_str());
//	//	}
//	//	catch (...) {
//	//		auto msg = GetLastErrorString();
//	//		MessageBoxA(NULL, msg.c_str(), "unknown exception in thread", MB_ICONWARNING);
//	//		SPDLOG_ERROR("unknown exception in thread\r\n" + msg);
//	//		::OutputDebugStringA(msg.c_str());
//	//	}
//	//};
//
//	return std::async(policy, ]<F, Args...>, std::forward<F>(function), std::forward<Args>(arguments)...);
//};
//
//template <class F, class R, class... Args>
//std::future<R> async_function_nothrow(std::launch policy, F&& function, R&& error,  Args&&... arguments)
//{
//	return std::async(policy, async_function_wrap<F, R, Args...>, std::forward<F>(function), std::forward<R>(error), std::forward<Args>(arguments)...);
//};
