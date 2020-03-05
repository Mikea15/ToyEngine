#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "DebugDraw.h"

class ViewportGrid
{
public:
	ViewportGrid() = default;
	ViewportGrid(float width, float depth, unsigned int rows, unsigned int cols);

	void Draw();

private:
	std::vector<DebugDraw::Line> m_lines;

	float m_width = 0;
	float m_depth = 0;

	unsigned int m_rows = 0;
	unsigned int m_cols = 0;

};