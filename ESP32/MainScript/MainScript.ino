/*
 * This is the main script for the weather datalogging Project
 * Projekt: uWeather
 */

// Libraries
#include <WiFi.h>
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>
#include "time.h"
#include <ESP32Time.h>
#include <Wire.h>
#include <BH1750.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <TinyGPSPlus.h>
#include <HardwareSerial.h>

// Replace with your network login data
const char* wifi_ssid = "REPLACE_WITH_WIFI_SSID";
const char* wifi_password = "REPLACE_WITH_WIFI_PASSWORD";

// Replace with your MySQL server login data
IPAddress db_server_addr(0, 0, 0, 0);
int db_port = 3306; 
char db_user[] = "REPLACE_WITH_DB_USER"; 
char db_password[] = "REPLACE_WITH_DB_PASSWORD"; 

WiFiClient client;
MySQL_Connection conn(&client);
MySQL_Cursor* cursor;

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;  // Does the daylightOffset change by itself?

ESP32Time rtc;

#define uS_TO_S_FACTOR 1000000ULL // Conversion factor for micro seconds to seconds
#define TIME_TO_SLEEP 5 * 60  // Time ESP32 will go to sleep (in seconds)

// Variables that get saved during DeepSleep
RTC_DATA_ATTR int bootCount = 0;
RTC_DATA_ATTR boolean requestedNTP;

// Sensors
BH1750 lightMeter;
Adafruit_BME280 bme; 

// Display
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin
#define SCREEN_ADDRESS 0x3C // Address 0x3D for 128x64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// RainSensor
#define rainAnalog 35;
RTC_DATA_ATTR boolean bIsRaining;

// GPS Tracker
static const int RXPin = 17, TXPin = 16;

TinyGPSPlus gps;  // The TinyGPSPlus object

HardwareSerial SerialGPS(1);  // The serial connection to the GPS device

// ESP32 ID
static const int ESP32_ID = 101;

void setup() {
  Serial.begin(115200);
  SerialGPS.begin(9600, SERIAL_8N1, TXPin, RXPin);

  initalizeDevices();

  ++bootCount;
  print_Status("BootCount: "+String(bootCount));

  connect_to_WiFi();

  get_network_info();

  if (requestedNTP == false) {
    request_NTP_set_RTC();
    requestedNTP = true;
  }

  connect_to_MySQL();

  transfer_SensorData();

  // Disconnect from MySQL server
  conn.close();
  print_Status("Disconnected from MySQL server");

  // Disconnect from WiFi
  WiFi.disconnect();
  print_Status("Disconnected from WiFi");

  start_DeepSleep();
}

void loop() {
  // Not executed because of Deep Sleep
}

// Connect to Wi-Fi network
void connect_to_WiFi() {
  WiFi.begin(wifi_ssid, wifi_password);
  // Stuck in loop (needs to be fixed)
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    print_Status("Connecting to WiFi...");
  }
  print_Status("Connected to WiFi");
}

// Establish a MySQL connection
void connect_to_MySQL() {
  Serial.print("Connecting to MySQL Server...");
  if (conn.connect(db_server_addr, db_port, db_user, db_password)) {
    print_Status("Connected to MySQL server");
  }
  else {
    print_Status("MySQL connection failed");
  }
}

// Gets all the network info
void get_network_info(){
    if(WiFi.status() == WL_CONNECTED) {
        Serial.print("[*] Network information for ");
        Serial.println(wifi_ssid);

        Serial.println("[+] BSSID : " + WiFi.BSSIDstr());
        Serial.print("[+] Gateway IP : ");
        Serial.println(WiFi.gatewayIP());
        Serial.print("[+] Subnet Mask : ");
        Serial.println(WiFi.subnetMask());
        Serial.println((String)"[+] RSSI : " + WiFi.RSSI() + " dB");
        Serial.print("[+] ESP32 IP : ");
        Serial.println(WiFi.localIP());
    }
}

// Connects to an NTP and sets the time of the integrated RTC Module
void request_NTP_set_RTC() {
  print_Status("Requesting NTP Server...");
  // Init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  print_Status("Request successfull");

  print_Status("Writing Time into RTC Module...");
  // Set rtc time using NTP
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)){
      rtc.setTimeStruct(timeinfo); 
  }
  print_Status("Write successfull");
}

