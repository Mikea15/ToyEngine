#pragma once

#define GLM_FORCE_SSE2
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES

#include "../TestRunner.h"
#include "Engine/Utils/MathUtils.h"

#include "Engine/Systems/AABB.h"
#include <glm/glm.hpp>

struct TestAABB
    : BaseTest
{
    GENERIC_TEST_CTOR(TestAABB);

    ~TestAABB() override {}

    void Init() override
    {
        points.clear();
        points.resize(nPoints);

        for (size_t i = 0; i < nPoints; i++)
        {
            points[i] = MathUtils::RandomInUnitSphere() * pointRange;
        }

        bb = AABB(glm::vec3(0.0f), aabbRange);
    }

    void Run() override
    {
        for (size_t i = 0; i < nPoints; ++i)
        {
            auto r = bb.Contains(points[i]);
        }
    }

protected:
    AABB bb;

    float aabbRange = 5.0f;
    float pointRange = 10.0f;

    size_t nPoints = 50000;
    size_t nTests = 1000;
    std::vector<glm::vec3> points;
};


struct TestAABBNoBranch
    : BaseTest
{
    GENERIC_TEST_CTOR(TestAABBNoBranch);

    ~TestAABBNoBranch() override {}

    void Init() override
    {
        points.clear();
        points.resize(nPoints);

        for (size_t i = 0; i < nPoints; i++)
        {
            points[i] = MathUtils::RandomInUnitSphere() * pointRange;
        }

        bb = AABB(glm::vec3(0.0f), aabbRange);
    }

    void Run() override
    {
        for (size_t i = 0; i < nPoints; ++i)
        {
            auto r = bb.ContainsNoBranch(points[i]);
        }
    }

protected:
    AABB bb;

    float aabbRange = 5.0f;
    float pointRange = 10.0f;

    size_t nPoints = 50000;
    size_t nTests = 1000;
    std::vector<glm::vec3> points;
};



