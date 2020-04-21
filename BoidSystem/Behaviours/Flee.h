#pragma once

#include "IBehaviorProvider.h"

#include "Composition/AgentComposition.h"

class FleeBehaviour
    : private ISteeringBehaviour
{
public:
    FleeBehaviour(AgentComposition* actor)
        : ISteeringBehaviour(actor)
    { }

    ~FleeBehaviour() override = default;

    glm::vec3 Calculate() override
    {
        glm::vec3 desiredVelocity = glm::normalize(m_agent->m_position - m_agent->m_targetPos) * m_agent->m_properties->m_maxSpeed;
        return desiredVelocity - m_agent->m_velocity;
    }

private:

};

