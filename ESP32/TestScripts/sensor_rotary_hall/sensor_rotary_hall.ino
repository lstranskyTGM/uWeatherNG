/*
 * This is a test file for the rotary hall sensor (Wind vane)
 */

#define WIND_DIRECTION_ANALOG_PIN 33    // Rotary Hall Angle Sensor Analog Pin (WindDirection[0-360°])

void setup()
{
  Serial.begin(9600);
  initializeRotaryHall();
}

void loop()
{
  // Read wind direction
  uint16_t windDirAnalog = analogRead(WIND_DIRECTION_ANALOG_PIN);
//  data.windDirection = map(windDirAnalog, 0, 3120, 0, 359);
  uint16_t windDirection;
  if (windDirAnalog > 3120) {
    windDirection = 0;
  } else {
    // 360 / 3120 (Max Value) = 0.115
    windDirection = windDirAnalog * 0.115;
  }
  Serial.println("WindDirectionAnalog="+String(windDirAnalog));
  Serial.println("WindDirection="+String(windDirection)+"deg");
}

/**
 * Initializes the wind vane (wind direction)
 * @return True if device is successfully initialized, false otherwise.
 */
bool initializeRotaryHall() {
//  pinMode(WIND_DIRECTION_ANALOG_PIN, OUTPUT);
  pinMode(WIND_DIRECTION_ANALOG_PIN, INPUT);
  int sensorValue = analogRead(WIND_DIRECTION_ANALOG_PIN);
  if (sensorValue >= 0 && sensorValue <= 1437) {   // Add check
    Serial.println("Rotary Hall initialized");
    return true;
  } else {
    Serial.println("Rotary Hall not found - check wiring");
    return false;
  }
}