/* 
 * This file is part of the SWT-pitch-control distribution (https://github.com/Jesus-Rocha/SWT-pitch-control).
 * Copyright (c) 2023 Jesus Angel Rocha Morales.
 * 
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

namespace scada
{
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

		template<typename T, typename...Ts>
		inline void tick(const T& func, Ts&&...args) {
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
					func(std::forward<Ts>(args)...);
				}
			}
			else {
				m_elapsedTime = timeDelta;
				m_totalTime += timeDelta;
				m_leftOverTime = 0;
				m_totalFrameCount++;
				func(std::forward<Ts>(args)...);
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
}