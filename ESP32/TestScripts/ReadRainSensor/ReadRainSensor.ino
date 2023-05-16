// Change Power from 5V to 3.3V

// for ESP32 Microcontroller
#define rainAnalog 35
// #define rainDigital 34
// #define sensorPower 7
boolean bIsRaining;

void setup() {
  Serial.begin(115200);
}
void loop() {
  // digitalWrite(sensorPower, HIGH);  // Turn the sensor ON
  // delay(10);              // Allow power to settle
  int rainAnalogVal = analogRead(rainAnalog);
  // int rainDigitalVal = digitalRead(rainDigital);

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
  
  if (!bIsRaining && rainAnalogVal < 2700) {
    Serial.println("Es regnet");
    bIsRaining = true;
  } else if (bIsRaining && rainAnalogVal > 3400) {
    Serial.println("Es regnet nicht");
    bIsRaining = false;
  }

  // Serial.print("\t");
  // Serial.println("Digital: "+rainDigitalVal); // No value gets printed
  delay(1000); 
}


