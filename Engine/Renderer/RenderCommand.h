#pragma once

#include <glm/glm.hpp>

class Mesh;
class Material;

struct RenderCommand
{
	glm::mat4 Transform;
	glm::mat4 PrevTransform;

	// Swap for AABB maybe?
	glm::vec3 BoxMin;
	glm::vec3 BoxMax;

	Material* Material;
	Mesh* Mesh;
};


