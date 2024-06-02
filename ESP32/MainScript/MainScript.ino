/*
 * This is the main script for the weather datalogging Project
 * Projekt: uWeather
 */

// Libraries
#include <time.h>
#include <ESP32Time.h>
#include <Wire.h>
#include <BH1750.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_SGP30.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <HardwareSerial.h>
#include <BotleticsSIM7000.h>

// LTE modem details
#define SIMCOM_7000
#define TX 16 // ESP32 hardware serial RX2 (GPIO16)
#define RX 17 // ESP32 hardware serial TX2 (GPIO17)

// MQTT Broker details
#define MQTT_SERVER      "mqtt.broker.net"
#define MQTT_PORT        1883
#define MQTT_USERNAME    "YourMQTTUsername"
#define MQTT_PASSWORD    "YourMQTTPassword"
#define MQTT_CLIENTID    "YourClientID"

// Other Settings

// Boolean flag indicating whether MQTT messages should be retained on the broker.
// Retain messages are stored on the broker and sent to new subscribers when they subscribe to a topic.
// 0 (false) means messages are not retained, 1 (true) means messages are retained.
const bool retain_messages = 0;

// Quality of Service (QoS) level for MQTT messages.
// QoS 0: The broker/client will deliver the message once, with no confirmation.
// QoS 1: The broker/client will deliver the message at least once, with confirmation required.
// QoS 2: The broker/client will deliver the message exactly once by using a four-step handshake.
// This setting specifies a QoS level of 2, ensuring that messages are delivered exactly once.
const int qos_level = 2;

// MQTT Topics
// const char* topicSensors = "sensors/data";

// For ESP32 hardware serial
HardwareSerial modemSS(1);
Botletics_modem_LTE modem = Botletics_modem_LTE();

// Declarations for ESP32 RTC
//ESP32Time rtc;

// Declarations for DeepSleep
#define uS_TO_S_FACTOR 1000000ULL   // Conversion factor for micro seconds to seconds
#define TIME_TO_SLEEP 5 * 60        // Time ESP32 will go to sleep (in seconds)

// Variables that get saved during DeepSleep
RTC_DATA_ATTR int bootCount = 0;
//RTC_DATA_ATTR boolean requestedNTP;               // If NTP Server was already requested
RTC_DATA_ATTR boolean bIsRaining = false;         // Last Known Rain Value for next boot (Inital state not raining)

// Declarations for Display
#define SCREEN_WIDTH 128        // OLED display width, in pixels
#define SCREEN_HEIGHT 64        // OLED display height, in pixels
#define OLED_RESET     -1       // Reset pin
//#define SCREEN_ADDRESS 0x3c     // Address 0x3D for 128x64
#define WHITE 1
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Declarations for OLED console buffer
String textLines[]={"", "", "", "", "", "", "", ""};
int curLine=0;

// Sensors/Modules
// Sensor Object Declerations
BH1750 lightMeter;      // BH1750 Light Level Sensor (Lightlevel[lux])
Adafruit_BME280 bme;    // BME280 Sensor (Temperature[°C], Humidity[%RH"], Pressure[hPa])
Adafruit_SGP30 sgp;     // SGP30 Air Quality Sensor (TVOC[ppb], CO2[ppm])

// Pin Definitions
#define RAIN_ANALOG_PIN 35              // Rain Sensor Analog Pin (Rain[True/False])
#define WIND_SPEED_DIGITAL_PIN 34       // KY-003 Hall Sensor Digital Pin (WindSpeed[km/h])
#define WIND_DIRECTION_ANALOG_PIN 36    // Rotary Hall Angle Sensor Analog Pin (WindDirection[0-360°])

// Other variables
volatile int rotationCount = 0;                // Counts Rotations (volatile for interrupts)
unsigned long measurementDuration = 10000;     // Measurement duration in milliseconds (10 seconds)

// Defining Sensor Data Struct
struct SensorData {
    // Location Data
    float latitude;         // Latitude coordinate of the sensor's location
    float longitude;        // Longitude coordinate of the sensor's location
    float altitude;         // Altitude in meters above sea level

    // Weather Data
    float temperature;      // Ambient temperature measured in degrees Celsius
    float pressure;         // Atmospheric pressure measured in hectoPascals (hPa)
    float humidity;         // Relative humidity measured in percent (%)
    bool rain;              // Presence of rain: true if raining, false otherwise

