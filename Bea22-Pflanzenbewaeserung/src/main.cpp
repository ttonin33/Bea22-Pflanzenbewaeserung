#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <FS.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Wire.h>
#include "ArduinoJson.h" // https://arduinojson.org/
#include "secrets.h"

//! Variablen
#define SensorPin 35     // ADC für Sensor
#define Pumpe 5          // Ausgang für Pumpe
int Pumpe_Zeit = 1000;   // Aktive Zeit der Pumpe
int Pumpe_Pause = 1000;  // Pause nach Wasserpumpen
int Sensor_Pause = 1000; // Pause nach Sensormessung
float wasser_soll = 100; // Sensorwert ab wann bewäsert werden soll

String getTime() //* String: Unix
{
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    // Serial.println("Failed to obtain time");
    return ("0");
  }
  time(&now);
  String strtm = String(now) + "000";
  return (strtm);
}

void upload(String body)
{
}

void setup()
{

  pinMode(Pumpe, OUTPUT);
  digitalWrite(Pumpe, LOW);

  //* Serial + WIFI
  Serial.begin(115200);
  Serial.println("Booting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  //* Zeit:
  Serial.println("Hole NTP Zeit");
  configTime(0, 0, "pool.ntp.org");
  Serial.print("Epoch Time: ");
  Serial.println(getTime());

  //* OTA
  ArduinoOTA.setHostname("ESP32-Plfanze");
  ArduinoOTA.setPasswordHash("4b24450591bbca92d6c7f4e650ee309a"); // pwd=dlKs3khbxXrCT37sponz
  ArduinoOTA
      .onStart([]()
               {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type); })
      .onEnd([]()
             { Serial.println("\nEnd"); })
      .onProgress([](unsigned int progress, unsigned int total)
                  { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); })
      .onError([](ota_error_t error)
               {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed"); });

  ArduinoOTA.begin();

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop()
{
  ArduinoOTA.handle();
  String data;
  DynamicJsonDocument doc(1024);
  doc["_time"] = getTime();

  // * Messung durchführen
  float wasser_ist = 0;
  int MeasurementsToAverage = 160; // Anzahl der in den Mettelwert aufgenommenen Messungen
  for (int i = 0; i < MeasurementsToAverage; ++i)
  {
    wasser_ist += analogRead(SensorPin);
    delay(1);
  }
  wasser_ist /= MeasurementsToAverage;
  doc["Sensor_1"] = wasser_ist;
  Serial.println(wasser_ist);

  // * Pflanze bewässern

  if (wasser_ist < wasser_soll)
  {
    Serial.print("_________Starte Pumpe   ...   ");
    int zeit = millis();
    digitalWrite(Pumpe, HIGH);
    delay(Pumpe_Zeit);
    digitalWrite(Pumpe, LOW);
    doc["Pumpe_Zeit"] = millis() - zeit;
    Serial.println("Fertig_________");
    doc["Pumpe"] = 1;

    serializeJson(doc, data);
    data = "[" + data + "]";
    upload(data);
    delay(Pumpe_Pause);
  }
  else
  {
    Serial.println("---Noch Feucht---");

    doc["Pumpe_Zeit"] = 0;
    doc["Pumpe"] = 0;

    serializeJson(doc, data);
    data = "[" + data + "]";
    upload(data);

    delay(Sensor_Pause);
  }

  Serial.println(data);
}