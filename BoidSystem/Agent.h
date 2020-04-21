#pragma once

#include <glm/glm.hpp>

#include <vector>
#include "Engine/Systems/AABB.h"

struct Agent
{
    glm::vec3 m_position;
    glm::vec3 m_velocity;
    glm::vec3 m_heading;

    std::vector<Agent>::iterator neighborStart;
    std::vector<size_t>::iterator neighborEnd;

    std::vector<size_t>::iterator neighborIndexStart;
    std::vector<size_t>::iterator neighborIndexEnd;

    AABB limits;

    unsigned int m_id = -1;
    static unsigned int ID;
};

unsigned int Agent::ID = 0;