#include "Octree.h"

#include "BoundingFrustum.h"

#include "Renderer/DebugDraw.h"

Octree::Octree()
    : Octree(glm::vec3(0.0f), 0.0f)
{
}

Octree::Octree(const glm::vec3& position, float halfSize)
    : m_bounds(position, halfSize)
    , m_ufl(nullptr)
    , m_ufr(nullptr)
    , m_ubl(nullptr)
    , m_ubr(nullptr)
    , m_dfl(nullptr)
    , m_dfr(nullptr)
    , m_dbl(nullptr)
    , m_dbr(nullptr)
{ }

Octree::~Octree()
{
    delete m_ufl;
    delete m_ufr;
    delete m_ubl;
    delete m_ubr;
    delete m_dfl;
    delete m_dfr;
    delete m_dbl;
    delete m_dbr;
}

void Octree::Subdivide()
{
    auto center = m_bounds.GetPosition();
    auto halfSize = m_bounds.GetHalfSize() * 0.5f;

    const glm::vec3 ufl = center + glm::vec3(-1, 1, 1)   * halfSize;
    const glm::vec3 ufr = center + glm::vec3(1, 1, 1)    * halfSize;
    const glm::vec3 ubl = center + glm::vec3(-1, 1, -1)  * halfSize;
    const glm::vec3 ubr = center + glm::vec3(1, 1, -1)   * halfSize;

    const glm::vec3 dfl = center + glm::vec3(-1, -1, 1)  * halfSize;
    const glm::vec3 dfr = center + glm::vec3(1, -1, 1)   * halfSize;
    const glm::vec3 dbl = center + glm::vec3(-1, -1, -1) * halfSize;
    const glm::vec3 dbr = center + glm::vec3(1, -1, -1)  * halfSize;

    m_ufl = new Octree(ufl, halfSize);
    m_ufr = new Octree(ufr, halfSize);
    m_ubl = new Octree(ubl, halfSize);
    m_ubr = new Octree(ubr, halfSize);
    m_dfl = new Octree(dfl, halfSize);
    m_dfr = new Octree(dfr, halfSize);
    m_dbl = new Octree(dbl, halfSize);
    m_dbr = new Octree(dbr, halfSize);
}

bool Octree::Insert(const glm::vec3& position, size_t index)
{
    if (!m_bounds.Contains(position)) { return false; }

    if (m_ufl == nullptr && m_nodes.size() < m_maxNodes)
    {
        m_nodes.push_back({ position, index });
        return true;
    }

    if (m_ufl == nullptr) { Subdivide(); }

    if (m_ufl->Insert(position, index)) return true;
    if (m_ufr->Insert(position, index)) return true;
    if (m_ubl->Insert(position, index)) return true;
    if (m_ubr->Insert(position, index)) return true;
    if (m_dfl->Insert(position, index)) return true;
    if (m_dfr->Insert(position, index)) return true;
    if (m_dbl->Insert(position, index)) return true;
    if (m_dbr->Insert(position, index)) return true;

    return false;
}

void Octree::Search(const AABB& aabb, std::vector<OcNode>& outResult)
{
    if (!m_bounds.Contains(aabb)) { return; }

    for (const OcNode& node : m_nodes)
    {
        if (aabb.Contains(node.m_storePos))
        {
            outResult.push_back(node);
        }
    }

    if (m_ufl == nullptr) { return; }

    m_ufl->Search(aabb, outResult);
    m_ufr->Search(aabb, outResult);
    m_ubl->Search(aabb, outResult);
    m_ubr->Search(aabb, outResult);

    m_dfl->Search(aabb, outResult);
    m_dfr->Search(aabb, outResult);
    m_dbl->Search(aabb, outResult);
    m_dbr->Search(aabb, outResult);
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

    if (m_ufl == nullptr)
    {
        return;
    }

    m_ufl->Search(frustum, outResult);
    m_ufr->Search(frustum, outResult);
    m_ubl->Search(frustum, outResult);
    m_ubr->Search(frustum, outResult);

    m_dfl->Search(frustum, outResult);
    m_dfr->Search(frustum, outResult);
    m_dbl->Search(frustum, outResult);
    m_dbr->Search(frustum, outResult);
}

void Octree::GetAllBoundingBoxes(std::vector<AABB>& outResult)
{
    if( !m_nodes.empty() ) {
        outResult.push_back(m_bounds);
    }

    if (m_ufl == nullptr) return;

    m_ufl->GetAllBoundingBoxes(outResult);
    m_ufr->GetAllBoundingBoxes(outResult);
    m_ubl->GetAllBoundingBoxes(outResult);
    m_ubr->GetAllBoundingBoxes(outResult);

    m_dfl->GetAllBoundingBoxes(outResult);
    m_dfr->GetAllBoundingBoxes(outResult);
    m_dbl->GetAllBoundingBoxes(outResult);
    m_dbr->GetAllBoundingBoxes(outResult);
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
