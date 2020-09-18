#pragma once

#include <chrono>

using namespace std::chrono;

using TimePrecision = double;

class GameTime
{
public:
	GameTime();

	void Init();
	void Tick();
	TimePrecision GetElapsed() const;
	TimePrecision GetTotalTime() const;
	TimePrecision GetUpdateRate() const { return m_updateRate * m_timeScale; }

	TimePrecision GetTimeScaleMin() const { return m_timeScaleMin; }
	TimePrecision GetTimeScaleMax() const { return m_timeScaleMax; }
	void IncreaseTimeScale();
	void DecreaseTimeScale();
	void SetTimeScale(TimePrecision scale);
	TimePrecision GetTimeScale() const { return m_timeScale; }

	template<typename Unit>
	TimePrecision GetTime(float time) const {
		return std::chrono::duration_cast<Unit<TimePrecision>>(time);
	}

	void TogglePause(bool pause) { m_pause = pause; }
	bool IsPaused() const { return m_pause; }

private:
	high_resolution_clock::time_point m_initialTime{};
	high_resolution_clock::time_point m_previousNow{};
	duration<TimePrecision> m_lastTimeSpan{};

	TimePrecision m_lastFrameTime = 0.0f;
	TimePrecision m_maxFrameTime = 0.25f;

	TimePrecision m_updateRate = 1.0f / 60.0f; // 1 second divided by 60 updates.

	TimePrecision m_timeScale = 1.0f;
	TimePrecision m_timeScaleMin = 0.1f;
	TimePrecision m_timeScaleMax = 5.0f;
	TimePrecision m_timeScaleIncrement = 0.1f;

	TimePrecision m_time = 0.0f;

	bool m_pause = false;
};