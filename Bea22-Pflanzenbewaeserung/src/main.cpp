#include <Arduino.h>

//! Variablen Test
#define SensorPin A5 // ADC für Sensor
#define Pumpe 5      // Ausgang für Pumpe
// int Pumpe_Zeit = 1500;         // Aktive Zeit der Pumpe
// long Pumpe_Pause = 1800000;  // Pause nach Wasserpumpen
// long Sensor_Pause = 600000; // Pause nach Sensormessung
float wasser_soll = 600; // Sensorwert ab wann bewäsert werden soll

//! Variablen Depl

int Pumpe_Zeit = 1500;    // Aktive Zeit der Pumpe
int Pumpe_Pause = 1000;  // Pause nach Wasserpumpen
int Sensor_Pause = 500; // Pause nach Sensormessung

void setup()
{

  pinMode(Pumpe, OUTPUT);
  digitalWrite(Pumpe, HIGH);

  //* Serial
  Serial.begin(115200);
  Serial.println("Booting");

  Serial.println("Ready");
}

void loop()
{

  // * Messung durchführen
  float wasser_ist = 0;
  int MeasurementsToAverage = 160; // Anzahl der in den Mettelwert aufgenommenen Messungen
  for (int i = 0; i < MeasurementsToAverage; ++i)
  {
    wasser_ist += analogRead(SensorPin);
    delay(1);
  }
  wasser_ist /= MeasurementsToAverage;
  Serial.println(wasser_ist);

  // * Pflanze bewässern

  if (wasser_ist > wasser_soll)
  {
    Serial.print("_________Starte Pumpe   ...   ");
    digitalWrite(Pumpe, LOW);
    delay(Pumpe_Zeit);
    digitalWrite(Pumpe, HIGH);
    Serial.println("Fertig_________");

    delay(Pumpe_Pause);
  }
  else
  {
    digitalWrite(Pumpe, HIGH);
    Serial.println("---Noch Feucht---");

    delay(Sensor_Pause);
  }
}