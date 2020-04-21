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
	glm::vec3 m_position;
	float m_halfSize;
	AABB m_bounds;

	size_t m_maxNodes = 64;
	std::vector<OcNode> m_nodes;

	Octree* m_upFrontLeft = nullptr;
	Octree* m_upFrontRight = nullptr;
	Octree* m_upBackLeft = nullptr;
	Octree* m_upBackRight = nullptr;
	Octree* m_downFrontLeft = nullptr;
	Octree* m_downFrontRight = nullptr;
	Octree* m_downBackLeft = nullptr;
	Octree* m_downBackRight = nullptr;
};

