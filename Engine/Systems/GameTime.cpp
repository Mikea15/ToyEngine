
#include "GameTime.h"
#include "Utils/MathUtils.h"

GameTime::GameTime()
{

}

void GameTime::Init()
{
	m_initialTime = high_resolution_clock::now();
	m_previousNow = m_initialTime;
}

void GameTime::Tick()
{
	auto now = high_resolution_clock::now();
	m_lastTimeSpan = duration_cast<duration<TimePrecision>>(now - m_previousNow);
	m_previousNow = now;

	m_time += m_lastTimeSpan.count();
}

TimePrecision GameTime::GetElapsed() const
{
	// cap on how many updates we can do.
	// eg. dt: 1/60 ( 60 ups ).
	// frametime: 0.25
	// max ups = frametime / m_deltaTime
	// max ups = 0.25 / 0.016 = 15.6ups
	// for 30 ups -> max frametime 0.25 ( 7.5 )
	// for 60 ups -> max frametime 0.083 ( 5 updates )
	TimePrecision clampedToBounds = m_lastTimeSpan.count();
	if (clampedToBounds > m_maxFrameTime)
	{
		clampedToBounds = m_maxFrameTime;
	}
	return clampedToBounds * m_timeScale;
}

TimePrecision GameTime::GetTotalTime() const
{
	return duration_cast<duration<TimePrecision>>(m_previousNow - m_initialTime).count();
}

void GameTime::IncreaseTimeScale()
{
	m_timeScale = MathUtils::Clamp(m_timeScale + m_timeScaleIncrement, m_timeScaleMin, m_timeScaleMax);
}

void GameTime::DecreaseTimeScale()
{
	m_timeScale = MathUtils::Clamp(m_timeScale - m_timeScaleIncrement, m_timeScaleMin, m_timeScaleMax);
}

void GameTime::SetTimeScale(TimePrecision scale)
{
	m_timeScale = MathUtils::Clamp(scale, m_timeScaleMin, m_timeScaleMax);
}
