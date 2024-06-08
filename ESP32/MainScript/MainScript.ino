/*
 * Projekt: uWeather Datalogging System
 * Description: Utilizes ESP32 to measure environmental data with various sensors, transmitting results via MQTT. 
 * Optimized for low-power consumption with automated deep sleep cycles.
 * Author: Leonhard Stransky
 * Creation Date: 2023.05.07
 */

// Libraries
#include <time.h>                       // Standard C library for time-related functions
#include <Wire.h>                       // Wire Library for I2C communication, part of the Arduino core
#include <BH1750.h>                     // Ambient light sensor library, source: https://github.com/claws/BH1750
#include <Adafruit_Sensor.h>            // Adafruit Unified Sensor Driver, source: https://github.com/adafruit/Adafruit_Sensor
#include <Adafruit_BME280.h>            // Adafruit library for BME280 sensors, source: https://github.com/adafruit/Adafruit_BME280_Library
#include <Adafruit_SGP30.h>             // Adafruit library for the SGP30 gas sensor, source: https://github.com/adafruit/Adafruit_SGP30
#include <Adafruit_GFX.h>               // Base library for Adafruit’s display drivers, source: https://github.com/adafruit/Adafruit-GFX-Library
#include <Adafruit_SH110X.h>            // Adafruit library for SH110x Monochrome OLED/LED Displays, source: https://github.com/adafruit/Adafruit_SH110X
#include <HardwareSerial.h>             // Provides Serial communication functionality on ESP32
#include <BotleticsSIM7000.h>           // Botletics SIM7000 LTE module library, source: https://github.com/botletics/SIM7000-LTE-Shield

// LTE modem details
#define SIMCOM_7000     // LTE Module Chip
#define TX 16           // ESP32 hardware serial RX2 (GPIO16)
#define RX 17           // ESP32 hardware serial TX2 (GPIO17)

// MQTT Broker details
#define MQTT_SERVER      "mqtt.broker.net"      // The address of the MQTT broker server
// Common MQTT Ports:
// 1883: Default unencrypted port
// 8883: Default encrypted port (TLS/SSL)
// 1884: MQTT over WebSockets (unencrypted)
// 8884: MQTT over WebSockets (encrypted)
#define MQTT_PORT        1883                   // The port of the MQTT broker server
#define MQTT_USERNAME    "YourMQTTUsername"     // Optional: Username for MQTT broker authentication, if required
#define MQTT_PASSWORD    "YourMQTTPassword"     // Optional: Password for MQTT broker authentication, if required
#define MQTT_CLIENTID    "YourClientID"         // Optional: A unique client identifier used to identify the device to the broker

// Other Settings

// Boolean flag indicating whether MQTT messages should be retained on the broker
// Retain messages are stored on the broker and sent to new subscribers when they subscribe to a topic
// If set to 0 (false), messages are not retained, not stored on the broker after delivery
// If set to 1 (true), messages are retained, stored on the broker and sends them to new subscribers
const bool retain_messages = 0;

// Quality of Service (QoS) level for MQTT messages
// QoS 0: The broker/client will deliver the message once, with no confirmation
// QoS 1: The broker/client will deliver the message at least once, with confirmation required
// QoS 2: The broker/client will deliver the message exactly once by using a four-step handshake
// This setting specifies a QoS level of 2, ensuring that messages are delivered exactly once
const int qos_level = 2;

// MQTT Topics
// Topic under which sensor data will be published.
const char* topicSensors = "sensors/";

// For ESP32 hardware serial
HardwareSerial modemSS(1);                            // Instance of the HardwareSerial class to manage serial communication
Botletics_modem_LTE modem = Botletics_modem_LTE();    // Instance of the LTE modem class

// Declarations for DeepSleep
#define uS_TO_S_FACTOR 1000000ULL   // Conversion factor for micro seconds to seconds
#define TIME_TO_SLEEP 5 * 60        // Time ESP32 will go to sleep (in seconds)

// Variables that get saved during DeepSleep
RTC_DATA_ATTR int bootCount = 0;                  // Counts the number of boots since last power cycle
//RTC_DATA_ATTR boolean requestedNTP;               // If NTP Server was already requested
RTC_DATA_ATTR boolean bIsRaining = false;         // Last Known Rain Value for next boot (Inital state not raining)

// Declarations for Display
#define SCREEN_WIDTH 128        // OLED display width in pixels
#define SCREEN_HEIGHT 64        // OLED display height in pixels
#define OLED_RESET     -1       // Reset pin for the OLED Display (-1 if not used)
//#define SCREEN_ADDRESS 0x3c     // Address 0x3D for 128x64
#define WHITE 1                 // Color definition for drawing text and graphics on the display
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);    // OLED display instance

