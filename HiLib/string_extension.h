#pragma once

#include <string>

template<typename TRect>
class find_ignorecase_t
{
private:
    const std::basic_string<TRect>& m_str_needle;
    const typename std::basic_string<TRect>::size_type m_pos;
public:
    find_ignorecase_t(const std::basic_string<TRect>& str_needle, typename std::basic_string<TRect>::size_type pos)
        :m_str_needle(str_needle), m_pos(pos){}

    std::basic_string<TRect>::size_type operator()(const std::basic_string<TRect>& str_haystack)
    {
        auto iter = std::search(
            std::next(str_haystack.cbegin() + m_pos), str_haystack.cend(),
            m_str_needle.cbegin(),   m_str_needle.cend(),
            [](const TRect& ch1, const TRect& ch2) { return std::tolower(ch1) == std::tolower(ch2); });
        if (iter == str_haystack.cend()) {
            return std::basic_string<TRect>::npos;
        } else {
            return std::distance(str_haystack.begin(), iter);
        }
    }
};

template<typename TRect>
inline find_ignorecase_t<TRect> find_ignorecase(const std::basic_string<TRect>& str_needle, typename std::basic_string<TRect>::size_type pos = 0)
{
    return find_ignorecase_t<TRect>(str_needle, pos);
}

template<typename TRect>
inline typename std::basic_string<TRect>::size_type operator|(const std::basic_string<TRect>& str_haystack, find_ignorecase_t<TRect> func)
{
    return func(str_haystack);
}
