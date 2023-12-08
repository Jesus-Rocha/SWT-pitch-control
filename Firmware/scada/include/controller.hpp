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
    enum class ControllerType : uint32_t
    {
        NONE,
        OPEN_LOOP,
        PID,
        FUZZY
    };

    class Controller
    {
    public:
        Controller();

    public: /*properties*/
        virtual ControllerType getType()const = 0;
        virtual double getOutput()const = 0;
        virtual double getState(uint32_t index)const = 0;
        virtual uint32_t getParamCount()const = 0;
        virtual double getParam(uint32_t index)const = 0;
        virtual void setParam(uint32_t index, double) = 0;

    public: /*methods*/
        virtual void start() = 0;
        virtual double update(double delta, double ref, array_view<Input> const& inputs) = 0;
        virtual void stop() = 0;
    };

    class ObserverController : public Controller
    {
    public:
        ObserverController();

    public: /*properties*/
        virtual ControllerType getType()const;
        virtual double getOutput()const;
        virtual double getState(uint32_t index)const;
        virtual uint32_t getParamCount()const;
        virtual double getParam(uint32_t index)const;
        virtual void setParam(uint32_t index, double);

    public: /*methods*/
        virtual void start();
        virtual double update(double delta, double ref, array_view<Input> const& inputs);
        virtual void stop();
    
    private:
        bool m_running;
        double m_output;  
    };

    class OpenLoopController : public Controller
    {
    public:
        OpenLoopController();

    public: /*properties*/
        virtual ControllerType getType()const;
        virtual double getOutput()const;
        virtual double getState(uint32_t index)const;
        virtual uint32_t getParamCount()const;
        virtual double getParam(uint32_t index)const;
        virtual void setParam(uint32_t index, double);

    public: /*methods*/
        virtual void start();
        virtual double update(double delta, double ref, array_view<Input> const& inputs);
        virtual void stop();
    
    private:
        bool m_running;
        double m_output;
    };

    class PIDController : public Controller
    {
    public:
        PIDController();

    public: /*properties*/
        virtual ControllerType getType()const;
        virtual double getOutput()const;
        virtual double getState(uint32_t index)const;
        virtual uint32_t getParamCount()const;
        virtual double getParam(uint32_t index)const;
        virtual void setParam(uint32_t index, double);

    public: /*methods*/
        virtual void start();
        virtual double update(double delta, double ref, array_view<Input> const& inputs);
        virtual void stop();

    private:
        bool m_running;
        double m_output;
        double m_K1, m_K2, m_K3;
        double m_sigma, m_e1;
        double m_yr, m_dyr;
        double m_reference;
    };

}