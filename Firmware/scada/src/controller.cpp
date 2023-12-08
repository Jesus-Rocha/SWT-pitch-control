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



//////////////////////////////////////////////////////////////////
Controller::Controller()
{

}

//////////////////////////////////////////////////////////////////
ObserverController::ObserverController()
    : m_running(false)
    , m_output(0)
{

}

ControllerType ObserverController::getType()const
{
    return ControllerType::NONE;
}

double ObserverController::getOutput()const
{
    return m_output;
}

double ObserverController::getState(uint32_t index)const
{
    return 0;
}

uint32_t ObserverController::getParamCount()const
{
    return 0;
}

double ObserverController::getParam(uint32_t index)const
{
    return 0;
}

void ObserverController::setParam(uint32_t index, double value)
{
}

void ObserverController::start()
{
    if(m_running)
        return;
    m_running = true;
}

double ObserverController::update(double, double ref, array_view<Input> const&)
{
    return m_output = 0;
}

void ObserverController::stop()
{
    m_running = false;
    m_output = 0;
}

//////////////////////////////////////////////////////////////////
OpenLoopController::OpenLoopController()
    : m_running(false)
    , m_output(0)
{

}

ControllerType OpenLoopController::getType()const
{
    return ControllerType::OPEN_LOOP;
}

double OpenLoopController::getOutput()const
{
    return m_output;
}

double OpenLoopController::getState(uint32_t index)const
{
    return 0;
}

uint32_t OpenLoopController::getParamCount()const
{
    return 1;
}

double OpenLoopController::getParam(uint32_t index)const
{
    switch (index)
    {
    default:
        return 0;
    }
}

void OpenLoopController::setParam(uint32_t index, double value)
{
    switch (index)
    {
    default:
        break;
    }
}

void OpenLoopController::start()
{
    if(m_running)
        return;
    m_running = true;
}

double OpenLoopController::update(double, double yr, array_view<Input> const&)
{
    return m_output = max(min(yr, 5.0), 0.0);
}

void OpenLoopController::stop()
{
    m_running = false;
    m_output = 0;
}

//////////////////////////////////////////////////////////////////
PIDController::PIDController()
    : m_running(false)
    , m_output(0)
    , m_K1(0.1)
    , m_K2(0.01)
    , m_K3(0)
    , m_sigma(0)
    , m_e1(0)
    , m_yr(0)
    , m_dyr(0)
    , m_reference(0)
{

}

ControllerType PIDController::getType()const
{
    return ControllerType::PID;
}

double PIDController::getOutput()const
{
    return m_output;
}

double PIDController::getState(uint32_t index)const
{
    return 0;
}

uint32_t PIDController::getParamCount()const
{
    return 1;
}

double PIDController::getParam(uint32_t index)const
{
    switch (index)
    {
    case 0:
        return m_K1;
    case 1:
        return m_K2;
    case 2:
        return m_K3;
    default:
        return 0;
    }
}

void PIDController::setParam(uint32_t index, double value)
{
    switch (index)
    {
    case 0:
        m_K1 = abs(value);//positive values only
        break;
    case 1:
        m_K2 = abs(value);//positive values only
        break;
    case 2:
        m_K3 = abs(value);//positive values only
        break;
    default:
        break;
    }
}

void PIDController::start()
{
    if(m_running)
        return;
    m_running = true;
    m_sigma = 0;
    m_e1 = 0;
    m_yr = 0;
    m_dyr = 0;
    m_reference = 0;
}

double PIDController::update(double dt, double yr, array_view<Input> const& ys)
{
    static constexpr double a = 0.705261382745352;
    static constexpr double b = 3.222119974954683;
    static constexpr double c = 1.956276376964970;
    static constexpr double _delay = 10.0;

    m_reference =  (m_reference < yr)? m_reference + dt/_delay :
        (m_reference > yr)? m_reference - dt/_delay : m_reference;

    double dyr;
    double d2yr;
    double e2;
    double e1 = ys.data()[0] - m_reference;

    dyr = (m_reference - m_yr) / dt;
    d2yr = (dyr - m_dyr) / dt;

    m_sigma = m_sigma + e1 * dt;
    e2 = (e1 - m_e1) / dt;

    m_e1 = e1;
    m_yr = m_reference;
    m_dyr = dyr;

    m_output = (a*m_reference + b*dyr + d2yr - m_K1*m_sigma - m_K2*e1 - m_K3*e2) / c;

    return m_output = max(min(m_output, 5.0), 0.0);
}

void PIDController::stop()
{
    m_running = false;
    m_output = 0;
}
