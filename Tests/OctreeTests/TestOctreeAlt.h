#pragma once

#include "TestOctreeBase.h"

#include "Engine/Core/AABBOctree.h"

struct TestOctreeAltInsert
    : OctreeBaseTest
{
	GENERIC_TEST_CTOR(TestOctreeAltInsert);

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

struct TestOctreeAltSearch
    : OctreeBaseTest
{
	GENERIC_TEST_CTOR(TestOctreeAltSearch);

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
		oct.FindNeighbors(qPoint, range, result);
		output = result.size();
	}

	AABBOctree oct;
	std::vector<OcNode> result;
};
