#pragma once

//If core is not defined, then we are running in Arduino or PIO
#ifndef CONFIG_ASYNC_TCP_RUNNING_CORE
#define CONFIG_ASYNC_TCP_RUNNING_CORE 0 //any available core
#define CONFIG_ASYNC_TCP_USE_WDT 1 //if enabled, adds between 33us and 200us per event
#endif

#define DEVICE_NAME "ESP-32"
#define FIRMWARE_VERSION "1.0"

#include <Arduino.h>
#include <stddef.h>
#include <inttypes.h>
#include <functional>
#include <queue>

#include <Wire.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiManager.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>
#include <LiquidCrystal_I2C.h>
#include <steptimer.h>
#include <Adafruit_ADS1X15.h>

#include "mutex.hpp"

namespace scada
{
    enum class HostState
    {
        INITIALIZING,
        CONNECTING,
        WAIT_FOR_CONNECTION,
        INIT_MDNS,
        INIT_SERVER,
        LISTENING
    };

    enum class ControllerState : uint32_t
    {
        waiting,
        starting,
        updating,
        stopped,
        calibrating,
    };

    typedef struct Input
    {
        Input(double value = 0, double scale = 1, double offset = 0) 
            : m_value(value)
            , m_scale(scale)
            , m_offset(offset)
        {
        }

        Input(Input const& input) 
            : m_value(input.m_value)
            , m_scale(input.m_scale)
            , m_offset(input.m_offset)
        {
        }

        double getValue()const { return m_value * m_scale + m_offset; }
        double getRawValue()const { return m_value; }
        double getScale()const { return m_scale; }
        double getOffset()const { return m_offset; }

        void setRawValue(double value) { m_value = value; }
        void setScale(double value) { m_scale = value; }
        void setOffset(double value) { m_offset = value; }

        Input& operator = (double const& value)
        {
            setRawValue(value);
            return *this;
        }

        Input& operator = (Input const& input)
        {
            setRawValue(input.getValue());
            setScale(input.getScale());
            setOffset(input.getOffset());       
            return *this;
        }

        operator double ()const{
            return getValue();
        }

    private:
        double m_value;
        double m_scale;
        double m_offset;
    }input_t;

    typedef struct Inputs
    {
        input_t current;
        input_t voltage;
        input_t RPMV; 
    }inputs_t;

    class Entry
    {
    public:
        static Entry& instance();

    private:
        static void OnInputReadyISR(void *arg);
        static void OnRPMPulseISR(void *arg);

    private:
        Entry();
        ~Entry();

    public:
        void setup();
        void loop();
        void postTask(std::function<void()>);

    private:
        void initAsyncWorker();
        void dispatchAll();

private:
        void updateServer();
        void onAsyncWorker();
        void onHandleRoot(/*AsyncWebServerRequest**/);
        void onHandleWebRequests(/*AsyncWebServerRequest**/);
        void onConnectionInfo(/*AsyncWebServerRequest**/);
        void onSystemStateInfo(/*AsyncWebServerRequest**/);
        void onSystemStateParam(/*AsyncWebServerRequest**/);
        void onControlStateInfo(/*AsyncWebServerRequest**/);
        void onControlStateParam(/*AsyncWebServerRequest**/);
        void onSystemCommand(/*AsyncWebServerRequest**/);

    private:
        enum CONSTANTS
        {
            IN_READY_PIN = 5,
            RPM_PULSE_PIN = 25,
        };

    private: /*utilities fields*/
        mutable Mutex m_mutex;
        std::queue<std::function<void()>> m_queue;

    private: /*server fields*/  
        WiFiManager m_wifiManager;
        WebServer m_server;
        TaskHandle_t m_asyncWorker;
        HostState m_hostState;
        StepTimer m_waitTimer;
        bool m_serverInitialized;

    private: /*peripherals fields*/
        Adafruit_ADS1115 m_ads;
        LiquidCrystal_I2C m_lcd;

        StepTimer m_stepTimer;
        bool m_inputReady;
        long m_lastTime;
        long m_currentTime;
    };
}