#include "Torus.h"

Torus::Torus(float r1, float r2, unsigned int numSteps1, unsigned int numSteps2)
{
	Positions.resize((numSteps1 + 1) * (numSteps2 + 1));
	Normals.resize((numSteps1 + 1) * (numSteps2 + 1));
	UV.resize((numSteps1 + 1) * (numSteps2 + 1));

	std::vector<glm::vec3> p(numSteps1 + 1);
	float a = 0.0f;
	float step = 2.0f * PI / numSteps1;
	for (unsigned int i = 0; i <= numSteps1; ++i)
	{
		float x = cos(a) * r1;
		float y = sin(a) * r1;
		p[i].x = x;
		p[i].y = y;
		p[i].z = 0.0f;
		a += step;
	}

	// generate all the vertices, UVs, Normals (and Tangents/Bitangents):
	for (unsigned int i = 0; i <= numSteps1; ++i)
	{
		// the basis vectors of the ring equal the difference  vector between the minorRing 
		// center and the donut's center position (which equals the origin (0, 0, 0)) and the 
		// positive z-axis.
		glm::vec3 u = glm::normalize(glm::vec3(0.0f) - p[i]) * r2; // Could be p[i] also
		glm::vec3 v = glm::vec3(0.0f, 0.0f, 1.0f) * r2;

		// create the vertices of each minor ring segment:
		float a = 0.0f;
		float step = 2.0f * PI / numSteps2;
		for (unsigned int j = 0; j <= numSteps2; ++j)
		{
			float c = cos(a);
			float s = sin(a);

			Positions[i * (numSteps2 + 1) + j] = p[i] + c * u + s * v;
			UV[i * (numSteps2 + 1) + j].x = ((float)i) / ((float)numSteps1) * TAU; // multiply by TAU to keep UVs symmetric along both axes.
			UV[i * (numSteps2 + 1) + j].y = ((float)j) / ((float)numSteps2);
			Normals[i * (numSteps2 + 1) + j] = glm::normalize(c * u + s * v);
			a += step;
		}
	}

	Indices.resize(numSteps1 * numSteps2 * 6);

	int index = 0;
	for (unsigned int i = 0; i < numSteps1; ++i)
	{
		int i1 = i;
		int i2 = (i1 + 1);

		for (unsigned int j = 0; j < numSteps2; ++j)
		{
			int j1 = j;
			int j2 = (j1 + 1);

			Indices[index++] = i1 * (numSteps2 + 1) + j1;
			Indices[index++] = i1 * (numSteps2 + 1) + j2;
			Indices[index++] = i2 * (numSteps2 + 1) + j1;

			Indices[index++] = i2 * (numSteps2 + 1) + j2;
			Indices[index++] = i2 * (numSteps2 + 1) + j1;
			Indices[index++] = i1 * (numSteps2 + 1) + j2;
		}
	}

	Topology = TOPOLOGY::TRIANGLES;
	Finalize();
}
