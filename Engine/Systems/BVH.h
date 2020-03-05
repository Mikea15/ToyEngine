
#pragma once

#include "AABB.h"
#include "Utils/MathUtils.h"

#include <vector>
#include <stack>
#include <queue>

#include <sstream>

namespace bvh
{
	using Vec3 = glm::vec3;
	struct Node;
	struct Tree;

	struct Node
	{
		AABB box;
		int objectIndex;
		int parentIndex;
		int child1;
		int child2;
		bool isLeaf;
	};

	struct Tree
	{
		static int nullIndex;

		void InsertNode(int objectIndex, AABB box)
		{
			int leafIndex = CreateNode(objectIndex, box);
			if (nodeCount == 1)
			{
				rootIndex = leafIndex;
				return;
			}

			// stage 1: find the best sibling for the new leaf
			int sibling = 0;
			for (int i = 0; i < nodeCount; ++i)
			{
				// sibling = PickBest(tree, leafIndex, i);
			}

			// stage 2: create new parent
			int oldParent = m_nodes[sibling]->parentIndex;
			int newParent = CreateInternalNode();

			m_nodes[newParent]->parentIndex = oldParent;
			m_nodes[newParent]->box = AABB::Union(m_nodes[sibling]->box, m_nodes[leafIndex]->box);

			if (m_nodes[sibling]->parentIndex == nullIndex)
			{
				// the sibling was the root
				m_nodes[newParent]->child1 = sibling;
				m_nodes[newParent]->child2 = leafIndex;

				m_nodes[sibling]->parentIndex = newParent;
				m_nodes[leafIndex]->parentIndex = newParent;

				rootIndex = newParent;
			}
			else
			{
				// the sibling was not the root
				if (m_nodes[oldParent]->child1 == sibling)
				{
					m_nodes[oldParent]->child1 = newParent;
				}
				else
				{
					m_nodes[oldParent]->child2 = newParent;
				}

				m_nodes[newParent]->child1 = sibling;
				m_nodes[newParent]->child2 = leafIndex;
				m_nodes[sibling]->parentIndex = newParent;
				m_nodes[leafIndex]->parentIndex = newParent;
			}

			// stages 3: walk back up the tree refitting AABBs
			int index = m_nodes[leafIndex]->parentIndex;
			while (index != nullIndex)
			{
				int child1 = m_nodes[index]->child1;
				int child2 = m_nodes[index]->child2;

				// Rotate(index);

				m_nodes[index]->box = AABB::Union(m_nodes[child1]->box, m_nodes[child2]->box);
				index = m_nodes[index]->parentIndex;
			}
		}

		int CreateNode(int objectIndex, AABB box) 
		{
			Node* node = new Node();
			node->box = box;
			node->objectIndex = objectIndex;
			node->child1 = nullIndex;
			node->child2 = nullIndex;
			node->parentIndex = nullIndex;
			node->isLeaf = true;

			m_nodes.push_back(node);
			nodeCount = m_nodes.size();
			return nodeCount -1;
		}

		int CreateInternalNode()
		{
			Node* node = new Node();
			node->box = AABB();
			node->objectIndex = nullIndex;
			node->child1 = nullIndex;
			node->child2 = nullIndex;
			node->parentIndex = nullIndex;
			node->isLeaf = false;

			m_nodes.push_back(node);
			nodeCount = m_nodes.size();
			return nodeCount-1;
		}

		const std::vector<Node*>& GetNodes() const { return m_nodes; }

		std::vector<Node*> m_nodes;
		Node* nodes;
		int nodeCount;
		int rootIndex;
	};

#if 0
	float ComputeCost(Tree tree)
	{
		float cost = 0.0f;
		for (int i = 0; i < tree.nodeCount; ++i)
		{
			if (!tree.nodes[i].isLeaf) 
			{
				cost += tree.nodes[i].box.Area();
			}
		}
		return cost;
	}

	bool TreeRayCast(Tree tree, Vec3 p1, Vec3 p2)
	{
		std::stack<int> stack;
		stack.push(tree.rootIndex);
		while (!stack.empty())
		{
			int index = stack.top();
			stack.pop();

			//if (!TestOverlap(tree.nodes[index].box, p1, p2))
			{
				continue;
			}

			// we hit an AABB
			if (tree.nodes[index].isLeaf)
			{
				int objectIndex = tree.nodes[index].objectIndex;
				//if (RayCast(objects[objectIndex], p1, p2))
				{
					// we hit the Object inside the AABB
					return true;
				}
			}
			else
			{
				// Not a leaf, so add children to be checked against.
				stack.push(tree.nodes[index].child1);
				stack.push(tree.nodes[index].child2);
			}
		}
		return false;
	}

	int PickBest(Tree tree, int leaf, int nodeIndex)
	{
		float inheritedCost = 0.0f;
		while( nodeIndex >= 0 ) {
			inheritedCost += tree.nodes[nodeIndex].box.Area();
			nodeIndex--;
		}

		float directCost = AABB::Union(tree.nodes[leaf].box, tree.nodes[nodeIndex].box).Area();
		//if (directCost + inheritedCost < cBest)
		{
		//	cBest = directCost + inheritedCost;
		}

		float lowerBoundCost = AABB::Union(tree.nodes[tree.nodes[nodeIndex].child1].box,
			tree.nodes[tree.nodes[nodeIndex].child2].box).Area();
		//if (lowerBoundCost > cBest)
		{
			return nodeIndex;
		}
	}

	int AllocateLeafNode(Tree tree, int objectIndex, AABB box)
	{
		Node* node = new Node();
		node->box = box;
		node->objectIndex = objectIndex;
		node->child1 = nullIndex;
		node->child2 = nullIndex;
		node->parentIndex = nullIndex;
		node->isLeaf = true;
		return 0;
	}

#endif
}
