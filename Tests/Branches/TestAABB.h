#pragma once

#define GLM_FORCE_SSE2
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES

#include "../TestRunner.h"
#include "Engine/Utils/MathUtils.h"

#include "Engine/Systems/AABB.h"
#include <glm/glm.hpp>

struct TestAABB
    : BaseCompareTest
{
    GENERIC_TEST_CTOR(TestAABB);

    ~TestAABB() override {}

    void Setup() override
    {
        points.clear();
        points.resize(nPoints);

        for (size_t i = 0; i < nPoints; i++)
        {
            points[i] = MathUtils::RandomInUnitSphere() * pointRange;
        }

        bb = AABB(glm::vec3(0.0f), aabbRange);
    }

    void Execute() override
    {
        Setup();
        printf("Test: %s ", TestName.c_str());
        int t1 = 0, t2 = 0;
        {
            ProfileTime time;
            for (size_t i = 0; i < nTests; ++i)
            {
                CoreTest1();
            }
            t1 = time.GetTime();
        }
        {
            ProfileTime time;
            for (size_t i = 0; i < nTests; ++i)
            {
                CoreTest2();
            }
            t2 = time.GetTime();
        }

        if (t1 > t2) {
            const float factor = 1.0f - static_cast<float>(t2) / static_cast<float>(t1);
            printf(" - Test2 is %.2f%% faster than Test1. T2: %d vs T1: %d  ( T2(true): %d - T1(true): %d )\n",
                factor, t2, t1, output2, output1);
        }
        else {
            const float factor = 1.0f - static_cast<float>(t1) / static_cast<float>(t2);
            printf(" - Test1 is %.2f%% faster than Test2. T1: %d vs T2: %d ( T1(true): %d - T2(true): %d )\n",
                factor, t1, t2, output1, output2);
        }
    }

    void CoreTest1() override
    {
        output1 = 0;
        for (size_t i = 0; i < nPoints; ++i)
        {
            if (bb.Contains(points[i])) 
                ++output1;
        }
    }

    void CoreTest2() override
    {
        output2 = 0;
        for (size_t i = 0; i < nPoints; ++i)
        {
            // if( bb.ContainsSIMD(points[i]))
            if (bb.ContainsNoBranch(points[i])) 
                ++output2;
        }
    }

protected:
    int output1 = -1;
    int output2 = -1;

    AABB bb;

    float aabbRange = 5.0f;
    float pointRange = 10.0f;

    size_t nPoints = 50000;
    size_t nTests = 1000;
    std::vector<glm::vec3> points;
};



