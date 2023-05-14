// Change Power from 5V to 3.3V

// for ESP32 Microcontroller
#define rainAnalog 35
#define rainDigital 34
// #define sensorPower 7
boolean bIsRaining;

void setup() {
  Serial.begin(9600);
}
void loop() {
  // digitalWrite(sensorPower, HIGH);  // Turn the sensor ON
  // delay(10);              // Allow power to settle
  int rainAnalogVal = analogRead(rainAnalog);
  int rainDigitalVal = digitalRead(rainDigital);

  /*
  if(rainAnalogVal < 100) {
    bIsRaining = true;
  }
  else {
    bIsRaining = false;
  }
  */
  // digitalWrite(sensorPower, LOW);    // Turn the sensor OFF

  Serial.println(rainAnalogVal);

  if (rainAnalogVal < 3000) {
    Serial.println("Es regnet")
  } else {
    Serial.println("Es regnet nicht")
  }
  // Serial.print("\t");
  Serial.println("Digital: "+rainDigitalVal); // No value gets printed
  delay(1000); 
}


