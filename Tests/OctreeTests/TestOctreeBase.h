#pragma once

#include "../TestRunner.h"

#include "Engine/Utils/MathUtils.h"

#include <glm/glm.hpp>

struct OctreeBaseTest
    : BaseTest
{
	~OctreeBaseTest() override {}

	void Setup() override
	{
		points.clear();
		points.resize(nPoints);

		for (size_t i = 0; i < nPoints; i++)
		{
			points[i] = MathUtils::RandomInUnitSphere() * 10.0f;
		}
	}

	void Execute() override
	{
		Setup();
		printf("Test: %s ", TestName.c_str());
		ProfileTime time;
		for (size_t i = 0; i < nTests; ++i)
		{
			CoreTest();
		}
		const int ms = time.GetTime();
		if (output >= 0) {
			printf(" - (output: %d) ", output);
		}
		printf(" - time: %d ms\n", ms);
	}

	void CoreTest() override
	{
		// NOTE (MA): needs to be implemented by children tests.
	}

protected:
	int output = -1;

	float range = 5.0f;
	glm::vec3 qPoint = { 0.0f, 0.0f, 0.0f };
	size_t nPoints = 50000;
	size_t nTests = 100;
	std::vector<glm::vec3> points;
};