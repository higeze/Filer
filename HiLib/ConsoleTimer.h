#pragma once
#include <chrono>
#include <Windows.h>
#include "MyString.h"

template<class _Elem>
class CConsoleTimer
{
private:
	HWND m_hWnd;
	std::chrono::system_clock::time_point m_tp;
	std::basic_string<_Elem> m_strMessage;
public:
	CConsoleTimer(const _Elem* strMessage)
		:m_tp(std::chrono::system_clock::now()), m_strMessage(strMessage){}

	CConsoleTimer(const std::basic_string<_Elem>& strMessage)
		:m_tp(std::chrono::system_clock::now()), m_strMessage(strMessage){}

	void Stop(){}

	virtual ~CConsoleTimer()
	{
		Stop();
	}
};

template<>
void CConsoleTimer<char>::Stop()
{
	SPDLOG_INFO(m_strMessage + " : " + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - m_tp).count()));
}

template<>
void CConsoleTimer<wchar_t>::Stop()
{
	//SPDLOG_INFO(m_strMessage + L" : " + std::to_wstring(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - m_tp).count()));
}

#define CONSOLETIMER(message) \
	CConsoleTimer consoleTimer(message)\

#define CONSOLETIMER_IF(cond, message) \
	std::unique_ptr<CConsoleTimer> upConsoleTimer;\
	if(cond)upConsoleTimer.reset(new CConsoleTimer(message))