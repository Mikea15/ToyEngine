#include "FlyCamera.h"

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/compatibility.hpp>


FlyCamera::FlyCamera(glm::vec3 position, glm::vec3 forward, glm::vec3 up) 
	: Camera(position, forward, up)
	, m_yaw(0.0f)
	, m_pitch(0.0f)
	, m_targetPitch(0.0f)
	, m_targetYaw(-90.0f)
{
	m_worldUp = up;
	m_targetPosition = position;
}

void FlyCamera::Update(float deltaTime)
{
	Camera::Update(deltaTime);

	const float dampingFactor = m_damping * deltaTime;
	m_position = glm::lerp(m_position, m_targetPosition, glm::clamp(dampingFactor, 0.0f, 1.0f));
	m_yaw = glm::lerp(m_yaw, m_targetYaw, glm::clamp(dampingFactor * 2.0f, 0.0f, 1.0f));
	m_pitch = glm::lerp(m_pitch, m_targetPitch, glm::clamp(dampingFactor * 2.0f, 0.0f, 1.0f));

	static constexpr float radian = glm::radians(1.0f);
	glm::vec3 forward(
		cos(radian * m_pitch) * cos(radian * m_yaw),
		sin(radian * m_pitch),
		cos(radian * m_pitch) * sin(radian * m_yaw)
	);

	m_forward = glm::normalize(forward);
	m_right = glm::normalize(glm::cross(m_forward, m_worldUp));
	m_up = glm::cross(m_right, m_forward);
	
	UpdateView();
}

void FlyCamera::HandleMove(float deltaTime, glm::vec3 moveInput, bool boostSpeed)
{
	const float velocity = m_speed * (boostSpeed ? m_boostSpeedFactor : 1.0f) * deltaTime;
	const glm::vec3 movement = m_right * moveInput.x 
		+ m_worldUp * moveInput.y 
		+ m_forward * moveInput.z;

	m_targetPosition = m_targetPosition + movement * velocity;
}

void FlyCamera::HandleMouse(float deltaX, float deltaY)
{
	m_targetYaw += deltaX * m_mouseSensitivity;
	m_targetPitch += deltaY * m_mouseSensitivity;

	if (m_targetYaw == 0.0f) { m_targetYaw = 0.01f; }
	if (m_targetPitch == 0.0f) { m_targetPitch = 0.01f; }

	if (m_targetPitch > s_maxPitchAngle)
	{
		m_targetPitch = s_maxPitchAngle;
	}

	if (m_targetPitch < -s_maxPitchAngle)
	{
		m_targetPitch = -s_maxPitchAngle;
	}
}

void FlyCamera::HandleScroll(float delta)
{
	m_properties.m_fov += delta;
	if (m_properties.m_fov < s_minFov)
	{
		m_properties.m_fov = s_minFov;
	}
	else if (m_properties.m_fov > s_maxFov)
	{
		m_properties.m_fov = s_maxFov;
	}

}
