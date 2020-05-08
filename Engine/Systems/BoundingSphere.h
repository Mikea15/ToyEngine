#pragma once

#include "Engine/Vector.h"

class BoundingSphere
{
public:
	BoundingSphere();
	~BoundingSphere();

private:
	float m_radius;
	glm::vec3 m_origin;
};

