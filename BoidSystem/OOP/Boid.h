#pragma once

#include <glm/glm.hpp>
#include <glm/ext/vector_float3.hpp>

#include "Engine/Core/VectorContainer.h"

#include "Definitions.h"
#include "Engine/Utils/MathUtils.h"

struct Boid
{
    Boid()
        : m_properties(&DefaultProperties)
    {
    }

    Boid(Properties* properties)
    {
        m_properties = properties;

        m_velocity = MathUtils::RandomInUnitSphere();
        m_position = MathUtils::RandomInUnitSphere();
        m_targetBoid = nullptr;
        m_fleeBoid = nullptr;

        m_neighborIndices.resize(ENTITY_COUNT);
    }

    Boid(const Boid& other)
        : m_id(other.m_id)
        , m_properties(other.m_properties)
    {
        m_velocity = other.m_velocity;
        m_position = other.m_position;
        m_direction = other.m_direction;

        m_targetBoid = other.m_targetBoid;
        m_fleeBoid = other.m_fleeBoid;

        m_neighborIndices.resize(ENTITY_COUNT);
    }

    bool operator==(const Boid& rhs) const { return m_id == rhs.m_id; }
    bool operator!=(const Boid& rhs) const { return m_id != rhs.m_id; }

    void SetTarget(glm::vec3 targetPos) { m_targetPos = targetPos; }
    void SetTarget(Boid* boid) { m_targetBoid = boid; }
    void SetFlee(glm::vec3 fleePos) { m_fleePos = fleePos; }
    void SetFlee(Boid* boid) { m_fleeBoid = boid; }

    void FullUpdate(float deltaTime, std::vector<Boid>& otherBoids, std::vector<size_t>& neighborIndices)
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

    glm::vec3 CalcSteeringBehavior(std::vector<Boid>& otherBoids, std::vector<size_t>& neighborIndices)
    {
        // Steering Bit
        glm::vec3 force = {};
        if (HasFeature(eWallLimits)) { force += m_properties->m_weightWallLimits * WallLimits(); }
        if (HasFeature(eWander)) { force += m_properties->m_weightWander * Wander(); }
        if (HasFeature(eSeek)) { force += m_properties->m_weightSeek * Seek(m_targetPos); }
        if (HasFeature(eArrive)) { force += m_properties->m_weightArrive * Arrive(m_targetPos); }
        if (HasFeature(eFlee)) { force += m_properties->m_weightFlee * Flee(m_fleePos); }
        if (HasFeature(eFleeRanged)) { force += m_properties->m_weightFlee * FleeRanged(m_fleePos); }

#if USE_OCTREE
        m_currentNeighborCount = neighborIndices.size();
#else
        Search(this, otherBoids, m_neighborIndices, m_currentNeighborCount);
#if MULTITHREAD
        std::vector<size_t> neiIndices = m_neighborIndices;
#else
        // NOTE (MA): This is a reference that comes from Main, and is shared accross threads
        // and is not protected against writes.
        neighborIndices = m_neighborIndices;
#endif
#endif
        if (HasFeature(eSeparation)) { force += m_properties->m_weightSeparation * Separation(otherBoids, 
#if MULTITHREAD
            neiIndices
#else
            neighborIndices
#endif
        ); }
        if (HasFeature(eCohesion)) { force += m_properties->m_weightCohesion * Cohesion(otherBoids, 
#if MULTITHREAD
            neiIndices
#else
            neighborIndices
#endif
        ); }
        if (HasFeature(eAlignment)) { force += m_properties->m_weightAlignment * Alignment(otherBoids, 
#if MULTITHREAD
            neiIndices
#else
            neighborIndices
#endif
        ); }

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

    glm::vec3 Separation(std::vector<Boid>& neighbors, std::vector<size_t>& neighborIndices)
    {
        glm::vec3 force = {};

        size_t neighborCount = m_currentNeighborCount;
        for (size_t i = 0; i < neighborCount; ++i)
        {
            auto& n = neighbors[neighborIndices[i]];
#if NEW_OCTREE // TODO: Avoid returning self index to prevent this check
            if (n == *this) {
                continue;
            }
#endif
            assert(m_position != n.m_position);
            glm::vec3 toAgent = m_position - n.m_position;
            float distanceToAgent = glm::length(toAgent);
            if (distanceToAgent > 0.0f)
            {
                force += glm::normalize(toAgent) / distanceToAgent;
            }
        }

        return force;
    }

    glm::vec3 Alignment(std::vector<Boid>& neighbors, std::vector<size_t>& neighborIndices)
    {
        glm::vec3 force = {};

        size_t neighborCount = m_currentNeighborCount;
        for (size_t i = 0; i < neighborCount; ++i)
        {
            force += neighbors[neighborIndices[i]].m_direction;
        }

        if (neighborCount > 0)
        {
            force /= static_cast<float>(neighborCount);
            force -= m_direction;
        }

        return force;
    }

    glm::vec3 Cohesion(std::vector<Boid>& neighbors, std::vector<size_t>& neighborIndices)
    {
        glm::vec3 centerOfMass = {};
        glm::vec3 force = {};

        size_t neighborCount = m_currentNeighborCount;
        for (size_t i = 0; i < neighborCount; ++i)
        {
            centerOfMass += neighbors[neighborIndices[i]].m_position;
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

        if (m_position.z > limits.GetMax().z) { force.z = -1; }
        else if (m_position.z < limits.GetMin().z) { force.z = 1.0f; }

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

    void Search(Boid* agent, std::vector<Boid>& neighbors, std::vector<size_t>& result, size_t& outResultCount, size_t maxNeighbors = 0)
    {
#if USE_AABB
        AABB aabb = AABB(agent->m_position, agent->m_properties->m_neighborRange);
#endif

        outResultCount = 0;
        size_t neighborSize = neighbors.size();
        for (size_t i = 0; i < neighborSize; ++i)
        {
            auto& n = neighbors[i];
            if (*agent == n) { continue; }

#if USE_AABB
            if (!aabb.Contains(n.m_position)) { continue; }
#endif
            glm::vec3 toAgent = agent->m_position - n.m_position;
            float distanceSqToAgent = glm::length2(toAgent);
            if (distanceSqToAgent < FLT_EPSILON || distanceSqToAgent > agent->m_properties->m_neighborRange * agent->m_properties->m_neighborRange)
            {
                continue;
            }

            if (maxNeighbors > 0 && outResultCount >= maxNeighbors)
            {
                break;
            }

            assert(result.size() > outResultCount);
            result[outResultCount++] = i;
        }
    }

    unsigned int m_id = -1;
    glm::vec3 m_position;
    glm::vec3 m_velocity;
    glm::vec3 m_direction;

    glm::vec3 m_wanderTarget;
    glm::vec3 m_targetPos;
    glm::vec3 m_fleePos;

    Boid* m_targetBoid;
    Boid* m_fleeBoid;

    Properties* m_properties = nullptr;



    // Have an array of boids. This will create lots of copies for now.
    // we'll deal with those optimizations later on.
    std::vector<Boid> m_neighborsScratch;

    std::vector<size_t> m_neighborIndices;
    size_t m_currentNeighborCount = 0u;

    static unsigned int ID;
    static Properties DefaultProperties;
};

unsigned int Boid::ID = 0;
Properties Boid::DefaultProperties{};
