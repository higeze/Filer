#pragma once

#include <string>

template<typename T>
class find_ignorecase_t
{
private:
    const std::basic_string<T>& m_str_needle;
    const typename std::basic_string<T>::size_type m_pos;
public:
    find_ignorecase_t(const std::basic_string<T>& str_needle, typename std::basic_string<T>::size_type pos)
        :m_str_needle(str_needle), m_pos(pos){}

    std::basic_string<T>::size_type operator()(const std::basic_string<T>& str_haystack)
    {
        auto iter = std::search(
            std::next(str_haystack.cbegin() + m_pos), str_haystack.cend(),
            m_str_needle.cbegin(),   m_str_needle.cend(),
            [](const T& ch1, const T& ch2) { return std::tolower(ch1) == std::tolower(ch2); });
        if (iter == str_haystack.cend()) {
            return std::basic_string<T>::npos;
        } else {
            return std::distance(str_haystack.begin(), iter);
        }
    }
};

template<typename T>
inline find_ignorecase_t<T> find_ignorecase(const std::basic_string<T>& str_needle, typename std::basic_string<T>::size_type pos = 0)
{
    return find_ignorecase_t<T>(str_needle, pos);
}

template<typename T>
inline typename std::basic_string<T>::size_type operator|(const std::basic_string<T>& str_haystack, find_ignorecase_t<T> func)
{
    return func(str_haystack);
}
