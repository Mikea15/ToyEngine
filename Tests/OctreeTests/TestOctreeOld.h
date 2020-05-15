#pragma once

#include "TestOctreeBase.h"

struct TestOctreeOldInsert
	: OctreeBaseTest
{
	GENERIC_TEST_CTOR(TestOctreeOldInsert);

	void Setup() override
	{
		OctreeBaseTest::Setup();

		oct = Octree(glm::vec3(0.0f), 10.0f);
	}

	void CoreTest() override
	{
		for (size_t i = 0; i < nPoints; i++)
		{
			oct.Insert(points[i], i);
		}
	}

	Octree oct;
};

struct TestOctreeOldSearch
	: OctreeBaseTest
{
	GENERIC_TEST_CTOR(TestOctreeOldSearch);

	void Setup() override
	{
		OctreeBaseTest::Setup();

		oct = Octree(glm::vec3(0.0f), 10.0f);
		for (size_t i = 0; i < nPoints; i++)
		{
			oct.Insert(points[i], i);
		}
	}

	void CoreTest() override
	{
		for (size_t i = 0; i < nTests; i++)
		{
			result.clear();
			oct.Search(AABB(qPoint, range), result);
			result.erase(std::remove_if(result.begin(), result.end(), [&](const OcNode& n) {
				return glm::length2(qPoint - points[n.m_data]) > range * range;
				}), result.end());
			output = result.size();
		}
	}

	Octree oct;
	std::vector<OcNode> result;
};
