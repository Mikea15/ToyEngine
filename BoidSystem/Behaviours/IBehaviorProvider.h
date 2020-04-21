#pragma once

#include <glm/glm.hpp>

class AgentComposition;

class ISteeringBehaviour
{
public:
    ISteeringBehaviour(AgentComposition* agent)
        : m_agent(agent)
    { }

    virtual ~ISteeringBehaviour()
    {
        m_agent = nullptr;
    };

    virtual glm::vec3 Calculate() = 0;

protected:
    AgentComposition* m_agent = nullptr;
};

