#pragma once

#include <glm/glm.hpp>

struct Agent
{


    glm::vec3 m_pos;
    glm::vec3 m_velocity;
    glm::vec3 m_heading;

    unsigned int m_id = -1;
    static unsigned int ID;
};

unsigned int Agent::ID = 0;