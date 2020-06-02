#pragma once
/*
	Octree Implementation heavily influenced by
	J. Behley, V. Steinhage, A.B. Cremers. Efficient Radius Neighbor Search
	in Three-dimensional Point Clouds, Proc. of the IEEE International
	Conference on Robotics and Automation (ICRA), 2015.

	https://jbehley.github.io/
	https://jbehley.github.io/papers/behley2015icra.pdf
*/

#include <vector>
#include <memory>
#include <glm/glm.hpp>

namespace core
{
    class Octree
    {
    public:
		struct Octant
		{
			Octant() 
			{
				for (size_t i = 0; i < 8; ++i)
				{
					m_child[i] = nullptr;
				}
			}
			~Octant()
			{
				for (size_t i = 0; i < 8; ++i)
				{
					delete m_child[i];
					m_child[i] = nullptr;
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
		~Octree();
		void Initialize(const std::vector<glm::vec3>& points);
		void Clear();

		void FindNeighbors(const glm::vec3& position, float radius, std::vector<size_t>& outIndices);

	private:
		Octant* CreateOctant(glm::vec3 center, float extent, size_t start, size_t end, size_t size);

		void FindNeighbors(Octant* octant, const glm::vec3& pos, float range, float rangeSq, std::vector<size_t>& outIndiceResults);

		bool ContainsOctant(Octant* octant, const glm::vec3& pos, float rangeSq);
		bool OverlapsOctant(Octant* octant, const glm::vec3& pos, float range, float rangeSq);
		bool InsideOctant(Octant* octant, const glm::vec3& pos, float rangeSq);

	private:
		Octant* m_root = nullptr;

		const std::vector<glm::vec3>* m_data = nullptr;
		std::vector<size_t> m_edges;

		const size_t m_maxNodesPerLeaf = 16;
    };
}