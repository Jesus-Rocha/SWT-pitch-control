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

   /*Gpi(s) = Kp(1 + 1/(Ti s))*/
    class PIOperator
    {
    public:
        inline PIOperator()
            : m_Kp(1)
            , m_Ti(1)
            , m_sigma(0)
        {}

        inline void reset() {
            m_sigma = 0;
            m_error = 0;
        }
        inline void resetSigma() { m_sigma = 0; }
        inline  double getKp()const { return m_Kp; }
        inline double getTi()const { return m_Ti; }
        inline double getSigma()const { return m_sigma; }

        inline void setKp(double value) { m_Kp = value; }
        inline void setTi(double value) { m_Ti = value; }

        inline PIOperator& operate(double dt, double error){
            m_sigma = error*dt + m_sigma;
            m_error = error;
            return*this;
        }
        inline PIOperator& operator()(double dt, double error){ return operate(dt, error); }
        inline operator double()const{ return m_Kp*(m_error + m_sigma/m_Ti); }

    private:
        double m_Kp, m_Ti;
        double m_error;
        double m_sigma;
    };

   /*Gpd(s) = Kp(1 + Td s)*/
    class PDOperator
    {
    public:
        inline PDOperator()
            : m_Kp(1)
            , m_Td(1)
            , m_delta(0)
        {}

        inline void reset() {
            m_delta = 0;
            m_error = 0;
        }
        inline  double getKp()const { return m_Kp; }
        inline double getTd()const { return m_Td; }
        inline double getDelta()const { return m_delta; }

        inline void setKp(double value) { m_Kp = value; }
        inline void setTd(double value) { m_Td = value; }

        inline PDOperator& operate(double dt, double error){
            m_delta = (error - m_error)/dt;
            m_error = error;
            return*this;
        }
        inline PDOperator& operator()(double dt, double error){ return operate(dt, error); }
        inline operator double()const{ return m_Kp*(m_error + m_delta*m_Td); }

    private:
        double m_Kp, m_Td;
        double m_error;
        double m_delta;
    };

    /*Gpid(s) = Kp(1 + 1/(Ti s) + Td s)*/
    class PIDOperator
    {
    public:
        inline PIDOperator()
            : m_Kp(1)
            , m_Ti(1)
            , m_Td(1)
            , m_delta(0)
            , m_sigma(0)
        {}

        inline void reset() {
            m_sigma = 0;
            m_delta = 0;
            m_error = 0;
        }
        inline void resetSigma() { m_sigma = 0; }
        inline  double getKp()const { return m_Kp; }
        inline double getTi()const { return m_Ti; }
        inline double getTd()const { return m_Td; }
        inline double getSigma()const { return m_sigma; }
        inline double getDelta()const { return m_delta; }

        inline void setKp(double value) { m_Kp = value; }
        inline void setTi(double value) { m_Ti = value; }
        inline void setTd(double value) { m_Td = value; }

        inline PIDOperator& operate(double dt, double error){
            m_sigma = error*dt + m_sigma;
            m_delta = (error - m_error)/dt;
            m_error = error;
            return*this;
        }
        inline PIDOperator& operator()(double dt, double error){ return operate(dt, error); }
        inline operator double()const{ return m_Kp*(m_error + m_sigma/m_Ti + m_delta*m_Td); }

    private:
        double m_Kp, m_Ti, m_Td;
        double m_error;
        double m_delta;
        double m_sigma;
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
        PIDOperator m_pid;
    };

}