#include "AABB.h"

#include "Plane.h"
#include "BoundingFrustum.h"

AABB::AABB()
	: AABB(glm::vec3(0.0f), 1.0f)
{
}

AABB::AABB(const glm::vec3& origin, float halfSize)
	: m_origin(origin), m_halfSize(halfSize)
{
	m_points.resize(8);
	m_points[0] = m_min = m_origin + glm::vec3(-m_halfSize, -m_halfSize, -m_halfSize);
	m_points[3] = m_origin + glm::vec3(-m_halfSize, -m_halfSize, m_halfSize);
	m_points[1] = m_origin + glm::vec3(-m_halfSize, m_halfSize, -m_halfSize);
	m_points[2] = m_origin + glm::vec3(-m_halfSize, m_halfSize, m_halfSize);
	m_points[5] = m_origin + glm::vec3(m_halfSize, -m_halfSize, -m_halfSize);
	m_points[6] = m_origin + glm::vec3(m_halfSize, -m_halfSize, m_halfSize);
	m_points[4] = m_origin + glm::vec3(m_halfSize, m_halfSize, -m_halfSize);
	m_points[7] = m_max = m_origin + glm::vec3(m_halfSize, m_halfSize, m_halfSize);
}

AABB::AABB(const glm::vec3& min, const glm::vec3& max)
{
	m_origin = min + (max - min) * 0.5f;

	m_points.resize(8);
	m_points[0] = m_min = min;
	m_points[3] = glm::vec3(min.x, min.y, max.z);
	m_points[1] = glm::vec3(min.x, max.y, min.z);
	m_points[2] = glm::vec3(min.x, max.y, max.z);
	m_points[5] = glm::vec3(max.x, min.y, min.z);
	m_points[6] = glm::vec3(max.x, min.y, max.z);
	m_points[4] = glm::vec3(max.x, max.y, min.z);
	m_points[7] = m_max = max;

	m_halfSize = GetWidth() * 0.5f;
}

AABB::AABB(const glm::vec3& origin, const glm::vec3& min, const glm::vec3& max)
{
	m_points.resize(8);
	m_points[0] = m_min = origin + min;
	m_points[3] = m_origin + glm::vec3(min.x, min.y, max.z);
	m_points[1] = m_origin + glm::vec3(min.x, max.y, min.z);
	m_points[2] = m_origin + glm::vec3(min.x, max.y, max.z);
	m_points[5] = m_origin + glm::vec3(max.x, min.y, min.z);
	m_points[6] = m_origin + glm::vec3(max.x, min.y, max.z);
	m_points[4] = m_origin + glm::vec3(max.x, max.y, min.z);
	m_points[7] = m_max = origin + max;

	m_halfSize = GetWidth() * 0.5f;
}

AABB::~AABB()
{
	m_points.clear();
}

void AABB::SetBounds(glm::vec3 min, glm::vec3 max)
{
	m_points[0] = m_min = m_origin + min;
	m_points[3] = m_origin + glm::vec3(min.x, min.y, max.z);
	m_points[1] = m_origin + glm::vec3(min.x, max.y, min.z);
	m_points[2] = m_origin + glm::vec3(min.x, max.y, max.z);
	m_points[5] = m_origin + glm::vec3(max.x, min.y, min.z);
	m_points[6] = m_origin + glm::vec3(max.x, min.y, max.z);
	m_points[4] = m_origin + glm::vec3(max.x, max.y, min.z);
	m_points[7] = m_max = m_origin + max;

	m_halfSize = GetWidth() * 0.5f;
}

ContainmentType AABB::Contains(const AABB& box) const
{
	if (box.m_max.x < m_min.x || box.m_min.x > m_max.x
		|| box.m_max.y < m_min.y || box.m_min.y > m_max.y
		|| box.m_max.z < m_min.z || box.m_min.z > m_max.z)
	{
		return ContainmentType::Disjoint;
	}

	if (box.m_min.x >= m_min.x && box.m_max.x <= m_max.x
		&& box.m_min.y >= m_min.y && box.m_max.y <= m_max.y
		&& box.m_min.z >= m_min.z && box.m_max.z <= m_max.z)
	{
		return ContainmentType::Contains;
	}

	return ContainmentType::Intersects;
}

ContainmentType AABB::Contains(const BoundingFrustum& frustum) const
{
	//Because question is not frustum contain box but reverse and this is not the same
	unsigned int i;

	const std::vector<glm::vec3> corners = frustum.GetCorners();
	const unsigned int cornersSize = static_cast<unsigned int>(corners.size());
	// First we check if frustum is in box
	for (i = 0; i < cornersSize; ++i)
	{
		if (Contains(corners[i]) == ContainmentType::Disjoint)
		{
			break;
		}
	}

	if (i == cornersSize)
	{
		// This means we checked all the corners and they were all contain or instersect
		return ContainmentType::Contains;
	}

	if (i != 0)
	{
		// if i is not equal to zero, we can fastpath and say that this box intersects
		return ContainmentType::Intersects;
	}

	// If we get here, it means the first (and only) point we checked was actually contained in the frustum.
	// So we assume that all other points will also be contained. If one of the points is disjoint, we can
	// exit immediately saying that the result is Intersects
	i++;
	for (; i < cornersSize; ++i)
	{
		if (Contains(corners[i]) != ContainmentType::Disjoint)
		{
			return ContainmentType::Intersects;
		}
	}

	// If we get here, then we know all the points were actually contained, therefore result is Contains
	return ContainmentType::Contains;
}


