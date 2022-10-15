#pragma once
#include <future>
#include "async_catch.h"

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

    template<class TRect,class Func>
    friend auto operator | (std::future<TRect> fut, Param<Func> param)->std::future<decltype(param.func(fut.get()))>
    {
        auto fun = [](std::future<TRect> fut, Func func)
        {
            return func(fut.get());
        };
        return std::async(
            std::launch::async,
            std::move(fun),
            std::move(fut),
            std::move(param.func));
    }

    template <class Func>
    friend auto operator | (std::future<void> fut, Param<Func> param)->std::future<decltype(param.func())>
    {
        auto fun = [](std::future<void> fut, Func func)
        {
            fut.wait();
            return func();
        };
        return std::async(
            std::launch::async,
            std::move(fun),
            std::move(fut),
            std::move(param.func));
    }

}then;