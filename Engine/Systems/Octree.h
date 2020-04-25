#pragma once

#include <memory>
#include <vector>

#include <glm/glm.hpp>

#include "AABB.h"

class BoundingFrustum;

struct OcNode
{
	glm::vec3 m_storePos;
	size_t m_storeIndex;
};

class Octree
{
public:
	Octree();
	Octree(const glm::vec3& position, float halfSize);
	~Octree();

	void Subdivide();
	bool Insert(const glm::vec3& pos, size_t index = -1);
	void Search(const AABB& aabb, std::vector<OcNode>& outResult);
	void Search(const BoundingFrustum& frustum, std::vector<OcNode>& outResult);

	void GetAllBoundingBoxes(std::vector<AABB>& outResult);

	void DebugDraw();

private:
	AABB m_bounds;

	size_t m_maxNodes = 16;
	std::vector<OcNode> m_nodes;

	Octree* m_ufl = nullptr;
	Octree* m_ufr = nullptr;
	Octree* m_ubl = nullptr;
	Octree* m_ubr = nullptr;
	Octree* m_dfl = nullptr;
	Octree* m_dfr = nullptr;
	Octree* m_dbl = nullptr;
	Octree* m_dbr = nullptr;
};

