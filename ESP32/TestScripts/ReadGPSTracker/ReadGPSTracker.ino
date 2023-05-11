/*
 * This is a test file for reading the GPS Tracker
 */

#include <TinyGPS++.h>
#include <SoftwareSerial.h>

SoftwareSerial serialGPS(4, 5); // RX, TX
TinyGPSPlus gps;
breite = 0
lange = 0

void setup() {
  Serial.begin(115200);
  serialGPS.begin(9600);
}

void loop() {
  while (serialGPS.available() > 0) {
    if (gps.encode(serialGPS.read())) {
      Serial.print("Location: ");
      breite = gps.location.lat();
      Serial.print(gps.location.lat(), 6);
      Serial.print(", ");
      lange = gps.location.lng()
      Serial.println(gps.location.lng(), 6);
    }
  }
  delay(1000);
}
