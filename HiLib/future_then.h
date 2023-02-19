#pragma once
#include <future>
#include "ThreadPool.h"

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
        return CThreadPool::GetInstance()->enqueue(
            std::move(fun),
            0,
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
        return CThreadPool::GetInstance()->enqueue(
            std::move(fun),
            0,
            std::move(fut),
            std::move(param.func));
    }

}then;