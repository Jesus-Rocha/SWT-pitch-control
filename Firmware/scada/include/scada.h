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
#include <mutex>
#include <condition_variable>

#include <Wire.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiManager.h>
//#include <WebServer.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>
#include <LiquidCrystal_I2C.h>
#include <Protocentral_ADS1220.h>

#include "ring.hpp"
#include "steptimer.hpp"
#include "inputs.hpp"
#include "controller.hpp"
#include "fuzzy.hpp"
#include "entry.hpp"