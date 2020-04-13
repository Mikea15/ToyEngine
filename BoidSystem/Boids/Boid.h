#pragma once

#include <glm/glm.hpp>
#include "BoidManager.h"

template<typename E>
struct EnableBitMaskOperators
{
    static const bool enable = false;
};

template<typename E>
typename std::enable_if<EnableBitMaskOperators<E>::enable, E>::type
operator |(E lhs, E rhs)
{
    using underlyingType = typename std::underlying_type<E>::type;
    return static_cast<E>(
        static_cast<underlyingType>(lhs) |
        static_cast<underlyingType>(lhs)
        );
}

#define ENABLE_BITMASK_OPERATORS(x)  \
template<>                           \
struct EnableBitMaskOperators<x>     \
{                                    \
	static const bool enable = true; \
};

struct Boid
{
    struct WeightParams
    {
        float m_wander      = 1.0f;
        float m_seek        = 1.0f;
        float m_flee        = 1.0f;
        float m_arrive      = 1.0f;

        float m_alignment   = 1.2f;
        float m_cohesion    = 1.8f;
        float m_separation  = 0.6f;
    };

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
        eAlignment  = 1 << 7
    };

    Boid()
        : m_id(++ID)
    {
        m_velocity = {};
        m_position = {};

        m_radius = 1.0f;
        m_maxSpeed = 150.0f;
        m_maxSteeringForce = 5.0f;

        m_features = eNone;

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

        glm::vec3 force = {};
        if (HasFeature(eWander))        { force += m_params.m_wander * Wander(); }
        if (HasFeature(eSeek))          { force += m_params.m_seek * Seek(); }
        if (HasFeature(eFlee))          { force += m_params.m_flee * Flee(); }
        if (HasFeature(eFleeRanged))    { force += m_params.m_flee * FleeRanged(); }
        if (HasFeature(eArrive))        { force += m_params.m_arrive * Arrive(); }

        if (HasFeature(eSeparation))    { force += m_params.m_separation * Separation(otherBoids); }
        if (HasFeature(eCohesion))      { force += m_params.m_cohesion * Cohesion(otherBoids); }
        if (HasFeature(eAlignment))     { force += m_params.m_alignment * Alignment(otherBoids); }

        force += glm::clamp(force, -m_maxSteeringForce, m_maxSteeringForce);

        glm::vec3 acceleration = force / m_mass;

        m_velocity += acceleration * deltaTime;
        m_velocity = glm::clamp(m_velocity, -m_maxSpeed, m_maxSpeed);

        m_position += m_velocity * deltaTime;
    }

    void DrawDebug()
    {
        DebugDraw::AddPosition(m_position, 0.1f, { 0.2f, 0.5f, 0.5f, 1.0f });
        DebugDraw::AddLine(m_position, m_position + m_velocity * 0.5f, { 0.75f, 0.75f, 0.75f, 1.0f });
        DebugDraw::AddAABB(m_position - glm::vec3(m_radius), m_position + glm::vec3(m_radius), { 0.2f, 0.5f, 0.5f, 1.0f });
    }

    glm::vec3 Seek()
    {
        glm::vec3 desiredVelocity = glm::normalize(m_targetPos - m_position) * m_maxSpeed;
        return desiredVelocity - m_velocity;
    }

    glm::vec3 Flee()
    {
        glm::vec3 desiredVelocity = glm::normalize(m_position - m_fleePos) * m_maxSpeed;
        return desiredVelocity - m_velocity;
    }

    glm::vec3 FleeRanged()
    {
        glm::vec3 desiredVelocity = glm::normalize(m_position - m_fleePos);
        float distance = glm::length(desiredVelocity);
        desiredVelocity = glm::normalize(desiredVelocity);

        float maxVelocity = m_maxSpeed;
        float fleeRadius = 0.8f;
        if (distance >= fleeRadius)
        {
            maxVelocity = 0.0f;
            // maxVelocity = MathUtils::Lerp(0.0f, m_maxSpeed, 1.0f - (distance / fleeRadius));
        }

        desiredVelocity *= maxVelocity;
        return desiredVelocity - m_velocity;
    }

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
        glm::vec3 targetPosJitter = m_position + GetHeading() * wanderDistance + jitterDirection;

        glm::vec3 desiredVelocity = glm::normalize(targetPosJitter - m_position);
        return desiredVelocity - m_velocity;
    }

    glm::vec3 Arrive()
    {
        glm::vec3 desiredVelocity = m_targetPos - m_position;
        float distance = glm::length(desiredVelocity);
        desiredVelocity = glm::normalize(desiredVelocity);

        float speed = m_maxSpeed;
        float arriveRadius = 4.0f;
        if (distance <= arriveRadius)
        {
            speed = MathUtils::Lerp(0.0f, speed, distance / arriveRadius);
        }

        desiredVelocity *= speed;
        return desiredVelocity - m_velocity;
    }

    glm::vec3 Separation(std::vector<Boid>& neighbors)
    {
        glm::vec3 force = {};
        for (size_t i = 0; i < neighbors.size(); i++)
        {
            if (*this == neighbors[i] && glm::length(m_position - neighbors[i].m_position) > m_neighborRange)
            {
                continue;
            }

            if (m_position != neighbors[i].m_position) 
            {
                glm::vec3 toAgent = m_position - neighbors[i].m_position;
                force += glm::normalize(toAgent) / glm::length(toAgent);
            }
        }

        return force;
    }

    glm::vec3 Alignment(std::vector<Boid>& neighbors)
    {
        glm::vec3 avgHeading = {};
        size_t neighborCount = {};

        for (size_t i = 0; i < neighbors.size(); i++)
        {
            if (*this == neighbors[i] && glm::length(m_position - neighbors[i].m_position) > m_neighborRange)
            {
                continue;
            }

            avgHeading += neighbors[i].GetHeading();
            ++neighborCount;
        }

        if (neighborCount > 0)
        {
            avgHeading /= neighborCount;
            avgHeading -= GetHeading();
        }

        return avgHeading;
    }

    glm::vec3 Cohesion(std::vector<Boid>& neighbors)
    {
        glm::vec3 centerOfMass = {};
        glm::vec3 avgForce = {};
        size_t neighborCount = {};

        for (size_t i = 0; i < neighbors.size(); i++)
        {
            if (*this == neighbors[i] && glm::length(m_position - neighbors[i].m_position) > m_neighborRange)
            {
                continue;
            }

            centerOfMass += neighbors[i].m_position;
            ++neighborCount;
        }

        if (neighborCount > 0)
        {
            centerOfMass /= neighborCount;

            // Seek Behavior
            glm::vec3 desired = glm::normalize(centerOfMass - m_position) * m_maxSpeed;
            glm::vec3 steer = desired - m_velocity;

            avgForce = glm::normalize(steer);
        }

        return avgForce;
    }

    glm::vec3 GetHeading() const {
        if (m_velocity != glm::vec3(0.0f))
        {
            return glm::normalize(m_velocity);
        }
        return glm::vec3(0.0f, 0.0f, -1.0f);
    }

    bool HasFeature(unsigned int feature) const {
        return (m_features & feature) != 0;
    }
    void SetFeature(unsigned int feature) { m_features = feature; }

    void DrawDebugUI()
    {
        ImGui::Begin("Boid Settings");

        ImGui::SliderFloat("Max Speed", &m_maxSpeed, 0.0f, 50.0f);
        ImGui::SliderFloat("Max Steering Force", &m_maxSteeringForce, 0.0f, 150.0f);
        ImGui::SliderFloat("Vehicle Mass", &m_mass, 0.1f, 10.0f);

        ImGui::SliderFloat("Params Wander", &m_params.m_wander, 0.0f, 5.0f);
        ImGui::SliderFloat("Params Seek", &m_params.m_seek, 0.0f, 5.0f);
        ImGui::SliderFloat("Params Flee", &m_params.m_flee, 0.0f, 5.0f);
        ImGui::SliderFloat("Params Arrive", &m_params.m_arrive, 0.0f, 5.0f);
        ImGui::SliderFloat("Params Alignment", &m_params.m_alignment , 0.0f, 5.0f);
        ImGui::SliderFloat("Params Cohesion", &m_params.m_cohesion , 0.0f, 5.0f);
        ImGui::SliderFloat("Params Separation", &m_params.m_separation , 0.0f, 5.0f);

        ImGui::End();
    }

    glm::vec3 m_position;
    glm::vec3 m_velocity;

    glm::vec3 m_wanderTarget;
    glm::vec3 m_targetPos;
    glm::vec3 m_fleePos;

    Boid* m_targetBoid;
    Boid* m_fleeBoid;

    WeightParams m_params;
    unsigned int m_features;

    float m_maxSpeed;
    float m_maxSteeringForce;
    float m_mass = 2.0f;
    float m_radius;

    float m_neighborRange = 3.0f;

    unsigned int m_id = -1;
    static unsigned int ID;
};

unsigned int Boid::ID = 0;

// ENABLE_BITMASK_OPERATORS(Boid::Feature);