    // Environmental Data
    float lightLevel;       // Ambient light level measured in lux
    float windSpeed;        // Wind speed measured in kilometers per hour (km/h)
    int windDirection;      // Wind direction measured in degrees from true north

    // Air Quality Data
    unsigned int tvoc;      // Total Volatile Organic Compounds measured in parts per billion (ppb)
    unsigned int eco2;      // Equivalent CO2 measured in parts per million (ppm)
};

// Defining GPS Data Struct
struct GPSData {
    float latitude;         // Latitude in decimal degrees
    float longitude;        // Longitude in decimal degrees
    float altitude;         // Altitude in meters above sea level
    bool valid;             // Validity of the GPS data
};

// ESP32 ID
static const int ESP32_ID = 101;

void setup() {
  Serial.begin(115200);

  // Initalize Display first to show status messages
  initializeSH1106G();
  printLogo();
  delay(2000);

  ++bootCount;
  printStatus("BootCount: " + String(bootCount));

  // Initalize all other Sensors or Modules
  initializeLTEModem();
  initializeDevices();

  // Turn on GPS at 1 boot
  if (bootCount == 0 || getGPSStatus() == 0) {
    toggleGPS(true);
  }
  // Implement check if there is still no fix after a long time

  // Read Sensor Data
  SensorData data = readSensors();
  mqttPublish(data);

  // Enter DeepSleep
  configureDeepSleep(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  startDeepSleep();
}

void loop() {
  // Not executed because of Deep Sleep
}

// Initialization Functions

// Initialize and setups all Devices
void initializeDevices() {
  // Start i2c bus
  Wire.begin();

  // Initialize Devices
//  initializeSH1106G();
  initializeLTEModem();
  initializeBH1750();
  initializeBME280();
  initializeRain();
  initializeSGP30();
  initializeRotaryHall();
  initializeHall();
}

bool initializeSH1106G() {
  if (display.begin(0x3c, true)) { 
    Serial.println(F("SH1106G allocation successfull"));
    return true;
  } else {
    Serial.println(F("SH1106G allocation failed - check wiring"));
    return false;
  }
}

bool initializeLTEModem() {
  printStatus(F("Initializing LTE modem...(May take several seconds)"));

  // SIM7000 takes about 3s to turn on and SIM7500 takes about 15s
  // Press reset button if the module is still turning on and the board doesn't find it.
  // When the module is on it should communicate right after pressing reset

  // Start at default SIM7000 shield baud rate
  modemSS.begin(115200, SERIAL_8N1, TX, RX); // baud rate, protocol, ESP32 RX pin, ESP32 TX pin

  //printStatus(F("Configuring to 9600 baud"));
  //modemSS.println("AT+IPR=9600"); // Set baud rate
  //delay(100); // Short pause to let the command run
  //modemSS.begin(9600, SERIAL_8N1, TX, RX); // Switch to 9600

  // Check if modem is available
  if (! modem.begin(modemSS)) {
    printStatus(F("LTE modem not found - check wiring"));
  } else {
    printStatus(F("LTE modem initialized"));
  }

  // Check type
  uint8_t type = modem.type();
  printStatus(F("Modem is OK"));
  printStatus(F("Found "));
  switch (type) {
    case SIM7000:
      printStatus(F("SIM7000")); 
      break;
    default:
      printStatus(F("Unknown")); 
      break;
  }

  // Print module IMEI number.
  char imei[16] = {0}; // MUST use a 16 character buffer for IMEI!
  uint8_t imeiLen = modem.getIMEI(imei);
  if (imeiLen > 0) {
    printStatus("Module IMEI: " + String(imei));
  } else {
    printStatus(F("Failed to retrive IMEI"));
  }

  // Set modem to full functionality
  modem.setFunctionality(1); // AT+CFUN=1

  modem.setNetworkSettings(F("webLGaut")); // For HoT IoT Simcard
}

bool initializeBH1750() {
  lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, 0x23);
  float lux = lightMeter.readLightLevel();
  if (lux != 0.0) { // Zero lux reading
    printStatus(F("BH1750 sensor initialized"));
    return true;
  } else {
    printStatus(F("BH1750 zero lux detected - check wiring."));
    return false;
  }
}

bool initializeBME280() {
  if (bme.begin(0x76)) {
    printStatus(F("BME280 sensor initialization successful"));
    return true;
  } else {
    printStatus(F("BME280 sensor not found - check wiring."));
    return false;
  }
}

