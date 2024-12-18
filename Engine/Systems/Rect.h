#pragma once

#include <glm/glm.hpp>

class Rect
{
public:
	Rect();
	Rect(const glm::vec2& origin, float halfSize);
	~Rect();

	bool Intersect(const glm::vec2& point) const;
	bool Intersect(const Rect& rect) const;

	const glm::vec2& GetPosition() const { return m_origin; }
	const float GetHalfSize() const { return m_halfSize; }
	const float GetExtent() const { return fabsf(m_max.x - m_min.x); }

	const glm::vec2& GetMin() const { return m_min; }
	const glm::vec2& GetMax() const { return m_max; }

private:
	float m_halfSize;
	glm::vec2 m_origin;
	glm::vec2 m_min;
	glm::vec2 m_max;
};

