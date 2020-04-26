#pragma once

#include "Engine/Renderer/DebugDraw.h"
#include "Engine/Utils/MathUtils.h"

struct Path
{
    Path() = default;
    Path(std::vector<glm::vec3> path)
    {
        m_nodes = path;
        m_currentNodeIndex = static_cast<unsigned int>(MathUtils::Rand(0, m_nodes.size() - 1));
    }

    glm::vec3 GetCurrentGoal() const { return m_nodes[m_currentNodeIndex]; }
    void UpdatePath(glm::vec3 position)
    {
        float distanceToGoal = glm::length(GetCurrentGoal() - position);
        if (distanceToGoal <= atGoalDistanceThreshold)
        {
            m_currentNodeIndex++;
            if (m_currentNodeIndex > m_nodes.size() - 1)
            {
                m_currentNodeIndex = 0;
            }
        }
    }

    void DebugDraw()
    {
        if (m_nodes.empty())
        {
            return;
        }

        for (size_t i = 0; i < m_nodes.size(); i++)
        {
            if (i == m_currentNodeIndex)
            {
                DebugDraw::AddAABB(
                    m_nodes[i] - glm::vec3(atGoalDistanceThreshold),
                    m_nodes[i] + glm::vec3(atGoalDistanceThreshold),
                    { 0.3f, 0.6f, 0.1, 1.0f });
            }

            if (i != m_nodes.size() - 1)
            {
                DebugDraw::AddLine(m_nodes[i], m_nodes[i + 1], { 0.3f, 0.6f, 0.1, 1.0f });
            }
        }
        DebugDraw::AddLine(m_nodes[m_nodes.size() - 1], m_nodes[0], { 0.3f, 0.6f, 0.1, 1.0f });
    }

    size_t m_currentNodeIndex;
    std::vector<glm::vec3> m_nodes;

    float atGoalDistanceThreshold = 2.0f;
};