bool initializeRain() {
  pinMode(RAIN_ANALOG_PIN, INPUT);
  int rainValue = analogRead(RAIN_ANALOG_PIN);
    if (rainValue > 0 && rainValue < 4095) {
        printStatus(F("Rain sensor initialized"));
        return true;
    } else {
        printStatus(F("Rain sensor not found - check wiring"));
    }
}

bool initializeSGP30() {
  if (!sgp.begin()){
    printStatus(F("SGP30 sensor not found - check wiring"));
  } else {
    printStatus(F("SGP30 sensor initialized"));
    Serial.print(F("Found SGP30 serial #"));
    Serial.print(sgp.serialnumber[0], HEX);
    Serial.print(sgp.serialnumber[1], HEX);
    Serial.println(sgp.serialnumber[2], HEX);

    // Humidity compensation
    sgp.setHumidity(getAbsoluteHumidity(bme.readTemperature(), bme.readHumidity()));

    // Update Baseline values
    if (bootCount % 30 == 0) {
      uint16_t TVOC_base, eCO2_base;
      if (sgp.getIAQBaseline(&eCO2_base, &TVOC_base)) {
        printStatus("Baseline eCO2: 0x" + String(eCO2_base, HEX));
        printStatus("Baseline TVOC: 0x" + String(TVOC_base, HEX));
      } else {
        printStatus(F("Failed to get baseline readings"));
      }
    }
  }
}

bool initializeHall() {
  pinMode(WIND_SPEED_DIGITAL_PIN, INPUT);
  if (true) {   // Add check
    printStatus(F("KY-003 Hall sensor initialized"));
    return true;
  } else {
    printStatus(F("KY-003 Hall sensor not found - check wiring"));
    return false;
  }
}

bool initializeRotaryHall() {
//  pinMode(WIND_DIRECTION_ANALOG_PIN, OUTPUT);
  pinMode(WIND_DIRECTION_ANALOG_PIN, INPUT);
  int sensorValue = analogRead(WIND_DIRECTION_ANALOG_PIN);
  if (true) {   // Add check
    printStatus(F("Rotary Hall sensor initialized"));
    return true;
  } else {
    printStatus(F("Rotary Hall sensor not found - check wiring"));
    return false;
  }
}

// Data Functions

// Reads sensorData
SensorData readSensors() {
  // Create struct
  SensorData data;

  printStatus(F("Reading sensor data.."));

  // Read lightlevel (BH1750)
  data.lightLevel = lightMeter.readLightLevel();
  printStatus("LightLevel="+String(data.lightLevel)+"lux");

  // Read temperature, pressure, and humidity (BME280)
  data.temperature  = bme.readTemperature();
  data.pressure = bme.readPressure() / 100.0F;
  data.humidity = bme.readHumidity();
  printStatus("Temperature ="+String(data.temperature)+"°C");
  printStatus("Pressure="+String(data.pressure)+"hPa");
  printStatus("Humidity="+String(data.humidity)+"%RH");

  // Read rain status (FC-37)
  data.rain = measureRain();
  bIsRaining = data.rain;   // Set value for next boot
  printStatus(data.rain ? F("Raining=TRUE") : F("Raining=NO"));

  // Read air quality (SGP30)
  data.tvoc = sgp.TVOC;
  data.eco2 = sgp.eCO2; 
  // unsigned int rawH2 = sgp.rawH2;
  // unsigned int rawEthanol = sgp.rawEthanol;
  printStatus("TVOC="+String(data.tvoc)+"ppb");
  printStatus("eC02="+String(data.eco2)+"ppm");
  // printStatus("rawH2="+String(rawH2));
  // printStatus("rawEthanol="+String(rawEthanol));

  // Read wind speed (KY-003)
  data.windSpeed = measureWindSpeed();
  printStatus("WindSpeed="+String(data.windSpeed)+"km/h");

  // Read wind direction
  uint16_t windDirAnalogRead = analogRead(WIND_DIRECTION_ANALOG_PIN);
  data.windDirection = map(windDirAnalogRead, 0, 1023, 0, 359);
  printStatus("WindDirection="+String(data.windDirection)+"°");

  // Read GNSS
  GPSData gpsData = getGPSData() 
  if (gpsData.valid) {
    data.latitude = gpsData.latitude;
    data.longitude = gpsData.longitude;
    data.altitude = gpsData.altitude;
  } else {
    data.latitude = -1;  // error value
    data.longitude = -1; // error value
    data.altitude = -1;  // error value
  }

  return data;
}