// Declarations for OLED console buffer
String textLines[8] = {""};     // Buffer to hold up lines of text for OLED display output
int curLine=0;                  // Current line index in the buffer

// Sensors/Modules
// Sensor Object Declerations
BH1750 lightMeter;      // BH1750 Light Level Sensor (Lightlevel[lux])
Adafruit_BME280 bme;    // BME280 Sensor (Temperature[°C], Humidity[%RH"], Pressure[hPa])
Adafruit_SGP30 sgp;     // SGP30 Air Quality Sensor (TVOC[ppb], CO2[ppm])

// Pin Definitions
#define RAIN_ANALOG_PIN 32              // Rain Sensor Analog Pin (Rain[True/False])
#define WIND_SPEED_DIGITAL_PIN 35       // KY-003 Hall Sensor Digital Pin (WindSpeed[km/h])
#define WIND_DIRECTION_ANALOG_PIN 33    // Rotary Hall Angle Sensor Analog Pin (WindDirection[0-360°])

// Other variables
volatile int rotationCount = 0;                // Counts Rotations (volatile for interrupts)
unsigned long measurementDuration = 10000;     // Measurement duration in milliseconds (10 seconds)
bool initalizedLTE = false;                    // If the LTE modem was successfully initialized

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

    // Other Environmental Data
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

// WeatherStation ID
static const int WeatherStationID  = 101;    // Unique identifier for this weather station

/**
 * Setup function: Initializes components, manages data transmission, and configures deep sleep settings.
 */
void setup() {
  Serial.begin(115200);

  // Initalize Display first to show status messages
  initializeSH1106G();
  printLogo();
  delay(2000);

  // Increment boot count on each restart
  ++bootCount;
  printStatus("BootCount: " + String(bootCount));

  // Initalize all other Sensors or Modules
  initializeDevices();

  // Turn on GPS at first boot or if no GPS fix is detected
  if (bootCount == 1 || getGPSStatus() == 0) {
    toggleGPS(true);
  }
  // Implement check if there is still no fix after a long time

  // Read sensor data and publish it via MQTT
  SensorData data = readSensors();
  if (initalizedLTE) {
    printStatus("Publishing data...");
    mqttPublish(data);
  } else {
    printStatus("Failed to publish data (no LTE modem)");
  }

  // Configure and start deep sleep to save power
  configureDeepSleep(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  startDeepSleep();
}

void loop() {
  // Not executed because the device enters deep sleep after setup
}

// Initialization Functions

/**
 * Initializes all connected sensors and modules.
 * Starts the I2C bus and configures each device for operation.
 */
void initializeDevices() {
  // Start i2c bus
  Wire.begin();

  // Initialize individual sensors and modules
  //initializeSH1106G();
  initalizedLTE = initializeLTEModem();
  initializeBH1750();
  initializeBME280();
  initializeRain();
  initializeSGP30();
  initializeRotaryHall();
  initializeHall();
}

/**
 * Initializes the SH1106G OLED display.
 * @return True if device is successfully initialized, false otherwise.
 */
bool initializeSH1106G() {
  if (display.begin(0x3c, true)) { 
    Serial.println("SH1106G allocation successfull");
    return true;
  } else {
    Serial.println("SH1106G allocation failed - check wiring");
    return false;
  }
}

/**
 * Initializes the LTE modem
 * @return True if device is successfully initialized, false otherwise.
 */
bool initializeLTEModem() {
  printStatus("Initializing LTE modem...(May take several seconds)");

  // SIM7000 takes about 3s to turn on and SIM7500 takes about 15s
  // Press reset button if the module is still turning on and the board doesn't find it.
  // When the module is on it should communicate right after pressing reset

  // Start at default SIM7000 shield baud rate
  //modemSS.begin(115200, SERIAL_8N1, TX, RX); // baud rate, protocol, ESP32 RX pin, ESP32 TX pin

  printStatus("Configuring to 9600 baud");
  modemSS.println("AT+IPR=9600"); // Set baud rate
  delay(100); // Short pause to let the command run
  modemSS.begin(9600, SERIAL_8N1, TX, RX); // Switch to 9600

  // Check if modem is available
  if (! modem.begin(modemSS)) {
    printStatus("LTE modem not found - check wiring");
    return false;
  } else {
    printStatus("LTE modem initialized");
  }

  // Check type
  uint8_t type = modem.type();
  printStatus("Modem is OK");
  printStatus("Found ");
  switch (type) {
    case SIM7000:
      printStatus("SIM7000"); 
      break;
    default:
      printStatus("Unknown"); 
      break;
  }

  // Print module IMEI number.
  char imei[16] = {0}; // MUST use a 16 character buffer for IMEI!
  uint8_t imeiLen = modem.getIMEI(imei);
  if (imeiLen > 0) {
    printStatus("Module IMEI: " + String(imei));
  } else {
    printStatus("Failed to retrive IMEI");
  }

  // Set modem to full functionality
  modem.setFunctionality(1); // AT+CFUN=1

  modem.setNetworkSettings(F("webLGaut")); // For HoT IoT Simcard

  return true;
}

/**
 * Initializes the BH1750 (light meter)
 * @return True if device is successfully initialized, false otherwise.
 */
bool initializeBH1750() {
  lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, 0x23);
  float lux = lightMeter.readLightLevel();
  if (lux != 0.0) { // Zero lux reading
    printStatus("BH1750 initialized");
    return true;
  } else {
    printStatus("BH1750 zero lux detected - check wiring.");
    return false;
  }
}

