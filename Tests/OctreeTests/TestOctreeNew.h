#pragma once

#include "TestOctreeBase.h"

struct TestOctreeNewInsert
	: OctreeBaseTest
{
	GENERIC_TEST_CTOR(TestOctreeNewInsert);

	void Setup() override
	{
		OctreeBaseTest::Setup();
	}

	void CoreTest() override
	{
		oct.Initialize(points);
	}

	Octree oct;
};

struct TestOctreeNewSearch
	: OctreeBaseTest
{
	GENERIC_TEST_CTOR(TestOctreeNewSearch);

	void Setup() override
	{
		OctreeBaseTest::Setup();
		oct.Initialize(points);
	}

	void CoreTest() override
	{
		int results = 0;
		for (size_t i = 0; i < nTests; i++)
		{
			indices.clear();
			oct.FindNeighborsAlt(qPoint, range, indices);

			results = indices.size();
		}
	}

	std::vector<size_t> indices;
	Octree oct;
};



