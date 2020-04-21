#pragma once

#include "IBehaviorProvider.h"

#include "Composition/AgentComposition.h"
#include "Engine/Utils/MathUtils.h"

class ArriveBehaviour
    : private ISteeringBehaviour
{
public:
    ArriveBehaviour(AgentComposition* actor)
        : ISteeringBehaviour(actor)
    { }

    ~ArriveBehaviour() override = default;

    glm::vec3 Calculate() override
    {
        glm::vec3 desiredVelocity = m_agent->m_targetPos - m_agent->m_position;
        float distance = glm::length(desiredVelocity);
        desiredVelocity = glm::normalize(desiredVelocity);

        float speed = m_agent->m_properties->m_maxSpeed;
        float arriveRadius = 4.0f;
        if (distance <= arriveRadius)
        {
            speed = MathUtils::Lerp(0.0f, speed, distance / arriveRadius);
        }

        desiredVelocity *= speed;
        return desiredVelocity - m_agent->m_velocity;
    }

private:

};