// Starts DeepSleep and sets wakeup event
void start_DeepSleep() {
  print_Status("Setting DeepSleep WakeUp");
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);

  print_Status("Starting DeepSleep...");
  Serial.flush(); // Waits for the transmisson of outgoing serial data to complete
  esp_deep_sleep_start();
}

// Initalizes and setups all Devices
void initalize_Devices() {
  Wire.begin();

  // Initalize Display
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) { 
    Serial.println(F("SSD1306 allocation failed"));
  }
  Serial.println(F("SSD1306 allocation successfull"));

  // Initalize BH1750
  lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, 0x23);

  // Initalize BME280
  bool status;
  status = bme.begin(0x76); 
  if (!status) {
    print_Status("Could not find a valid BME280 sensor, check wiring!");
  }

  // Initalize NEO-6M GPS Tracker
  
  
}

// Reads and sends the Data from the Sensors to the MySQL Server
void transfer_SensorData() {
  // Read BH1750
  float lightLevel = lightMeter.readLightLevel();

  // Read BME280
  float temparature = bme.readTemperature();
  float pressure = bme.readPressure() / 100.0F;
  float humidity = bme.readHumidity();

  // Read FC-37
  int rainAnalogVal = analogRead(rainAnalog);
  if (!bIsRaining && rainAnalogVal < 2700) {
    Serial.println("Es regnet");
    bIsRaining = true;
  } else if (bIsRaining && rainAnalogVal > 3400) {
    Serial.println("Es regnet nicht");
    bIsRaining = false;
  }

  	// Read NEO-6M GPS Tracker


    // Transfer Data
    send_SQLQuery(1, lightLevel);  // Transfer BH1750
    send_SQLQuery(2, temparature);  // Transfer BME280
    send_SQLQuery(3, pressure);  // Transfer BME280
    send_SQLQuery(4, humidity);  // Transfer BME280
    send_SQLQuery(5, (float)bIsRaining);  // Transfer FC-37
    send_SQLQuery();  // Transfer NEO-6M GPS 
    send_SQLQuery();  // Transfer NEO-6M GPS 

}

void send_SQLQuery(int messpunkt, float wert) {
  MySQL_Cursor *cursor = new MySQL_Cursor(&conn);
  char query = "INSERT INTO usr_web204_3.Messpunkte (mw_messpunkt, mw_messort, mw_wert, mw_datumZeit) VALUES("+messpunkt+", "+ESP32_ID+", "+wert+", "+rtc.getTime("%A, %B %d %Y %H:%M:%S")+");");
  cursor->execute(query);
  delete cursor;
}

void print_Status(String statusText) {
  Serial.println(statusText);
  // Status on display
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  // Display static text
  display.println(statusText);
  display.display(); 
}

