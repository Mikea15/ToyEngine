#pragma once

#include "TestOctreeBase.h"
#include "Engine/Core/AABBOctree.h"

struct TestOctreeOldInsert
	: OctreeBaseTest
{
	GENERIC_TEST_CTOR(TestOctreeOldInsert);

	void CoreTest() override
	{
		oct = AABBOctree(glm::vec3(0.0f), 10.0f);
		for (size_t i = 0; i < nPoints; i++)
		{
			oct.Insert(points[i], i);
		}
	}

	AABBOctree oct;
};

struct TestOctreeOldSearch
	: OctreeBaseTest
{
	GENERIC_TEST_CTOR(TestOctreeOldSearch);

	void Setup() override
	{
		OctreeBaseTest::Setup();

		oct = AABBOctree(glm::vec3(0.0f), 10.0f);
		for (size_t i = 0; i < nPoints; i++)
		{
			oct.Insert(points[i], i);
		}
	}

	void CoreTest() override
	{
		result.clear();
		oct.Search(AABB(qPoint, range), result);
		const float rangeSq = range * range;
		result.erase(std::remove_if(result.begin(), result.end(), [&](const OcNode& n) {
			return glm::length2(qPoint - points[n.m_data]) > rangeSq;
			}), result.end());
		output = result.size();
	}

	AABBOctree oct;
	std::vector<OcNode> result;
};
