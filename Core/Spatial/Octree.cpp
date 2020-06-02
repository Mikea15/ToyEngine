#include "Octree.h"

#include <algorithm>
#include <glm/gtx/norm.hpp>

namespace core
{
    Octree::Octree()
        : m_root(nullptr)
        , m_data(nullptr)
    {

    }

    Octree::~Octree()
    {
        delete m_root;
        delete m_data;
    }

    void Octree::Initialize(const std::vector<glm::vec3>& points)
    {
        assert(!points.empty());

        Clear();

        const size_t n = points.size();
        m_data = new std::vector<glm::vec3>(points); // copy points for now

        m_edges = std::vector<size_t>(n);

        glm::vec3 min = points[0];
        glm::vec3 max = points[0];

        for (size_t i = 0; i < n; ++i)
        {
            // always point to the next.
            m_edges[i] = i + 1;

            if (points[i].x < min.x) { min.x = points[i].x; }
            if (points[i].y < min.y) { min.y = points[i].y; }
            if (points[i].z < min.z) { min.z = points[i].z; }
            if (points[i].x > max.x) { max.x = points[i].x; }
            if (points[i].y > max.y) { max.y = points[i].y; }
            if (points[i].z > max.z) { max.z = points[i].z; }
        }

        glm::vec3 center = min + (max - min) * 0.5f;
        glm::vec3 extent = (max - min);
        float maxExtent = 0.0f;
        for (size_t d = 0; d < 3; ++d)
        {
            if (extent[d] > maxExtent)
            {
                maxExtent = extent[d];
            }
        }
        m_root = CreateOctant(center, maxExtent, 0, n - 1, n);
    }

    void Octree::Clear()
    {
        delete m_root;
        m_root = nullptr;
        m_data = nullptr;
    }

    void Octree::FindNeighbors(const glm::vec3& position, float radius, std::vector<size_t>& outIndices)
    {
        outIndices.clear();
        if (m_root == nullptr)
        {
            return;
        }

        FindNeighbors(m_root, position, radius, radius * radius, outIndices);
    }

    Octree::Octant* Octree::CreateOctant(glm::vec3 center, float extent, size_t start, size_t end, size_t size)
    {
        Octant* octant = new Octant();
        octant->m_isLeaf = true;
        octant->m_center = center;
        octant->m_radius = extent;
        octant->m_start = start;
        octant->m_end = end;
        octant->m_size = size;

        if (size > m_maxNodesPerLeaf)
        {
            octant->m_isLeaf = false;

            const std::vector<glm::vec3>& points = *m_data;
            // each child ( 8 octants ), start index, end index, and size.
            std::vector<std::tuple<size_t, size_t, size_t>> child(8, { 0, 0, 0 });

            // link disjoint child subsets.
            size_t currentIndex = start;
            for (size_t i = 0; i < size; ++i)
            {
                const glm::vec3& p = points[currentIndex];

                uint32_t mortonCode = 0;
                if (p.x > center.x) mortonCode |= 1;
                if (p.y > center.y) mortonCode |= 2;
                if (p.z > center.z) mortonCode |= 4;

                // find octant child based on morton code.
                auto& childStartIdx = std::get<0>(child[mortonCode]);
                auto& childEndIdx = std::get<1>(child[mortonCode]);
                auto& childSize = std::get<2>(child[mortonCode]);

                if (childSize == 0)
                {
                    childStartIdx = currentIndex;
                }
                else
                {
                    m_edges[childEndIdx] = currentIndex;
                }

                childSize++;
                childEndIdx = currentIndex;
                currentIndex = m_edges[currentIndex];
            }

            float childExtent = extent * 0.5f;
            bool first = true;
            size_t lastChildIndex = 0u;
            for (size_t i = 0; i < 8; ++i)
            {
                auto& childStartIdx = std::get<0>(child[i]);
                auto& childEndIdx = std::get<1>(child[i]);
                auto& childSize = std::get<2>(child[i]);

                if (childSize == 0) { continue; }

                // determine the position of this child.
                glm::vec3 childDirection = {
                    (i & 1) > 0 ? 1.0f : -1.0f,
                    (i & 2) > 0 ? 1.0f : -1.0f,
                    (i & 4) > 0 ? 1.0f : -1.0f
                };
                glm::vec3 childPos = center + childDirection * childExtent;

                octant->m_child[i] = CreateOctant(childPos, childExtent, childStartIdx, childEndIdx, childSize);

                if (first)
                {
                    octant->m_start = octant->m_child[i]->m_start;
                }
                else
                {
                    m_edges[octant->m_child[lastChildIndex]->m_end] = octant->m_child[i]->m_start;
                }

                lastChildIndex = i;

                // push last index of parent octant.
                octant->m_end = octant->m_child[i]->m_end;
                first = false;
            }
        }

        return octant;
    }

