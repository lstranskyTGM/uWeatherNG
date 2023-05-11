/*
 * This is a test file for reading the Rain Sensor
 */

const int rainPin = 13; 

void setup() {
  Serial.begin(115200);
  pinMode(rainPin, INPUT_PULLUP);
}

void loop() {
  int rainValue = digitalRead(rainPin);
  if (rainValue == LOW) {
    Serial.println("Es regnet.");
  } else {
    Serial.println("Es regnet nicht.");
  }
  delay(1000); 
}
