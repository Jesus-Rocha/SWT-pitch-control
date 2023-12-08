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

void IRAM_ATTR Inputs::OnInputReadyISR(void *arg)
{
    reinterpret_cast<Inputs*>(arg)->m_inputReady = true;
}

void IRAM_ATTR Inputs::OnRPMPulseISR(void *arg)
{
    reinterpret_cast<Inputs*>(arg)->m_rpmPulseReady = true;
}

Inputs::Inputs()
    : m_ads()
    , m_state(STARTING)
    , m_calibratingCount(0)
    , m_currentADCInput(0)
    , m_sensorBufferIndex(0)
    , m_rpmBufferIndex(0)
    , m_sensorBuffer()
    , m_sensors{{0, 1, 0}, {0, 1, 0}}
    , m_RPM(0, 1, 0)
    , m_lastTime(0)
    , m_inputReady(false)
    , m_rpmPulseReady(false)
    , m_inputsReady(false)
{
    for(auto& value : m_averageRawSensor)
        value = 0;

     for(auto& sensors : m_sensorBuffer)
        for(auto& value : sensors.data)
            value = 0;
}

void Inputs::init()
{
    if(m_state == STARTING)
    {
        m_state = READY;
        m_ads.begin(SPI_CS_PIN, INPUT_READY_PIN);
        m_ads.set_conv_mode_single_shot();
        m_ads.set_data_rate(DR_1000SPS);
        m_ads.set_pga_gain(PGA_GAIN_1);
        m_ads.set_OperationMode(MODE_TURBO);

        attachInterruptArg(digitalPinToInterrupt(INPUT_READY_PIN), &Inputs::OnInputReadyISR, this, FALLING);
        attachInterruptArg(digitalPinToInterrupt(RPM_PULSE_PIN), &Inputs::OnRPMPulseISR, this, FALLING);
        m_lastTime = micros() / 1000000.0;
    }
}

bool Inputs::start()
{
    if(m_state == CALIBRATING || m_state == READY)
    {
        if(m_state == READY)
            m_state = WORKING;
        m_inputsReady = false;
        m_ads.select_mux_channels(ADS1220_MUX[m_currentADCInput]);
        m_ads.Start_Conv();
        return true;
    }
    return false;
}

bool Inputs::calibrating()
{
    if(m_state != READY)
        return false;
    m_state = CALIBRATING;
    m_calibratingCount = 0;
    m_inputsReady = false;
    m_ads.select_mux_channels(ADS1220_MUX[m_currentADCInput]);
    m_ads.Start_Conv();
    return true;
}

bool Inputs::update()
{
    if(m_rpmPulseReady)
    {
        m_rpmPulseReady = false;
        double currentTime = micros() / 1000000.0;
        double delta = currentTime - m_lastTime;
        if(delta > 0.0009)
        {
            auto newIndex = m_rpmBuffer.getIndex(0, m_rpmBufferIndex + 1);
            auto temp = m_RPM.getRawValue() - m_rpmBuffer[newIndex] / FILTER_BUFFER_COUT;
            m_rpmBuffer[newIndex] = 60.0 / (18.0 * delta);
            m_lastTime = currentTime;
            temp += m_rpmBuffer[newIndex] / FILTER_BUFFER_COUT;
            m_rpmBufferIndex = newIndex;
            m_RPM.setRawValue(temp);
        }
    }

    if(m_inputReady)
    {
        static constexpr double PGA = 1;
        static constexpr double VREF = 2.048;
        static constexpr double VFSR = VREF / PGA;
        static constexpr long int FSR = (1ul << 23) - 1;

        m_inputReady = false;
        auto newIndex = m_sensorBuffer.getIndex(m_currentADCInput, m_sensorBufferIndex + ((m_currentADCInput == 0)? 1 : 0));
        auto temp = m_sensors[m_currentADCInput].getRawValue() - m_sensorBuffer[newIndex].data[m_currentADCInput] / FILTER_BUFFER_COUT;
        m_sensorBuffer[newIndex].data[m_currentADCInput] = m_ads.Read_Data_Samples() * VFSR * 1000.0 / FSR;
        temp = temp + m_sensorBuffer[newIndex].data[m_currentADCInput] / FILTER_BUFFER_COUT;
        m_sensors[m_currentADCInput].setRawValue(temp);

        m_sensorBufferIndex = newIndex;
        m_currentADCInput = (m_currentADCInput + 1) % INPUT_COUNT;
       
        if(m_currentADCInput != 0)
        {
            m_ads.select_mux_channels(ADS1220_MUX[m_currentADCInput]);
            m_ads.Start_Conv();
        }
        else if(m_state == WORKING)
        {
            m_state = READY;
            m_inputsReady = true;
        }
        else if(m_state == CALIBRATING)
        {
            m_calibratingCount++;
            if(m_calibratingCount == FILTER_BUFFER_COUT)
                m_state = READY;

            m_inputsReady = true;
        } 
    }
    return m_state == READY;
}