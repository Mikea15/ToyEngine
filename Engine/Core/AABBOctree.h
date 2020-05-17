#pragma once

#include <memory>
#include <vector>

#include <glm/glm.hpp>

#include "Systems/AABB.h"

class BoundingFrustum;

struct OcNode
{
	glm::vec3 m_pos;
	size_t m_data;
};

// template<class T>
// template<size_t maxSize = 16>
class AABBOctree
{
public:
	/*
	using value_type		= T;
	using pointer			= T*;
	using const_pointer		= const T*;
	using reference			= T&;
	using const_reference	= const T&;
	*/

	AABBOctree();
	AABBOctree(const glm::vec3& position, float halfSize);
	~AABBOctree();

	bool Insert(const glm::vec3& pos, size_t index = -1);
	void FindNeighbors(const glm::vec3& pos, float radius, std::vector<OcNode>& outResult);
	void Search(const AABB& aabb, std::vector<OcNode>& outResult);
	void Search(const BoundingFrustum& frustum, std::vector<OcNode>& outResult);

	void GetAllBoundingBoxes(std::vector<AABB>& outResult);
	void DebugDraw();

private:
	void Subdivide();
	void InternalSearch(const AABB& aabb, std::vector<OcNode>& outResult);
	void InternalSearch(const BoundingFrustum& frustum, std::vector<OcNode>& outResult);
	void InternalFindNeighbors(const glm::vec3& pos, float radius, float radiusSq, std::vector<OcNode>& outResult);

private:
	AABB m_bounds;

	size_t m_maxNodes = 16;
	std::vector<OcNode> m_nodes;

	AABBOctree* m_child[8];
};

