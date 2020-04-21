#pragma once

#include "IBehaviorProvider.h"

#include "../OOP/Boid.h"

class SeekBehaviour
    : private ISteeringBehaviour
{
public:
    SeekBehaviour(Boid* actor)
        : ISteeringBehaviour(actor)
    { }

    ~SeekBehaviour() override = default;

    glm::vec3 Calculate() override
    {
        glm::vec3 desiredVelocity = glm::normalize(actor->m_targetPos - actor->m_position) * actor->m_properties->m_maxSpeed;
        return desiredVelocity - actor->m_velocity;
    }

private:

};

