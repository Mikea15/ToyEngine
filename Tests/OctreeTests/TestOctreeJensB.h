#pragma once

#include "TestOctreeBase.h"
#include "Core/Spatial/exp_Octree.h"

struct TestOctreeJensBInsert
	: OctreeBaseTest
{
	GENERIC_TEST_CTOR(TestOctreeJensBInsert);

	void Init() override
	{
		OctreeBaseTest::Init();
		oParams.bucketSize = 16;
	}

	void Run() override
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

	void Init() override
	{
		OctreeBaseTest::Init();
		oParams.bucketSize = 16;
		oct.initialize(points, oParams);
	}

	void Run() override
	{
		oct.radiusNeighbors<unibn::L2Distance<glm::vec3>>(qPoint, range, indices);
		output = indices.size();
	}

	unibn::OctreeParams oParams;
	unibn::Octree<glm::vec3> oct;
	std::vector<uint32_t> indices;
};



