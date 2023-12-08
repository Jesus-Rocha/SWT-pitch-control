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

using scoped_lock = std::scoped_lock<std::recursive_mutex>;

Entry& Entry::instance()
{
    static Entry s_entry;
    return s_entry;
}

Entry::Entry()
    : m_wifiManager()
    , m_server(80)
    , m_events("/events")
    , m_serverInitialized(false)
    , m_asyncWorker(nullptr)
    , m_hostState(INITIALIZING)
    , m_waitTimer()
    , m_inputs()
    , m_lcd(0x3F, 20, 4)
    , m_currentControl(&m_openLoop)
    , m_controllerState(WAITING)

{

}

Entry::~Entry()
{

}

void Entry::setup()
{
    Serial.begin(115200);
    SPIFFS.begin();

    m_lcd.begin(20, 4);
    m_lcd.backlight();

    m_lcd.setCursor(0, 0);
    m_lcd.print("Starting...");

    delay(500);
    yield();

    m_inputs.init();

    initAsyncWorker();
    m_stepTimer.framesPerSecond(100);
    m_stepTimer.fixedTimeStep(true);
    m_stepTimer.reset();

    m_lcd.setCursor(0, 1);
    m_lcd.print("State: Stopped");

    postTask([=]()
    {
        //m_lcd.setCursor(0,1);
        //m_lcd.print("State: Calibrating");
        //CalibrateSensors();
        //m_lcd.setCursor(0,1);
        //m_lcd.print("State: Stopped    "); 
    });
}

void Entry::loop()
{
    dispatchAll();
    m_inputs.update();
    switch (m_controllerState)
    {
    case ControllerState::WAITING:
        
        break;

    case ControllerState::STARTING:
        m_fuzzy.start();
        m_currentControl->start();
        m_lcd.setCursor(0, 2);
        m_lcd.print("State: Running    ");
        m_inputs.start();
        m_stepTimer.reset();
        m_controllerState = ControllerState::UPDATING;
        break;

    case ControllerState::UPDATING:
        if(m_inputs.areInputsReady())
        {
            m_stepTimer.tick([this](StepTimer const& timer)
            {
                auto delta = timer.elapsedTime();
                auto ref = m_fuzzy.update(delta, 0, m_inputs.getInputs());
                m_currentControl->update(delta, ref, m_inputs.getInputs());
                m_inputs.start();
            }, m_stepTimer);
        }
        break;

    case ControllerState::STOPPING:
        m_fuzzy.stop();
        m_currentControl->stop();
        m_lcd.setCursor(0, 2);
        m_lcd.print("State: Stopped    ");

        m_controllerState = ControllerState::WAITING;
        break;

    case ControllerState::CALIBRATING:
        if(m_inputs.isCalibratig())
        {
            m_inputs.update();
            if(m_inputs.areInputsReady())
            {
                m_stepTimer.tick([this](StepTimer const& timer)
                {
                    m_inputs.start();
                }, m_stepTimer);
            }
        }
        else
        {
            m_lcd.setCursor(0, 2);
            m_lcd.print("State: Stopped    ");
            m_controllerState = ControllerState::WAITING;
        }
        break;
    }


}

void Entry::startControllerAsync()
{
    postTask([this]()
    {
        if(m_controllerState != ControllerState::WAITING)
            return;
        m_controllerState = ControllerState::STARTING;
    });
}

void Entry::startCalibratingAsync()
{
    postTask([this]()
    {
        if(m_controllerState != ControllerState::WAITING)
            return;
        m_lcd.setCursor(0, 2);
        m_lcd.print("State: Calibrating    ");
        m_inputs.calibrating();
        m_stepTimer.reset();
        m_controllerState = ControllerState::CALIBRATING;
    });
}

void Entry::stopControllerAsync()
{
    postTask([this]()
    {
        if(m_controllerState != ControllerState::UPDATING)
            return;
        m_controllerState = ControllerState::STOPPING;
    });
}

void Entry::initAsyncWorker()
{
    xTaskCreatePinnedToCore(
        [](void *arg){ reinterpret_cast<Entry *>(arg)->onAsyncWorker(); },
        "App::OnAsyncWorker", 20480, this, 1, &m_asyncWorker, 0);
}

void Entry::onAsyncWorker()
{
    delay(1000);
    yield();
    m_waitTimer.fixedTimeStep(true);
    m_waitTimer.framesPerSecond(100);
    m_waitTimer.reset();
    while (true)
    {
        updateServer();
        delay(1);
        yield();
    }
}

void Entry::postTask(std::function<void()> task)
{
    scoped_lock locker(m_mutex);
    m_queue.push(task);
}

void Entry::dispatchAll()
{
    std::function<void()> method;
    scoped_lock locker(m_mutex);
    while (m_queue.size() > 0)
    {
        method = std::move(m_queue.front());
        m_queue.pop();
        m_mutex.unlock();
        try
        {
            method();
        }
        catch (...)
        {
        }
        m_mutex.lock();
    }
}