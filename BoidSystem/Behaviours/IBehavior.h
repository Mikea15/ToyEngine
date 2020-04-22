#pragma once

#include "Agent.h"

class IBehavior
{
public:
    virtual glm::vec3 Calculate(AgentWorld* world) = 0;
};

class SeekBehaviour
   : private IBehavior
{
public:
    glm::vec3 Calculate(AgentWorld* world) override
    {
        glm::vec3 desiredVelocity = glm::normalize(world->target->m_position - world->agent->m_position) * world->agent->m_properties->m_maxSpeed;
        return desiredVelocity - world->agent->m_velocity;
    }
};

class FleeBehaviour
    : private IBehavior
{
public:
    glm::vec3 Calculate(AgentWorld* world) override
    {
        glm::vec3 desiredVelocity = glm::normalize(world->agent->m_position - world->target->m_position) * world->agent->m_properties->m_maxSpeed;
        return desiredVelocity - world->agent->m_velocity;
    }
};

class ArriveBehaviour
    : private IBehavior
{
public:
    glm::vec3 Calculate(AgentWorld* world) override
    {
        glm::vec3 desiredVelocity = world->target->m_position - world->agent->m_position;
        float distance = glm::length(desiredVelocity);
        desiredVelocity = glm::normalize(desiredVelocity);

        float speed = world->agent->m_properties->m_maxSpeed;
        float arriveRadius = 4.0f;
        if (distance <= arriveRadius)
        {
            speed = MathUtils::Lerp(0.0f, speed, distance / arriveRadius);
        }

        desiredVelocity *= speed;
        return desiredVelocity - world->agent->m_velocity;
    }
};


class WanderBehaviour
    : private IBehavior
{
public:
    // MA: WIP
    glm::vec3 Calculate(AgentWorld* world) override
    {
        float wanderRadius = 2.0f;
        glm::vec3 jitterDirection = glm::normalize(
            glm::vec3(
                MathUtils::Rand01() * 1.0f,
                MathUtils::Rand01() * 1.0f,
                MathUtils::Rand01() * 1.0f
            )) * wanderRadius;

        float wanderDistance = 0.0f;
        glm::vec3 targetPosJitter = world->agent->m_position + glm::normalize(world->agent->m_velocity) 
            * wanderDistance + jitterDirection;

        glm::vec3 desiredVelocity = glm::normalize(targetPosJitter - world->agent->m_position);
        return desiredVelocity - world->agent->m_velocity;
    }

private:
    glm::vec3 m_wanderTarget;
};

class WorldBoundsBehaviour
    : private IBehavior
{
public:
    glm::vec3 Calculate(AgentWorld* world) override
    {
        glm::vec3 force = {};
        if (world->agent->m_position.x > world->limits.GetMax().x) { force.x = -1; }
        else if (world->agent->m_position.x < world->limits.GetMin().x) { force.x = 1.0f; }

        if (world->agent->m_position.y > world->limits.GetMax().y) { force.y = -1; }
        else if (world->agent->m_position.y < world->limits.GetMin().y) { force.y = 1.0f; }

        if (world->agent->m_position.z > world->limits.GetMax().z) { force.x = -1; }
        else if (world->agent->m_position.z < world->limits.GetMin().z) { force.x = 1.0f; }

        return force;
    }
};

class AlignmentBehaviour
     : private IBehavior
{
public:
    glm::vec3 Calculate(AgentWorld* world) override
    {
        glm::vec3 avgDirection = {};

        std::vector<Agent>::iterator it = world->neighborStart;
        std::vector<Agent>::iterator end = world->neighborEnd;

        size_t neighborCount = end - it;
        for (; it != end; ++it)
        {
            avgDirection += it->m_heading;
        }

        if (neighborCount > 0)
        {
            avgDirection /= static_cast<float>(neighborCount);
            avgDirection -= world->agent->m_heading;
        }

        return avgDirection;
    }
};


class CohesionBehaviour
    : private IBehavior
{
public:
    glm::vec3 Calculate(AgentWorld* world) override
    {
        glm::vec3 centerOfMass = {};
        glm::vec3 force = {};

        std::vector<Agent>::iterator it = world->neighborStart;
        std::vector<Agent>::iterator end = world->neighborEnd;

        size_t neighborCount = end - it;
        for (; it != end; ++it)
        {
            centerOfMass += it->m_position;
        }

        if (neighborCount > 0)
        {
            centerOfMass /= static_cast<float>(neighborCount);
            glm::vec3 desiredVelocity = glm::normalize(world->target->m_position - world->agent->m_position) * world->agent->m_properties->m_maxSpeed;
            force = desiredVelocity - world->agent->m_velocity;

            return glm::normalize(force);
        }

    }
};


class SeparationBehaviour
    : private IBehavior
{
public:
    glm::vec3 Calculate(AgentWorld* world) override
    {
        glm::vec3 force = {};

        std::vector<Agent>::iterator it = world->neighborStart;
        std::vector<Agent>::iterator end = world->neighborEnd;

        size_t neighborCount = end - it;
        for (; it != end; ++it)
            {
            glm::vec3 toAgent = world->agent->m_position - it->m_position;

            float distanceToAgent = glm::length(toAgent);
            if (distanceToAgent > 0.0f)
            {
                force += glm::normalize(toAgent) / distanceToAgent;
            }
        }

        return force;
    }
};
