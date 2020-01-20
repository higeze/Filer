#pragma once
#include <tuple>

template<typename Char, Char... Chars>
struct basic_literal_string{};

template<char... Chars>
struct literal_string : basic_literal_string<char, Chars...> {};

template<typename Name, typename T>
struct parameter
{
    T value;
};

//template<typename CharT, std::size_t N>
//struct basic_fixed_string
//{
//    CharT buf[N+1]{};
//    constexpr basic_fixed_string(const CharT* str)
//    {
//        for (unsigned i = 0; i != N; i++) {
//            buf[i] = str[i];
//        }
//    }
//    auto operator<=>(const basic_fixed_string&) = default;
//    constexpr operator const char* ()const { return buf; }
//};
//
//template<typename CharT, std::size_t N> basic_fixed_string(const CharT(&str)[N])->basic_fixed_string<CharT, N - 1>;
//
//template<std::size_t N>
//using fixed_string = basic_fixed_string<char, N>;

template<typename... Chars>
struct name
{
    template<typename T>
    auto
        operator =(T t)
    {
        return parameter<name, T>{t};
    }
};

template<char... Chars>
auto operator "" _a()
{
    return name<literal_string<Chars...>>{};
}

template<char Char>
using arg = name<literal_string<Char>>;

template<typename T>
struct default_t
{
    T value;
};

template<typename T>
auto default_(T t)
{
    return default_t<T>{t};
}


template<typename Name, typename... Args>
struct find_name_type;

template<typename Name>
struct find_name_type<Name> {};

template<typename Name, typename T, typename... Args>
struct find_name_type<Name, parameter<Name, T>, Args...>
{
    using type = parameter<Name, T>;
};

template<typename Name, typename T, typename... Args>
struct find_name_type<Name, default_t<T>, Args...>
{
    using type = default_t<T>;
};


template<typename Name, typename T, typename... Args>
struct find_name_type<Name, T, Args...> : find_name_type<Name, Args...> {};
;



template<
    typename Name,
    typename... Args,
    typename Result = typename find_name_type<Name, Args...>::type
>
auto
get(Name, Args... args)
{
    return std::get<Result>(std::make_tuple(args...)).value;
}


