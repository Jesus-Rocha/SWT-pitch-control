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
Fuzzy::Fuzzy()
    : m_output(0)
    , m_running(false)
{

}

ControllerType Fuzzy::getType()const
{
    return ControllerType::OPEN_LOOP;
}

double Fuzzy::getOutput()const
{
    return m_output;
}

double Fuzzy::getState(uint32_t index)const
{
    return 0;
}

uint32_t Fuzzy::getParamCount()const
{
    return 1;
}

double Fuzzy::getParam(uint32_t index)const
{
    switch (index)
    {
    default:
        return 0;
    }
}

void Fuzzy::setParam(uint32_t index, double value)
{
    switch (index)
    {
    default:
        break;
    }
}

void Fuzzy::start()
{
    if(m_running)
        return;
    m_running = true;
}

double Fuzzy::update(double, double yr, array_view<Input> const&)
{
    return m_output = max(min(yr, 5.0), 0.0);
}

void Fuzzy::stop()
{
    m_running = false;
    m_output = 0;
}