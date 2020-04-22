#pragma once

#include "Definitions.h"

#include <glm/glm.hpp>
#include <vector>

#include "Engine/Systems/AABB.h"
#include "Engine/Utils/MathUtils.h"

struct Agent
{
    glm::vec3 m_position;
    glm::vec3 m_velocity;
    glm::vec3 m_heading;

    unsigned int m_id = -1;

    unsigned int m_features = eSeek;
    Properties* m_properties = nullptr;

private:
    static unsigned int ID;
};

struct AgentWorld
{
    Agent* agent;
    Agent* target;

    std::vector<Agent>::iterator neighborStart;
    std::vector<Agent>::iterator neighborEnd;

    std::vector<size_t>::iterator neighborIndexStart;
    std::vector<size_t>::iterator neighborIndexEnd;
    
    AABB limits;
};

unsigned int Agent::ID = 0;