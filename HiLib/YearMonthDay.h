#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <regex>
#include <chrono>
#include <ctime>
#include "JsonSerializer.h"
#include "reactive_property.h"

class CYearMonthDay: public std::chrono::year_month_day
{
private:
    static const size_t INVALID = 0;
public:
    CYearMonthDay(int year = INVALID, size_t month = INVALID, size_t day = INVALID)
        :std::chrono::year_month_day(std::chrono::year_month_day{std::chrono::year{year}, std::chrono::month{month}, std::chrono::day{day}}){}

    CYearMonthDay(const std::chrono::year_month_day& yearmonthday)
        :std::chrono::year_month_day(yearmonthday){}

    CYearMonthDay(const CYearMonthDay& ymd) = default;
    
    ~CYearMonthDay() = default;

    auto operator<=>(const CYearMonthDay& rhs) const
    {
        return static_cast<std::chrono::year_month_day>(*this) <=> static_cast<std::chrono::year_month_day>(rhs);
    }
    bool operator==(const CYearMonthDay& rhs)const
    {
        return static_cast<std::chrono::year_month_day>(*this) == static_cast<std::chrono::year_month_day>(rhs);
    }

    static const CYearMonthDay Now()
    {
        time_t tm = time(NULL);
        struct tm local_tm;
        localtime_s(&local_tm, &tm);

        return CYearMonthDay(
            local_tm.tm_year + 1900, 
            local_tm.tm_mon + 1,
            local_tm.tm_mday);

        //auto sys_ymd = std::chrono::year_month_day{std::chrono::floor<std::chrono::days>(std::chrono::system_clock::now())};
        //return std::chrono::year_month_day(sys_ymd.operator std::chrono::local_days());
    }

    static const CYearMonthDay Today()
    {
        return Now();
    }

    static const CYearMonthDay Tomorrow()
    {
        return {Now().operator std::chrono::sys_days() + std::chrono::days{1}};
    }

    std::chrono::weekday GetWeekDay()const
    {
        return std::chrono::weekday(static_cast<std::chrono::year_month_day>(*this));
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
        return weekdays_eng[GetWeekDay().c_encoding()];
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
        return weekdays_jpn[GetWeekDay().c_encoding()];
    }

    static CYearMonthDay Parse(const std::wstring& str)
    {
        std::chrono::year year = Now().year();
        std::chrono::month month;
        std::chrono::day day;
        std::wsmatch m;

        if (std::regex_match(str, m, std::wregex(LR"(([0-9]{4})/([0-9]{1,2})/([0-9]{1,2}))"))) {
            year = std::chrono::year{static_cast<int>(_wtol(m[1].str().c_str()))};
            month = std::chrono::month{static_cast<size_t>(_wtol(m[2].str().c_str()))};
            day = std::chrono::day{static_cast<size_t>(_wtol(m[3].str().c_str()))};
        } else if (std::regex_match(str, m, std::wregex(LR"(([0-9]{1,2})/([0-9]{1,2}))"))) {
            month = std::chrono::month{static_cast<size_t>(_wtol(m[1].str().c_str()))};
            day = std::chrono::day{static_cast<size_t>(_wtol(m[2].str().c_str()))};
        } else {
            year = std::chrono::year{INVALID};;
            month = std::chrono::month{INVALID};
            day = std::chrono::day{INVALID};
        }
            return CYearMonthDay(std::chrono::year_month_day{year, month, day});
    }

    bool IsValid() const
    {
        return year().operator int() > INVALID || month().operator size_t() > INVALID || day().operator size_t() > INVALID;
    }

    bool IsInvalid() const
    {
        return !IsValid();
    }

    operator bool() const
    {
        return IsValid();
    }

	friend void to_json(json& j, const CYearMonthDay& o)
	{
        j["Year"] = o.year().operator int();
        j["Month"] = o.month().operator size_t();
        j["Day"] = o.day().operator size_t();
	}

	friend void from_json(const json& j, CYearMonthDay& o)
	{
        int year = CYearMonthDay::INVALID;
        size_t month = CYearMonthDay::INVALID;
        size_t day = CYearMonthDay::INVALID;
        json_safe_from(j, "Year", year);
        json_safe_from(j, "Month", month);
        json_safe_from(j, "Day", day);
        o = CYearMonthDay(year, month, day);
	}
};
