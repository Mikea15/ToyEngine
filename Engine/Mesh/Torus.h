#pragma once

#include "Mesh.h"


/*

  A 3D torus (donut) mesh characterized by its two radii: inner hole and ring's radius and the
  number of horizontal and vertical ring segments.

*/
class Torus : public Mesh
{
public:
	Torus(float r1, float r2, unsigned int lod1, unsigned int lod2);
};


