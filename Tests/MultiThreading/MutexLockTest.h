#pragma once

#include "../TestRunner.h"

#include "Core/CustomMutex.h"
#include "Engine/Systems/Terrain.h"

struct StdMutexLockTest
    : BaseTest
{
    GENERIC_TEST_CTOR(StdMutexLockTest);

    ~StdMutexLockTest() override
    {
        delete pTerrain;
    }

    void Init() override
    {
        pTerrain = new Terrain<>(1.0f, 100.0f, 100.0f, 100.0f);
    }

    void Run() override
    {
        pTerrain->GenerateMesh();
    }

    Terrain<>* pTerrain = nullptr;
};

struct CustomMutexLockTest
    : BaseTest
{
    GENERIC_TEST_CTOR(CustomMutexLockTest);

    ~CustomMutexLockTest() override
    {
        delete pTerrain;
    }

    void Init() override
    {
        pTerrain = new Terrain<CustomMutex>(1.0f, 100.0f, 100.0f, 100.0f);
    }

    void Run() override
    {
        pTerrain->GenerateMesh();
    }

    Terrain<CustomMutex>* pTerrain = nullptr;
};