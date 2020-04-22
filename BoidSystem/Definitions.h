#pragma once

#define MULTITHREAD 0
#define NUM_THREADS 4

#define USE_OCTREE 0
#define USE_AABB 0

#if _DEBUG
#define ENTITY_COUNT 400
#else
#define ENTITY_COUNT 2000
#endif

enum Feature : unsigned int
{
    eNone = 0,
    eWander = 1 << 0,
    eSeek = 1 << 1,
    eFlee = 1 << 2,
    eFleeRanged = 1 << 3,
    eArrive = 1 << 4,

    eSeparation = 1 << 5,
    eCohesion = 1 << 6,
    eAlignment = 1 << 7,

    eWallLimits = 1 << 8
};

struct Properties
{
    unsigned int m_features = eWander;

    float m_maxSpeed = 5.0f;     // [m/s]
    float m_maxForce = 10.0f;
    float m_mass = 2.0f;
    float m_radius = 1.0f;

    float m_neighborRange = 3.0f;

    float m_weightWallLimits = 1.0f;
    float m_weightWander = 1.0f;
    float m_weightSeek = 1.0f;
    float m_weightFlee = 1.0f;
    float m_weightArrive = 1.0f;

    float m_weightAlignment = 1.2f;
    float m_weightCohesion = 1.8f;
    float m_weightSeparation = 0.6f;
};