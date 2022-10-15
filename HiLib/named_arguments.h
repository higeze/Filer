#pragma once
#include <tuple>

//template<typename Char, Char... Chars>
//struct basic_literal_string{};
//
//template<char... Chars>
//struct literal_string : basic_literal_string<char, Chars...> {};

template<typename Name, typename TRect>
struct parameter
{
    TRect value;
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

//template<typename CharT, std::size_t N> basic_fixed_string(const CharT(&str)[N])->basic_fixed_string<CharT, N - 1>;

//template<std::size_t N>
//using fixed_string = basic_fixed_string<char, N>;

template<std::uint64_t Bits>
struct name
{
    template<typename TRect>
    auto
        operator =(TRect t)
    {
        return parameter<name, TRect>{t};
    }
};

//template<char... Chars>
//auto operator "" _a()
//{
//    return name<literal_string<Chars...>>{};
//}

template<std::uint64_t Bits>
constexpr auto arg()
{
    return name<Bits>{};
}

constexpr auto operator "" _s(char const* str, std::size_t size)
{
    std::uint64_t bits = 0;
    for (std::size_t i = 0; i < size; ++i) {
        bits = bits << 8;
        bits = bits | str[i];
    }
    return bits;
}


//template<char Char>
//using arg = name<literal_string<Char>>;

template<typename TRect>
struct default_t
{
    TRect value;
};

template<typename TRect>
auto default_(TRect t)
{
    return default_t<TRect>{t};
}

template<typename Name, typename... Args>
struct find_name_type;

template<typename Name>
struct find_name_type<Name> {};

template<typename Name, typename TRect, typename... Args>
struct find_name_type<Name, parameter<Name, TRect>, Args...>
{
    using type = parameter<Name, TRect>;
};

template<typename Name, typename TRect, typename... Args>
struct find_name_type<Name, default_t<TRect>, Args...>
{
    using type = default_t<TRect>;
};

template<typename Name, typename TRect, typename... Args>
struct find_name_type<Name, TRect, Args...> : find_name_type<Name, Args...> {};

template<
    typename Name,
    typename... Args,
    typename Result = typename find_name_type<Name, Args...>::type>
auto get(Name, Args... args)
{
    return std::get<Result>(std::make_tuple(args...)).value;
}

//#include <type_traits>
//#include <cstdint>
//#include <boost/mpl/string.hpp>
//
//namespace mpl = boost::mpl;
//
//namespace type_str
//{
//    template <std::uint64_t str, std::size_t length, std::size_t step = 0, bool finish = (length - 1 <= step), char... chars>
//    struct to_string_impl
//    {
//        using type = mpl::string<chars...>;
//    };
//
//    template <std::uint64_t str, std::size_t length, std::size_t step, char... chars>
//    struct to_string_impl<str, length, step, false, chars...> :
//        to_string_impl<str, length, step + 1, length - 1 <= step, static_cast<char>((str & (static_cast<std::uint64_t>(0b11111111) << (8 * step))) >> (8 * step)),
//        chars...>
//    {
//    };
//
//    template <std::uint64_t str, std::size_t N = 0, bool finish = ((str >> 8 * N) & 0b11111111) == 0>
//    struct to_string
//    {
//        using type = typename to_string_impl<str, N - 1>::type;
//    };
//
//    template <std::uint64_t str, std::size_t N>
//    struct to_string<str, N, false> : to_string<str, N + 1, ((str >> 8 * N) & 0b11111111) == 0>
//    {};

//}

