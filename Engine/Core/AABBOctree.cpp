
#include "AABBOctree.h"

#include "Systems/BoundingFrustum.h"
#include "Renderer/DebugDraw.h"

#include <algorithm>
#include <glm/gtx/norm.hpp>

AABBOctree::AABBOctree()
    : AABBOctree(glm::vec3(0.0f), 0.0f)
{
}

AABBOctree::AABBOctree(const glm::vec3& position, float halfSize)
    : m_bounds(position, halfSize)
{ 
    for (size_t i = 0; i < 8; ++i)
    {
        m_child[i] = nullptr;
    }
}

AABBOctree::~AABBOctree()
{
    m_nodes.clear();

    for (size_t i = 0; i < 8; ++i)
    {
        delete m_child[i];
    }
}

bool AABBOctree::Insert(const glm::vec3& position, size_t index)
{
    if (!m_bounds.Contains(position)) { return false; }

    if (m_child[0] == nullptr && m_nodes.size() < m_maxNodes)
    {
        m_nodes.push_back({ position, index });
        return true;
    }

    if (m_child[0] == nullptr) { Subdivide(); }

    for (size_t i = 0; i < 8; ++i)
    {
        if (m_child[i]->Insert(position, index)) 
        {
            return true;
        }
    }

    return false;
}

void AABBOctree::FindNeighbors(const glm::vec3& pos, float radius, std::vector<OcNode>& outResult)
{
    outResult.clear();
    const float radiusSq = radius * radius;
    InternalFindNeighbors(pos, radius, radiusSq, outResult);
}

void AABBOctree::Search(const AABB& aabb, std::vector<OcNode>& outResult)
{
    outResult.clear();
    InternalSearch(aabb, outResult);
}

void AABBOctree::Search(const BoundingFrustum& frustum, std::vector<OcNode>& outResult)
{
    outResult.clear();
    InternalSearch(frustum, outResult);
}

void AABBOctree::GetAllBoundingBoxes(std::vector<AABB>& outResult)
{
    if (!m_nodes.empty()) {
        outResult.push_back(m_bounds);
    }

    if (m_child[0] == nullptr) return;
    for (size_t i = 0; i < 8; ++i)
    {
        m_child[i]->GetAllBoundingBoxes(outResult);
    }
}

void AABBOctree::DebugDraw()
{
    // draw octree
    std::vector<AABB> octreeVis;
    GetAllBoundingBoxes(octreeVis);
    const size_t oSize = octreeVis.size();
    for (size_t i = 0; i < oSize; ++i)
    {
        auto min = octreeVis[i].GetMin();
        auto max = octreeVis[i].GetMax();
        DebugDraw::AddAABB(min, max, { 1.0f, 1.0f, 0.0f, 0.30f });
    }
}

void AABBOctree::Subdivide()
{
    static const glm::vec3 positions[] = {
        glm::vec3(1, 1, 1),
        glm::vec3(1, -1, 1),
        glm::vec3(-1, 1, 1),
        glm::vec3(-1, -1, 1),
        glm::vec3(1, 1, -1),
        glm::vec3(1, -1, -1),
        glm::vec3(-1, 1, -1),
        glm::vec3(-1, -1, -1)
    };

    const auto center = m_bounds.GetPosition();
    const auto halfSize = m_bounds.GetHalfSize() * 0.5f;

    for (size_t i = 0; i < 8; ++i)
    {
        const glm::vec3 pos = center + positions[i] * halfSize;
        m_child[i] = new AABBOctree(pos, halfSize);
    }
}

void AABBOctree::InternalSearch(const AABB& aabb, std::vector<OcNode>& outResult)
{
    if (!m_bounds.Contains(aabb)) { return; }

    for (const OcNode& node : m_nodes)
    {
        if (aabb.Contains(node.m_pos))
        {
            outResult.push_back(node);
        }
    }

    if (m_child[0] == nullptr) return;
    for (size_t i = 0; i < 8; ++i)
    {
        m_child[i]->InternalSearch(aabb, outResult);
    }
}

void AABBOctree::InternalSearch(const BoundingFrustum& frustum, std::vector<OcNode>& outResult)
{
    if (frustum.Contains(m_bounds) == ContainmentType::Disjoint) { return; }

    for (const OcNode& node : m_nodes)
    {
        if (frustum.Contains(node.m_pos) != ContainmentType::Disjoint)
        {
            outResult.push_back(node);
        }
    }

    if (m_child[0] == nullptr) return;
    for (size_t i = 0; i < 8; ++i)
    {
        m_child[i]->InternalSearch(frustum, outResult);
    }
}

void AABBOctree::InternalFindNeighbors(const glm::vec3& pos, float radius, float radiusSq, std::vector<OcNode>& outResult)
{
    if (!m_bounds.Contains(pos, radius)) { return; }
    for (const OcNode& node : m_nodes)
    {
        if (glm::length2(pos - node.m_pos) <= radiusSq)
        {
            outResult.push_back(node);
        }
    }

    if (m_child[0] == nullptr) return;
    for (size_t i = 0; i < 8; ++i)
    {
        m_child[i]->InternalFindNeighbors(pos, radius, radiusSq, outResult);
    }
}
