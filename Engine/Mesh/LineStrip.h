#pragma once

#include "Mesh.h"

class LineStrip : public Mesh
{
public:
    LineStrip(float width, unsigned int segments);
};
