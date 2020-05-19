#pragma once

#include "Shading/TextureCube.h"

#include <glm/glm.hpp>

struct PBRCapture
{
	TextureCube* Irradiance = nullptr;
	TextureCube* Prefiltered = nullptr;

	glm::vec3 m_position;
	float      Radius;
};


