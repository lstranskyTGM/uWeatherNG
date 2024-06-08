/*
 * This is a test file for the KY-003 Hall sensor (Anemometer)
 */

#define SENSOR_PIN 23
volatile int rotationCount = 0; // Counts rotations, marked volatile as it's changed inside an interrupt
unsigned long lastMeasurement = 0; // Timestamp of the last measurement
float windSpeed = 0; // Wind speed in meters per second (or any unit you prefer)
unsigned long measurementDuration = 10000; // Measurement duration in milliseconds (e.g., 10 seconds)

void setup()
{
  Serial.begin(9600);
  pinMode(SENSOR_PIN, INPUT);
}

void loop()
{
  attachInterrupt(digitalPinToInterrupt(SENSOR_PIN), countRotation, RISING); // Trigger countRotation on the RISING edge
	unsigned long startMillis = millis();
  if (startMillis - lastMeasurement >= measurementDuration) { // Measure for 5 seconds
    // Detach the interrupt to ensure rotationCount isn't changed while calculating wind speed
    detachInterrupt(digitalPinToInterrupt(SENSOR_PIN));
    
    // Assuming each rotation is 1 meter of wind travel, adjust as per your anemometer's spec
    // windSpeed = (rotationCount * 1.0 /* meters per rotation */) / (5 /* measurement interval in seconds */);
    windSpeed = calculateWindSpeed(rotationCount, measurementDuration);
    
    // Print the wind speed
    Serial.print("Wind speed: ");
    Serial.print(windSpeed);
    Serial.println(" m/s");
    
    // Reset for the next measurement
    rotationCount = 0;
    lastMeasurement = startMillis;
  }
}

float calculateWindSpeed(int rotations, unsigned long duration) {
  // Convert duration from milliseconds to seconds
  float durationInSeconds = duration / 1000.0;
  // Assuming each rotation is 1 meter of wind travel, adjust as per your anemometer's spec
  return (rotations * 1.0) / durationInSeconds;
}

// Interrupt service routine to count each rotation
void countRotation() {
  rotationCount++;
}







