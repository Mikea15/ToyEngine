#pragma once

#include "TestOctreeBase.h"
#include "Engine/Core/Octree.h"

struct TestOctreeNewInsert
	: OctreeBaseTest
{
	GENERIC_TEST_CTOR(TestOctreeNewInsert);

	void Init() override
	{
		OctreeBaseTest::Init();
	}

	void Run() override
	{
		oct.Initialize(points);
	}

	core::Octree oct;
};

struct TestOctreeNewSearch
	: OctreeBaseTest
{
	GENERIC_TEST_CTOR(TestOctreeNewSearch);

	void Init() override
	{
		OctreeBaseTest::Init();
		oct.Initialize(points);
	}

	void Run() override
	{
		oct.FindNeighbors(qPoint, range, indices);
		output = indices.size();
	}

	std::vector<size_t> indices;
	core::Octree oct;
};



