#pragma once

#include "IBehaviorProvider.h"

#include "../OOP/Boid.h"

class FleeBehaviour
    : private ISteeringBehaviour
{
public:
    FleeBehaviour(Boid* actor)
        : ISteeringBehaviour(actor)
    { }

    ~FleeBehaviour() override = default;

    glm::vec3 Calculate() override
    {
        glm::vec3 desiredVelocity = glm::normalize(actor->m_position - actor->m_targetPos) * actor->m_properties->m_maxSpeed;
        return desiredVelocity - actor->m_velocity;
    }

private:

};

