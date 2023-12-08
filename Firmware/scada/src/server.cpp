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


void Entry::updateServer()
{
bool temp = false;
    switch (m_hostState)
    {
    case HostState::INITIALIZING:
        m_waitTimer.tick([&]()
        {
            postTask([=]()
            { 
                m_lcd.setCursor(0,0);
                m_lcd.print("Connecting..."); 
            });
            WiFi.mode(WIFI_STA);
            m_hostState = HostState::CONNECTING;
            temp = true;
        });
        break;

    case HostState::CONNECTING:
        if(!m_wifiManager.autoConnect("TUNNEL_CONNECT_AP",""))
        {
            return;
        }
        m_hostState = HostState::WAIT_FOR_CONNECTION;
        temp = true;
        
        if (temp) {
            m_waitTimer.reset();
        }
        break;

    case HostState::WAIT_FOR_CONNECTION:
        m_waitTimer.tick([&]()
        {
            if(WiFi.status() == WL_CONNECTED)
            {
                postTask([=]() { 
                    m_lcd.setCursor(0,0);
                    m_lcd.print(String("IP: ") + WiFi.localIP().toString()); 
                });
                
                m_hostState = HostState::INIT_MDNS;
            }
        });
        break;

    case HostState::INIT_MDNS:
        m_waitTimer.tick([&]()
                         {
            if(WiFi.status() != WL_CONNECTED)
            {
                m_hostState = HostState::CONNECTING;
                temp = true;
                return;
            }
            if(MDNS.begin("air-tunnel"))
            {
                //PostTask([=]() { 
                //    //m_lcd.clear();
                //    m_lcd.setCursor(0,1);
                //    m_lcd.print(String("MDNS: air-tunnel")); 
                //});
                m_hostState = HostState::INIT_SERVER;
            } });

        if (temp)
            m_waitTimer.framesPerSecond(1);
        break;

    case HostState::INIT_SERVER:
        m_waitTimer.tick([&]()
                         {
            if(WiFi.status() != WL_CONNECTED)
            {
                m_hostState = HostState::CONNECTING;
                temp = true;
                return;
            }

            if(!m_serverInitialized){
                m_serverInitialized = true;
                m_server.on("/", HTTP_GET, [this](AsyncWebServerRequest* request){ this->onHandleRoot(request); });
                m_server.on("/connectionData", HTTP_GET,[this](AsyncWebServerRequest* request){ this->onConnectionInfo(request); });
                m_server.on("/systemInfo", HTTP_GET, [this](AsyncWebServerRequest* request){ this->onSystemStateInfo(request); });
                m_server.on("/systemParam", HTTP_GET, [this](AsyncWebServerRequest* request){ this->onSystemStateParam(request); }); 
                m_server.on("/controlState", HTTP_GET, [this](AsyncWebServerRequest* request){ this->onControlStateInfo(request); });
                m_server.on("/controlParam", HTTP_GET, [this](AsyncWebServerRequest* request){ this->onControlStateParam(request); }); 
                m_server.on("/systemCommand", HTTP_GET, [this](AsyncWebServerRequest* request){ this->onSystemCommand(request); }); 
                m_server.onNotFound([this](AsyncWebServerRequest* request){ this->onHandleWebRequests(request); }); //Set server all paths are not found so we can handle as per URI
                m_server.on("/controller", HTTP_POST, [this](AsyncWebServerRequest* request){});
            }
            m_server.end();
            m_server.begin();         
            m_hostState = HostState::LISTENING; });

        if (temp)
            m_waitTimer.framesPerSecond(1);
        break;

    case HostState::LISTENING:
        if (WiFi.status() != WL_CONNECTED)
        {
            m_hostState = HostState::CONNECTING;
            m_waitTimer.framesPerSecond(1);
        }
        else
        {
            //m_server.handleClient();
        }
        break;
    };
}

void Entry::onHandleRoot(AsyncWebServerRequest* request)
{ 
    request->redirect("/index.html"); // Redirect to our html web page
    request->send(302, "text/plane", "");
}

void Entry::onHandleWebRequests(AsyncWebServerRequest* request)
{
    String dataType = "text/plain";
    String path = request->url();
    if (path.endsWith("/"))
        path += "index.htm";

    if (path.endsWith(".src"))
        path = path.substring(0, path.lastIndexOf("."));
    else if (path.endsWith(".html"))
        dataType = "text/html";
    else if (path.endsWith(".htm"))
        dataType = "text/html";
    else if (path.endsWith(".css"))
        dataType = "text/css";
    else if (path.endsWith(".json"))
        dataType = "text/json";
    else if (path.endsWith(".js"))
        dataType = "application/javascript";
    else if (path.endsWith(".png"))
        dataType = "image/png";
    else if (path.endsWith(".gif"))
        dataType = "image/gif";
    else if (path.endsWith(".jpg"))
        dataType = "image/jpeg";
    else if (path.endsWith(".ico"))
        dataType = "image/x-icon";
    else if (path.endsWith(".xml"))
        dataType = "text/xml";
    else if (path.endsWith(".pdf"))
        dataType = "application/pdf";
    else if (path.endsWith(".zip"))
        dataType = "application/zip";

    Serial.println(path);
    //File dataFile = SPIFFS.open(path.c_str(), "r");
    //m_server.streamFile(dataFile, dataType);
     request->send(SPIFFS, path, dataType);
}

void Entry::onConnectionInfo(AsyncWebServerRequest* request)
{
    String json;
    StaticJsonDocument<500> root;
    root["DeviceName"] = DEVICE_NAME;
    byte mac[6];
    WiFi.macAddress(mac);
    root["MacAddress"] = String(mac[5], HEX) + ":" + String(mac[4], HEX) + ":" + String(mac[3], HEX) + ":" + String(mac[2], HEX) + ":" + String(mac[1], HEX) + ":" + String(mac[0], HEX);
    root["Firmware"] = FIRMWARE_VERSION;
    root["SSID"] = WiFi.SSID();
    IPAddress ip = WiFi.localIP();
    IPAddress mask = WiFi.subnetMask();
    root["IPAddress"] = String() + ip[0] + "." + ip[1] + "." + ip[2] + "." + ip[3];
    root["SubnetMask"] = String() + mask[0] + "." + mask[1] + "." + mask[2] + "." + mask[3];
    serializeJson(root, json);
    request->send(200, "text/json", json);
}

void Entry::onSystemStateInfo(AsyncWebServerRequest* request)
{
    String json;
    StaticJsonDocument<500> root;

    //root["SystemState"] = (uint32_t)getCurrentControllerState();
    //root["ControlType"] = (uint32_t)getCurrentControllerType();
    //root["SamplingTime"] = (double)getSamplingTime() / 1000000.0;
    //root["Reference"] = (double)getReferenceValue();
    //root["WindSpeed"] = (double)getSensorValue();
    //root["OutputFile"] = getOutputFileName();
    //root["SaveData"] = getSaveData();

    serializeJson(root, json);
    request->send(200, "text/json", json);
}

void Entry::onSystemStateParam(AsyncWebServerRequest* request)
{
    onSystemStateInfo(request);
}

void Entry::onControlStateInfo(AsyncWebServerRequest* request)
{
    onSystemStateInfo(request);
}

void Entry::onControlStateParam(AsyncWebServerRequest* request)
{
    onSystemStateInfo(request);
}

void Entry::onSystemCommand(AsyncWebServerRequest* request)
{
    onSystemStateInfo(request);
}