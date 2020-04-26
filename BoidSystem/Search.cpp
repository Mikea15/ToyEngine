#include "Search.h"

#include "Definitions.h"
#include "Agent.h"
#include "OOP/Boid.h"

void NeighborSearch::Search(Agent* agent, AgentWorld& world, size_t maxNeighbors)
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

void NeighborSearch::Search(Boid* agent, std::vector<Boid>& neighbors, std::vector<size_t>& result, size_t& outResultCount, size_t maxNeighbors)
{
#if USE_AABB
    AABB aabb = AABB(agent->m_position, agent->m_properties->m_neighborRange);
#endif

    outResultCount = 0;
    size_t neighborSize = neighbors.size();
    for (size_t i = 0; i < neighborSize; ++i)
    {
        auto& n = neighbors[i];
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
        if (maxNeighbors > 0 && result.size() >= maxNeighbors)
        {
            break;
        }

        assert(result.size() > outResultCount);
        result[outResultCount++] = i;
    }
}
