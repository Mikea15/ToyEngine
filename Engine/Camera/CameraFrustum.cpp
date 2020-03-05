#include "CameraFrustum.h"

#include "Camera.h"



void CameraFrustum::Update(Camera* camera)
{
	float tan = 2.0f * std::tan(camera->GetFov() * 0.5f);
	const float nearPlane = camera->GetNearPlane();
	const float farPlane = camera->GetFarPlane();

	float nearHeight = tan * nearPlane;
	const float nearHalfHeight = nearHeight * 0.5f;

	float nearWidth = nearHeight * camera->GetAspectRatio();
	const float nearHalfWidth = nearWidth * 0.5f;

	float farHeight = tan * farPlane;
	float farHalfHeight = farHeight * 0.5f;
	float farWidth = farHeight * camera->GetAspectRatio();
	float farHalfWidth = farWidth * 0.5f;

	const glm::vec3 fwd = camera->GetForward();
	const glm::vec3 up = camera->GetUp();
	const glm::vec3 right = camera->GetRight();

	const glm::vec3 nearCenter = camera->GetPosition() + camera->GetForward() * nearPlane;
	const glm::vec3 farCenter = camera->GetPosition() + camera->GetForward() * farPlane;

	glm::vec3 v;
	// left plane
	v = (nearCenter - right * nearHalfWidth) - camera->GetPosition();
	Left.SetNormalD(glm::cross(glm::normalize(v), up), nearCenter - right * nearHalfWidth);
	// right plane
	v = (nearCenter + right * nearHalfWidth) - camera->GetPosition();
	Right.SetNormalD(glm::cross(up, glm::normalize(v)), nearCenter + right * nearHalfWidth);
	// top plane
	v = (nearCenter + up * nearHeight) - camera->GetPosition();
	Top.SetNormalD(glm::cross(glm::normalize(v), right), nearCenter + up * nearHeight);
	// bottom plane
	v = (nearCenter - up * nearHeight) - camera->GetPosition();
	Bottom.SetNormalD(glm::cross(right, glm::normalize(v)), nearCenter - up * nearHeight);
	// near plane
	Near.SetNormalD(fwd, nearCenter);
	// far plane
	Far.SetNormalD(-fwd, farCenter);

	NTL = nearCenter + up * nearHalfHeight - right * nearHalfWidth;
	NTR = nearCenter + up * nearHalfHeight + right * nearHalfWidth;
	NBL = nearCenter - up * nearHalfHeight - right * nearHalfWidth;
	NBR = nearCenter - up * nearHalfHeight + right * nearHalfWidth;

	FTL = farCenter + up * farHalfHeight - right * farHalfWidth;
	FTR = farCenter + up * farHalfHeight + right * farHalfWidth;
	FBL = farCenter - up * farHalfHeight - right * farHalfWidth;
	FBR = farCenter - up * farHalfHeight + right * farHalfWidth;
}

bool CameraFrustum::Intersect(glm::vec3 point)
{
	for (int i = 0; i < 6; ++i)
	{
		if (Planes[i].Distance(point) < 0)
		{
			return false;
		}
	}
	return true;
}

bool CameraFrustum::Intersect(glm::vec3 point, float radius)
{
	for (int i = 0; i < 6; ++i)
	{
		if (Planes[i].Distance(point) < -radius)
		{
			return false;
		}
	}
	return true;
}

bool CameraFrustum::Intersect(glm::vec3 boxMin, glm::vec3 boxMax)
{
	for (int i = 0; i < 6; ++i)
	{
		glm::vec3 positive = boxMin;
		if (Planes[i].Normal.x >= 0)
		{
			positive.x = boxMax.x;
		}
		if (Planes[i].Normal.y >= 0)
		{
			positive.y = boxMax.y;
		}
		if (Planes[i].Normal.z >= 0)
		{
			positive.z = boxMax.z;
		}
		if (Planes[i].Distance(positive) < 0)
		{
			return false;
		}
	}
	return true;
}

void CameraFrustum::CreateCorners()
{
	NTL = IntersectionPoint(Near, Left, Top); // Near, Left, Top
	NTR = IntersectionPoint(Near, Right, Top); // Near, Right, Top
	NBL = IntersectionPoint(Near, Left, Bottom); // Near, Left, Bottom
	NBR = IntersectionPoint(Near, Right, Bottom); // Near, Right, Bottom

	FTL = IntersectionPoint(Far, Left, Top); // Far, Left, Top
	FTR = IntersectionPoint(Far, Right, Top); // Far, Right, Top
	FBL = IntersectionPoint(Far, Right, Bottom); // Far, Right, Bottom
	FBR = IntersectionPoint(Far, Left, Bottom); // Far, Left, Bottom
}

glm::vec3 CameraFrustum::IntersectionPoint(const FrustumPlane& a, const FrustumPlane& b, const FrustumPlane& c) const
{
	glm::vec3 bCrossC = glm::cross(b.Normal, c.Normal);
	const float f = -glm::dot(a.Normal, bCrossC);
	glm::vec3 v1 = bCrossC * a.D;

	glm::vec3 cCrossA = glm::cross(c.Normal, a.Normal);
	glm::vec3 v2 = cCrossA * b.D;

	glm::vec3 aCrossB = glm::cross(a.Normal, b.Normal);
	glm::vec3 v3 = aCrossB * c.D;

	return {
		(v1.x + v2.x + v3.x) / f,
		(v1.y + v2.y + v3.y) / f,
		(v1.z + v2.z + v3.z) / f
	};
}

