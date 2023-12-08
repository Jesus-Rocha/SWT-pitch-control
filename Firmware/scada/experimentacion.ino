#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"
#include <ArduinoJson.h>

// Replace with your network credentials
// const char* ssid = "oscar-wifi01";
// const char* password = "12345678";
const char* ssid = "Inzunza Soto";
const char* password = "InzunzaSoto18";

// Crear un objeto AsyncWebServer en el puerto 80
AsyncWebServer server(80);

// Crear un origen de eventos en /events
AsyncEventSource events("/events");


// Variables de tiempo
unsigned long lastTime = 0;
unsigned long timerDelay = 50;
unsigned long tiempoActual = 0;
unsigned long tiempoInicio = 0;

// Variables de control
bool iniciarCapturas = false;

// Variables de datos
float rpm;
float voltaje;
float corriente;
float tiempoAdquisicion;

// Variables auxiliares. Eliminar y colocar variables para capturas directas.
// No es necesario mantenerlas, solo están para simulación de datos.
int valores[3] = {4, 3, 10};
int tiempo = 0;


// Tomar las lecturas del sensor y enviarlas como objeto JSON
String getSensorReadings(){
  String response;
  StaticJsonDocument<500> root;
  tiempo++;
  if (tiempo <= 10){
      valores[0] += 1;
      valores[1] += 1;
      valores[2] += 1;
  }
  else{
      valores[0] -= 1;
      valores[1] -= 1;
      valores[2] -= 1;
  }
  if (tiempo >= 20)
      tiempo = 0;
  root["tiempoActual"] = String(tiempoActual);
  root["rpmActual"] = String(valores[0]);
  root["voltajeActual"] = String(valores[1]);
  root["corrienteActual"] = String(valores[2]);
  serializeJson(root, response);
  return response;
}

// Inicializar SPIFFS
void iniciarSPIFFS() {
  if (!SPIFFS.begin()) {
    Serial.println("Ha ocurrido un error al intentar conectar SPIFFS");
  }
  else{
    Serial.println("SPIFFS conectado correctamente");
  }
}

// Inicializar WiFi
void iniciarWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}

void setup() {
  // Serial port for debugging purposes
  Serial.begin(115200);
  iniciarWiFi();
  iniciarSPIFFS();

  // Raiz principal del servidor web
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html");
  });

  server.serveStatic("/", SPIFFS, "/");

  // Recibir los datos del cliente por medio de parametros
  server.on("/enviar", HTTP_GET, [](AsyncWebServerRequest *request){
    rpm = request->arg("campo1").toFloat(); 
    voltaje = request->arg("campo2").toFloat();
    corriente = request->arg("campo3").toFloat();
    tiempoAdquisicion = request->arg("campo4").toFloat();
    iniciarCapturas = true;
    tiempoInicio = millis();
    request->send(200, "text/plain", "Datos recibidos correctamente");
  });

  // Peticion para las ultimas lecturas del sensor
  server.on("/readings", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = getSensorReadings();
    request->send(200, "application/json", json);
  });

  // Peticion para detener la grafica y poner el estado en false
  server.on("/detenerGrafica", HTTP_GET, [](AsyncWebServerRequest *request){
    iniciarCapturas = false;
    request->send(200, "text/plain", "Señal de frenado");
  });

  // Evento para indicar que se ha conectado correctamente a internet
  events.onConnect([](AsyncEventSourceClient *client){
    if(client->lastId()){
      Serial.printf("Cliente conectado! El último ID de mensaje que recibió es: %u\n", client->lastId());
    }
    // enviar evento con el mensaje "¡Hola!", ID actual en milisegundos
    // y establecer un retraso de reconexión de 1 segundo
    client->send("Hola!", NULL, millis(), 10000);
  });
  server.addHandler(&events);

  server.begin();
}

void loop() {
  if(iniciarCapturas){
    tiempoActual = (millis() - tiempoInicio);
    if ((millis() - lastTime) > timerDelay) {
      // Enviar el evento al cliente con las lecturas cada "timerDelay" milisegundos
      events.send("ping",NULL,millis());
      events.send(getSensorReadings().c_str(),"new_readings" ,millis());
      lastTime = millis();
    }
  }
}
