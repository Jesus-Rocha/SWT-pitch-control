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

#include "scada.h"

using namespace scada;

StepTimer::StepTimer()
	: m_framesPerSecond(0)
	, m_timeCounter(0)
	, m_targetElapsedTime(1000000 / 60)
	, m_maxDelta(100000)
	, m_lastTime(micros())
	, m_elapsedTime(0)
	, m_totalTime(0)
	, m_leftOverTime(0)
	, m_frameCount(0)
	, m_totalFrameCount(0)
	, m_isFixedTimeStep(false)	
{

}

uint64_t StepTimer::elapsedTime() const
{
	return m_elapsedTime;
}

uint64_t StepTimer::totalTime() const
{
	return m_totalTime;
}

uint StepTimer::frameCount() const
{
	return m_totalFrameCount;
}

double StepTimer::frameRate() const
{
	return m_framesPerSecond;
}

void StepTimer::fixedTimeStep(bool value)
{
	m_isFixedTimeStep = value;
}

void StepTimer::framesPerSecond(uint64_t targetElapsed)
{
	m_targetElapsedTime = 1000000 / targetElapsed;
}

void StepTimer::reset()
{
	m_framesPerSecond = 0;
	m_timeCounter = 0;
	m_elapsedTime = 0;
	m_totalTime = 0;
	m_leftOverTime = 0;
	m_frameCount = 0;
	m_totalFrameCount = 0;
	m_lastTime = micros();
}

void StepTimer::update()
{
	uint64_t currentTime = micros();
	uint64_t timeDelta = currentTime - m_lastTime;

	m_lastTime = currentTime;
	m_timeCounter += timeDelta;

//	if (timeDelta > m_maxDelta)
//		timeDelta = m_maxDelta;

	m_elapsedTime = timeDelta;
	m_totalTime += timeDelta;
	m_leftOverTime = 0;
	m_totalFrameCount++;
	m_frameCount++;
	if (m_frameCount >= m_targetElapsedTime / 1000)
	{
		m_framesPerSecond = 1000000.0 * (double)m_frameCount / (double)m_timeCounter;
		m_timeCounter = 0;
		m_frameCount = 0;
	}
}