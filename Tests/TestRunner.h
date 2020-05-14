
#pragma once

#include <vector>
#include <functional>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define TIME_START() \
    LARGE_INTEGER start, end;           \
    QueryPerformanceCounter(&start);    \

#define TIME_END2(n) \
    QueryPerformanceCounter(&end);                                                      \
    int totalTime = (int)((end.QuadPart - start.QuadPart) * 1000 / freq.QuadPart);      \

#define PRINT_T(t, n) \
    printf(" avg %.4f (ms)\n", static_cast<float>(t)/static_cast<float>(n));  \


#define TIME_END() TIME_END2(1)

struct BaseTest
{
    virtual ~BaseTest() {};

    virtual void Setup() = 0;
    virtual void CoreTest() = 0;
    virtual void Execute() = 0;

    std::string TestName = "BaseTest";
};

#define GENERIC_TEST_CTOR(className) \
    className()  { TestName = #className; } \

struct ProfileTime
{
    ProfileTime()
    {
        QueryPerformanceFrequency(&freq);
        QueryPerformanceCounter(&start);
    }

    int GetTime() 
    { 
        QueryPerformanceCounter(&end);
        return static_cast<int>((end.QuadPart - start.QuadPart) * 1000 / freq.QuadPart);
    }

    std::function<void(int&)> endCb = nullptr;

    LARGE_INTEGER freq;
    LARGE_INTEGER start, end;
};

struct TestRunner
{
    ~TestRunner()
    {
        for (BaseTest* t : m_tests)
        {
            delete t;
        }
    }

    void Add(BaseTest* t) { m_tests.push_back(t); }
    void RunAll() {
        for (BaseTest* t : m_tests)
        {
            t->Execute();
        }
    }

private:
    std::vector<BaseTest*> m_tests;
};