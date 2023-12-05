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

        //if (temp)
        //    m_waitTimer.framesPerSecond(100);
        break;

    case HostState::CONNECTING:
        //m_waitTimer.tick([&]()
        //{
            if(!m_wifiManager.autoConnect("TUNNEL_CONNECT_AP",""))
            {
                return;
            }
            m_hostState = HostState::WAIT_FOR_CONNECTION;
            temp = true;
       // });
        
        if (temp) {
            //m_waitTimer.framesPerSecond(100);
            m_waitTimer.reset();
        }
        break;

    case HostState::WAIT_FOR_CONNECTION:
        m_waitTimer.tick([&]()
        {
            if(WiFi.status() == WL_CONNECTED)
            {
                postTask([=]() { 
                    //m_lcd.clear();
                    //m_lcd.setCursor(0,0);
                    //m_lcd.print(String("WiFi:") + WiFi.SSID()); 
                    m_lcd.setCursor(0,0);
                    m_lcd.print(String("IP: ") + WiFi.localIP().toString()); 

                    //debug_print(String("\nConnected to ") + WiFi.SSID() + "\n");
                    //debug_print(String("IP address: ") + WiFi.localIP().toString() + "\n");
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
                m_server.on("/", HTTP_GET, [this](){ this->onHandleRoot(); });
                m_server.on("/connectionData", HTTP_GET,[this](){ this->onConnectionInfo(); });
                m_server.on("/systemInfo", HTTP_GET, [this](){ this->onSystemStateInfo(); });
                m_server.on("/systemParam", HTTP_GET, [this](){ this->onSystemStateParam(); }); 
                m_server.on("/controlState", HTTP_GET, [this](){ this->onControlStateInfo(); });
                m_server.on("/controlParam", HTTP_GET, [this](){ this->onControlStateParam(); }); 
                m_server.on("/systemCommand", HTTP_GET, [this](){ this->onSystemCommand(); }); 
                m_server.onNotFound([this](){ this->onHandleWebRequests(); }); //Set server all paths are not found so we can handle as per URI
                m_server.on("/controller", HTTP_POST, [this](){});
            }
            m_server.close();
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
            m_server.handleClient();
        }
        break;
    };
}

void Entry::onHandleRoot()
{
    m_server.sendHeader("Location", "/index.html", true); // Redirect to our html web page
    m_server.send(302, "text/plane", "");
}

void Entry::onHandleWebRequests()
{
    String dataType = "text/plain";
    String path = m_server.uri();
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
    File dataFile = SPIFFS.open(path.c_str(), "r");
    m_server.streamFile(dataFile, dataType);
    // m_server.send(SPIFFS, path, dataType);
}

void Entry::onConnectionInfo()
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
    m_server.send(200, "text/json", json);
}

void Entry::onSystemStateInfo()
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
    m_server.send(200, "text/json", json);
}

void Entry::onSystemStateParam()
{
    onSystemStateInfo();
}

void Entry::onControlStateInfo()
{
    onSystemStateInfo();
}

void Entry::onControlStateParam()
{
    onSystemStateInfo();
}

void Entry::onSystemCommand()
{
    onSystemStateInfo();
}