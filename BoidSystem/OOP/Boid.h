#pragma once

#include <glm/glm.hpp>

#include "SteeringBehavior.h"

#include "Engine/Core/VectorContainer.h"

#include "Definitions.h"
#include "Engine/Utils/MathUtils.h"

struct Boid
{
    Boid()
        : Boid(&m_defaultProperties)
    {

    }

    Boid(Properties* properties)
        : m_id(++ID)
        , m_properties(properties)
    {
        m_velocity = MathUtils::RandomInUnitSphere();
        m_position = MathUtils::RandomInUnitSphere();
        m_targetBoid = nullptr;
        m_fleeBoid = nullptr;
    }

    bool operator==(const Boid& rhs) const { return m_id == rhs.m_id; }
    bool operator!=(const Boid& rhs) const { return m_id != rhs.m_id; }

    void SetTarget(glm::vec3 targetPos) { m_targetPos = targetPos; }
    void SetTarget(Boid* boid) { m_targetBoid = boid; }
    void SetFlee(glm::vec3 fleePos) { m_fleePos = fleePos; }
    void SetFlee(Boid* boid) { m_fleeBoid = boid; }

    void FullUpdate(float deltaTime, std::vector<Boid>& otherBoids, VectorContainer<size_t>& neighborIndices)
    {
        UpdateTargets();

        glm::vec3 force = CalcSteeringBehavior(otherBoids, neighborIndices);
        UpdatePosition(deltaTime, force);
    }

    void UpdateTargets()
    {
        if (m_targetBoid)
        {
            m_targetPos = m_targetBoid->m_position;
        }
        if (m_fleeBoid)
        {
            m_fleePos = m_fleeBoid->m_position;
        }
    }

    void UpdatePosition(float deltaTime, glm::vec3 force)
    {
        const glm::vec3 acceleration = force / m_properties->m_mass;

        m_velocity += acceleration * deltaTime;
        m_velocity = glm::clamp(m_velocity, -m_properties->m_maxSpeed, m_properties->m_maxSpeed);

        if (glm::length(m_velocity) > 0.0001f) 
        {
            m_direction = glm::normalize(m_velocity);
        }

        m_position += m_velocity * deltaTime;
    }

    glm::vec3 CalcSteeringBehavior(std::vector<Boid>& otherBoids, VectorContainer<size_t>& neighborIndices)
    {
        // Steering Bit
        glm::vec3 force = {};
        if (HasFeature(eWallLimits)) { force += m_properties->m_weightWallLimits * WallLimits(); }
        if (HasFeature(eWander)) { force += m_properties->m_weightWander * Wander(); }
        if (HasFeature(eSeek)) { force += m_properties->m_weightSeek * Seek(m_targetPos); }
        if (HasFeature(eArrive)) { force += m_properties->m_weightArrive * Arrive(m_targetPos); }
        if (HasFeature(eFlee)) { force += m_properties->m_weightFlee * Flee(m_fleePos); }
        if (HasFeature(eFleeRanged)) { force += m_properties->m_weightFlee * FleeRanged(m_fleePos); }

#if !USE_OCTREE
        FindNearbyNeighbors(otherBoids);
#endif
        if (HasFeature(eSeparation)) { force += m_properties->m_weightSeparation * Separation(otherBoids, neighborIndices); }
        if (HasFeature(eCohesion)) { force += m_properties->m_weightCohesion * Cohesion(otherBoids, neighborIndices); }
        if (HasFeature(eAlignment)) { force += m_properties->m_weightAlignment * Alignment(otherBoids, neighborIndices); }

        return glm::clamp(force, -m_properties->m_maxForce, m_properties->m_maxForce);
    }

    void DrawDebug()
    {
        // DebugDraw::AddPosition(m_position, 0.1f, { 0.2f, 0.5f, 0.5f, 1.0f });
        DebugDraw::AddLine(m_position, m_position + m_velocity, { 0.75f, 0.0f, 1.0f, 1.0f });
        DebugDraw::AddLine(m_position, m_position + m_direction, { 0.0f, 0.75f, 1.0f, 1.0f });
        // DebugDraw::AddAABB(m_position - glm::vec3(m_properties->m_radius), m_position + glm::vec3(m_properties->m_radius), { 0.2f, 0.5f, 0.5f, 1.0f });
    }

    glm::vec3 Seek(glm::vec3 target)
    {
        glm::vec3 desiredVelocity = glm::normalize(target - m_position) * m_properties->m_maxSpeed;
        return desiredVelocity - m_velocity;
    }

    glm::vec3 Flee(glm::vec3 target)
    {
        glm::vec3 desiredVelocity = glm::normalize(m_position - target) * m_properties->m_maxSpeed;
        return desiredVelocity - m_velocity;
    }