/**
 * Checks the rain sensor's analog value to determine if it is raining.
 * Updates the `bIsRaining` status based on threshold values:
 * - Sets `bIsRaining` to true if the reading is below 2700.
 * - Sets `bIsRaining` to false if the reading is above 3400.
 * @return bool - True if raining, false otherwise.
 */
bool measureRain() {
  int rainAnalogVal = analogRead(RAIN_ANALOG_PIN);
  bool isRaining;
  if (!bIsRaining && rainAnalogVal < 2700) {
    isRaining = true;
  } else if (bIsRaining && rainAnalogVal > 3400) {
    isRaining = false;
  }
  return isRaining;
}

/**
 * Measures wind speed by counting rotations detected by the KY-003 Hall sensor.
 * The function attaches an interrupt to the sensor, counts rotations for a fixed duration,
 * and then calculates the wind speed based on the number of rotations.
 * @return The calculated wind speed in meters per second.
 */
float measureWindSpeed() {
  unsigned long startMillis = millis(); // Record start time
  unsigned long currentMillis;
  rotationCount = 0; // Reset rotation count at the beginning

  // Attach interrupt to count rotations
  attachInterrupt(digitalPinToInterrupt(WIND_SPEED_DIGITAL_PIN), countRotation, RISING);

  // Loop until measurementDuration passes
  do {
    currentMillis = millis(); // Update current time
  } while (currentMillis - startMillis < measurementDuration);

  // Detach the interrupt to stop counting
  detachInterrupt(digitalPinToInterrupt(WIND_SPEED_DIGITAL_PIN));

  // Assuming each rotation is 1 meter of wind travel, adjust as per your anemometer's spec
  // windSpeed = (rotationCount * 1.0 /* meters per rotation */) / (5 /* measurement interval in seconds */);
  return calculateWindSpeed(rotationCount, measurementDuration);
} 

// Network Functions

// MQTT Publish
// rtc.getTime("%A, %B %d %Y %H:%M:%S")
void mqttPublish(SensorData data) {
  // Open Connection
  modem.openWirelessConnection(true);

  // Set up MQTT parameters (see MQTT app note for explanation of parameter values)
  modem.MQTT_setParameter("URL", MQTT_SERVER, MQTT_PORT);
  // Set up MQTT username and password if necessary
  modem.MQTT_setParameter("USERNAME", MQTT_USERNAME);
  modem.MQTT_setParameter("PASSWORD", MQTT_PASSWORD);
  modem.MQTT_setParameter("CLIENTID", MQTT_CLIENTID);

  printStatus(F("Connecting to MQTT broker..."));
  if (! modem.MQTT_connect(true)) {
    printStatus(F("MQTT broker connect failed"));
  } else { 
    printStatus(F("MQTT broker connected"));
  }

  // Getting current time as Unix timestamp
  //time_t now;
  //time(&now);  // Get the current time as a time_t object

  // Topic for publish
  String topic = "sensors/" + ESP32_ID;

  // Format the sensor data into InfluxDB Line Protocol
  String lineProtocol = "weather,stationID=" + String(ESP32_ID) + " "
                        + String("latitude=") + data.latitude + ","
                        + String("longitude=") + data.longitude + ","
                        + String("altitude=") + data.altitude + ","
                        + String("temperature=") + data.temperature + ","
                        + String("pressure=") + data.pressure + ","
                        + String("humidity=") + data.humidity + ","
                        + String("rain=") + (data.rain ? "true" : "false") + ","
                        + String("lightLevel=") + data.lightLevel + ","
                        + String("windSpeed=") + data.windSpeed + ","
                        + String("windDirection=") + data.windDirection + ","
                        + String("tvoc=") + data.tvoc + ","
                        + String("eco2=") + data.eco2;

  // Publish MQTT Data
  if (!modem.MQTT_publish(topic.c_str(), lineProtocol.c_str(), lineProtocol.length(), qos_level, retain_messages)) {
    printStatus(F("Publish failed")); 
  } else {
    printStatus(F("Publish successful")); 
  }

  // Close Connection
  modem.MQTT_connect(false);
  modem.openWirelessConnection(false);
}

// GPS Functions

