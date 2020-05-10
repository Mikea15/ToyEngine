#pragma once

#include <glm/glm.hpp>

class RenderTarget;

class DirectionalLight
{
public:
	glm::vec3 m_direction = glm::vec3(0.0f);
	glm::vec3 m_color = glm::vec3(1.0f);
	float m_intensity = 1.0f;

	bool m_castShadows = true;
	RenderTarget* m_shadowMatRenderTarget;
	glm::mat4 m_lightSpaceViewPrrojection;
};


