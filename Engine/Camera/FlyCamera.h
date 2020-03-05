#pragma once

#include "Camera.h"

#include <glm/glm.hpp>

/*

  Derivation of the base camera with support for fly-through motions. Think of WASD forward/right
  type of movement, combined with strafing and free yaw/pitch camera rotation.

*/
class FlyCamera 
	: public Camera
{
public:
	FlyCamera(glm::vec3 position, glm::vec3 forward = glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f));

	void Update(float deltaTime);

	void HandleMove(float deltaTime, glm::vec3 moveInput, bool boostSpeed);
	void HandleMouse(float deltaX, float deltaY);
	void HandleScroll(float deltaX);

private:
	float m_yaw;
	float m_pitch;
	
	float m_targetYaw;
	float m_targetPitch;
	
	float m_speed = 25.0f;
	float m_boostSpeedFactor = 2.0f;
	float m_mouseSensitivity = 0.1f;
	float m_damping = 5.0f;

	glm::vec3 m_worldUp;
	glm::vec3 m_targetPosition;
};