ContainmentType AABB::Contains(const glm::vec3& point) const
{
	// first we get if point is out of box
	if (point.x < m_min.x || point.x > m_max.x
		|| point.y < m_min.y || point.y > m_max.y
		|| point.z < m_min.z || point.z > m_max.z)
	{
		return ContainmentType::Disjoint;
	}
	return ContainmentType::Contains;
}

//bool AABB::Contains(const glm::vec3& point) const
//{
//	if (point.x < m_min.x || point.x > m_max.x) return false;
//	if (point.y < m_min.y || point.y > m_max.y) return false;
//	if (point.z < m_min.z || point.z > m_max.z) return false;
//
//	return true;
//}

//bool AABB::Contains(const AABB& aabb) const
//{
//	if (aabb.m_max.x < m_min.x || aabb.m_min.x > m_max.x) return false;
//	if (aabb.m_max.y < m_min.y || aabb.m_min.y > m_max.y) return false;
//	if (aabb.m_max.z < m_min.z || aabb.m_min.z > m_max.z) return false;
//
//	return true;
//}

bool AABB::Intersects(const glm::vec3& point, float r) const
{
	if (point.x - m_min.x > r &&
		point.y - m_min.y > r &&
		point.z - m_min.z > r &&
		m_max.x - point.x > r &&
		m_max.y - point.y > r &&
		m_max.z - point.z > r)
	{
		return true;
	}

	float dMin = 0.0f;
	if (point.x - m_min.x <= r)
	{
		dMin += (point.x - m_min.x) * (point.x - m_min.x);
	}
	else if (m_max.x - point.x <= r)
	{
		dMin += (point.x - m_max.x) * (point.x - m_max.x);
	}

	if (point.y - m_min.y <= r)
	{
		dMin += (point.y - m_min.y) * (point.y - m_min.y);
	}
	else if (m_max.y - point.y <= r)
	{
		dMin += (point.y - m_max.y) * (point.y - m_max.y);
	}

	if (point.z - m_min.z <= r)
	{
		dMin += (point.z - m_min.z) * (point.z - m_min.z);
	}
	else if (m_max.z - point.z <= r)
	{
		dMin += (point.z - m_max.z) * (point.z - m_max.z);
	}

	if (dMin <= r * r)
	{
		return true;
	}

	return false;
}

bool AABB::Intersects(const AABB& aabb) const
{
	if (m_max.x >= aabb.m_min.x && m_min.x <= aabb.m_max.x)
	{
		if (m_max.y >= aabb.m_min.y && m_min.y <= aabb.m_max.y)
		{
			return false;
		}
		return (m_max.z >= aabb.m_min.z) && (m_min.z <= aabb.m_max.z);
	}
	return false;
}

bool AABB::Intersects(const BoundingFrustum& frustum) const
{
	return frustum.Intersects(*this);
}

PlaneIntersectionType AABB::Intersects(const Plane& plane) const
{
	glm::vec3 pos;
	glm::vec3 neg;
	if (plane.normal.x >= 0)
	{
		pos.x = m_max.x;
		neg.x = m_min.x;
	}
	else
	{
		pos.x = m_min.x;
		neg.x = m_max.x;
	}

	if (plane.normal.y >= 0)
	{
		pos.y = m_max.y;
		neg.y = m_min.y;
	}
	else
	{
		pos.y = m_min.y;
		neg.y = m_max.y;
	}

	if (plane.normal.z >= 0)
	{
		pos.z = m_max.z;
		neg.z = m_min.z;
	}
	else
	{
		pos.z = m_min.z;
		neg.z = m_max.z;
	}

	float distance = plane.normal.x * neg.x + plane.normal.y * neg.y + plane.normal.z * neg.z + plane.d;
	if (distance > 0)
	{
		return PlaneIntersectionType::Front;
	}

	distance = plane.normal.x * pos.x + plane.normal.y * pos.y + plane.normal.z * pos.z + plane.d;
	if (distance < 0)
	{
		return PlaneIntersectionType::Back;
	}

	return PlaneIntersectionType::Intersecting;
}

const float AABB::Area() const
{
	glm::vec3 d = m_max - m_min;
	return 2.0f * (d.x * d.y + d.y * d.z + d.z * d.x);
}

AABB AABB::Union(const AABB& a, const AABB& b)
{	
#if 0
	return AABB(
		Min2(a.GetMin(), b.GetMin()),
		Max2(a.GetMax(), b.GetMax())
	);
#else
	return AABB(
		glm::min(a.GetMin(), b.GetMin()),
		glm::max(a.GetMax(), b.GetMax())
	);
#endif
}

