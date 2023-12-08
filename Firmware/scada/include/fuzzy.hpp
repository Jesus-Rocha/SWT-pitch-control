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
    class Fuzzy : public Controller
    {
    public:
        Fuzzy();

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
        double m_output;
        bool m_running;
    };
}