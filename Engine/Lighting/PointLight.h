#pragma once

#include <glm/glm.hpp>

class PointLight
{
public:
	glm::vec3 m_position = glm::vec3(0.0f);
	glm::vec3 m_color = glm::vec3(1.0f);
	float m_intensity = 1.0f;
	float m_radius = 1.0f;
	bool m_visible = true;
	bool m_renderMesh = false;
};

