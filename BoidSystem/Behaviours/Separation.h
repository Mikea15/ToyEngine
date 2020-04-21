#pragma once

#include "IBehaviorProvider.h"

#include "Composition/AgentComposition.h"
#include "Engine/Utils/MathUtils.h"

class SeparationBehaviour
    : private ISteeringBehaviour
{
public:
    SeparationBehaviour(AgentComposition* actor)
        : ISteeringBehaviour(actor)
    { }

    ~SeparationBehaviour() override = default;

    glm::vec3 Calculate() override
    {
#if 0
        glm::vec3 force = {};
#if !USE_OCTREE
        size_t neighborCount = actor->m_currentNeighborCount;
#else
        size_t neighborCount = actor->indices.size();
#endif
        for (size_t i = 0; i < neighborCount; ++i)
        {
#if !USE_OCTREE
            glm::vec3 toAgent = actor->m_position - actor->neighbors[*(actor->m_neighborIndices + i)].m_position;
#else
            glm::vec3 toAgent = actor->m_position - actor->neighbors[actor->indices.get(i)].m_position;
#endif
            float distanceToAgent = glm::length(toAgent);
            if (distanceToAgent > 0.0f)
            {
                force += glm::normalize(toAgent) / distanceToAgent;
            }
        }

        return force;
#endif
        return {};
    }

private:

};

