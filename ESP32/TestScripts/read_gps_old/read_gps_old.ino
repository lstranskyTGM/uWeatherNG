/*
 * Test file for reading the GPS Tracker
 */

#include <TinyGPSPlus.h>
#include <HardwareSerial.h>

static const int RXPin = 17, TXPin = 16;

// The TinyGPSPlus object
TinyGPSPlus gps;

// The serial connection to the GPS device
HardwareSerial SerialGPS(1);

void setup()
{
  Serial.begin(115200);
  SerialGPS.begin(9600, SERIAL_8N1, 16, 17);

  Serial.println(F("ReadGPSTracker.ino"));
}

void loop()
{
  // This sketch displays information every time a new sentence is correctly encoded.
  while (SerialGPS.available() > 0)
    if (gps.encode(SerialGPS.read()))
      displayInfo();

  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    Serial.println(F("No GPS detected: check wiring."));
    while(true);
  }
}

void displayInfo()
{
  Serial.print(F("Location: ")); 
  if (gps.location.isValid())
  {
    Serial.print(F("Latitude: "));
    Serial.println(gps.location.lat(), 6);
    Serial.print(F("Longitude: "));
    Serial.println(gps.location.lng(), 6);
  }
  else
  {
    Serial.print(F("Latitude: "));
    Serial.println(0.0);
    Serial.print(F("Longitude: "));
    Serial.println(0.0);
  }

  Serial.print(F("  Date/Time: "));
  if (gps.date.isValid())
  {
    Serial.print(gps.date.month());
    Serial.print(F("/"));
    Serial.print(gps.date.day());
    Serial.print(F("/"));
    Serial.print(gps.date.year());
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.print(F(" "));
  if (gps.time.isValid())
  {
    if (gps.time.hour() < 10) Serial.print(F("0"));
    Serial.print(gps.time.hour());
    Serial.print(F(":"));
    if (gps.time.minute() < 10) Serial.print(F("0"));
    Serial.print(gps.time.minute());
    Serial.print(F(":"));
    if (gps.time.second() < 10) Serial.print(F("0"));
    Serial.print(gps.time.second());
    Serial.print(F("."));
    if (gps.time.centisecond() < 10) Serial.print(F("0"));
    Serial.print(gps.time.centisecond());
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.println();
}
