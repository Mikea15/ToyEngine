#pragma once

#include <glm/glm.hpp>


class Camera;

struct FrustumPlane
{
	FrustumPlane(float a, float b, float c, float d)
		: Normal(glm::vec3(a, b, c))
		, D(d)
	{}

	void SetNormalD(glm::vec3 normal, glm::vec3 point)
	{
		Normal = glm::normalize(normal);
		D = -glm::dot(Normal, point);
	}

	float Distance(glm::vec3 point)
	{
		return glm::dot(Normal, point) + D;
	}

	void Normalize()
	{
		const float length = glm::length(Normal);
		if (length > 0.0f)
		{
			const float factor = 1.0f / length;
			Normal = Normal * factor;
			D = D * factor;
		}
	}

	glm::vec3 Normal;
	float D;
};

class CameraFrustum
{
public:
	union
	{
		FrustumPlane Planes[6];
		struct
		{
			FrustumPlane Left;
			FrustumPlane Right;
			FrustumPlane Top;
			FrustumPlane Bottom;
			FrustumPlane Near;
			FrustumPlane Far;
		};
	};

	union
	{
		glm::vec3 Corners[8];
		struct 
		{
			glm::vec3 NTL;
			glm::vec3 NTR;
			glm::vec3 NBL;
			glm::vec3 NBR;
			glm::vec3 FTL;
			glm::vec3 FTR;
			glm::vec3 FBL;
			glm::vec3 FBR;
		};
	};

public:
	CameraFrustum() { } // NOTE(Joey): why do I need to define a constructor here? (otherwise I get deleted constructor error) LOOK IT UP!

	void Update(Camera* camera);

	bool Intersect(glm::vec3 point);
	bool Intersect(glm::vec3 point, float radius);
	bool Intersect(glm::vec3 boxMin, glm::vec3 boxMax);

private:
	void CreateCorners();
	glm::vec3 IntersectionPoint(const FrustumPlane& a, const FrustumPlane& b, const FrustumPlane& c) const;

};

