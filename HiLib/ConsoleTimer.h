#pragma once
#include <boost\timer.hpp>
#include <Windows.h>
#include "MyString.h"

class CConsoleTimer:public boost::timer
{
private:
	HWND m_hWnd;
	std::wstring m_strMessage;
public:
	CConsoleTimer(const std::wstring& strMessage)
		:timer(), m_strMessage(strMessage){}
	virtual ~CConsoleTimer()
	{
		BOOST_LOG_TRIVIAL(trace) <<m_strMessage<<" : "<<elapsed();
	}

};

#define CONSOLETIMER_IF(cond, message) \
	std::unique_ptr<CConsoleTimer> upConsoleTimer;\
	if(cond)upConsoleTimer.reset(new CConsoleTimer(message));