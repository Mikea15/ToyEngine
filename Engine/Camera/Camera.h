#pragma once

#include <glm/glm.hpp>

#include "CameraFrustum.h"

class Camera
{
public:
	struct Properties
	{
		float m_fov = 70.0f;
		float m_aspectRatio = 16.0f / 9.0f;
		float m_nearPlane = 0.01f;
		float m_farPlane = 1000.0f;
		bool m_isPerspective = false;

		float m_orthoLeft = 5.0f;
		float m_orthoRight = 5.0f;
		float m_orthoTop = 5.0f;
		float m_orthoBottom = 5.0f;

		bool operator==(const Properties& rhs) 
		{
			if (m_fov != rhs.m_fov) return false;
			if (m_aspectRatio != rhs.m_aspectRatio) return false;
			if (m_nearPlane != rhs.m_nearPlane) return false;
			if (m_farPlane != rhs.m_farPlane) return false;
			if (m_isPerspective != rhs.m_isPerspective) return false;
			if (m_orthoLeft != rhs.m_orthoLeft) return false;
			if (m_orthoRight != rhs.m_orthoRight) return false;
			if (m_orthoTop != rhs.m_orthoTop) return false;
			if (m_orthoBottom != rhs.m_orthoBottom) return false;
			return true;
		}

		bool operator!=(const Properties& rhs) 
		{
			return !(*this == rhs);
		}
	};

	Camera();
	Camera(glm::vec3 position, glm::vec3 forward = glm::vec3(0, 0, -1), glm::vec3 up = glm::vec3(0, 1, 0));

	void Update(float deltaTime);

	void SetPerspective(float fov, float aspect, float near, float far);
	void SetOrthographic(float left, float right, float top, float bottom, float near, float far);

	void UpdateView();

	float FrustumHeightAtDistance(float distance);
	float DistanceAtFrustumHeight(float frustumHeight);

	const glm::vec3& GetPosition() const { return m_position; }
	void SetPosition(const glm::vec3& position);

	const glm::mat4& GetView() const { return m_view; }
	const glm::mat4& GetProjection() const { return m_projection; }
	glm::mat4 GetViewProjection() const { return m_projection * m_view; }

	const glm::vec3& GetUp() const { return m_up; }
	const glm::vec3& GetForward() const { return m_forward; }
	const glm::vec3& GetRight() const { return m_right; }

	CameraFrustum GetFrustum() { return m_frustum; }

	bool IsOrthographic() const { return m_properties.m_isPerspective; }

	const Properties& GetProperties() const { return m_properties; }
	void SetProperties(const Properties& properties);

	float GetFov() const { return m_properties.m_fov; }
	void SetFov(float fov);

	float GetNearPlane() const { return m_properties.m_nearPlane; }
	float GetFarPlane() const { return m_properties.m_farPlane; }
	void SetNearFarPlane(float nearPlane, float farPlane);

	float GetAspectRatio() const { return m_properties.m_aspectRatio; }
	void SetAspectRatio(float ratio);

	glm::vec3 WorldSpaceToScreenSpace(const glm::vec3& worldPosition);
	glm::vec3 ScreenSpaceToWorldSpace(const glm::vec3& screenPosition);

	glm::mat4 m_projection;
	glm::mat4 m_view;
private:
	void UpdateProjection();

protected:

	glm::vec3 m_position = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 m_forward = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 m_up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 m_right = glm::vec3(1.0f, 0.0f, 0.0f);

	CameraFrustum m_frustum;

	Properties m_properties;

	static const float s_minFov;
	static const float s_maxFov;
	static const float s_maxPitchAngle;
};
