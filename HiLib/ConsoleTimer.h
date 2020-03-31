#pragma once
#include <chrono>
#include <Windows.h>
#include "MyString.h"

class CConsoleTimer
{
private:
	HWND m_hWnd;
	std::chrono::system_clock::time_point m_tp;
	std::string m_strMessage;
public:
	CConsoleTimer(const std::string& strMessage)
		:m_tp(std::chrono::system_clock::now()), m_strMessage(strMessage){}
	virtual ~CConsoleTimer()
	{
		SPDLOG_INFO(m_strMessage + " : " + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - m_tp).count()));
	}

};

#define CONSOLETIMER(message) \
	CConsoleTimer consoleTimer(message)\

#define CONSOLETIMER_IF(cond, message) \
	std::unique_ptr<CConsoleTimer> upConsoleTimer;\
	if(cond)upConsoleTimer.reset(new CConsoleTimer(message))