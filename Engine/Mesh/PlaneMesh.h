#pragma once

#include "Mesh.h" 

class PlaneMesh : public Mesh
{
public:
    PlaneMesh() = default;
	PlaneMesh(unsigned int xSegments, unsigned int ySegments);
};

