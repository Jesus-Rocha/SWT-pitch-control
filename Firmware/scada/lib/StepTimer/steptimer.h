#pragma once
#include <Arduino.h>
#include <inttypes.h>

class StepTimer
{
public:
	StepTimer();

public: /*properties*/
	uint64_t elapsedTime() const;
	uint64_t totalTime() const;
	uint32_t frameCount() const;
	double frameRate() const;
	void fixedTimeStep(bool value);
	void framesPerSecond(uint64_t targetElapsed);
	
public: /*methods*/
	void reset();
	void update();

    template<typename T>
    inline void tick(const T& func) {
    	uint64_t currentTime = micros();
		uint64_t timeDelta = currentTime - m_lastTime;
		m_lastTime = currentTime;
		m_timeCounter += timeDelta;
		if (timeDelta > m_maxDelta) {
			timeDelta = m_maxDelta;
		}
		uint32_t lastFrameCount = m_totalFrameCount;
		if (m_isFixedTimeStep) {
			if (abs(static_cast<int64_t>(timeDelta - m_targetElapsedTime)) < 1250) {
				timeDelta = m_targetElapsedTime;
			}
			m_leftOverTime += timeDelta;
			while (m_leftOverTime >= m_targetElapsedTime) {
				m_elapsedTime = m_targetElapsedTime;
				m_totalTime += m_targetElapsedTime;
				m_leftOverTime -= m_targetElapsedTime;
				m_totalFrameCount++;
				func();
			}
		}
		else {
			m_elapsedTime = timeDelta;
			m_totalTime += timeDelta;
			m_leftOverTime = 0;
			m_totalFrameCount++;
			func();
		}
		m_frameCount += m_totalFrameCount - lastFrameCount;
		if (m_frameCount >= m_targetElapsedTime / 1000) {
			m_framesPerSecond = 1000000.0 * (double)m_frameCount / (double)m_timeCounter;
			m_timeCounter = 0;
			m_frameCount = 0;
		}
	}

private:
   	double m_framesPerSecond;
	uint64_t m_timeCounter;
	uint64_t m_targetElapsedTime;
	uint64_t m_maxDelta;
	uint64_t m_lastTime;
	uint64_t m_elapsedTime;
	uint64_t m_totalTime;
	uint64_t m_leftOverTime;
	uint32_t m_frameCount;
	uint32_t m_totalFrameCount;
	bool m_isFixedTimeStep;
};