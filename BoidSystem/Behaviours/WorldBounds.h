#pragma once

#include "IBehaviorProvider.h"

#include "Composition/AgentComposition.h"
#include "Engine/Utils/MathUtils.h"

class WorldBoundsBehaviour
    : private ISteeringBehaviour
{
public:
    WorldBoundsBehaviour(AgentComposition* agent)
        : ISteeringBehaviour(agent)
    { }

    ~WorldBoundsBehaviour() override = default;

    glm::vec3 Calculate() override
    {
        glm::vec3 force = {};
        if (m_agent->m_position.x > m_agent->limits.GetMax().x) { force.x = -1; }
        else if (m_agent->m_position.x < m_agent->limits.GetMin().x) { force.x = 1.0f; }

        if (m_agent->m_position.y > m_agent->limits.GetMax().y) { force.y = -1; }
        else if (m_agent->m_position.y < m_agent->limits.GetMin().y) { force.y = 1.0f; }

        if (m_agent->m_position.z > m_agent->limits.GetMax().z) { force.x = -1; }
        else if (m_agent->m_position.z < m_agent->limits.GetMin().z) { force.x = 1.0f; }

        return force;
    }

private:

};

