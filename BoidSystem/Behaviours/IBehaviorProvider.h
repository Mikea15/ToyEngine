#pragma once

#include <glm/glm.hpp>

class Boid;

class ISteeringBehaviour
{
public:
    ISteeringBehaviour(Boid* boid)
        : actor(boid)
    { }

    virtual ~ISteeringBehaviour()
    {
        actor = nullptr;
    };

    virtual glm::vec3 Calculate() = 0;

protected:
    Boid* actor = nullptr;
};

