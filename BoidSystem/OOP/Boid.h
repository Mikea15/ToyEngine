#pragma once

#include <glm/glm.hpp>

#include "BoidManager.h"

struct Boid
{
    enum Feature : unsigned int
    {
        eNone       = 0,
        eWander     = 1 << 0,
        eSeek       = 1 << 1,
        eFlee       = 1 << 2,
        eFleeRanged = 1 << 3,
        eArrive     = 1 << 4,

        eSeparation = 1 << 5,
        eCohesion   = 1 << 6,
        eAlignment  = 1 << 7,

        eWallLimits = 1 << 8
    };

    struct Properties
    {
        unsigned int m_features     = eWander;

        float m_maxSpeed            = 5.0f;     // [m/s]
        float m_maxForce            = 10.0f;
        float m_mass                = 2.0f;
        float m_radius              = 1.0f;

        float m_neighborRange       = 3.0f;

        float m_weightWallLimits    = 1.0f;
        float m_weightWander        = 1.0f;
        float m_weightSeek          = 1.0f;
        float m_weightFlee          = 1.0f;
        float m_weightArrive        = 1.0f;

        float m_weightAlignment     = 1.2f;
        float m_weightCohesion      = 1.8f;
        float m_weightSeparation    = 0.6f;
    };

    Boid()
        : m_id(++ID)
        , m_properties(&m_defaultProperties)
    {
        m_velocity = MathUtils::RandomInUnitSphere();
        m_position = MathUtils::RandomInUnitSphere();
        m_targetBoid = nullptr;
        m_fleeBoid = nullptr;
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

    void Update(float deltaTime, std::vector<Boid>& otherBoids)
    {
        if (m_targetBoid)
        {
            m_targetPos = m_targetBoid->m_position;
        }
        if (m_fleeBoid)
        {
            m_fleePos = m_fleeBoid->m_position;
        }

        // Steering Bit
        glm::vec3 force = {};
        if (HasFeature(eWallLimits))    { force += m_properties->m_weightWallLimits * WallLimits(); }
        if (HasFeature(eWander))        { force += m_properties->m_weightWander * Wander(); }
        if (HasFeature(eSeek))          { force += m_properties->m_weightSeek * Seek(m_targetPos); }
        if (HasFeature(eArrive))        { force += m_properties->m_weightArrive * Arrive(m_targetPos); }
        if (HasFeature(eFlee))          { force += m_properties->m_weightFlee * Flee(m_fleePos); }
        if (HasFeature(eFleeRanged))    { force += m_properties->m_weightFlee * FleeRanged(m_fleePos); }

        std::vector<Boid> neighborSubset = FindNearbyNeighbors(otherBoids);
        if (HasFeature(eSeparation))    { force += m_properties->m_weightSeparation * Separation(neighborSubset); }
        if (HasFeature(eCohesion))      { force += m_properties->m_weightCohesion * Cohesion(neighborSubset); }
        if (HasFeature(eAlignment))     { force += m_properties->m_weightAlignment * Alignment(neighborSubset); }

        force = glm::clamp(force, -m_properties->m_maxForce, m_properties->m_maxForce);

        // Vehicle Bit
        glm::vec3 acceleration = force / m_properties->m_mass;

        m_velocity += acceleration * deltaTime;
        m_velocity = glm::clamp(m_velocity, -m_properties->m_maxSpeed, m_properties->m_maxSpeed);

        m_position += m_velocity * deltaTime;

        if (glm::length(m_velocity) > 0.0001f) {
            m_direction = glm::normalize(m_velocity);
        }
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

    std::vector<Boid> FindNearbyNeighbors(std::vector<Boid>& neighbors)
    {
        std::vector<Boid> result = {};

        for (size_t i = 0; i < neighbors.size(); ++i)
        {
            if (*this == neighbors[i])
            {
                continue;
            }

            glm::vec3 toAgent = m_position - neighbors[i].m_position;
            float distanceToAgent = glm::length(toAgent);
            if (distanceToAgent > m_properties->m_neighborRange)
            {
                continue;
            }

            result.push_back(neighbors[i]);
        }

        return result;
    }

    glm::vec3 Separation(std::vector<Boid>& neighbors)
    {
        glm::vec3 force = {};
        size_t neighborCount = neighbors.size();
        for (size_t i = 0; i < neighborCount; ++i)
        {
            glm::vec3 toAgent = m_position - neighbors[i].m_position;
            float distanceToAgent = glm::length(toAgent);
            if (distanceToAgent > 0.0f) 
            {
                force += glm::normalize(toAgent) / distanceToAgent;
            }
        }

        return force;
    }

    glm::vec3 Alignment(std::vector<Boid>& neighbors)
    {
        glm::vec3 avgDirection = {};
        size_t neighborCount = neighbors.size();
        for (size_t i = 0; i < neighborCount; ++i)
        {
            avgDirection += neighbors[i].m_direction;
        }

        if (neighborCount > 0)
        {
            avgDirection /= static_cast<float>(neighborCount);
            avgDirection -= m_direction;
        }

        return avgDirection;
    }

    glm::vec3 Cohesion(std::vector<Boid>& neighbors)
    {
        glm::vec3 centerOfMass = {};
        glm::vec3 force = {};
        size_t neighborCount = neighbors.size();

        for (size_t i = 0; i < neighborCount; ++i)
        {
            centerOfMass += neighbors[i].m_position;
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

    unsigned int m_id = -1;
    static unsigned int ID;
};

unsigned int Boid::ID = 0;

namespace Debug
{
    void ShowPanel(Boid::Properties& properties)
    {
        ImGui::Begin("Boid::Properties");

        ImGui::SliderFloat("Max Speed", &properties.m_maxSpeed, 0.0f, 50.0f);
        ImGui::SliderFloat("Max Force", &properties.m_maxForce, 0.0f, 50.0f);
        ImGui::SliderFloat("Vehicle Mass", &properties.m_mass, 0.1f, 10.0f);

        ImGui::Text("Independent Behavior");
        ImGui::SliderFloat("Wander", &properties.m_weightWander, 0.0f, 5.0f);
        ImGui::SliderFloat("Seek", &properties.m_weightSeek, 0.0f, 5.0f);
        ImGui::SliderFloat("Flee", &properties.m_weightFlee, 0.0f, 5.0f);
        ImGui::SliderFloat("Arrive", &properties.m_weightArrive, 0.0f, 5.0f);

        ImGui::Separator();
        ImGui::Text("Group Behavior");
        ImGui::SliderFloat("Alignment", &properties.m_weightAlignment, 0.0f, 5.0f);
        ImGui::SliderFloat("Cohesion", &properties.m_weightCohesion, 0.0f, 5.0f);
        ImGui::SliderFloat("Separation", &properties.m_weightSeparation, 0.0f, 5.0f);

        ImGui::Separator();
        ImGui::SliderFloat("Wall Limits", &properties.m_weightWallLimits, 0.0f, 5.0f);

        ImGui::End();
    }
}

// ENABLE_BITMASK_OPERATORS(Boid::Feature);