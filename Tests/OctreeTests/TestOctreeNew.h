#pragma once

#include "TestOctreeBase.h"
#include "Engine/Core/Octree.h"

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

	core::Octree oct;
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
		oct.FindNeighbors(qPoint, range, indices);
		output = indices.size();
	}

	std::vector<size_t> indices;
	core::Octree oct;
};



