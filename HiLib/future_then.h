#pragma once
#include <future>

constexpr struct future_then
{
private:
    template<class Func>
    struct Param
    {
        Func func;
    };
public:
    template<class Func>
    Param<Func> operator ()(Func f)const
    {
        return { std::move(f) };
    }

    template<class T,class Func>
    friend auto operator | (std::future<T> fut, Param<Func> param)->std::future<decltype(param.func(fut.get()))>
    {
        return std::async([](std::future<T> fut, Func func)
        {
            return func(fut.get());
        }, std::move(fut), std::move(param.func));
    }

    template <class Func>
    friend auto operator | (std::future<void> fut, Param<Func> param)->std::future<decltype(param.func())>
    {
        return std::async([](std::future<void> fut, Func func)
        {
            fut.wait();
            return func(); 
        }, std::move(fut), std::move(param.func));
    }

}then;