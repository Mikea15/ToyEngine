#include "PlaneMesh.h"

PlaneMesh::PlaneMesh(unsigned int xSegments, unsigned int ySegments)
{
	bool oddRow = false;

	float dX = 1.0f / xSegments;
	float dY = 1.0f / ySegments;

	for (unsigned int y = 0; y <= ySegments; ++y)
	{
		for (unsigned int x = 0; x <= xSegments; ++x)
		{
			Positions.push_back(glm::vec3(dX * x * 2.0f - 1.0f, dY * y * 2.0f - 1.0f, 0.0f));
			UV.push_back(glm::vec2(dX * x, 1.0f - y * dY));
			Normals.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
		}
	}

	for (unsigned int y = 0; y < ySegments; ++y)
	{
		if (!oddRow) // even rows: y == 0, y == 2; and so on
		{
			for (unsigned int x = 0; x <= xSegments; ++x)
			{
				Indices.push_back(y * (xSegments + 1) + x);
				Indices.push_back((y + 1) * (xSegments + 1) + x);
			}
		}
		else
		{
			for (unsigned int x = xSegments; x >= 0; --x)
			{
				Indices.push_back((y + 1) * (xSegments + 1) + x);
				Indices.push_back(y * (xSegments + 1) + x);
			}
		}
		oddRow = !oddRow;
	}

	Topology = TOPOLOGY::TRIANGLE_STRIP;
	Finalize();
}