bool toggleGPS(bool status) {
  // Determine the action being taken based on the status
  String action = status ? "on" : "off";

  if (!modem.enableGPS(status)) {
    printStatus(String("GPS failed to toggle ") + action);
    return false;
  } else {
    printStatus(String("GPS toggled ") + action);
    return true;
  }
}

int8_t getGPSStatus() {
  int8_t stat;

  // Check GPS fix
  stat = modem.GPSstatus();
  if (stat < 0) {
    Serial.println(F("Failed to query"));
  } else if (stat == 0) {
    Serial.println(F("GPS off"));
  } else if (stat == 1) {
    Serial.println(F("No fix"));
  } else if (stat == 2) {
    Serial.println(F("2D fix"));
  } else if (stat == 3) {
    Serial.println(F("3D fix"));
  }
  return stat;
}

GPSData getGPSData() {
  GPSData gpsData;
  float latitude, longitude, speed_kph, heading, altitude;

  // For UTC time parsing.
  /* 
   float second;
  uint16_t year;
  uint8_t month, day, hour, minute;
  */

  // For UTC time data
  // if (modem.getGPS(&latitude, &longitude, &speed_kph, &heading, &altitude, &year, &month, &day, &hour, &minute, &second)) {

  // Check for GPS status before
  if (getGPSStatus() == 3) {
    if (modem.getGPS(&latitude, &longitude, &speed_kph, &heading, &altitude)) { // Use this line if UTC time is not needed
      //Serial.println(F("---------------------"));
      printStatus(F("GPS Data Retrieved Successfully"));
      printStatus("Latitude: " + String(latitude, 6) + "°");
      printStatus("Longitude: " + String(longitude, 6) + "°");
      //Serial.print(F("Speed: ")); Serial.println(speed_kph);
      //Serial.print(F("Heading: ")); Serial.println(heading);
      printStatus("Altitude: " + String(altitude) + "m");

      // Add Data to GPSData
      gpsData.latitude = latitude;
      gpsData.longitude = longitude;
      gpsData.altitude = altitude;
      gpsData.valid = true;

      // For UTC time parsing.
      /*
        Serial.println(F("---------------------"));
        Serial.print(F("Year: ")); Serial.println(year);
        Serial.print(F("Month: ")); Serial.println(month);
        Serial.print(F("Day: ")); Serial.println(day);
        Serial.print(F("Hour: ")); Serial.println(hour);
        Serial.print(F("Minute: ")); Serial.println(minute);
        Serial.print(F("Second: ")); Serial.println(second);
        Serial.println(F("---------------------"));
      */
    } else {
      printStatus("Failed to retrieve GPS data despite 3D fix");
      gpsData.valid = false;
    }
  } else {
    printStatus("GPS fix not adequate for data retrieval");
    gpsData.valid = false;
  }

  return gpsData;
}

// Time Functions

// toggle network time sync
bool toggleRTC(bool status) {
  // Determine the action being taken based on the status
  String action = status ? "on" : "off";

  if (!modem.enableRTC(status)) {
    printStatus(String("RTC failed to toggle ") + action);
    return false;
  } else {
    printStatus(String("RTC toggled ") + action);
    return true;
  }
}


// toggle NTP time sync
bool toggleNTP(bool status) {
  // Determine the action being taken based on the status
  String action = status ? "on" : "off";

  if (!modem.enableNTPTimeSync(status, F("pool.ntp.org"))) {
    printStatus(String("NTP failed to toggle ") + action);
    return false;
  } else {
    printStatus(String("NTP toggled ") + action);
    return true;
  }
}


// DeepSleep Functions
// Calculate the remaining time for deep sleep using (interval - millis())

// Starts DeepSleep and sets wakeup event
void configureDeepSleep(int sleepTime) {
  printStatus(F("Set DeepSleep WakeUp.."));
  esp_sleep_enable_timer_wakeup(sleepTime);
}

void startDeepSleep() {
  printStatus(F("Starting DeepSleep.."));
  Serial.flush(); // Waits for the transmisson of outgoing serial data to complete
  esp_deep_sleep_start();
}

// Display Functions

// Prints the Status on the Serial Monitor and OLED
void printStatus(String statusText) {
  Serial.println(statusText);
  // Status on display
  printOledLine(statusText);
  delay(1000);
}

