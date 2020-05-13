#include "Octree.h"

#include "BoundingFrustum.h"

#include "Renderer/DebugDraw.h"

#include <algorithm>
#include <glm/gtx/norm.hpp>

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

void Octree::Clear()
{
    delete m_root;
    m_root = nullptr;
    m_data = nullptr;
}

void Octree::Initialize(const std::vector<glm::vec3>& points)
{
    assert(!points.empty());

    Clear();

    const size_t n = points.size();
    m_data = &points;

    m_edges = std::vector<size_t>(n);

    glm::vec3 min = points[0];
    glm::vec3 max = points[0];

    for (size_t i = 0; i < n; i++)
    {
        // always point to the next.
        m_edges[i] = i + 1;

        if (points[i].x < min.x) min.x = points[i].x;
        if (points[i].y < min.y) min.y = points[i].y;
        if (points[i].z < min.z) min.z = points[i].z;
        if (points[i].x > max.x) max.x = points[i].x;
        if (points[i].y > max.y) max.y = points[i].y;
        if (points[i].z > max.z) max.z = points[i].z;
    }

    glm::vec3 extent = (max - min) * 0.5f;
    glm::vec3 center = min + extent;

    // m_root.m_bounds = AABB(center, extent);
    m_root = CreateOctant(center, extent, 0, n - 1, n);
}

Octree::Octant* Octree::CreateOctant(glm::vec3 center, glm::vec3 extent, size_t startIndex, size_t endIndex, size_t numObjects)
{
    Octant* octant = new Octant();
    octant->m_isLeaf = true;
    octant->m_bounds = AABB(center, extent);
    octant->m_startIndex = startIndex;
    octant->m_endIndex = endIndex;
    octant->m_count = numObjects;

    if (numObjects > 4)
    {
        octant->m_isLeaf = false;

        const std::vector<glm::vec3>& points = *m_data;
        // each child ( 8 octants ), start index, end index, and size.
        std::vector<std::tuple<size_t, size_t, size_t>> child(8, { 0, 0, 0 });

        // link disjoint child subsets.
        size_t index = startIndex;
        for (size_t i = 0; i < numObjects; ++i)
        {
            const glm::vec3& p = points[index];
            uint32_t mortonCode = 0;
            if (p.x > center.x) mortonCode |= 1;
            if (p.y > center.y) mortonCode |= 2;
            if (p.z > center.z) mortonCode |= 4;

            // find octant child based on morton code.
            auto& childTuple = child[mortonCode];
            auto& childStartIndex = std::get<0>(childTuple);
            auto& childEndIndex = std::get<1>(childTuple);
            auto& childSize = std::get<2>(childTuple);

            if (childSize == 0)
            {
                childStartIndex = index;
            }
            else
            {
                // the index from the end index of this child
                m_edges[childEndIndex] = index;
            }

            // increment size.
            childSize++;

            // end index
            childEndIndex = index;

            // advance the index
            index = m_edges[index];
        }

        glm::vec3 childExtent = extent * 0.5f;
        bool first = true;
        size_t lastChildIndex = 0u;
        for (size_t i = 0; i < 8; ++i)
        {
            auto& childTuple = child[i];
            // child size
            if (std::get<2>(childTuple) == 0) { continue; }

            // determine the position of this child.
            glm::vec3 childDirection = {
                (i & 1) > 0 ? 1.0f : -1.0f,
                (i & 2) > 0 ? 1.0f : -1.0f,
                (i & 4) > 0 ? 1.0f : -1.0f
            };
            glm::vec3 childPos = center + childDirection * childExtent;

            octant->m_child[i] = CreateOctant(childPos, childExtent,
                std::get<0>(childTuple), std::get<1>(childTuple), std::get<2>(childTuple));

            if (first)
            {
                first = false;
                octant->m_startIndex = octant->m_child[i]->m_startIndex;
            }
            else
            {
                m_edges[octant->m_child[lastChildIndex]->m_endIndex] = octant->m_child[i]->m_startIndex;
            }
            
            lastChildIndex = i;

            // push last index of parent octant.
            octant->m_endIndex = octant->m_child[i]->m_endIndex;
        }

    }

    return octant;
}

