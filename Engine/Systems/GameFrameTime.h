#pragma once

#include <chrono>

using namespace std::chrono;

class GameFrameTime
{
public:
	GameFrameTime();

	void Init();
	float GetFrameTime();

private:
	high_resolution_clock::time_point m_currentTime{};
};