#pragma once

#include <glm/glm.hpp>
#include "BoidManager.h"

struct Boid
{
	struct WeightParams
	{
		float m_alignment = 1.0f;
		float m_cohesion = 1.0f;
		float m_separation = 1.0f;
	};

	Boid()
	{
		m_acceleration = {};
		m_velocity = {};
		m_position = {};

		m_radius = 1.0f;
		m_maxSpeed = 3.0f;
		m_maxForce = 1.0f;
	}

	void Update( float deltaTime )
	{
		m_velocity += m_acceleration * deltaTime;
		m_velocity = glm::clamp(m_velocity, -m_maxSpeed, m_maxSpeed);

		m_position += m_velocity * deltaTime;
		m_acceleration = {};
	}

	void Seek(glm::vec3 target)
	{
		glm::vec3 desired = target - m_position;
		desired = glm::normalize(desired);
		desired *= m_maxSpeed;

		glm::vec3 steer = desired - m_velocity;
		steer = glm::clamp(steer, -m_maxForce, m_maxForce);

		ApplyForce(steer);
	}

	void Arrive(glm::vec3 target)
	{
		glm::vec3 desired = target - m_position;
		float distance = glm::length(desired);
		desired = glm::normalize(desired);

		float speed = m_maxSpeed;
		float arriveRadius = 3.0f;
		if (distance < arriveRadius)
		{
			speed = MathUtils::Lerp(0.0f, speed, distance / arriveRadius);
		}

		desired *= speed;
		glm::vec3 steer = desired - m_velocity;
		steer = glm::clamp(steer, -m_maxForce, m_maxForce);

		ApplyForce(steer);
	}

	void ApplyForce(glm::vec3 force)
	{
		m_acceleration += force;
	}

	glm::vec3 m_position;
	glm::vec3 m_velocity;
	glm::vec3 m_acceleration;

	float m_maxSpeed;
	float m_maxForce;

	float m_radius;
};
