#pragma once

#include <vector>
#include <functional>

#define GLM_SWIZZLE
#include <glm/glm.hpp>


static const float PI = 3.14159265359f;
#ifndef TAU
static const float TAU = 2.0f * PI;
#endif

/*

  Manually define a list of topology types as we don't want to directly link a mesh to an
  OpenGL topology type as this would reduce the renderer's cross compatibility.

  Yes, yes I know that we still have OpenGL indices in here (VAO, VBO, EBO) which we'll
  get rid of in a cross renderer way soon enough.

*/
enum class TOPOLOGY
{
	POINTS,
	LINES,
	LINE_STRIP,
	TRIANGLES,
	TRIANGLE_STRIP,
	TRIANGLE_FAN,
};

/*

  Base Mesh class. A mesh in its simplest form is purely a list of vertices, with some added
  functionality for easily setting up the hardware configuration relevant for rendering.

*/
class Mesh
{
	// NOTE(Joey): public for now for testing and easy access; will eventually be private and only visible to renderer (as a friend class)
public:
	unsigned int m_VAO = 0;
	unsigned int m_VBO;
	unsigned int m_EBO;
public:
	std::vector<glm::vec3> Positions;
	std::vector<glm::vec2> UV;
	std::vector<glm::vec3> Normals;
	std::vector<glm::vec3> Tangents;
	std::vector<glm::vec3> Bitangents;

	TOPOLOGY Topology = TOPOLOGY::TRIANGLES;
	std::vector<unsigned int> Indices;

	// support multiple ways of initializing a mesh
	Mesh();
	Mesh(std::vector<glm::vec3> positions, std::vector<unsigned int> indices);
	Mesh(std::vector<glm::vec3> positions, std::vector<glm::vec2> uv, std::vector<unsigned int> indices);
	Mesh(std::vector<glm::vec3> positions, std::vector<glm::vec2> uv, std::vector<glm::vec3> normals, std::vector<unsigned int> indices);
	Mesh(std::vector<glm::vec3> positions, std::vector<glm::vec2> uv, std::vector<glm::vec3> normals, std::vector<glm::vec3> tangents, std::vector<glm::vec3> bitangents, std::vector<unsigned int> indices);

	// set vertex data manually
	// TODO(Joey): not sure if these are required if we can directly set vertex data from public fields; construct several use-cases to test.
	void SetPositions(std::vector<glm::vec3> positions);
	void SetUVs(std::vector<glm::vec2> uv);
	void SetNormals(std::vector<glm::vec3> normals);
	void SetTangents(std::vector<glm::vec3> tangents, std::vector<glm::vec3> bitangents); // NOTE(Joey): you can only set both tangents and bitangents at the same time to prevent mismatches

	// commits all buffers and attributes to the GPU driver
	void Finalize(bool interleaved = true);

	// generate triangulated mesh from signed distance field
	void FromSDF(std::function<float(glm::vec3)>& sdf, float maxDistance, uint16_t gridResolution);

private:
	void calculateNormals(bool smooth = true);
	void calculateTangents();
};

