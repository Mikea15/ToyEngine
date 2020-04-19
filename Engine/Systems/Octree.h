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

	int m_maxNodes = 1024;
	std::vector<OcNode> m_nodes;

	std::shared_ptr<Octree> m_upFrontLeft;
	std::shared_ptr<Octree> m_upFrontRight;
	std::shared_ptr<Octree> m_upBackLeft;
	std::shared_ptr<Octree> m_upBackRight;
	std::shared_ptr<Octree> m_downFrontLeft;
	std::shared_ptr<Octree> m_downFrontRight;
	std::shared_ptr<Octree> m_downBackLeft;
	std::shared_ptr<Octree> m_downBackRight;
};

