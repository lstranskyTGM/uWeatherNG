/*
 * This is a test file for reading the bme280 and KY018 sensor.
 */

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme; // I2C

// Sensor Pins
#define KY018 26 // KY018 pin (PhotoresistorModule)

void setup() {
  Serial.begin(115200);

  bool status;
  status = bme.begin(0x76); 
  if (!status) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }
}

void loop() {
  delay(3000); // 3 Sekunde warten
  
  printKY018();

  printBME280();
}

void printKY018() {
  Serial.println("KY018 Messung:");
  Serial.println("--------------");
  int lightLevel = analogRead(KY018);
  Serial.print("Brightness = ");
  Serial.print(lightLevel);
  Serial.println(" LUX");
  
  Serial.println();
}

void printBME280() {
  Serial.println("BME280 Messung:");
  Serial.println("---------------");
  Serial.print("Temperature = ");
  Serial.print(bme.readTemperature());
  Serial.println(" *C");
  
  Serial.print("Pressure = ");
  Serial.print(bme.readPressure() / 100.0F);
  Serial.println(" hPa");

  Serial.print("Approx. Altitude = ");
  Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
  Serial.println(" m");

  Serial.print("Humidity = ");
  Serial.print(bme.readHumidity());
  Serial.println(" %");

  Serial.println();
}
