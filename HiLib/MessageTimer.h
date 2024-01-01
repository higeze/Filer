#pragma once
#include <chrono>
#include <Windows.h>
#include "MyString.h"


template<typename char_type>
int MessageBoxT(HWND hWnd, const char_type* lpText, const char_type* lpCaption, UINT uType)
{
	return ::MessageBoxA(hWnd, lpText, lpCaption, uType);
}

//template<>
//int MessageBoxT<wchar_t>(HWND hWnd, const wchar_t* lpText, const wchar_t* lpCaption, UINT uType)
//{
//	return ::MessageBoxW(hWnd, lpText, lpCaption, uType);
//}


template<typename char_type>
class CMessageTimer
{
private:
	HWND m_hWnd;
	std::chrono::system_clock::time_point m_tp;
	std::basic_string<char_type> m_strMessage;
public:
	CMessageTimer(HWND hWnd, const std::basic_string<char_type>& wstrMessage)
		:m_tp(std::chrono::system_clock::now()), m_hWnd(hWnd), m_strMessage(wstrMessage){}
	virtual ~CMessageTimer()
	{
		MessageBoxT<char_type>(m_hWnd,
			(m_strMessage + TSTRING_CAST(char_type, " : ") + boost::lexical_cast<std::basic_string<char_type>>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - m_tp).count())).c_str(),
			TSTRING_CAST(char_type, "MessageTimer"),
			MB_OK | MB_ICONINFORMATION);
	}

};

#define MESSAGETIMER_IF(cond, char_type, hwnd, message) \
	std::unique_ptr<CMessageTimer<char_type>> upMessageTimer;\
	if(cond)upMessageTimer.reset(new CMessageTimer<char_type>(hwnd, message));