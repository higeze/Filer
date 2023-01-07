#pragma once

#include <shlwapi.h>
#include <spdlog/spdlog.h>
#include <fmt/format.h>
#include <chrono>
#include "MyWin32.h"

/****************/
/* scoped_timer */
/****************/

template<class _Elem>
class scoped_time_logger
{
private:
	std::chrono::system_clock::time_point m_tp;
	std::basic_string<_Elem> m_message;
public:
	scoped_time_logger(const _Elem* message)
		:m_tp(std::chrono::system_clock::now()), m_message(message){}

	scoped_time_logger(const std::basic_string<_Elem>& message)
		:m_tp(std::chrono::system_clock::now()), m_message(message){}

	void stop(){}

	virtual ~scoped_time_logger()
	{
		stop();
	}
};

/*************/
/* exception */
/*************/

std::string exception_msg_to_string(const std::exception& e, UINT msg, WPARAM wParam, LPARAM lParam);

std::string exception_to_string(const std::exception& e);

std::string msg_to_string(UINT msg, WPARAM wParam, LPARAM lParam);

/*********/
/* Macro */
/*********/

#define LOG_THIS_0 \
	SPDLOG_INFO(fmt::format("{}\t{}\t{}\t{}",::PathFindFileNameA(__FILE__), __LINE__, __FUNCTION__, typeid(*this).name()).c_str())

#define LOG_1(message) \
	SPDLOG_INFO(fmt::format("{}\t{}\t{}\t{}",::PathFindFileNameA(__FILE__), __LINE__, __FUNCTION__, message).c_str())

#define LOG_2(arg1, arg2) \
	SPDLOG_INFO(fmt::format("{}\t{}\t{}\t{}\t{}",::PathFindFileNameA(__FILE__), __LINE__, __FUNCTION__, arg1, arg2).c_str())

#define LOG_THIS_1(message) \
	SPDLOG_INFO(fmt::format("{}\t{}\t{}\t{}\t{}",::PathFindFileNameA(__FILE__), __LINE__, __FUNCTION__, typeid(*this).name(), message).c_str())

#define LOG_THIS_2(arg1, arg2) \
	SPDLOG_INFO(fmt::format("{}\t{}\t{}\t{}\t{}\t{}",::PathFindFileNameA(__FILE__), __LINE__, __FUNCTION__, typeid(*this).name(), arg1, arg2).c_str())

#define LOG_SCOPED_TIMER_1(message) \
	scoped_time_logger stl(fmt::format("{}\t{}\t{}\t{}", ::PathFindFileNameA(__FILE__), __LINE__, __FUNCTION__, message).c_str())

#define LOG_SCOPED_TIMER_THIS_1(message) \
	scoped_time_logger stl(fmt::format("{}\t{}\t{}\t{}\t{}", ::PathFindFileNameA(__FILE__), __LINE__, __FUNCTION__, typeid(*this).name(), message).c_str())

#define CONSOLETIMER_IF(cond, message) \
	std::unique_ptr<scoped_time_logger> pStl;\
	if(cond)pStl.reset(new scoped_time_logger(message))

#define FILE_LINE_FUNC std::format("File:{}, Line:{}, Func:{}", ::PathFindFileNameA(__FILE__), __LINE__, __FUNCTION__).c_str()

#define FALSE_THROW(expression) if(!(expression)){throw std::exception(FILE_LINE_FUNC);}

#define FAILED_THROW(expression) if(HRESULT hr = expression; FAILED(hr)){throw std::exception(std::format("HRESULT:{:#X}, {}", hr, FILE_LINE_FUNC).c_str());}

#define FAILED_RETURN(expression) if(FAILED(expression)){return;}

#define FAILED_BREAK(expression) if(FAILED(expression)){break;}

#define THROW_FILE_LINE_FUNC throw std::exception(FILE_LINE_FUNC)

template<>
inline void scoped_time_logger<char>::stop()
{
	SPDLOG_INFO(m_message + " : " + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - m_tp).count()));
}

template<>
inline void scoped_time_logger<wchar_t>::stop()
{
	//SPDLOG_INFO(m_message + L" : " + std::to_wstring(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - m_tp).count()));
}

