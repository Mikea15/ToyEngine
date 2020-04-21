#pragma once

#include <glm/glm.hpp>

#include "Composition/SteeringBehaviourComposed.h"

#include "Engine/Core/VectorContainer.h"

#include "Definitions.h"

#include "Engine/Renderer/DebugDraw.h"

struct AgentComposition
{
    AgentComposition()
        : AgentComposition(&m_defaultProperties)
    {

    }

    AgentComposition(Properties* properties)
        : m_id(++ID)
        , m_properties(properties)
    {
        m_velocity = MathUtils::RandomInUnitSphere();
        m_position = MathUtils::RandomInUnitSphere();
        m_targetBoid = nullptr;
        m_fleeBoid = nullptr;
    }

    bool operator==(const AgentComposition& rhs) const { return m_id == rhs.m_id; }
    bool operator!=(const AgentComposition& rhs) const { return m_id != rhs.m_id; }

    void SetTarget(glm::vec3 targetPos) { m_targetPos = targetPos; }
    void SetTarget(AgentComposition* boid) { m_targetBoid = boid; }
    void SetFlee(glm::vec3 fleePos) { m_fleePos = fleePos; }
    void SetFlee(AgentComposition* boid) { m_fleeBoid = boid; }

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

    void Update(float deltaTime)
    {
        UpdateTargets();
        const glm::vec3 force = steeringBehaviour.CalculateWeighedSum();

        UpdatePosition(deltaTime, force);
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

    glm::vec3 CalcSteeringBehavior()
    {
        return steeringBehaviour.CalculateWeighedSum();
    }

    void DrawDebug()
    {
        // DebugDraw::AddPosition(m_position, 0.1f, { 0.2f, 0.5f, 0.5f, 1.0f });
        DebugDraw::AddLine(m_position, m_position + m_velocity, { 0.75f, 0.0f, 1.0f, 1.0f });
        DebugDraw::AddLine(m_position, m_position + m_direction, { 0.0f, 0.75f, 1.0f, 1.0f });
        // DebugDraw::AddAABB(m_position - glm::vec3(m_properties->m_radius), m_position + glm::vec3(m_properties->m_radius), { 0.2f, 0.5f, 0.5f, 1.0f });
    }

    bool HasFeature(unsigned int feature) const { return (m_properties->m_features & feature) != 0; }
    void SetFeature(unsigned int feature) { m_properties->m_features = feature; }

    glm::vec3 m_position;
    glm::vec3 m_velocity;
    glm::vec3 m_direction;

    glm::vec3 m_wanderTarget;
    glm::vec3 m_targetPos;
    glm::vec3 m_fleePos;

    AgentComposition* m_targetBoid;
    AgentComposition* m_fleeBoid;

    Properties* m_properties = nullptr;
    Properties m_defaultProperties;

    SteeringBehaviourComposed steeringBehaviour;

    int* m_neighborIndices = nullptr;
    size_t m_neighborIndicesCount = 0u;
    size_t m_currentNeighborCount = 0u;

    unsigned int m_id = -1;
    static unsigned int ID;
};

unsigned int AgentComposition::ID = 0;