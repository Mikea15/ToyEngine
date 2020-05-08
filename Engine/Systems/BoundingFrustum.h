#pragma once

#include "Engine/Vector.h"

#include <GL/glew.h>

#include <vector>

#include "Plane.h"

class AABB;

class BoundingFrustum
{
	enum
	{
		Top = 0,
		Bottom,
		Left,
		Right,
		Near,
		Far
	};

public:
	BoundingFrustum();
	// (view * projection)
	BoundingFrustum(const glm::mat4& viewProjection);

	~BoundingFrustum();

	void Update(const glm::mat4& viewProjection);

	bool Intersects(const AABB& aabb) const;
	bool Intersects(const BoundingFrustum& frustum) const;
	PlaneIntersectionType Intersects(const Plane& plane) const;

	ContainmentType Contains(const glm::vec3& point) const;
	ContainmentType Contains(const AABB& aabb) const;
	ContainmentType Contains(const BoundingFrustum& frustum) const;

	void CreateCorners();
	void CreatePlanes();

	glm::vec3 IntersectionPoint(const Plane& a, const Plane& b, const Plane& c);

	const std::vector<glm::vec3> GetCorners() const { return corners; }

private:
	glm::mat4 viewProj;
	std::vector<glm::vec3> corners;
	std::vector<Plane> planes;
};

