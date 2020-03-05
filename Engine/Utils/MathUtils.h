#pragma once

namespace MathUtils
{
	static float Lerp(float a, float b, float t)
	{
		return a + t * (b - a);
	}

	static float InverseLerp(float a, float b, float v)
	{
		assert(abs(b - a) > FLT_EPSILON);
		return (v - a) / (b - a);
	}

	static float Remap(float minA, float maxA, float minB, float maxB, float value)
	{
		float t = InverseLerp(minA, maxA, value);
		return Lerp(minB, maxB, t);
	}

	static float Rand01()
	{
		return static_cast<float>(rand()) / static_cast <float> (RAND_MAX);
	}

	static float Rand(float min, float max)
	{
		return min + static_cast<float>(rand()) / (static_cast <float> (RAND_MAX / (max - min)));
	}

	static float Rand(int min, int max)
	{
		return min + static_cast<int>(rand()) / (static_cast <float> (RAND_MAX / (max - min)));
	}
}