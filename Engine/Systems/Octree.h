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
class Octree
{
public:
	/*
	using value_type		= T;
	using pointer			= T*;
	using const_pointer		= const T*;
	using reference			= T&;
	using const_reference	= const T&;
	*/

	struct alignas(16) Octant
	{
		Octant()
		{
			memset(&m_child, 0, 8 * sizeof(Octant*));
		}
		~Octant()
		{
			for (size_t i = 0; i < 8; ++i)
			{
				delete m_child[i];
			}
		}

		Octant* m_child[8];

		glm::vec3 m_center{ 0.0f, 0.0f, 0.0f };
		float m_radius = 0.0f;

		size_t m_start = 0u;
		size_t m_end = 0u;
		size_t m_size = 0u;
		
		bool m_isLeaf = false;
	};
	

	Octree();
	Octree(const glm::vec3& position, float halfSize);
	~Octree();

	void Clear();
	void Initialize(const std::vector<glm::vec3>& points);
	bool Insert(const glm::vec3& pos, size_t index = -1);
	void FindNeighborsAlt(const glm::vec3& pos, float range, std::vector<size_t>& outIndiceResults);
	void FindNeighbors(const glm::vec3& pos, float range, std::vector<OcNode>& outResult);
	void Search(const AABB& aabb, std::vector<OcNode>& outResult);
	void Search(const BoundingFrustum& frustum, std::vector<OcNode>& outResult);
	void GetAllBoundingBoxes(std::vector<AABB>& outResult);
	void DebugDraw();

private:
	Octant* CreateOctant(glm::vec3 center, float extent, size_t start, size_t end, size_t size);
	
	void FindNeighborsAlt(Octant* octant, const glm::vec3& pos, float range, float rangeSq, std::vector<size_t>& outIndiceResults);
	bool ContainsOctant(Octant* octant, const glm::vec3& pos, float rangeSq);
	bool OverlapsOctant(Octant* octant, const glm::vec3& pos, float range, float rangeSq);
	bool InsideOctant(Octant* octant, const glm::vec3& pos, float rangeSq);
	void Subdivide();

private:
	AABB m_bounds;

	size_t m_maxNodes = 16;
	std::vector<OcNode> m_nodes;

	const std::vector<glm::vec3>* m_data;
	std::vector<size_t> m_edges;
	Octant* m_root = nullptr;

	Octree* m_ufl = nullptr;
	Octree* m_ufr = nullptr;
	Octree* m_ubl = nullptr;
	Octree* m_ubr = nullptr;
	Octree* m_dfl = nullptr;
	Octree* m_dfr = nullptr;
	Octree* m_dbl = nullptr;
	Octree* m_dbr = nullptr;
};

