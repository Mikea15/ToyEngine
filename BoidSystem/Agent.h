#pragma once

#include "Definitions.h"

#include <glm/glm.hpp>
#include <vector>

#include "Path.h"

#include "Engine/Systems/AABB.h"
#include "Engine/Utils/MathUtils.h"

struct Agent
{
    // base entity
    unsigned int m_id = -1;
    glm::vec3 m_position{};
    float m_radius;

    // moving entity
    glm::vec3 m_velocity{};
    glm::vec3 m_heading{};
    float m_mass;
    float m_maxSpeed;
    float m_maxForce;
    float m_maxTurnRate;

    // vehicle.
    // gameWorld state
    // steering behavior
    
    // steering
    Agent* target = nullptr;
    Agent* flee = nullptr;

    Path* m_path = nullptr;

    unsigned int m_features = eNone;
    Properties* m_properties = nullptr;

    static Agent CreateAgent()
    {
        Agent a;
        a.m_id = ++ID;
        return a;
    }

    bool operator==(const Agent& rhs) const { return m_id == rhs.m_id; }
    bool operator!=(const Agent& rhs) const { return m_id != rhs.m_id; }

private:
    static unsigned int ID;
};

struct AgentWorld
{
    Agent* agent = nullptr;

    std::vector<size_t> neighborIndices;
    size_t neighborIndiceCount = 0;

    std::vector<Agent>* neighbors;

    AABB limits;
};

unsigned int Agent::ID = 0;

void Search(Agent* agent, AgentWorld& world, size_t maxNeighbors = 0)
{
#if USE_AABB
    AABB aabb = AABB(agent->m_position, agent->m_properties->m_neighborRange);
#endif

    world.neighborIndiceCount = 0;
    size_t neighborSize = world.neighbors->size();
    for (size_t i = 0; i < neighborSize; ++i)
    {
        auto& n = world.neighbors->at(i);
        if (*agent == n) { continue; }

#if USE_AABB
        if (!aabb.Contains(n.m_position)) { continue; }
#endif
#if !USE_AABB || (USE_AABB && USE_AABB_PRUNE_BY_DIST)
        glm::vec3 toAgent = agent->m_position - n.m_position;
        float distanceSqToAgent = glm::length2(toAgent);
        if (distanceSqToAgent > agent->m_properties->m_neighborRange * agent->m_properties->m_neighborRange)
        {
            continue;
        }
#endif
        if (maxNeighbors > 0 && world.neighborIndiceCount >= maxNeighbors)
        {
            break;
        }

        assert(world.neighborIndices.size() > world.neighborIndiceCount);
        world.neighborIndices[world.neighborIndiceCount++] = i;
    }
}