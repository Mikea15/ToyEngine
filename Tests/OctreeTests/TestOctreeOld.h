#pragma once

#include "TestOctreeBase.h"
#include "Engine/Core/AABBOctree.h"

struct TestOctreeOldInsert
	: OctreeBaseTest
{
	GENERIC_TEST_CTOR(TestOctreeOldInsert);

	void Run() override
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

	void Init() override
	{
		OctreeBaseTest::Init();

		oct = AABBOctree(glm::vec3(0.0f), 10.0f);
		for (size_t i = 0; i < nPoints; i++)
		{
			oct.Insert(points[i], i);
		}
	}

	void Run() override
	{
		result.clear();
		oct.Search(AABB(qPoint, range), result);
		result.erase(std::remove_if(result.begin(), result.end(), [&](const OcNode& n) {
			return glm::length2(qPoint - points[n.m_data]) > rangeSq;
			}), result.end());
		output = result.size();

		OctreeBaseTest::Run();
	}

	AABBOctree oct;
	std::vector<OcNode> result;
};
