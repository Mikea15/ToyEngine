#pragma once
#include <functional>
#include <assert.h>

struct IBaseJob
{
    virtual void Execute() = 0;
};

struct GenericJob
    : IBaseJob
{
    GenericJob(std::function<void(void)> func)
        : fn(func)
    {
        assert(fn);
    }

    void Execute() 
    {
        fn();
    }

    std::function<void(void)> fn{};
};
   