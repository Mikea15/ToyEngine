#pragma once

#define MULTITHREAD 1

#if MULTITHREAD
#define NUM_THREADS 16
#define USE_THREAD 1
#define USE_THREAD_JOBS 0
#define USE_ASYNC 1
#endif

#define USE_OCTREE 0
#define USE_OCTREE_PRUNE_BY_DIST 0
#define USE_AABB 1

#define NEW_OCTREE 1

#if _DEBUG
#define ENTITY_COUNT 800
#else
#define ENTITY_COUNT 2500
#endif

#ifndef APP_INFO
#if MULTITHREAD
    #if USE_THREAD
    #define TYPE "MT - " NUM_THREADS " Threads - Blocks"
    #elif USE_THREAD_JOBS
    #define TYPE "MT - " NUM_THREADS " Threads - Jobs"
    #else
    #define TYPE "MT - Async Jobs"
    #endif
#else
    #define TYPE "Single Thread"
#endif
#if USE_OCTREE
#define EXTRA " - Search: Octree"
#elif USE_AABB
#define EXTRA " - Search: AABB"
#else
#define EXTRA " - Search: Distance"
#endif
#define APP_INFO "Boid System - " TYPE " - " EXTRA
#endif

#include <imgui.h>

enum Feature : unsigned int
{
    eNone       = 0,
    eWander     = 1 << 0,
    eSeek       = 1 << 1,
    eFlee       = 1 << 2,
    eFleeRanged = 1 << 3,
    eArrive     = 1 << 4,
    eFollowPath = 1 << 5,

    eSeparation = 1 << 6,
    eCohesion   = 1 << 7,
    eAlignment  = 1 << 8,

    eWallLimits = 1 << 9
};

struct Properties
{
    unsigned int m_features = eWander;

    float m_maxSpeed = 5.0f;     // [m/s]
    float m_maxForce = 10.0f;
    float m_mass = 2.0f;
    float m_radius = 0.35f;

    float m_neighborRange = 15.0f;

    float m_weightWallLimits = 1.0f;
    float m_weightWander = 1.0f;
    float m_weightSeek = 1.0f;
    float m_weightFlee = 1.0f;
    float m_weightArrive = 1.0f;
    float m_weightFollowPath = 1.0f;

    float m_weightAlignment = 1.2f;
    float m_weightCohesion = 1.2f;
    float m_weightSeparation = 2.6f;
};

namespace Debug
{
    void ShowPanel(Properties& properties)
    {
        ImGui::Begin("Boid::Properties");
        ImGui::SliderFloat("Max Speed", &properties.m_maxSpeed, 0.0f, 50.0f);
        ImGui::SliderFloat("Max Force", &properties.m_maxForce, 0.0f, 50.0f);
        ImGui::SliderFloat("Vehicle Mass", &properties.m_mass, 0.1f, 10.0f);
        ImGui::SliderFloat("Neighbor Range", &properties.m_neighborRange, 1.0f, 35.0f);
        ImGui::Separator();

        ImGui::Text("Independent Behavior");
        ImGui::SliderFloat("Wander", &properties.m_weightWander, 0.0f, 5.0f);
        ImGui::SliderFloat("Seek", &properties.m_weightSeek, 0.0f, 5.0f);
        ImGui::SliderFloat("Flee", &properties.m_weightFlee, 0.0f, 5.0f);
        ImGui::SliderFloat("Arrive", &properties.m_weightArrive, 0.0f, 5.0f);
        ImGui::SliderFloat("FollowPath", &properties.m_weightFollowPath, 0.0f, 5.0f);
        ImGui::Separator();

        ImGui::Text("Group Behavior");
        ImGui::SliderFloat("Alignment", &properties.m_weightAlignment, 0.0f, 5.0f);
        ImGui::SliderFloat("Cohesion", &properties.m_weightCohesion, 0.0f, 5.0f);
        ImGui::SliderFloat("Separation", &properties.m_weightSeparation, 0.0f, 5.0f);
        ImGui::Separator();

        ImGui::SliderFloat("Wall Limits", &properties.m_weightWallLimits, 0.0f, 5.0f);
        ImGui::Separator();

        ImGui::End();
    }
}
