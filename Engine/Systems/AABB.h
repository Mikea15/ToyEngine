#pragma once

#include <vector>


#include <glm/glm.hpp>
#include "GeomDefines.h"

class Plane;
class BoundingFrustum;

class AABB
{
public:
	AABB();
	AABB(const glm::vec3& origin, float halfSize);
	AABB(const glm::vec3& min, const glm::vec3& max);
	AABB(const glm::vec3& origin, const glm::vec3& min, const glm::vec3& max);
	~AABB();

	void SetBounds(glm::vec3 min, glm::vec3 max);

	ContainmentType Contains(const AABB& box) const;
	ContainmentType Contains(const BoundingFrustum& frustum) const;
	ContainmentType Contains(const glm::vec3& point) const;

	//! Intersects BoundingSphere
	bool Intersects(const glm::vec3& point, float r) const;
	bool Intersects(const AABB& aabb) const;
	bool Intersects(const BoundingFrustum& frustum) const;
	PlaneIntersectionType Intersects(const Plane& plane) const;

	const glm::vec3& GetPosition() const { return m_origin; }
	const float GetSize() const { return m_halfSize; }
	const float GetWidth() const { return fabsf(m_max.x - m_min.x); }

	const glm::vec3 GetMin() const { return m_min; }
	const glm::vec3 GetMax() const { return m_max; }

	const float Area() const;

	static AABB Union(const AABB& a, const AABB& b);

	static glm::vec3 Min2(glm::vec3 a, glm::vec3 b)
	{
		glm::vec3 r;
		r.x = a.x < b.x ? a.x : b.x;
		r.y = a.y < b.y ? a.y : b.y;
		r.x = a.z < b.z ? a.z : b.z;
		return r;
	}

	static glm::vec3 Max2(glm::vec3 a, glm::vec3 b)
	{
		glm::vec3 r;
		r.x = a.x > b.x ? a.x : b.x;
		r.y = a.y > b.y ? a.y : b.y;
		r.x = a.z > b.z ? a.z : b.z;
		return r;
	}

private:
	float m_halfSize;
	glm::vec3 m_origin;
	glm::vec3 m_min;
	glm::vec3 m_max;

	std::vector<glm::vec3> m_points;
};