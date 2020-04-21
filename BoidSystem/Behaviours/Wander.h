#pragma once

#include "IBehaviorProvider.h"

#include "../OOP/Boid.h"
#include "Engine/Utils/MathUtils.h"

class WanderBehaviour
    : private ISteeringBehaviour
{
public:
    WanderBehaviour(Boid* actor)
        : ISteeringBehaviour(actor)
    { }

    ~WanderBehaviour() override = default;

    // MA: WIP
    glm::vec3 Calculate() override
    {
        float wanderRadius = 2.0f;
        glm::vec3 jitterDirection = glm::normalize(
            glm::vec3(
                MathUtils::Rand01() * 1.0f,
                MathUtils::Rand01() * 1.0f,
                MathUtils::Rand01() * 1.0f
            )) * wanderRadius;

        float wanderDistance = 0.0f;
        glm::vec3 targetPosJitter = actor->m_position + glm::normalize(actor->m_velocity) * wanderDistance + jitterDirection;

        glm::vec3 desiredVelocity = glm::normalize(targetPosJitter - actor->m_position);
        return desiredVelocity - actor->m_velocity;
    }

private:

};

