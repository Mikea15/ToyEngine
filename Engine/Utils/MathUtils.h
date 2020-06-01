#pragma once

#include <glm/gtx/norm.hpp>

namespace MathUtils
{
	template<typename T = float>
	static T Clamp(T v, T min, T max)
	{
        if (v > max) { v = max; }
        if (v < min) { v = min; }
		return v;
	}

	template<typename T = float>
	static T Lerp(T a, T b, T t)
	{
		return a + t * (b - a);
	}

	template<typename T = float>
	static T InverseLerp(T a, T b, T v)
	{
		assert(abs(b - a) > FLT_EPSILON);
		return (v - a) / (b - a);
	}

	template<typename T = float>
	static T Remap(T minA, T maxA, T minB, T maxB, T value)
	{
		const T t = InverseLerp(minA, maxA, value);
		return Lerp(minB, maxB, t);
	}

	static float Rand01()
	{
		return static_cast<float>(rand()) / static_cast <float> (RAND_MAX);
	}

	static glm::vec3 RandomInUnitSphere()
	{
		glm::vec3 p;
		do {
			p = 2.0f * glm::vec3(
				MathUtils::Rand01(), 
				MathUtils::Rand01(), 
				MathUtils::Rand01()
			) - glm::vec3(1.0f, 1.0f, 1.0f);
		} while (glm::length2(p) >= 1.0f);
		return p;
	}

	static glm::vec3 RandomInUnitDisk()
	{
		glm::vec3 p;
		do {
			p = 2.0f * glm::vec3(
				MathUtils::Rand01(), 
				MathUtils::Rand01(), 
				0
			) - glm::vec3(1.0f, 1.0f, 0.0f);
		} while ( glm::dot(p, p) >= 1.0f);
		return p;
	}

	static glm::vec3 RandomInUnitHemisphere(const glm::vec3& direction)
	{
		glm::vec3 p = RandomInUnitSphere();
		if (glm::dot(p, direction) <= 0.0f)
		{
			p -= p;
		}
		return p;
	}

	static float Rand(float min, float max)
	{
		return min + static_cast<float>(rand()) / (static_cast <float> (RAND_MAX / (max - min)));
	}

	static int Rand(int min, int max)
	{
		return min + static_cast<int>(rand() / (static_cast <float> (RAND_MAX / (max - min))));
	}
}