    void Octree::FindNeighbors(Octant* octant, const glm::vec3& pos, float radius, float radiusSq, std::vector<size_t>& outIndiceResults)
    {
        const std::vector<glm::vec3>& points = *m_data;

        // contains full octant, add all indices.
        if (ContainsOctant(octant, pos, radiusSq))
        {
            size_t index = octant->m_start;
            for (size_t i = 0; i < octant->m_size; ++i)
            {
                outIndiceResults.emplace_back(index);
                index = m_edges[index];
            }
            return;
        }

        if (octant->m_isLeaf)
        {
            size_t index = octant->m_start;
            for (size_t i = 0; i < octant->m_size; i++)
            {
                const glm::vec3& p = points[index];
                float distSq = glm::length2(p - pos);
                if (distSq > 0.0 && distSq < radiusSq)
                {
                    outIndiceResults.emplace_back(index);
                }
                index = m_edges[index];
            }
            return;
        }

        for (size_t i = 0; i < 8; ++i)
        {
            if (octant->m_child[i] == nullptr) { continue; }
            if (!OverlapsOctant(octant->m_child[i], pos, radius, radiusSq)) { continue; }
            FindNeighbors(octant->m_child[i], pos, radius, radiusSq, outIndiceResults);
        }
    }

    bool Octree::ContainsOctant(Octant* octant, const glm::vec3& pos, float rangeSq)
    {
        // find the distance to the center.
        glm::vec3 diff = glm::abs(octant->m_center - pos);
        // add the extent.
        diff += glm::vec3(octant->m_radius, octant->m_radius, octant->m_radius);
        // diff is now the vector to the furthest points on the octant
        return glm::length2(diff) < rangeSq;
    }

    bool Octree::OverlapsOctant(Octant* octant, const glm::vec3& pos, float radius, float radiusSq)
    {
        // find the distance to the center.
        glm::vec3 diff = glm::abs(octant->m_center - pos);

        float maxDistance = radius + octant->m_radius;

        if (diff.x > maxDistance || diff.y > maxDistance || diff.z > maxDistance)
        {
            return false;
        }

        size_t numLessExtent = (diff.x < octant->m_radius) + (diff.y < octant->m_radius) + (diff.z < octant->m_radius);

        // inside surface region of octant
        if (numLessExtent > 1)
        {
            return true;
        }

        diff = {
            std::max(diff.x - octant->m_radius, 0.0f),
            std::max(diff.y - octant->m_radius, 0.0f),
            std::max(diff.z - octant->m_radius, 0.0f),
        };

        return glm::length2(diff) < radiusSq;
    }

    bool Octree::InsideOctant(Octant* octant, const glm::vec3& pos, float range)
    {
        glm::vec3 diff = glm::abs(pos - octant->m_center);
        diff += glm::vec3(octant->m_radius, octant->m_radius, octant->m_radius);

        if (diff.x > octant->m_radius) return false;
        if (diff.y > octant->m_radius) return false;
        if (diff.z > octant->m_radius) return false;

        return true;
    }
}