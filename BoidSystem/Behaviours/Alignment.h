#pragma once

#include "IBehaviorProvider.h"

#include "../OOP/Boid.h"
#include "Engine/Utils/MathUtils.h"

class AlignmentBehaviour
    : private ISteeringBehaviour
{
public:
    AlignmentBehaviour(Boid* actor)
        : ISteeringBehaviour(actor)
    { }

    ~AlignmentBehaviour() override = default;

    glm::vec3 Calculate() override
    {
#if 0
        glm::vec3 avgDirection = {};
#if !USE_OCTREE
        size_t neighborCount = actor->m_currentNeighborCount;
#else
        size_t neighborCount = actor->indices.size();
#endif
        for (size_t i = 0; i < neighborCount; ++i)
        {
#if !USE_OCTREE
            avgDirection += actor->neighbors[*(actor->m_neighborIndices + i)].m_direction;
#else
            avgDirection += actor->neighbors[actor->indices.get(i)].m_direction;
#endif
        }

        if (neighborCount > 0)
        {
            avgDirection /= static_cast<float>(neighborCount);
            avgDirection -= actor->m_direction;
        }

        return avgDirection;
#endif 
        return {};
    }

private:

};

