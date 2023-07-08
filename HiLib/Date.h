#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <regex>
#include <chrono>
#include <ctime>
#include "JsonSerializer.h"
#include "reactive_property.h"

class CDate
{
private:
    static const int INVALID = -1;
public:
    reactive_property_ptr<int> Year;
    reactive_property_ptr<int> Month;
    reactive_property_ptr<int> Day;

public:
    CDate(int year = INVALID, int month = INVALID, int day = INVALID)
        :Year(make_reactive_property<int>(year)), Month(make_reactive_property<int>(month)), Day(make_reactive_property<int>(day)){}

    CDate(const std::wstring& str):Year(-1), Month(-1), Day(-1)
    {
        Parse(str);
    }

    auto operator<=>(const CDate& rhs) const
    {
        return (Year.get() * 10000 + Month.get() * 100 + Day.get()) <=> (rhs.Year.get() * 10000 + rhs.Month.get() * 100 + rhs.Day.get());
    }
    bool operator==(const CDate& rhs)const
    {
        return Year.get() == rhs.Year.get() && Month.get() == rhs.Month.get() && Day.get() == rhs.Day.get();
    }

    static const CDate& Now()
    {
        static CDate now;

        if (now.IsInvalid()) {
            time_t tt = time(NULL);
            struct tm local_tm;
            localtime_s(&local_tm, &tt);

            now.Year.set(local_tm.tm_year + 1900);
            now.Month.set(local_tm.tm_mon + 1);
            now.Day.set(local_tm.tm_mday);
        }

        return now;
    }

    std::tuple<int, int, int> GetYMD() const
    {
        return { Year.get(), Month.get(), Day.get() };
    }

    int GetWeekDay()const
    {
        int year = Year.get();
        int month = Month.get();
        if (month < 3) { year--;  month += 12;  }
        return (year + year / 4 - year / 100 + year / 400 + (13 * month + 8) / 5 + Day.get()) % 7;
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
        Year.set(local_tm.tm_year + 1900);
        if (std::regex_match(str, m, std::wregex(LR"(([0-9]{4})/([0-9]{1,2})/([0-9]{1,2}))"))) {
            Year.set(_wtoi(m[1].str().c_str()));
            Month.set(_wtoi(m[2].str().c_str()));
            Day.set(_wtoi(m[3].str().c_str()));
            return true;
        } else if (std::regex_match(str, m, std::wregex(LR"(([0-9]{1,2})/([0-9]{1,2}))"))) {
            Month.set(_wtoi(m[1].str().c_str()));
            Day.set(_wtoi(m[2].str().c_str()));
            return true;
        } else {
            Year.set(INVALID);
            Month.set(INVALID);
            Day.set(INVALID);
            return false;
        }
    }

    bool IsInvalid() const
    {
        return Year.get() < 0 || Month.get() < 0 || Day.get() < 0;
    }

    operator bool() const
    {
        return !IsInvalid();
    }

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_NOTHROW(
        CDate,
        Year,
        Month,
        Day);
};
