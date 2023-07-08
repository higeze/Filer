#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <regex>
#include <chrono>
#include <ctime>
#include "JsonSerializer.h"
#include "reactive_property.h"

class CYearMonthDay
{
private:
    static const size_t INVALID = 0;
public:
    reactive_property_ptr<std::chrono::year_month_day> YearMonthDay;
public:
    CYearMonthDay(int year = INVALID, size_t month = INVALID, size_t day = INVALID)
        :YearMonthDay(make_reactive_property<std::chrono::year_month_day>(std::chrono::year_month_day{std::chrono::year{year}, std::chrono::month{month}, std::chrono::day{day}})){}

    CYearMonthDay(const std::chrono::year_month_day& yearmonthday)
        :YearMonthDay(make_reactive_property<std::chrono::year_month_day>(yearmonthday)){}

    CYearMonthDay(const std::wstring& str)
        :YearMonthDay(make_reactive_property<std::chrono::year_month_day>())
    {
        Parse(str);
    }

    CYearMonthDay(const CYearMonthDay& ymd) = default;
    
    ~CYearMonthDay() = default;

    //CYearMonthDay Clone() const
    //{
    //    return CYearMonthDay(YearMonthDay->get());
    //}

    auto operator<=>(const CYearMonthDay& rhs) const
    {
        return YearMonthDay <=> rhs.YearMonthDay;
    }
    bool operator==(const CYearMonthDay& rhs)const
    {
        return YearMonthDay == rhs.YearMonthDay;
    }

    static const CYearMonthDay Now()
    {
        time_t tm = time(NULL);
        struct tm local_tm;
        localtime_s(&local_tm, &tm);

        return std::chrono::year_month_day(
            std::chrono::year(local_tm.tm_year + 1900), 
            std::chrono::month(local_tm.tm_mon + 1),
            std::chrono::day(local_tm.tm_mday));

        //auto sys_ymd = std::chrono::year_month_day{std::chrono::floor<std::chrono::days>(std::chrono::system_clock::now())};
        //return std::chrono::year_month_day(sys_ymd.operator std::chrono::local_days());
    }

    static const CYearMonthDay Today()
    {
        return Now();
    }

    static const CYearMonthDay Tomorrow()
    {
        return {Now().YearMonthDay->get_const().operator std::chrono::sys_days() + std::chrono::days{1}};
    }

    //std::tuple<int, int, int> GetYMD() const
    //{
    //    return { Year.get(), Month.get(), Day.get() };
    //}

    std::chrono::weekday GetWeekDay()const
    {
        return std::chrono::weekday{YearMonthDay->get_const()};
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
            L"“ú",
            L"ŒŽ",
            L"‰Î",
            L"…",
            L"–Ø",
            L"‹à",
            L"“y" 
        };
        return weekdays_jpn[GetWeekDay().c_encoding()];
    }

    bool Parse(const std::wstring& str)
    {
        std::chrono::year year = Now().YearMonthDay->get_const().year();
        std::chrono::month month;
        std::chrono::day day;
        std::wsmatch m;

        if (std::regex_match(str, m, std::wregex(LR"(([0-9]{4})/([0-9]{1,2})/([0-9]{1,2}))"))) {
            year = std::chrono::year{static_cast<int>(_wtol(m[1].str().c_str()))};
            month = std::chrono::month{static_cast<size_t>(_wtol(m[2].str().c_str()))};
            day = std::chrono::day{static_cast<size_t>(_wtol(m[3].str().c_str()))};
            YearMonthDay->set(std::chrono::year_month_day{year, month, day});
            return true;
        } else if (std::regex_match(str, m, std::wregex(LR"(([0-9]{1,2})/([0-9]{1,2}))"))) {
            month = std::chrono::month{static_cast<size_t>(_wtol(m[1].str().c_str()))};
            day = std::chrono::day{static_cast<size_t>(_wtol(m[2].str().c_str()))};
            YearMonthDay->set(std::chrono::year_month_day{year, month, day});
            return true;
        } else {
            year = std::chrono::year{INVALID};;
            month = std::chrono::month{INVALID};
            day = std::chrono::day{INVALID};
            YearMonthDay->set(std::chrono::year_month_day{year, month, day});
            return false;
        }
    }

    bool IsValid() const
    {
        return YearMonthDay->get_const().year().operator int() > INVALID || YearMonthDay->get_const().month().operator size_t() > INVALID || YearMonthDay->get_const().day().operator size_t() > INVALID;
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
        j["Year"] = o.YearMonthDay->get_const().year().operator int();
        j["Month"] = o.YearMonthDay->get_const().month().operator size_t();
        j["Day"] = o.YearMonthDay->get_const().day().operator size_t();
	}

	friend void from_json(const json& j, CYearMonthDay& o)
	{
        int year = CYearMonthDay::INVALID;
        size_t month = CYearMonthDay::INVALID;
        size_t day = CYearMonthDay::INVALID;
        get_to(j, "Year", year);
        get_to(j, "Month", month);
        get_to(j, "Day", day);
        o.YearMonthDay->set(std::chrono::year_month_day{std::chrono::year{year}, std::chrono::month{month}, std::chrono::day{day}});
	}
};
