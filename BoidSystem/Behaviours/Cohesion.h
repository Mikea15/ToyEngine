#pragma once

#include "IBehaviorProvider.h"

#include "../OOP/Boid.h"
#include "Engine/Utils/MathUtils.h"

class CohesionBehaviour
    : private ISteeringBehaviour
{
public:
    CohesionBehaviour(Boid* actor)
        : ISteeringBehaviour(actor)
    { }

    ~CohesionBehaviour() override = default;

    glm::vec3 Calculate() override
    {
#if 0
        glm::vec3 centerOfMass = {};
        glm::vec3 force = {};
#if !USE_OCTREE
        size_t neighborCount = actor->m_currentNeighborCount;
#else
        size_t neighborCount = actor->indices.size();
#endif
        for (size_t i = 0; i < neighborCount; ++i)
        {
#if !USE_OCTREE
            centerOfMass += actor->neighbors[*(actor->m_neighborIndices + i)].m_position;
#else
            centerOfMass += actor->neighbors[actor->indices.get(i)].m_position;
#endif
    }

        if (neighborCount > 0)
        {
            centerOfMass /= static_cast<float>(neighborCount);
            force = Seek(centerOfMass);

            return glm::normalize(force);
        }
#endif

        return {};
    }

private:

};