void Octree::Subdivide()
{
    auto center = m_bounds.GetPosition();
    auto halfSize = m_bounds.GetHalfSize() * 0.5f;

    const glm::vec3 ufl = center + glm::vec3(-1, 1, 1) * halfSize;
    const glm::vec3 ufr = center + glm::vec3(1, 1, 1) * halfSize;
    const glm::vec3 ubl = center + glm::vec3(-1, 1, -1) * halfSize;
    const glm::vec3 ubr = center + glm::vec3(1, 1, -1) * halfSize;

    const glm::vec3 dfl = center + glm::vec3(-1, -1, 1) * halfSize;
    const glm::vec3 dfr = center + glm::vec3(1, -1, 1) * halfSize;
    const glm::vec3 dbl = center + glm::vec3(-1, -1, -1) * halfSize;
    const glm::vec3 dbr = center + glm::vec3(1, -1, -1) * halfSize;

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

void Octree::FindNeighborsAlt(const glm::vec3& pos, float range, std::vector<size_t>& outIndiceResults)
{
    outIndiceResults.clear();
    if (m_root == nullptr) 
    {
        return;
    }

    FindNeighborsAlt(m_root, pos, range, range * range, outIndiceResults);
}

void Octree::FindNeighborsAlt(Octant* octant, const glm::vec3& pos, float range, float rangeSq, std::vector<size_t>& outIndiceResults)
{
    const std::vector<glm::vec3>& points = *m_data;

    // contains full octant, add all indices.
    if (ContainsOctant(octant, pos, rangeSq))
    {
        size_t index = octant->m_startIndex;
        for (size_t i = 0; i < octant->m_count; i++)
        {
            outIndiceResults.push_back(index);
            index = m_edges[index];
        }
        return;
    }

    if (octant->m_isLeaf)
    {
        size_t index = octant->m_startIndex;
        for (size_t i = 0; i < octant->m_count; i++)
        {
            const glm::vec3& p = points[index];
            if (glm::length2(pos - p) < rangeSq) 
            {
                outIndiceResults.push_back(index);
            }
            index = m_edges[index];
        }
        return;
    }

    for (size_t i = 0; i < 8; i++)
    {
        if (octant->m_child[i] == nullptr) { continue; }
        if (!OverlapsOctant(octant->m_child[i], pos, range, rangeSq)) { continue; }
        FindNeighborsAlt(octant->m_child[i], pos, range, rangeSq, outIndiceResults);
    }
}

bool Octree::ContainsOctant(Octant* octant, const glm::vec3& pos, float rangeSq)
{
    // find the distance to the center.
    glm::vec3 diff = glm::abs(octant->m_bounds.GetPosition() - pos);
    // add the extent.
    diff += glm::vec3(octant->m_bounds.GetHalfSize(), octant->m_bounds.GetHalfSize(), octant->m_bounds.GetHalfSize());
    // diff is now the vector to the furthest points on the octant
    return glm::length2(diff) < rangeSq;
}

bool Octree::OverlapsOctant(Octant* octant, const glm::vec3& pos, float range, float rangeSq)
{
    // find the distance to the center.
    glm::vec3 diff = glm::abs(octant->m_bounds.GetPosition() - pos);

    float extent = octant->m_bounds.GetHalfSize();
    float maxDistance = range + octant->m_bounds.GetHalfSize();

    if (diff.x > maxDistance || diff.y > maxDistance || diff.z > maxDistance)
    {
        return false;
    }

    size_t numLessExtent = (diff.x < extent) + (diff.y < extent) + (diff.z < extent);
    
    // inside surface region of octant
    if (numLessExtent > 1) 
    {
        return true;
    }

    diff = {
        std::max(diff.x - extent, 0.0f),
        std::max(diff.y - extent, 0.0f),
        std::max(diff.z - extent, 0.0f),
    };

    return glm::length2(diff) < rangeSq;
}

void Octree::FindNeighbors(const glm::vec3& pos, float range, std::vector<OcNode>& outResult)
{
    if (!m_bounds.Contains(pos, range)) { return; }
    for (const OcNode& node : m_nodes)
    {
        if (glm::length(pos - node.m_pos) <= range)
        {
            outResult.push_back(node);
        }
    }

    if (m_ufl == nullptr) { return; }

    m_ufl->FindNeighbors(pos, range, outResult);
    m_ufr->FindNeighbors(pos, range, outResult);
    m_ubl->FindNeighbors(pos, range, outResult);
    m_ubr->FindNeighbors(pos, range, outResult);
    m_dfl->FindNeighbors(pos, range, outResult);
    m_dfr->FindNeighbors(pos, range, outResult);
    m_dbl->FindNeighbors(pos, range, outResult);
    m_dbr->FindNeighbors(pos, range, outResult);
}

void Octree::Search(const AABB& aabb, std::vector<OcNode>& outResult)
{
    if (!m_bounds.Contains(aabb)) { return; }

    for (const OcNode& node : m_nodes)
    {
        if (aabb.Contains(node.m_pos))
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
        if (frustum.Contains(node.m_pos) != ContainmentType::Disjoint)
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
    if (!m_nodes.empty()) {
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
