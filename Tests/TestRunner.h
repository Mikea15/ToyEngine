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
    virtual void Init() = 0;
    virtual void Run() = 0;

    std::string TestName = "BaseTest";
};


template<class T1 = BaseTest, class T2 = BaseTest>
struct QuickBench
{
    void Init1() { t1->Init(); }
    void Init2() { t2->Init(); }

    void Run1() { t1->Execute(); }
    void Run2() { t2->Execute(); }

    T1* t1;
    T2* t2;
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

    LARGE_INTEGER freq{};
    LARGE_INTEGER start{}, end{};
};

template<size_t TestCount>
struct TestRunner
{
    ~TestRunner()
    {
        for (BaseTest* t : m_tests) { delete t; }
        m_tests.clear();
        for (std::pair<BaseTest*, BaseTest*> qb : m_quickBenchs) { delete qb.first; delete qb.second; }
        m_quickBenchs.clear();
    }

    void Add(BaseTest* t) { m_tests.push_back(t); }

    void Add(BaseTest* t1, BaseTest* t2) { m_quickBenchs.push_back({ t1, t2 }); }

    void RunTests()
    {
        printf("Running Tests\n");
        for (BaseTest* t : m_tests)
        {
            RunTest(t);
        }
    }

    void RunBenchs()
    {
        printf("Running Benchs\n");
        for (std::pair<BaseTest*, BaseTest*> qb : m_quickBenchs)
        {
            float timeAvg1 = RunTest(qb.first);
            float timeAvg2 = RunTest(qb.second);

            if (timeAvg1 > timeAvg2)
            {
                const float factor = 1.0f - static_cast<float>(timeAvg2) / static_cast<float>(timeAvg1);
                printf("  [] %s ( %0.1f ms ) is faster than %s ( %0.1f ms ) by %.2f%%\n",
                    qb.second->TestName.c_str(), timeAvg2, qb.first->TestName.c_str(), timeAvg1, factor);
            }
            else {
                const float factor = 1.0f - static_cast<float>(timeAvg1) / static_cast<float>(timeAvg2);
                printf("  [] %s ( %0.1f ms ) is faster than %s ( %0.1f ms ) by %.2f%%\n",
                    qb.first->TestName.c_str(), timeAvg1, qb.second->TestName.c_str(), timeAvg2, factor);
            }
        }
    }

private:
    float RunTest(BaseTest* test)
    {
        printf("  Test: %s ", test->TestName.c_str());
        test->Init();

        int time = 0;
        for (size_t i = 0; i < TestCount; ++i)
        {
            ProfileTime prof;
            test->Run();
            time += prof.GetTime();
        }

        float timeAvg = static_cast<float>(time) / TestCount;
        printf("in %d (ms) / %0.5f (ms) avg.\n", time, timeAvg);
        return timeAvg;
    }


private:
    std::vector<BaseTest*> m_tests;
    std::vector<std::pair<BaseTest*, BaseTest*>> m_quickBenchs;
};