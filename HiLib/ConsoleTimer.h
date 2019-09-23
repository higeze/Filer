#pragma once
#include <boost\timer.hpp>
#include <Windows.h>
#include "MyString.h"

class CConsoleTimer:public boost::timer
{
private:
	HWND m_hWnd;
	std::string m_strMessage;
public:
	CConsoleTimer(const std::string& strMessage)
		:timer(), m_strMessage(strMessage){}
	virtual ~CConsoleTimer()
	{
		spdlog::info(m_strMessage + " : " + std::to_string(elapsed()));
	}

};

#define CONSOLETIMER(message) \
	CConsoleTimer consoleTimer(message)\

#define CONSOLETIMER_IF(cond, message) \
	std::unique_ptr<CConsoleTimer> upConsoleTimer;\
	if(cond)upConsoleTimer.reset(new CConsoleTimer(message))