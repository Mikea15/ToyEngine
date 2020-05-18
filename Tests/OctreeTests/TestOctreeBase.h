#pragma once

#include "../TestRunner.h"

#include "Engine/Utils/MathUtils.h"

#include <glm/glm.hpp>

struct OctreeBaseTest
    : BaseTest
{
	~OctreeBaseTest() override {}

	void Init() override
	{
		points.clear();
		points.resize(nPoints);

		for (size_t i = 0; i < nPoints; i++)
		{
			points[i] = MathUtils::RandomInUnitSphere() * 10.0f;
		}
	}

	void Run() override
	{

	}

protected:
	int output = -1;

	float range = 5.0f;
	float rangeSq = 5.0f * 5.0f;
	glm::vec3 qPoint = { 0.0f, 0.0f, 0.0f };
	size_t nPoints = 5000;
	std::vector<glm::vec3> points;
};