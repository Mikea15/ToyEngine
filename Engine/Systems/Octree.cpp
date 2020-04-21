#include "Octree.h"

#include "BoundingFrustum.h"

#include "Renderer/DebugDraw.h"

Octree::Octree()
    : Octree(glm::vec3(0.0f), 0.0f)
{
}


Octree::Octree(const glm::vec3& position, float halfSize)
    : m_position(position), m_halfSize(halfSize)
    , m_upFrontLeft(nullptr)
    , m_upFrontRight(nullptr)
    , m_upBackLeft(nullptr)
    , m_upBackRight(nullptr)
    , m_downFrontLeft(nullptr)
    , m_downFrontRight(nullptr)
    , m_downBackLeft(nullptr)
    , m_downBackRight(nullptr)
{
    m_bounds = AABB(m_position, halfSize);
}

Octree::~Octree()
{
    delete m_upFrontLeft;
    delete m_upFrontRight;
    delete m_upBackLeft;
    delete m_upBackRight;
    delete m_downFrontLeft;
    delete m_downFrontRight;
    delete m_downBackLeft;
    delete m_downBackRight;
}

void Octree::Subdivide()
{
    const glm::vec3 ufl = (m_position + (m_position + glm::vec3(-1, 1, 1)   * m_halfSize)) * 0.5f;
    const glm::vec3 ufr = (m_position + (m_position + glm::vec3(1, 1, 1)    * m_halfSize)) * 0.5f;
    const glm::vec3 ubl = (m_position + (m_position + glm::vec3(-1, 1, -1)  * m_halfSize)) * 0.5f;
    const glm::vec3 ubr = (m_position + (m_position + glm::vec3(1, 1, -1)   * m_halfSize)) * 0.5f;
    const glm::vec3 dfl = (m_position + (m_position + glm::vec3(-1, -1, 1)  * m_halfSize)) * 0.5f;
    const glm::vec3 dfr = (m_position + (m_position + glm::vec3(1, -1, 1)   * m_halfSize)) * 0.5f;
    const glm::vec3 dbl = (m_position + (m_position + glm::vec3(-1, -1, -1) * m_halfSize)) * 0.5f;
    const glm::vec3 dbr = (m_position + (m_position + glm::vec3(1, -1, -1)  * m_halfSize)) * 0.5f;

    const float size = m_halfSize * 0.5f;
    m_upFrontLeft = new Octree(ufl, size);
    m_upFrontRight = new Octree(ufr, size);
    m_upBackLeft = new Octree(ubl, size);
    m_upBackRight = new Octree(ubr, size);
    m_downFrontLeft = new Octree(dfl, size);
    m_downFrontRight = new Octree(dfr, size);
    m_downBackLeft = new Octree(dbl, size);
    m_downBackRight = new Octree(dbr, size);
}

bool Octree::Insert(const glm::vec3& position, size_t index)
{
    if (!m_bounds.Contains(position))
    {
        return false;
    }

    if (m_upFrontLeft == nullptr && m_nodes.size() < m_maxNodes)
    {
        m_nodes.push_back({ position, index });
        return true;
    }

    if (m_upFrontLeft == nullptr)
    {
        Subdivide();
    }

    if (m_upFrontLeft->Insert(position, index)) return true;
    if (m_upFrontRight->Insert(position, index)) return true;
    if (m_upBackLeft->Insert(position, index)) return true;
    if (m_upBackRight->Insert(position, index)) return true;
    if (m_downFrontLeft->Insert(position, index)) return true;
    if (m_downFrontRight->Insert(position, index)) return true;
    if (m_downBackLeft->Insert(position, index)) return true;
    if (m_downBackRight->Insert(position, index)) return true;

    return false;
}

void Octree::Search(const AABB& aabb, std::vector<OcNode>& outResult)
{
    if (m_bounds.GetContainmentType(aabb) == ContainmentType::Disjoint)
    {
        return;
    }

    // check objects at this bounds level#
    for (const OcNode& node : m_nodes)
    {
        if (aabb.Contains(node.m_storePos))
        {
            outResult.push_back(node);
        }
    }

    if (m_upFrontLeft == nullptr)
    {
        return;
    }

    m_upFrontLeft->Search(aabb, outResult);
    m_upFrontRight->Search(aabb, outResult);
    m_upBackLeft->Search(aabb, outResult);
    m_upBackRight->Search(aabb, outResult);

    m_downFrontLeft->Search(aabb, outResult);
    m_downFrontRight->Search(aabb, outResult);
    m_downBackLeft->Search(aabb, outResult);
    m_downBackRight->Search(aabb, outResult);
}

void Octree::Search(const BoundingFrustum& frustum, std::vector<OcNode>& outResult)
{
    if (frustum.Contains(m_bounds) == ContainmentType::Disjoint)
    {
        return;
    }

    // check objects at this bounds level#
    // while no BoundingSphere, this BoundingBox is a Generous aproximation of the spheres.
    // if (frustum.Contains(BoundingBox(m_storePos, 1.0f)) != ContainmentType::Disjoint)
    for (const OcNode& node : m_nodes)
    {
        if (frustum.Contains(node.m_storePos) != ContainmentType::Disjoint)
        {
            outResult.push_back(node);
        }
    }

    if (m_upFrontLeft == nullptr)
    {
        return;
    }

    m_upFrontLeft->Search(frustum, outResult);
    m_upFrontRight->Search(frustum, outResult);
    m_upBackLeft->Search(frustum, outResult);
    m_upBackRight->Search(frustum, outResult);

    m_downFrontLeft->Search(frustum, outResult);
    m_downFrontRight->Search(frustum, outResult);
    m_downBackLeft->Search(frustum, outResult);
    m_downBackRight->Search(frustum, outResult);
}

void Octree::GetAllBoundingBoxes(std::vector<AABB>& outResult)
{
    outResult.push_back(m_bounds);

    if (m_upFrontLeft == nullptr) return;

    m_upFrontLeft->GetAllBoundingBoxes(outResult);
    m_upFrontRight->GetAllBoundingBoxes(outResult);
    m_upBackLeft->GetAllBoundingBoxes(outResult);
    m_upBackRight->GetAllBoundingBoxes(outResult);

    m_downFrontLeft->GetAllBoundingBoxes(outResult);
    m_downFrontRight->GetAllBoundingBoxes(outResult);
    m_downBackLeft->GetAllBoundingBoxes(outResult);
    m_downBackRight->GetAllBoundingBoxes(outResult);
}

void Octree::DebugDraw()
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
