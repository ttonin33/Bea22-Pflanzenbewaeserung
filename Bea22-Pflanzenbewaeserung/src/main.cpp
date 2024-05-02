#include <Arduino.h>
#include <HTTPClient.h>
#include "ArduinoJson.h" // https://arduinojson.org/

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
  http.addHeader("Authorization", "Bearer xaat-6268ec0b-04a2-45e2-80c4-e0fb9571db21");

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
}

void loop()
{

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
