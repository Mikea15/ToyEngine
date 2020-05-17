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
	void FindNeighbors(const glm::vec3& pos, float range, std::vector<OcNode>& outResult);
	void Search(const AABB& aabb, std::vector<OcNode>& outResult);
	void Search(const BoundingFrustum& frustum, std::vector<OcNode>& outResult);
	void GetAllBoundingBoxes(std::vector<AABB>& outResult);
	void DebugDraw();

private:
	void Subdivide();

private:
	AABB m_bounds;

	size_t m_maxNodes = 16;
	std::vector<OcNode> m_nodes;

	AABBOctree* m_ufl = nullptr;
	AABBOctree* m_ufr = nullptr;
	AABBOctree* m_ubl = nullptr;
	AABBOctree* m_ubr = nullptr;
	AABBOctree* m_dfl = nullptr;
	AABBOctree* m_dfr = nullptr;
	AABBOctree* m_dbl = nullptr;
	AABBOctree* m_dbr = nullptr;
};

