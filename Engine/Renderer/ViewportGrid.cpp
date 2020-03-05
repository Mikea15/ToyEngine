
#include "ViewportGrid.h"

ViewportGrid::ViewportGrid(float width, float depth, unsigned int rows, unsigned int cols)
	: m_width(width)
	, m_depth(depth)
	, m_rows(rows)
	, m_cols(cols)
{
	const glm::vec4 color(0.65f, 0.65f, 0.65f, 0.25f);

	const float halfWidth = m_width * 0.5f;
	const float halfDepth = m_depth * 0.5f;

	const glm::vec3 gridOrigin = { -halfWidth, 0.0f, -halfDepth };
	const glm::vec3 widthDisplacement(m_width, 0.0f, 0.0f);
	for (int y = 0; y <= m_rows; ++y)
	{
		const glm::vec3 start = gridOrigin + glm::vec3(0.0f, 0.0f, y);
		const glm::vec3 end = start + widthDisplacement;
		m_lines.push_back({ start, end, color });
	}

	const glm::vec3 depthDisplacement(0.0f, 0.0f, m_depth);
	for (int x = 0; x <= m_cols; ++x)
	{
		const glm::vec3 start = gridOrigin + glm::vec3(x, 0.0f, 0.0f);
		const glm::vec3 end = start + depthDisplacement;
		m_lines.push_back({ start, end, color });
	}
}

void ViewportGrid::Draw()
{
	for (const DebugDraw::Line& l : m_lines)
	{
		DebugDraw::AddLine(l.start, l.end, l.col);
	}
}
