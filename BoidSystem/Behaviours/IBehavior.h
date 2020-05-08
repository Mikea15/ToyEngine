#pragma once

#include "Agent.h"

#pragma warning(disable:6011)

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
        assert(world->agent->target);

        glm::vec3 desiredVelocity = glm::normalize(world->agent->target->m_position - world->agent->m_position) 
            * world->agent->m_properties->m_maxSpeed;
        return desiredVelocity - world->agent->m_velocity;
    }
};

class FleeBehaviour
    : private IBehavior
{
public:
    glm::vec3 Calculate(AgentWorld* world) override
    {
        assert(world->agent->flee);

        glm::vec3 desiredVelocity = glm::normalize(world->agent->m_position - world->agent->flee->m_position)
            * world->agent->m_properties->m_maxSpeed;
        return desiredVelocity - world->agent->m_velocity;
    }
};

class ArriveBehaviour
    : private IBehavior
{
public:
    glm::vec3 Calculate(AgentWorld* world) override
    {
        assert(world->agent->target);

        glm::vec3 desiredVelocity = world->agent->target->m_position - world->agent->m_position;
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

class FollowPathBehaviour
    : private IBehavior
{
public:
    glm::vec3 Calculate(AgentWorld* world) override
    {
        assert(world->agent->m_path);

        world->agent->m_path->UpdatePath(world->agent->m_position);

        glm::vec3 target = world->agent->m_path->GetCurrentGoal();

        glm::vec3 desiredVelocity = glm::normalize(target - world->agent->m_position)
            * world->agent->m_properties->m_maxSpeed;
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

        if (world->agent->m_position.z > world->limits.GetMax().z) { force.z = -1; }
        else if (world->agent->m_position.z < world->limits.GetMin().z) { force.z = 1.0f; }

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

        size_t neighborCount = world->neighborIndiceCount;
        for (size_t i = 0; i < neighborCount; ++i)
        {
            avgDirection += world->neighbors->at(world->neighborIndices[i]).m_heading;
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

        size_t neighborCount = world->neighborIndiceCount;
        for (size_t i = 0; i < neighborCount; ++i)
        {
            centerOfMass += world->neighbors->at(world->neighborIndices[i]).m_position;
        }

        if (neighborCount > 0)
        {
            centerOfMass /= static_cast<float>(neighborCount);

            glm::vec3 desiredVelocity = glm::normalize(centerOfMass - world->agent->m_position) * world->agent->m_properties->m_maxSpeed;
            force = desiredVelocity - world->agent->m_velocity;

            return glm::normalize(force);
        }
        return {};
    }
};


class SeparationBehaviour
    : private IBehavior
{
public:
    glm::vec3 Calculate(AgentWorld* world) override
    {
        glm::vec3 force = {};

        size_t neighborCount = world->neighborIndiceCount;
        for (size_t i = 0; i < neighborCount; ++i)
        {
            glm::vec3 toAgent = world->agent->m_position - world->neighbors->at(world->neighborIndices[i]).m_position;

            float distanceToAgent = glm::length(toAgent);
            if (distanceToAgent > 0.0f)
            {
                force += glm::normalize(toAgent) / distanceToAgent;
            }
        }

        return force;
    }
};

struct SteeringBehavior
{
    glm::vec3 CalculateWeighted(AgentWorld* world)
    {
        const Properties* weights = world->agent->m_properties;
        unsigned int feature = world->agent->m_features;

        glm::vec3 force = {};
        if (feature & eWallLimits) { force += weights->m_weightWallLimits * worldLimits.Calculate(world); }
        // if (feature & eWander) { force += weights->m_weightWander * Wander(); }
        if (feature & eSeek) { force += weights->m_weightSeek * seek.Calculate(world); }
        if (feature & eFollowPath) { force += weights->m_weightFollowPath * followPath.Calculate(world); }
        // if (feature & eArrive) { force += weights->m_weightArrive * arrive.Calculate(world); }
        // if (feature & eFlee) { force += weights->m_weightFlee * flee.Calculate(world); }
        // if (feature & eFleeRanged) { force += weights->m_weightFlee * FleeRanged(m_fleePos); }

        if (feature & eSeparation) { force += weights->m_weightSeparation * separation.Calculate(world); }
        if (feature & eCohesion) { force += weights->m_weightCohesion * cohesion.Calculate(world); }
        if (feature & eAlignment) { force += weights->m_weightAlignment * alignment.Calculate(world); }

        return glm::clamp(force, -weights->m_maxForce, weights->m_maxForce);
    }

    SeekBehaviour seek;
    FollowPathBehaviour followPath;
    AlignmentBehaviour alignment;
    CohesionBehaviour cohesion;
    SeparationBehaviour separation;
    WorldBoundsBehaviour worldLimits;
};
