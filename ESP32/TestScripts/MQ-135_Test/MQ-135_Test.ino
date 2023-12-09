#include <Arduino.h>

#define analogPin 35
#define digitalPin 34

int sensorValue;
int digitalValue;

void setup()
{
  Serial.begin(9600); // sets the serial port to 9600
  pinMode(2, INPUT);
}

void loop()
{
  sensorValue = analogRead(analogPin); // read analog input pin 0
  digitalValue = digitalRead(digitalPin);
  Serial.println(sensorValue, DEC); // prints the value read
  Serial.println(digitalValue, DEC);
  delay(1000); // wait 100ms for next reading
}