// print console line to OLED 
// manages a console buffer of 8 lines (x 21 chars) to simulate a terminal console on OLED
void printOledLine(String statusText) {
   if (curLine > 7) {
      textLines[0] = textLines[1];
      textLines[1] = textLines[2];
      textLines[2] = textLines[3];
      textLines[3] = textLines[4];
      textLines[4] = textLines[5];
      textLines[5] = textLines[6];
      textLines[6] = textLines[7];
      textLines[7] = statusText.substring(0,21);
   } else {
      textLines[curLine] = statusText.substring(0,21);
      curLine++;
   }
   display.clearDisplay();
   display.setTextSize(1);
   display.setTextColor(WHITE);
   display.setCursor(0, 0);
   display.println(textLines[0]);
   display.println(textLines[1]);
   display.println(textLines[2]);
   display.println(textLines[3]);
   display.println(textLines[4]);
   display.println(textLines[5]);
   display.println(textLines[6]);
   display.println(textLines[7]);
   display.display(); 
}

// Prints the uweather Logo
void printLogo() {

  // 'uweather', 128x64px
  const unsigned char uWeather [] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x01, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x30, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xfe, 0x70, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x07, 0xe0, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x01, 0xc0, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0xc0, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x0f, 0xc0, 0x18, 0x1d, 0x80, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xf0, 0x1f, 0xf0, 0x38, 0x1d, 0x80, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x30, 0x1f, 0xf0, 0x78, 0x39, 0x80, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x20, 0x01, 0xf0, 0xf8, 0x31, 0x80, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0xf0, 0xf8, 0x71, 0x80, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0xf1, 0xf8, 0x63, 0xc0, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x01, 0xc0, 0x00, 0x00, 0xf3, 0xf8, 0xe3, 0xf0, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0xf3, 0xf8, 0xc0, 0x38, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0xf7, 0x79, 0xc0, 0x0c, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x01, 0xe7, 0x7b, 0x80, 0x0e, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x03, 0x01, 0xe3, 0xc1, 0xee, 0x7f, 0x80, 0x06, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0xe3, 0xc1, 0xfc, 0x7f, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0xe7, 0xc1, 0xfc, 0x7f, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0xc7, 0x81, 0xf8, 0x7e, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x01, 0x07, 0xcf, 0x81, 0xf0, 0x7c, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x01, 0x87, 0xcf, 0x01, 0xf0, 0x7c, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x01, 0xc7, 0xff, 0xf9, 0xe0, 0x78, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0xc7, 0xff, 0xf9, 0xc0, 0x70, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x07, 0xf0, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
  };

  // Display set image
  display.clearDisplay();
  display.drawBitmap(0, 0, uWeather, SCREEN_WIDTH, SCREEN_HEIGHT, WHITE);
  display.display();

}

// Calculation Functions

/**
 * return absolute humidity [mg/m^3] with approximation formula
 * @param temperature [°C]
 * @param humidity [%RH]
 * @return The absolute humidity in milligrams per cubic meter (mg/m^3).
 */
uint32_t getAbsoluteHumidity(float temperature, float humidity) {
    // approximation formula from Sensirion SGP30 Driver Integration chapter 3.15
    const float absoluteHumidity = 216.7f * ((humidity / 100.0f) * 6.112f * exp((17.62f * temperature) / (243.12f + temperature)) / (273.15f + temperature)); // [g/m^3]
    const uint32_t absoluteHumidityScaled = static_cast<uint32_t>(1000.0f * absoluteHumidity); // [mg/m^3]
    return absoluteHumidityScaled;
}

/**
 * Calculates wind speed based on the number of rotations of an anemometer and the duration of measurement.
 * Assumes each rotation corresponds to a unit distance (e.g., 1 meter) traveled by the wind.
 * @param rotations The number of rotations detected.
 * @param duration The duration of the measurement in milliseconds.
 * @return The wind speed in meters per second.
 */
float calculateWindSpeed(int rotations, unsigned long duration) {
  // Convert duration from milliseconds to seconds
  float durationInSeconds = duration / 1000.0;
  // Assuming each rotation is 1 meter of wind travel, adjust as per your anemometer's spec
  float distancePerRotation = 1.0;
  float speedMetersPerSecond = (rotations * distancePerRotation) / durationInSeconds;
  // Convert m/s to km/h
  return speedMetersPerSecond * 3.6;
}

// Interrupt Functions

/**
 * Interrupt service routine for counting rotations.
 * This function increments the global variable `rotationCount` each time a rotation is detected.
 * Should be attached to a digital pin set to interrupt on the rising edge.
 */
void countRotation() {
  rotationCount++;
}