/**
 * Initializes the BME280 (temperature, humidity, pressure)
 * @return True if device is successfully initialized, false otherwise.
 */
bool initializeBME280() {
  if (bme.begin(0x76)) {
    printStatus("BME280 initialized");
    return true;
  } else {
    printStatus("BME280 not found - check wiring.");
    return false;
  }
}

/**
 * Initializes the FC-37 (rain)
 * @return True if device is successfully initialized, false otherwise.
 */
bool initializeRain() {
  pinMode(RAIN_ANALOG_PIN, INPUT);
  int rainValue = analogRead(RAIN_ANALOG_PIN);
    if (rainValue >= 0 && rainValue <= 4095) {
      printStatus("Rain initialized");
      return true;
    } else {
      printStatus("Rain not found - check wiring");
      return false;
    }
}

/**
 * Initializes the SGP30 (air quality)
 * @return True if device is successfully initialized, false otherwise.
 */
bool initializeSGP30() {
  if (!sgp.begin()){
    printStatus("SGP30 not found - check wiring");
    return false;
  } else {
    printStatus("SGP30 initialized");
    Serial.print("Found SGP30 serial #");
    Serial.print(sgp.serialnumber[0], HEX);
    Serial.print(sgp.serialnumber[1], HEX);
    Serial.println(sgp.serialnumber[2], HEX);

    /*
    // Humidity compensation
    sgp.setHumidity(getAbsoluteHumidity(bme.readTemperature(), bme.readHumidity()));

    // Update Baseline values
    if (bootCount % 30 == 0) {
      uint16_t TVOC_base, eCO2_base;
      if (sgp.getIAQBaseline(&eCO2_base, &TVOC_base)) {
        printStatus("Baseline eCO2: 0x" + String(eCO2_base, HEX));
        printStatus("Baseline TVOC: 0x" + String(TVOC_base, HEX));
      } else {
        printStatus("Failed to get baseline readings");
      }
    }
    */

    return true;
  }
}

/**
 * Initializes the anemometer (wind speed)
 * @return True if device is successfully initialized, false otherwise.
 */
bool initializeHall() {
  pinMode(WIND_SPEED_DIGITAL_PIN, INPUT);
  if (true) {   // Add check
    printStatus("KY-003 Hall initialized");
    return true;
  } else {
    printStatus("KY-003 Hall not found - check wiring");
    return false;
  }
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
    printStatus("Rotary Hall initialized");
    return true;
  } else {
    printStatus("Rotary Hall not found - check wiring");
    return false;
  }
}

// Data Functions

/**
 * Reads and aggregates data from environmental sensors into a structured format.
 * @return SensorData struct containing all the sensor readings.
 */
