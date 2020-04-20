#pragma once

#include <glm/glm.hpp>

#include "Agent.h"

#if 0

struct SteeringBehavior
{
    glm::vec3 Seek(glm::vec3 target)
    {
        glm::vec3 desiredVelocity = glm::normalize(target - m_agent->m_position) * m_agent->m_properties->m_maxSpeed;
        return desiredVelocity - m_agent->m_velocity;
    }

    glm::vec3 Flee(glm::vec3 target)
    {
        glm::vec3 desiredVelocity = glm::normalize(m_agent->m_position - target) * m_agent->m_properties->m_maxSpeed;
        return desiredVelocity - m_agent->m_velocity;
    }

    glm::vec3 FleeRanged(glm::vec3 target)
    {
        glm::vec3 desiredVelocity = glm::normalize(m_agent->m_position - target);
        float distance = glm::length(desiredVelocity);
        if (distance > 0.0f)
        {
            desiredVelocity = glm::normalize(desiredVelocity);
        }

        float maxVelocity = m_agent->m_properties->m_maxSpeed;
        float fleeRadius = 0.8f;
        if (distance >= fleeRadius)
        {
            maxVelocity = 0.0f;
            // maxVelocity = MathUtils::Lerp(0.0f, m_maxSpeed, 1.0f - (distance / fleeRadius));
        }

        desiredVelocity *= maxVelocity;
        return desiredVelocity - m_agent->m_velocity;
    }

    // NOTE: WIP
    glm::vec3 Wander()
    {
        float wanderRadius = 2.0f;
        glm::vec3 jitterDirection = glm::normalize(
            glm::vec3(
                MathUtils::Rand01() * 1.0f,
                MathUtils::Rand01() * 1.0f,
                MathUtils::Rand01() * 1.0f
            )) * wanderRadius;

        float wanderDistance = 0.0f;
        glm::vec3 targetPosJitter = m_agent->m_position + glm::normalize(m_agent->m_velocity) * wanderDistance + jitterDirection;

        glm::vec3 desiredVelocity = glm::normalize(targetPosJitter - m_agent->m_position);
        return desiredVelocity - m_agent->m_velocity;
    }

    glm::vec3 Arrive(glm::vec3 target)
    {
        glm::vec3 desiredVelocity = target - m_agent->m_position;
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

    void FindNearbyNeighbors(std::vector<Boid>& neighbors)
    {
        // NOTE (MA): One time allocation of the max neighbors size.
        if (m_neighborIndices == nullptr)
        {
            m_neighborIndices = AllocNeighborIndices(neighbors.size());
        }

        assert(m_neighborIndices != nullptr);

        m_currentNeighborCount = 0;

#if USE_AABB
        AABB aabb = AABB(m_position, m_properties->m_neighborRange);
#endif

        size_t neighborSize = neighbors.size();
        for (size_t i = 0; i < neighborSize; ++i)
        {
            auto& n = neighbors[i];
            if (*m_agent == n)
            {
                continue;
            }
#if USE_AABB
            if (!aabb.Contains(n.m_position))
            {
                continue;
            }
#else
            glm::vec3 toAgent = m_agent->m_position - n.m_position;
            float distanceSqToAgent = glm::length2(toAgent);
            if (distanceSqToAgent > m_agent->m_properties->m_neighborRange * m_agent->m_properties->m_neighborRange)
            {
                continue;
            }
#endif
            * (m_neighborIndices + m_currentNeighborCount++) = i;
        }
    }

    glm::vec3 Separation(std::vector<Boid>& neighbors, std::vector<size_t>& indices)
    {
        glm::vec3 force = {};
#if !USE_OCTREE
        size_t neighborCount = m_currentNeighborCount;
#else
        size_t neighborCount = indices.size();
#endif
        for (size_t i = 0; i < neighborCount; ++i)
        {
#if !USE_OCTREE
            glm::vec3 toAgent = m_agent->m_position - neighbors[*(m_neighborIndices + i)].m_position;
#else
            glm::vec3 toAgent = m_position - neighbors[indices[i]].m_position;
#endif
            float distanceToAgent = glm::length(toAgent);
            if (distanceToAgent > 0.0f)
            {
                force += glm::normalize(toAgent) / distanceToAgent;
            }
        }

        return force;
    }

    glm::vec3 Alignment(std::vector<Boid>& neighbors, std::vector<size_t>& indices)
    {
        glm::vec3 avgDirection = {};
#if !USE_OCTREE
        size_t neighborCount = m_currentNeighborCount;
#else
        size_t neighborCount = indices.size();
#endif
        for (size_t i = 0; i < neighborCount; ++i)
        {
#if !USE_OCTREE
            avgDirection += neighbors[*(m_neighborIndices + i)].m_direction;
#else
            avgDirection += neighbors[indices[i]].m_direction;
#endif
        }

        if (neighborCount > 0)
        {
            avgDirection /= static_cast<float>(neighborCount);
            avgDirection -= m_agent->m_direction;
        }

        return avgDirection;
    }

    glm::vec3 Cohesion(std::vector<Boid>& neighbors, std::vector<size_t>& indices)
    {
        glm::vec3 centerOfMass = {};
        glm::vec3 force = {};
#if !USE_OCTREE
        size_t neighborCount = m_currentNeighborCount;
#else
        size_t neighborCount = indices.size();
#endif
        for (size_t i = 0; i < neighborCount; ++i)
        {
#if !USE_OCTREE
            centerOfMass += neighbors[*(m_neighborIndices + i)].m_position;
#else
            centerOfMass += neighbors[indices[i]].m_position;
#endif
        }

        if (neighborCount > 0)
        {
            centerOfMass /= static_cast<float>(neighborCount);
            force = Seek(centerOfMass);

            return glm::normalize(force);
        }

        return {};
    }

    glm::vec3 WallLimits(AABB limits = AABB(glm::vec3(0.0f, 25.0f, 0.0f), 50))
    {
        glm::vec3 force = {};
        if (m_agent->m_position.x > limits.GetMax().x) { force.x = -1; }
        else if (m_agent->m_position.x < limits.GetMin().x) { force.x = 1.0f; }

        if (m_agent->m_position.y > limits.GetMax().y) { force.y = -1; }
        else if (m_agent->m_position.y < limits.GetMin().y) { force.y = 1.0f; }

        if (m_agent->m_position.z > limits.GetMax().z) { force.x = -1; }
        else if (m_agent->m_position.z < limits.GetMin().z) { force.x = 1.0f; }

        return force;
    }

    bool HasFeature(unsigned int feature) const {
        return (m_agent->m_properties->m_features & feature) != 0;
    }
    void SetFeature(unsigned int feature) { m_agent->m_properties->m_features = feature; }

    const Agent* m_agent;
};

#endif