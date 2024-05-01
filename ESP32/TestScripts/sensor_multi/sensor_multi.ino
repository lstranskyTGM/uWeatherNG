/*
 * This is a test file for reading the bme280 and KY018 sensor.
 */

#include <Wire.h>
#include <BH1750.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define SEALEVELPRESSURE_HPA (1013.25)

BH1750 lightMeter;
Adafruit_BME280 bme; // I2C

void setup() {
  Serial.begin(115200);

  Wire.begin();

  lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, 0x23);

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
  float lux = lightMeter.readLightLevel();
  Serial.print("Light: ");
  Serial.print(lux);
  Serial.println(" Lx");
  
  Serial.println();
}

void printBME280() {
  Serial.println("BME280 Messung:");
  Serial.println("---------------");
  Serial.print("Temperature: ");
  Serial.print(bme.readTemperature());
  Serial.println(" *C");
  
  Serial.print("Pressure: ");
  Serial.print(bme.readPressure() / 100.0F);
  Serial.println(" hPa");

  Serial.print("Approx. Altitude: ");
  Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
  Serial.println(" m");

  Serial.print("Humidity: ");
  Serial.print(bme.readHumidity());
  Serial.println(" %");

  Serial.println();
}
