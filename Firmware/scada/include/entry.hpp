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
    

    class Entry
    {
    public:
        static Entry& instance();

    private:
        Entry();
        ~Entry();

    public:
        void setup();
        void loop();
        void postTask(std::function<void()>);

        void startControllerAsync();
        void startCalibratingAsync();
        void stopControllerAsync();

    private:
        void initAsyncWorker();
        void dispatchAll();

private:
        void updateServer();
        void onAsyncWorker();
        void onHandleRoot(AsyncWebServerRequest*);
        void onHandleWebRequests(AsyncWebServerRequest*);
        void onConnectionInfo(AsyncWebServerRequest*);
        void onSystemStateInfo(AsyncWebServerRequest*);
        void onSystemStateParam(AsyncWebServerRequest*);
        void onControlStateInfo(AsyncWebServerRequest*);
        void onControlStateParam(AsyncWebServerRequest*);
        void onSystemCommand(AsyncWebServerRequest*);

    private:
        enum Constants : uint8_t
        {
            IN_READY_PIN = 5,
            RPM_PULSE_PIN = 25,
        };

        enum HostState : uint8_t
        {
            INITIALIZING,
            CONNECTING,
            WAIT_FOR_CONNECTION,
            INIT_MDNS,
            INIT_SERVER,
            LISTENING
        };

        enum ControllerState : uint8_t
        {
            WAITING,
            STARTING,
            UPDATING,
            STOPPING,
            CALIBRATING,
        };

    private: /*utilities fields*/
        mutable std::recursive_mutex m_mutex;
        std::queue<std::function<void()>> m_queue;

    private: /*server fields*/  
        WiFiManager m_wifiManager;
        AsyncWebServer m_server;
        AsyncEventSource m_events;
        TaskHandle_t m_asyncWorker;
        HostState m_hostState;
        StepTimer m_waitTimer;
        bool m_serverInitialized;

    private: /*peripherals fields*/
        Inputs m_inputs;
        LiquidCrystal_I2C m_lcd;
    private: /*Controller fields*/
        Controller* m_currentControl;
        Fuzzy m_fuzzy;
        ObserverController m_observer;
        OpenLoopController m_openLoop;
        PIDController m_pid;
        StepTimer m_stepTimer;
        ControllerState m_controllerState;
    };
}