#pragma once

#include "IBehaviorProvider.h"

#include "../OOP/Boid.h"
#include "Engine/Utils/MathUtils.h"

class ArriveBehaviour
    : private ISteeringBehaviour
{
public:
    ArriveBehaviour(Boid* actor)
        : ISteeringBehaviour(actor)
    { }

    ~ArriveBehaviour() override = default;

    glm::vec3 Calculate() override
    {
        glm::vec3 desiredVelocity = actor->m_targetPos - actor->m_position;
        float distance = glm::length(desiredVelocity);
        desiredVelocity = glm::normalize(desiredVelocity);

        float speed = actor->m_properties->m_maxSpeed;
        float arriveRadius = 4.0f;
        if (distance <= arriveRadius)
        {
            speed = MathUtils::Lerp(0.0f, speed, distance / arriveRadius);
        }

        desiredVelocity *= speed;
        return desiredVelocity - actor->m_velocity;
    }

private:

};

