#pragma once

#include "Shading/TextureCube.h"

#include "Engine/Vector.h"

/*

  Container object for holding all per-environment specific pre-computed PBR data.

*/
struct PBRCapture
{
	TextureCube* Irradiance = nullptr;
	TextureCube* Prefiltered = nullptr;

	glm::vec3 m_position;
	float      Radius;
};