    glm::vec3 FleeRanged(glm::vec3 target)
    {
        glm::vec3 desiredVelocity = glm::normalize(m_position - target);
        float distance = glm::length(desiredVelocity);
        if (distance > 0.0f) 
        {
            desiredVelocity = glm::normalize(desiredVelocity);
        }

        float maxVelocity = m_properties->m_maxSpeed;
        float fleeRadius = 0.8f;
        if (distance >= fleeRadius)
        {
            maxVelocity = 0.0f;
            // maxVelocity = MathUtils::Lerp(0.0f, m_maxSpeed, 1.0f - (distance / fleeRadius));
        }

        desiredVelocity *= maxVelocity;
        return desiredVelocity - m_velocity;
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
        glm::vec3 targetPosJitter = m_position + glm::normalize(m_velocity) * wanderDistance + jitterDirection;

        glm::vec3 desiredVelocity = glm::normalize(targetPosJitter - m_position);
        return desiredVelocity - m_velocity;
    }

    glm::vec3 Arrive(glm::vec3 target)
    {
        glm::vec3 desiredVelocity = target - m_position;
        float distance = glm::length(desiredVelocity);
        desiredVelocity = glm::normalize(desiredVelocity);

        float speed = m_properties->m_maxSpeed;
        float arriveRadius = 4.0f;
        if (distance <= arriveRadius)
        {
            speed = MathUtils::Lerp(0.0f, speed, distance / arriveRadius);
        }

        desiredVelocity *= speed;
        return desiredVelocity - m_velocity;
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
            if (*this == n)
            {
                continue;
            }
#if USE_AABB
            if (!aabb.Contains(n.m_position)) 
            {
                continue;
            }
#else
            glm::vec3 toAgent = m_position - n.m_position;
            float distanceSqToAgent = glm::length2(toAgent);
            if (distanceSqToAgent > m_properties->m_neighborRange * m_properties->m_neighborRange)
            {
                continue;
            }
#endif
            *(m_neighborIndices + m_currentNeighborCount++) = i;
        }
    }

    glm::vec3 Separation(std::vector<Boid>& neighbors, VectorContainer<size_t>& indices)
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
            glm::vec3 toAgent = m_position - neighbors[*(m_neighborIndices+i)].m_position;
#else
            glm::vec3 toAgent = m_position - neighbors[indices.get(i)].m_position;
#endif
            float distanceToAgent = glm::length(toAgent);
            if (distanceToAgent > 0.0f) 
            {
                force += glm::normalize(toAgent) / distanceToAgent;
            }
        }

        return force;
    }

    glm::vec3 Alignment(std::vector<Boid>& neighbors, VectorContainer<size_t>& indices)
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
            avgDirection += neighbors[indices.get(i)].m_direction;
#endif
        }

        if (neighborCount > 0)
        {
            avgDirection /= static_cast<float>(neighborCount);
            avgDirection -= m_direction;
        }

        return avgDirection;
    }

    glm::vec3 Cohesion(std::vector<Boid>& neighbors, VectorContainer<size_t>& indices)
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
            centerOfMass += neighbors[indices.get(i)].m_position;
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
        if (m_position.x > limits.GetMax().x) { force.x = -1; }
        else if (m_position.x < limits.GetMin().x) { force.x = 1.0f; }

        if (m_position.y > limits.GetMax().y) { force.y = -1; }
        else if (m_position.y < limits.GetMin().y) { force.y = 1.0f; }

        if (m_position.z > limits.GetMax().z) { force.x = -1; }
        else if (m_position.z < limits.GetMin().z) { force.x = 1.0f; }

        return force;
    }

    bool HasFeature(unsigned int feature) const {
        return (m_properties->m_features & feature) != 0;
    }
    void SetFeature(unsigned int feature) { m_properties->m_features = feature; }


    int* AllocNeighborIndices(size_t size) 
    {
        return static_cast<int*>(malloc(sizeof(int) * size));
    }
    void DeallocNeighborIndices(int* memoryBlock)
    {
        free(memoryBlock);
    }

    glm::vec3 m_position;
    glm::vec3 m_velocity;
    glm::vec3 m_direction;

    glm::vec3 m_wanderTarget;
    glm::vec3 m_targetPos;
    glm::vec3 m_fleePos;

    Boid* m_targetBoid;
    Boid* m_fleeBoid;

    Properties* m_properties = nullptr;
    Properties m_defaultProperties;

    int* m_neighborIndices = nullptr;
    size_t m_neighborIndicesCount = 0u;
    size_t m_currentNeighborCount = 0u;

    unsigned int m_id = -1;
    static unsigned int ID;
};

unsigned int Boid::ID = 0;



// ENABLE_BITMASK_OPERATORS(Boid::Feature);