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

    //template<class T,class Func>
    //friend auto operator | (std::future<T> fut, Param<Func> param)->std::future<decltype(param.func(fut.get()))>
    //{
    //    auto fun = [](std::future<T> fut, Func func)
    //    {
    //        return func(fut.get());
    //    };
    //    return CThreadPool::GetInstance()->enqueue(
    //        "then",
    //        0,
    //        std::move(fun),
    //        std::move(fut),
    //        std::move(param.func));
    //}

    template <class Func>
    friend auto operator | (std::future<void> fut, Param<Func> param)->std::future<decltype(param.func())>
    {
        auto fun = [fut = std::move(fut), func = std::move(param.func)]()mutable ->decltype(param.func())
            {
                fut.wait();
                return func();
            };
        return CThreadPool::GetInstance()->enqueue(
            "then",
            0,
            std::move(fun));

        //auto fun = [](std::future<void> fut, Func func)->decltype(func())
        //{
        //    fut.wait();
        //    return func();
        //};
        //return CThreadPool::GetInstance()->enqueue(
        //    "then",
        //    0,
        //    std::move(fun),
        //    std::move(fut),
        //    std::move(param.func));
        //return std::future<void>();
    }

}then;