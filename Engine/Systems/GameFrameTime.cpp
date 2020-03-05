#include "GameFrameTime.h"

GameFrameTime::GameFrameTime()
{

}

void GameFrameTime::Init()
{
	m_currentTime = high_resolution_clock::now();
}

float GameFrameTime::GetFrameTime()
{
	auto now = high_resolution_clock::now();
	auto timespan = duration_cast<duration<double>>(now - m_currentTime);
	m_currentTime = now;
	return static_cast<float>(timespan.count());
}