void printLogo() {

  const unsigned char uWeather [] PROGMEM = {
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xf8, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x80, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x03, 0xc0, 0x00, 0x00, 0x00, 0x00, 
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0xe0, 0x00, 0x00, 0x00, 0x00, 
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xf0, 0x70, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xfc, 0x60, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x0e, 0xc0, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xc0, 0x03, 0xc0, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x00, 
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x01, 0x80, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x01, 0x80, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 
	  0x00, 0x00, 0x00, 0x00, 0x1e, 0x06, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xc6, 0x00, 0x00, 0x00, 0x00, 
	  0x00, 0x00, 0x00, 0x00, 0x1e, 0x0e, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xc6, 0x00, 0x00, 0x00, 0x00,
	  0x00, 0x00, 0x00, 0x00, 0x1e, 0x0c, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xc3, 0x00, 0x00, 0x00, 0x00, 
	  0x00, 0x00, 0x00, 0x00, 0x1e, 0x0c, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xc3, 0x00, 0x00, 0x00, 0x00, 
	  0x00, 0x00, 0x00, 0x00, 0x1e, 0x0c, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 
	  0x00, 0x00, 0x00, 0x00, 0x1e, 0x7c, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 
	  0x00, 0x00, 0x00, 0x00, 0x1e, 0x7c, 0x0f, 0x3c, 0x01, 0xc0, 0x0e, 0x03, 0x00, 0x00, 0x00, 0x00, 
	  0x00, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x0f, 0x3c, 0x01, 0xe0, 0x1e, 0x03, 0x00, 0x00, 0x00, 0x00, 
	  0x00, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x0f, 0x1e, 0x03, 0xe0, 0x1e, 0x03, 0x00, 0x00, 0x00, 0x00, 
	  0x00, 0x00, 0x00, 0x00, 0x9e, 0x00, 0x0f, 0x1e, 0x03, 0xe0, 0x1c, 0x06, 0x00, 0x00, 0x00, 0x00, 
	  0x00, 0x00, 0x00, 0x01, 0x9e, 0x00, 0x0f, 0x1e, 0x03, 0xf0, 0x1c, 0x06, 0x00, 0x00, 0x00, 0x00, 
	  0x00, 0x00, 0x00, 0x03, 0x9e, 0x00, 0x0f, 0x0e, 0x03, 0xf0, 0x3c, 0x07, 0x80, 0x00, 0x00, 0x00, 
	  0x00, 0x00, 0x00, 0x03, 0x1e, 0x00, 0x0f, 0x0f, 0x07, 0x70, 0x3c, 0x03, 0xe0, 0x00, 0x00, 0x00, 
	  0x00, 0x00, 0x00, 0x06, 0x1e, 0x00, 0x0f, 0x0f, 0x07, 0x78, 0x38, 0x00, 0x78, 0x00, 0x00, 0x00, 
	  0x00, 0x00, 0x00, 0x06, 0x1e, 0x00, 0x0f, 0x07, 0x07, 0x38, 0x78, 0x00, 0x1c, 0x00, 0x00, 0x00, 
	  0x00, 0x00, 0x00, 0x06, 0x1e, 0x00, 0x0f, 0x07, 0x0e, 0x38, 0x78, 0x00, 0x0e, 0x00, 0x00, 0x00, 
	  0x00, 0x00, 0x00, 0x0e, 0x1e, 0x00, 0x0f, 0x07, 0x8e, 0x3c, 0x70, 0x00, 0x06, 0x00, 0x00, 0x00, 
	  0x00, 0x00, 0x00, 0x0c, 0x1e, 0x00, 0x0f, 0x03, 0x8e, 0x1c, 0x70, 0x00, 0x03, 0x00, 0x00, 0x00, 
	  0x00, 0x00, 0x00, 0x0c, 0x1e, 0x00, 0x0f, 0x03, 0x9c, 0x1c, 0xf0, 0x00, 0x03, 0x00, 0x00, 0x00, 
	  0x00, 0x00, 0x00, 0x0c, 0x1e, 0x00, 0x0e, 0x03, 0x9c, 0x1c, 0xe0, 0x00, 0x03, 0x00, 0x00, 0x00, 
	  0x00, 0x00, 0x00, 0x0c, 0x1e, 0x00, 0x0e, 0x03, 0xdc, 0x0e, 0xe0, 0x00, 0x01, 0x80, 0x00, 0x00, 
	  0x00, 0x00, 0x00, 0x0e, 0x1e, 0x00, 0x1e, 0x01, 0xd8, 0x0e, 0xe0, 0x00, 0x01, 0x80, 0x00, 0x00, 
  	0x00, 0x00, 0x00, 0x06, 0x0f, 0x00, 0x1e, 0x01, 0xf8, 0x0f, 0xc0, 0x00, 0x01, 0x80, 0x00, 0x00, 
	  0x00, 0x00, 0x00, 0x06, 0x0f, 0x00, 0x3c, 0x01, 0xf8, 0x07, 0xc0, 0x00, 0x01, 0x80, 0x00, 0x00, 
  	0x00, 0x00, 0x00, 0x06, 0x0f, 0x80, 0x7c, 0x00, 0xf0, 0x07, 0xc0, 0x00, 0x03, 0x00, 0x00, 0x00, 
	  0x00, 0x00, 0x00, 0x03, 0x07, 0xe1, 0xf8, 0x00, 0xf0, 0x07, 0x80, 0x00, 0x03, 0x00, 0x00, 0x00, 
	  0x00, 0x00, 0x00, 0x03, 0x83, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 
	  0x00, 0x00, 0x00, 0x01, 0x81, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 
  	0x00, 0x00, 0x00, 0x00, 0xc0, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00, 
	  0x00, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x00, 
	  0x00, 0x00, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 
	  0x00, 0x00, 0x00, 0x00, 0x1f, 0x8f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 
	  0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 
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

