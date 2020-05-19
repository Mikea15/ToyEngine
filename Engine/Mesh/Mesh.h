#pragma once

#include <vector>
#include <functional>

#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>

static const float PI = 3.14159265359f;
#ifndef TAU
static const float TAU = 2.0f * PI;
#endif

enum class TOPOLOGY
{
	POINTS,
	LINES,
	LINE_STRIP,
	TRIANGLES,
	TRIANGLE_STRIP,
	TRIANGLE_FAN,
};

class Mesh
{
public:
	std::vector<glm::vec3> Positions;
	std::vector<glm::vec2> UV;
	std::vector<glm::vec3> Normals;
	std::vector<glm::vec3> Tangents;
	std::vector<glm::vec3> Bitangents;

	TOPOLOGY Topology = TOPOLOGY::TRIANGLES;
	std::vector<unsigned int> Indices;

	Mesh();
	Mesh(std::vector<glm::vec3> positions, std::vector<unsigned int> indices);
	Mesh(std::vector<glm::vec3> positions, std::vector<glm::vec2> uv, std::vector<unsigned int> indices);
	Mesh(std::vector<glm::vec3> positions, std::vector<glm::vec2> uv, std::vector<glm::vec3> normals, std::vector<unsigned int> indices);
	Mesh(std::vector<glm::vec3> positions, std::vector<glm::vec2> uv, std::vector<glm::vec3> normals, std::vector<glm::vec3> tangents, std::vector<glm::vec3> bitangents, std::vector<unsigned int> indices);

	void SetPositions(std::vector<glm::vec3> positions);
	void SetUVs(std::vector<glm::vec2> uv);
	void SetNormals(std::vector<glm::vec3> normals);
	void SetTangents(std::vector<glm::vec3> tangents, std::vector<glm::vec3> bitangents);

	// commits all buffers and attributes to the GPU driver
	void Finalize(bool interleaved = true);

private:
	void calculateNormals(bool smooth = true);

public:
	unsigned int m_vao = 0;
	unsigned int m_vbo = 0;
	unsigned int m_ebo = 0;
};

