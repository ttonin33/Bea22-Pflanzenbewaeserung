#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoOTA.h>
// #include <DNSServer.h>
// #include "FS.h"
#include "ArduinoJson.h" // https://arduinojson.org/
// #include <AsyncFsWebServer.h> // https://github.com/cotestatnt/async-esp-fs-webserver
#include "ESPAsyncWebServer.h"


#include "secrets.h"

#define SensorPin 35

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
  HTTPClient http;
  http.begin("https://api.axiom.co/v1/datasets/esp/ingest");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", axiom_secret);

  String httpRequestData = body;

  int httpResponseCode = http.POST(httpRequestData);

  // Serial.print("HTTP Response code: ");
  // Serial.println(httpResponseCode);

  http.end();
}

void setup(void)
{
  //* Serial
  Serial.begin(115200);
  Serial.println("Moin");

  //* Wifi
  Serial.print("Verbinde mit Wlan");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("");
  while (WiFi.status() != WL_CONNECTED) // Wait for connection
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println(""); // Wenn Ferbunden
  Serial.print("IP Addresse: ");
  Serial.print(WiFi.localIP());
  Serial.print(WiFi.status());
  Serial.println("");

  //* Zeit:
  Serial.println("Hole NTP Zeit");
  configTime(0, 0, "pool.ntp.org");
  Serial.print("Epoch Time: ");
  Serial.println(getTime());

  ArduinoOTA
      .onStart([]()
               {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH) {
        type = "sketch";
      } else {  // U_SPIFFS
        type = "filesystem";
      }

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type); })
      .onEnd([]()
             { Serial.println("\nEnd"); })
      .onProgress([](unsigned int progress, unsigned int total)
                  { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); })
      .onError([](ota_error_t error)
               {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) {
        Serial.println("Auth Failed");
      } else if (error == OTA_BEGIN_ERROR) {
        Serial.println("Begin Failed");
      } else if (error == OTA_CONNECT_ERROR) {
        Serial.println("Connect Failed");
      } else if (error == OTA_RECEIVE_ERROR) {
        Serial.println("Receive Failed");
      } else if (error == OTA_END_ERROR) {
        Serial.println("End Failed");
      } });

  ArduinoOTA.begin();
}

void loop()
{
  ArduinoOTA.handle();

  String data;
  DynamicJsonDocument doc(1024);

  int value = analogRead(SensorPin);

  doc["_time"] = getTime();
  doc["ADC_1"] = value;

  serializeJson(doc, data);
  data = "[" + data + "]";

  Serial.println(data);
  upload(data);
  // delay(100);

  // Serial.println(value);
}
