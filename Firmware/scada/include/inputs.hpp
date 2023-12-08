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
    typedef struct Input
    {
        inline Input(double value = 0, double scale = 1, double offset = 0) 
            : m_value(value)
            , m_scale(scale)
            , m_offset(offset)
        {
        }

        inline Input(Input const& input) 
            : m_value(input.m_value)
            , m_scale(input.m_scale)
            , m_offset(input.m_offset)
        {
        }

        inline double getValue()const { return m_value * m_scale + m_offset; }
        inline double getRawValue()const { return m_value; }
        inline double getScale()const { return m_scale; }
        inline double getOffset()const { return m_offset; }

        inline void setRawValue(double value) { m_value = value; }
        inline void setScale(double value) { m_scale = value; }
        inline void setOffset(double value) { m_offset = value; }

        inline Input& operator = (double const& value) {
            setRawValue(value);
            return *this;
        }

        inline Input& operator = (Input const& input) {
            setRawValue(input.getValue());
            setScale(input.getScale());
            setOffset(input.getOffset());       
            return *this;
        }

        inline operator double ()const{
            return getValue();
        }

    private:
        double m_value;
        double m_scale;
        double m_offset;
    }input_t;

    class Inputs
    {
    private:
        enum : uint8_t
        {
            INPUT0 = 0,
            INPUT1 = 1,
            INPUT2 = 2,
            INPUT3 = 3,
            INPUT_COUNT = 2,

            SPI_CS_PIN = 5,
            INPUT_READY_PIN = 17,
            RPM_PULSE_PIN = 25,
            FILTER_BUFFER_COUT = 10,
        };

        enum InputState : uint8_t
        {
            STARTING,
            CALIBRATING,
            WORKING,
            READY
        }; 

        static constexpr int32_t ADS1220_MUX[4] = {
            MUX_AIN0_AVSS,
            MUX_AIN1_AVSS,
            MUX_AIN2_AVSS,
            MUX_AIN3_AVSS 
        };

    public:
        Inputs();

    public:
        inline input_t& getRPM() { return m_RPM; }
        inline input_t& getVoltage() { return m_sensors[0]; }
        inline input_t& getCurrent() { return m_sensors[1]; }

        inline input_t const& getRPM()const { return m_RPM; }
        inline input_t const& getVoltage()const { return m_sensors[0]; }
        inline input_t const& getCurrent()const { return m_sensors[1]; }

        inline array_view<input_t> getInputs()const { return array_view<input_t>((input_t*)m_sensors, (input_t*)m_sensors + INPUT_COUNT); }
    
        inline bool areInputsReady()const { return m_inputsReady; }
        inline bool isCalibratig()const { return m_state == CALIBRATING; }

    public:
        void init();
        bool start();
        bool calibrating();
        bool update();
        
    
    private:
        static void OnInputReadyISR(void *arg);
        static void OnRPMPulseISR(void *arg);

    private: /*peripherals fields*/
        struct __input_block
        {
            double data[INPUT_COUNT];
        };

        Protocentral_ADS1220 m_ads;
        InputState m_state;
        int m_calibratingCount;
        uint8_t m_currentADCInput;
        long m_sensorBufferIndex;
        long m_rpmBufferIndex;
        circular_array<__input_block, FILTER_BUFFER_COUT> m_sensorBuffer;
        circular_array<double, FILTER_BUFFER_COUT> m_rpmBuffer;
        double m_averageRawSensor[INPUT_COUNT];
        input_t m_sensors[INPUT_COUNT];
        input_t m_RPM;
        double m_lastTime;
        bool m_inputReady;
        bool m_rpmPulseReady;
         bool m_inputsReady;
    };

}