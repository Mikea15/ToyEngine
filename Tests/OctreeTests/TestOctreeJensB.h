#pragma once

#include "TestOctreeBase.h"
#include "Engine/Core/exp_Octree.h"

struct TestOctreeJensBInsert
	: OctreeBaseTest
{
	GENERIC_TEST_CTOR(TestOctreeJensBInsert);

	void Setup() override
	{
		OctreeBaseTest::Setup();
		oParams.bucketSize = 4;
	}

	void CoreTest() override
	{
		oct.initialize(points, oParams);
	}

	unibn::OctreeParams oParams;
	unibn::Octree<glm::vec3> oct;
};

struct TestOctreeJensBSearch
	: OctreeBaseTest
{
	GENERIC_TEST_CTOR(TestOctreeJensBSearch);

	void Setup() override
	{
		OctreeBaseTest::Setup();
		oParams.bucketSize = 4;
		oct.initialize(points, oParams);
	}

	void CoreTest() override
	{
		for (size_t i = 0; i < nTests; i++)
		{
			indices.clear();
			oct.radiusNeighbors<unibn::L2Distance<glm::vec3>>(qPoint, range, indices);
			output = indices.size();
		}
	}

	unibn::OctreeParams oParams;
	unibn::Octree<glm::vec3> oct;
	std::vector<uint32_t> indices;
};



