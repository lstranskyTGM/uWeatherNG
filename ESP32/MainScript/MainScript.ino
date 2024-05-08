/*
 * This is the main script for the weather datalogging Project
 * Projekt: uWeather
 */

// Libraries
#include <WiFi.h>
#include "time.h"
#include <ESP32Time.h>
#include <Wire.h>
#include <BH1750.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <TinyGPSPlus.h>
#include <HardwareSerial.h>

// GPRS credentials
// const char apn[] = "YourAPN";
// const char gprsUser[] = "YourGPRSUsername";
// const char gprsPass[] = "YourGPRSPassword";

// MQTT Broker details
// const char* broker = "mqtt.broker.net";
// const int port = 8883;
// const char* client_id = "YourClientID"; 
// const char* mqtt_username = "YourMQTTUsername";
// const char* mqtt_password = "YourMQTTPassword";

// Other Settings
// const bool retain_messages = false;
// const int qos_level = 2;

// MQTT Topics
// const char* topicSensors = "sensors/data";

// Wifi login data
const char* wifi_ssid = "REPLACE_WITH_WIFI_SSID";
const char* wifi_password = "REPLACE_WITH_WIFI_PASSWORD";

// Declarations for WiFi
WiFiClient client;

// Declarations for NTP
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;  // Does the daylightOffset change by itself?

// Declarations for ESP32 RTC
ESP32Time rtc;

// Declarations for DeepSleep
#define uS_TO_S_FACTOR 1000000ULL // Conversion factor for micro seconds to seconds
#define TIME_TO_SLEEP 5 * 60  // Time ESP32 will go to sleep (in seconds)

// Variables that get saved during DeepSleep
RTC_DATA_ATTR int bootCount = 0;
RTC_DATA_ATTR boolean requestedNTP;

// Declarations for Display
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin
#define SCREEN_ADDRESS 0x3C // Address 0x3D for 128x64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Declarations for OLED console buffer
String textLines[]={"", "", "", "", "", "", "", ""};
int curLine=0;

// Declarations for Weather Sensors
BH1750 lightMeter;
Adafruit_BME280 bme; 
#define rainAnalog 35;
RTC_DATA_ATTR boolean bIsRaining;

// Declarations for GPS Sensor
static const int RXPin = 17, TXPin = 16;
TinyGPSPlus gps;  // The TinyGPSPlus object
HardwareSerial SerialGPS(1);  // The serial connection to the GPS device

// ESP32 ID
static const int ESP32_ID = 101;

void setup() {
  Serial.begin(115200);
  SerialGPS.begin(9600, SERIAL_8N1, TXPin, RXPin);

  initalizeDevices();
  printLogo();
  delay(2000);

  ++bootCount;
  printStatus("BootCount: "+String(bootCount));

  // connect_to_WiFi();

  // get_network_info();

  if (requestedNTP == false) {
    requestNTP();
    requestedNTP = true;
  }

  // connect_to_MySQL();

  // transfer_SensorData();

  // Disconnect from WiFi
  // WiFi.disconnect();
  // print_Status("Disconnected from WiFi");

  startDeepSleep();
}

void loop() {
  // Not executed because of Deep Sleep
}

// Connect to Wi-Fi network
void connectToWiFi() {
  WiFi.begin(wifi_ssid, wifi_password);
  // Stuck in loop (needs to be fixed)
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    printStatus("Connecting to WiFi...");
  }
  printStatus("Connected to WiFi");
}

// Gets all the network info
void getNetworkInfo(){
    if(WiFi.status() == WL_CONNECTED) {
        printStatus("SSID="+String(wifi_ssid));
        printStatus("IP="+String(WiFi.localIP()));
        printStatus("SubnetMask="+String(WiFi.subnetMask()));
        printStatus("Gateway="+String(WiFi.gatewayIP()));
        printStatus("RSSI="+String(WiFi.RSSI())+" dB");
        printStatus("Encryption="+String(WiFi.encryptionType()));
    }
}

// Connects to an NTP and sets the time of the integrated RTC Module
void requestNTP() {
  print_Status("Requesting NTP Server...");
  // Init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printStatus("Request successfull");

  print_Status("Writing Time to RTC..");
  // Set rtc time using NTP
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)){
      rtc.setTimeStruct(timeinfo); 
  }
  printStatus("Write successfull");
}

// Starts DeepSleep and sets wakeup event
void startDeepSleep() {
  print_Status("Set DeepSleep WakeUp..");
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);

  print_Status("Starting DeepSleep..");
  Serial.flush(); // Waits for the transmisson of outgoing serial data to complete
  esp_deep_sleep_start();
}

// Initalizes and setups all Devices
void initalizeDevices() {
  Wire.begin();

  // Initalize Display
  if (!display.begin(SCREEN_ADDRESS, true)) { 
    Serial.println(F("SH110X allocation failed"));
  }
  Serial.println(F("SH110X allocation successfull"));

  // Initalize BH1750
  lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, 0x23);

  // Initalize BME280
  bool status;
  status = bme.begin(0x76); 
  if (!status) {
    printStatus("Could not find a valid BME280 sensor, check wiring!");
  }

  // Initalize NEO-6M GPS Tracker
  
  
}

// Reads and sends the Data from the Sensors to the MySQL Server
void transferSensorData() {

  printStatus("Reading sensor data..");

  // Read BH1750
  float lightLevel = lightMeter.readLightLevel();
  printStatus("LightLevel="+String(lightLevel));

  // Read BME280
  float temparature = bme.readTemperature();
  float pressure = bme.readPressure() / 100.0F;
  float humidity = bme.readHumidity();
  printStatus("Temperature="+String(temperature));
  printStatus("Pressure="+String(pressure));
  printStatus("Humidity="+String(humidity));

  // Read FC-37
  int rainAnalogVal = analogRead(rainAnalog);
  if (!bIsRaining && rainAnalogVal < 2700) {
    bIsRaining = true;
  } else if (bIsRaining && rainAnalogVal > 3400) {
    bIsRaining = false;
  }
  if (bIsRaining) {
    printStatus("Raining=YES");
  } else {
    printStatus("Raining=NO");
  }

  // Read NEO-6M GPS Tracker


  // Transfer Data
  

}

// MQTT Publish
// rtc.getTime("%A, %B %d %Y %H:%M:%S")

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