SensorData readSensors() {
  // Create struct
  SensorData data;

  printStatus("Reading sensor data..");

  // Read lightlevel (BH1750)
  data.lightLevel = lightMeter.readLightLevel();
  printStatus("LightLevel="+String(data.lightLevel)+"lux");

  // Read temperature, pressure, and humidity (BME280)
  data.temperature = bme.readTemperature();
  data.pressure = bme.readPressure() / 100.0F;
  data.humidity = bme.readHumidity();
  printStatus("Temperature="+String(data.temperature)+"degC");
  printStatus("Pressure="+String(data.pressure)+"hPa");
  printStatus("Humidity="+String(data.humidity)+"%RH");

  // Read rain status (FC-37)
  data.rain = measureRain();
  printStatus(data.rain ? "Raining=TRUE" : "Raining=FALSE");

  // Read air quality (SGP30)
  data.tvoc = sgp.TVOC;
  data.eco2 = sgp.eCO2; 
//  unsigned int rawH2 = sgp.rawH2;
//  unsigned int rawEthanol = sgp.rawEthanol;
  printStatus("TVOC="+String(data.tvoc)+"ppb");
  printStatus("eC02="+String(data.eco2)+"ppm");
//  printStatus("rawH2="+String(rawH2));
//  printStatus("rawEthanol="+String(rawEthanol));

  // Read wind speed (KY-003)
  data.windSpeed = measureWindSpeed();
  printStatus("WindSpeed="+String(data.windSpeed)+"km/h");

  // Read wind direction
  uint16_t windDirAnalog = analogRead(WIND_DIRECTION_ANALOG_PIN);
//  data.windDirection = map(windDirAnalog, 0, 3120, 0, 359);
  if (windDirAnalog > 3120) {
    data.windDirection = 0;
  } else {
    // 360 / 3120 (Max Value) = 0.115
    data.windDirection = windDirAnalog * 0.115;
  }
  printStatus("WindDirectionAnalog="+String(windDirAnalog));
  printStatus("WindDirection="+String(data.windDirection)+"deg");

  // Fetch GPS data if a valid fix is available
  GPSData gpsData = getGPSData();
  if (gpsData.valid) {
    data.latitude = gpsData.latitude;
    data.longitude = gpsData.longitude;
    data.altitude = gpsData.altitude;
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
  printStatus("rainAnalog="+String(rainAnalogVal));
  if (!bIsRaining && rainAnalogVal < 2500) {
    bIsRaining = true;
  } else if (bIsRaining && rainAnalogVal > 3000) {
    bIsRaining = false;
  }
  return bIsRaining;
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

  printStatus("rotationCount=" + String(rotationCount));

  // Calculate the wind speed
  return calculateWindSpeed(rotationCount);
} 

// Network Functions

/**
 * Publishes collected sensor data to an MQTT broker using specified settings.
 * Constructs and sends a data payload formatted according to the InfluxDB Line Protocol.
 * @param data The SensorData struct containing environmental readings to be published.
 */
void mqttPublish(SensorData data) {
  // Establish a wireless connection
  modem.openWirelessConnection(true);

  // Configure MQTT settings for connection
  modem.MQTT_setParameter("URL", MQTT_SERVER, MQTT_PORT);
  // Optional settings
  modem.MQTT_setParameter("USERNAME", MQTT_USERNAME);
  modem.MQTT_setParameter("PASSWORD", MQTT_PASSWORD);
  modem.MQTT_setParameter("CLIENTID", MQTT_CLIENTID);

  printStatus("Connecting to MQTT broker...");
  if (! modem.MQTT_connect(true)) {
    printStatus("MQTT broker connect failed");
    // Closes connection on failure
    modem.openWirelessConnection(false);
    return;
  } else { 
    printStatus("MQTT broker connected");
  }

  // Getting current time as Unix timestamp
//  time_t now;
//  time(&now);  // Get the current time as a time_t object

  // Configure the topic string with the station ID
  String topic = String(topicSensors) + String(WeatherStationID);

  // Add checks for building Line Protocol

  // Build the payload in InfluxDB Line Protocol format
  String lineProtocol = "weather,stationID=" + String(WeatherStationID) + " ";

  if (!(data.latitude == 0 && data.longitude == 0 && data.altitude == 0)) {
    lineProtocol += String("latitude=") + data.latitude + ","
                  + String("longitude=") + data.longitude + ","
                  + String("altitude=") + data.altitude + ",";
  }
  lineProtocol += String("temperature=") + data.temperature + ","
                + String("pressure=") + data.pressure + ","
                + String("humidity=") + data.humidity + ","
                + String("rain=") + (data.rain ? "true" : "false") + ","
                + String("lightLevel=") + data.lightLevel + ","
                + String("windSpeed=") + data.windSpeed + ","
                + String("windDirection=") + data.windDirection;
  if (!(data.tvoc == 0 && data.eco2 == 0)) {
    lineProtocol += "," + String("tvoc=") + data.tvoc + ","
                  + String("eco2=") + data.eco2;
  }

  // Convert String to char array
  char lineProtocolChar[512];
  lineProtocol.toCharArray(lineProtocolChar, lineProtocol.length() + 1);

  // Publish MQTT Data
  if (!modem.MQTT_publish(topic.c_str(), lineProtocolChar, lineProtocol.length(), qos_level, retain_messages)) {
    printStatus("Publish failed"); 
  } else {
    printStatus("Publish successful"); 
  }

  // Close Connection (MQTT Broker & wireless)
  modem.MQTT_connect(false);
  modem.openWirelessConnection(false);
}

// GPS Functions

/**
 * Toggles the GPS module on or off.
 * @param status True to turn on the GPS, false to turn it off.
 * @return true if the GPS toggling was successful, false otherwise.
 */
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

/**
 * Queries the current GPS status from the modem and prints the result.
 * @return GPS status code
 */
int8_t getGPSStatus() {
  int8_t stat;
  printStatus("Getting GPS Status...");

  // Check GPS fix
  stat = modem.GPSstatus();
  if (stat < 0) {
    printStatus("Failed to query");
  } else if (stat == 0) {
    printStatus("GPS off");
  } else if (stat == 1) {
    printStatus("No fix");
  } else if (stat == 2) {
    printStatus("2D fix");
  } else if (stat == 3) {
    printStatus("3D fix");
  }
  return stat;
}

/**
 * Retrieves GPS data if a 3D fix is available.
 * @return Struct containing GPS data with validity flag.
 */
GPSData getGPSData() {
  GPSData gpsData;
  float latitude, longitude, speed_kph, heading, altitude;

  printStatus("Getting GPS Data...");

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
    printStatus("Got GPS Fix");
    if (modem.getGPS(&latitude, &longitude, &speed_kph, &heading, &altitude)) { // Use this line if UTC time is not needed
      //Serial.println(F("---------------------"));
      printStatus("GPS Data Retrieved Successfully");
      printStatus("Latitude: " + String(latitude, 6) + "deg");
      printStatus("Longitude: " + String(longitude, 6) + "deg");
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

/**
 * Toggles the RTC on or off.
 * @param status True to turn on the RTC, false to turn it off.
 * @return true if the toggle was successful, false otherwise.
 */
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

/**
 * Toggles the NTP time synchronizatio on or off.
 * @param status True to turn on the NTP sync, false to turn it off.
 * @return true if the toggle was successful, false otherwise.
 */
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

/**
 * Configures the ESP32 to wake up after a specified sleep time.
 * @param sleepTime Time to sleep in microseconds before waking up.
 */
void configureDeepSleep(int sleepTime) {
  printStatus("Set DeepSleep WakeUp..");
  esp_sleep_enable_timer_wakeup(sleepTime);
}

/**
 * Initiates deep sleep mode, suspending most CPU activities.
 * The ULP (Ultra-Low-Power) coprocessor remains active and can perform tasks during deep sleep.
 */
void startDeepSleep() {
  printStatus("Starting DeepSleep..");
  Serial.flush(); // Waits for the transmisson of outgoing serial data to complete
  esp_deep_sleep_start();
}

// Display Functions

/**
 * Displays a status message on both the Serial Monitor and the OLED display.
 * @param statusText The text message to be displayed.
 */
void printStatus(String statusText) {
  Serial.println(statusText);
  // Status on display
  printOledLine(statusText);
  delay(1000);
}

// print console line to OLED 
// manages a console buffer of 8 lines (x 21 chars) to simulate a terminal console on OLED

/**
 * Displays a line of text on the OLED display, managing a buffer to simulate a terminal console.
 * This function handles up to 8 lines of text, scrolling older lines upward as new ones are added.
 * @param statusText The text to display on the OLED, truncated to 21 characters if longer.
 */
void printOledLine(String statusText) {
  if (curLine > 7) {
    // Scroll lines up when the buffer is full
    for (int i = 0; i < 7; i++) {
      textLines[i] = textLines[i + 1];
    }
    textLines[7] = statusText.substring(0, 21);
  } else {
    // Add new line to buffer
    textLines[curLine++] = statusText.substring(0,21);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  for (int i = 0; i < 8; i++) {
    display.println(textLines[i]);
  }
  display.display(); 
}

/**
 * Displays the uWeather logo on the OLED screen.
 * Renders a predefined bitmap image.
 */
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
  // Draws the bitmap image at the top-left corner of the display
  display.drawBitmap(0, 0, uWeather, SCREEN_WIDTH, SCREEN_HEIGHT, WHITE);
  display.display();
}

// Other Functions

//String checkDataPoint(String dataPoint, float value, String unit, float invalidValue) {
// Add code
//}

//String buildLinePrtocol() {
// Add code
//}

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
 * Calculates wind speed based on the number of rotations and a calibrated speed factor.
 * @param rotations The number of rotations detected
 * @return The wind speed in km/h
 */
float calculateWindSpeed(int rotations) {
  float speedFactor = 0.7;
  float speedKmPerHour = rotations * speedFactor;
  return speedKmPerHour;
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





