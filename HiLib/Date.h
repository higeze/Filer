#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <regex>
#include <chrono>
#include <ctime>
#include "JsonSerializer.h"

class CDate
{
private:
    int m_year = -1;
    int m_month = -1;
    int m_day = -1;

public:
    CDate():m_year(-1), m_month(-1), m_day(-1)
    {
        //time_t tt = time(NULL);
        //struct tm local_tm;
        //localtime_s(&local_tm, &tt);

        //m_year = local_tm.tm_year + 1900;
        //m_month = local_tm.tm_mon + 1;
        //m_day = local_tm.tm_mday;
    }

    CDate(const std::wstring& str):m_year(-1), m_month(-1), m_day(-1)
    {
        Parse(str);
    }

    auto operator<=>(const CDate&) const = default;

    static const CDate& Now()
    {
        static CDate now;

        if (now.IsInvalid()) {
            time_t tt = time(NULL);
            struct tm local_tm;
            localtime_s(&local_tm, &tt);

            now.m_year = local_tm.tm_year + 1900;
            now.m_month = local_tm.tm_mon + 1;
            now.m_day = local_tm.tm_mday;
        }

        return now;
    }

    int GetYear()const { return m_year; }
    int GetMonth()const { return m_month; }
    int GetDay()const { return m_day; }
    int GetWeekDay()const
    {
        int year = m_year;
        int month = m_month;
        if (month < 3) { year--;  month += 12;  }
        return (year + year / 4 - year / 100 + year / 400 + (13 * month + 8) / 5 + m_day) % 7;
    }
    std::wstring GetEngWeekDay()const
    {
        static std::vector<std::wstring> weekdays_eng =
        {
            L"Sun",
            L"Mon",
            L"Tue",
            L"Wed",
            L"Thu",
            L"Fri",
            L"Sat" 
        };
        return weekdays_eng[GetWeekDay()];
    }
    std::wstring GetJpnWeekDay()const
    {
        static std::vector<std::wstring> weekdays_jpn =
        {
            L"ì˙",
            L"åé",
            L"âŒ",
            L"êÖ",
            L"ñÿ",
            L"ã‡",
            L"ìy" 
        };
        return weekdays_jpn[GetWeekDay()];
    }

    bool Parse(const std::wstring& str)
    {
        time_t tt = time(NULL);
        struct tm local_tm;
        localtime_s(&local_tm, &tt);
        std::wsmatch m;
        m_year = local_tm.tm_year + 1900; int mm = 0; int dd = 0;
        if (std::regex_match(str, m, std::wregex(LR"(([0-9]{4})/([0-9]{1,2})/([0-9]{1,2}))"))) {
            m_year = _wtoi(m[1].str().c_str());
            m_month = _wtoi(m[2].str().c_str());
            m_day = _wtoi(m[3].str().c_str());
            return true;
        } else if (std::regex_match(str, m, std::wregex(LR"(([0-9]{1,2})/([0-9]{1,2}))"))) {
            m_month = _wtoi(m[1].str().c_str());
            m_day = _wtoi(m[2].str().c_str());
            return true;
        } else {
            m_year = m_month = m_day = -1;
            return false;
        }
    }

    bool IsInvalid() const
    {
        return m_year < 0 || m_month < 0 || m_day < 0;
    }

    operator bool() const
    {
        return !IsInvalid();
    }

    NLOHMANN_DEFINE_TYPE_M_INTRUSIVE_NOTHROW(
        CDate,
        year,
        month,
        day);
};
